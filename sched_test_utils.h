int find_index_of_pid(processes_info *information, int pid);
int wait_for_tickets_to_set(int no_of_children, int *child_pids, int *tickets);
int create_process(int tickets);
int create_more_processes(int *child_pids, int *tickets, int n);
int create_nested_processes(int *child_pids, int *tickets, int n);
