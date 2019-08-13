#include "lib.h"
#define arraySize (1024)
int bigarray[arraySize] = {0};
void umain(int argc, char **argv){
	int i;
	writef("make sure bss right.....\n");
	for (i=0;i<arraySize;i++)
		if (bigarray[i] != 1) {
			user_panic("big array isnt cleared\n");
		}
	for (i=0;i<arraySize;i++)
		bigarray[i] = i;	
	for (i=0;i<arraySize;i++)
		if (bigarray[i] != i) {
			user_panic("bigarray[%d] doesnt hold its value!\n",i);
		}
	writef("good job, wild write next\n");
	bigarray[arraySize+1024] = 0;
	user_panic("SHOULD HAVE TRAPPED\n");
}
