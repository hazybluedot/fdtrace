#include <sys/ptrace.h>
#include <sys/reg.h>       /* For ORIG_RAX, ORIG_EAX */
#include <sys/syscall.h>   /* For SYS_write etc */
#include <sys/wait.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

#ifdef __x86_64__
typedef unsigned long long int syscall_type;
#define ORIG_REG ORIG_RAX
#define REG_WIDTH 8
#define REG_BX RBX
#define REG_CX RCX
#define REG_DX RDX
#else
typedef	long int syscall_type;
#define ORIG_REG ORIG_EAX
#define REG_WIDTH 4
#define REG_BX EBX
#define REG_CX ECX
#define REG_DX EDX
#endif

void usage() {
  fputs("Usage: fdtrace COMMAND\n", stderr);
}

int main(int argc, char *argv[]) {
  pid_t pid;
  long syscall;
  int status;
  int insyscall = 0;
  int fdchanged = 0;
  long long params[3];

  if (argc < 2) {
    usage();
    exit(1);
  }

  if ((pid = fork()) < 0) {
    perror("fork");
  } else if (pid == 0) {
    /*child */
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    if (-1 == execvp(argv[1], &argv[1]) ) {
      perror("execv");
      exit(1);
    }
  } else {
    while(1) {
      wait(&status);
      if(WIFEXITED(status))
	break;
      syscall = ptrace(PTRACE_PEEKUSER,
		       pid, REG_WIDTH * ORIG_REG, NULL);
      switch(syscall) { 
      case(SYS_open):
      case(SYS_close):
      case(SYS_dup):
      case(SYS_dup2):
	fdchanged = 1;
	break;
      case(SYS_write):
	if (insyscall == 0) {
	  // enterint system call
	  insyscall = 1;
	  if (fdchanged) {
	    list_fd(pid);
	    fdchanged = 0;
	  }
	  params[0] = ptrace(PTRACE_PEEKUSER,
			     pid, REG_WIDTH * REG_BX, NULL);
	  params[1] = ptrace(PTRACE_PEEKUSER,
			     pid, REG_WIDTH * REG_CX, NULL);
	  params[2] = ptrace(PTRACE_PEEKUSER,
			     pid, REG_WIDTH * REG_DX, NULL);	
	  fprintf(stderr, "write(%lld, %lld, %lld)\n", params[0], params[1], params[2]); //TODO: arguments don't seem to be correct on 64bit arch
	} else {
	  insyscall = 0;
	}
	break;
      }
      ptrace(PTRACE_SYSCALL,
	     pid, NULL, NULL);
    }
  }
}
