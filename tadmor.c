#include "task.h"
#include "communication.h"
#include "request.h"
#include "reply.h"

#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
    int opt;
    struct request req;
    struct reply reply;
    if ((opt = getopt(argc, argv, "lx:o:e:")) != -1) {
        switch (opt) {
        case 'l':
            req.opcode = LS_OPCODE;
            break;

        case 'x':
            req.opcode = TX_OPCODE;
            string_to_uint64(&(req.content.taskid), optarg);
            break;

        case 'o':
            req.opcode = SO_OPCODE;
            string_to_uint64(&(req.content.taskid), optarg);
            break;

        case 'e':
            req.opcode = SE_OPCODE;
            string_to_uint64(&(req.content.taskid), optarg);
            break;
        }
    }
    char pipes[PATH_MAX] = "/tmp";

    size_t len = strlen(pipes);
    snprintf(pipes + len, sizeof(pipes) - len, "/%s/erraid/pipes/", getenv("USER"));
    len = strlen(pipes);

    snprintf(pipes + len, sizeof(pipes) - len, "%s", "erraid-request-pipe");
    int fdreq = open(pipes, O_WRONLY | O_NONBLOCK);

    writerequest(fdreq, &req);

    snprintf(pipes + len, sizeof(pipes) - len, "%s", "erraid-reply-pipe");
    int fdreply = open(pipes, O_RDONLY);

    handle_reply(fdreply, req.opcode);

    freerequest(&req);

    close(fdreq);
    close(fdreply);
    return 0;
}