#include "task.h"
#include "command.h"
#include "timing.h"
#include "string_uint.h"
#include "times_exitcodes.h"

#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>

int readtask(int fd, struct task *tbuf){
    uint64_t taskid_be;
    if (read(fd, &taskid_be, sizeof(uint64_t)) < 0) return 1;
    tbuf->taskid = be64toh(taskid_be);
    if (readtiming(fd, &(tbuf->task_timing)) == 1) return 1;
    if (readcmd_fd(fd, &(tbuf->task_command)) == 1) return 1;
    return 0;
}

int writetask(int fd, struct task *tbuf){
    uint64_t taskid_be = htobe64(tbuf->taskid);
    if (write(fd, &taskid_be, sizeof(uint64_t)) < 0) return 1;
    if (writetiming(fd, &(tbuf->task_timing)) == 1) return 1;
    if (writecmd(fd, &(tbuf->task_command)) == 1) return 1;
    return 0;
}


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
    if (readcmd_path(path_task_command, task_command) == 1) return 1;
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

int writelastrun(char *path_task, struct run *r) {
    char path_task_times_exitcodes[PATH_MAX];
    snprintf(path_task_times_exitcodes, sizeof(path_task_times_exitcodes), "%s/times-exitcodes", path_task);
    ssize_t fd_times_exitcodes = open(path_task_times_exitcodes, O_WRONLY | O_APPEND);
    if (fd_times_exitcodes < 0) return 1;
    if (writerun(fd_times_exitcodes, r) == 1) return 1;
    return 0;
}

int executetask(char *path_task) {
    struct task t;
    if (readtask_timing(path_task, &t.task_timing) == 1) return 1;
    if (is_it_time(&t.task_timing) == 1) {
        if (readtask_command(path_task, &t.task_command) == 1) return 1;
        if (redirectstdout(path_task) == 1) return 1;
        if (redirectstderr(path_task) == 1) return 1;
        struct run r = {time(NULL), executecmd(&t.task_command)};
        if (writelastrun(path_task, &r) == 1) return 1;
        freecmd(&t.task_command);
    }
    return 0;
}

int readstd(int fd, struct string *output) {
    struct stat st;
    size_t size;
    
    if (fstat(fd, &st) < 0) {
        perror("erreur fstat");
        return 1;
    }
    size = st.st_size;

    output->length = (uint32_t)size;
    
    if (size > 0) {
        output->data = malloc(size);
        if (!output->data) {
            perror("erreur allocation");
            output->length = 0;
            return 1; 
        }
        
        if (read(fd, output->data, size) != size) {
            perror("erreur lecture");
            free(output->data); 
            output->data = NULL;
            output->length = 0;
            return 1;
        }
    }
    
    return 0;
}

int task_to_string(struct task t, struct string *s) {
    struct string colon = {1, (uint8_t*)":"};
    struct string space = {1, (uint8_t*)" "};
    if(uint_to_string(t.taskid, s) == 1) return 1;
    if(catstring(s, colon) == 1) return 1;
    if(catstring(s, space) == 1) return 1;
    if(timing_to_string(t.task_timing, s) == 1) return 1;
    if(catstring(s, space) == 1) return 1;
    if(command_to_string(t.task_command, s) == 1) return 1;
    return 0;
}