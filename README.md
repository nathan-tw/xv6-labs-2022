## #1 System call tracing 

### 1. Add $U/_trace to UPROGS in Makefile

```
UPROGS=\
    $U/_trace\
    ...
```

### 2. Add system call number in kernel/syscall.h and extern system call in kernel/syscall.c


```c
//kernel/syscall.h
...
#define SYS_mkdir  20
#define SYS_close  21
#define SYS_trace  22
```

```c
// kernel/syscall.c
...
extern uint64 sys_write(void);
extern uint64 sys_uptime(void);
extern uint64 sys_trace(void);
```

### 3. Add a sys_trace() function in kernel/sysproc.c.

According to xv6 book:
> The user code places the arguments for exec in registers a0 and a1, and puts the system call number in a7.

Inorder to retreive system call argument, `argint` function defined in kernel/syscall.c can be used in form of `argint(0, &trace_mask)`, i.e., fetch p->trapframe->a0.

```c
// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  *ip = argraw(n);
  return 0;
}
```

`sys_trace` will remember its argument in a new variable in the proc structure (see kernel/proc.h), so add a new member `trace_mask` to `struct proc`.

```c
struct proc {
  ...
  int trace_mask;               // mask for trace syscall
};

```

Then add `sys_trace` in kernel/sysproc.c
```c
uint64
sys_trace(void)
{
  int trace_mask;

  if (argint(0, &trace_mask) < 0) // trace_mask = p->trapframe->a0 (syscall argument)
    return -1;
  struct proc *p = myproc();
  p->trace_mask = trace_mask;

  return 0;
}
```

### 4. Modify fork() (see kernel/proc.c) to copy the trace mask from the parent to the child process.

Since `trace` will call `exec` to execute the following command, parent process should pass the mask to child process.

```c
int
fork(void)
{
    ...
    np->trace_mask = p->trace_mask;
    ...
}
```

### 5. Modify the syscall() function in kernel/syscall.c to print the trace output.

```
$ trace 32 grep hello README
3: syscall read -> 1023
3: syscall read -> 966
3: syscall read -> 70
3: syscall read -> 0
```

To print the system call name, you will need to add an array of syscall names to index into.

```c
static char *sysnames[] = {
[SYS_fork]    "fork",
[SYS_exit]    "exit",
[SYS_wait]    "wait",
[SYS_pipe]    "pipe",
[SYS_read]    "read",
[SYS_kill]    "kill",
[SYS_exec]    "exec",
[SYS_fstat]   "fstat",
[SYS_chdir]   "chdir",
[SYS_dup]     "dup",
[SYS_getpid]  "getpid",
[SYS_sbrk]    "sbrk",
[SYS_sleep]   "sleep",
[SYS_uptime]  "uptime",
[SYS_open]    "open",
[SYS_write]   "write",
[SYS_mknod]   "mknod",
[SYS_unlink]  "unlink",
[SYS_link]    "link",
[SYS_mkdir]   "mkdir",
[SYS_close]   "close",
[SYS_trace] "trace",
[SYS_sysinfo] "sysinfo",
};
```

Print trace result when executing system call.

> When the system call implementation function returns, syscall records its return value in
p->trapframe->a0. This will cause the original user-space call to exec() to return that value,
since the C calling convention on RISC-V places return values in a0.

```c
void
syscall(void)
{
  int num;
  struct proc *p = myproc();

  // user code puts the system call number in a7.
  num = p->trapframe->a7;

  // check if 0 < num < len(syscalls) && syscalls[num] != 0
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    // put return number to a0
    p->trapframe->a0 = syscalls[num]();
    if ((1 << num) & p->trace_mask)
      printf("%d: syscall %s -> %d\n", p->pid, sysnames[num], p->trapframe->a0);
  } else {
    printf("%d %s: unknown sys call %d\n",
            p->pid, p->name, num);
            ...
  }
  ...
}
            
```