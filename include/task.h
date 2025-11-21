#ifndef TASK_H
#define TASK_H

#include "command.h"
#include "timing.h"


struct task {
    char id[200];
    struct command cmd;
    struct timing time;
};



int readtask(char *path, struct task *tbuf);
void freetask(struct task *tbuf);
int executetask(struct task *tbuf);

#endif