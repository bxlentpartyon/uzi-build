/**************************************************
UZI (Unix Z80 Implementation) Kernel:  scall1.c
***************************************************/


/*LINTLIBRARY*/
#include <unix.h>
#include <extern.h>

#include <devio.h>
#include <extras.h>
#include <filesys.h>
#include <machdep.h>
#include <process.h>
#include <scall.h>

/**********************************************************
unlink(path)
char *path;
**************************************************/

#define path (char *)udata.u_argn

_unlink()
{
    register inoptr ino;
    inoptr pino;
    char *filename();
    inoptr i_open();
    inoptr n_open();

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

#undef path

/****************************************
seek(file,offset,flag)
int16 file;
uint16 offset;
int16 flag;
*****************************************/

#define file (int16)udata.u_argn2
#define offset (uint16)udata.u_argn1
#define flag (int16)udata.u_argn

_seek()
{
    register inoptr ino;
    register int16 oftno;
    register uint16 retval;
    inoptr getinode();

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

#undef file
#undef offset
#undef flag



/************************************
chdir(dir)
char *dir;
************************************/

#define dir (char *)udata.u_argn

_chdir()
{
    register inoptr newcwd;
    inoptr n_open();

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

#undef dir

/****************************************
access(path,mode)
char *path;
int16 mode;
****************************************/

#define path (char *)udata.u_argn1
#define mode (int16)udata.u_argn

_access()
{
    register inoptr ino;
    register int16 euid;
    register int16 egid;
    register int16 retval;
    inoptr n_open();

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

#undef path
#undef mode



/*******************************************
chmod(path,mode)
char *path;
int16 mode;
*******************************************/

#define path (char *)udata.u_argn1
#define mode (int16)udata.u_argn

_chmod()
{

    inoptr ino;
    inoptr n_open();

    ifnot (ino = n_open(path,NULLINOPTR))
	return (-1);

    if (ino->c_node.i_uid != udata.u_euid && !super())
    {
	i_deref(ino);
	udata.u_error = EPERM;
	return(-1);
    }

    ino->c_node.i_mode_lo = mode & MODE_MASK;
    ino->c_node.i_mode_hi = ino->c_node.i_mode_hi & F_MASK;
    setftime(ino, C_TIME);
    i_deref(ino);
    return(0);
}

#undef path
#undef mode



/***********************************************
chown(path, owner, group)
char *path;
int owner;
int group;
**********************************************/

#define path (char *)udata.u_argn2
#define owner (int16)udata.u_argn1
#define group (int16)udata.u_argn

_chown()
{
    register inoptr ino;
    inoptr n_open();

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

#undef path
#undef owner
#undef group

/********************************************
fstat(fd, buf)
int16 fd;
char *buf;
********************************************/

#define fd (int16)udata.u_argn1
#define buf (char *)udata.u_argn

_fstat()
{
    register inoptr ino;
    inoptr getinode();

    ifnot (valadr(buf,sizeof(struct stat)))
	return(-1);

    if ((ino = getinode(fd)) == NULLINODE)
	return(-1);

    stcpy(ino,buf);
    return(0);
}

#undef fd
#undef buf

/************************************
dup(oldd)
int16 oldd;
************************************/

#define oldd (uint16)udata.u_argn

_dup()
{
    register int newd;
    inoptr getinode();

    if (getinode(oldd) == NULLINODE)
	return(-1);

    if ((newd = uf_alloc()) == -1)
	return (-1);

    udata.u_files[newd] = udata.u_files[oldd];
    ++of_tab[udata.u_files[oldd]].o_refs;

    return(newd);
}

#undef oldd



/****************************************
dup2(oldd, newd)
int16 oldd;
int16 newd;
****************************************/

#define oldd (int16)udata.u_argn1
#define newd (int16)udata.u_argn

_dup2()
{
    inoptr getinode();

    if (getinode(oldd) == NULLINODE)
	return(-1);

    if (newd < 0 || newd >= UFTSIZE)
    {
	udata.u_error = EBADF;
	return (-1);
    }

    ifnot (udata.u_files[newd] & 0x80)
	doclose(newd);

    udata.u_files[newd] = udata.u_files[oldd];
    ++of_tab[udata.u_files[oldd]].o_refs;

    return(0);
}

#undef oldd
#undef newd



/**************************************
umask(mask)
int mask;
*************************************/

#define mask (int16)udata.u_argn

_umask()
{
    register int omask;

    omask = udata.u_mask;
    udata.u_mask = mask & 0777;
    return(omask);
}

#undef mask



/* Special system call returns super-block of given
filesystem for users to determine free space, etc.
Should be replaced with a sync() followed by a read
of block 1 of the device.  */

/***********************************************
getfsys(dev,buf)
int16 dev;
struct filesys *buf;
**************************************************/

#define dev (int16)udata.u_argn1
#define buf (struct filesys *)udata.u_argn

_getfsys()
{
   if (dev < 0 || dev >= NDEVS || fs_tab[dev].s_mounted != SMOUNTED)
   {
       udata.u_error = ENXIO;
       return(-1);
    }

    bcopy((char *)&fs_tab[dev],(char *)buf,sizeof(struct filesys));
    return(0);
}

#undef dev
#undef buf



/****************************************
ioctl(fd, request, data)
int fd;
int request;
char *data;
*******************************************/

#define fd (int)udata.u_argn2
#define request (int)udata.u_argn1
#define data (char *)udata.u_argn

_ioctl()
{

    register inoptr ino;
    register int dev;
    inoptr getinode();

    if ((ino = getinode(fd)) == NULLINODE)
	return(-1);

    ifnot (isdevice(ino))
    {
	udata.u_error = ENOTTY;
	return(-1);
    }

    ifnot (getperm(ino) & OTH_WR)
    {
	udata.u_error = EPERM;
	return(-1);
    }

    dev = ino->c_node.i_addr[0];

    if (d_ioctl(dev, request,data))
	return(-1);
    return(0);
}

#undef fd
#undef request
#undef data



/* This implementation of mount ignores the rwflag */

/*****************************************
mount(spec, dir, rwflag)
char *spec;
char *dir;
int rwflag;
*******************************************/

#define spec (char *)udata.u_argn2
#define dir (char *)udata.u_argn1
#define rwflag (int)udata.u_argn

_mount()
{
    register inoptr sino, dino;
    register int dev;
    inoptr n_open();

    ifnot(super())
    {
	udata.u_error = EPERM;
	return (-1);
    }

    ifnot (sino = n_open(spec,NULLINOPTR))
	return (-1);

    ifnot (dino = n_open(dir,NULLINOPTR))
    {
	i_deref(sino);
	return (-1);
    }

    if (getmode(sino) != F_BDEV)
    {
	udata.u_error = ENOTBLK;
	goto nogood;
    }

    if (getmode(dino) != F_DIR)
    {
	udata.u_error = ENOTDIR;
	goto nogood;
    }

    dev = (int)sino->c_node.i_addr[0];

    if ( dev >= NDEVS || d_open(dev))
    {
	udata.u_error = ENXIO;
	goto nogood;
    }

    if (fs_tab[dev].s_mounted || dino->c_refs != 1 || dino->c_num == ROOTINODE)
    {
       udata.u_error = EBUSY;
       goto nogood;
    }

    _sync();

    if (fmount(dev,dino))
    {
       udata.u_error = EBUSY;
       goto nogood;
    }

    i_deref(dino);
    i_deref(sino);
    return(0);

nogood:
    i_deref(dino);
    i_deref(sino);
    return (-1);
}

#undef spec
#undef dir
#undef rwflag



/******************************************
umount(spec)
char *spec;
******************************************/

#define spec (char *)udata.u_argn

_umount()
{
    register inoptr sino;
    register int dev;
    register inoptr ptr;
    inoptr n_open();

    ifnot(super())
    {
	udata.u_error = EPERM;
	return (-1);
    }

    ifnot (sino = n_open(spec,NULLINOPTR))
	return (-1);

    if (getmode(sino) != F_BDEV)
    {
	udata.u_error = ENOTBLK;
	goto nogood;
    }

    dev = (int)sino->c_node.i_addr[0];
    ifnot (validdev(dev))
    {
	udata.u_error = ENXIO;
	goto nogood;
    }

    if (!fs_tab[dev].s_mounted)
    {
	udata.u_error = EINVAL;
	goto nogood;
    }

    for (ptr = i_tab; ptr < i_tab+ITABSIZE; ++ptr)
	if (ptr->c_refs > 0 && ptr->c_dev == dev)
	{
	    udata.u_error = EBUSY;
	    goto nogood;
	}

    _sync();
    fs_tab[dev].s_mounted = 0;
    i_deref(fs_tab[dev].s_mntpt);

    i_deref(sino);
    return(0);

nogood:
    i_deref(sino);
    return (-1);
}

#undef spec

