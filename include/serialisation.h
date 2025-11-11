#include <stdint.h>

struct string {
    uint32_t length;
    uint8_t *data;
};

struct timing {
    uint64_t minutes;
    uint32_t hours;
    uint8_t daysofweek;
};

struct arguments {
    uint32_t argc;
    struct string *argv;
};

struct command {
    uint16_t type;
    struct arguments args;
    uint32_t nbcmds;
    struct command *cmd;
};

int readstring(int fd, struct string *sbuf);
void freestring(struct string *sbuf);

int readtiming(int fd, struct timing *tbuf);

int readarguments(int fd, struct arguments *abuf);
void freearguments(struct arguments *abuf);

int readcmd(char *path, struct command *cbuf);
void freecmd(struct command *cbuf);

int executecmd(struct command *cbuf);

#define SI_TYPE 0X5349  // 'SI'
#define SQ_TYPE 0X5351  // 'SQ'
#define PL_TYPE 0X504C  // 'PL'
#define IF_TYPE 0x4946  // 'IF'