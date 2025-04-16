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

