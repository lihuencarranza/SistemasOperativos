#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"


char prompt[PRMTLEN] = { 0 };

// runs a shell command
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}

static void
sigchld_handler()
{
	pid_t pid;
	int status;

	char buf[BUFLEN] = { 0 };
	while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
		snprintf(buf, sizeof buf, "%s ===> terminado: PID=%d", COLOR_BLUE, pid);
	}
}

// sets the signal handlers
static void 
set_signal_handlers()
{
	// register signal
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));

	sa.sa_handler = sigchld_handler;
	sa.sa_flags = SA_RESTART;

	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	set_signal_handlers();

	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}

int
main(void)
{
	init_shell();

	run_shell();

	return 0;
}
