#ifndef _EXECV_H_
#define _EXECV_H_



#include <types.h>
#include <lib.h>

int sys_execv(const char* program, char** args);

#endif
