#include "param.h"

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef uint pde_t;

typedef struct processes_info {
    int num_processes;
    int pids[NPROC];
    int ticks[NPROC];
    int tickets[NPROC];
} processes_info;
