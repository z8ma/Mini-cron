#include "request.h"
#include "arguments.h"
#include "task.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>


int readrequest(int fdrequest, struct request *rbuf) {
    if (read(fdrequest, &(rbuf->opcode), sizeof(uint16_t)) < 0) return 1;
    switch (be16toh(rbuf->opcode)) {
        case CR_OPCODE :
            if (readtiming(fdrequest, &(rbuf->content.cr.task_timing)) == 1) return 1;
            if (readarguments(fdrequest, &(rbuf->content.cr.content.args)) == 1) return 1;
            break;
        case CB_OPCODE :
            if (readtiming(fdrequest, &(rbuf->content.cr.task_timing)) == 1) return 1;
            if (read(fdrequest, &(rbuf->content.cr.content.combined.type), sizeof(uint16_t)) < 0) return 1;
            if (read(fdrequest, &(rbuf->content.cr.content.combined.nbtasks), sizeof(uint32_t)) < 0) return 1;
            rbuf->content.cr.content.combined.nbtasks = be64toh(rbuf->content.cr.content.combined.nbtasks);
            rbuf->content.cr.content.combined.tasksid = malloc(rbuf->content.cr.content.combined.nbtasks * sizeof(uint64_t));
            if (read(fdrequest, rbuf->content.cr.content.combined.tasksid, rbuf->content.cr.content.combined.nbtasks * sizeof(uint64_t)) < 0) return 1;
            break;
        case LS_OPCODE : break;
        default :
            if (read(fdrequest, &rbuf->content.taskid, sizeof(uint64_t)) < 0) return 1;
            break;
    }
    return 0;
}

int writerequest(int fdrequest, struct request *rbuf){
    if (write(fdrequest, &(rbuf->opcode), sizeof(uint16_t)) < 0) return 1;
    switch (be16toh(rbuf->opcode)) {
        case CR_OPCODE :
            if (writetiming(fdrequest, &(rbuf->content.cr.task_timing)) == 1) return 1;
            if (writearguments(fdrequest, &(rbuf->content.cr.content.args)) == 1) return 1;
            break;
        case CB_OPCODE :
            if (writetiming(fdrequest, &(rbuf->content.cr.task_timing)) == 1) return 1;
            if (write(fdrequest, &(rbuf->content.cr.content.combined.type), sizeof(uint16_t)) < 0) return 1;
            if (write(fdrequest, &(rbuf->content.cr.content.combined.nbtasks), sizeof(uint32_t)) < 0) return 1;
            if (write(fdrequest, rbuf->content.cr.content.combined.tasksid, rbuf->content.cr.content.combined.nbtasks * sizeof(uint64_t)) < 0) return 1;
            break;
        case LS_OPCODE :
        case TM_OPCODE :
            break;
        default :
            if (write(fdrequest, &rbuf->content.taskid, sizeof(uint64_t)) < 0) return 1;
            break;
    }
    return 0;
}

void freerequest(struct request *rbuf) {
    switch (be16toh(rbuf->opcode)) {
        case CR_OPCODE :
            freearguments(&(rbuf->content.cr.content.args));
            break;
        case CB_OPCODE :
            free(rbuf->content.cr.content.combined.tasksid);
            break;
    }
}