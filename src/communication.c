#include "request.h"
#include "reply.h"
#include "task.h"
#include "times_exitcodes.h"

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
    *n = *n;
}

int handle_list_request(struct request req, struct reply *rbuf) {
    rbuf->anstype = OK_ANSTYPE;
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
    struct stat st;
    char path_tec[PATH_MAX];
    snprintf(path_tec, sizeof(path_tec), "tasks/%zu/", req.content.taskid);
    size_t len = strlen(path_tec);
    if (stat(path_tec, &st) < 0) {
        rbuf->anstype = ER_ANSTYPE;
        rbuf->content.errcode = htobe16(NF_ERRCODE);
        return 0;
    }
    snprintf(path_tec + len, sizeof(path_tec) - len, "times-exitcodes");
    rbuf->anstype = OK_ANSTYPE;
    int fd = open(path_tec, O_RDONLY);

    if (fd < 0) {
        perror("Erreur open");
        return 1; 
    }

    if (fstat(fd, &st) < 0) {
        perror("erreur fstat");
        return 1;
    }
    int count = st.st_size / sizeof(struct times_exitcodes);
    rbuf->content.tec.nbruns = htobe32(count);

    rbuf->content.tec.runs = malloc(st.st_size);
    if (rbuf->content.tec.runs == NULL) {
        close(fd);
        return 1;
    }
    for (int i = 0; i < count; i++) {
        // On passe l'adresse de l'élément i du tableau : &rbuf->...runs[i]
        // On utilise la fonction wrapper qui gère l'endianness
        read_times_exitcodes(fd, &rbuf->content.tec.runs[i], 0);
    }
    close(fd);
    return 0;
}

int handle_std_request(struct request req, struct reply *rbuf) {
    struct stat st;
    char path_std[PATH_MAX];
    snprintf(path_std, sizeof(path_std), "tasks/%zu/", req.content.taskid);
    size_t len = strlen(path_std);
    if (stat(path_std, &st) < 0) {
        rbuf->anstype = ER_ANSTYPE;
        rbuf->content.errcode = NF_ERRCODE;
        return 0;
    }
    if (req.opcode == SO_OPCODE) {
        snprintf(path_std + len, sizeof(path_std) - len, "stdout");
        if (stat(path_std, &st) < 0) {
            rbuf->anstype = ER_ANSTYPE;
            rbuf->content.errcode = NR_ERRCODE;
            return 0;
    }
    } else {
        snprintf(path_std + len, sizeof(path_std) - len, "stderr");
        if (stat(path_std, &st) < 0) {
            rbuf->anstype = ER_ANSTYPE;
            rbuf->content.errcode = NR_ERRCODE;
            return 0;
        }
    }
    rbuf->anstype = OK_ANSTYPE;
    int fd = open(path_std, O_RDONLY);
    readstd(fd, &(rbuf->content.output));
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
        case SE_OPCODE :
            handle_std_request(req, &rep);  
            break;
        case TM_OPCODE :
            exit(0);
            break;
    }
    writereply(fdreply, &rep, req.opcode);
    freereply(&rep, req.opcode);
    freerequest(&req);
    return 0;
}

int handle_reply(int fdreply, uint16_t opcode) {
    struct reply rep;
    struct string msg = {0, NULL};
    readreply(fdreply, &rep, opcode);
    if (rep.anstype == OK_ANSTYPE) {
        switch (opcode) {
            case LS_OPCODE :
                struct string rtl = {1, (uint8_t*)"\n"};
                for (int i = 0; i < rep.content.list.nbtasks; i++) {
                    task_to_string(rep.content.list.tasks[i], &msg);
                    catstring(&msg, rtl);
                }
                break;
            case CR_OPCODE :
                break;
            case CB_OPCODE :
                break;
            case RM_OPCODE :
                break;
            case TX_OPCODE :
                break;
            case SO_OPCODE :
            case SE_OPCODE :
                catstring(&msg, rep.content.output);
                break;
            case TM_OPCODE :
                exit(0);
                break;
        }
    } else {
        
    }
    write(STDOUT_FILENO, msg.data, msg.length);
    freereply(&rep, opcode);
    freestring(&msg);     
    return 0;
}