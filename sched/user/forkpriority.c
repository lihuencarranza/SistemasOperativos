#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	int f = fork();
	cprintf("Current priority: %d\n", sys_get_priority());
	sys_set_priority(3);
	cprintf("Current priority: %d\n", sys_get_priority());

	if (f == 0) {
		if (sys_get_priority() == 3) {
			cprintf("Child process has the same proprity as his "
			        "father\n");
		} else {
			cprintf("Child priority not reduced\n");
		}
	} else {
		cprintf("Parent reduced his priority\n");
		sys_set_priority(2);
	}
}
