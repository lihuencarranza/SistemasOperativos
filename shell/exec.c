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
	// printf("Key: %s\n", eargv[0]);
	// printf("Value: %s\n", eargv[1]);
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
	int fd = open(file, flags, S_IWUSR | S_IRUSR);
	return fd;
}

// private function to encapsule the redirection
void
redir_stdout(char *file)
{
	int fd = open_redir_fd(file, O_CREAT | O_WRONLY | O_TRUNC);
	int fd2 = dup2(fd, STDOUT_FILENO); //Cambio el fd 1 para que apunte al nuevo
	close(fd);
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
		//
		// Your code here
		e = (struct execcmd *) cmd;
		printf("Llego a ejecutar un comando EXEC\n");
		printf("e->argv[0] es %s, y e->argv es %s\n",
		       e->argv[0],
		       e->argv[0]);
		int i = 0;
		while (e->argv[i]) {
			printf("e->argv[0] es %s, y e->argv[%i] es %s\n",
			       e->argv[0],
			       i,
			       e->argv[i]);
			i++;
		}


		set_environ_vars(e->eargv, e->eargc);

		if (execvp(e->argv[0], e->argv) < 0) {
			perror("Error");
		}

		break;

	case BACK: {
		// runs a command in background
		b = (struct backcmd *) cmd;
		printf("Llego a ejecutar un comando BACK\n");

		exec_cmd(b->c);
		int pid = fork();
		if (pid == 0) {
			// hijo
			exec_cmd(b->c);
		}
		perror("Error in background process\n");


		break;
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
	 	if(strlen(r->out_file) > 0){ //Caso redir output
			//habria que forkear aca? (TODO)
			redir_stdout(r->out_file);
			r->type = EXEC; //Le cambio el type porque ya cambie el fd
			exec_cmd(r);
		}
		
		printf("Redirections are not yet implemented\n");
		_exit(-1);
		break;
	}

	case PIPE: {
		// pipes two commands
		//
		// Your code here
		printf("Pipes are not yet implemented\n");

		// free the memory allocated
		// for the pipe tree structure
		free_command(parsed_pipe);

		break;
	}
	}
}
