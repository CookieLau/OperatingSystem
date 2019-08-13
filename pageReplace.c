//Using CAR Algorithm

#include "pageReplace.h"
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#define PAGE 64
#define PAGE2 128
#define PPAGE 1048577
#define none 0
#define inT1 1
#define inT2 2
#define inB1 3
#define inB2 4
#define getPage(x) (x>>12)
#define max(a,b) (((a) < (b))? b:a)
#define min(a,b) (((a) < (b))? a:b)
#define assert(x) (x!=NULL)
#define null NULL

typedef struct node node;
struct node{
	int pagenum;//pageNumber
	int refer;//refer bit -- 0 or 1
};

typedef struct binode binode;
struct binode{
	int pagenum;
	binode *prev;
	binode *next;
};

binode* create(int pagenum){
	binode* temp;
	temp = (binode*)malloc(sizeof(binode));
	temp->pagenum = pagenum;
	temp->prev = NULL;
	temp->next = NULL;
	return temp;
}

void pageReplace(long *physic_memory, long newAddr) {
	static int state[PPAGE]; //mark the state of each physical page, inT1,T2,B1 or B2
	static int p = 0;//threahold of T1Size
	static int freeList[PAGE];
	static int ffront,ftail;//f(reeList)front, f(reeList)tail
	static node T1Circle[PAGE];
	static int T1Size,T1head,T1tail;
	static node T2Circle[PAGE];
	static int T2Size,T2head,T2tail;
	//static int B1Queue[PAGE];//Queue since we determine the Least and Most using Time
	//static int B1front,B1tail,B1Size;
	static binode* B1head;
	static binode* B1tail;
	static binode* B2head;
	static binode* B2tail;
	static int B1Size,B2Size;
	static binode* index2Bnode[PPAGE];//every Page has a binode* which come to effect when delete Bnode in BQueue
	//static int B2Queue[PAGE2];//LargerSize
	//static int B2front,B2tail,B2Size;
	static int init = 0;
    static char position[PPAGE];
	static char index2T[PPAGE];//convert Page to position in T

	int i;
	if (!init) {
		i = 0;
		p = 0;
		B1head = create(-1);
		B2head = create(-1);
		B1tail = B1head;
		B2tail = B2head;
//		T1Size = 0; T1head = 0; T1tail = 0;//for easy coding,we make when T1head == T1tail, it's end
//		T2Size = 0; T2head = 0; T2tail = 0;//constant invariant tail-head == size;
//		B1front = 0; B1tail = 0; B2Size = 0;//Btail is readTail+1 which has no element
//		B2front = 0; B2tail = 0; B2Size = 0;//Bhead is readHead which has element
//		ffront = 0; ftail = 0;//tail has no element
		for (i=0;i<PAGE;i++) {
			freeList[ftail] = i;
            ftail = (ftail + 1) % PAGE;
		}
		init = 1;
	}
	int pagenum = getPage(newAddr);
	if (state[pagenum] == inT1) {
		T1Circle[(int)index2T[pagenum]].refer = 1;
	} else if (state[pagenum] == inT2) {
		T2Circle[(int)index2T[pagenum]].refer = 1;
	} else {
		if (T1Size + T2Size == PAGE) { //full situation -- find a worst one
			int found = 0;
			int temppage;
			do {
				if (T1Size >= max(1,p)) {
					temppage = T1Circle[T1head].pagenum;
					if (T1Circle[T1head].refer == 0) {
						//move T1 to B1
						found = 1;
						//B1Queue[B1tail] = temppage;
						B1tail->next = create(temppage);
						B1tail->next->prev = B1tail;
						B1tail = B1tail->next;
						index2Bnode[B1tail->pagenum] = B1tail;
						//B1tail = (B1tail+1) % PAGE;
						B1Size++;
						state[temppage] = inB1;
						T1head = (T1head+1) % PAGE;
						T1Size--;
						freeList[ftail] = position[temppage];
						ftail = (ftail+1) % PAGE;
						position[temppage] = -1;
					} else {
						//move T1 to T2
						T1Circle[T1head].refer = 0;
						T2Circle[T2tail].pagenum = temppage;
						index2T[temppage] = T2tail;
						T2Circle[T2tail].refer = 0;
						state[temppage] = inT2;
						T1head = (T1head+1) % PAGE;
						T1Size--;
						T2tail = (T2tail+1) % PAGE;
						T2Size++;
					}
				} else {
					temppage = T2Circle[T2head].pagenum;
					if (T2Circle[T2head].refer == 0) {
						//move T2 to B2
						found = 1;
						//B2Queue[B2tail] = temppage;
						B2tail->next = create(temppage);
						B2tail->next->prev = B2tail;
						B2tail = B2tail->next;
						index2Bnode[B2tail->pagenum] = B2tail;
						//B2tail = (B2tail+1) % (PAGE2);
						B2Size++;
						T2head = (T2head+1) % PAGE;
						T2Size--;
						state[temppage] = inB2;
						freeList[ftail] = position[temppage];
						ftail = (ftail+1) % PAGE;
						position[temppage] = -1;
					} else {
						//move T2head to T2tail
						T2Circle[T2head].refer = 0;
						T2Circle[T2tail].pagenum = temppage;
						index2T[temppage] = T2tail;
						T2Circle[T2tail].refer = 0;
						T2head = (T2head+1) % PAGE;
						T2tail = (T2tail+1) % PAGE;
					}
				}
			} while (!found);
			if (state[pagenum] != inB1 && state[pagenum] != inB2) {
				if (T1Size + B1Size == PAGE && B1Size > 0) {
					//state[B1Queue[B1front]] = none;
					//B1front = (B1front+1) % PAGE;
					state[B1head->next->pagenum] = none;
					B1head = B1head->next;
					index2Bnode[B1head->pagenum] = null;
					free(B1head->prev);
					B1head->prev = NULL;
					B1Size--;
				} else if (T1Size + T2Size + B1Size + B2Size == PAGE2 && B2Size > 0) {
					//state[B2Queue[B2front]] = none;
					//B2front = (B2front+1) % PAGE2;
					state[B2head->next->pagenum] = none;
					B2head = B2head->next;
					index2Bnode[B2head->pagenum] = null;
					free(B2head->prev);
					B2head->prev = NULL;
					B2Size--;
				}
			}
		}
		if (state[pagenum] != inB1 && state[pagenum] != inB2) {
			T1Circle[T1tail].pagenum = pagenum;
			index2T[pagenum] = T1tail; 
			T1Circle[T1tail].refer = 0;
			state[pagenum] = inT1;
			T1tail = (T1tail+1) % PAGE;
			T1Size++;
		} else if (state[pagenum] == inB1) {
			p = min(PAGE,(p+max(1,(B2Size/B1Size))));
			//how to move from B1 to T2
			binode* tt;
			tt = index2Bnode[pagenum];
			if (!assert(tt))
				while(1)
					printf("wwwwwwwwwwwwwwwwwwwwwwwwwwwwww\n");
			tt->prev->next = tt->next;
			if (B1tail==tt) {
				B1tail = tt->prev;
			} else {
				tt->next->prev = tt->prev;
			}
			free(tt);
			index2Bnode[pagenum] = null;
            B1Size--;
			T2Circle[T2tail].pagenum = pagenum;
			index2T[pagenum] = T2tail;
			T2Circle[T2tail].refer = 0;
			T2tail = (T2tail+1) % PAGE;
			T2Size++;
			state[pagenum] = inT2;
		} else { //inB2
			p = max(0,(p-max(1,(B1Size/B2Size))));
			//how to move from B2 to T2
            binode* tt;
			tt = index2Bnode[pagenum];
			if (!assert(tt))
				while(1)
					printf("wwwwwwwwwwwwwwwwwwwwwwwwwwwwww\n");
			tt->prev->next = tt->next;
			if (B2tail==tt) {
				B2tail = tt->prev;
			} else {
				tt->next->prev = tt->prev;
			}
			free(tt);
			index2Bnode[pagenum] = null;
            B2Size--;
			state[pagenum] = inT2;
			T2Circle[T2tail].pagenum = pagenum;
			index2T[pagenum] = T2tail;
			T2Circle[T2tail].refer = 0;
			T2tail = (T2tail+1) % PAGE;
			T2Size++;
		}
		physic_memory[freeList[ffront]] = pagenum;
        position[pagenum] = freeList[ffront];
		ffront = (ffront+1) % PAGE;
	}
}
