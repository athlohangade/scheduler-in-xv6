## Project

This project implements the lottery scheduler in xv6, a unix based operating system developed by MIT

---

## Lottery Scheduler

- A process is assigned some tickets when it is created. The scheduler generates
  a random number and based on that decides which process is to be scheduled.
- Most of the scheduler logic code resides in *proc.c*.
- Flow :
  1. The scheduler finds out the total tickets of the *RUNNABLE* processes.
  (The different states of the process can be found in *proc.h*.)
  2. If any *RUNNABLE* process is found, then scheduler generates a random
  number (*rand.c*). The random number generator used is **linear feedback shift
  register** random number generator. Information about this can be found [here](https://en.wikipedia.org/wiki/Linear-feedback_shift_register).
  3. The scheduler then finds the process to be scheduled by adding the tickets
  of the *RUNNABLE* processes and then selecting that process at which the total
  tickets count becomes greater than the random number.

---

## How to run ?

- The repo contains a folder *xv6* that contains the original xv6 code. The
  original xv6 code can also be found [here](https://github.com/mit-pdos/xv6-public.).
  Refer the *ADDED_MODIFIED.md* file to see the new files added and the modified
  ones.
- Run the script *copy_files.sh* to copy the new files in the *xv6* folder. This
  will replace some files of the original xv6 code in xv6 folder.
- To run xv6 OS, an emulator **qemu** is required. qemu is free and open source
  emulator that performs hardware virtualization. Install qemu using : `sudo apt-get install qemu`
- Open the *xv6* folder and type `make qemu-nox` to run qemu on existing
  terminal. Or type `make qemu` to run qemu on new window.
- To exit qemu, type `Ctrl-A` followed by letter `C` and then type `quit`.

---

## Code files

1. *assign-tickets* : This is an user program that allows the user to change the
   tickets assigned to the process. This program makes a system call to enter
   kernel to assign the tickets.
2. *defs.h* : Contains the definitions of kernel functions.
3. *param.h* : Contains kernel level macros.
4. *proc.c* : Contains all code related to the process creation, termination,
   scheduling, etc.
5. *proc.h* : Contains the cpu structure, proc structure, proc states.
6. *ps.c* : User program that allows the user to see process info : pid, tickets
   and ticks. Makes a system call to retrieve the information.
7. *rand.c* : Contains code for random number generation.
8. *scheduler-test.c* : Contains code for testing the scheduler. Refer below to
   see the testing procedure.
9. *sleep.c* : User program to sleep for some timeticks. Timeticks increments at
   each timer interrupt.
10. *syscall.c, syscall.h* : Contains various system call entries and function
    to be called.
11. *sysproc* : Contains the implementation of system calls. (like
    assign-tickets...).
12. *types.h* : Contains the data types and process info structure.
13. *user.h* : Contains the all callable function prototypes and system call
    that can be made by user.
14. *usys.S* : Assembly code that loads register with syscall number makes
    syscall and make transition to the kernel mode.

---

## Testing procedure

- Refer the file *scheduler-test.c* to see how the lottery scheduler testing is
  done.
- The test program forks a number of child process then let the parent
  process sleep for some ticks, retrieve the process info of all children and
  then kill all of them.
- The test program takes input :
    1. The timeticks for which parent process must sleep. (Generally **1 tick = 10ms**,
   depends on cpu bus freq. Eg: *300 ticks* corresponds to *3 sec* sleep).
    2. The tickets to be assigned for the parent process.
    3. The tickets to be assigned for the child processes. Depending on number
       of child process tickets given as input, the test program decides the
       number of children to be forked.
- Then the program forks required children and assign their tickets.
- Then a system call is made to get info about child processes.
- Then the program goes to sleep for some timeticks entered by user and allow
  the child processes to get scheduled.
- Then again system call is made to get info about child processes.
- Printing the difference of ticks before and after parent going to sleep let us
  know for how ticks individual child process was scheduled.
- **Result** : Mostly, the results were that the process which was assigned
  relatively more tickets had more ticks (more scheduled) than the lesser ones.
  Thus the scheduler algorithm works correctly.

---

## Supporting info

- The ticks variable tells us how many times the process has been scheduled. The
  global 'ticks' variable gets incremented at every timer interrupt.
- The timer interrupt is approximately equal to 10ms but depends on the cpu bus
  frequency. Thus timer interrupt duration can be changed by adjusting the
  countdown value in one of the register of the APIC hardware. (Refer *lapic.c*
  file).
- The number of CPU to be used for xv6 can be configured by changing the 'CPU'
  variable in the *Makefile*.
