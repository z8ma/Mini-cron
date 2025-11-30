#ifndef TASK_H
#define TASK_H

#include "command.h"
#include "timing.h"


int readtask_timing(char *path_task, struct timing *task_timing);
int readtask_command(char *path_task, struct command *task_command);
int redirectstdout(char *path_task);
int redirectstderr(char *path_task);
int writetask_times_exitcodes(char *path_task, uint16_t exitcodes);
int executetask(char *path_task);

#endif