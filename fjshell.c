#define _GNU_SOURCE

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define PROMPT "fjshell"
#define ERROR_MESSAGE "An error has ocurred"
#define MAX_ARGS 10
#define PATH_MAX 4096


enum BUILT_IN_COMMAND { BIC_UNRECOGNIZED, BIC_EXIT, BIC_CD, BIC_PATH };

char cwd[PATH_MAX];

void print_err() 
{
    perror(ERROR_MESSAGE);
}

char *replace(char x, char *source)
{
    char *cpy = malloc(sizeof(char) * strlen(source) + 1);
    int i = 0;

    while (*source != '\0') {
        if (*source == x) {
            cpy[i] = x;
        } else {
            cpy[i] = *source;
        }

        i++;
    }

    cpy[i] = '\0';

    return cpy;
}

enum BUILT_IN_COMMAND getbuiltin(char *str)
{
    if (strcmp(str, "exit") == 0) {
        return BIC_EXIT;
    } else if (strcmp(str, "cd") == 0) {
        return BIC_CD;
    } else if (strcmp(str, "path") == 0) {
        return BIC_PATH;
    } else {
        return BIC_UNRECOGNIZED;
    }
}

char *getpath(char *program, char *envpath) 
{
    char *menvpath = strdup(envpath);
    char *ppath = NULL;

    while (menvpath != NULL) {
        char *path = strsep(&menvpath, ":");
        char *fpath = strcat(strdup(path), "/");
        char *fullpath = strcat(fpath, program);

        if (access(fullpath, X_OK) ==  0) {
            ppath = strdup(fullpath);
            break;
        }
    }

    return ppath;
}

char *argsconcat(char delim, char **args)
{
    char *cpy = malloc(sizeof(char) * 255);
    cpy[0] = '\0';

    while (*args != NULL) {
        strcat(cpy, strdup(*args));
        strcat(cpy, ":");

        args++;
    }

    cpy[strlen(cpy) - 1] = '\0';

    return cpy;
}

void execbic(enum BUILT_IN_COMMAND biccommand, char **args, char **envpath)
{
    switch (biccommand) {
        case BIC_EXIT:
            exit(0);
            break;
        case BIC_PATH:
            char *newpath = argsconcat(' ', ++args); 
            *envpath = newpath;
            break;
        case BIC_CD:
            if (chdir(args[1]) == -1) {
                print_err();
            } else {
                if (getcwd(cwd, PATH_MAX) == NULL) {
                    print_err();
                }
            }
            break;
        case BIC_UNRECOGNIZED:
            exit(3);
            break;
    }
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

    if (getcwd(cwd, PATH_MAX) == NULL) {
        print_err();
        exit(1);
    }

    while (true) {
        printf("%s:%s$ ", PROMPT, cwd);

        if ((nread = getline(&line, &len, stdin)) == -1) {
            print_err();
        }

        if (line[nread - 1] == '\n') {
            line[nread - 1] = '\0';
        }

        if (nread == 1) {
            continue;
        }

        char **args = getcommand(line);
        enum BUILT_IN_COMMAND bic = getbuiltin(args[0]);
        
        if (bic != BIC_UNRECOGNIZED) {
            execbic(bic, args, &envpath);
            continue;
        }

        pid_t pid;

        if ((pid = fork()) < 0) {
            print_err();
        } else if (pid == 0) {
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
