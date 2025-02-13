#include "builtin.h"

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int exit_shell(char *cmd) {
  if (strcmp(cmd, "exit") == 0)
    return 1;

  return 0;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int cd(char *cmd) {
  if (cmd[0] == 'c' && cmd[1] == 'd') {
    strtok(cmd, " ");

    char *dir = strtok(NULL, " ");
    if (dir == NULL) {
      char *home = getenv("HOME");
      if (chdir(home) < 0) {
        perror("Error in directory HOME\n");
        return 1;
      }
    } else {
      if (chdir(dir) < 0) {
        perror("Error in directory\n");
        return 1;
      }
    }
    return 1;
  }
  return 0;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int pwd(char *cmd) {
  if (!strcmp(cmd, "pwd")) {
    char *path = getcwd(NULL, 100);
    if (path == NULL) {
      return 1;
    }
  }
  return 0;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int history(char *cmd) {
  if (strcmp(cmd, "history") != 0)
    return 0;

  FILE *f;
  char buffer[BUFLEN];

  f = fopen(".sh_history", "r");
  if (!f)
    return 1;
  int i = 0;
  while (fgets(buffer, sizeof(buffer), f)) {
    fprintf(stdout, "%d  %s", i, buffer);
    i++;
  }

  fclose(f);
  return 1;
}
