#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h> 

#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\n"

// Function to print error message
void error() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    fflush(stderr);
}

// Function to read a line of input from stdin
char *readLine() {
    char *input = NULL;
    size_t bufsize = 0;

    if (getline(&input, &bufsize, stdin) == -1) {
        error();
        exit(EXIT_FAILURE);
    }

    return input;
}

// Function to split a line into tokens based on a delimiter
char **splitLine(char *input, char *delim) {
    int bufSize = TOK_BUFSIZE;
    int i = 0;
    char **tokens = malloc(bufSize * sizeof(char *));
    char *token=NULL;

    if (!tokens) {
        error();
        exit(EXIT_FAILURE);
    }

    token = strtok(input, delim);
    while (token != NULL) {
        tokens[i] = strdup(token);
        i++;
        if (i >= bufSize) {
            bufSize += TOK_BUFSIZE;
            tokens = realloc(tokens, bufSize * sizeof(char *));
            if (!tokens) {
                error();
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, delim);
    }
    tokens[i] = NULL;
    return tokens;
}

// Function to generate a new path based on the input arguments
char **generatePath(char **args1) {
    int size = 0;
    while (args1[size] != NULL) {
        size++;
    }
    char **path = malloc(size * sizeof(char *));
    if (!path) {
        error();
        exit(EXIT_FAILURE);
    }
    for (int i = 1; args1[i] != NULL; i++) {
        path[i - 1] = strdup(args1[i]);
    }
    path[size - 1] = NULL;
    return path;
}

// Function to check if a command is valid by searching for it in the specified paths using the "access" function
char *checkIfValidCommand(char **args, char **path)
{
    char *final_full_path = NULL;
    for(int j = 0; path[j] != NULL; j++)
    {
        size_t full_path_len = strlen(path[j]) + strlen("/") + strlen(args[0]) + 1;
        char *full_path = (char *)malloc(full_path_len * sizeof(char));
        if (full_path == NULL) 
        {
            free(final_full_path);
            continue;
        }

        strcpy(full_path, path[j]);
        strcat(full_path, "/");
        strcat(full_path, args[0]);
        int checkIfFileExists = access(full_path, X_OK);
        if(checkIfFileExists == 0)
        {
            if(final_full_path != NULL) free(final_full_path);
            final_full_path = (char *)malloc(full_path_len * sizeof(char));
            strcpy(final_full_path, full_path);
            break;
        }
    }

    return final_full_path;
}

// Function to execute a command with the "execv" function
void execute(char **args, char **path)
{
    char *final_full_path = checkIfValidCommand(args, path);

    int redir_count = 0;
    int redir_index = -1;
    // checks if args has ">" and if it does, it checks if its valid command
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            redir_count++;
            redir_index = i;
            if (redir_count > 1 || args[i+1] == NULL || args[i+2] != NULL ) {
                error();
                exit(EXIT_FAILURE);
            }
        }
    }

    // handles redirection here 
    // args[redir_index + 1] -> this is the filename of the file you want to dump the data in
    if (redir_index != -1 && args[redir_index + 1] != NULL) {
        int file_desc = open(args[redir_index + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (file_desc == -1) {
            error();
            exit(EXIT_FAILURE);
        }
        dup2(file_desc, STDOUT_FILENO);
        close(file_desc);
        args[redir_index] = NULL;
        args[redir_index + 1] = NULL;
    }

    // executes the command 
    if(execv(final_full_path, args) == -1){
        error();
        exit(EXIT_FAILURE);
    }
}

// Main function
int main(int argc, char *argv[]) 
{
    if(argc > 1)
    {
        error();
        return EXIT_FAILURE;
    }
    
    char *input;
    char **args;
    char *initialPath[2];
    initialPath[0] = strdup("/bin");
    initialPath[1] = NULL;

    char **path = initialPath;

    while (1) {
        printf("rush> ");
        fflush(stdout);

        input = readLine();
        args = splitLine(input, "&");

        // Initialize counter for child processes
        int num_args = 0;

        for(int i = 0; args[i] != NULL; i++)
        {
            pid_t pid;
            char **command = NULL;
            command = splitLine(args[i], TOK_DELIM);

            // If the command is empty, skip it
            if(command[0] == NULL )
            {
                continue;
            }
            // Execute built-in commands (exit)
            else if (strcmp(command[0], "exit") == 0) 
            {
                if(command[1] == NULL)
                    exit(1);
                else
                    error();
            }
            // Execute built-in commands (cd)
            else if (strcmp(command[0], "cd") == 0) 
            {
                if (command[1] == NULL) {
                    error(); 
                } else {
                    if (chdir(command[1]) != 0) {
                        error(); 
                    }
                }
            }
            // Execute built-in commands (path)
            else if (strcmp(command[0], "path") == 0) 
            {
                path = generatePath(command);
            }
            else
            {
                //checks if the path is not NULL or if checks if the command is a valid executable in the path
                if(path[0] == NULL || checkIfValidCommand(command, path) == NULL)
                {
                    error();
                    continue;
                }
                // Fork a child process
                pid = fork();
                if (pid == -1)
                {
                    error();
                }
                else if(pid == 0)
                {
                    // Child process: execute the command
                    execute(command, path);
                }
                else if(pid < 0)
                {
                    error();
                }
                else
                {
                    // Parent process: increment child process counter
                    num_args++;
                }
            }
        }

        // Wait for all child processes to finish
        for (int i = 0; i < num_args; i++) {
            int status;
            wait(&status); 
        }

        free(input);
        for (int i = 0; args[i] != NULL; i++) {
            free(args[i]);
        }
        free(args);
    }
    return 0;
}
