#ifndef __SHELL_H__
#define __SHELL_H__

#include <termios.h>

#define GSH_RL_BUFSIZE 1024
#define GSH_TOK_BUFSIZE 64
#define GSH_TOK_DELIM " \t\r\n\a"
#define HISTORY_SIZE 100
#define MAX_CMD_LEN 1024

/*
  Function Declarations for builtin shell commands:
 */
int gsh_cd(char **args);
int gsh_help(char **args);
int gsh_exit(char **args);

void enable_raw_mode(struct termios *original);
void disable_raw_mode(struct termios *original);

#endif // __SHELL_H__