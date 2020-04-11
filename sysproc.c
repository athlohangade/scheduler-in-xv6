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
#ifdef SCHED_TEST
struct proc_sched_test {
    int pid;
    int ticks;
    int startflag;
    struct spinlock lock;
};
extern struct proc_sched_test proc_sched;
#endif

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

/* System call that gives process information */
int sys_proc_info(void) {

    struct proc *p;
    processes_info *information;

    // Get the process info pointer from user space in which information will be
    // stored
    if (argptr(0, (char **)(&information), sizeof(information)) < 0)
        return -1;

    information->num_processes = 0;
    // Acquire the process table lock
    acquire(&ptable.lock);

    // Get the process information and store it
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state == UNUSED)
            continue;

        information->pids[information->num_processes] = p->pid;
        information->ticks[information->num_processes] = p->ticks;
        information->tickets[information->num_processes] = p->tickets;
        information->num_processes++;
    }
    // Release the process table lock
    release(&ptable.lock);
    return 0;
}

int sys_assign_tickets(void) {

    struct proc *p;
    int pid;
    int tickets;

    // Get the data from user space
    if (argint(0, &pid) < 0)
        return -1;
    if (argint(1, &tickets) < 0)
        return -1;
    if (pid < 1 || tickets < 1)
        return -1;

    // Acquire the process table lock
    acquire(&ptable.lock);

    // Find and assign the tickets to the process
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->pid == pid) {
            p->tickets = tickets;
            release(&ptable.lock);
            return 0;
        }
    }
    // Release the process table lock
    release(&ptable.lock);

    return -2;
}

int sys_change_schedorder_flag(void) {

    int flag;

    // Get the argument
    if (argint(0, &flag) < 0)
        return -1;
    if (flag != 0 && flag != 1)
        return -1;

#ifdef SCHED_TEST
    // Acquire lock, change flag and release
    acquire(&proc_sched.lock);
    proc_sched.startflag = flag;
    release(&proc_sched.lock);
    return 0;
#else
    cprintf("Scheduler Testing is OFF\n");
    return -1;
#endif
}
