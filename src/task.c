#include "task.h"
#include "command.h"
#include "timing.h"

#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <time.h>
#include <unistd.h>


int readtask(char *taskname, struct task *tbuf) {
    snprintf(tbuf->id, 200,"%s", taskname);
    char path[PATH_MAX] = "tasks";
    size_t len = strlen(path);
    snprintf(path + len, sizeof(path) -  len, "/%s", tbuf->id);
    len = strlen(path);

    snprintf(path + len, sizeof(path) -  len, "/%s", "cmd");
    if (readcmd(path, &(tbuf->cmd)) == 1) {
        perror("readcmd");
        return 1;
    } 

    snprintf(path + len, sizeof(path) -  len, "/%s", "timing");
    int fd_timing = open(path, O_RDONLY);
    if (fd_timing < 0) {
        perror("open");
        return 1;
    }
    if (readtiming(fd_timing, &(tbuf->time)) == 1) {
        perror("readtiming");
        close(fd_timing);
        return 1;
    }
    close(fd_timing);

    return 0;
}

void freetask(struct task *tbuf) {
    freecmd(&(tbuf->cmd));
}


int executetask(struct task *tbuf) {
    if (is_it_time(&(tbuf->time))){
        char path[PATH_MAX] = "tasks";
        size_t len = strlen(path);
        snprintf(path + len, sizeof(path) -  len, "/%s", tbuf->id);
        len = strlen(path);

        snprintf(path + len, sizeof(path) -  len, "/%s", "stdout");
        ssize_t fd_out = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_out < 0) {
            perror("open");
            return 1;
        }

        snprintf(path + len, sizeof(path) -  len, "/%s", "stderr");
        ssize_t fd_err = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_err < 0) {
            perror("open");
            return 1;
        }

        snprintf(path + len, sizeof(path) -  len, "/%s", "times-exitcodes");
        ssize_t fd_times_exitcodes = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd_times_exitcodes < 0) {
            perror("open");
            return 1;
        }
        dup2(fd_out, STDOUT_FILENO);
        dup2(fd_err, STDERR_FILENO);
        int ret = executecmd(&(tbuf->cmd));
        uint64_t now = htobe64(time(NULL));
        uint16_t exitcodes = htobe16(ret);
        if (write(fd_times_exitcodes, &now, sizeof(uint64_t)) != sizeof(uint64_t)) {
            return -1;
        }
        if (write(fd_times_exitcodes, &exitcodes, sizeof(uint16_t)) != sizeof(uint16_t)) {
            return -1;
        }
        return ret;
    }
    return -1;
}