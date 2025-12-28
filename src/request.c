#include "request.h"
#include "arguments.h"
#include "task.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>


int readrequest(int fdrequest, struct request *rbuf) {
    uint16_t opcode_be;
    if (read(fdrequest, &opcode_be, sizeof(uint16_t)) < 0) return 1;
    rbuf->opcode = be16toh(opcode_be);
    switch (rbuf->opcode) {
        case CR_OPCODE :
            if (readtiming(fdrequest, &(rbuf->content.cr.task_timing)) == 1) return 1;
            if (readarguments(fdrequest, &(rbuf->content.cr.content.args)) == 1) return 1;
            break;
        case CB_OPCODE :
            if (readtiming(fdrequest, &(rbuf->content.cr.task_timing)) == 1) return 1;

            uint16_t type_be;
            if (read(fdrequest, &type_be, sizeof(uint16_t)) < 0) return 1;
            rbuf->content.cr.content.combined.type = be16toh(type_be);

            uint32_t nbtasks_be;
            if (read(fdrequest, &nbtasks_be, sizeof(uint32_t)) < 0) return 1;
            rbuf->content.cr.content.combined.nbtasks = be64toh(nbtasks_be);

            uint64_t *tasksid_be = malloc((rbuf->content.cr.content.combined.nbtasks) * sizeof(uint64_t));
            if (read(fdrequest, tasksid_be, rbuf->content.cr.content.combined.nbtasks * sizeof(uint64_t)) < 0) return 1;

            rbuf->content.cr.content.combined.tasksid = malloc(rbuf->content.cr.content.combined.nbtasks * sizeof(uint64_t));
            for (int i = 0; i < rbuf->content.cr.content.combined.nbtasks ; i++) {
                rbuf->content.cr.content.combined.tasksid[i] = be64toh(tasksid_be[i]);
            }
            free(tasksid_be);
            break;
        case LS_OPCODE :
        case TM_OPCODE :
            break;
        default :
            uint64_t taskid_be;
            if (read(fdrequest, &taskid_be, sizeof(uint64_t)) < 0) return 1;
            rbuf->content.taskid = be64toh(taskid_be);
            break;
    }
    return 0;
}

int writerequest(int fdrequest, struct request *rbuf){
    uint16_t opcode_be = htobe16(rbuf->opcode);
    if (write(fdrequest, &opcode_be, sizeof(uint16_t)) < 0) return 1;
    switch (rbuf->opcode) {
        case CR_OPCODE :
            if (writetiming(fdrequest, &(rbuf->content.cr.task_timing)) == 1) return 1;
            if (writearguments(fdrequest, &(rbuf->content.cr.content.args)) == 1) return 1;
            break;
        case CB_OPCODE :
            if (writetiming(fdrequest, &(rbuf->content.cr.task_timing)) == 1) return 1;
            uint16_t type_be = htobe16(rbuf->content.cr.content.combined.type);
            if (write(fdrequest, &type_be, sizeof(uint16_t)) < 0) return 1;

            uint32_t nbtasks_be = htobe32(rbuf->content.cr.content.combined.nbtasks);
            if (write(fdrequest, &nbtasks_be, sizeof(uint32_t)) < 0) return 1;

            uint64_t *tasksid_be = malloc((rbuf->content.cr.content.combined.nbtasks) * sizeof(uint64_t));
            for (int i = 0; i < rbuf->content.cr.content.combined.nbtasks ; i++) {
                tasksid_be[i] = htobe64(rbuf->content.cr.content.combined.tasksid[i]);
            }
            if (write(fdrequest, tasksid_be, rbuf->content.cr.content.combined.nbtasks * sizeof(uint64_t)) < 0) return 1;
            free(tasksid_be);
            break;
        case LS_OPCODE :
        case TM_OPCODE :
            break;
        default :
            uint64_t taskid_be = htobe64(rbuf->content.taskid);
            if (write(fdrequest, &taskid_be, sizeof(uint64_t)) < 0) return 1;
            break;
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