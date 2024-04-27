#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"

void sigchld_handler();

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

void
sigchld_handler()
{
	pid_t pid;
	int status;
	
	while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
		print_status_info(parsed_pipe);
	}
}

// sets the signal handlers
static void 
set_signal_handlers(){
	// register signal
	struct sigaction sa = { .sa_handler = sigchld_handler,
		                .sa_flags = SA_RESTART };

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
