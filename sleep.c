#include "types.h"
#include "user.h"

int main(int argc, char *argv[]) {

	int sleep_ticks;

    /* Check if required inputs are provided */
	if (argc < 2) {
		printf(1, "Usage: sleep ticks\n");
		exit();
	}

    /* Convert to integer */
	sleep_ticks = atoi(argv[1]);

    /* Check if the input is positive */
	if (sleep_ticks > 0) {
		sleep(sleep_ticks);
	}
    /* If input is negative */
    else {
		printf(1, "Invalid input\n");
	}

	exit();
}
