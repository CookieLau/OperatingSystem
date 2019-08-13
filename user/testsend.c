// Ping-pong a counter between two processes.
// Only need to start one of these -- splits into two with fork.

#include "lib.h"

void umain(void)
{
	u_int who=4097, i=0;
	for (;;) {
		i++;
		ipc_send(who, i, 0, 0);
		writef("\n@@@@@send %d from %x to %x\n",i, syscall_getenvid(), who);
		if (i == 10) {
			return;
		}
	}

}

