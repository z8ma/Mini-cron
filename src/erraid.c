#include "task.h"
#include "communication.h"
#include "request.h"
#include "reply.h"

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

pid_t fork_detached() {
    pid_t p = fork();
    if (p < 0)
        return -1;
    if (p == 0) {
        pid_t g = fork();
        if (g < 0)
            _exit(1);
        if (g == 0) {
            return 0;
        }
        _exit(0);
    }
    waitpid(p, NULL, 0);
    return 1;
}

int select_pipe_request_until_next_minute(int fd_pipe_request, struct request *req) {
    fd_set fdreads;
    FD_ZERO(&fdreads);
    FD_SET(fd_pipe_request, &fdreads);

    struct timeval tv;
    tv.tv_sec = 60 - (time(NULL) % 60);
    tv.tv_usec = 0;
    int ret = select(fd_pipe_request + 1, &fdreads, NULL, NULL, &tv);
    if (ret == 1) {
        readrequest(fd_pipe_request, req);
    }
    return ret;
}

int check_tasks() {
    struct dirent *entry;
    char path_task[27] = "tasks/";
    struct stat st;
    DIR *dirp = opendir(path_task);
    if (!dirp) {
        perror("opendir");
        return 1;
    }
    while ((entry = readdir(dirp))) {
        strcat(path_task, entry->d_name);
        if (stat(path_task, &st) < 0) {
            perror("stat");
            return 1;
        }
        if (S_ISDIR(st.st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            pid_t p = fork_detached();
            if (p == 0) {
                executetask(path_task);
                _exit(0);
                }
        }
        path_task[6] = '\0';
    }
    closedir(dirp);
    return 0;
}

int main(int argc, char *argv[]) {

    int opt;
    int ropt = 0;
    char *run_directori;
    int popt = 0;
    char *pipes_directori;
    int fopt = 0;
    while ((opt = getopt(argc, argv, "R:P:F")) != -1) {
        switch (opt) {
        case 'R':
            ropt = 1;
            run_directori = malloc(strlen(optarg) * sizeof(char) + 1);
            strcpy(run_directori, optarg);
            break;
        case 'P':
            popt = 1;
            pipes_directori = malloc(strlen(optarg) * sizeof(char) + 1);
            strcpy(pipes_directori, optarg);
            break;
        case 'F':
            fopt = 1;
            break;
        case '?':
            exit(1);
            break;
        }
    }

    // Prise en charge en cas d'absence d'options
    if (!ropt) {   
        run_directori = malloc((strlen("/tmp/") + strlen(getenv("USER")) + strlen("/erraid/") + 1) * sizeof(char));
        strcpy(run_directori, "/tmp/");

        strcat(run_directori, getenv("USER"));
        if (mkdir(run_directori, 0744) < 0) {
            if (errno != EEXIST) {
                perror("mkdir1");
                return 1;
            }
        }

        strcat(run_directori, "/erraid/");
        if (mkdir(run_directori, 0744) < 0) {
            if (errno != EEXIST) {
                perror("mkdir2");
                return 1;
            }
        }
    }

    if (!popt) {
        pipes_directori = malloc((strlen(run_directori) + strlen("/pipes/") + 1)* sizeof(char));
        strcpy(pipes_directori, run_directori);
        strcat(pipes_directori, "/pipes/");
        if (mkdir(pipes_directori, 0744) < 0) {
            if (errno != EEXIST) {
                return 1;
            }
        }
    }
    if (!fopt) {
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
            } else if (p > 0) {
                return 0;
            }
        }
    }

    // Ouverture des descripteurs request et reply
    char *request_fifo = malloc(strlen(pipes_directori) + strlen("erraid-request-pipe") + 1);
    strcpy(request_fifo, pipes_directori);
    strcat(request_fifo, "erraid-request-pipe");
    if (mkfifo(request_fifo, 0644) < 0) {
        if (errno != EEXIST) {
            printf("%s\n", request_fifo);
            perror("mkfifo1");
            return 1;
        }
    }
    int fd_pipe_request = open(request_fifo, O_RDWR);
    free(request_fifo);

    char abs_reply_fifo[PATH_MAX];
    char *reply_fifo = malloc(strlen(pipes_directori) + strlen("erraid-reply-pipe") + 1);
    strcpy(reply_fifo, pipes_directori);
    strcat(reply_fifo, "erraid-reply-pipe");
    if (mkfifo(reply_fifo, 0644) < 0) {
        if (errno != EEXIST) {
            printf("%s\n", reply_fifo);
            perror("mkfifo2");
            return 1;
        }
    }
    realpath(reply_fifo, abs_reply_fifo);
    free(reply_fifo);

    free(pipes_directori);

    // Déplacement jusqu'au dossier dans lequel travaille erraid
    if (chdir(run_directori) < 0) {
        perror("chdir");
        return 1;
    }
    free(run_directori);

    if (mkdir("tasks", 0744) < 0) {
        if (errno != EEXIST) {
            perror("mkdir3");
            return 1;
        }
    }

    int fd = open("/dev/null", O_RDWR);
    dup2(fd, STDIN_FILENO);
    close(fd);

    struct request request;
    struct reply reply;

    while (1) {
        int action = select_pipe_request_until_next_minute(fd_pipe_request, &request);
        if (action < 0) {
            return 1;
        } else if (action == 0) {
            if (check_tasks()) {
                return 1;
            }
        } else {
            handle_request(request, &reply);
            uint16_t opcode = request.opcode;
            freerequest(&request);
            pid_t p = fork_detached();
            if (p < 0) {
                return 1;
            } else if (p == 0) {
                int fd_pipe_reply = open(abs_reply_fifo, O_WRONLY);
                writereply(fd_pipe_reply, &reply, opcode);
                freereply(&reply, opcode);
                close(fd_pipe_reply);
                return 0;
            }
            if (opcode == TM_OPCODE) {
                return 0;   
            }
        }
    }
    return 0;
}