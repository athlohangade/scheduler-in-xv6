#include "types.h"
#include "user.h"

#define MAX_PARENT_YIELDS   1000

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

        /* Allow a chance to the child process to get scheduled */
        sleep(1);

        /* Assume the tickets of all child processes are set */
        done = 1;
        /* System call to get the processes information */
        proc_info(&info);

        /* Loop to check if the tickets are set */
        for (j = 0; j < no_of_children; j++) {

            /* Find the index of child in processes information */
            index = find_index_of_pid(&info, child_pids[j]);

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

/* Function to fork the child process and assign tickets */
int create_process(int tickets) {

    /* Fork the process */
    int pid;
    pid = fork();

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

/* Function to fork multiple child process and assign tickets */
int create_more_processes(int *child_pids, int *tickets, int n) {

    int i;
    int pid;
    int pfd[2];

    /* Create pipe */
    if (pipe(pfd) != 0) {
        printf(1, "Error creating pipe\n");
        exit();
    }

    /* Fork the process */
    pid = fork();
    if (pid == 0) {

        /* Close reading end */
        close(pfd[0]);

        pid = getpid();
        /* Assign the tickets to the child process */
        if (assign_tickets(pid, tickets[0]) == 0) {

            /* Fork multiple process and assign them tickets */
            for (i = 1; i < n; i++) {
                pid = create_process(tickets[i]);
                if (pid == -1) {
                    printf(1, "Error creating child process\n");
                    exit();
                }

                /* Write the pids of forked process into pipe so that parent
                 * could read */
                write(pfd[1], &pid, sizeof(int));
            }

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

        /* Close writing end */
        close(pfd[1]);

        child_pids[0] = pid;
        /* Read the child pids that child process has written in pipe */
        for (i = 1; i < n; i++) {
            read(pfd[0], &child_pids[i], sizeof(int));
        }

        return pid;
    }
    /* If fork fails */
    else {
        return -1;
    }
}

/* Function to fork nested child process and assign tickets */
int create_nested_processes(int *child_pids, int *tickets, int n) {

    /* Stopping condition */
    if (n == 0) {
        return 0;
    }

    int pid, i;
    int pfd[2];

    /* Create pipe */
    if (pipe(pfd) != 0) {
        printf(1, "Error creating pipe\n");
        exit();
    }

    /* Fork the proces */
    pid = fork();
    if (pid == 0) {

        close(pfd[0]);

        pid = getpid();
        /* Assign the tickets to the child process */
        if (assign_tickets(pid, tickets[0]) == 0) {

            /* Make recursive call to create nested process */
            pid = create_nested_processes(child_pids, tickets + 1, n - 1);
            if (pid != 0) {

                /* Write the collected pids into pipe */
                for (i = 0; i < n - 1; i++) {
                    write(pfd[1], &child_pids[i], sizeof(int));
                }
            }

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

        close(pfd[1]);

        /* Read from pipe */
        child_pids[0] = pid;
        for (i = 1; i < n; i++) {
            read(pfd[0], &child_pids[i], sizeof(int));
        }

        return pid;
    }
    /* If fork fails */
    else {
        return -1;
    }
}

/* Function to print the scheduler test result */
void print_test_result(int no_of_children, processes_info *before, processes_info *after, int *child_pids) {

    int i, index1, index2;

    /* Print the child process information */
    printf(1, "SUMMARY :\nCHILD\t\tPID\t\tTICKS\t\tTICKETS\n");
    for (i = 0; i < no_of_children; i++) {

        /* Get the index of child in process info before the parent going to sleep */
        index1 = find_index_of_pid(before, child_pids[i]);
        /* Get the index of child in process info after the parent wakes up */
        index2 = find_index_of_pid(after, child_pids[i]);

        /* If child process not found in process info, skip it */
        if (index1 == -1 || index2 == -1) {
            printf(1, "Child Process with pid %d not found in process list\n", child_pids[i]);
            continue;
        }
        printf(1, "%d\t\t%d\t\t%d\t\t%d\n", i + 1, child_pids[i], after->ticks[index2] - before->ticks[index1], after->tickets[index2]);
    }
}
