#include "arguments.h"
#include "string.h"

#include <stdlib.h>
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