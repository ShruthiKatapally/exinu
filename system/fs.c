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
int next_open_fd = 0;
struct filetable * ft_ptr;

#define INODES_PER_BLOCK (fsd.blocksz / sizeof(struct inode))
#define NUM_INODE_BLOCKS (( (fsd.ninodes % INODES_PER_BLOCK) == 0) ? fsd.ninodes / INODES_PER_BLOCK : (fsd.ninodes / INODES_PER_BLOCK) + 1)
#define FIRST_INODE_BLOCK 2

struct inode blank_inode;
int next_free_inode = FIRST_INODE_BLOCK;

int fileblock_to_diskblock(int dev, int fd, int fileblock);

/* Your code goes here! */
int fcreate(char *filename, int mode) {
	int i;	
	struct dirent * dirent_ptr;	

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
	
	/* Make an entry in open file table. */
	ft_ptr = &oft[next_open_fd];
	ft_ptr->state = FSTATE_OPEN;
	ft_ptr->fileptr = 0;
	ft_ptr->de = dirent_ptr;

	/* search for an empty inode */
	
	/* search for empty slot in bitmask */
	for(i=0; i<fsd.ninodes; i++) {
		if(getmaskbit(next_free_inode) == 0) {
			break;
		}
		next_free_inode++;
		if(next_free_inode >= MDEV_NUM_BLOCKS)
			next_free_inode = FIRST_INODE_BLOCK;
	}
	
	ft_ptr->in.id = next_free_inode;
	ft_ptr->in.type = INODE_TYPE_FILE;
	ft_ptr->in.nlink = 0;
	ft_ptr->in.device = 0;
	ft_ptr->in.size = 0;
	ft_ptr->in.blocks[0] = -1;

	printf("\nFile %s created successfully.\n",dirent_ptr->name);
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

