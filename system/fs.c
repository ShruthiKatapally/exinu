#include <kernel.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <bufpool.h>

#if FS
#include <fs.h>

static struct fsystem fsd;
struct fsystem * fsd_ptr;
int dev0_numblocks;
int dev0_blocksize;
char *dev0_blocks;

extern int dev0;

char block_cache[512];

#define SB_BLK 0
#define BM_BLK 1
#define RT_BLK 2

#define NUM_FD 16
struct filetable oft[NUM_FD];
static int next_open_fd = 0;
struct filetable * ft_ptr;

#define INODES_PER_BLOCK (fsd.blocksz / sizeof(struct inode))
#define NUM_INODE_BLOCKS (( (fsd.ninodes % INODES_PER_BLOCK) == 0) ? fsd.ninodes / INODES_PER_BLOCK : (fsd.ninodes / INODES_PER_BLOCK) + 1)
#define FIRST_INODE_BLOCK 2
#define FIRST_DATA_BLOCK (2 + NUM_INODE_BLOCKS)

struct inode blank_inode;
static int next_free_inode = 0;
static int next_free_data_block;

int fileblock_to_diskblock(int dev, int fd, int fileblock);

/* Your code goes here! */
int fcreate(char *filename, int mode) {
	int i, m;	
	int block, offset, len;
	struct dirent * dirent_ptr;	

	/* check filename length */
	/* check mode */
	/* check if file already exists */

	/* Allocate next dir entry and initialize it to empty file */
	// dirent_ptr = &fsd_ptr->root_dir.entry[fsd_ptr->root_dir.numentries++];
	dirent_ptr = &fsd.root_dir.entry[fsd.root_dir.numentries++];
	strncpy(dirent_ptr->name, filename, FILENAMELEN-1);
	dirent_ptr->name[FILENAMELEN-1]='\0';
	dirent_ptr->inode_num=-1;

	/* search for empty slot in open file table */
	for(i=0; i<NUM_FD; i++) {
		if(oft[next_open_fd].state == FSTATE_CLOSED) {
			break;
		}
		next_open_fd++;
		if(next_open_fd >= NUM_FD)
			next_open_fd = 0;
	}
	if(oft[next_open_fd].state != FSTATE_CLOSED) {
		printf("max open files reached\n");
		return SYSERR;			
	}

	/* Make an entry in open file table. */
	ft_ptr = &oft[next_open_fd];
	ft_ptr->state = FSTATE_OPEN;
	ft_ptr->fileptr = 0;
	ft_ptr->de = dirent_ptr;

	/* search for an empty inode */
	for(i=0; i<fsd.ninodes; i++) {
		get_inode_by_num(0, next_free_inode, &ft_ptr->in);
		if(ft_ptr->in.id < 0 || ft_ptr->in.id >= fsd.ninodes) {
			break;
		}
		next_free_inode++;
		if(next_free_inode >= fsd.ninodes)
			next_free_inode = 0;
	}

	if(!(ft_ptr->in.id < 0 || ft_ptr->in.id >= fsd.ninodes)) {
		printf("max files limit reached\n");
		return SYSERR;			
	}

	ft_ptr->in.id = next_free_inode;
	ft_ptr->in.type = INODE_TYPE_FILE;
	ft_ptr->in.nlink = 0;
	ft_ptr->in.device = 0;
	ft_ptr->in.size = 0;

	fsd.inodes_used++;
	/* write inode to disk */	
	put_inode_by_num(dev0, ft_ptr->in.id, &ft_ptr->in);

	printf("\nFile %s created and opened successfully.\n",dirent_ptr->name);
	return next_open_fd;

}

int fwrite(int fd, void *buf, int nbytes) {
	int block, offset, len;
	int i, m;
	int num_bytes_written = nbytes;
	int bytes_transferred = 0;
	int add_eof = 0;

	// verify arguments
	/* get the entry in open file table. */
	ft_ptr = &oft[fd];
	
	/* check if the file is open */
	if(ft_ptr->state != FSTATE_OPEN) {
		printf("file is not open\n");
		return SYSERR;	
	}

	if ( nbytes <= 0 ) {
		printf("bytes to write is less than 1\n");
		return SYSERR;
	}

	/* check if writing given data to file will exceed the max file length */
	if ( (nbytes + ft_ptr->fileptr) >= (MDEV_BLOCK_SIZE * INODEDIRECTBLOCKS) ) {
		printf("max file length exceeded\n");
		return SYSERR;
	}

	// check if current write operation overrides EOF in the file
	if ((nbytes + ft_ptr->fileptr) > ft_ptr->in.size) {
		add_eof = 1;
	}

	// fill current block
	// get current block number and offset
	block = ft_ptr->in.blocks[ft_ptr->fileptr/MDEV_BLOCK_SIZE];
	offset = ft_ptr->fileptr % MDEV_BLOCK_SIZE;
	len = nbytes < (MDEV_BLOCK_SIZE - offset) ? nbytes : (MDEV_BLOCK_SIZE - offset);

	printf("wrtining: %s\n", (char *) buf);

	if (bwrite(dev0, block, offset, buf, len)!=OK) {
		printf("block write failed\n");
		return SYSERR;
	}
	ft_ptr->fileptr += len;
	nbytes -= len;
	bytes_transferred += len;

	// extract each block from buffer and write to disk	
	while(nbytes > 0) {
		// allocate a new data block
		/* search for empty data blocks using bitmask and allocate them to current inode */
		for(i=FIRST_DATA_BLOCK; i<fsd.nblocks; i++) {
			m = getmaskbit(next_free_data_block);
			if(m == 0) {
				break;
			}
			next_free_data_block++;
			if(next_free_data_block >= MDEV_NUM_BLOCKS)
				next_free_data_block = FIRST_DATA_BLOCK;
		}
		if(m != 0) {
			printf("disk full\n");
			return SYSERR;			
		}
		// mark this block as used using bit mask
		setmaskbit(next_free_data_block);
		/* update bitmask block */
		setmaskbit(BM_BLK);
		bwrite(dev0, BM_BLK, 0, fsd.freemask, fsd.freemaskbytes);

		ft_ptr->in.blocks[ft_ptr->fileptr/MDEV_BLOCK_SIZE] = next_free_data_block;

		// fill the allocated block
		block = ft_ptr->in.blocks[ft_ptr->fileptr/MDEV_BLOCK_SIZE];
		offset = 0;
		len = nbytes < MDEV_BLOCK_SIZE ? nbytes : MDEV_BLOCK_SIZE;
		if (bwrite(dev0, block, offset, buf+bytes_transferred, len)!=OK) {
			printf("block write failed\n");
			return SYSERR;
		}
		ft_ptr->fileptr += len;
		nbytes -= len;
		bytes_transferred += len;
	}
	
	//update other attributes of ft_ptr specially file size (this can be used for EOF handling)
	if (add_eof) {
		ft_ptr->in.size = ft_ptr->fileptr;
	}
	
	/* write inode to disk */
	put_inode_by_num(dev0, ft_ptr->in.id, &ft_ptr->in);
	return num_bytes_written;	
}

int fread(int fd, void *buf, int nbytes) {
	int block, offset, len;
	int i, m;
	int num_bytes_read = nbytes;
	int bytes_transferred = 0;

	// verify arguments
	/* get the entry in open file table. */
	ft_ptr = &oft[fd];
	
	/* check if the file is open */
	if(ft_ptr->state != FSTATE_OPEN) {
		printf("file is not open\n");
		return SYSERR;	
	}

	if ( nbytes <= 0 ) {
		printf("bytes to read is less than 1\n");
		return SYSERR;
	}
	
	/* check if reading from file will exceed EOF */
	//printf("file ptr: %d\nfile size: %d\n", ft_ptr->fileptr, ft_ptr->in.size);
	if( (nbytes + ft_ptr->fileptr) > ft_ptr->in.size ) {
		printf("INFO: bytes to read was greater than file size. data is read only till the end of file.\n");
		nbytes = ft_ptr->in.size - ft_ptr->fileptr;
	}

	/* check if reading from file will exceed the max file length */
	if( (nbytes + ft_ptr->fileptr) >= (MDEV_BLOCK_SIZE * INODEDIRECTBLOCKS) ) {
		printf("bytes to read is greater than file size\n");
		return SYSERR;
	}

	// read current block
	// get current block number and offset
	block = ft_ptr->in.blocks[ft_ptr->fileptr/MDEV_BLOCK_SIZE];
	offset = ft_ptr->fileptr % MDEV_BLOCK_SIZE;
	len = nbytes < (MDEV_BLOCK_SIZE - offset) ? nbytes : (MDEV_BLOCK_SIZE - offset);

	if (bread(dev0, block, offset, buf, len)!=OK) {
		printf("block read failed\n");
		return SYSERR;
	}
	ft_ptr->fileptr += len;
	nbytes -= len;
	bytes_transferred += len;

	// extract each block from disk and copy to buffer	
	while(nbytes > 0) {
		block = ft_ptr->in.blocks[ft_ptr->fileptr/MDEV_BLOCK_SIZE];
		offset = 0;
		len = nbytes < MDEV_BLOCK_SIZE ? nbytes : MDEV_BLOCK_SIZE;
		if (block == -1)
			break;
		if (bread(dev0, block, offset, buf+bytes_transferred, len)!=OK) {
			printf("block write failed\n");
			return SYSERR;
		}
		ft_ptr->fileptr += len;
		nbytes -= len;
		bytes_transferred += len;
	}
	
	return num_bytes_read;	
}

int fseek(int fd, int offset) {
	// verify arguments
	/* get the entry in open file table. */
	ft_ptr = &oft[fd];
	
	/* check if the file is open */
	if(ft_ptr->state != FSTATE_OPEN) {
		printf("file is not open\n");
		return SYSERR;	
	}

	if ( (offset + ft_ptr->fileptr) < 0 ) {
		printf("beginning of the file reached\n");
		return SYSERR;
	}
	
	/* check if reading from file will exceed the max file length */
	if( (offset + ft_ptr->fileptr) >= (MDEV_BLOCK_SIZE * INODEDIRECTBLOCKS) ) {
		printf("bytes to read is greater than file size\n");
		return SYSERR;
	}

	/* check if seek will exceed EOF */
	if( (offset + ft_ptr->fileptr) >= ft_ptr->in.size ) {
		printf("bytes to read was greater than file size\n");
		return SYSERR;
	}
	
	ft_ptr->fileptr = offset + ft_ptr->fileptr;
	//printf("file ptr after seek: %d\n",ft_ptr->fileptr);
	return OK;
}

int fclose(int fd) {

	ft_ptr = &oft[fd];
	
	/* check if the file is open */
	if(ft_ptr->state != FSTATE_OPEN) {
		printf("file is not open\n");
		return SYSERR;	
	}
	
	ft_ptr->state = FSTATE_CLOSED;

	return OK;
}

int mkfs(int dev, int num_inodes) {
  int i, j;

  if (dev == 0) {
    fsd.nblocks = dev0_numblocks;
    fsd.blocksz = dev0_blocksize;
  }
  else {
    printf("Unsupported device\n");
    return SYSERR;
  }

  if (num_inodes < 1) {
    fsd.ninodes = DEFAULT_NUM_INODES;
  }
  else {
    fsd.ninodes = num_inodes;
  }

  i = fsd.nblocks;
  while ( (i % 8) != 0) {i++;}
  fsd.freemaskbytes = i / 8;

  if ((fsd.freemask = memget(fsd.freemaskbytes)) == (void *)SYSERR) {
    printf("mkfs memget failed.\n");
    return SYSERR;
  }

  /* zero the free mask */
  for(i=0;i<fsd.freemaskbytes;i++) {
    fsd.freemask[i] = '\0';
  }

  fsd.inodes_used = 0;

  /* write the fsystem block to SB_BLK, mark block used */
  setmaskbit(SB_BLK);
  bwrite(dev0, SB_BLK, 0, &fsd, sizeof(struct fsystem));

  /* write the free block bitmask in BM_BLK, mark block used */
  setmaskbit(BM_BLK);
  bwrite(dev0, BM_BLK, 0, fsd.freemask, fsd.freemaskbytes);

  // Save reference to file system block. This is equivalent to copying file system from disk to memory.
  //fsd_ptr = (struct fsystem *) dev0_blocks;
  //fsd_prt->freemask = (char *) dev0_blocks + fsd_ptr->blocksz;

  // initialize open file table.
  for (i=0; i<NUM_FD; i++) {
    oft[i].state = FSTATE_CLOSED;
    oft[i].fileptr = -1;
    oft[i].de = NULL;
    oft[i].in.id = -1;
    oft[i].in.type = INODE_TYPE_FILE;
    oft[i].in.nlink = -1;
    oft[i].in.device = 0;
    oft[i].in.size = -1;
    for(j=0; j<INODEBLOCKS; j++)
      oft[i].in.blocks[j] = -1;
  }

  next_free_data_block = FIRST_DATA_BLOCK;

  return 1;
}

int fileblock_to_diskblock(int dev, int fd, int fileblock) {
  int diskblock;

  if (fileblock >= INODEBLOCKS - 2) {
    printf("No indirect block support\n");
    return SYSERR;
  }

  diskblock = oft[fd].in.blocks[fileblock]; //get the logical block address

  return diskblock;
}

/* read in an inode and fill in the pointer */
int get_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;
  int inode_off;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    printf("get_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  inode_off = inn * sizeof(struct inode);

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  printf("inn*sizeof(struct inode): %d\n", inode_off);
  */

  bread(dev0, bl, 0, &block_cache[0], fsd.blocksz);
  memcpy(in, &block_cache[inode_off], sizeof(struct inode));

  return OK;

}

int put_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    printf("put_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  */

  bread(dev0, bl, 0, block_cache, fsd.blocksz);
  memcpy(&block_cache[(inn*sizeof(struct inode))], in, sizeof(struct inode));
  bwrite(dev0, bl, 0, block_cache, fsd.blocksz);

  return OK;
}

/* specify the block number to be set in the mask */
int setmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  fsd.freemask[mbyte] |= (0x80 >> mbit);
  return OK;
}

/* specify the block number to be read in the mask */
int getmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  return( ( (fsd.freemask[mbyte] << mbit) & 0x80 ) >> 7);
  return OK;

}

/* specify the block number to be unset in the mask */
int clearmaskbit(int b) {
  int mbyte, mbit, invb;
  mbyte = b / 8;
  mbit = b % 8;

  invb = ~(0x80 >> mbit);
  invb &= 0xFF;

  fsd.freemask[mbyte] &= invb;
  return OK;
}

/* This is maybe a little overcomplicated since the lowest-numbered
   block is indicated in the high-order bit.  Shift the byte by j
   positions to make the match in bit7 (the 8th bit) and then shift
   that value 7 times to the low-order bit to print.  Yes, it could be
   the other way...  */
void printfreemask(void) {
  int i,j;

  for (i=0; i < fsd.freemaskbytes; i++) {
    for (j=0; j < 8; j++) {
      printf("%d", ((fsd.freemask[i] << j) & 0x80) >> 7);
    }
    if ( (i % 8) == 7) {
      printf("\n");
    }
  }
  printf("\n");
}

#endif /* FS */

