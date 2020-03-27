#include "types.h"
#include "user.h"
#include "param.h"

#define MAX_CHILD_PROCESS   16
#define MAX_PARENT_YIELDS   100

int create_process(int tickets) {

    int pid = fork();
    if (pid == 0) {
        if (assign_tickets(getpid(), tickets) == 0) {
            while (1);
        }
        else {
            printf(1, "Error setting the tickets\n");
            exit();
        }
    }
    else if (pid > 0) {
        return pid;
    }
    else {
        return -1;
    }
}

int find_index_of_pid(processes_info *information, int pid) {

    int i;
    for (i = 0; i < information->num_processes; i++) {
        if (information->pids[i] == pid) {
            return i;
        }
    }
    return -1;
}

void wait_for_tickets_to_set(int no_of_children, int *child_pids, int *tickets) {

    int i, j, done, index;
    processes_info info;

    for (i = 0; i < MAX_PARENT_YIELDS; i++) {
        done = 1;
        proc_info(&info);
        for (j = 0; j < no_of_children; j++) {
            index = find_index_of_pid(&info, child_pids[i]);
            if (index != -1) {
                if (tickets[j] != info.tickets[index]) {
                    done = 0;
                    break;
                }
            }
        }
        if (done) {
            break;
        }
    }
}

int main(int argc, char *argv[]) {

    if (argc < 4) {
        printf(1, "Usage : %s 'time-ticks' parent_tickets child1_tickets child2_tickets ... \n" \
                  "time-ticks is the number of ticks the parent process must sleep for\n" \
                  "parent_tickets to be assigned to the parent process\n" \
                  "childN_tickets is the number of tickets to be assigned to the Nth child process\n", argv[0]);
        exit();
    }
    if (argc > MAX_CHILD_PROCESS + 3) {
        printf(1, "Only %d child process supported", MAX_CHILD_PROCESS);
        exit();
    }

    int i;
    int timeticks, no_of_children;
    int tickets[MAX_CHILD_PROCESS];
    int child_pids[MAX_CHILD_PROCESS];
    int parent_pid, parent_tickets;
    processes_info before, after;
    int index1, index2;

    parent_pid = getpid();
    parent_tickets = atoi(argv[2]);
    timeticks = atoi(argv[1]);
    no_of_children = argc - 3;
    if (assign_tickets(parent_pid, parent_tickets) != 0) {
        printf(1, "Error setting the tickets\n");
        exit();
    }

    for (i = 0; i < argc - 3; i++) {
        tickets[i] = atoi(argv[i + 3]);
        if (tickets[i] > MAX_TICKETS_PER_PROCESS) {
            printf(1, "Maximum tickets allowed for a process is %d\n", MAX_TICKETS_PER_PROCESS);
            exit();
    }
/*
    if (sched_test(timeticks, i, tickets) == -1) {
        printf(1, "Error encountered\n");
    }
*/

    for (i = 0; i < no_of_children; i++) {
        child_pids[i] = create_process(tickets[i]);
        if (child_pids[i] == -1) {
            printf(1, "Error creating child process\n");
            exit();
        }
    }

    wait_for_tickets_to_set(no_of_children, child_pids, tickets);
    proc_info(&before);
    sleep(timeticks);
    proc_info(&after);

    for (i = 0; i < no_of_children; i++) {
        kill(child_pids[i]);
    }
    for (i = 0; i < no_of_children; i++) {
        wait();
    }

    printf(1, "CHILD\t\tPID\t\tTICKS\t\tTICKETS\n");
    for (i = 0; i < no_of_children; i++) {

        index1 = find_index_of_pid(&before, child_pids[i]);
        index2 = find_index_of_pid(&after, child_pids[i]);
        if (index1 == -1 || index2 == -1) {
            printf(1, "Child Process with pid %d not found in process list\n", child_pids[i]);
            continue;
        }
        printf(1, "%d\t\t%d\t\t%d\t\t%d\n", i + 1, child_pids[i], after.ticks[index2] - before.ticks[index1], tickets[i]);
    }

    printf(1, "THIS PROCESS / PARENT :\n");
    proc_info(&after);

    index1 = find_index_of_pid(&after, parent_pid);
    if (index1 == -1) {
        printf(1, "THIS process not found in process list\n");
        exit();
    }
    printf(1, "PID = %d\t\tTICKS = %d\t\t TICKETS = %d\n", parent_pid, after.ticks[index2], after.tickets[index2]);

    exit();
}
