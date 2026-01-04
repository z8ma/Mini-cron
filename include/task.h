#ifndef TASK_H
#define TASK_H

#include "command.h"
#include "timing.h"
#include "string_uint.h"

struct task{
    uint64_t taskid;
    struct timing task_timing;
    struct command task_command;
};

int readtask(int fd, struct task *tbuf);
int mkdirtask(char *path_task, struct task *tbuf);
int readtask_timing(char *path_task, struct timing *task_timing);
int readtask_command(char *path_task, struct command *task_command);
int readstd(int fd,struct string *output);
int writetask(int fd, struct task *tbuf);
int executetask(char *path_task);
int task_to_string(struct task t, struct string *s);

#endif