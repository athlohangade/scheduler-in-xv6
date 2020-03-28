#include "types.h"
#include "user.h"
#include "param.h"

int main(int argc, char *argv[]) {

    /* Local Variables */
    int pid;
    int tickets;
    int retval;

    /* Check if the input is correct */
    if (argc != 3) {
        printf(1, "Usage : %s 'pid' 'tickets'\n", argv[0]);
        exit();
    }

    /* Convert string to integer */
    pid = atoi(argv[1]);
    tickets = atoi(argv[2]);

    /* Check for correct input pid and tickets */
    if (pid < 1 || tickets < 1) {
        printf(1, "Check the parameters!\n");
        exit();
    }

    /* Check if the input tickets are less than maximum allowed tickets */
    if (tickets > MAX_TICKETS_PER_PROCESS) {
        printf(1, "Maximum tickets allowed for a process = %d\n", MAX_TICKETS_PER_PROCESS);
        exit();
    }

    /* Make the system call to assgin tickets to the process */
    retval = assign_tickets(pid, tickets);

    /* Check for any error occurred while assigning tickets */
    if (retval == -1) {
        printf(1, "Error encountered!\n");
    }
    else if (retval == -2) {
        printf(1, "Process not found!\n");
    }
    else {
        printf(1, "Tickets Updated!\n");
    }

    exit();
}
