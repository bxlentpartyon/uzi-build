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

/* User's execve() call. All other flavors are library routines. */

/*****************************************
execve(name, argv, envp)
char *name;
char *argv[];
char *envp[];
*****************************************/

/*
#define name (char *)udata.u_argn2
#define argv (char **)udata.u_argn1
#define envp (char **)udata.u_argn
*/

int wargs(char **argv, int blk);
void exec2(void);

int _execve(char *name, char *argv[], char *envp[])
{
    register inoptr ino;
    register char *buf;

    ifnot (ino = n_open(name,NULLINOPTR))
	return(-1);

    if (ino->c_node.i_size.o_blkno >= ((uint16)(&udata)/512))
    {
	udata.u_error = ENOMEM;
	goto nogood;
    }

    ifnot ( (getperm(ino) & OTH_EX) &&
	    (ino->c_node.i_mode & F_REG) &&
	    (ino->c_node.i_mode & (OWN_EX | OTH_EX | GRP_EX)) )
    {
	udata.u_error = EACCES;
	goto nogood;
    }

    setftime(ino, A_TIME);

    /* Gather the arguments, and put them on the root device */
    /* Put environment on another block */
    if (wargs(argv, 0) || wargs(envp, 1))
	goto nogood;

    /* Read in the first block of the new program */
    buf = bread( ino->c_dev, bmap(ino, 0, 1), 0);

    if ((*buf & 0xff) != EMAGIC)
    {
	udata.u_error = ENOEXEC;
	goto nogood2;
    }

    /* Here, check the setuid stuff. No other changes need be made in
    the user data */
    if (ino->c_node.i_mode & SET_UID)
	udata.u_euid = ino->c_node.i_uid;

    if (ino->c_node.i_mode & SET_GID)
	udata.u_egid = ino->c_node.i_gid;

    bcopy(buf,PROGBASE,512);
    bfree(buf, 0);

    /* At this point, we are committed to reading in and executing
    the program. We switch to a local stack, and pass to it
    the necessary parameter: ino */

    udata.u_ino = ino;     /* Termorarily stash these here */

    tempstack();
    exec2();   /* Never returns */

nogood2:
    bfree(buf, 0);
nogood:
    i_deref(ino);
    return(-1);

}

char *rargs(char *ptr, int blk, int *cnt);

void exec2(void)
{
    register blkno_t blk;
    register char **argv;
    register char **envp;
    register int (**sp)();
    int argc;
    register char *progptr;
    char *buf;
    blkno_t pblk;

    /* Read in the rest of the program */
    progptr = PROGBASE+512;
    for (blk = 1; blk <= udata.u_ino->c_node.i_size.o_blkno; ++blk)
    {
	pblk = bmap(udata.u_ino, blk, 1);
	if (pblk != -1)
	{
	    buf = bread( udata.u_ino->c_dev, pblk, 0);
	    bcopy(buf, progptr, 512);
	    bfree(buf, 0);
	}
	progptr += 512;
    }
    i_deref(udata.u_ino);

    /* Zero out the free memory */
    bzero(progptr,(uint16)((char *)&udata - progptr));
    udata.u_break = progptr;


    /* Read back the arguments and the environment */
    argv = (char **)rargs((char *)&udata, 0, &argc);
    envp = (char **)rargs((char *)argv, 1, NULL);

    /* Fill in udata.u_name */
    bcopy(*argv,udata.u_name,8);

    /* Turn off caught signals */
    for (sp= udata.u_sigvec; sp < (udata.u_sigvec+NSIGS); ++sp)
	if (*sp != SIG_IGN)
	    *sp = SIG_DFL;

    /* Shove argc and the address of argv just below envp */
    *(envp - 1) = (char *)argc;
    *(envp - 2) = (char *)argv;

    /* Go jump into the program, first setting the stack */
    doexec((int16 *)(udata.u_isp = envp - 2));

}

/*
#undef name
#undef argv
#undef envp
*/

int wargs(char **argv, int blk)
{
    register char *ptr;    /* Address of base of arg strings in user space */
    register int n;
    struct s_argblk *argbuf;
    register char *bufp;
    register int j;

    /* Gather the arguments, and put them on the swap device */
    argbuf = (struct s_argblk *)bread(SWAPDEV, udata.u_ptab->p_swap+blk, 2);
    bufp = argbuf->a_buf;
    for (j=0; argv[j] != NULL; ++j)
    {
	ptr = argv[j];
	do
	{
	    *bufp++ = *ptr;
	    if (bufp >= argbuf->a_buf+500)
	    {
	        udata.u_error = E2BIG;
	        bfree((char *)argbuf, 1);
	        return (1);
	    }
	}
	while (*ptr++ != '\0');
    }

    argbuf->a_argc = j;  /* Store argc in argbuf. */
    argbuf->a_arglen = bufp - argbuf->a_buf;  /*Store total string size. */

    /* Swap out the arguments into the given swap block */
    bfree((char *)argbuf, 1);

    return (0);
}

char *rargs(char *ptr, int blk, int *cnt)
{
    struct s_argblk *argbuf;
    register char **argv;  /* Address of users argv[], just below ptr */
    register int n;

    /* Read back the arguments */
    argbuf = (struct s_argblk *)bread(SWAPDEV,udata.u_ptab->p_swap+blk, 0);

    /* Move them into the users address space, at the very top */
    ptr -= argbuf->a_arglen;
    if (argbuf->a_arglen)
	bcopy(argbuf->a_buf, ptr, argbuf->a_arglen);

    /* Set argv to point below the argument strings */
    argv = (char **)ptr - (argbuf->a_argc + 1);

    /* Set each element of argv[] to point to its argument string */
    argv[0] = ptr;
    for (n=1; n < argbuf->a_argc; ++n)
	argv[n] = argv[n-1] + strlen(argv[n-1]) + 1;
    argv[argbuf->a_argc] = NULL;

    if (cnt)
	*cnt = argbuf->a_argc;

    bfree((char *)argbuf, 0);
    return (argv);
}

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
