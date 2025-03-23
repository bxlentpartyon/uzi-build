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

