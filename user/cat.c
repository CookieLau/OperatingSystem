#include "lib.h"

char buf[8192];
#define debug 0
void
cat(int f, char *s)
{
	long n;
	int r;

	while((n=read(f, buf, (long)sizeof buf))>0) {
		if (debug) 
			writef("enter cat loop, buf is ^%s$\n",buf);
		if((r=write(1, buf, n))!=n) {
			writef("r is %d n is %d\n",r,n);
			user_panic("write error copying %s: %d", s, r);
		}
	}
	if(n < 0)
		user_panic("error reading %s: %d", s, n);
}

void
umain(int argc, char **argv)
{
	int f, i;

	if(argc == 1)
		cat(0, "<stdin>");
	else for(i=1; i<argc; i++){
		if (debug)
			writef("opening %s in cat.c\n",argv[i]);
		f = open(argv[i], O_RDONLY);
		if(f < 0)
			user_panic("can't open %s: %e", argv[i], f);
			//close(f);
		else{
			cat(f, argv[i]);
			close(f);
		}
	}
}

