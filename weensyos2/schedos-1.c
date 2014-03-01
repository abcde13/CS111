#include "schedos-app.h"
#include "x86sync.h"

#define EX8
/*****************************************************************************
 * schedos-1
 *
 *   This tiny application prints red "1"s to the console.
 *   It yields the CPU to the kernel after each "1" using the sys_yield()
 *   system call.  This lets the kernel (schedos-kern.c) pick another
 *   application to run, if it wants.
 *
 *   The other schedos-* processes simply #include this file after defining
 *   PRINTCHAR appropriately.
 *
 *****************************************************************************/

#ifndef PRINTCHAR
#define PRINTCHAR	('1' | 0x0C00)
#endif


void
start(void)
{
	int i;

	//sys_set_share(10);
	//sys_set_priority(-1);
	for (i = 0; i < RUNCOUNT; i++) {
		// Write characters to the console, yielding after each one.
		//*cursorpos++ = PRINTCHAR;
#ifdef EX8
		while(atomic_swap(&lock,1) != 0)
			continue;
		*cursorpos++ = PRINTCHAR;
		atomic_swap(&lock,0);
#endif
#ifndef EX8

		sys_print_char(PRINTCHAR);
#endif
		sys_yield();
	}
	// Yield forever.
	while (1)
		sys_exit(1);
}
