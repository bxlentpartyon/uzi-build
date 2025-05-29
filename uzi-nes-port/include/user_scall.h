#ifndef __USER_SCALL_H__
#define __USER_SCALL_H__

#pragma wrapped-call(push, trampoline, SCALL1_PAGE)
int _open(char *name, int16 flag);
int _close(int uindex);
int _creat(char *name, int16 mode);
int _mknod(char *name, int16 mode, int16 dev);
int _link(char *name1, char *name2);
int _unlink(char *path);
int _read(int16 d, char *buf, unsigned nbytes);
int _write(int16 d, char *buf, uint16 nbytes);
int _seek(int16 file, uint16 offset, int16 flag);
int _chdir(char *dir);
int _access(char *path, int16 mode);
int _chmod(char *path, int16 mode);
int _chown(char *path, int owner, int group);
int _stat(char *path, char *buf);
int _fstat(int16 fd, char *buf);
int _dup(int16 oldd);
int _umask(int mask);
int _getfsys(int16 dev, struct filesys *buf);
int _ioctl(int fd, int request, char *data);
int _mount(char *spec, char *dir, int rwflag);
int _umount(char *spec);
int _dup2(int16 oldd, int16 newd);
int _pipe(int fildes[]);
#pragma wrapped-call(pop)

#pragma wrapped-call(push, trampoline, SCALL2_PAGE)
int __exit(int16 val);
int _getpid(void);
int _getppid(void);
int _getuid(void);
int _wait(int *statloc);
int _setuid(int uid);
int _setgid(int gid);
int _time(int tvec[]);
int _stime(int tvec[]);
int _brk(char *addr);
int _sbrk(uint16 incr);
int _fork(void);
int _signal(int16 sig, int16 (*func)());
int _pause(void);
int _alarm(uint16 secs);
int _kill(int16 pid, int16 sig);
int _getgid(void);
int _times(char *buf);
#pragma wrapped-call(pop)

#endif /* __USER_SCALL_H__ */
