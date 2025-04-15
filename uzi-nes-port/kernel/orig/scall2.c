/**************************************************
UZI (Unix Z80 Implementation) Kernel:  scall2.c
***************************************************/

#include <string.h>

#include <unix.h>
#include <extern.h>

#include <devio.h>
#include <extras.h>
#include <filesys.h>
#include <machdep.h>
#include <process.h>
#include <scall.h>

/**************************************
wait(statloc)
int *statloc;
****************************************/

#define statloc (int *)udata.u_argn

_wait()
{
    register ptptr p;
    register int retval;

    if (statloc > (int *)(&udata))
    {
	udata.u_error = EFAULT;
	return(-1);
    }

    di();
    /* See if we have any children. */
    for (p=ptab;p < ptab+PTABSIZE; ++p)
    {
	if (p->p_status && p->p_pptr == udata.u_ptab && p != udata.u_ptab)
	    goto ok;
    }
    udata.u_error = ECHILD;
    ei();
    return (-1);

ok:
    /* Search for an exited child; */
    for (;;)
    {
	chksigs();
	if (udata.u_cursig)
	{
	    udata.u_error = EINTR;
	    return(-1);
	}
	di();
	for(p=ptab;p < ptab+PTABSIZE; ++p)
	{
	    if (p->p_status == P_ZOMBIE && p->p_pptr == udata.u_ptab)
	    {
	        if (statloc)
	            *statloc = p->p_exitval;
	        p->p_status = P_EMPTY;
	        retval = p->p_pid;

	        /* Add in child's time info */
	        /* It was stored on top of p_wait in the childs process
	        table entry */
	        addtick(&udata.u_cutime, &(p->p_wait));
	        addtick(&udata.u_cstime, (char *)(&(p->p_wait)) +
	                         sizeof(time_t));

	        ei();
	        return(retval);
	    }
	}
	/* Nothing yet, so wait */
	psleep(udata.u_ptab);
    }

}

#undef statloc



/**************************************
_exit(val)
int16 val;
**************************************/

#define val (int16)udata.u_argn

__exit()
{
    doexit(val,0);
}

#undef val

_fork()
{
   return (dofork());
}



_pause()
{
    psleep(0);
    udata.u_error = EINTR;
    return(-1);
}


/*************************************
signal(sig, func)
int16 sig;
int16 (*func)();
***************************************/

#define sig (int16)udata.u_argn1
#define func (int (*)())udata.u_argn

_signal()
{
    int retval;

    di();
    if (sig < 1 || sig == SIGKILL || sig >= NSIGS)
    {
	udata.u_error = EINVAL;
	goto nogood;
    }

    if (func == SIG_IGN)
	udata.u_ptab->p_ignored |= sigmask(sig);
    else
    {
	if (func != SIG_DFL && ((char *)func < PROGBASE ||
	       (struct u_data *)func >= &udata))
	{
	    udata.u_error = EFAULT;
	    goto nogood;
	}
	udata.u_ptab->p_ignored &= ~sigmask(sig);
    }
    retval = udata.u_sigvec[sig];
    udata.u_sigvec[sig] = func;
    ei();
    return(retval);

nogood:
    ei();
    return(-1);
}

#undef sig
#undef func



/**************************************
kill(pid, sig)
int16 pid;
int16 sig;
*****************************************/

#define pid (int16)udata.u_argn1
#define sig (int16)udata.u_argn

_kill()
{
    ptptr p;

    if (sig <= 0 || sig > 15)
	goto nogood;

    for (p=ptab; p < ptab+PTABSIZE; ++p)
    {
	if (p->p_pid == pid)
	{
	    sendsig(p,sig);
	    return(0);
	}
    }

nogood:
    udata.u_error = EINVAL;
    return(-1);
}

#undef pid
#undef sig



/********************************
alarm(secs)
uint16 secs;
*********************************/

#define secs (int16)udata.u_argn

_alarm()
{
    int retval;

    di();
    retval = udata.u_ptab->p_alarm;
    udata.u_ptab->p_alarm = secs;
    ei();
    return(retval);
}

#undef secs

