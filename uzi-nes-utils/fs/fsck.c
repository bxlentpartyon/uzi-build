#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "unix.h"
#include "extern.h"
#include "linux_fs.h"

#define MAXDEPTH 20   /* Maximum depth of directory tree to search */

/* This checks a filesystem */

int fd;
struct filesys *filsys;
char fs_buf[DUMMY_BLOCKSIZE] = { 0 };

char *bitmap;
int16 *linkmap;


char *daread(uint16 blk);
void dwrite(uint16 blk, char *addr);
void iread(uint16 ino, struct dinode *buf);
void iwrite(uint16 ino, struct dinode *buf);
void dirread(struct dinode *ino, uint16 j, struct direct *dentry);
void dirwrite(struct dinode *ino, uint16 j, struct direct *dentry);
blkno_t getblkno(struct dinode *ino, blkno_t num);
void setblkno(struct dinode *ino, blkno_t num, blkno_t dnum);
blkno_t blk_alloc(struct filesys *filsys);
void ckdir(uint16 inum, uint16 pnum, char *name);
void mkentry(uint16 inum);

void panic(char *msg);

void pass1(void);
void pass2(void);
void pass3(void);
void pass4(void);
void pass5(void);

int main(int argc, char **argv)
{
    char *buf, *filename;

    if (argc != 2)
    {
	fprintf(stderr,"Usage: fsck filename\n");
	exit(-1);
    }


    filename = argv[1];
    printf("Running check on file %s\n", filename);

    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
	fprintf(stderr,"Can't open file %s\n", filename);
	exit(-1);
    }

    /* Read in the super block. */

    buf = daread(1);
    filsys = (filesys *) fs_buf;
    bcopy(buf, (char *) filsys, sizeof(struct filesys));

    /* Verify the fsize and isize parameters */

    if (filsys->s_mounted != SMOUNTED)
    {
	printf("File %s has invalid magic number %d. Fix? ",
	       filename, filsys->s_mounted);
	if (!yes())
	    exit(-1);
	filsys->s_mounted = SMOUNTED;
	dwrite((blkno_t)1,(char *) filsys);
    }

    printf("File %s has fsize = %d and isize = %d. Continue? ",
		filename, filsys->s_fsize, filsys->s_isize);
    if (!yes())
	exit(-1);

    bitmap = calloc(filsys->s_fsize,sizeof(char));
    linkmap = (int16 *)calloc(8 * filsys->s_isize, sizeof(int16));

    if (!bitmap || !linkmap)
    {
	fprintf(stderr,"Not enough memory.\n");
	exit(-1);
    }

    printf("Pass 1: Checking inodes.\n");
    pass1();

    printf("Pass 2: Rebuilding free list.\n");
    pass2();

    printf("Pass 3: Checking block allocation.\n");
    pass3();

    printf("Pass 4: Checking directory entries.\n");
    pass4();

    printf("Pass 5: Checking link counts.\n");
    pass5();

    printf("Done.\n");

    exit(0);
}



/* Pass 1 checks each inode independently for validity, zaps bad block
numbers in the inodes, and builds the block allocation map. */

void pass1()
{
    uint16 n;
    struct dinode ino;
    uint16 mode;
    blkno_t b;
    blkno_t bno;
    uint16 icount;
    blkno_t *buf;

    icount = 0;

    for (n = ROOTINODE; n < 8*(filsys->s_isize-2); ++n)
    {
	iread(n, &ino);
	linkmap[n] = -1;

	if (ino.i_mode == 0)
	    continue;

	mode = ino.i_mode & F_MASK;

	/* Check mode */
	if (mode != F_REG && mode != F_DIR && mode != F_BDEV && mode != F_CDEV)
	{
	    printf("Inode %d with mode 0%o is not of correct type. Zap? ",
		    n, ino.i_mode);
	    if (yes())
	    {
		ino.i_mode = 0;
		ino.i_nlink = 0;
		iwrite(n, &ino);
		continue;
	    }
	}

	linkmap[n] = 0;
	++icount;
	/* Check size */

	if (ino.i_size.o_offset >= 512)
	{
	    printf("Inode %d offset is too big with value of %d. Fix? ",
		    n,ino.i_size.o_offset);
	    if (yes())
	    {
		while (ino.i_size.o_offset >= 512)
		{
		    ino.i_size.o_offset -= 512;
		    ++ino.i_size.o_blkno;
		}
		iwrite(n,&ino);
	    }
	}


	if (ino.i_size.o_offset < 0)
	{
	    printf("Inode %d offset is negative with value of %d. Fix? ",
		    n,ino.i_size.o_offset);
	    if (yes())
	    {
		ino.i_size.o_offset = 0;
		iwrite(n,&ino);
	    }
	}

	
	/* Check blocks and build free block map */
	if (mode == F_REG || mode == F_DIR)
	{
	    /* Check singly indirect blocks */

	    for (b=18; b < 20; ++b)
	    {
		if (ino.i_addr[b] != 0 && (ino.i_addr[b] < filsys->s_isize ||
			ino.i_addr[b] >= filsys->s_fsize))
		{
		    printf("Inode %d singly indirect block %d is out of range with value of %u. Zap? ",
			    n, b, ino.i_addr[b]);
		    if (yes())
		    {
			ino.i_addr[b] = 0;
			iwrite(n, &ino);
		    }
		}

		if (ino.i_addr[b] != 0 && ino.i_size.o_blkno < 18)
		{
		    printf("Inode %d singly indirect block %d is past end of file with value of %u. Zap? ",
			    n, b, ino.i_addr[b]);
		    if (yes())
		    {
			ino.i_addr[b] = 0;
			iwrite(n, &ino);
		    }
		}
		if (ino.i_addr[b] != 0)
		    bitmap[ino.i_addr[b]] = 1;
	    }

	    /* Check the double indirect blocks */
	    if (ino.i_addr[19] != 0)
	    {
		buf = (blkno_t *)daread(ino.i_addr[19]);
		for (b=0; b < 256; ++b)
		{
		    if (buf[b] != 0 && (buf[b] < filsys->s_isize ||
			    buf[b] >= filsys->s_fsize))
		    {
			printf("Inode %d doubly indirect block %d is out of range with value of %u. Zap? ",
				n, b, buf[b]);
			if (yes())
			{
			    buf[b] = 0;
			    dwrite(b, (char *)buf);
			}
		    }
		    if (buf[b] != 0)
			bitmap[buf[b]] = 1;
		    
		}
	    }


	    /* Check the rest */
	    for (bno=0; bno <= ino.i_size.o_blkno; ++bno)
	    {
		b = getblkno(&ino, bno);

	        if (b != 0 && (b < filsys->s_isize ||
			b >= filsys->s_fsize))
		{
		    printf("Inode %d block %d is out of range with value of %u. Zap? ",
			    n, bno, b);
		    if (yes())
		    {
			setblkno(&ino, bno,0);
			iwrite(n, &ino);
		    }
		}

		if (b != 0)
		    bitmap[b] = 1;
	    }

	}
    }

    /* Fix free inode count in super block */
    if (filsys->s_tinode != 8*(filsys->s_isize-2) - ROOTINODE - icount)
    {
	printf("Free inode count in super block is %u should be %u. Fix? ",
	    filsys->s_tinode, 8*(filsys->s_isize-2) - ROOTINODE - icount);
	
	if (yes())
	{
	    filsys->s_tinode = 8*(filsys->s_isize-2) - ROOTINODE - icount;
	    dwrite((blkno_t)1,(char *) filsys);
	}
    }
}
		    


/* Clear inode free list, rebuild block free list using bit map. */

void pass2()
{
    blkno_t j;
    blkno_t oldtfree;

    printf("Rebuild free list? ");
    if (!yes())
	return;

    oldtfree = filsys->s_tfree;

    /* Initialize the super-block */

    filsys->s_ninode =  0;
    filsys->s_nfree =  1;
    filsys->s_free[0] =  0;
    filsys->s_tfree =  0;

    /* Free each block, building the free list */

    for (j= filsys->s_fsize-1; j >= filsys->s_isize; --j)
    {
	if (bitmap[j] == 0)
	{
	    if (filsys->s_nfree == 50)
	    {
		dwrite(j, (char *)&filsys->s_nfree);
		filsys->s_nfree = 0;
	    }

	    ++(filsys->s_tfree);
	    filsys->s_free[(filsys->s_nfree)++] = j;
	}
    }

    dwrite((blkno_t)1,(char *) filsys);

    if (oldtfree != filsys->s_tfree)
	printf("During free list regeneration s_tfree was changed to %d from %d.\n",
		filsys->s_tfree, oldtfree);

}



/* Pass 3 finds and fixes multiply allocated blocks. */
void pass3()
{
    uint16 n;
    struct dinode ino;
    uint16 mode;
    blkno_t b;
    blkno_t bno;
    blkno_t newno;

    for (b = filsys->s_isize; b < filsys->s_fsize; ++b)
	bitmap[b] = 0;

    for (n = ROOTINODE; n < 8*(filsys->s_isize-2); ++n)
    {
	iread(n, &ino);

	mode = ino.i_mode & F_MASK;
	if (mode != F_REG && mode != F_DIR)
	    continue;

	/* Check singly indirect blocks */

	for (b=18; b < 20; ++b)
	{
	    if (ino.i_addr[b] != 0)
	    {
		if (bitmap[ino.i_addr[b]] != 0)
		{
		    printf("Indirect block %d in inode %u value %u multiply allocated. Fix? ", b, n, ino.i_addr[b]);
		    if (yes())
		    {
			newno = blk_alloc(filsys);
			if (newno == 0)
			    printf("Sorry... No more free blocks.\n");
			else
			{
			    dwrite(newno,daread(ino.i_addr[b]));
			    ino.i_addr[b] = newno;
			    iwrite(n, &ino);
			}
		    }
		}
		else
		    bitmap[ino.i_addr[b]] = 1;
	    }
	}

	/* Check the rest */
	for (bno=0; bno <= ino.i_size.o_blkno; ++bno)
	{
	    b = getblkno(&ino, bno);

	    if (b != 0)
	    {
		if (bitmap[b] != 0)
		{
		    printf("Block %d in inode %u value %u multiply allocated. Fix? ",
		    	    bno, n, b);
		    if (yes())
		    {
			newno = blk_alloc(filsys);
			if (newno == 0)
			    printf("Sorry... No more free blocks.\n");
			else
			{
			    dwrite(newno,daread(b));
			    setblkno(&ino, bno, newno);
			    iwrite(n, &ino);
			}
		    }
		}
		else
		    bitmap[b] = 1;
	    }
	}

    }

}

int depth;

/* Pass 4 traverses the directory tree, fixing bad directory entries
and finding the actual number of references to each inode. */

void pass4()
{
    depth = 0;
    linkmap[ROOTINODE] = 1;
    ckdir(ROOTINODE,ROOTINODE,"/");
    if (depth != 0)
	panic("Inconsistent depth");
}



/* This recursively checks the directories */
void ckdir(uint16 inum, uint16 pnum, char *name)
{
    struct dinode ino;
    struct direct dentry;
    uint16 j;
    int c;
    int nentries;
    char ename[150];

    iread(inum, &ino);
    if ((ino.i_mode & F_MASK) != F_DIR)
	return;
    ++depth;

    if (ino.i_size.o_offset % 16 != 0)
    {
	printf("Directory inode %d has improper length. Fix? ");
	if (yes())
	{
	    ino.i_size.o_offset &= (~0x0f);
	    ++ino.i_size.o_offset;
	    iwrite(inum, &ino);
	}
    }

    nentries = 8*ino.i_size.o_blkno + ino.i_size.o_offset/16;

    for (j=0; j < nentries; ++j)
    {
	dirread(&ino, j, &dentry);

	if (dentry.d_ino == 0)
	    continue;

	if (dentry.d_ino < ROOTINODE || dentry.d_ino >= 8*filsys->s_isize)
	{
	    printf("Directory entry %s%-1.14s has out-of-range inode %u. Zap? ",
		   name, dentry.d_name, dentry.d_ino);
	    if (yes())
	    {
		dentry.d_ino = 0;
		dentry.d_name[0] = '\0';
		dirwrite(&ino, j, &dentry);
		continue;
	    }
	}

	if (dentry.d_ino && linkmap[dentry.d_ino] == -1)
	{
	    printf("Directory entry %s%-1.14s points to bogus inode %u. Zap? ",
		   name, dentry.d_name, dentry.d_ino);
	    if (yes())
	    {
		dentry.d_ino = 0;
		dentry.d_name[0] = '\0';
		dirwrite(&ino, j, &dentry);
		continue;
	    }
	}

	++linkmap[dentry.d_ino];

	for (c=0; c<14 && dentry.d_name[c]; ++c)
	{
	    if (dentry.d_name[c] == '/')
	    {
		printf("Directory entry %s%-1.14s contains slash. Fix? ",
		       name, dentry.d_name);
		if (yes())
		{
		    dentry.d_name[c] = 'X';
		    dirwrite(&ino, j, &dentry);
		}
	    }
	}

	if (strncmp(dentry.d_name,".",14) == 0 && dentry.d_ino != inum)
	{
	    printf("Dot entry %s%-1.14s points to wrong place. Fix? ",
		   name, dentry.d_name);
	    if (yes())
	    {
		dentry.d_ino = inum;
		dirwrite(&ino, j, &dentry);
	    }
	}

	if (strncmp(dentry.d_name,"..",14) == 0 && dentry.d_ino != pnum)
	{
	    printf("DotDot entry %s%-1.14s points to wrong place. Fix? ",
		   name, dentry.d_name);
	    if (yes())
	    {
		dentry.d_ino = pnum;
		dirwrite(&ino, j, &dentry);
	    }
	}

	if (dentry.d_ino != pnum && dentry.d_ino != inum && depth < MAXDEPTH) 
	{
	    strcpy(ename,name);
	    strcat(ename,dentry.d_name);
	    strcat(ename,"/");
	    ckdir(dentry.d_ino,inum,ename);
	}
    }
    --depth;
}


/* Pass 5 compares the link counts found in pass 4 with the inodes. */

void pass5()
{
    uint16 n;
    struct dinode ino;

    for (n = ROOTINODE; n < 8*(filsys->s_isize-2); ++n)
    {
	iread(n, &ino);

	if (ino.i_mode == 0)
	{
	    if (linkmap[n] != -1)
		panic("Inconsistent linkmap");
	    continue;
	}

	if (linkmap[n] == -1 && ino.i_mode != 0)
	    panic("Inconsistent linkmap");

	if (linkmap[n] > 0 && ino.i_nlink != linkmap[n])
	{
	    printf("Inode %d has link count %d should be %d. Fix? ",
		    n, ino.i_nlink, linkmap[n]);
	    if (yes())
	    {
		ino.i_nlink = linkmap[n];
		iwrite(n, &ino);
	    }
	}

	if (linkmap[n] == 0)
	{
	    if ((ino.i_mode&F_MASK) == F_BDEV ||
	        (ino.i_mode&F_MASK) == F_CDEV ||
	        (ino.i_size.o_blkno == 0 && ino.i_size.o_offset == 0))
	    {
		printf("Useless inode %d with mode 0%o has become detached. Link count is %d. Zap? ",
			n, ino.i_mode, ino.i_nlink);
		if (yes())
		{
		    ino.i_nlink = 0;
		    ino.i_mode = 0;
		    iwrite(n, &ino);
		    ++filsys->s_tinode;
		    dwrite((blkno_t)1,(char *) filsys);
		}
	    }
	    else
	    {
		printf("Inode %d has become detached. Link count is %d. Fix? ",
			n, ino.i_nlink);
		if (yes())
		{
		    ino.i_nlink = 1;
		    iwrite(n, &ino);
		    mkentry(n);
		}
	    }
	}
    }
}


/* This makes an entry in "lost+found" for inode n */

void mkentry(uint16 inum)
{
    struct dinode rootino;
    struct direct dentry;
    uint16 d;

    iread(ROOTINODE,&rootino);
    for (d=0; d < 32*rootino.i_size.o_blkno + rootino.i_size.o_offset/16; ++d)
    {
	dirread(&rootino,d,&dentry);
	if (dentry.d_ino == 0 && dentry.d_name[0] == '\0')
	{
	   dentry.d_ino = inum;
	   sprintf(dentry.d_name,"l+f%d",inum);
	   dirwrite(&rootino,d,&dentry);
	   return;
	}
    }
    printf("Sorry... No empty slots in root directory.\n");
}

/* Beginning of fsck1.c */

/* Getblkno gets a pointer index, and a number of a block in the file.
It returns the number of the block on the disk.  A value of zero
means an unallocated block. */

blkno_t getblkno(struct dinode *ino, blkno_t num)
{

    blkno_t indb;
    blkno_t dindb;
    blkno_t *buf;

    if (num < 18) /* Direct block */
    {
	return(ino->i_addr[num]);
    }

    if (num < 256+18)  /* Single indirect */
    {
	indb = ino->i_addr[18];
	if (indb==0)
	    return(0);
	buf = (blkno_t *)daread(indb);
	return(buf[num - 18]);
    }

    /* Double indirect */
    indb = ino->i_addr[19];
    if(indb==0)
	return(0);

    buf = (blkno_t *)daread(indb);

    dindb = buf[ (num-(18+256)) >> 8];
    buf = (blkno_t *)daread(dindb);

    return(buf[ (num-(18+256)) & 0x00ff]);
}


/* Setblkno sets the given block number of the given file to the given
disk block number, possibly creating or modifiying the indirect blocks.
A return of zero means there were no blocks available to create an 
indirect block. This should never happen in fsck. */


void setblkno(struct dinode *ino, blkno_t num, blkno_t dnum)
{

    blkno_t indb;
    blkno_t dindb;
    blkno_t *buf;

    if (num < 18) /* Direct block */
    {
	ino->i_addr[num] = dnum;
    }
    else if (num < 256+18)  /* Single indirect */
    {
	indb = ino->i_addr[18];
	if (indb==0)
	    panic("Missing indirect block");

	buf = (blkno_t *)daread(indb);
	buf[num - 18] = dnum;
	dwrite(indb,(char *)buf);
    }
    else /* Double indirect */
    {
	indb = ino->i_addr[19];
	if (indb == 0)
	    panic("Missing indirect block");

	buf = (blkno_t *)daread(indb);
	dindb = buf[ (num-(18+256)) >> 8];
	if (dindb==0)
	    panic("Missing indirect block");

	buf = (blkno_t *)daread(dindb);
	buf[ (num-(18+256)) & 0x00ff] = num;
	dwrite(indb,(char *)buf);
    }

}


/* Blk_alloc allocates an unused block.
A returned block number of zero means no more blocks. */

blkno_t blk_alloc(struct filesys *filsys)
{

    blkno_t newno;
    blkno_t *buf;
    int16 j;

    newno = filsys->s_free[--filsys->s_nfree];
    ifnot (newno)
    {
	++filsys->s_nfree;
	return(0);
    }

    /* See if we must refill the s_free array */

    ifnot (filsys->s_nfree)
    {
	buf = (blkno_t *)daread(newno);
	filsys->s_nfree = buf[0];
	for (j=0; j < 50; j++)
	{
	    filsys->s_free[j] = buf[j+1];
	}
    }

    --filsys->s_tfree;

    if (newno < filsys->s_isize || newno >= filsys->s_fsize)
    {
	printf("Free list is corrupt.  Did you rebuild it?\n");
	return(0);
    }

    dwrite((blkno_t)1,(char *) filsys);
    return(newno);
}


void fsck_abort(void)
{
	close(fd);
	exit(1);
}

void panic(char *msg)
{
	fprintf(stderr, msg);
	fsck_abort();
}

void seek_block(int fd, uint16 blk)
{
    int blkoff = blk * BLOCKSIZE;
    off_t ret = lseek(fd, blkoff, SEEK_SET);

    if (ret != blkoff)
    {
        fprintf(stderr,"Seek to block %d failed.\n",blk);
        fsck_abort();
    }
}

char readbuf[BLOCKSIZE];

char *daread(uint16 blk)
{
    int ret;

    seek_block(fd, blk);

    ret = read(fd, readbuf, BLOCKSIZE);
    if (ret < 0)
    {
	fprintf(stderr,"Read of block %d failed: %s\n", blk, strerror(errno));
        fsck_abort();
    }

    return readbuf;
}


/*
 * We need a different dwrite from mkfs, since we're operating on an fd, instead
 * of reading in the whole file up front and examining chunks of it.
 */
void dwrite(uint16 blk, char *addr)
{
    ssize_t ret;

    seek_block(fd, blk);

    ret = write(fd, addr, BLOCKSIZE);
    if (ret != BLOCKSIZE)
    {
        fprintf(stderr,"Write of block %d failed.\n",blk);
        fsck_abort();
    }
}


void iread(uint16 ino, struct dinode *buf)
{
    struct dinode *addr;

    addr = (struct dinode *) daread((ino>>3) + 2);
    bcopy((char *)&addr[ino & 7], (char *)buf, sizeof(struct dinode));
}


void iwrite(uint16 ino, struct dinode *buf)
{
    struct dinode *addr;

    addr = (struct dinode *) daread((ino>>3) + 2);
    bcopy((char *)buf, (char *)&addr[ino & 7], sizeof(struct dinode));
    dwrite((ino>>3) + 2,(char *)addr);
}


void dirread(struct dinode *ino, uint16 j, struct direct *dentry)
{
    blkno_t blkno;
    char *buf;

    blkno = getblkno(ino,(blkno_t)j/32);
    if (blkno == 0)
	panic("Missing block in directory");
    buf = daread(blkno);
    bcopy(buf+(16*j), (char *)dentry, 16);
}


void dirwrite(struct dinode *ino, uint16 j, struct direct *dentry)
{
    blkno_t blkno;
    char *buf;

    blkno = getblkno(ino,(blkno_t)j/32);
    if (blkno == 0)
	panic("Missing block in directory");
    buf = daread(blkno);
    bcopy((char *)dentry, buf+(16*j), 16);
    dwrite(blkno,buf);
}


