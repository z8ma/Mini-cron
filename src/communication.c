#include "request.h"
#include "reply.h"
#include "task.h"

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
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

void insertion_sort2(char **arr, size_t n) {
    for (size_t i = 1; i < n; i++) {
        char *key = arr[i];
        size_t j = i - 1;
        while (j >= 0 && strcmp(arr[j], key) > 0) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

void string_to_uint64(uint64_t *n, char *s) {
    *n = 0;
    while (*s) {
        *n = (*n) * 10 + (*s - '0');
        s++;
    }
    *n = htobe64(*n);
}

int handle_list_request(struct request req, struct reply *rbuf) {
    rbuf->anstype = htobe16(OK_ANSTYPE);
    rbuf->content.list.nbtasks = 0;

    char **names = NULL;
    char path_task[PATH_MAX];

    DIR *dirp = opendir("tasks");
    if (!dirp) return 1;

    struct dirent *entry;
    while ((entry = readdir(dirp))) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && (entry->d_type == DT_DIR)) {
            names = realloc(names, (rbuf->content.list.nbtasks + 1) * sizeof(char *));
            names[rbuf->content.list.nbtasks++] = (entry->d_name);
        }
    }
    closedir(dirp);

    insertion_sort2(names, rbuf->content.list.nbtasks);
    rbuf->content.list.tasks = malloc(rbuf->content.list.nbtasks * sizeof(struct task));
    for (int i = 0; i < rbuf->content.list.nbtasks; i++) {
        snprintf(path_task, sizeof(path_task), "tasks/%s", names[i]);

        string_to_uint64(&((rbuf->content.list.tasks + i)->taskid), names[i]);
        readtask_timing(path_task, &((rbuf->content.list.tasks + i)->task_timing));
        readtask_command(path_task, &((rbuf->content.list.tasks + i)->task_command));
    }
    rbuf->content.list.nbtasks = htobe32(rbuf->content.list.nbtasks);
    return 0;
}

int handle_creat_request(struct request req, struct reply *rbuf) {
    rbuf->anstype = OK_ANSTYPE;
    // TODO fonction qui créer une tache dans task.h et renvoie l'id de cette tache
    return 0;
}

int handle_combine_request(struct request req, struct reply *rbuf) {
    // TODO fonction qui combine des taches dans task.h et renvoie l'id de la nouvelle tache
    return 0;
}

int handle_remove_request(struct request req, struct reply *rbuf) {
    char path_task[PATH_MAX];
    snprintf(path_task, sizeof(path_task), "task/%zu", req.content.taskid);
    // TODO fonction qui supprime une tache dans task.h et renvoie si la supreesion à échoué ou non
    return 0;
}

int handle_times_exitcodes_request(struct request req, struct reply *rbuf) {
    char path_task[PATH_MAX];
    snprintf(path_task, sizeof(path_task), "task/%zu", req.content.taskid);
    // TODO fonction qui affecte à un tableau de times_ecxitcodes dans task.h
    return 0;
}

int handle_stdout_request(struct request req, struct reply *rbuf) {
    char path_task[PATH_MAX];
    snprintf(path_task, sizeof(path_task), "task/%zu", req.content.taskid);
    // TODO fonction qui affecte à un pointeur de string le stdout du taskid dans task.h
    int fd = open(path_task, O_RDONLY);
    readstd(fd, &(rbuf->content.output));
    return 0;
}

int handle_stderr_request(struct request req, struct reply *rbuf) {
    char path_task[PATH_MAX];
    snprintf(path_task, sizeof(path_task), "task/%zu", req.content.taskid);
    // TODO fonction qui affecte à un pointeur de string le stdout du taskid dans task.h
    int fd = open(path_task, O_RDONLY);
    readstd(fd, &(rbuf->content.output));
    return 0;
}

int handle_request(int fdrequest, int fdreply) {
    struct request req;
    struct reply rep;
    if (readrequest(fdrequest, &req) == 1) return 1;
    switch (be16toh(req.opcode)) {
        case LS_OPCODE :
            handle_list_request(req, &rep);
            writereply(fdreply, &rep, LS_OPCODE);
            for (int i = 0; i < be32toh(rep.content.list.nbtasks); i++) freecmd(&(rep.content.list.tasks + i)->task_command);
            free(rep.content.list.tasks);
            return 0;
            break;
        case CR_OPCODE :
            handle_creat_request(req, &rep);
            break;
        case CB_OPCODE :
            handle_combine_request(req, &rep);
            break;
        case RM_OPCODE :
            handle_remove_request(req, &rep);
            break;
        case TX_OPCODE :
            handle_times_exitcodes_request(req, &rep);
            break;
        case SO_OPCODE :
            handle_stdout_request(req, &rep);
            break;
        case SE_OPCODE :
            handle_stderr_request(req, &rep);  
            break;
        case TM_OPCODE :
            exit(0);
            break;
    }
    return 0;
}