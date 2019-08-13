// Ping-pong a counter between two processes.
// Only need to start one of these -- splits into two with fork.

#include "lib.h"

	void
umain(void)
{
	u_int who, i;
	for (;;) {
		writef("%x am waiting.....\n", syscall_getenvid());
		i = ipc_recv(&who, 0, 0);

		writef("%x got %d from %x\n", syscall_getenvid(), i, who);

		//user_panic("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&");
		if (i == 10) {
			return;
		}
	}
}

