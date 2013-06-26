#ifndef _SYSCALL_H_
#define _SYSCALL_H_

/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_reboot(int code);

#if OPT_A2
void sys__exit(int exitcode);
int sys_write(int fd, const void* buf, size_t nbytes>);

int sys_open(const char* filename, int flags);
int sys_close(int fd);
int sys_read(int fd, void* buf, size_t buflen);

pid_t sys_fork(void);
pid_t sys_getpid(void);
pid_t sys_waitpid(pid_t pid, int* status, int options);

int sys_execv(const char* program, char** args);
#endif

#endif /* _SYSCALL_H_ */

