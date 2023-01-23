#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include <string.h>
#include "process.h"
//struct lock filesys_lock;

void syscall_init (void);
int check_valid_addr(const void *addr);
void syscall_halt(void);
void syscall_exit(int status);
int syscall_exec(const char *cmd_line);
int syscall_wait(int child_tid);
int syscall_read(int file_d, void *buffer, unsigned size);
int syscall_write(int file_d, const void *buffer, unsigned size);
int syscall_fibonacci(int n);
int syscall_max_of_four_int(int n1, int n2, int n3, int n4);
int sys_create(char* file_name, unsigned size);
int sys_remove(char* file_name);
int sys_open(char* file_name);
void sys_close(int file_d);
unsigned sys_filesize(int file_d);
void sys_seek(int file_d, unsigned position);
unsigned sys_tell(int file_d);

#endif /* userprog/syscall.h */
