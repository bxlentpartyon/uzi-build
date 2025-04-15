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

_fork()
{
   return (dofork());
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

