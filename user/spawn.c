#include "lib.h"
#include <mmu.h>
#include <env.h>
#include <kerelf.h>

#define debug 0
#define TMPPAGE		(BY2PG)
#define TMPPAGETOP	(TMPPAGE+BY2PG)

	int
init_stack(u_int child, char **argv, u_int *init_esp)
{
	int argc, i, r, tot;
	char *strings;
	u_int *args;

	// Count the number of arguments (argc)
	// and the total amount of space needed for strings (tot)
	tot = 0;
	for (argc=0; argv[argc]; argc++)
		tot += strlen(argv[argc])+1;

	// Make sure everything will fit in the initial stack page
	if (ROUND(tot, 4)+4*(argc+3) > BY2PG)
		return -E_NO_MEM;

	// Determine where to place the strings and the args array
	strings = (char*)TMPPAGETOP - tot;
	args = (u_int*)(TMPPAGETOP - ROUND(tot, 4) - 4*(argc+1));

	if ((r = syscall_mem_alloc(0, TMPPAGE, PTE_V|PTE_R)) < 0)
		return r;
	// Replace this with your code to:
	//
	//	- copy the argument strings into the stack page at 'strings'
	char *ctemp,*argv_temp;
	u_int j;
	ctemp = strings;
	for(i = 0;i < argc; i++)
	{
		argv_temp = argv[i];
		for(j=0;j < strlen(argv[i]);j++)
		{
			*ctemp = *argv_temp;
			ctemp++;
			argv_temp++;
		}
		*ctemp = 0;
		ctemp++;
	}
	//	- initialize args[0..argc-1] to be pointers to these strings
	//	  that will be valid addresses for the child environment
	//	  (for whom this page will be at USTACKTOP-BY2PG!).
	ctemp = (char *)(USTACKTOP - TMPPAGETOP + (u_int)strings);
	for(i = 0;i < argc;i++)
	{
		args[i] = (u_int)ctemp;
		ctemp += strlen(argv[i])+1;
	}
	//	- set args[argc] to 0 to null-terminate the args array.
	ctemp--;
	args[argc] = ctemp;
	//	- push two more words onto the child's stack below 'args',
	//	  containing the argc and argv parameters to be passed
	//	  to the child's umain() function.
	u_int *pargv_ptr;
	pargv_ptr = args - 1;
	*pargv_ptr = USTACKTOP - TMPPAGETOP + (u_int)args;
	pargv_ptr--;
	*pargv_ptr = argc;
	//
	//	- set *init_esp to the initial stack pointer for the child
	//
	*init_esp = USTACKTOP - TMPPAGETOP + (u_int)pargv_ptr;
	//	*init_esp = USTACKTOP;	// Change this!

	if ((r = syscall_mem_map(0, TMPPAGE, child, USTACKTOP-BY2PG, PTE_V|PTE_R)) < 0)
		goto error;
	if ((r = syscall_mem_unmap(0, TMPPAGE)) < 0)
		goto error;

	return 0;

error:
	syscall_mem_unmap(0, TMPPAGE);
	return r;
}

int usr_is_elf_format(u_char *binary){
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)binary;
	if (ehdr->e_ident[0] == ELFMAG0 &&
			ehdr->e_ident[1] == ELFMAG1 &&
			ehdr->e_ident[2] == ELFMAG2 &&
			ehdr->e_ident[3] == ELFMAG3) {
		return 1;
	}   

	return 0;
}

int 
usr_load_elf(int fd , int child_envid){
	//Hint: maybe this function is useful 
	//      If you want to use this func, you should fill it ,it's not hard
	u_char *binary = fd2data(fd);
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)binary;
	Elf32_Phdr *phdr;
	u_char *ptr_ph_table = NULL;
	Elf32_Half ph_entry_count;
	Elf32_Half ph_entry_size;
	u_int *blk;
	int r,i;
	int size = ((struct Filefd*)fd)->f_file.f_size;
	if (size < 4 || !usr_is_elf_format(binary)) {
		writef("error in usr_load_elf, not elf_format\n");
		return -E_INVAL;
	}

	ptr_ph_table = binary + ehdr->e_phoff;
	ph_entry_count = ehdr->e_phnum;
	ph_entry_size  = ehdr->e_phentsize;

	while (ph_entry_count--) {
		phdr = (Elf32_Phdr *)ptr_ph_table;
		if (phdr->p_type == PT_LOAD){
			if ((r=usr_load_icode_mapper(phdr->p_vaddr,phdr->p_memsz,binary+phdr->p_offset,phdr->p_filesz,child_envid)) <0) {
				user_panic("wrong in usr_load_icode_mapper\n");
				return r;
			}
		}
		ptr_ph_table += ph_entry_size;
	}
	return 0;
}
int usr_load_icode_mapper(u_long va, u_int32_t sgsize,
		u_char *bin, u_int32_t bin_size, int child_envid)
{
	u_long i;
	int r;
	u_long offset = va - ROUNDDOWN(va, BY2PG);

	for (i = 0; i < bin_size; i += BY2PG) {
		if ((r = syscall_mem_alloc(0, TMPPAGE, PTE_V | PTE_R)) < 0) {
			writef("cannot alloc in usr_load_icode_mapper\n");
			return r;
		}

		if (i == 0) {
			user_bcopy((void *)bin, TMPPAGE + offset, ((BY2PG-offset) < bin_size)? (BY2PG-offset) : (bin_size));
		}
		else {
			user_bcopy((void *)bin+i-offset, TMPPAGE, (BY2PG < (bin_size-i))? BY2PG : (bin_size-i));
		}

		if ((r = syscall_mem_map(0, TMPPAGE, child_envid, va+i, PTE_V | PTE_R)) < 0) {
			writef("cannot mem_map in usr_load_icode_mapper\n");
			return r;
		}
		if ((r = syscall_mem_unmap(0, TMPPAGE)) < 0) {
			writef("cannot unmap in usr_load_icode_mapper\n");
			return r;
		}	
	}
	//Step 2: alloc pages to reach `sgsize` when `bin_size` < `sgsize`.
	//  i has the value of `bin_size` now. 
	while (i < sgsize) {
		if ((r = syscall_mem_alloc(0, TMPPAGE, PTE_V | PTE_R)) < 0) {
			writef("cannot alloc in usr_load_icode_mapper\n");
			return r;
		}
		if ((r = syscall_mem_map(0, TMPPAGE, child_envid, va+i, PTE_V | PTE_R)) < 0) {
			writef("cannot map in usr_load_icode_mapper\n");
			return r;
		}
		if ((r = syscall_mem_unmap(0, TMPPAGE)) < 0) {
			writef("cannot unmap in usr_load_icode_mapper\n");
			return r;
		}	

		i+=BY2PG;
	}
	return 0;
}
int spawn(char *prog, char **argv)
{
	u_char elfbuf[512];
	int r;
	int fdnum;
	u_int child_envid;
	int size, text_start;
	u_int i, *blk;
	u_int esp;
	Elf32_Ehdr* elf;
	Elf32_Phdr* ph;
	// Note 0: some variable may be not used,you can cancel them as you like
	// Step 1: Open the file specified by `prog` (prog is the path of the program)
	if((r=open(prog, O_RDONLY))<0){
		user_panic("spawn ::open line 102 RDONLY wrong !\n");
		return r;
	}
	fdnum = r;
	if (debug) {
		writef("success open the prog:%s the fdnum is %d\n",prog,fdnum);
	}
	// Your code begins here
	// Before Step 2 , You had better check the "target" spawned is a execute bin 
	//pre-check
	if (!(prog[strlen(prog)-1] == 'b' && prog[strlen(prog)-2] == '.')) {
		writef("strlen is %d strlen is %c strlen -1 is %c; strlen -2 is %c; strlen - 3 is %c\n",strlen(prog),prog[strlen(prog)],prog[strlen(prog-1)],prog[strlen(prog-2)],prog[strlen(prog-3)]);
		writef("0:%c1:%c2:%c3:%c4:%c5:%c\n",prog[0],prog[1],prog[2],prog[3],prog[4],prog[5]);
		user_panic("%s is not an executable file\n",prog);
		return -E_INVAL;
	}
	// Step 2: Allocate an env (Hint: using syscall_env_alloc())
	if ((r = syscall_env_alloc()) < 0) {
		user_panic("cannot create child env in spawn!\n");
		return r;
	}
	child_envid = r;
	if (debug) {
		writef("success alloc an env for child, envid is %d\n",child_envid);
	}
	// Step 3: Using init_stack(...) to initialize the stack of the allocated env
	if ((r = init_stack(child_envid, argv, &esp)) < 0) {
		user_panic("spawn init_stack failed\n");
		return r;
	}
	if (debug) {
		writef("success init the stack for child\n");
	}
	// Step 3: Map file's content to new env's text segment
	//        Hint 1: what is the offset of the text segment in file? try to use objdump to find out.
	//        Hint 2: using read_map(...)
	//		  Hint 3: Important!!! sometimes ,its not safe to use read_map ,guess why 
	//				  If you understand, you can achieve the "load APP" with any method
	// Note1: Step 1 and 2 need sanity check. In other words, you should check whether
	//       the file is opened successfully, and env is allocated successfully.
	// Note2: You can achieve this func in any way ，remember to ensure the correctness
	//        Maybe you can review lab3 
	/*	text_start = 0x00400000;
		size = ((struct Filefd*)(num2fd(fdnum)))->f_file.f_size;
		for (i=0x1000;i<size;i+=BY2PG) {
		if ((r = read_map(fdnum, i, &blk)) < 0) {
		user_panic("error in read_map in spawn\n");
		return r;
		}
		syscall_mem_map(0, blk, child_envid, text_start+i, PTE_V|PTE_R);
		}*/
	size = ((struct Filefd*)(num2fd(fdnum)))->f_file.f_size;
	if ((r = usr_load_elf(num2fd(fdnum),child_envid)) < 0) {
		writef("error in spawn usr_load_elf\n");
		return r;
	}
	if (debug) {
		writef("reached finish mapping in spawn.c\n");
	}
	// Your code ends here

	struct Trapframe *tf;
	writef("\n::::::::::spawn size : %x  sp : %x::::::::\n",size,esp);
	tf = &(envs[ENVX(child_envid)].env_tf);
	tf->pc = UTEXT;
	tf->regs[29]=esp;


	// Share memory
	u_int pdeno = 0;
	u_int pteno = 0;
	u_int pn = 0;
	u_int va = 0;
	for(pdeno = 0;pdeno<PDX(UTOP);pdeno++)
	{
		if(!((* vpd)[pdeno]&PTE_V))
			continue;
		for(pteno = 0;pteno<=PTX(~0);pteno++)
		{
			pn = (pdeno<<10)+pteno;
			if(((* vpt)[pn]&PTE_V)&&((* vpt)[pn]&PTE_LIBRARY))
			{
				va = pn*BY2PG;

				if((r = syscall_mem_map(0,va,child_envid,va,(PTE_V|PTE_R|PTE_LIBRARY)))<0)
				{

					writef("va: %x   child_envid: %x   \n",va,child_envid);
					user_panic("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
					return r;
				}
			}
		}
	}


	if((r = syscall_set_env_status(child_envid, ENV_RUNNABLE)) < 0)
	{
		writef("set child runnable is wrong\n");
		return r;
	}
	return child_envid;		

}

	int
spawnl(char *prog, char *args, ...)
{
	return spawn(prog, &args);
}

