#ifndef TASK_H
#define TASK_H

#include "command.h"
#include "timing.h"


struct task {
    struct command cmd;
    struct timing time;
    int fd_out;
    int fd_err;
    int fd_times_exitcodes;
};



int readtask(char *path, struct task *tbuf);
void freetask(struct task *tbuf);
int executetask(struct task *tbuf);

#endif