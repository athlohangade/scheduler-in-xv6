#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct process_table {                                                          
    struct spinlock lock;                                                         
    struct proc proc[NPROC];                                                      
};
extern struct process_table ptable;

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int sys_proc_stat(void) {
    
    struct proc *p;

    cprintf("NAME\t\tPID\t\tSIZE\t\tTICKETS\t\tSTATE\n");

    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (!p->pid)
            break;
        cprintf("%s\t\t%d\t\t%d\t\t%d\t\t", p->name, p->pid, p->sz, p->tickets);
        switch (p->state) {
            case UNUSED : cprintf("UNUSED\n");
                          break;
            case EMBRYO : cprintf("EMBRYO\n");
                          break;
            case SLEEPING : cprintf("SLEEPING\n");
                          break;
            case RUNNABLE : cprintf("RUNNABLE\n");
                          break;
            case RUNNING : cprintf("RUNNING\n");
                          break;
            case ZOMBIE : cprintf("ZOMBIE\n");
                          break;
            default     : cprintf("NO STATE\n");
        }
    }
    release(&ptable.lock);
    return 0;
}

int sys_assign_tickets(void) {

    struct proc *p;
    int pid;
    int tickets;

    if(argint(0, &pid) < 0)
        return -1;
    if(argint(1, &tickets) < 0)
        return -1;

    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->pid == pid) {
            p->tickets = tickets;
            release(&ptable.lock);
            return 0;
        }
    }
    release(&ptable.lock);

    return -2;
}
