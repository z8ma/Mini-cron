#include "arguments.h"
#include "timing.h"
#include "command.h"
#include "reply.h"

#include <stdlib.h>
#include <unistd.h>

void freereply(struct reply *rbuf) {
    switch(rbuf->type) {
        case LS_TYPE :
            for (int i = 0; i < (rbuf->content.list.nbtasks); i++) freecmd(&((rbuf->content.list.tasks + i)->task_command));
            free(rbuf->content.list.tasks);
            break;
        case TX_TYPE :
            free(rbuf->content.replytimes_exitcodes.runs);
            break;
        case SD_TYPE :
            freestring(&(rbuf->content.output));
            break;
    }
}

int writereply(int fdreply, struct reply *rbuf) {
    if (write(fdreply, rbuf + sizeof(uint16_t), sizeof(&rbuf) - sizeof(uint16_t))) return 1;
    freereply(rbuf);
    return 0;
}