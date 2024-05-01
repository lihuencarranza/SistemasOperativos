#include "exec.h"

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	char key[BUFLEN];
	char value[BUFLEN];
	for (int i = 0; i < eargc; i++) {
		get_environ_key(eargv[i], key);
		get_environ_value(eargv[i], value, block_contains(eargv[i], '='));
		setenv(key, value, 1);
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	// Your code here
	int fd;
	if (flags & O_CREAT) {
		fd = open(file, flags, S_IWUSR | S_IRUSR);
	} else {
		fd = open(file, flags);
	}
	if (fd < 0) {
		perror("Error opening file");
		exit(-1);
	}
	return fd;
}

// private function to encapsule the redirection of stdin
static void
redir_stdin(char *file)
{
	int fd = open_redir_fd(file, O_RDONLY);
	int fd2 = dup2(fd, STDIN_FILENO);  // Cambio el fd 0 para que apunte al nuevo
	close(fd);
}

// private function to encapsule the redirection of stdout
static void
redir_stdout(char *file)
{
	int fd = open_redir_fd(file, O_CREAT | O_WRONLY | O_TRUNC);
	int fd2 = dup2(fd, STDOUT_FILENO);  // Cambio el fd 1 para que apunte al nuevo
	close(fd);
}

// private function to encapsule the redirection of stderr
static void
redir_stderr(char *file)
{
	int fd = open_redir_fd(file, O_CREAT | O_WRONLY);
	int fd2 = dup2(fd, STDERR_FILENO);  // Cambio el fd 2 para que apunte al nuevo
	close(fd);
}

static void
redir_strerr_to_stdout()
{
	dup2(STDOUT_FILENO, STDERR_FILENO);
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC:
		// spawns a command
		e = (struct execcmd *) cmd;
		if (strstr(e->scmd, "env") != NULL) {
			set_environ_vars(e->eargv, e->eargc);
		}
		execvp(e->argv[0], e->argv);
		exit(-1);

	case BACK: {
		// runs a command in background
		b = (struct backcmd *) cmd;
		exec_cmd(b->c);
		exit(-1);
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		//
		// Your code here
		r = (struct execcmd *) cmd;
		if (strlen(r->out_file) > 0) {  // Caso redir output
			redir_stdout(r->out_file);
		}
		if (strlen(r->in_file) > 0) {  // Caso redir input
			redir_stdin(r->in_file);
		}
		if (strlen(r->err_file) > 0) {  // Caso redir error
			if (strcmp(r->err_file, "&1") == 0) {  // Caso combinando
				redir_strerr_to_stdout();
			} else {
				redir_stderr(r->err_file);
			}
		}

		r->type = EXEC;  // Le cambio el type porque ya cambie el fd
		exec_cmd(r);

		printf("Redirections are not yet implemented\n");
		_exit(-1);
		break;
	}

	case PIPE: {
		// pipes two commands

		p = (struct pipecmd *) cmd;
		int fd[2];

		if (pipe(fd) < 0) {
			perror("Error creating pipe\n");
			exit(-1);
		}

		int pid_left = fork();
		if (pid_left < 0) {
			perror("Error in fork\n");
			close(fd[READ]);
			close(fd[WRITE]);
			exit(-1);
		}

		if (pid_left == 0) {
			close(fd[READ]);
			dup2(fd[WRITE], STDOUT_FILENO);
			close(fd[WRITE]);
			exec_cmd(p->leftcmd);
		}

		int pid_right = fork();
		if (pid_right < 0) {
			perror("Error in fork\n");
			close(fd[READ]);
			close(fd[WRITE]);
			exit(-1);
		}

		if (pid_right == 0) {
			close(fd[WRITE]);
			dup2(fd[READ], STDIN_FILENO);
			close(fd[READ]);
			exec_cmd(p->rightcmd);
		}

		close(fd[READ]);
		close(fd[WRITE]);

		waitpid(pid_left, NULL, 0);
		waitpid(pid_right, NULL, 0);

		// free the memory allocated
		// for the pipe tree structure
		// free_command(parsed_pipe); //variable global

		break;
	}
	}
}
