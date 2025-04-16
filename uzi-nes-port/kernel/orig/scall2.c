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
