#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  uint64 buf_addr, abit_addr;
  int size;
  struct proc *p = myproc();
  argaddr(0, &buf_addr);
  argint(1, &size);
  argaddr(2, &abit_addr);
  uint64 start, end;
  unsigned int abits = 0;
  pte_t *pte;


  if(size == 0)
    panic("pgaccess: size can not be zero");

  start = PGROUNDDOWN(buf_addr);
  end = PGROUNDDOWN(buf_addr + size * PGSIZE - 1);
  end = end > MAXVA ? MAXVA : end;

  for (unsigned i = 0; i < 32 && start <= end; i++, start += PGSIZE) {
    if((pte = walk(p->pagetable, start, 0)) == 0)
      continue;
    if(*pte & PTE_A) {
      abits |= (1 << i);
      *pte &= ~PTE_A;
    }
  }

  copyout(p->pagetable, abit_addr, (char *)&abits, sizeof(abits));
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
