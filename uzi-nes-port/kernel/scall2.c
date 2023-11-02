#include <extern.h>
#include <filesys.h>
#include <lib/string.h>
#include <machdep.h>
#include <ppu.h>
#include <process.h>
#include <scall.h>
#include <unix.h>

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
