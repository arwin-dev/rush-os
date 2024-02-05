#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\n"

void printErrorMessage() {
    char error_message[] = "An error has occurred\n";
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

char **rushSplitLine(char *input) {
    int bufSize = TOK_BUFSIZE;
    int i = 0;
    char **tokens = malloc(bufSize * sizeof(char *));
    char *token;

    if (!tokens) {
        printErrorMessage();
        exit(EXIT_FAILURE);
    }

    token = strtok(input, TOK_DELIM);
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
        token = strtok(NULL, TOK_DELIM);
    }
    tokens[i] = NULL;
    return tokens;
}

void rushExecute(char **args, char **path)
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
            pid_t pid;
            int status;

            pid = fork();
            if(pid == 0)
            {
                if(execvp(args[0], args) == -1){
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
            }
        }
        free(full_path);
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

int main() {
    char *input;
    char **args;
    char *initialPath[2];
    initialPath[0] = strdup("/bin");
    initialPath[1] = NULL;

    char **path = initialPath;

    for(int j = 0; path[j] != NULL; j++)
        printf("%s\n", path[j]);

    while (1) {
        printf("rush> ");
        fflush(stdout);

        input = rushReadLine();
        args = rushSplitLine(input);

        if(args[0] == NULL){
            continue;
        }
        if (strcmp(args[0], "exit") == 0 && args[1] == NULL) {
            exit(1);
        } else if (strcmp(args[0], "cd") == 0) {
            if (chdir(args[1]) != 0) {
                printErrorMessage();
            }
        } else if (strcmp(args[0], "path") == 0) {
            path = generatePath(args);
        }else {
            if(path[0] == NULL)
                continue;
                
            rushExecute(args, path);
        }

        free(input);
        for (int i = 0; args[i] != NULL; i++) {
            free(args[i]);
        }
        free(args);
    }
    return 0;
}
