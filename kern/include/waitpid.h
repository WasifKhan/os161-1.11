#ifndef _WAITPID_H_
#define _WAITPID_H_



#include <types.h>
#include <lib.h>

pid_t sys_waitpid(pid_t pid, int* status, int options);

#endif
