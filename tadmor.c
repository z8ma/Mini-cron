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
    if ((opt = getopt(argc, argv, "lx:o:e:")) != -1)
    {
        switch (opt)
        {
        case 'l':
            req.opcode = htobe16(LS_OPCODE);
            break;

        case 'x':
            req.opcode = htobe16(TX_OPCODE);
            string_to_uint64(&(req.content.taskid), optarg);
            break;

        case 'o':
            req.opcode = htobe16(SO_OPCODE);
            string_to_uint64(&(req.content.taskid), optarg);
            break;

        case 'e':
            req.opcode = htobe16(SE_OPCODE);
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
    readreply(fdreply, &reply, be16toh(req.opcode));
    printf("%#x\n", reply.anstype);
    printf("%d\n", be64toh(reply.content.output.length));
    write(1, reply.content.output.data, be32toh(reply.content.output.length));
    freereply(&reply, htobe16(req.opcode));
    freerequest(&req);
    close(fdreq);
    close(fdreply);
    return 0;
}