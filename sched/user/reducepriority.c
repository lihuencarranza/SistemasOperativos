#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	// Reduce the priority of the current environment
	int p = sys_get_priority();
	cprintf("Current priority: %d\n", p);
	int j = p - 1;
	cprintf("Trying to reduce priority to: %d\n", j);
	int r = sys_set_priority(j);
	if (r < p || r == 0) {
		cprintf("Priority reduced to: %d\n", r);
	} else {
		cprintf("Priority not reduced\n");
	}
}
