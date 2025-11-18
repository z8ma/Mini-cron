#include "arguments.h"
#include "string_uint.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


int readarguments(int fd, struct arguments *abuf) {
    if (read(fd, abuf, sizeof(uint32_t)) < 0) return 1;

    abuf->argc = be32toh(abuf->argc);
    abuf->argv = malloc((abuf->argc) * sizeof(struct string));
    for (size_t i = 0; i < abuf->argc; i++) {
        if (readstring(fd, &abuf->argv[i]) < 0) return 1;
    }

    return 0;
}

void freearguments(struct arguments *abuf) {
    for(int i = 0; i < abuf->argc; i++) {
        freestring(&(abuf->argv[i]));
    }
    free((abuf)->argv);
}

void executearg(struct arguments *abuf) {
    char **exec_argv = malloc((abuf->argc + 1) * sizeof(char *));
    for (uint32_t i = 0; i < abuf->argc; i++) {
        exec_argv[i] = (char *)abuf->argv[i].data;
    }
    exec_argv[abuf->argc] = NULL;
    execvp(exec_argv[0], exec_argv);

    perror("execvp");
    free(exec_argv);
    exit(1);
}