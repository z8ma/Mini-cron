#ifndef REQUEST_H
#define REQUEST_H


#include "arguments.h"
#include "timing.h"

struct combine_request {
    uint16_t type;
    uint32_t nbtasks;
    uint64_t *tasksid;
};

struct create_request {
    struct timing task_timing;
    union {
        struct arguments args;
        struct combine_request combined;
    } content;
};

struct request {
    uint16_t opcode;
    union {
        struct create_request cr;
        uint64_t taskid;
    } content;
};

int handle_request(int fdrequest, int fdreply);

#define LS_OPCODE 0x4c53 // 'LS'
#define CR_OPCODE 0x4352 // 'CR'
#define CB_OPCODE 0x4342 // 'CB'
#define RM_OPCODE 0x524d // 'RM'
#define TX_OPCODE 0x5458 // 'TX'
#define SO_OPCODE 0x534f // 'SO'
#define SE_OPCODE 0x5345 // 'SE'
#define TM_OPCODE 0x4b49 // 'KI' surement pour kill

#endif