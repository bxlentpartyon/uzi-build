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

#pragma code-name (pop)
