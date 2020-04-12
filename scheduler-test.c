#include "types.h"
#include "user.h"
#include "param.h"
#include "sched_test_utils.h"

#define MAX_CHILD_PROCESS               16
#define PRINT_SCHED_ORDER               1

#define AUTO_TEST_MAX_CHILD             ((10) % (MAX_CHILD_PROCESS + 1))
#define AUTO_TEST_PARENT_SLEEPTICKS     ((AUTO_TEST_MAX_CHILD) * (100))
#define AUTO_TEST_PARENT_TICKETS        500
#define AUTO_TEST_FIRST_CHILD_TICKETS   100
#define AUTO_TEST_TICKET_INCR           ((int)((MAX_TICKETS_PER_PROCESS) - (AUTO_TEST_FIRST_CHILD_TICKETS)) \
                                        / (AUTO_TEST_MAX_CHILD))

/* Main function */
int main(int argc, char *argv[]) {

    int autotest = 0;

    if (argc == 2 && (!strcmp(argv[1], "auto"))) {
        autotest = 1;
    }
    /* Check if required inputs are provided by user */
    else if (argc < 4) {
        printf(1, "Usage : FOR AUTO TESTING : %s auto\n" \
                  "Usage : FOR MANUAL TESTING : %s 'time-ticks' 'parent_tickets' 'child1_tickets' 'child2_tickets' ... \n" \
                  "\ttime-ticks is the number of ticks the parent process must sleep for\n" \
                  "\tparent_tickets to be assigned to the parent process\n" \
                  "\tchildN_tickets is the number of tickets to be assigned to the Nth child process\n", argv[0], argv[0]);
        exit();
    }
    /* Check if child processes more than a particular limit are to be forked */
    else if (argc > MAX_CHILD_PROCESS + 3) {
        printf(1, "Only %d child process supported\n", MAX_CHILD_PROCESS);
        exit();
    }

    /* Local Variables */
    int i;
    int timeticks, no_of_children;
    int tickets[MAX_CHILD_PROCESS];
    int child_pids[MAX_CHILD_PROCESS];
    int parent_pid, parent_tickets;
    processes_info before, after;
    int temp1, temp2, temp3;

    /* Get parent pid */
    parent_pid = getpid();

    if (autotest) {

        /* Ticks for which parent should sleep */
        timeticks = AUTO_TEST_PARENT_SLEEPTICKS;
        /* Number of children to be forked */
        no_of_children = AUTO_TEST_MAX_CHILD;
        /* Tickets to be assigned to the first child process */
        temp1 = AUTO_TEST_FIRST_CHILD_TICKETS;

        /* Assign tickets to the parent process (this process) */
        if (assign_tickets(parent_pid, AUTO_TEST_PARENT_TICKETS) != 0) {
            printf(1, "Error setting the tickets\n");
            exit();
        }

        for (i = 0; i < no_of_children; i++) {
            tickets[i] = temp1;
            temp1 += AUTO_TEST_TICKET_INCR;
        }
    }
    else {

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
    }

    /* --------------------------------------------------------------------- */

    printf(1, "-----------------------\n");
    printf(1, "Scheduler Test 1 ...\n");
    if (PRINT_SCHED_ORDER)
        printf(1, "Scheduling Order :\n");

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

    /* Set/clear the scheduling order flag */
    change_schedorder_flag(PRINT_SCHED_ORDER);
    /* Get the process information before the parent goes to sleep */
    proc_info(&before);
    /* Let the parent sleep */
    sleep(timeticks);
    /* Get the process information after the parent wakes up */
    proc_info(&after);
    /* Clear the scheduling order flag */
    change_schedorder_flag(0);

    /* Kill all the child processes */
    for (i = 0; i < no_of_children; i++) {
        kill(child_pids[i]);
    }

    /* Wait till all child processes are killed */
    for (i = 0; i < no_of_children; i++) {
        wait();
    }

    /* Print the child process information */
    print_test_result(no_of_children, &before, &after, child_pids);

    printf(1, "Scheduler Test 1 OK\n");

    /* --------------------------------------------------------------------- */

    /* Run next test if processes to be created are enough */
    if (no_of_children >= 2) {

        printf(1, "-----------------------\n");
        printf(1, "Scheduler Test 2 ...\n");
        if (PRINT_SCHED_ORDER)
            printf(1, "Scheduling Order :\n");

        /* Create a process that creates multiple processes */
        temp1 = (int)(no_of_children / 2);
        create_more_processes(child_pids, tickets, temp1);

        /* Create a process that creates multiple processes */
        temp2 = no_of_children - temp1;
        create_more_processes(child_pids + temp1, tickets + temp1, temp2);

        /* Wait till the tickets of child processes are fixed */
        if (wait_for_tickets_to_set(no_of_children, child_pids, tickets) == -1) {
            printf(1, "Not able to set child processes tickets\n");
            exit();
        }

        /* Set/clear the scheduling order flag */
        change_schedorder_flag(PRINT_SCHED_ORDER);
        /* Get the process information before the parent goes to sleep */
        proc_info(&before);
        /* Let the parent sleep */
        sleep(timeticks);
        /* Get the process information after the parent wakes up */
        proc_info(&after);
        /* Clear the scheduling order flag */
        change_schedorder_flag(0);

        /* Kill all the child processes */
        for (i = 0; i < no_of_children; i++) {
            kill(child_pids[i]);
        }

        /* Wait till all child processes are killed */
        wait();
        wait();

        /* Print the child process information */
        print_test_result(no_of_children, &before, &after, child_pids);

        printf(1, "Scheduler Test 2 OK\n");
    }

    /* --------------------------------------------------------------------- */

    /* Run next test if processes to be created are enough */
    if (no_of_children >= 3) {

        printf(1, "-----------------------\n");
        printf(1, "Scheduler Test 3 ...\n");
        if (PRINT_SCHED_ORDER)
            printf(1, "Scheduling Order :\n");

        /* Create a process that creates multiple processes */
        temp1 = (int)(no_of_children / 3);
        create_more_processes(child_pids, tickets, temp1);

        /* Create a process that creates multiple processes */
        temp2 = (int)((no_of_children - temp1) / 2);
        create_more_processes(child_pids + temp1, tickets + temp1, temp2);

        /* Create a process that creates multiple processes */
        temp3 = no_of_children - temp1 - temp2;
        create_more_processes(child_pids + temp1 + temp2, tickets + temp1 + temp2, temp3);

        /* Wait till the tickets of child processes are fixed */
        if (wait_for_tickets_to_set(no_of_children, child_pids, tickets) == -1) {
            printf(1, "Not able to set child processes tickets\n");
            exit();
        }

        /* Set/clear the scheduling order flag */
        change_schedorder_flag(PRINT_SCHED_ORDER);
        /* Get the process information before the parent goes to sleep */
        proc_info(&before);
        /* Let the parent sleep */
        sleep(timeticks);
        /* Get the process information after the parent wakes up */
        proc_info(&after);
        /* Clear the scheduling order flag */
        change_schedorder_flag(0);

        /* Kill all the child processes */
        for (i = 0; i < no_of_children; i++) {
            kill(child_pids[i]);
        }

        /* Wait till all child processes are killed */
        wait();
        wait();
        wait();

        /* Print the child process information */
        print_test_result(no_of_children, &before, &after, child_pids);

        printf(1, "Scheduler Test 3 OK\n");
    }

    /* --------------------------------------------------------------------- */

    printf(1, "-----------------------\n");
    printf(1, "Scheduler Test 4 ...\n");
    if (PRINT_SCHED_ORDER)
        printf(1, "Scheduling Order :\n");

    /* Create a process that creates nested processes */
    create_nested_processes(child_pids, tickets, no_of_children);

    /* Wait till the tickets of child processes are fixed */
    if (wait_for_tickets_to_set(no_of_children, child_pids, tickets) == -1) {
        printf(1, "Not able to set child processes tickets\n");
        exit();
    }

    /* Set/clear the scheduling order flag */
    change_schedorder_flag(PRINT_SCHED_ORDER);
    /* Get the process information before the parent goes to sleep */
    proc_info(&before);
    /* Let the parent sleep */
    sleep(timeticks);
    /* Get the process information after the parent wakes up */
    proc_info(&after);
    /* Clear the scheduling order flag */
    change_schedorder_flag(0);

    /* Kill all the child processes */
    for (i = 0; i < no_of_children; i++) {
        kill(child_pids[i]);
    }

    /* Wait till all child processes are killed */
    wait();

    /* Print the child process information */
    print_test_result(no_of_children, &before, &after, child_pids);

    printf(1, "Scheduler Test 4 OK\n");

    /* --------------------------------------------------------------------- */

    /* Run next test if processes to be created are enough */
    if (no_of_children >= 3) {

        printf(1, "-----------------------\n");
        printf(1, "Scheduler Test 5 ...\n");
        if (PRINT_SCHED_ORDER)
            printf(1, "Scheduling Order :\n");

        /* Create a process that creates nested processes */
        temp1 = (int)(no_of_children / 3);
        create_nested_processes(child_pids, tickets, temp1);

        /* Create a process that creates nested processes */
        temp2 = (int)((no_of_children - temp1) / 2);
        create_nested_processes(child_pids + temp1, tickets + temp1, temp2);

        /* Create a process that creates nested processes */
        temp3 = no_of_children - temp1 - temp2;
        create_nested_processes(child_pids + temp1 + temp2, tickets + temp1 + temp2, temp3);

        /* Wait till the tickets of child processes are fixed */
        if (wait_for_tickets_to_set(no_of_children, child_pids, tickets) == -1) {
            printf(1, "Not able to set child processes tickets\n");
            exit();
        }

        /* Set/clear the scheduling order flag */
        change_schedorder_flag(PRINT_SCHED_ORDER);
        /* Get the process information before the parent goes to sleep */
        proc_info(&before);
        /* Let the parent sleep */
        sleep(timeticks);
        /* Get the process information after the parent wakes up */
        proc_info(&after);
        /* Clear the scheduling order flag */
        change_schedorder_flag(0);

        /* Kill all the child processes */
        for (i = 0; i < no_of_children; i++) {
            kill(child_pids[i]);
        }

        /* Wait till all child processes are killed */
        wait();

        /* Print the child process information */
        print_test_result(no_of_children, &before, &after, child_pids);

        printf(1, "Scheduler Test 5 OK\n");
    }

    /* --------------------------------------------------------------------- */

    /* Run next test if processes to be created are enough */
    if (no_of_children >= 3) {

        printf(1, "-----------------------\n");
        printf(1, "Scheduler Test 6 ...\n");
        if (PRINT_SCHED_ORDER)
            printf(1, "Scheduling Order :\n");

        /* Create a process that creates nested processes */
        temp1 = (int)(no_of_children / 3);
        create_nested_processes(child_pids, tickets, temp1);

        /* Create a process that creates nested processes */
        create_nested_processes(child_pids + temp1, tickets + temp1, temp1);

        /* Create a process that creates multiple processes */
        temp2 = no_of_children - (2 * temp1);
        create_more_processes(child_pids + (2 * temp1), tickets + (2 * temp1), temp2);

        /* Wait till the tickets of child processes are fixed */
        if (wait_for_tickets_to_set(no_of_children, child_pids, tickets) == -1) {
            printf(1, "Not able to set child processes tickets\n");
            exit();
        }

        /* Set/clear the scheduling order flag */
        change_schedorder_flag(PRINT_SCHED_ORDER);
        /* Get the process information before the parent goes to sleep */
        proc_info(&before);
        /* Let the parent sleep */
        sleep(timeticks);
        /* Get the process information after the parent wakes up */
        proc_info(&after);
        /* Clear the scheduling order flag */
        change_schedorder_flag(0);

        /* Kill all the child processes */
        for (i = 0; i < no_of_children; i++) {
            kill(child_pids[i]);
        }

        /* Wait till all child processes are killed */
        wait();

        /* Print the child process information */
        print_test_result(no_of_children, &before, &after, child_pids);

        printf(1, "Scheduler Test 6 OK\n");
    }

    /* --------------------------------------------------------------------- */

    /* Print the parent process (this process) information */
    printf(1, "-----------------------\n");
    printf(1, "PARENT PROCESS :\n");
    proc_info(&after);

    /* Get the parent process index in process info */
    temp1 = find_index_of_pid(&after, parent_pid);
    if (temp1 == -1) {
        printf(1, "PARENT PROCESS not found in process list\n");
        exit();
    }
    printf(1, "PID = %d\t\tTICKS = %d\t\t TICKETS = %d\n", parent_pid, after.ticks[temp1], after.tickets[temp1]);
    printf(1, "-----------------------\n");

    exit();
}
