#include "types.h"
#include "user.h"
int main(int argc, char *argv[]) {

    int pid;
    int tickets;
    int retval;

    if (argc != 3) {
        printf(1, "Usage : assign-tickets 'pid' 'tickets'\n");
        exit();
    }

    pid = atoi(argv[1]);
    tickets = atoi(argv[2]);
    if (pid < 1 || tickets < 1) {
        printf(1, "Check the parameters!\n");
        exit();
    }

    retval = assign_tickets(pid, tickets);
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
