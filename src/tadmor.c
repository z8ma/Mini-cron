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
    struct reply rep;
    struct string msg = {0, NULL};
    req.opcode = 0;
    char pipes[PATH_MAX];
    int popt = 0;
    while ((opt = getopt(argc, argv, "p:lx:o:e:")) != -1) {
        switch (opt) {
            case 'p':
                popt = 1;
                strcpy(pipes, optarg); 
                break;
            case 'l':
                req.opcode = LS_OPCODE;
                break;
            case 'x':
                req.opcode = TX_OPCODE;
                string_to_uint64((struct string) {strlen(optarg), (uint8_t*) strdup(optarg)} , &(req.content.taskid));
                break;

            case 'o':
                req.opcode = SO_OPCODE;
                string_to_uint64((struct string) {strlen(optarg), (uint8_t*) strdup(optarg)} , &(req.content.taskid));
                break;
            case 'e':
                req.opcode = SE_OPCODE;
                string_to_uint64((struct string) {strlen(optarg), (uint8_t*) strdup(optarg)} , &(req.content.taskid));
                break;
        }
    }

    if (req.opcode == 0) {
        fprintf(stderr, "Erreur : vous devez choisir une commande (-l, -x, -o ou -e).\n");
        return 1;
    }

    if (!popt) {
        snprintf(pipes, sizeof(pipes), "/tmp/%s/erraid/pipes", getenv("USER"));
    }
    int len = strlen(pipes); 

    strcat(pipes, "/erraid-request-pipe");
    int fdreq = open(pipes, O_WRONLY);
    pipes[len] = '\0';
    if (fdreq < 0) {
        perror("Erreur ouverture pipe request");
        return 1;
    }
    writerequest(fdreq, &req);
    close(fdreq);

    strcat(pipes, "/erraid-reply-pipe");
    int fdreply = open(pipes, O_RDONLY);
    
    if (fdreply < 0) {
        perror("Erreur ouverture pipe reply");
        return 1;
    }

    readreply(fdreply, &rep, req.opcode);
    int ret = handle_reply(rep, req.opcode, &msg);
    write(STDOUT_FILENO, msg.data, msg.length);
    
    freestring(&msg);
    freereply(&rep, req.opcode);
    freerequest(&req);

    close(fdreply);
    return ret;
}