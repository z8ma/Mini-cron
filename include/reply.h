#ifndef REPLY_H
#define REPLY_H


#include "arguments.h"
#include "timing.h"
#include "command.h"

    struct reply_list {
        uint32_t nbtasks;
        struct task *tasks;
    };

    struct reply_times_exitcodes {
        uint32_t nbruns;
        struct times_exitcodes *runs;
    };

    struct reply {
        uint16_t anstype;
        union {
            struct reply_list list;
            uint64_t taskid;
            struct reply_times_exitcodes tec;
            struct string output;
            uint16_t errcode;
        } content;
    };

int readreply(int fdreply, struct reply *rbuf, uint16_t opcode);
int writereply(int fdreply, struct reply *rbuf, uint16_t opcode);
void freereply(struct reply *rbuf, uint16_t opcode);

#define OK_ANSTYPE 0x4f4b // 'OK'
#define ER_ANSTYPE 0x4552 // 'ER'

#endif