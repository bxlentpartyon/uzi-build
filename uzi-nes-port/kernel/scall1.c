#include <devio.h>
#include <extern.h>
#include <filesys.h>
#include <machdep.h>
#include <process.h>
#include <scall.h>
#include <unix.h>

#include <lib/string.h>

#pragma code-name (push, "SCALL1_CODE")

/*
#define name (char *)udata.u_argn1
#define flag (int16)udata.u_argn
*/

_open(char *name, int16 flag)
{
    int16 uindex;
    register int16 oftindex;
    register inoptr ino;
    register int16 perm;

    if (flag < 0 || flag > 2)
    {
	udata.u_error = EINVAL;
	return (-1);
    }
    if ((uindex = uf_alloc()) == -1)
	return (-1);

    if ((oftindex = oft_alloc()) == -1)
	goto nooft;

    ifnot (ino = n_open(name,NULLINOPTR))
	goto cantopen;

    of_tab[oftindex].o_inode = ino;

    perm = getperm(ino);
    if (((flag == O_RDONLY || flag == O_RDWR) && !(perm & OTH_RD)) ||
	((flag == O_WRONLY || flag == O_RDWR) && !(perm & OTH_WR)))
    {
	udata.u_error = EPERM;
	goto cantopen;
    }

    if (getmode(ino) == F_DIR &&
	(flag == O_WRONLY || flag == O_RDWR))
    {
	udata.u_error = EISDIR;
	goto cantopen;
    }

    if (isdevice(ino) && d_open((int)ino->c_node.i_addr[0]) != 0)
    {
	udata.u_error = ENXIO;
	goto cantopen;
    }

    udata.u_files[uindex] = oftindex;

    of_tab[oftindex].o_ptr.o_offset = 0;
    of_tab[oftindex].o_ptr.o_blkno = 0;
    of_tab[oftindex].o_access = flag;

    return (uindex);

cantopen:
    oft_deref(oftindex);  /* This will call i_deref() */
nooft:
    udata.u_files[uindex] = -1;
    return (-1);
}

_close(int uindex)
{
    return doclose(uindex);
}

#undef uindex

int doclose(int16 uindex)
{
    register int16 oftindex;
    inoptr ino;

    ifnot(ino = getinode(uindex))
	return(-1);
    oftindex = udata.u_files[uindex];

    if (isdevice(ino)
	/* && ino->c_refs == 1 && of_tab[oftindex].o_refs == 1 */ )
	d_close((int)(ino->c_node.i_addr[0]));

    udata.u_files[uindex] = -1;
    oft_deref(oftindex);

    return(0);
}

/****************************************
creat(name, mode)
char *name;
int16 mode;
*****************************************/

/*
#define name (char *)udata.u_argn1
#define mode (int16)udata.u_argn
*/

int _creat(char *name, int16 mode)
{
    register inoptr ino;
    register int16 uindex;
    register int16 oftindex;
    inoptr parent;
    register int16 j;
    inoptr n_open();
    inoptr newfile();

    parent = NULLINODE;

    if ((uindex = uf_alloc()) == -1)
	return (-1);
    if ((oftindex = oft_alloc()) == -1)
	return (-1);

    if (ino = n_open(name,&parent))  /* The file exists */
    {
	i_deref(parent);
	if (getmode(ino) == F_DIR)
	{
	    i_deref(ino);
	    udata.u_error = EISDIR;
	    goto nogood;
	}
	ifnot (getperm(ino) & OTH_WR)
	{
	    i_deref(ino);
	    udata.u_error = EACCES;
	    goto nogood;
	}
	if (getmode(ino) == F_REG)
	{
	    /* Truncate the file to zero length */
	    f_trunc(ino);
	    /* Reset any oft pointers */
	    for (j=0; j < OFTSIZE; ++j)
	        if (of_tab[j].o_inode == ino)
	            of_tab[j].o_ptr.o_blkno = of_tab[j].o_ptr.o_offset = 0;
	}

    }
    else
    {
	if (parent && (ino = newfile(parent,name)))
	         /* Parent was derefed in newfile */
	{
	    ino->c_node.i_mode = (F_REG | (mode & MODE_MASK & ~udata.u_mask));
	    setftime(ino, A_TIME|M_TIME|C_TIME);
	    /* The rest of the inode is initialized in newfile() */
	    wr_inode(ino);
	}
	else
	{
	    /* Doesn't exist and can't make it */
	    if (parent)
	        i_deref(parent);
	    goto nogood;
	}
    }

    udata.u_files[uindex] = oftindex;

    of_tab[oftindex].o_ptr.o_offset = 0;
    of_tab[oftindex].o_ptr.o_blkno = 0;
    of_tab[oftindex].o_inode = ino;
    of_tab[oftindex].o_access = O_WRONLY;

    return (uindex);

nogood:
    oft_deref(oftindex);
    return (-1);

}

/*
#undef name
#undef mode
*/

/********************************************
pipe(fildes)
int fildes[];
*******************************************/

/*
#define fildes (int *)udata.u_argn
*/

int _pipe(int fildes[])
{
    register int16 u1, u2, oft1, oft2;
    register inoptr ino;

    if ((u1 = uf_alloc()) == -1)
	goto nogood2;
    if ((oft1 = oft_alloc()) == -1)
	goto nogood2;
    udata.u_files[u1] = oft1;

    if ((u2 = uf_alloc()) == -1)
	goto nogood;
    if ((oft2 = oft_alloc()) == -1)
    {
	oft_deref(oft1);
	goto nogood;
    }

    ifnot (ino = i_open(ROOTDEV, 0))
    {
	oft_deref(oft1);
	oft_deref(oft2);
	goto nogood;
    }

    udata.u_files[u2] = oft2;

    of_tab[oft1].o_ptr.o_offset = 0;
    of_tab[oft1].o_ptr.o_blkno = 0;
    of_tab[oft1].o_inode = ino;
    of_tab[oft1].o_access = O_RDONLY;

    of_tab[oft2].o_ptr.o_offset = 0;
    of_tab[oft2].o_ptr.o_blkno = 0;
    of_tab[oft2].o_inode = ino;
    of_tab[oft2].o_access = O_WRONLY;

    ++ino->c_refs;
    ino->c_node.i_mode = F_PIPE | 0777; /* No permissions necessary on pipes */
    ino->c_node.i_nlink = 0;            /* a pipe is not in any directory */

    *fildes = u1;
    *(fildes+1) = u2;
    return (0);

nogood:
    udata.u_files[u1] = -1;
nogood2:
    return(-1);

}

/*
#undef fildes
*/

/********************************************
link(name1, name2)
char *name1;
char *name2;
*********************************************/

/*
#define name1 (char *)udata.u_argn1
#define name2 (char *)udata.u_argn
*/

int _link(char *name1, char *name2)
{
    register inoptr ino;
    register inoptr ino2;
    inoptr parent2;

    ifnot (ino = n_open(name1,NULLINOPTR))
	return(-1);

    if (getmode(ino) == F_DIR && !super())
    {
	udata.u_error = EPERM;
	goto nogood;
    }

    /* Make sure file2 doesn't exist, and get its parent */
    if (ino2 = n_open(name2,&parent2))
    {
	i_deref(ino2);
	i_deref(parent2);
	udata.u_error = EEXIST;
	goto nogood;
    }

    ifnot (parent2)
	goto nogood;

    if (ino->c_dev != parent2->c_dev)
    {
	i_deref(parent2);
	udata.u_error = EXDEV;
	goto nogood;
    }

    if (ch_link(parent2,"",filename(name2),ino) == 0)
	goto nogood;


    /* Update the link count. */
    ++ino->c_node.i_nlink;
    wr_inode(ino);
    setftime(ino, C_TIME);

    i_deref(parent2);
    i_deref(ino);
    return(0);

nogood:
    i_deref(ino);
    return(-1);

}

/**********************************************************
unlink(path)
char *path;
**************************************************/

/*
#define path (char *)udata.u_argn
*/

int _unlink(char *path)
{
    register inoptr ino;
    inoptr pino;

    ino = n_open(path,&pino);

    ifnot (pino && ino)
    {
	udata.u_error = ENOENT;
	return (-1);
    }

    if (getmode(ino) == F_DIR && !super())
    {
	udata.u_error = EPERM;
	goto nogood;
    }

    /* Remove the directory entry */

    if (ch_link(pino,filename(path),"",NULLINODE) == 0)
	goto nogood;

    /* Decrease the link count of the inode */

    ifnot (ino->c_node.i_nlink--)
    {
	ino->c_node.i_nlink += 2;
	warning("_unlink: bad nlink");
    }
    setftime(ino, C_TIME);
    i_deref(pino);
    i_deref(ino);
    return(0);

nogood:
    i_deref(pino);
    i_deref(ino);
    return(-1);
}

/*
#undef path
*/

/*****************************************************
read(d, buf, nbytes)
int16 d;
char *buf;
uint16 nbytes;
**********************************************/

/*
#define d (int16)udata.u_argn2
#define buf (char *)udata.u_argn1
#define nbytes (uint16)udata.u_argn
*/

void updoff(int16 d);
inoptr rwsetup(int16 d, char *buf, unsigned nbytes, int16 rwflag);

_read(int16 d, char *buf, unsigned nbytes)
{
    register inoptr ino;

    /* Set up u_base, u_offset, ino; check permissions, file num. */
    if ((ino = rwsetup(d, buf, nbytes, 1)) == NULLINODE)
	return (-1);   /* bomb out if error */

    readi(ino);
    updoff(d);

    return (udata.u_count);
}

/*
#undef d
#undef buf
#undef nbytes
*/

/***********************************
write(d, buf, nbytes)
int16 d;
char *buf;
uint16 nbytes;
***********************************/

/*
#define d (int16)udata.u_argn2
#define buf (char *)udata.u_argn1
#define nbytes (uint16)udata.u_argn
*/

int _write(int16 d, char *buf, uint16 nbytes)
{
    register inoptr ino;
    off_t *offp;

    /* Set up u_base, u_offset, ino; check permissions, file num. */
    if ((ino = rwsetup(d, buf, nbytes, 0)) == NULLINODE)
	return (-1);   /* bomb out if error */

    writei(ino);
    updoff(d);

    return (udata.u_count);
}

/*
#undef d
#undef buf
#undef nbytes
*/

inoptr rwsetup(int16 d, char *buf, unsigned nbytes, int16 rwflag)
{
    register inoptr ino;
    register struct oft *oftp;

    udata.u_base = buf;
    udata.u_count = nbytes;

    if ((ino = getinode(d)) == NULLINODE)
	return (NULLINODE);

    oftp = of_tab + udata.u_files[d];
    if (oftp->o_access == (rwflag ? O_WRONLY : O_RDONLY))
    {
	udata.u_error = EBADF;
	return (NULLINODE);
    }

    setftime(ino, rwflag ? A_TIME : (A_TIME | M_TIME | C_TIME));

    /* Initialize u_offset from file pointer */
    udata.u_offset.o_blkno = oftp->o_ptr.o_blkno;
    udata.u_offset.o_offset = oftp->o_ptr.o_offset;

    return (ino);
}

void readi(register inoptr ino)
{
    register uint16 amount;
    register uint16 toread;
    register blkno_t pblk;
    register char *bp;
    int dev;
    int ispipe;
    char *zerobuf();
    blkno_t bmap();

    dev = ino->c_dev;
    ispipe = 0;
    switch (getmode(ino))
    {

    case F_DIR:
    case F_REG:

	/* See of end of file will limit read */
	toread = udata.u_count =
	    ino->c_node.i_size.o_blkno-udata.u_offset.o_blkno >= 64 ?
	        udata.u_count :
	        min(udata.u_count,
	         512*(ino->c_node.i_size.o_blkno-udata.u_offset.o_blkno) +
	         (ino->c_node.i_size.o_offset-udata.u_offset.o_offset));
	goto loop;

    case F_PIPE:
	ispipe = 1;
	while (psize(ino) == 0)
	{
	    if (ino->c_refs == 1) /* No writers */
	        break;
	    /* Sleep if empty pipe */
	    psleep(ino);
	}
	toread = udata.u_count = min(udata.u_count, psize(ino));
	goto loop;

    case F_BDEV:
	toread = udata.u_count;
	dev = *(ino->c_node.i_addr);

    loop:
	while (toread)
	{
	    if ((pblk = bmap(ino, udata.u_offset.o_blkno, 1)) != NULLBLK)
	        bp = bread(dev, pblk, 0);
	    else
	        bp = zerobuf();

	    bcopy(bp+udata.u_offset.o_offset, udata.u_base,
	            (amount = min(toread, 512 - udata.u_offset.o_offset)));
	    brelse(bp);

	    udata.u_base += amount;
	    addoff(&udata.u_offset, amount);
	    if (ispipe && udata.u_offset.o_blkno >= 18)
	        udata.u_offset.o_blkno = 0;
	    toread -= amount;
	    if (ispipe)
	    {
	        addoff(&(ino->c_node.i_size), -amount);
	        wakeup(ino);
	    }
	}

	break;

    case F_CDEV:
	udata.u_count = cdread(ino->c_node.i_addr[0]);

	if (udata.u_count != -1)
	    addoff(&udata.u_offset, udata.u_count);
	break;

    default:
	udata.u_error = ENODEV;
    }
}

/* Writei (and readi) need more i/o error handling */
void writei(register inoptr ino)
{
    register uint16 amount;
    register uint16 towrite;
    register char *bp;
    int ispipe;
    blkno_t pblk;
    int created;        /* Set by bmap if newly allocated block used */
    int dev;
    char *zerobuf();
    blkno_t bmap();

    dev = ino->c_dev;

    switch (getmode(ino))
    {

    case F_BDEV:
	dev = *(ino->c_node.i_addr);
    case F_DIR:
    case F_REG:
	ispipe = 0;
	towrite = udata.u_count;
	goto loop;

    case F_PIPE:
	ispipe = 1;
	while ((towrite = udata.u_count) > (16*512) - psize(ino))
	{
	    if (ino->c_refs == 1) /* No readers */
	    {
	        udata.u_count = -1;
	        udata.u_error = EPIPE;
	        ssig(udata.u_ptab, SIGPIPE);
	        return;
	    }
	    /* Sleep if empty pipe */
	    psleep(ino);
	}

	/* Sleep if empty pipe */
	goto loop;

    loop:

	while (towrite)
	{
	    amount = min(towrite, 512 - udata.u_offset.o_offset);


	    if ((pblk = bmap(ino, udata.u_offset.o_blkno, 0)) == NULLBLK)
	        break;    /* No space to make more blocks */

	    /* If we are writing an entire block, we don't care
	    about its previous contents */
	    bp = bread(dev, pblk, (amount == 512));

	    bcopy(udata.u_base, bp+udata.u_offset.o_offset, amount);
	    bawrite(bp);

	    udata.u_base += amount;
	    addoff(&udata.u_offset, amount);
	    if(ispipe)
	    {
	        if (udata.u_offset.o_blkno >= 18)
	            udata.u_offset.o_blkno = 0;
	        addoff(&(ino->c_node.i_size), amount);
	        /* Wake up any readers */
	        wakeup(ino);
	    }
	    towrite -= amount;
	}

	/* Update size if file grew */
	ifnot (ispipe)
	{
	    if ( udata.u_offset.o_blkno > ino->c_node.i_size.o_blkno ||
	        (udata.u_offset.o_blkno == ino->c_node.i_size.o_blkno &&
	            udata.u_offset.o_offset > ino->c_node.i_size.o_offset))
	    {
	        ino->c_node.i_size.o_blkno = udata.u_offset.o_blkno;
	        ino->c_node.i_size.o_offset = udata.u_offset.o_offset;
	        ino->c_dirty = 1;
	    }
	}

	break;

    case F_CDEV:
	udata.u_count = cdwrite(ino->c_node.i_addr[0]);

	if (udata.u_count != -1)
	    addoff(&udata.u_offset, udata.u_count);
	break;

    default:
	udata.u_error = ENODEV;
    }

}

int min(int a, int b)
{
    return ( a < b ? a : b);
}

int psize(inoptr ino)
{
    return (512*ino->c_node.i_size.o_blkno+ino->c_node.i_size.o_offset);
}

void addoff(off_t *ofptr, int amount)
{
    if (amount >= 0)
    {
    ofptr->o_offset += amount % 512;
    if (ofptr->o_offset >= 512)
    {
	ofptr->o_offset -= 512;
	++ofptr->o_blkno;
    }
    ofptr->o_blkno += amount/512;
    }
    else
    {
	ofptr->o_offset -= (-amount) % 512;
	if (ofptr->o_offset < 0)
	{
	    ofptr->o_offset += 512;
	    --ofptr->o_blkno;
	}
	ofptr->o_blkno -= (-amount)/512;
    }
}

void updoff(int16 d)
{
    register off_t *offp;

    /* Update current file pointer */
    offp = &of_tab[udata.u_files[d]].o_ptr;
    offp->o_blkno = udata.u_offset.o_blkno;
    offp->o_offset = udata.u_offset.o_offset;
}

/****************************************
seek(file,offset,flag)
int16 file;
uint16 offset;
int16 flag;
*****************************************/

/*
#define file (int16)udata.u_argn2
#define offset (uint16)udata.u_argn1
#define flag (int16)udata.u_argn
*/

int _seek(int16 file, uint16 offset, int16 flag)
{
    register inoptr ino;
    register int16 oftno;
    register uint16 retval;

    if ((ino = getinode(file)) == NULLINODE)
	return(-1);

    if (getmode(ino) == F_PIPE)
    {
	udata.u_error = ESPIPE;
	return(-1);
    }

    oftno = udata.u_files[file];


    if (flag <= 2)
	retval = of_tab[oftno].o_ptr.o_offset;
    else
	retval = of_tab[oftno].o_ptr.o_blkno;

    switch(flag)
    {
    case 0:
	of_tab[oftno].o_ptr.o_blkno = 0;
	of_tab[oftno].o_ptr.o_offset = offset;
	break;
    case 1:
	of_tab[oftno].o_ptr.o_offset += offset;
	break;
    case 2:
	of_tab[oftno].o_ptr.o_blkno = ino->c_node.i_size.o_blkno;
	of_tab[oftno].o_ptr.o_offset = ino->c_node.i_size.o_offset + offset;
	break;
    case 3:
	of_tab[oftno].o_ptr.o_blkno = offset;
	break;
    case 4:
	of_tab[oftno].o_ptr.o_blkno += offset;
	break;
    case 5:
	of_tab[oftno].o_ptr.o_blkno = ino->c_node.i_size.o_blkno + offset;
	break;
    default:
	udata.u_error = EINVAL;
	return(-1);
    }

    while ((unsigned)of_tab[oftno].o_ptr.o_offset >= 512)
    {
	of_tab[oftno].o_ptr.o_offset -= 512;
	++of_tab[oftno].o_ptr.o_blkno;
    }

    return((int16)retval);
}

/*
#undef file
#undef offset
#undef flag
*/

/************************************
chdir(dir)
char *dir;
************************************/

/*
#define dir (char *)udata.u_argn
*/

int _chdir(char *dir)
{
    register inoptr newcwd;

    ifnot (newcwd = n_open(dir,NULLINOPTR))
	return(-1);

    if (getmode(newcwd) != F_DIR)
    {
	udata.u_error = ENOTDIR;
	i_deref(newcwd);
	return(-1);
    }
    i_deref(udata.u_cwd);
    udata.u_cwd = newcwd;
    return(0);
}

/*
#undef dir
*/

/*************************************
mknod(name,mode,dev)
char *name;
int16 mode;
int16 dev;
***************************************/

/*
#define name (char *)udata.u_argn2
#define mode (int16)udata.u_argn1
#define dev (int16)udata.u_argn
*/

int _mknod(char *name, int16 mode, int16 dev)
{
    register inoptr ino;
    inoptr parent;
    inoptr newfile();

    udata.u_error = 0;
    ifnot (super())
    {
	udata.u_error = EPERM;
	return(-1);
    }

    if (ino = n_open(name,&parent))
    {
	udata.u_error = EEXIST;
	goto nogood;
    }

    ifnot (parent)
    {
	udata.u_error = ENOENT;
	goto nogood3;
    }

    ifnot (ino = newfile(parent,name))
	goto nogood2;

    /* Initialize mode and dev */
    ino->c_node.i_mode = mode & ~udata.u_mask;
    ino->c_node.i_addr[0] = isdevice(ino) ? dev : 0;
    setftime(ino, A_TIME|M_TIME|C_TIME);
    wr_inode(ino);

    i_deref(ino);
    return (0);

nogood:
    i_deref(ino);
nogood2:
    i_deref(parent);
nogood3:
    return (-1);
}

/****************************************
sync()
***************************************/
void _sync(void)
{
    register j;
    register inoptr ino;
    register char *buf;

    /* Write out modified inodes */

    for (ino=i_tab; ino < i_tab+ITABSIZE; ++ino)
	if ((ino->c_refs) > 0 && ino->c_dirty != 0)
	{
	    wr_inode(ino);
	    ino->c_dirty = 0;
	}

    /* Write out modified super blocks */
    /* This fills the rest of the super block with garbage. */

    for (j=0; j < NDEVS; ++j)
    {
	if (fs_tab[j].s_mounted == SMOUNTED && fs_tab[j].s_fmod)
	{
	    fs_tab[j].s_fmod = 0;
	    buf = bread(j, 1, 1);
	    bcopy((char *)&fs_tab[j], buf, 512);
	    bfree(buf, 2);
	}
    }

    bufsync();   /* Clear buffer pool */
}

/****************************************
access(path,mode)
char *path;
int16 mode;
****************************************/

/*
#define path (char *)udata.u_argn1
#define mode (int16)udata.u_argn
*/

int _access(char *path, int16 mode)
{
    register inoptr ino;
    register int16 euid;
    register int16 egid;
    register int16 retval;

    if ((mode & 07) && !*(path))
    {
	udata.u_error = ENOENT;
	return (-1);
    }

    /* Temporarily make eff. id real id. */
    euid = udata.u_euid;
    egid = udata.u_egid;
    udata.u_euid = udata.u_ptab->p_uid;
    udata.u_egid = udata.u_gid;

    ifnot (ino = n_open(path,NULLINOPTR))
    {
	retval = -1;
	goto nogood;
    }

    retval = 0;
    if (~getperm(ino) & (mode&07))
    {
	udata.u_error = EPERM;
	retval = -1;
    }

    i_deref(ino);
nogood:
    udata.u_euid = euid;
    udata.u_egid = egid;

    return(retval);
}

/*
#undef path
#undef mode
*/

/*******************************************
chmod(path,mode)
char *path;
int16 mode;
*******************************************/

/*
#define path (char *)udata.u_argn1
#define mode (int16)udata.u_argn
*/

int _chmod(char *path, int16 mode)
{
    inoptr ino;

    ifnot (ino = n_open(path,NULLINOPTR))
	return (-1);

    if (ino->c_node.i_uid != udata.u_euid && !super())
    {
	i_deref(ino);
	udata.u_error = EPERM;
	return(-1);
    }

    ino->c_node.i_mode = (mode & MODE_MASK) | (ino->c_node.i_mode & F_MASK);
    setftime(ino, C_TIME);
    i_deref(ino);
    return(0);
}

/*
#undef path
#undef mode
*/

/***********************************************
chown(path, owner, group)
char *path;
int owner;
int group;
**********************************************/

/*
#define path (char *)udata.u_argn2
#define owner (int16)udata.u_argn1
#define group (int16)udata.u_argn
*/

int _chown(char *path, int owner, int group)
{
    register inoptr ino;

    ifnot (ino = n_open(path,NULLINOPTR))
	return (-1);

    if (ino->c_node.i_uid != udata.u_euid && !super())
    {
	i_deref(ino);
	udata.u_error = EPERM;
	return(-1);
    }

    ino->c_node.i_uid = owner;
    ino->c_node.i_gid = group;
    setftime(ino, C_TIME);
    i_deref(ino);
    return(0);
}

/*
#undef path
#undef owner
#undef group
*/

/**************************************
stat(path,buf)
char *path;
char *buf;
****************************************/

/*
#define path (char *)udata.u_argn1
#define buf (char *)udata.u_argn
*/

void stcpy(inoptr ino, char *buf);

_stat(char *path, char *buf)
{

    register inoptr ino;

    ifnot (valadr(buf,sizeof(struct stat)) && (ino = n_open(path,NULLINOPTR)))
    {
	return (-1);
    }

    stcpy(ino,buf);
    i_deref(ino);
    return(0);
}

/*
#undef path
#undef buf
*/

/* Utility for stat and fstat */
void stcpy(inoptr ino, char *buf)
{
    /* violently system-dependent */
    bcopy((char *)&(ino->c_dev), buf, 12);
    bcopy((char *)&(ino->c_node.i_addr[0]), buf+12, 2);
    bcopy((char *)&(ino->c_node.i_size), buf+14, 16);
}

#pragma code-name (pop)
