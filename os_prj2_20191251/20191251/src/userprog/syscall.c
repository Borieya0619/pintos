#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#include <stdlib.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/synch.h"

struct semaphore mutex, wrt;
int count_read;

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  sema_init(&mutex, 1); sema_init(&wrt, 1);
  count_read=0;
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&filesys_lock);
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{ //printf("f->esp: %d\n", *(uint32_t*)f->esp);
  //printf(check_valid_addr(f->esp+4) ? "true\n" : "false\n");
  
  switch(*(int*)(f->esp)){
	  case SYS_HALT: syscall_halt();break;
	  case SYS_EXIT: 
		  if(f->esp==NULL||!check_valid_addr(f->esp+4)||!pagedir_get_page(thread_current()->pagedir, (f->esp+4))) syscall_exit(-1); 
	          else {  //printf("\nf->esp+4는 %d\n", (int)*(uint64_t*)(f->esp+4));
			  syscall_exit((int)*(uint32_t**)(f->esp+4));}
	          break;
	  case SYS_EXEC:
		  if(f->esp==NULL||!check_valid_addr(f->esp+4)||!pagedir_get_page(thread_current()->pagedir, (f->esp+4))) syscall_exit(-1); 
                  else f->eax = syscall_exec((char*)*(uint32_t*)(f->esp+4));
                  break;
	  case SYS_WAIT:
		  if(f->esp==NULL||!check_valid_addr(f->esp+4)||!pagedir_get_page(thread_current()->pagedir, (f->esp+4))) syscall_exit(-1); 
		  else f->eax = syscall_wait((int)*(uint32_t*)(f->esp+4)); 
		  break;
	  case SYS_READ:
		  if(!check_valid_addr((const void*)*(uint32_t*)(f->esp+8))||!pagedir_get_page(thread_current()->pagedir, (f->esp+8))) syscall_exit(-1);
		  else{
			lock_acquire(&filesys_lock);
			if((int)*(uint32_t*)(f->esp+4)==0){
				int i;
				for(i=0; i<(unsigned)*(uint32_t*)(f->esp+12); i++){

					char inc=input_getc();
					if(inc=='\0') {
						f->eax = i;
						break;
					}
				}
				lock_release(&filesys_lock);
			break;
			}
			else if((int)*(uint32_t*)(f->esp+4)-3>=0){
				if(!thread_current()->fd[(int)*(uint32_t*)(f->esp+4)]){
					lock_release(&filesys_lock);
					syscall_exit(-1);
				}

				f->eax=file_read(thread_current()->fd[(int)*(uint32_t*)(f->esp+4)], (const void*)*(uint32_t*)(f->esp+8), (unsigned)*(uint32_t*)(f->esp+12));
				lock_release(&filesys_lock);
			break;
			}
			lock_release(&filesys_lock);
			f->eax =  -1;
			break;	
	          }
          case SYS_WRITE:
		if(!check_valid_addr((const void*)*(uint32_t*)(f->esp+8))||!pagedir_get_page(thread_current()->pagedir, (f->esp+8))) syscall_exit(-1);
		else{
			lock_acquire(&filesys_lock);
			if((int)*(uint32_t*)(f->esp+4)==1){
				char* buf=(char*)(const void*)*(uint32_t*)(f->esp+8);
				putbuf(buf,(size_t)(unsigned)*(uint32_t*)(f->esp+12));
				lock_release(&filesys_lock);
				f->eax=(unsigned)*(uint32_t*)(f->esp+12);
			break;
			}
			else if((int)*(uint32_t*)(f->esp+4)-3>=0){
			        if (!thread_current()->fd[(int)*(uint32_t*)(f->esp+4)]) syscall_exit(-1);
    				if (thread_current()->fd[(int)*(uint32_t*)(f->esp+4)]->deny_write)
        				file_deny_write(thread_current()->fd[(int)*(uint32_t*)(f->esp+4)]);
    					
				lock_release(&filesys_lock);
				f->eax = file_write(thread_current()->fd[(int)*(uint32_t*)(f->esp+4)], (const void*)*(uint32_t*)(f->esp+8), (unsigned)*(uint32_t*)(f->esp+12));
			break;
			}
			lock_release(&filesys_lock);
			f->eax = -1;
			break;
	        }
	  case SYS_FIBO: 
		  f->eax = syscall_fibonacci((int)*(uint32_t*)(f->esp+4)); break;
	  case SYS_MAX_FOUR:
		  f->eax = syscall_max_of_four_int((int)*(uint32_t*)(f->esp+4), (int)*(uint32_t*)(f->esp+8), (int)*(uint32_t*)(f->esp+12),(int)*(uint32_t*)(f->esp+16)); break;
	  case SYS_CREATE:
		  if(!check_valid_addr(f->esp+4)||!check_valid_addr(f->esp+8)||!pagedir_get_page(thread_current()->pagedir, (f->esp+4))||!pagedir_get_page(thread_current()->pagedir, (f->esp+8))) syscall_exit(-1);
		  else if((char*)*(uint32_t*)(f->esp+4)==NULL) syscall_exit(-1);
		  else f->eax = sys_create((char*)*(uint32_t*)(f->esp+4), (unsigned)*(uint32_t*)(f->esp+8));
		  break;
	  case SYS_REMOVE:
		  if(!check_valid_addr(f->esp+4)||!pagedir_get_page(thread_current()->pagedir, (f->esp+4))) syscall_exit(-1);
		  else if((char*)*(uint32_t*)(f->esp+4)==NULL) syscall_exit(-1);
                  else f->eax = sys_remove((char*)*(uint32_t*)(f->esp+4));
		  break;
	  case SYS_OPEN:
		  if(!check_valid_addr(f->esp+4)||!pagedir_get_page(thread_current()->pagedir, (f->esp+4))) syscall_exit(-1);
		  else if((char*)*(uint32_t*)(f->esp+4)==NULL) syscall_exit(-1);
                  else f->eax = (uint32_t)sys_open((char*)*(uint32_t*)(f->esp+4));
                  break;
	  case SYS_CLOSE:
		  if(!check_valid_addr(f->esp+4)||!pagedir_get_page(thread_current()->pagedir, (f->esp+4))) syscall_exit(-1);
		  else if(thread_current()->fd[(int)*(uint32_t*)(f->esp+4)]==NULL) syscall_exit(-1);
                  else  sys_close((int)*(uint32_t*)(f->esp+4));
                  break;
	  case SYS_FILESIZE:
		  if(!check_valid_addr(f->esp+4)||!pagedir_get_page(thread_current()->pagedir, (f->esp+4))) syscall_exit(-1);
		  else if(thread_current()->fd[(int)*(uint32_t*)(f->esp+4)]==NULL) syscall_exit(-1);
                  else f->eax = sys_filesize((int)*(uint32_t*)(f->esp+4));
                  break;
	  case SYS_SEEK:
		  if(!check_valid_addr(f->esp+4)||!check_valid_addr(f->esp+8)||!pagedir_get_page(thread_current()->pagedir, (f->esp+4))||!pagedir_get_page(thread_current()->pagedir, (f->esp+8))) syscall_exit(-1);
		  else if(thread_current()->fd[(int)*(uint32_t*)(f->esp+4)]==NULL) syscall_exit(-1);
                  else sys_seek((int)*(uint32_t*)(f->esp+4), (unsigned)*(uint32_t*)(f->esp+8));
                  break;
          case SYS_TELL: 
		  if(!check_valid_addr(f->esp+4)||!pagedir_get_page(thread_current()->pagedir, (f->esp+4))) syscall_exit(-1);
		  else if(thread_current()->fd[(int)*(uint32_t*)(f->esp+4)]==NULL) syscall_exit(-1);
                  else f->eax = sys_tell((int)*(uint32_t*)(f->esp+4));
                  break;
  
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
	int id=3;
	while(id<128){
		if(thread_current()->fd[id]!=NULL){
			file_close(thread_current()->fd[id]);
		        thread_current()->fd[id]=NULL;

		}
		id++;
	}
	
	struct thread* td;
	for(struct list_elem* e=list_begin(&(thread_current())->child_l); e!=list_end(&(thread_current()->child_l)); e=list_next(e)){
        td=list_entry(e, struct thread, child_e);
        process_wait(td->tid);
  	}

	file_close(thread_current()->current_file);

	thread_exit();
	
}

int syscall_exec(const char *cmd_line){
	return process_execute(cmd_line);
}

int syscall_wait(int child_tid){

	return process_wait(child_tid);

}

int syscall_read(int file_d, void *buffer, unsigned size){
	return -1;
}

int syscall_write(int file_d, const void *buffer, unsigned size){
	return -1;

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

int sys_create(char* file_name, unsigned size){
	return filesys_create(file_name, (off_t)size);
}

int sys_remove(char* file_name){
	return filesys_remove(file_name);
}

int sys_open(char* file_name){
	int rv = -1;
	struct thread* cur_thread = thread_current();
	lock_acquire(&filesys_lock);
	struct file* f_open = filesys_open(file_name);
	if(f_open!=NULL){for(int i=3; i<128; i++){
		if(!thread_current()->fd[i]){
			if(strcmp(thread_name(),file_name)==0)
				file_deny_write(f_open);
			thread_current()->fd[i] = f_open;
			rv = i;
			break;
	}}}
	lock_release(&filesys_lock);
	return rv;
}
	
void sys_close(int file_d){
	struct thread* cur_thread = thread_current();
	file_close(thread_current()->fd[file_d]);
	thread_current()->fd[file_d]=NULL;		
}

unsigned sys_filesize(int file_d){
	struct thread* cur_thread = thread_current(); 
	return file_length(thread_current()->fd[file_d]);
}

void sys_seek(int file_d, unsigned new_posi){
	struct thread* cur_thread = thread_current();
	file_seek(thread_current()->fd[file_d], new_posi);	
}

unsigned sys_tell(int file_d){
	struct thread* cur_thread = thread_current();
	return file_tell(thread_current()->fd[file_d]);
}
