#define _GNU_SOURCE

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define PROMPT "fjshell> "
#define ERROR_MESSAGE "An error has ocurred"
#define MAX_ARGS 10

void print_err() 
{
    perror(ERROR_MESSAGE);
}

char *getpath(char *program, char *envpath) 
{
    char *menvpath = envpath;
    char *ppath = NULL;

    for (char *path = strsep(&menvpath, ":"); menvpath != NULL; path = strsep(&menvpath, ":")) {
        char *fpath = strcat(strdup(path), "/");
        char *fullpath = strcat(fpath, program);

        if (access(fullpath, X_OK) ==  0) {
            ppath = strdup(fullpath);
            break;
        }
    }

    return ppath;
}

char **getcommand(char *source)
{
    char **args = malloc(MAX_ARGS * sizeof(char*));
    int i = 0;
    args[i++] = strsep(&source, " ");

    for (;source != NULL && i < MAX_ARGS; i++) {
        char *arg = strsep(&source, " ");
        args[i] = arg;
    }

    args[i] = NULL;
    return args;
}

int
main(int argc, char **argv)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t nread = 0;
    char *envpath = getenv("PATH");

    while (true) {
        printf("%s", PROMPT);

        if ((nread = getline(&line, &len, stdin)) == -1) {
            print_err();
        }

        if (line[nread - 1] == '\n') {
            line[nread - 1] = '\0';
        }

        pid_t pid;

        if ((pid = fork()) < 0) {
            print_err();
        } else if (pid == 0) {
            char **args = getcommand(line);
            char *path = getpath(args[0], envpath);

            if (path == NULL || execv(path, args) == -1) {
                free(args);
                print_err();
            }

        } else {
            if (wait(NULL) != pid) {
                print_err();
            }
        }
    }
}
