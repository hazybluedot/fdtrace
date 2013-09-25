#include <sys/ptrace.h>
#include <sys/reg.h>       /* For ORIG_RAX, ORIG_EAX */
#include <sys/syscall.h>   /* For SYS_write etc */
#include <sys/wait.h>

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "defs.h"

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

int f_debug = 0;
int f_verbose = 1;

static struct pib *head;

static struct pib*
alloc_pib(pid_t pid) {
    struct pib *p;
    p = malloc(sizeof(struct pib));
    memset(p, 0, sizeof(struct pib));
    p->pid = pid;
    return p;
}

static struct pib*
pid2pib_helper(pid_t pid, struct pib *start) {
    struct pib *cur = start;
    while(cur != NULL) {
        if (f_debug > 1)
            fprintf(stderr, "checking if %jd == %jd\n", (intmax_t)cur->pid, (intmax_t)pid);
        if (cur->pid == pid) 
            return cur;
        cur = cur->next;
    }
    
    return NULL;
}

static struct pib*
pid2pib(pid_t pid) {
    if (f_debug) {
        fprintf(stderr, "searching for %jd in pid list\n", (intmax_t)pid);
        fprintf(stderr, "head (%p) ->pid is %jd\n", head, (intmax_t)head->pid);
    }
    return pid2pib_helper(pid, head);
}

void usage() {
    fputs("Usage: fdtrace COMMAND\n", stderr);
}

void get_params(struct pib *pip) {
    pid_t pid = pip->pid;
    pip->params[0] = ptrace(PTRACE_PEEKUSER,
                           pid, REG_WIDTH * REG_BX, NULL);
    pip->params[1] = ptrace(PTRACE_PEEKUSER,
                            pid, REG_WIDTH * REG_CX, NULL);
    pip->params[2] = ptrace(PTRACE_PEEKUSER,
                           pid, REG_WIDTH * REG_DX, NULL);	
}

void print_syscall(struct pib *pip) {
    char name[255];
    int argc;

    switch(pip->syscall) { 
    case(SYS_open):
        strcpy(name, "open"); argc=3;
        break;
    case(SYS_close):
        strcpy(name, "close"); argc=1;
        break;
    case(SYS_dup):
        strcpy(name, "dup"); argc=1;
        break;
    case(SYS_dup2):
        strcpy(name, "dup2"); argc=2;
        break;
    case(SYS_fcntl):
        strcpy(name, "fnctl"); argc=-1;
        break;
    case(SYS_write):
        strcpy(name, "write"); argc=3;
        break;
    }
    switch(argc) {
    case(3):
        fprintf(stderr, "%s(%lld, %lld, %lld)\n", name, pip->params[0], pip->params[1], pip->params[2]); //TODO: arguments don't seem to be correct on 64bit arch
        break;
    }
}

void exit_syscall(struct pib *pip) {
}

void enter_syscall(struct pib *pip) {
    switch(pip->syscall) { 
    case(SYS_open):
    case(SYS_close):
    case(SYS_dup):
    case(SYS_dup2):
    case(SYS_fcntl):
        pip->flags |= PIB_FDCHANGE;
        break;
    case(SYS_read):
        break;
    case(SYS_write):
        if (pip->flags & PIB_FDCHANGE) {
            list_fd(pip->pid);
            pip->flags &= ~PIB_FDCHANGE;
        }
        get_params(pip);
        print_syscall(pip);
    }
}

int main(int argc, char *argv[]) {
    pid_t pid;
    int status;

    if (argc < 2) {
        usage();
        exit(1);
    }

    for(unsigned int i=3; i<9; i++)
        close(i); //close file descriptors higher than '2'. Mainly a hack to eliminate confusing descriptors open due to bash completion.

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
        if (f_debug)
            fprintf(stderr, "setting pid %jd as head\n", (intmax_t)pid);
        head = alloc_pib(pid);
        if (f_debug)
            fprintf(stderr, "head (%p) ->pid is %jd\n", head, (intmax_t)head->pid);
        while(1) {
            pid_t ppid = wait(&status);
      
            struct pib *pip = pid2pib(ppid);
            if (!pip) {
                //follow fork by default
                fprintf(stderr, "Untracked pid: %jd\n", (intmax_t)ppid);
            } else {

                if(WIFEXITED(status))
                    break;
                pip->syscall = ptrace(PTRACE_PEEKUSER,
                                      pid, REG_WIDTH * ORIG_REG, NULL);
                if (pip->flags & TCB_INSYSCALL) {
                    exit_syscall(pip);
                    pip->flags &= ~TCB_INSYSCALL;
                } else {
                    enter_syscall(pip);
                    pip->flags |= TCB_INSYSCALL;
                }

                ptrace(PTRACE_SYSCALL,
                       pid, NULL, NULL);
            }
        }
    }
}
