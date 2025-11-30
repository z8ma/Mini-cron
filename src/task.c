#include "task.h"
#include "command.h"
#include "timing.h"

#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

int readtask_timing(char *path_task, struct timing *task_timing) {
    char path_task_timing[PATH_MAX];
    snprintf(path_task_timing, sizeof(path_task_timing), "%s/timing", path_task);
    ssize_t fd_timing = open(path_task_timing, O_RDONLY);
    if (fd_timing < 0) return 1;
    if (readtiming(fd_timing, task_timing) == 1) return 1;
    return 0;
}

int readtask_command(char *path_task, struct command *task_command) {
    char path_task_command[PATH_MAX];
    snprintf(path_task_command, sizeof(path_task_command), "%s/cmd", path_task);
    if (readcmd(path_task_command, task_command) == 1) return 1;
    return 0;
}

int redirectstdout(char *path_task) {
    char path_task_stdout[PATH_MAX];
    snprintf(path_task_stdout, sizeof(path_task_stdout), "%s/stdout", path_task);
    ssize_t fd_stdout = open(path_task_stdout, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (fd_stdout < 0) return 1;
    if (dup2(fd_stdout, STDOUT_FILENO) == -1) return 1;
    close(fd_stdout);
    return 0;
}

int redirectstderr(char *path_task) {
    char path_task_stderr[PATH_MAX];
    snprintf(path_task_stderr, sizeof(path_task_stderr), "%s/stderr", path_task);
    ssize_t fd_stderr = open(path_task_stderr, O_WRONLY | O_TRUNC| O_CREAT, 0644);
    if (fd_stderr < 0) return 1;
    if (dup2(fd_stderr, STDERR_FILENO) == -1) return 1;
    close(fd_stderr);
    return 0;
}

int writetask_times_exitcodes(char *path_task, uint16_t exitcodes) {
    uint64_t now = htobe64(difftime(time(NULL), 0));
    char path_task_times_exitcodes[PATH_MAX];
    snprintf(path_task_times_exitcodes, sizeof(path_task_times_exitcodes), "%s/times-exitcodes", path_task);
    ssize_t fd_times_exitcodes = open(path_task_times_exitcodes, O_WRONLY | O_APPEND);
    if (fd_times_exitcodes < 0) return 1;
    if (write(fd_times_exitcodes, &now, sizeof(uint64_t)) < 0) return 1;
    if (write(fd_times_exitcodes, &exitcodes, sizeof(uint16_t)) < 0) return 1;
    return 0;
}

int executetask(char *path_task) {
    struct timing task_timing;
    if (readtask_timing(path_task, &task_timing) == 1) return 1;
    if (is_it_time(&task_timing) == 1) {
        struct command task_command;
        if (readtask_command(path_task, &task_command) == 1) return 1;
        if (redirectstdout(path_task) == 1) return 1;
        if (redirectstderr(path_task) == 1) return 1;
        if (writetask_times_exitcodes(path_task, htobe16(executecmd(&task_command))) == 1) return 1;
        freecmd(&task_command);
    }
    return 0;
}