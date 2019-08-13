#include <asm/asm.h>
#include <pmap.h>
#include <env.h>
#include <printf.h>
#include <kclock.h>
#include <trap.h>

extern char aoutcode[];
extern char boutcode[];

void mips_init()
{
	printf("init.c:\tmips_init() is called\n");

	// Lab 2 memory management initialization functions
	mips_detect_memory();
	mips_vm_init();
	page_init();
	//page_check();

	// Lab 3 user environment initialization functions
	env_init();
//	printf("pass env_init line 23\n");
	// initialize exception vector.
	trap_init();
//	printf("pass trap_init line 26\n");
	// initialize PIT.
	kclock_init();
//	printf("pass kclock_init line 29\n");

	// Create process using macro 'ENC_CREATE'
	// For more details about 'ENV_CREATE', see include/env.h
	ENV_CREATE(user_fstest);
//ENV_CREATE(user_pingpong);
ENV_CREATE(fs_serv);

	panic("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");

	while (1);

	panic("init.c:\tend of mips_init() reached!");
}

void bcopy(const void *src, void *dst, size_t len)
{
	void *max;

	max = dst + len;

	// copy machine words while possible
	while (dst + 3 < max) {
		*(int *)dst = *(int *)src;
		dst += 4;
		src += 4;
	}

	// finish remaining 0-3 bytes
	while (dst < max) {
		*(char *)dst = *(char *)src;
		dst += 1;
		src += 1;
	}
}

void bzero(void *b, size_t len)
{
	void *max;

	max = b + len;

	//printf("init.c:\tzero from %x to %x\n",(int)b,(int)max);

	// zero machine words while possible

	while (b + 3 < max) {
		*(int *)b = 0;
		b += 4;
	}

	// finish remaining 0-3 bytes
	while (b < max) {
		*(char *)b++ = 0;
	}

}
