#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include "shell.h"

/* Global declartion function */
char *history[HISTORY_SIZE];
int history_head = 0;    // where to store next
int history_size = 0;    // number of commands stored (max = HISTORY_SIZE)
int history_index = -1;  // for arrow key navigation

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
  printf("Giri Kishore's shell \n");

  if (sizeof(args) > 8 ) {
    printf("help command does not support arguments\n");
  }

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
  if (sizeof(args) > 8) {
    printf("exit command does not support arguments\n");
  }
  return 0;
}

char *gsh_read_line(void)
{
    int bufsize = GSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    struct termios original_term;
    enable_raw_mode(&original_term);

    if (!buffer) {
        fprintf(stderr, "gsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
      c = getchar();
      if (c == '\x1b') { // ESC
        char seq[2];
        seq[0] = getchar();
        seq[1] = getchar();

        if (seq[0] == '[') {
          if (seq[1] == 'A') {
            if (history_size > 0) {
              if (history_index < history_size - 1) {
                history_index++;
              }
              
              int idx = (history_head + HISTORY_SIZE - 1 - history_index) % HISTORY_SIZE;
              strcpy(buffer, history[idx]);
              position = strlen(buffer);
                
              // Clear line and reprint prompt + buffer
              printf("\33[2K\r%s > %s", getcwd(NULL, 0), buffer);
            }
          } 
          else if (seq[1] == 'B') {
            if (history_index > 0) {
              history_index--;
              int idx = (history_head + HISTORY_SIZE - 1 - history_index) % HISTORY_SIZE;
              strcpy(buffer, history[idx]);
              position = strlen(buffer);
            } 
            else {
              // No more recent command — clear the buffer
              history_index = -1;
              buffer[0] = '\0';
              position = 0;
            }
            
            // Clear line and print prompt + buffer
            printf("\33[2K\r%s > %s", getcwd(NULL, 0), buffer);
            fflush(stdout);
          }
        }
        continue;
      }

      if (c == '\n') {
        if (strcmp(buffer, "\n") != 0) {
          if (history[history_head]) {
            free(history[history_head]);
          }
          history[history_head] = strdup(buffer);
          history_head = (history_head + 1) % HISTORY_SIZE;
        
          if (history_size < HISTORY_SIZE) {
            history_size++;
          }
        }
            
        buffer[position] = '\0';
        printf("\n");
        break;
      } 
      else if (c == 127 || c == '\b') {  // handle backspace
        if (position > 0) {
          position--;
          printf("\b \b");
        }
      } 
      else {
        buffer[position++] = c;
        putchar(c);
      }

      if (position >= bufsize) {
        bufsize += GSH_RL_BUFSIZE;
        buffer = realloc(buffer, bufsize);
        if (!buffer) {
          fprintf(stderr, "gsh: allocation error\n");
          exit(EXIT_FAILURE);
        }
      }
      
    }

    disable_raw_mode(&original_term);
    return buffer;
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
  pid_t pid;
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
      waitpid(pid, &status, WUNTRACED);
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


int main()
{

  // Run command loop.
  gsh_loop();

  return EXIT_SUCCESS;
}