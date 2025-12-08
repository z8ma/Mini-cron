#include "request.h"
#include "arguments.h"
#include "task.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>


void insertion_sort(char **arr, size_t n) {
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

int handle_list_request(int fdreply) {
    uint16_t v = OK_ANSTYPE;
    if (write(fdreply, &v, sizeof(uint16_t)) < 0) return 1;
    uint32_t nbtasks = 0;
    uint64_t taskid;
    struct timing task_timing;
    struct command task_command;

    char **names = NULL;
    char path_task[PATH_MAX];

    DIR *dirp = opendir("task");
    if (!dirp) return 1;

    struct dirent *entry;
    while ((entry = readdir(dirp))) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && (entry->d_type == DT_DIR)) {
            names = realloc(names, (nbtasks + 1) * sizeof(char *));
            names[nbtasks++] = (entry->d_name);
        }
    }
    closedir(dirp);
    insertion_sort(names, nbtasks);
    if (write(fdreply, &nbtasks, sizeof(uint32_t)) < 0) return 1;
    for (int i = 0; i < nbtasks; i++) {
        string_to_uint64(&taskid, names[i]);
        if (write(fdreply, &taskid, sizeof(uint64_t)) < 0) return 1;
        snprintf(path_task, sizeof(path_task), "tasks/%s", names[i]);

        readtask_timing(path_task, &task_timing);
        if (write(fdreply, &task_timing, sizeof(struct timing)) < 0) return 1;

        readtask_command(path_task, &task_command);
        if (write(fdreply, &task_command, sizeof(task_command)) < 0) return 1;
    }
    freecmd(&task_command);
    return 0;
}

int handle_creat_request(int fdrequest, int fdreply) {
    uint16_t v = OK_ANSTYPE;
    if (write(fdreply, &v, sizeof(uint16_t)) < 0) return 1;
    struct timing newtask_timing;
    struct arguments newtask_command;
    readtiming(fdrequest, &newtask_timing);
    readarguments(fdrequest, &newtask_command);
    // TODO fonction qui créer une tache dans task.h et renvoie l'id de cette tache
    freearguments(&newtask_command);
    return 0;
}

int handle_combine_request(int fdrequest, int fdreply) {
    uint16_t v; //depend de la fonction à faire
    struct timing newtask_timing;
    uint16_t type;
    uint32_t nbtasks;

    readtiming(fdrequest, &newtask_timing);
    read(fdrequest, &type, sizeof(uint32_t));
    nbtasks = be32toh(nbtasks);
    uint64_t tasks[nbtasks];
    for (int i =0; i < nbtasks; i++) {
        read(fdrequest, tasks + i, sizeof(uint64_t));
    }
    // TODO fonction qui combine des taches dans task.h et renvoie l'id de la nouvelle tache
    return 0;
}

int handle_remove_request(int fdrequest, int fdreply) {
    uint16_t v; //depend de la fonction à faire
    uint16_t taskid;
    read(fdrequest, &taskid, sizeof(uint16_t));
    taskid = be16toh(taskid);
    // TODO fonction qui supprime une tache dans task.h et renvoie si la supreesion à échoué ou non
    return 0;
}

int handle_times_exitcodes_request(int fdrequest, int fdreply) {
    return 1;
}

int handle_stdout_request(int fdrequest, int fdreply) {
    return 1;
}

int handle_stderr_request(int fdrequest, int fdreply) {
    return 1;
}

int handle_terminate_request(int fdrequest, int fdreply) {
    return 1;
}


int handle_request(int fdrequest, int fdreply) {
    uint16_t opcode;
    if (read(fdrequest, &opcode, sizeof(uint16_t)) < 0) {
        return 1;
    }

    switch (opcode) {
        case LS_OPCODE :
            handle_list_request(fdreply);
            break;
        case CR_OPCODE :
            handle_creat_request(fdrequest, fdreply);
            break;
        case CB_OPCODE :
            handle_combine_request(fdrequest, fdreply);
            break;
        case RM_OPCODE :
            handle_remove_request(fdrequest, fdreply);
            break;
        case TX_OPCODE :
            handle_times_exitcodes_request(fdrequest, fdreply);
            break;
        case SO_OPCODE :
            handle_stdout_request(fdrequest, fdreply);
            break;
        case SE_OPCODE :
            handle_stderr_request(fdrequest, fdreply);  
            break;
        case TM_OPCODE :
            exit(0);
            break;
    }
    return 0;
}