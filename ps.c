#include "types.h"
#include "user.h"

int main(int argc, char *argv[]) {

    /* Local Variables */
    processes_info information;
    int i;

    /* Initialization */
    i = 0;
    information.num_processes = 0;

    /* Make the system call to get the processes information */
    proc_info(&information);

    /* Print the information */
    printf(1, "PID\t\tTICKS\t\tTICKETS\n");
    for (i = 0; i < information.num_processes; i++) {
        printf(1, "%d\t\t%d\t\t%d\n", information.pids[i], information.ticks[i], information.tickets[i]);
    }

    exit();
}
