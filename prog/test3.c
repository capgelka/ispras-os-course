#include <inc/lib.h>

void (* volatile sys_yield)(void);
int (*volatile cprintf) (const char *fmt, ...);

void
umain( int argc, char **argv )
{
	int i, j;
	i = 0;
	sys_yield();
	i++;

	i++;
	sys_yield();
	cprintf("start main\n");
	for( j = 0; j < 3; ++j ) {
		cprintf("%d %d\n", i, j);
		for( i = 0; i < 10000; ++i ) {}
		sys_yield();
	}
}

