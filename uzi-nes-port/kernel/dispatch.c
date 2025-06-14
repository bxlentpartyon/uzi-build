/**************************************************
UZI (Unix Z80 Implementation) Kernel:  dispatch.c
***************************************************/

#include <extern.h>
#include <machdep.h>
#include <scall.h>
#include <user_scall.h>

#pragma code-name (push, "DISPATCH_CODE")

#define HANDLE_SYSCALL0(num, name)			\
	case num:					\
		return name();				\
		break

#define HANDLE_SYSCALL1(num, name, typeN)		\
	case num:					\
		return name((typeN) udata.u_argn);	\
		break

#define HANDLE_SYSCALL2(num, name, type1, typeN)	\
	case num:					\
		return name((type1) udata.u_argn1,	\
			    (typeN) udata.u_argn);	\
		break

#define HANDLE_SYSCALL3(num, name, type1, type2, typeN)	\
	case num:					\
		return name((type2) udata.u_argn2,	\
			    (type1) udata.u_argn1,	\
			    (typeN) udata.u_argn);	\
		break

int __dispatch_call(void)
{
	kprintf("***dispatch call\n");
	switch (udata.u_callno) {
		HANDLE_SYSCALL1(0, __exit, int16);
		HANDLE_SYSCALL2(1, _open, char *, int16);
		HANDLE_SYSCALL1(2, _close, int);
		HANDLE_SYSCALL2(3, _creat, char *, int16);
		HANDLE_SYSCALL3(4, _mknod, char *, int16, int16);
		HANDLE_SYSCALL2(5, _link, char *, char *);
		HANDLE_SYSCALL1(6, _unlink, char *);
		HANDLE_SYSCALL3(7, _read, int16, char *, unsigned);
		HANDLE_SYSCALL3(8, _write, int16, char *, uint16);
		HANDLE_SYSCALL3(9, _seek, int16, uint16, int16);
		HANDLE_SYSCALL1(10, _chdir, char *);
		HANDLE_SYSCALL0(11, _sync);
		HANDLE_SYSCALL2(12, _access, char *, int16);
		HANDLE_SYSCALL2(13, _chmod, char *, int16);
		HANDLE_SYSCALL3(14, _chown, char *, int, int);
		HANDLE_SYSCALL2(15, _stat, char *, char *);
		HANDLE_SYSCALL2(16, _fstat, int16, char *);
		HANDLE_SYSCALL1(17, _dup, int16);
		HANDLE_SYSCALL0(18, _getpid);
		HANDLE_SYSCALL0(19, _getppid);
		HANDLE_SYSCALL0(20, _getuid);
		HANDLE_SYSCALL1(21, _umask, int);
		HANDLE_SYSCALL2(22, _getfsys, int16, struct filesys *);
		HANDLE_SYSCALL3(23, _execve, char *, char **, char **);
		HANDLE_SYSCALL1(24, _wait, int *);
		HANDLE_SYSCALL1(25, _setuid, int);
		HANDLE_SYSCALL1(26, _setgid, int);
		HANDLE_SYSCALL1(27, _time, int *);
		HANDLE_SYSCALL1(28, _stime, int *);
		HANDLE_SYSCALL3(29, _ioctl, int, int, char *);
		HANDLE_SYSCALL1(30, _brk, char *);
		HANDLE_SYSCALL1(31, _sbrk, uint16);
		HANDLE_SYSCALL0(32, _fork);
		HANDLE_SYSCALL3(33, _mount, char *, char *, int);
		HANDLE_SYSCALL1(34, _umount, char *);
		HANDLE_SYSCALL2(35, _signal, int16, int (*)());
		HANDLE_SYSCALL2(36, _dup2, int16, int16);
		HANDLE_SYSCALL0(37, _pause);
		HANDLE_SYSCALL1(38, _alarm, int16);
		HANDLE_SYSCALL2(39, _kill, int16, int16);
		HANDLE_SYSCALL1(40, _pipe, int *);
		HANDLE_SYSCALL0(41, _getgid);
		HANDLE_SYSCALL1(42, _times, char *);
		default:
			panic("unkown syscall\n");
			break;
	}
}

#pragma code-name (pop)
