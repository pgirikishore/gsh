# Simple Shell (gsh)

A minimal command-line shell written in C, inspired by Stephen Brennan’s [Write a Shell in C](https://brennan.io/2015/01/16/write-a-shell-in-c/) tutorial. This project serves as both a learning tool and a foundation for building more advanced shell features.

## Features Implemented

- Built-in commands:
  - `cd` – change directories (supports `cd`, `cd ..`)
  - `help` – display information about built-in commands
  - `exit` – exit the shell
- Executes external programs using `fork` and `execvp`
- Displays a prompt showing the current working directory
- Input is parsed into tokens using `strtok`
- Makefile included for easy compilation
- Command history with arrow key navigation

## Planned Features

- Line editing (backspace, cursor movement)
- Command chaining with `&&`, `||`, and `;`
- Input and output redirection (`<`, `>`, `>>`)
- Piping support (`|`)
- Background job execution with `&`
- Prompt formatting with support for `~`, username, and hostname
- Environment variable expansion (e.g., `$HOME`, `$PATH`)

## Build Instructions

To compile the shell:

```bash
make
```

## Run gsh

To run the shell

```bash
./gsh
```
