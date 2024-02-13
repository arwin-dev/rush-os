#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h> 

#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\n"

void printErrorMessage() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    fflush(stderr);
}

char *rushReadLine() {
    char *input = NULL;
    size_t bufsize = 0;

    if (getline(&input, &bufsize, stdin) == -1) {
        printErrorMessage();
        exit(EXIT_FAILURE);
    }

    return input;
}

char **rushSplitLine(char *input, char *delim) {
    int bufSize = TOK_BUFSIZE;
    int i = 0;
    char **tokens = malloc(bufSize * sizeof(char *));
    char *token=NULL;

    if (!tokens) {
        printErrorMessage();
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
                printErrorMessage();
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, delim);
    }
    tokens[i] = NULL;
    return tokens;
}

void rushExecuteSingleCommand(char **args, char **path)
{   
    for(int j = 0; path[j] != NULL; j++)
    { 
        size_t full_path_len = strlen(path[j]) + strlen("/") + strlen(args[0]) + 1;
        char *full_path = (char *)malloc(full_path_len * sizeof(char));
        if (full_path == NULL) {
            printErrorMessage();
            return;
        }

        strcpy(full_path, path[j]);
        strcat(full_path, "/");
        strcat(full_path, args[0]);
        int fd = access(full_path, X_OK);
        if (fd == -1) {
            printErrorMessage();
        }
        else
        {
            int redir_count = 0;
            int redir_index = -1;
            for (int i = 0; args[i] != NULL; i++) {
                if (strcmp(args[i], ">") == 0) {
                    redir_count++;
                    redir_index = i;
                    if (redir_count > 1 || args[i+2] != NULL) {
                        printErrorMessage();
                        exit(EXIT_FAILURE);
                    }
                }
            }

            if (redir_index != -1 && args[redir_index + 1] != NULL)
            {
                int file_desc = open(args[redir_index + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (file_desc == -1)
                {
                    printErrorMessage();
                    exit(EXIT_FAILURE);
                }
                dup2(file_desc, STDOUT_FILENO);
                close(file_desc);
                args[redir_index] = NULL;
                args[redir_index + 1] = NULL;
            }

            if (execvp(args[0], args) == -1)
            {
                printErrorMessage();
            }
        }
        free(full_path);
    }
}

void rushExecute(char **args, char **path)
{
    int success = 0;
    size_t final_full_path_len = 0; 
    char *final_full_path = NULL;
    for(int j = 0; path[j] != NULL; j++)
    {
        size_t full_path_len = strlen(path[j]) + strlen("/") + strlen(args[0]) + 1;
        char *full_path = (char *)malloc(full_path_len * sizeof(char));
        if (full_path == NULL) 
        {
            continue;
        }

        strcpy(full_path, path[j]);
        strcat(full_path, "/");
        strcat(full_path, args[0]);
        int checkIfFileExists = access(full_path, X_OK);
        if(checkIfFileExists == 0)
        {
            success = 1;
            if(final_full_path != NULL) free(final_full_path);
            final_full_path = (char *)malloc(full_path_len * sizeof(char));
            strcpy(final_full_path, full_path);
            break;
        }
    }

    if(success == 0)
    {
        printErrorMessage();
        return;
    }
    else
    {
        pid_t pid;
        int status;

        pid = fork();
        if(pid == 0)
        {
            int redir_count = 0;
            int redir_index = -1;
            for (int i = 0; args[i] != NULL; i++) {
                if (strcmp(args[i], ">") == 0) {
                    redir_count++;
                    redir_index = i;
                    if (redir_count > 1 || args[i+1] == NULL ||args[i+2] != NULL ) {
                        printErrorMessage();
                        exit(EXIT_FAILURE);
                    }
                }
            }

            if (redir_index != -1 && args[redir_index + 1] != NULL) {
                int file_desc = open(args[redir_index + 1], 1 | 512 | 64, 0644);
                if (file_desc == -1) {
                    printErrorMessage();
                    exit(EXIT_FAILURE);
                }
                dup2(file_desc, STDOUT_FILENO);
                close(file_desc);
                args[redir_index] = NULL;
                args[redir_index + 1] = NULL;
            }

            if(execvp(final_full_path, args) == -1){
                printErrorMessage();
            }
        }
        else if (pid < 0)
        {
            printErrorMessage();
        }
        else
        {
            wait(&status);
            return;
        }
    }
}

void rushExecuteASDASD(char **args, char **path)
{

    for(int j = 0; args[j] != NULL; j++)
        printf("%s\n", args[j]);

    int num_commands = 0;
    char *commands[64];

    if (args[0] != NULL) {
        commands[num_commands++] = args[0]; 
    }
    
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "&") == 0)
        {
            args[i] = NULL;                          
            if (args[i + 1] != NULL) {
                commands[num_commands++] = args[i + 1]; // Include next token as command
            }
        }
        else {
            commands[num_commands++] = args[i]; // Include current token as command
        }
    }
    commands[num_commands++] = NULL; 

    pid_t pids[num_commands];
    for (int i = 0; i < num_commands; i++)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            printErrorMessage();
            return;
        }
        else if (pid == 0)
        {
            rushExecuteSingleCommand(rushSplitLine(commands[i], TOK_DELIM), path);
            exit(EXIT_FAILURE);
        }
        else
        {
            pids[i] = pid;
        }
    }

    for (int i = 0; i < num_commands; i++)
    {
        int status;
        waitpid(pids[i], &status, 0);
    }
}

char **generatePath(char **args1) {
    int size = 0;
    while (args1[size] != NULL) {
        size++;
    }
    char **path = malloc(size * sizeof(char *));
    if (!path) {
        printErrorMessage();
        exit(EXIT_FAILURE);
    }
    for (int i = 1; args1[i] != NULL; i++) {
        path[i - 1] = strdup(args1[i]);
    }
    path[size - 1] = NULL;
    return path;
}

int main(int argc, char *argv[]) 
{
    if(argc > 1)
    {
        printErrorMessage();
        return EXIT_FAILURE;
    }
    
    char *input;
    char **args;
    char *initialPath[2];
    initialPath[0] = strdup("/bin");
    initialPath[1] = strdup("/usr/bin");
    initialPath[2] = NULL;

    char **path = initialPath;

    // for(int j = 0; path[j] != NULL; j++)
    //     printf("%s\n", path[j]);

    while (1) {
        printf("rush> ");
        fflush(stdout);

        input = rushReadLine();
        args = rushSplitLine(input, "&");

        for(int i = 0; args[i] != NULL; i++)
        {
            char **parall=NULL;
            parall = rushSplitLine(args[i], TOK_DELIM);

            if(parall[0] == NULL)
            {
                continue;
            }
            if (strcmp(parall[0], "exit") == 0) 
            {
                if(parall[1] == NULL)
                    exit(1);
                else
                    printErrorMessage();
            } 
            else if (strcmp(parall[0], "cd") == 0) 
            {
                if (chdir(parall[1]) != 0) {
                    printErrorMessage();
                }
            } 
            else if (strcmp(parall[0], "path") == 0) 
            {
                path = generatePath(parall);
            }
            else
            {
                if(path[0] == NULL)
                {
                    printErrorMessage();
                    continue;
                }
                rushExecute(parall, path);
            }
        }

        free(input);
        for (int i = 0; args[i] != NULL; i++) {
            free(args[i]);
        }
        free(args);
    }
    return 0;
}
