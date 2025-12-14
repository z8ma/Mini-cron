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

int main(int argc, char *argv[]) {
    int opt;
    struct request req;
    req.opcode=0;
    char *custom_dir = NULL;
    while ((opt = getopt(argc, argv, "p:lx:o:e:")) != -1) {
        switch (opt) {
            case 'p':
            custom_dir = optarg; 
            break;
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

    if (req.opcode == 0) {
        fprintf(stderr, "Erreur : vous devez choisir une commande (-l, -x, -o ou -e).\n");
        return 1;
    }

    char pipes_dir[PATH_MAX];
    
    int len;
    if (custom_dir != NULL) {
        len = snprintf(pipes_dir, sizeof(pipes_dir), "%s", custom_dir);
    } else {
        len = snprintf(pipes_dir, sizeof(pipes_dir), "/tmp/%s/erraid/pipes", getenv("USER"));
    } 

    char pipes[PATH_MAX];
    snprintf(pipes, sizeof(pipes) - len, "%s/erraid-request-pipe", pipes_dir);
    int fdreq = open(pipes, O_WRONLY | O_NONBLOCK);
    if (fdreq < 0) {
        perror("Erreur ouverture pipe request");
        return 1;
    }
    writerequest(fdreq, &req);

    snprintf(pipes, sizeof(pipes) - len, "%s/erraid-reply-pipe", pipes_dir);
    int fdreply = open(pipes, O_RDONLY);
    if (fdreply < 0) {
        perror("Erreur ouverture pipe reply");
        close(fdreq);
        return 1;
    }
    int ret = handle_reply(fdreply, req.opcode);

    freerequest(&req);

    close(fdreq);
    close(fdreply);
    return ret;
}