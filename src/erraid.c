#include "task.h"
#include "communication.h"

#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>

int select_pipe_until_next_minute(int fd_pipe_request) {
    fd_set fdreads;
    FD_ZERO(&fdreads);
    FD_SET(fd_pipe_request, &fdreads);

    struct timeval tv;
    tv.tv_sec = 60 - (time(NULL) % 60);
    tv.tv_usec = 0;
    int ret = select(fd_pipe_request + 1, &fdreads, NULL, NULL, &tv);
    if (ret <= 0) {
        return ret;
    }
    return ret;
}

int main(int argc, char *argv[]) {
    int opt;
    if ((opt = getopt(argc, argv, "r:")) != -1) {
        switch (opt) {
        case 'r':
                if (chdir(optarg) < 0) {
                    perror("chdir");
                    return 1;
                }
            break;
        case '?':
            exit(1);
            break;
        }
    }
    else {
        char run_directori[PATH_MAX] = "/tmp";
        size_t len = strlen(run_directori);
        snprintf(run_directori + len, sizeof(run_directori) - len, "/%s", getenv("USER"));
        len = strlen(run_directori);

        if (mkdir(run_directori, 0744) < 0) {
            if (errno != EEXIST) {
                perror("mkdir");
                return 1;
            }
        }
        snprintf(run_directori + len, sizeof(run_directori) - len, "/%s", "erraid");
        if (mkdir(run_directori, 0744) < 0) {
            if (errno != EEXIST) {
                perror("mkdir");
                return 1;
            }
        }
        if (chdir(run_directori) < 0) {
            perror("chdir");
            return 1;
        }
    }

    if (mkdir("tasks", 0744) < 0) {
        if (errno != EEXIST) {
            perror("mkdir");
            return 1;
        }
    }

    if (mkdir("pipes", 0744) < 0) {
        if (errno != EEXIST) {
            perror("mkdir");
            return 1;
        }
    }

    if (mkfifo("pipes/erraid-request-pipe", 0644) < 0) {
        if (errno != EEXIST) {
            perror("mkfifo");
            return 1;
        }
    }
    int fd_pipe_request = open("pipes/erraid-request-pipe", O_RDONLY | O_NONBLOCK);

    if (mkfifo("pipes/erraid-reply-pipe", 0644) < 0) {
        if (errno != EEXIST) {
            perror("mkfifo");
            return 1;
        }
    }

    pid_t p = fork();
    if (p < 0) {
        perror("fork");
        return 1;
    }
    else if (p > 0) {
        return 0;
    }
    else {
        if (setsid() < 0) {
            perror("setsid");
            return 1;
        }
        p = fork();
        if (p < 0) {
            perror("fork");
            return 1;
        }
        else if (p > 0) {
            return 0;
        }
    }

    int fd = open("/dev/null", O_RDWR);
    dup2(fd, STDIN_FILENO);
    close(fd);

    struct dirent *entry;
    char path_task[PATH_MAX];
    struct stat st;
    
    while (1) {
        int action = select_pipe_until_next_minute(fd_pipe_request);
        if (action < 0) {
            return 1;
        } else if (action == 0) {
            DIR *dirp = opendir("tasks");
            if (!dirp) {
                perror("opendir");
                return 1;
            }
            while ((entry = readdir(dirp))) {
                snprintf(path_task, PATH_MAX, "%s/%s", "tasks", entry->d_name);
                if (stat(path_task, &st) < 0) {
                    perror("stat");
                    return 1;
                }
                if (S_ISDIR(st.st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    p = fork();
                    if (p == 0) {
                        executetask(path_task);
                        return 0;
                    }
                }
            }
            closedir(dirp);
        } else {
            int fd_pipe_reply = open("pipes/erraid-reply-pipe", O_WRONLY);
            handle_request(fd_pipe_request, fd_pipe_reply);
        }
    }
    return 0;
}