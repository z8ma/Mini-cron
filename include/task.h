#ifndef TASK_H
#define TASK_H

#include "command.h"
#include "timing.h"

struct task{
    uint64_t taskid;
    struct timing task_timing;
    struct command task_command;
};

int readtask(int fd, struct task *tbuf);
int readtask_timing(char *path_task, struct timing *task_timing);
int readtask_command(char *path_task, struct command *task_command);
int writetask(int fd, struct task *tbuf);
int executetask(char *path_task);

#endif