#ifndef EXEC_H
#define EXEC_H

#include "defs.h"
#include "freecmd.h"
#include "types.h"
#include "utils.h"

extern struct cmd *parsed_pipe;

void exec_cmd(struct cmd *c);

#endif // EXEC_H
