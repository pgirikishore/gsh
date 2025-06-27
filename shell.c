#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

#define GSH_RL_BUFSIZE 1024
#define GSH_TOK_BUFSIZE 64
#define GSH_TOK_DELIM " \t\r\n\a"

/*
  Function Declarations for builtin shell commands:
 */
int gsh_cd(char **args);
int gsh_help(char **args);
int gsh_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &gsh_cd,
  &gsh_help,
  &gsh_exit
};

int gsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/
int gsh_cd(char **args)
{
  if (args[1] == NULL) {
    // cd with no arguments -> go to $HOME
    char *home = getenv("HOME");
    if (home == NULL) {
      fprintf(stderr, "gsh: HOME not set\n");
    } else if (chdir(home) != 0) {
      perror("gsh");
    }
  } 
  else {
    if (strcmp(args[1], "..") == 0) {
      char *cwd = getcwd(NULL, 0);
      strcpy(args[1], dirname(cwd));
      free(cwd);
      
    }
    if (chdir(args[1]) != 0) {
      perror("gsh");
    }
  }
  return 1;
}

int gsh_help(char **args)
{
  int i;
  printf("Giri Kishore's shell\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("Built-in progams are below:\n");

  for (i = 0; i < gsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int gsh_exit(char **args)
{
  return 0;
}

char *gsh_read_line(void)
{
  int bufsize = GSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "gsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    // If we hit EOF, replace it with a null character and return.
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } 
    else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += GSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "gsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

char **gsh_split_line(char *line)
{
  int bufsize = GSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "gsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, GSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += GSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "gsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, GSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

int gsh_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("gsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("gsh");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int gsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < gsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return gsh_launch(args);
}


void gsh_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("%s > ", getcwd(NULL, 0));
    line = gsh_read_line();
    args = gsh_split_line(line);
    status = gsh_execute(args);

    free(line);
    free(args);
  } while (status);
}


int main(int argc, char **argv)
{
  // Run command loop.
  gsh_loop();

  return EXIT_SUCCESS;
}