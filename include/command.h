#ifndef COMMAND_H
#define COMMAND_H

#include "arguments.h"
#include <stdint.h>


struct command {
    uint16_t type;
    union {
        struct arguments args;
        struct combined_commands {
            uint32_t nbcmds;
            struct command *cmds;
        } combined;
    } content;
};



int readcmd_path(char *path, struct command *cbuf);
int readcmd_fd(int fd, struct command *cbuf);
int writecmd(int fd, struct command *cbuf);
void freecmd(struct command *cbuf);
uint16_t executecmd(struct command *cbuf);

#define SI_TYPE 0X5349  // 'SI'
#define SQ_TYPE 0X5351  // 'SQ'
#define PL_TYPE 0X504C  // 'PL'
#define IF_TYPE 0x4946  // 'IF'

#endif