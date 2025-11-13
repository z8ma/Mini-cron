#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include "string.h"
#include <stdint.h>


struct arguments {
    uint32_t argc;
    struct string *argv;
};

int readarguments(int fd, struct arguments *abuf);
void freearguments(struct arguments *abuf);

#endif