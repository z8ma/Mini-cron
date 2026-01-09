#include "request.h"
#include "reply.h"
#include "task.h"
#include "times_exitcodes.h"
#include "string_uint.h"

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

int handle_list_request(struct request req, struct reply *rbuf) {
    rbuf->anstype = OK_ANSTYPE;
    rbuf->content.list.nbtasks = 0;

    struct string *names = NULL;
    char path_task[PATH_MAX];

    DIR *dirp = opendir("tasks");
    if (!dirp) return 1;

    struct dirent *entry;
    while ((entry = readdir(dirp))) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && (entry->d_type == DT_DIR)) {
            names = realloc(names, (rbuf->content.list.nbtasks + 1) * sizeof(struct string));
            names[rbuf->content.list.nbtasks].length = strlen(entry->d_name);
            names[rbuf->content.list.nbtasks].data = malloc(names[rbuf->content.list.nbtasks].length + 1);
            strcpy((char *) names[rbuf->content.list.nbtasks].data, entry->d_name);
            rbuf->content.list.nbtasks++;
        }
    }
    closedir(dirp);
    insertion_sort_strings(names, rbuf->content.list.nbtasks);
    rbuf->content.list.tasks = malloc(rbuf->content.list.nbtasks * sizeof(struct task));
    for (int i = 0; i < rbuf->content.list.nbtasks; i++) {
        snprintf(path_task, sizeof(path_task), "tasks/%s", names[i].data);
        string_to_uint64(names[i], &((rbuf->content.list.tasks + i)->taskid));
        readtask_timing(path_task, &((rbuf->content.list.tasks + i)->task_timing));
        readtask_command(path_task, &((rbuf->content.list.tasks + i)->task_command));
        freestring(names + i);
    }
    
    free(names);
    return 0;
}

int handle_creat_request(struct request req, struct reply *rbuf) {
    uint64_t taskid_be = 0;
    rbuf->anstype = OK_ANSTYPE;
    int fd = open("last-taskid", O_RDWR | O_CREAT, 0644);
    struct stat st;
    if (fstat(fd, &st) < 0)
        return 1;
    if (st.st_size != 0) {
        read(fd, &taskid_be, sizeof(uint64_t));
    }
    struct task task = {be64toh(taskid_be) + 1, req.content.cr.task_timing, {SI_TYPE, {req.content.cr.content.args}}};
    char path_task[PATH_MAX];
    snprintf(path_task, sizeof(path_task), "tasks/%zu/", task.taskid);
    mkdirtask(path_task, &task);
    rbuf->content.taskid = task.taskid;
    taskid_be = htobe64(task.taskid);
    lseek(fd, 0, SEEK_SET);
    write(fd, &taskid_be, sizeof(uint64_t));
    close(fd);
    return 0;
}

int handle_combine_request(struct request req, struct reply *rbuf) {
    uint64_t taskid_be = 0;
    int fd = open("last-taskid", O_RDWR | O_CREAT, 0644);
    struct stat st;
    if (fstat(fd, &st) < 0)
        return 1;
    if (st.st_size != 0) {
        read(fd, &taskid_be, sizeof(uint64_t));
    }
    char path_tasksid[req.content.cr.content.combined.nbtasks][PATH_MAX];
    for (uint64_t i = 0; i < req.content.cr.content.combined.nbtasks; i++) {
        snprintf(path_tasksid[i], sizeof(path_tasksid), "tasks/%zu/", req.content.cr.content.combined.tasksid[i]);
        if (!access(path_tasksid[i], F_OK) == 0) {
            rbuf->anstype = ER_ANSTYPE;
            rbuf->content.errcode = NF_ERRCODE;
            return 0;
        }
    }
    struct task task = {be64toh(taskid_be) + 1, req.content.cr.task_timing, {req.content.cr.content.combined.type, {}}};
    char path_task[PATH_MAX];
    snprintf(path_task, sizeof(path_task), "tasks/%zu/", task.taskid);
    mkdirtask(path_task, &task);
    rbuf->content.taskid = task.taskid;
    taskid_be = htobe64(task.taskid);
    strcat(path_task, "cmd/");
    int len_path_task = strlen(path_task);
    for (uint64_t i = 0; i < req.content.cr.content.combined.nbtasks; i++) {
        int len_path_tasksid = strlen(path_tasksid[i]);
        strcat(path_tasksid[i], "cmd/");
        snprintf(path_task + len_path_task, sizeof(path_tasksid[i]) - len_path_task, "%zu", i);
        rename(path_tasksid[i], path_task);
        path_tasksid[i][len_path_tasksid] = '\0';
        rmdirtask(path_tasksid[i]);
    }
    ftruncate(fd, 0);
    lseek(fd, 0, SEEK_SET);
    write(fd, &taskid_be, sizeof(uint64_t));
    close(fd);
    return 0;
}

int handle_remove_request(struct request req, struct reply *rbuf) {
    struct stat st;
    char path_task[PATH_MAX];
    snprintf(path_task, sizeof(path_task), "tasks/%zu/", req.content.taskid);
    if (stat(path_task, &st) < 0) {
        rbuf->anstype = ER_ANSTYPE;
        rbuf->content.errcode = NF_ERRCODE;
        return 0;
    }
    rmdirtask(path_task);
    rbuf->anstype = OK_ANSTYPE;
    rbuf->content.taskid =  req.content.taskid;
    return 0;
}

int handle_times_exitcodes_request(struct request req, struct reply *rbuf) {
    struct stat st;
    char path_tec[PATH_MAX];
    snprintf(path_tec, sizeof(path_tec), "tasks/%zu/", req.content.taskid);
    size_t len = strlen(path_tec);
    if (stat(path_tec, &st) < 0) {
        rbuf->anstype = ER_ANSTYPE;
        rbuf->content.errcode = NF_ERRCODE;
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
    rbuf->content.tec.nbruns = 0;
    read_times_exitcodes(fd, &(rbuf->content.tec));
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

int handle_request(struct request req, struct reply *rep) {
    switch (req.opcode) {
        case LS_OPCODE :
            handle_list_request(req, rep);
            break;
        case CR_OPCODE :
            handle_creat_request(req, rep);
            break;
        case CB_OPCODE :
            handle_combine_request(req, rep);
            break;
        case RM_OPCODE :
            handle_remove_request(req, rep);
            break;
        case TX_OPCODE :
            handle_times_exitcodes_request(req, rep);
            break;
        case SO_OPCODE :
        case SE_OPCODE :
            handle_std_request(req, rep);  
            break;
        case TM_OPCODE :
            rep->anstype = OK_ANSTYPE;
            break;
    }
    return 0;
}

int handle_reply(struct reply rep, uint16_t opcode, struct string *msg) {
    int ret = 0;
    struct string rtl = {1, (uint8_t*) "\n"};
    if (rep.anstype == OK_ANSTYPE) {
        switch (opcode) {
            case LS_OPCODE :
                for (int i = 0; i < rep.content.list.nbtasks; i++) {
                    task_to_string(rep.content.list.tasks[i], msg);
                    catstring(msg, rtl);
                }
                break;
            case CR_OPCODE :
            case CB_OPCODE :
            case RM_OPCODE :
                uint_to_string(rep.content.taskid, msg);
                catstring(msg, rtl);
                break;
            case TX_OPCODE :
                times_exitcodes_to_string(rep.content.tec, msg);
                break;
            case SO_OPCODE :
            case SE_OPCODE :
                catstring(msg, rep.content.output);
                break;
            case TM_OPCODE :
                break;
        }
    } else {
        dup2(STDERR_FILENO, STDOUT_FILENO);
        switch (rep.content.errcode) {
            case NF_ERRCODE :
                catstring(msg, (struct string) {strlen("Identifiant de tâche non trouvé\n"), (uint8_t*) "Identifiant de tâche non trouvé\n"});
                break;
            case NR_ERRCODE :
                catstring(msg, (struct string) {strlen("La tâche n'a pas encore été exécutée au moins une fois\n"), (uint8_t*) "La tâche n'a pas encore été exécutée au moins une fois\n"});
                break;
        }
        ret = 1;
    }   
    return ret;
}