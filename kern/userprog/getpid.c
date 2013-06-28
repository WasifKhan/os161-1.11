#include <types.h>
#include <lib.h>
#include <thread.h>
#include <getpid.h>
#include <curthread.h>


pid_t sys_getpid()
{
   return (pid_t) curthread->pid;
}
