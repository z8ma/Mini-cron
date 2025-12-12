#include "arguments.h"
#include "string_uint.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int readarguments(int fd, struct arguments *abuf) {
    if (read(fd, &(abuf->argc), sizeof(uint32_t)) < 0) return 1;

    uint32_t host_argc = be32toh(abuf->argc);
    abuf->argc = host_argc;

    abuf->argv = malloc(host_argc * sizeof(struct string));
    for (size_t i = 0; i < host_argc; i++) {
        if (readstring(fd, abuf->argv + i) < 0) return 1;
    }
    return 0;
}

int writearguments(int fd, struct arguments *abuf) {
    uint32_t argc_be = htobe32(abuf->argc);
    if (write(fd, &argc_be, sizeof(uint32_t)) < 0) return 1;

    for (size_t i = 0; i < abuf->argc; i++) {
        if (writestring(fd, abuf->argv + i) < 0) return 1;
    }
    return 0;
}

void freearguments(struct arguments *abuf) {
    for (int i = 0; i < abuf->argc; i++) {
        freestring(abuf->argv + i);
    }
    free(abuf->argv);
}

uint16_t executearg(struct arguments *abuf) {
    uint32_t host_argc = abuf->argc;
    pid_t p = fork();
    if (p == 0) {
        char **exec_argv = malloc((host_argc+1) * sizeof(char *));
        if (!exec_argv) { 
            perror("malloc"); exit(1); 
        }

        for (uint32_t i = 0; i < host_argc; i++) {
            exec_argv[i] = (char *)abuf->argv[i].data;
        }
        exec_argv[host_argc] = NULL;
        execvp(exec_argv[0], exec_argv);

        perror("execvp");
        free(exec_argv);
        exit(1);
    }
    else {
        int status;
        wait(&status);
        if (WIFEXITED(status)) {
            return (uint16_t) WEXITSTATUS(status);
        }
        return 0xffff;
    }
}