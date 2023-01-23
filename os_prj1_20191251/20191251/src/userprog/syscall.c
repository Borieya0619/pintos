#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#include <stdlib.h>

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{ //printf("f->esp: %d\n", *(uint32_t*)f->esp);
  //printf(check_valid_addr(f->esp+4) ? "true\n" : "false\n");
  
  switch(*(int*)(f->esp)){
	  case SYS_HALT: syscall_halt();break;
	  case SYS_EXIT: 
		  if(!check_valid_addr(f->esp+4)) syscall_exit(-1); 
	          else {  //printf("\nf->esp+4는 %d\n", (int)*(uint64_t*)(f->esp+4));
			  syscall_exit((int)*(uint32_t**)(f->esp+4));}
	          break;
	  case SYS_EXEC: 
	  	  if(!check_valid_addr(f->esp+4)) syscall_exit(-1);
                  else f->eax = syscall_exec((char*)*(uint32_t*)(f->esp+4));
                  break;
	  case SYS_WAIT: 
	  	  if(!check_valid_addr(f->esp+4)) syscall_exit(-1);
		  else f->eax = syscall_wait((int)*(uint32_t*)(f->esp+4)); 
		  break;
	  case SYS_READ: 
	  	  //if(!check_valid_addr(f->esp+4)||!check_valid_addr(f->esp+8)||!check_valid_addr(f->esp+12)) {syscall_exit(-1);  }
                  //else {
			  f->eax = syscall_read((int)*(uint32_t*)(f->esp+4), (void*)*(uint32_t*)(f->esp+8), (unsigned)*(uint32_t*)(f->esp+12)); 
			  //printf("else f->eax : %d\n", f->eax);
			  //}
                  break;
	  case SYS_WRITE: 
	  	 // if(is_user_vaddr(f->esp+4)||is_user_vaddr(f->esp+8)||is_user_vaddr(f->esp+12)) {syscall_exit(-1);}
                 // else {
			  f->eax = syscall_write((int)*(uint32_t*)(f->esp+4), (const void*)*(uint32_t*)(f->esp+8), (unsigned)*(uint32_t*)(f->esp+12)); 
			  //printf("else f->eax : %d\n", f->eax);
			  //}
                  break;
	  case SYS_FIBO: 
		  f->eax = syscall_fibonacci((int)*(uint32_t*)(f->esp+4)); break;
	  case SYS_MAX_FOUR:
		  f->eax = syscall_max_of_four_int((int)*(uint32_t*)(f->esp+4), (int)*(uint32_t*)(f->esp+8), (int)*(uint32_t*)(f->esp+12),(int)*(uint32_t*)(f->esp+16)); break;
	  
  
  }
  //printf ("system call!\n");
}

int check_valid_addr(const void *addr){
	struct thread* cur_thread = thread_current();
	if(is_user_vaddr(addr)) return true;
	return false;

}

void syscall_halt(void){
	shutdown_power_off();
}

void syscall_exit(int status){
	struct thread* cur_thread = thread_current();	
	cur_thread->exit_status = status;
	printf("%s: exit(%d)\n", thread_name(), cur_thread->exit_status);
	thread_exit();

}

int syscall_exec(const char *cmd_line){
	return process_execute(cmd_line);
}

int syscall_wait(int child_tid){

	return process_wait(child_tid);

}

int syscall_read(int fd, void *buffer, unsigned size){
	char c;
	int i;
	if (fd==0){
		for(i=0; i<size; i++){
			c = (char)input_getc();
			if(c=='\0') break;
		}	
	}
	
	int rv = (fd==0) ? i:-1;

	return rv;
}

int syscall_write(int fd, const void *buffer, unsigned size){
	if(fd==1){
		char* buf = (char*) buffer;
		putbuf(buf, size);
	}

	int rv = (fd==1) ? size:-1;

	return rv;

}

int syscall_fibonacci(int n){
	int *fib = (int*)malloc(sizeof(int)*(n+1));
	//printf("%d", n);
	if(n==0 || n==1) return n;
	fib[0] = 0;
	fib[1] = 1;
	for(int i=2; i<=n+1; i++)
		fib[i] = fib[i-1]+fib[i-2];
	return fib[n];
}

int syscall_max_of_four_int(int n1, int n2, int n3, int n4){
	int max = n1;
	int nums[] = {n2, n3, n4};
	//printf("%d %d %d %d\n", n1, n2, n3, n4);
	for(int i=1; i<4; i++)
		if(max<nums[i])
			max = nums[i];
	return max;
}
