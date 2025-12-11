#include "request.h"
#include "arguments.h"
#include "timing.h"
#include "command.h"
#include "reply.h"
#include "task.h"
#include "times_exitcodes.h"

#include <stdlib.h>
#include <unistd.h>

int readreply(int fdreply, struct reply *rbuf, uint16_t opcode) {
    if (read(fdreply, &(rbuf->anstype), sizeof(uint16_t)) < 0) return 1;
    if (be16toh(rbuf->anstype) == OK_ANSTYPE) {
        switch(opcode) {
            case LS_OPCODE :
                if (read(fdreply, &(rbuf->content.list.nbtasks), sizeof(uint32_t)) < 0) return 1;
                rbuf->content.list.tasks = malloc(rbuf->content.list.nbtasks * sizeof(struct task));
                for (int i = 0; i < be32toh(rbuf->content.list.nbtasks); i++) {
                    if (readtask(fdreply, rbuf->content.list.tasks) == 1) return 1; 
                }
            break;
            case TX_OPCODE :
            if (read(fdreply, &(rbuf->content.tec.nbruns), sizeof(uint32_t)) < 0) return 1;
            rbuf->content.tec.runs = malloc(rbuf->content.tec.nbruns * sizeof(struct times_exitcodes));
            if (read_times_exitcodes(fdreply, rbuf->content.tec.runs, rbuf->content.tec.nbruns) == 1) return 1;
                break;
            case SO_OPCODE : 
            case SE_OPCODE :
            if (readstring(fdreply, &(rbuf->content.output)) == 1) return 1;
                break;
        }
    } else {
        if (read(fdreply, &(rbuf->content.errcode), sizeof(uint16_t)) < 0) return 1;
    }
    return 0;
}

int writereply(int fdreply, struct reply *rbuf, uint16_t opcode) {
    if (write(fdreply, &(rbuf->anstype), sizeof(uint16_t)) < 0) return 1; 
    if (be16toh(rbuf->anstype) == OK_ANSTYPE) {
        switch(opcode) {
            case LS_OPCODE :
                if (write(fdreply, &(rbuf->content.list.nbtasks), sizeof(uint32_t)) < 0) return 1;
                for (int i = 0; i < be32toh(rbuf->content.list.nbtasks); i++) {
                    if (writetask(fdreply, rbuf->content.list.tasks + i) == 1) return 1; 
                }
                break;
            case TX_OPCODE :
                if (write(fdreply, &(rbuf->content.tec.nbruns), sizeof(uint32_t)) < 0) return 1;
                if (write_times_exitcodes(fdreply, rbuf->content.tec.runs, rbuf->content.tec.nbruns) == 1) return 1;
                break;
            case SO_OPCODE : 
            case SE_OPCODE :
                if (writestring(fdreply, &(rbuf->content.output)) == 1) return 1;
                break;
            default :
                if (write(fdreply, &(rbuf->content.taskid), sizeof(uint64_t)) < 0) return 1;
                break;
        }
    } else {
        if (write(fdreply, &(rbuf->content.errcode), sizeof(uint16_t)) < 0) return 1; 
    }
    return 0;
}

void freereply(struct reply *rbuf, uint16_t opcode) {
     if (be16toh(rbuf->anstype) == OK_ANSTYPE) {
        switch(opcode) {
            case LS_OPCODE :
                for (int i = 0; i < be32toh(rbuf->content.list.nbtasks); i++) {
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