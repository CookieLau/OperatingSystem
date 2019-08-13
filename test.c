//#include "pageReplace.h"
#include<unistd.h>
#include<stdio.h>
#include<math.h>
#define PAGE 64
#define PPAGE 1048576
#define PAGESIZE 12
#define getPage(x) (x>>PAGESIZE)
#define EMPTY -1
#define lambda 0.0001
#define base 0.5
#define weight(x) (pow(0.5,lambda*x))
#define factor 10000000
long long count = 0;
#define cost 3
void pageReplace(long* physic_memory, long newAddr) {
	//static int init = 0;
	static long times = -1;
	static double weights[PPAGE];
	static int last[PPAGE];
	static const double w0 = weight(0);
	int i,diff,flag;
	int site = -1;
	double lowest = 0.0;
	long long tempAddr = newAddr & 0xffffffff;
	long pagenum = getPage(tempAddr);
	printf("the new Addr is %lld, and pagenum is %ld\n",tempAddr,pagenum);
	//static variables do not need init
	/*if (!init) {
		for (i=0;i<PPAGE;i++) {
			printf("init page:%d\n w:%lf last:%d\n",i,weights[i],last[i]);
			weights[i] = 1;
			last[i] = 0;
			printf("init page:%d\n w:%lf last:%d\n",i,weights[i],last[i]);
		}
		init = 1;
	}*/
	times++;
	flag = 0;
	for (i=0;i<PAGE;i++) {
		if (physic_memory[i] == EMPTY) {//not found in previous but has empty location
			if (!flag) {
				diff = times ;//this never show before
				last[pagenum] = times;
				weights[pagenum] = weight(diff)*weights[pagenum] + w0;
				physic_memory[i] = pagenum;
			}
			return ;
		}
		diff = times - last[physic_memory[i]];
		weights[physic_memory[i]] = weight(diff)*weights[physic_memory[i]];
		last[physic_memory[i]] = times;
		if (physic_memory[i] == pagenum) {
			flag = 1;
			weights[pagenum] += w0;//last w0 is new update: F(times - times)
		}
		//printf("<<<<<<<<<<flag:%d site:%d lowest:%lf weights[physic_memory[i]]:%lf\n",flag,site,lowest,weights[physic_memory[i]]);
		if (!flag && (site == -1 || lowest > weights[physic_memory[i]])) {//update the least important
			site = i;
			lowest = weights[physic_memory[i]];
		}
	}
	//not found in all PAGE location and also no empty room
	if (!flag) {
		physic_memory[site] = pagenum;
		diff = times - last[pagenum];
		last[pagenum] = times;
		weights[pagenum] = weight(diff)*weights[pagenum] + w0;
	}
}

#define setAddr(x) x<<PAGESIZE
#define printAll(list) for(int i=0;i<PAGE;i++) {\
	printf("%ld%c",list[i],(i==PAGE-1)? '\n':' ');\
}
int main() {
	long phy[PAGE];
	long i,n;
	for (i=0;i<PAGE;i++) {
		phy[i] = -1;
		//printf("%u\n",&phy[i]);
	}
	printAll(phy);
	while (EOF!=scanf("%ld",&n))
	{
		//pageReplace(phy,setAddr(n));
		pageReplace(phy,(n));
		printAll(phy);
	}
	printf("%lld\n",count);
	return 0;
}


