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
#else
typedef	long int syscall_type;
#define ORIG_REG ORIG_EAX
#define REG_WIDTH 4
#endif

int main(int argc, char *argv[]) {
  pid_t pid;
  long syscall;
  int status;
  int insyscall = 0;
  //char *execv[argc+2];

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
    fprintf(stderr, "Tracing file descriptors used by %s\n", argv[1]);
    while(1) {
      wait(&status);
      if(WIFEXITED(status))
	break;
      syscall = ptrace(PTRACE_PEEKUSER,
			pid, REG_WIDTH * ORIG_REG, NULL);
      if(syscall == SYS_write) {
	if (insyscall == 0) {
	  // enterint system call
	  insyscall = 1;
	  list_fd(pid);
	} else {
	  insyscall = 0;
	}
      }
      ptrace(PTRACE_SYSCALL,
	     pid, NULL, NULL);
    }
  }
}
