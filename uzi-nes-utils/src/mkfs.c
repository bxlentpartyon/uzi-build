/**************************************************
UZI (Unix Z80 Implementation) Utilities:  mkfs.c
***************************************************/
/* This utility creates an UZI file system for a defined block device.
 * the format is:
 *                  mkfs device isize fsize
 * where: device is the logical block on which to install a filesystem
 *        isize is the number of 512-byte blocks (less two reserved for
 *            system use) to use for storing 64-byte i-nodes
 *        fsize is the total number of 512-byte blocks to assign to the
 *            filesystem.  Space available for storage of data is therefore
 *            fsize-isize blocks.
 * This utility uses some modified files from the UZI kernel, and some
 * older, simplified files.  It is compiled and linked with CP/M libraries
 * and executed as a CP/M program.
 *
 * Revisions:
 *   Based on UZI280 version (minor changes from original UZI.  HFB
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <unix.h>
#include <config.h>		/* 1.4.98 - HFB */
#include <extern.h>
#include "fs.h"

/*extern char zerobuf(); */
direct dirbuf[32] = { ROOTINODE, ".", ROOTINODE, ".." };
struct dinode inode[8];

void *mkfs(uint16 fsize, uint16 isize);
void dwrite(char *buf, uint16 blk, char *addr);
int yes(void);

int main(int argc, char *argv[])
{
    char *filename;
    void *fs_buffer;
    int fd;
    uint16 fsize, isize;

    if (argc != 4) {
	printf("Usage: mkfs filename isize fsize\n");
	exit(-1);
    }
    filename = argv[1];
    isize = (uint16) atoi(argv[2]);
    fsize = (uint16) atoi(argv[3]);

    if (fsize < 3 || isize < 2 || isize >= fsize) {
	printf("Bad parameter values\n");
	exit(-1);
    }
    printf("Making filesystem in file %s with isize %u fsize %u. Confirm? ",
	   filename, isize, fsize);
    if (!yes())
	exit(-1);

    fd = open(filename, O_RDWR | O_CREAT | O_TRUNC,
              S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0) {
	printf("Can't open file\n");
	exit(-1);
    }
    printf("Opened: %s fd %d\n", filename, fd);
    fs_buffer = mkfs(fsize, isize);
    if (write(fd, fs_buffer, fsize * BLOCKSIZE) < 0) {
        printf("Couldn't write buffer\n");
        free(fs_buffer);
        close(fd);
        exit(-1);
    }

    close(fd);
    free(fs_buffer);
    exit(0);
}


void *mkfs(uint16 fsize, uint16 isize)
{
    uint16 j;
    filesys sb;
    char *fs_buffer;

    /* Zero out the blocks */
    /*
     * I left the "zeroizing" stuff for posterity.  We really just malloc/zero
     * the whole fs all at once
     */
    printf("Zeroizing i-blocks...\n");

    fs_buffer = malloc(fsize * BLOCKSIZE);
    memset(fs_buffer, 0 , BLOCKSIZE);

    /* Initialize the super-block */
    sb.s_mounted = SMOUNTED;	/* Magic number */
    sb.s_isize = isize;
    sb.s_fsize = fsize;
    sb.s_nfree = 1;
    sb.s_free[0] = 0;
    sb.s_tfree = 0;
    sb.s_ninode = 0;
    sb.s_tinode = (8 * (isize - 2)) - 2;

    /* Free each block, building the free list */

    printf("Building free list...\n");
    for (j = fsize - 1; j > isize; --j) {
	if (sb.s_nfree == 50) {
	    dwrite(fs_buffer, j, (char *) &sb.s_nfree);
	    sb.s_nfree = 0;
	}
	++sb.s_tfree;
	sb.s_free[(sb.s_nfree)++] = j;
    }

    /* The inodes are already zeroed out */
    /* create the root dir */

    inode[ROOTINODE].i_mode = F_DIR | (0777 & MODE_MASK);
    inode[ROOTINODE].i_nlink = 3;
    inode[ROOTINODE].i_size.o_blkno = 0;
    inode[ROOTINODE].i_size.o_offset = 32;
    inode[ROOTINODE].i_addr[0] = isize;

    /* Reserve reserved inode */
    inode[0].i_nlink = 1;
    inode[0].i_mode = ~0;

    printf("Writing initial inode and superblock...\n");

    dwrite(fs_buffer, 2, (char *) inode);
    dwrite(fs_buffer, isize, (char *) dirbuf);

    /* Write out super block */
    dwrite(fs_buffer, 1, (char *) &sb);

    printf("Done.\n");

    return fs_buffer;
}


void dwrite(char* buf, uint16 blk, char *addr)
{
    bcopy(addr, buf + (blk * BLOCKSIZE), BLOCKSIZE);
}
