#ifndef _DEFS_H_
#define _DEFS_H_

struct pib {
    int flags;
    struct pib *next;
    pid_t pid;
    long syscall;         /* system call number */
    long long params[3];  /* system call arguments */
    long rval;            /* return value */
    struct pib *children;
};

/* TCB flags: grabbed from strace source */ 
#define TCB_INUSE		00001	/* This table entry is in use */
/* We have attached to this process, but did not see it stopping yet */
#define TCB_STARTUP		00002
#define TCB_IGNORE_ONE_SIGSTOP	00004	/* Next SIGSTOP is to be ignored */
/*
 * Are we in system call entry or in syscall exit?
 *
 * This bit is set after all syscall entry processing is done.
 * Therefore, this bit will be set when next ptrace stop occurs,
 * which should be syscall exit stop. Other stops which are possible
 * directly after syscall entry (death, ptrace event stop)
 * are simpler and handled without calling trace_syscall(), therefore
 * the places where TCB_INSYSCALL can be set but we aren't in syscall stop
 * are limited to trace(), this condition is never observed in trace_syscall()
 * and below.
 * The bit is cleared after all syscall exit processing is done.
 * User-generated SIGTRAPs and post-execve SIGTRAP make it necessary
 * to be very careful and NOT set TCB_INSYSCALL bit when they are encountered.
 * TCB_WAITEXECVE bit is used for this purpose (see below).
 *
 * Use entering(tcp) / exiting(tcp) to check this bit to make code more readable.
 */
#define TCB_INSYSCALL	00010
#define TCB_ATTACHED	00020   /* It is attached already */
/* Are we PROG from "strace PROG [ARGS]" invocation? */
#define TCB_STRACE_CHILD 0040
#define TCB_BPTSET	00100	/* "Breakpoint" set after fork(2) */
#define TCB_REPRINT	00200	/* We should reprint this syscall on exit */
#define TCB_FILTERED	00400	/* This system call has been filtered out */

#define PIB_FDCHANGE     01000   /* Have file descriptors changed since last display? */
#endif
