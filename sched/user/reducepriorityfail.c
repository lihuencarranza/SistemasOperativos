#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	// Reduce the priority of the current environment
	int p = sys_get_priority();
	int j = 10;
	// esto va a fallar, pues la prioridad maxima es 5
	cprintf("Trying to reduce priority from %d to %d\n", p, j);
	int r = sys_set_priority(j);
	if (r < p) {
		cprintf("Priority reduced to: %d\n", r);
	} else {
		cprintf("Priority not reduced because it was out of scope\n");
	}
}