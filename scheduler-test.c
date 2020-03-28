#include "types.h"
#include "user.h"
#include "param.h"

#define MAX_CHILD_PROCESS   16
#define MAX_PARENT_YIELDS   100

/* Function to fork the child process and assign tickets */
int create_process(int tickets) {

    /* Fork the process */
    int pid = fork();

    /* If the process is child */
    if (pid == 0) {

        /* Assign the tickets to the child process */
        if (assign_tickets(getpid(), tickets) == 0) {
            /* Loop forever */
            while (1);
        }
        /* If assigning the tickets to the process fail */
        else {
            printf(1, "Error setting the tickets\n");
            exit();
        }
    }
    /* If the process is parent */
    else if (pid > 0) {
        return pid;
    }
    /* If fork fails */
    else {
        return -1;
    }
}

/* Function to find the pid in the processes information structure */
int find_index_of_pid(processes_info *information, int pid) {

    int i;
    for (i = 0; i < information->num_processes; i++) {
        if (information->pids[i] == pid) {
            return i;
        }
    }
    return -1;
}

/* Function that waits until the tickets of all the child processes are set */
int wait_for_tickets_to_set(int no_of_children, int *child_pids, int *tickets) {

    /* Local variables */
    int i, j, done, index;
    processes_info info;

    /* Loop until tickets of all child processes are set */
    for (i = 0; i < MAX_PARENT_YIELDS; i++) {

        /* Assume the tickets of all child processes are set */
        done = 1;
        /* System call to get the processes information */
        proc_info(&info);

        /* Loop to check if the tickets are set */
        for (j = 0; j < no_of_children; j++) {

            /* Find the index of child in processes information */
            index = find_index_of_pid(&info, child_pids[i]);

            /* If the child is found in the processes information */
            if (index != -1) {

                /* Check if the tickets are set */
                if (tickets[j] != info.tickets[index]) {
                    /* Set the flag if the tickets are not set */
                    done = 0;
                    break;
                }
            }
        }
        /* If tickets of all child processes is set */
        if (done) {
            return 0;
        }
    }
    /* If tickets of all child processes is not set */
    return -1;
}

/* Main function */
int main(int argc, char *argv[]) {

    /* Check if required inputs are provided by user */
    if (argc < 4) {
        printf(1, "Usage : %s 'time-ticks' parent_tickets child1_tickets child2_tickets ... \n" \
                  "time-ticks is the number of ticks the parent process must sleep for\n" \
                  "parent_tickets to be assigned to the parent process\n" \
                  "childN_tickets is the number of tickets to be assigned to the Nth child process\n", argv[0]);
        exit();
    }
    /* Check if child process more than a particulare limit are to be forked */
    if (argc > MAX_CHILD_PROCESS + 3) {
        printf(1, "Only %d child process supported", MAX_CHILD_PROCESS);
        exit();
    }

    /* Local Variables */
    int i;
    int timeticks, no_of_children;
    int tickets[MAX_CHILD_PROCESS];
    int child_pids[MAX_CHILD_PROCESS];
    int parent_pid, parent_tickets;
    processes_info before, after;
    int index1, index2;

    /* Get parent pid */
    parent_pid = getpid();
    /* Get the tickets to be set for parent */
    parent_tickets = atoi(argv[2]);
    /* Get the ticks for which parent should sleep */
    timeticks = atoi(argv[1]);
    /* Number of children to be forked */
    no_of_children = argc - 3;

    /* Assign tickets to the parent process (this process) */
    if (assign_tickets(parent_pid, parent_tickets) != 0) {
        printf(1, "Error setting the tickets\n");
        exit();
    }

    /* Get the tickets to be set for the child processes */
    for (i = 0; i < argc - 3; i++) {

        tickets[i] = atoi(argv[i + 3]);
        /* If the tickets to be assigned are more than a particular limit */
        if (tickets[i] > MAX_TICKETS_PER_PROCESS) {
            printf(1, "Maximum tickets allowed for a process is %d\n", MAX_TICKETS_PER_PROCESS);
            exit();
        }
    }

    /* Create the child process and assign tickets to them */
    for (i = 0; i < no_of_children; i++) {

        child_pids[i] = create_process(tickets[i]);
        if (child_pids[i] == -1) {
            printf(1, "Error creating child process\n");
            exit();
        }
    }

    /* Wait till the tickets of child processes are fixed */
    if (wait_for_tickets_to_set(no_of_children, child_pids, tickets) == -1) {
        printf(1, "Not able to set child processes tickets\n");
        exit();
    }

    /* Get the process information before the parent goes to sleep */
    proc_info(&before);
    /* Let the parent sleep */
    sleep(timeticks);
    /* Get the process information after the parent wakes up */
    proc_info(&after);

    /* Kill all the child processes */
    for (i = 0; i < no_of_children; i++) {
        kill(child_pids[i]);
    }

    /* Wait till all child processes are killed */
    for (i = 0; i < no_of_children; i++) {
        wait();
    }

    /* Print the child process information */
    printf(1, "CHILD\t\tPID\t\tTICKS\t\tTICKETS\n");
    for (i = 0; i < no_of_children; i++) {

        /* Get the index of child in process info before the parent going to sleep */
        index1 = find_index_of_pid(&before, child_pids[i]);
        /* Get the index of child in process info after the parent wakes up */
        index2 = find_index_of_pid(&after, child_pids[i]);

        /* If child process not found in process info, skip it */
        if (index1 == -1 || index2 == -1) {
            printf(1, "Child Process with pid %d not found in process list\n", child_pids[i]);
            continue;
        }
        printf(1, "%d\t\t%d\t\t%d\t\t%d\n", i + 1, child_pids[i], after.ticks[index2] - before.ticks[index1], after.tickets[index2]);
    }

    /* Print the parent process (this process) information */
    printf(1, "THIS PROCESS :\n");
    proc_info(&after);

    /* Get the parent process index in process info */
    index1 = find_index_of_pid(&after, parent_pid);
    if (index1 == -1) {
        printf(1, "THIS PROCESS not found in process list\n");
        exit();
    }
    printf(1, "PID = %d\t\tTICKS = %d\t\t TICKETS = %d\n", parent_pid, after.ticks[index1], after.tickets[index1]);

    exit();
}
