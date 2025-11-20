#include "task.h"
#include "command.h"
#include "timing.h"

#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

int readtask(char *taskname, struct task *tbuf) {
    char path[PATH_MAX];

    snprintf(path, PATH_MAX, "%s/%s", taskname, "cmd");
    if (readcmd(path, &(tbuf->cmd)) == 1) {
        perror("readcmd");
        return 1;
    } 

    snprintf(path, PATH_MAX, "%s/%s", taskname, "timing");
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

    snprintf(path, PATH_MAX, "%s/%s", taskname, "stdout");
    tbuf->fd_out = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (tbuf->fd_out < 0) {
        perror("open");
        return 1;
    }
    

    snprintf(path, PATH_MAX, "%s/%s", taskname, "stderr");
    tbuf->fd_err = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (tbuf->fd_err < 0) {
        perror("open");
        return 1;
    }

    snprintf(path, PATH_MAX, "%s/%s", taskname, "times-exitcodes");
    tbuf->fd_times_exitcodes = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (tbuf->fd_times_exitcodes < 0) {
        perror("open");
        return 1;
    }

    return 0;
}

void freetask(struct task *tbuf) {
    freecmd(&(tbuf->cmd));
    close(tbuf->fd_out);
    close(tbuf->fd_err);
    close(tbuf->fd_times_exitcodes);
}


int executetask(struct task *tbuf) {
    if (is_it_time(&(tbuf->time))){
        dup2(tbuf->fd_out, STDOUT_FILENO);
        dup2(tbuf->fd_err, STDERR_FILENO);
        int ret = executecmd(&(tbuf->cmd));
        uint16_t exitcodes = htobe16(ret);
        uint64_t now = htobe64(time(NULL));
        if (write(tbuf->fd_times_exitcodes, &now, sizeof(uint64_t)) != sizeof(uint64_t)) {
            return -1;
        }
        if (write(tbuf->fd_times_exitcodes, &exitcodes, sizeof(uint16_t)) != sizeof(uint64_t)) {
            return -1;
        }
        return ret;
    }
    return -1;
}