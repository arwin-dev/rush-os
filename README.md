# Rapid Unix SHell (rush)

## Introduction

`rush` (Rapid Unix SHell) is a simple, efficient Unix shell implemented as a command line interpreter (CLI). It repeatedly prints a prompt `rush> `, parses the input, executes the specified command, and waits for the command to finish. This cycle continues until the user types `exit`.

## Features

- Basic shell functionality: prompt, parse input, execute commands.
- Path handling to locate executables.
- Built-in commands: `exit`, `cd`, and `path`.
- Output redirection.
- Parallel command execution.
- Robust error handling and whitespace management.

## Program Specification

### Basic Shell

The shell operates in a loop, performing the following steps:
1. Print the prompt `rush> `.
2. Read the input command.
3. Parse the command and its arguments.
4. Execute the command.
5. Wait for the command to complete.
6. Repeat until the user types `exit`.

### Execution of Commands

Commands are executed by creating a new process using `fork()` and `execv()`. If `fork()` fails, an error is reported. The shell waits for the command to finish using `wait()` or `waitpid()`.

### Paths

The shell searches for executables in directories specified by the `path` variable. Initially, the path contains one directory: `/bin`. The shell checks if a command is executable using the `access()` system call. If a command is not found in any specified directory, an error is reported.

### Built-in Commands

- `exit`: Exits the shell. No arguments are allowed.
- `cd <directory>`: Changes the current directory to `<directory>`. If `cd` fails, an error is reported. It must take exactly one argument.
- `path [<dir1> <dir2> ...]`: Sets the search path for executables. If no arguments are provided, the path is cleared.

### Redirection

The shell supports output redirection using the `>` symbol. For example, `ls -la /tmp > output` redirects the output of `ls` to the file `output`, overwriting its content if it exists. Errors in opening the file are reported.

### Parallel Commands

The shell can execute multiple commands in parallel using the `&` symbol. For example, `cmd1 & cmd2 args1 args2 & cmd3 args1` runs `cmd1`, `cmd2`, and `cmd3` in parallel. The shell waits for all commands to complete before returning control to the user.

### Error Handling

The shell prints a single error message `An error has occurred\n` to `stderr` whenever an error is encountered. The shell continues processing after most errors, except when invoked with arguments, in which case it exits with status 1.

### Whitespace Management

The shell handles various kinds of whitespace, including spaces and tabs, ensuring that commands and arguments are correctly parsed regardless of whitespace variations.


## Usage

1. Compile the program:
    ```
    make
    ```
2. Run the compiled program:
    ```
    ./rush
    ```
