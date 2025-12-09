#ifndef REPLY_H
#define REPLY_H


#include "arguments.h"
#include "timing.h"
#include "command.h"

struct reply_list {
    uint32_t nbtasks;
    struct task{
    uint64_t tasksid;
    struct timing task_timing;
    struct command task_command;
    } *tasks;
};

struct times_exitcodes{
        uint64_t time;
        uint16_t exitcode;
};

struct reply_times_exitcodes {
    uint32_t nbruns;
    struct times_exitcodes *runs;
};

struct reply {
    uint16_t type;
    uint16_t anstype;
    union {
        struct reply_list list;
        uint64_t taskid;
        struct reply_times_exitcodes replytimes_exitcodes;
        struct string output;
        uint16_t errcode;
    } content;
};

int writereply(int fdreply, struct reply *rbuf);
int handle_reply(int fdreply);

#define DE_TYPE 0x4544 // 'ED'
#define LS_TYPE 0x4c53 // 'LS'
#define TX_TYPE 0x5458 // 'TX'
#define SD_TYPE 0x5344 // 'SD'

#define OK_ANSTYPE 0x4f4b // 'OK'
#define ER_ANSTYPE 0x4552 // 'ER'

#endif