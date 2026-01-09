#include "request.h"
#include "arguments.h"
#include "timing.h"
#include "command.h"
#include "reply.h"
#include "task.h"
#include "times_exitcodes.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int readreply(int fdreply, struct reply *rbuf, uint16_t opcode) {
    uint16_t anstype_be;
    if (read(fdreply, &anstype_be, sizeof(uint16_t)) < 0) return 1;
    rbuf->anstype = be16toh(anstype_be);
    if (rbuf->anstype == OK_ANSTYPE) {
        switch(opcode) {
            case LS_OPCODE :
                uint32_t nbtasks_be;
                if (read(fdreply, &nbtasks_be, sizeof(uint32_t)) < 0) return 1;
                rbuf->content.list.nbtasks = be32toh(nbtasks_be);
                rbuf->content.list.tasks = malloc(rbuf->content.list.nbtasks * sizeof(struct task));
                for (int i = 0; i < rbuf->content.list.nbtasks; i++) {
                    if (readtask(fdreply, rbuf->content.list.tasks + i) == 1) return 1; 
                }
                break;
            case TX_OPCODE :
                uint32_t nbruns_be;
                if (read(fdreply, &nbruns_be, sizeof(uint32_t)) < 0) return 1;
                rbuf->content.tec.nbruns = be32toh(nbruns_be); 
                if (read_times_exitcodes(fdreply, &(rbuf->content.tec)) == 1) return 1;
                break;
            case SO_OPCODE : 
            case SE_OPCODE :
                if (readstring(fdreply, &(rbuf->content.output)) == 1) return 1;
                break;
            case CR_OPCODE :
            case CB_OPCODE :
            case RM_OPCODE :
                uint64_t taskid_be;
                if (read(fdreply, &taskid_be, sizeof(uint64_t)) < 0) return 1;
                rbuf->content.taskid =  be64toh(taskid_be);
                break;
        }
    } else {
        uint16_t errcode_be;
        if (read(fdreply, &errcode_be, sizeof(uint16_t)) < 0) return 1;
        rbuf->content.errcode = be16toh(errcode_be);
    }
    return 0;
}

int writereply(int fdreply, struct reply *rbuf, uint16_t opcode) {
    uint16_t anstype_be = htobe16(rbuf->anstype);
    if (write(fdreply, &anstype_be, sizeof(uint16_t)) < 0) return 1; 
    if (rbuf->anstype == OK_ANSTYPE) {
        switch(opcode) {
            case LS_OPCODE :
                uint32_t nbtasks_be = htobe32(rbuf->content.list.nbtasks);
                if (write(fdreply, &nbtasks_be, sizeof(uint32_t)) < 0) return 1;
                for (int i = 0; i < rbuf->content.list.nbtasks; i++) {
                    if (writetask(fdreply, rbuf->content.list.tasks + i) == 1) return 1; 
                }
                break;
            case TX_OPCODE :
                if (write_times_exitcodes(fdreply, &rbuf->content.tec) == 1) return 1;
                break;
            case SO_OPCODE : 
            case SE_OPCODE :
                if (writestring(fdreply, &(rbuf->content.output)) == 1) return 1;
                break;
            case TM_OPCODE :
                break;
            default :
                uint64_t taskid_be = htobe64(rbuf->content.taskid);
                if (write(fdreply, &taskid_be, sizeof(uint64_t)) < 0) return 1;
                break;
        }
    } else {
        uint16_t errcode_be = htobe16(rbuf->content.errcode);
        if (write(fdreply, &errcode_be, sizeof(uint16_t)) < 0) return 1; 
    }
    return 0;
}

void freereply(struct reply *rbuf, uint16_t opcode) {
     if (rbuf->anstype == OK_ANSTYPE) {
        switch(opcode) {
            case LS_OPCODE :
                for (int i = 0; i < rbuf->content.list.nbtasks; i++) {
                    freecmd(&((rbuf->content.list.tasks+i)->task_command));
                }
                free(rbuf->content.list.tasks);
                break;
            case TX_OPCODE :
                free(rbuf->content.tec.runs);
                break;
            case SO_OPCODE :
            case SE_OPCODE :
                freestring( &(rbuf->content.output));
                break;
        }
    }
}