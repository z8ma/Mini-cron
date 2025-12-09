#include "request.h"
#include "arguments.h"
#include "task.h"
#include "reply.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>


int readrequest(int fdrequest, struct request *rbuf) {
    if (read(fdrequest, &(rbuf->opcode), sizeof(uint16_t)) < 0) return 1;
    switch (rbuf->opcode) {
        case CR_OPCODE :
            if (readtiming(fdrequest, &(rbuf->content.cr.task_timing)) == 1) return 1;
            if (readarguments(fdrequest, &(rbuf->content.cr.content.args)) == 1) return 1;
            break;
        case CB_OPCODE :
            if (readtiming(fdrequest, &(rbuf->content.cr.task_timing)) == 1) return 1;
            if (read(fdrequest, &(rbuf->content.cr.content.combined.type), sizeof(uint16_t)) < 0) return 1;
            if (read(fdrequest, &(rbuf->content.cr.content.combined.nbtasks), sizeof(uint32_t)) < 0) return 1;
            rbuf->content.cr.content.combined.tasksid = malloc(be64toh(rbuf->content.cr.content.combined.nbtasks) * sizeof(uint64_t));
            if (read(fdrequest, rbuf->content.cr.content.combined.tasksid, be64toh(rbuf->content.cr.content.combined.nbtasks) * sizeof(uint64_t)) < 0) return 1;
            break;
        default :
            if (read(fdrequest, &rbuf->content.taskid, sizeof(uint64_t)) < 0) return 1;
    }
    return 0;
}

void freerequest(struct request *rbuf) {
    switch (rbuf->opcode) {
        case CR_OPCODE :
            freearguments(&(rbuf->content.cr.content.args));
            break;
        case CB_OPCODE :
            free(rbuf->content.cr.content.combined.tasksid);
            break;
    }
}

void insertion_sort2(char **arr, size_t n) {
    for (size_t i = 1; i < n; i++) {
        char *key = arr[i];
        ssize_t j = i - 1;
        while (j >= 0 && strcmp(arr[j], key) > 0) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

void string_to_uint64(uint64_t *n, char *s) {
    while (*s) {
        *n = (*n) * 10 + (*s - '0');
        s++;
    }
}

int handle_list_request(struct request req, struct reply *rbuf) {
    rbuf->type = LS_TYPE;
    rbuf->anstype = OK_ANSTYPE;
    rbuf->content.list.nbtasks = 0;

    char **names = NULL;
    char path_task[PATH_MAX];

    DIR *dirp = opendir("task");
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
    rbuf->content.list.tasks = malloc(rbuf->content.list.nbtasks * sizeof(rbuf->content.list.tasks));
    for (int i = 0; i < rbuf->content.list.nbtasks; i++) {
        snprintf(path_task, sizeof(path_task), "tasks/%s", names[i]);

        string_to_uint64(&((rbuf->content.list.tasks + i)->tasksid), names[i]);
        readtask_timing(path_task, &((rbuf->content.list.tasks + i)->task_timing));
        readtask_command(path_task,  &((rbuf->content.list.tasks + i)->task_command));
    }
    return 0;
}

int handle_creat_request(struct request req, struct reply *rbuf) {
    rbuf->type = DE_TYPE;
    rbuf->anstype = OK_ANSTYPE;
    // TODO fonction qui créer une tache dans task.h et renvoie l'id de cette tache
    return 0;
}

int handle_combine_request(struct request req, struct reply *rbuf) {
    rbuf->type = DE_TYPE;
    // TODO fonction qui combine des taches dans task.h et renvoie l'id de la nouvelle tache
    return 0;
}

int handle_remove_request(struct request req, struct reply *rbuf) {
    rbuf->type = DE_TYPE;
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
    return 0;
}

int handle_stderr_request(struct request req, struct reply *rbuf) {
    char path_task[PATH_MAX];
    snprintf(path_task, sizeof(path_task), "task/%zu", req.content.taskid);
    // TODO fonction qui affecte à un pointeur de string le stdout du taskid dans task.h
    return 0;
}

int handle_request(int fdrequest, int fdreply) {
    struct request req;
    struct reply rep;
    if (readrequest(fdrequest, &req) == 1) return 1;
    switch (req.opcode) {
        case LS_OPCODE :
            handle_list_request(req, &rep);
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
    if (writereply(fdreply, &rep) == 1) return 1;
    freerequest(&req);
    return 0;
}