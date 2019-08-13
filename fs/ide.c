/*
 * operations on IDE disk.
 */

#include "fs.h"
#include "lib.h"
#include <mmu.h>
// Overview:
// 	read data from IDE disk. First issue a read request through
// 	disk register and then copy data from disk buffer
// 	(512 bytes, a sector) to destination array.
//
// Parameters:
//	diskno: disk number.
// 	secno: start sector number.
// 	dst: destination for data read from IDE disk.
// 	nsecs: the number of sectors to read.
//
// Post-Condition:
// 	If error occurred during read the IDE disk, panic. 
// 	
// Hint: use syscalls to access device registers and buffers
	void
ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs)
{
	// 0x200: the size of a sector: 512 bytes.
	int offset_begin = secno * 0x200;
	int offset_end = offset_begin + nsecs * 0x200;
	int offset = 0;
	char ret;
	char option = 0x0;
	u_int buff = 0x13000000 + 0x4000;
	while (offset_begin + offset < offset_end) {
		// Your code here
		u_int va = offset_begin+offset;
		syscall_write_dev(&va, 	   0x13000000, 4);
		syscall_write_dev(&option, 0x13000020, 1);
		syscall_read_dev (&ret,    0x13000030, 1);
		if (ret == 0) {
			user_panic("wrong in ide_read\n");
			return ;
		}
		if ((ret = syscall_read_dev(dst+offset,buff,0x200)) < 0) {
			user_panic("error in ide_read\n");
			return ;
		}
		offset += 0x200;
	}
}


// Overview:
// 	write data to IDE disk.
//
// Parameters:
//	diskno: disk number.
//	secno: start sector number.
// 	src: the source data to write into IDE disk.
//	nsecs: the number of sectors to write.
//
// Post-Condition:
//	If error occurred during read the IDE disk, panic.
//	
// Hint: use syscalls to access device registers and buffers
	void
ide_write(u_int diskno, u_int secno, void *src, u_int nsecs)
{
	//Your code here
	int offset_begin = secno * 0x200;
	int offset_end = offset_begin + nsecs * 0x200;
	int offset = 0;
	char ret;
	char option = 1;
	u_int buff = 0x13000000+0x4000;
	writef("diskno: %d\n", diskno);
	while (offset_begin+offset < offset_end) {
		// copy data from source array to disk buffer.
		if ((ret = syscall_write_dev(src+offset,buff,0x200)) < 0 ) {
			user_panic("wrong in ide_write\n");
			return ;
		}
		u_int va = offset_begin+offset;
		syscall_write_dev(&diskno, 0x13000010, 4);
		syscall_write_dev(&va, 	   0x13000000, 4);
		syscall_write_dev(&option, 0x13000020, 1);
		syscall_read_dev (&ret,    0x13000030, 1);
		if (ret == 0) {
			user_panic("wrong in ide_write\n");
			return ;
		}		
		offset += 0x200;
		// if error occur, then panic.
	}
}

