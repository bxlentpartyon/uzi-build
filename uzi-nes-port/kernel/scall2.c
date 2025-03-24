#include <devio.h>
#include <extern.h>
#include <filesys.h>
#include <lib/string.h>
#include <machdep.h>
#include <ppu.h>
#include <process.h>
#include <scall.h>
#include <time.h>
#include <unix.h>

#pragma code-name (push, "SCALL2_CODE")

/* Getpid() */
_getpid()
{
    return(udata.u_ptab->p_pid);
}

/* Getppid() */
_getppid()
{
    return(udata.u_ptab->p_pptr->p_pid);
}

/* Getuid() */
_getuid()
{
    return(udata.u_ptab->p_uid);
}

_getgid()
{
    return(udata.u_gid);
}

/*********************************
setuid(uid)
***********************************/

/*
#define uid (int)udata.u_argn
*/

int _setuid(int uid)
{
    if (super() || udata.u_ptab->p_uid == uid)
    {
	udata.u_ptab->p_uid = uid;
	udata.u_euid = uid;
	return(0);
    }
    udata.u_error = EPERM;
    return(-1);
}

/*
#undef uid
*/

/*****************************************
setgid(gid)
****************************************/

/*
#define gid (int16)udata.u_argn
*/

int _setgid(int gid)
{
    if (super() || udata.u_gid == gid)
    {
	udata.u_gid = gid;
	udata.u_egid = gid;
	return(0);
    }
    udata.u_error = EPERM;
    return(-1);
}

/*
#undef gid;
*/

/***********************************
time(tvec)
int tvec[];
**************************************/

/*
#define tvec (int *)udata.u_argn
*/

int _time(int tvec[])
{
    rdtime(tvec);  /* In machdep.c */
    return(0);
}

#undef tvec


/**************************************
stime(tvec)
int tvec[];
**********************************/

/*
#define tvec (int *)udata.u_argn
*/

int _stime(int tvec[])
{
/*
    ifnot (super())
    {
	udata.u_error = EPERM;
	return(-1);
    }
    sttime(tvec);
    return(0);
*/

    udata.u_error = EPERM;
    return(-1);
}

/*
#undef tvec
*/

/********************************************
times(buf)
char *buf;
**********************************************/

/*
#define buf (char *)udata.u_argn
*/

int _times(char *buf)
{
    ifnot (valadr(buf,6*sizeof(time_t)))
	return(-1);

    di();
    bcopy(&udata.u_utime, buf, 4*sizeof(time_t));
    bcopy(&ticks, buf + 4*sizeof(time_t), sizeof(time_t));
    ei();
    return(0);
}

/*
#undef buf
*/

void doexit(int16 val, int16 val2)
{
    register int16 j;
    register ptptr p;

    for (j=0; j < UFTSIZE; ++j)
    {
	ifnot (udata.u_files[j] & 0x80)  /* Portable equivalent of == -1 */
	    doclose(j);
    }

/*    _sync();  /* Not necessary, but a good idea. */

    di();
    udata.u_ptab->p_exitval = (val<<8) | (val2 & 0xff);

    /* Set child's parents to init */
    for(p=ptab;p < ptab+PTABSIZE; ++p)
    {
	if (p->p_status && p->p_pptr == udata.u_ptab)
	    p->p_pptr = initproc;
    }
    i_deref(udata.u_cwd);

    /* Stash away child's execution tick counts in process table,
    overlaying some no longer necessary stuff. */
    addtick(&udata.u_utime,&udata.u_cutime);
    addtick(&udata.u_stime,&udata.u_cstime);
    bcopy(&udata.u_utime, &(udata.u_ptab->p_wait), 2 * sizeof(time_t));

    /* Wake up a waiting parent, if any. */
    if (udata.u_ptab != initproc)
	wakeup((char *)udata.u_ptab->p_pptr);
    udata.u_ptab->p_status = P_ZOMBIE;
    ei();
    swapin(getproc());
    panic("doexit:won't exit");
}

/* Special system call returns super-block of given
filesystem for users to determine free space, etc.
Should be replaced with a sync() followed by a read
of block 1 of the device.  */

/***********************************************
getfsys(dev,buf)
int16 dev;
struct filesys *buf;
**************************************************/

/*
#define dev (int16)udata.u_argn1
#define buf (struct filesys *)udata.u_argn
*/

int _getfsys(int16 dev, struct filesys *buf)
{
   if (dev < 0 || dev >= NDEVS || fs_tab[dev].s_mounted != SMOUNTED)
   {
       udata.u_error = ENXIO;
       return(-1);
    }

    bcopy((char *)&fs_tab[dev],(char *)buf,sizeof(struct filesys));
    return(0);
}

/*
#undef dev
#undef buf
*/

/****************************************
ioctl(fd, request, data)
int fd;
int request;
char *data;
*******************************************/

/*
#define fd (int)udata.u_argn2
#define request (int)udata.u_argn1
#define data (char *)udata.u_argn
*/

int _ioctl(int fd, int request, char *data)
{

    register inoptr ino;
    register int dev;

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

/*
#undef fd
#undef request
#undef data
*/

/* This implementation of mount ignores the rwflag */

/*****************************************
mount(spec, dir, rwflag)
char *spec;
char *dir;
int rwflag;
*******************************************/

/*
#define spec (char *)udata.u_argn2
#define dir (char *)udata.u_argn1
#define rwflag (int)udata.u_argn
*/

int _mount(char *spec, char *dir, int rwflag)
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

/*
#undef spec
#undef dir
#undef rwflag
*/

/******************************************
umount(spec)
char *spec;
******************************************/

/*
#define spec (char *)udata.u_argn
*/

int _umount(char *spec)
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

/*
#undef spec
*/

#pragma code-name (pop)
