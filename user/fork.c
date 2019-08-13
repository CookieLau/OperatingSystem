// implement fork from user space

#include "lib.h"
#include <mmu.h>
#include <env.h>


/* ----------------- help functions ---------------- */

/* Overview:
 * 	Copy `len` bytes from `src` to `dst`.
 *
 * Pre-Condition:
 * 	`src` and `dst` can't be NULL. Also, the `src` area 
 * 	 shouldn't overlap the `dest`, otherwise the behavior of this 
 * 	 function is undefined.
 */
void user_bcopy(const void *src, void *dst, size_t len)
{
	void *max;

	//	writef("~~~~~~~~~~~~~~~~ src:%x dst:%x len:%x\n",(int)src,(int)dst,len);
	max = dst + len;

	// copy machine words while possible
	if (((int)src % 4 == 0) && ((int)dst % 4 == 0)) {
		while (dst + 3 < max) {
			*(int *)dst = *(int *)src;
			dst += 4;
			src += 4;
		}
	}

	// finish remaining 0-3 bytes
	while (dst < max) {
		*(char *)dst = *(char *)src;
		dst += 1;
		src += 1;
	}

	//for(;;);
}

/* Overview:
 * 	Sets the first n bytes of the block of memory 
 * pointed by `v` to zero.
 * 
 * Pre-Condition:
 * 	`v` must be valid.
 *
 * Post-Condition:
 * 	the content of the space(from `v` to `v`+ n) 
 * w½(ill be set to zero.
 */
void user_bzero(void *v, u_int n)
{
	char *p;
	int m;

	p = v;
	m = n;

	while (--m >= 0) {
		*p++ = 0;
	}
}
/*--------------------------------------------------------------*/

/* Overview:
 * 	Custom page fault handler - if faulting page is copy-on-write,
 * map in our own private writable copy.
 * 
 * Pre-Condition:
 * 	`va` is the address which leads to a TLBS exception.
 *
 * Post-Condition:
 *  Launch a user_panic if `va` is not a copy-on-write page.
 * Otherwise, this handler should map a private writable copy of 
 * the faulting page at correct address.
 */
static void pgfault(u_int va)
{

	u_int *tmp;
	u_int temp = UXSTACKTOP-2*BY2PG;
	//	writef("fork.c:pgfault():\t va:%x\n",va);
   	u_int round_va = ROUNDDOWN(va,BY2PG);
	u_int perm = (* vpt)[VPN(round_va)] & 0xfff;
	writef("here enter the pgfault func va:%x, perm:%x\n",va,perm);
	if (!(perm & PTE_COW)) {
		user_panic("va:%x perm is not copy-on-write\n",va);
		return;
	}
    //map the new page at a temporary place
	if (syscall_mem_alloc(0,temp,perm & (~PTE_COW)) < 0) {
		user_panic("can not alloc mem in pgfault\n");
		return;
	}
	//copy the content
	user_bcopy((void *)round_va,(void *)temp,BY2PG);
    //map the page on the appropriate place
	if (syscall_mem_map(0,temp,0,round_va,perm & (~PTE_COW)) < 0) {
		user_panic("can not map in pgfault\n");
		return;
	}
    //unmap the temporary place
	if (syscall_mem_unmap(0,temp) < 0) {
		user_panic("can not unmap in pgfault\n");
		return;
	}
	return;
}

/* Overview:
 * 	Map our virtual page `pn` (address pn*BY2PG) into the target `envid`
 * at the same virtual address. 
 *
 * Post-Condition:
 *  if the page is writable or copy-on-write, the new mapping must be 
 * created copy on write and then our mapping must be marked 
 * copy on write as well. In another word, both of the new mapping and
 * our mapping should be copy-on-write if the page is writable or 
 * copy-on-write.
 * 
 * Hint:
 * 	PTE_LIBRARY indicates that the page is shared between processes.
 * A page with PTE_LIBRARY may have PTE_R at the same time. You
 * should process it correctly.
 */
static void
duppage(u_int envid, u_int pn)
{
	u_int addr;
	u_int perm;
	
	addr = pn*BY2PG;
	perm = (* vpt)[pn] & 0xfff;
	//two situation: writable or copy-on-write
	//writable = PTE_R
	//copy-on-write = PTE_COW
	//both should have a pre-requisite:PTE_V
	if (((perm & PTE_R) || ((perm & PTE_COW)) && (perm & PTE_V))) {
		if (perm & PTE_LIBRARY) {
			perm = PTE_V | PTE_R | PTE_LIBRARY;
		} else {
			perm = PTE_V | PTE_R | PTE_COW;
		}
		if (syscall_mem_map(0,addr,envid,addr,perm) < 0) {
			user_panic("syscall_mem_map for son failed.\n");
		}
		if (syscall_mem_map(0,addr,0,addr,perm) < 0) {
			user_panic("syscall_mem_map for father failed.\n");
		}
	} else {
		if (syscall_mem_map(0,addr,envid,addr,perm) < 0) {
			user_panic("syscall_mem_map for son failed.\n");
		}
	}


	//	user_panic("duppage not implemented");
}

/* Overview:
 * 	User-level fork. Create a child and then copy our address space
 * and page fault handler setup to the child.
 *
 * Hint: use vpd, vpt, and duppage.
 * Hint: remember to fix "env" in the child process!
 * Note: `set_pgfault_handler`(user/pgfault.c) is different from 
 *       `syscall_set_pgfault_handler`. 
 */
extern void __asm_pgfault_handler(void);
int
fork(void)
{
	// Your code here.
	u_int newenvid;
	extern struct Env *envs;
	extern struct Env *env;
	u_int i,j;

	//The parent installs pgfault using set_pgfault_handler
	set_pgfault_handler(pgfault);
	//writef("reach line 184\n");
	//alloc a new alloc
	if ((newenvid = syscall_env_alloc()) == 0) { //means it's a son,who can not get its own envid
		env = &envs[ENVX(syscall_getenvid())];//ENVX is to get the envid
		return 0;//success creating a son env
	}

	
//	writef("reach line 190\n");
/*	for (i=0;i<UTOP-2*BY2PG;i+=BY2PG) {
		if (((*vpd)[VPN(i)/1024]) != 0 && (*vpt)[VPN(i)] != 0) {
			duppage(newenvid,VPN(i));
		}
	}*/
	for (i=0;i<=PDX(UTOP-2*BY2PG);i++) {
		if ((*vpd)[i]) {
			for (j=0;j<1024;j++) {
				if (i*1024+j<VPN(UTOP-2*BY2PG) && (*vpt)[i*1024+j]) {
					duppage(newenvid,i*1024+j);
				}
			}
		}
	}
//	set_pgfault_handler(pgfault);
	if (syscall_mem_alloc(newenvid,UXSTACKTOP-BY2PG,PTE_V|PTE_R) < 0){
		user_panic("cannot alloc uxstacktop in fork\n");
		return 0;
	}
//	writef("reach line 203\n");
	if (syscall_set_pgfault_handler(newenvid,__asm_pgfault_handler,UXSTACKTOP) < 0) {
		user_panic("cannot handle pgfault in fork\n");
		return 0;
	}
	syscall_set_env_status(newenvid,ENV_RUNNABLE);
	return newenvid;
}

// Challenge!
int
sfork(void)
{
	user_panic("sfork not implemented");
	return -E_INVAL;
}
