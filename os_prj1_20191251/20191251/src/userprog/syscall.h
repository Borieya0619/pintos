#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
int check_valid_addr(const void *addr);
void syscall_halt(void);
void syscall_exit(int status);
int syscall_exec(const char *cmd_line);
int syscall_wait(int child_tid);
int syscall_read(int fd, void *buffer, unsigned size);
int syscall_write(int fd, const void *buffer, unsigned size);
int syscall_fibonacci(int n);
int syscall_max_of_four_int(int n1, int n2, int n3, int n4);
#endif /* userprog/syscall.h */
