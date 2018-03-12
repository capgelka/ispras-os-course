#include <inc/lib.h>

void (* volatile sys_yield)(void);
int (*volatile cprintf) (const char *fmt, ...);


void
umain( int argc, char **argv )
{
	int i, j;
	cprintf("%c", "TESTST 2");
	sys_exit();
 	cprintf("%c", "TESTST");
	for( j = 0; j < 3; ++j ) {
		for( i = 0; i < 10000; ++i ) {}
		sys_exit();
	}
}

