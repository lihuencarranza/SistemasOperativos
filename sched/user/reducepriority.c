#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("%d\n", sys_get_priority());
}