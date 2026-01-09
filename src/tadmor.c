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
    char *errmsg[6] = {
        "Erreur : Vous ne pouvez pas utiliser plusieurs options consultatives différentes.\n", 
        "Erreur : Cette option prends un ou des entiers en argument.\n", 
        "Erreur : Vous ne pouvez pas utiliser plusieurs options de création.\n", 
        "Erreur : Vous ne pouvez pas utiliser une option de création et une option consultative.\n",
        "Erreur : Cette option doit être utiliser après une option de création.\n", 
        "Erreur : L'option -i doit prendre 2 ou 3 identifiant de tâche en argument.\n"
    };
    struct request req;
    req.opcode = 0;
    struct reply rep;
    struct string msg = {0, NULL};
    char pipes[PATH_MAX];
    struct string minutes = {0, NULL};
    struct string hours = {0, NULL};
    struct string daysofweek = {0, NULL};
    int opt;
    int createopt = 0;
    int consultopt = 0;
    int ropt = 0;
    int lopt = 0;
    int xopt = 0;
    int oopt = 0;
    int eopt = 0;
    int popt = 0;
    int qopt = 0;
    while ((opt = getopt(argc, argv, "c:s:p:i: m:H:d:n r: lx:o:e: P:q")) != -1) {
        switch (opt) {
            case 'c':
                if (createopt) {
                    write(STDERR_FILENO, errmsg[2], strlen(errmsg[2]));
                    return 1;
                } else if (consultopt) {
                    write(STDERR_FILENO, errmsg[3], strlen(errmsg[3]));
                    return 1;
                }
                createopt = 1;
                req.opcode = CR_OPCODE;
                optind--;
                break;
            case 's':
                if (createopt) {
                    write(STDERR_FILENO, errmsg[2], strlen(errmsg[2]));
                    return 1;
                } else if (consultopt) {
                    write(STDERR_FILENO, errmsg[3], strlen(errmsg[3]));
                    return 1;
                }
                createopt = 1;
                req.opcode = CB_OPCODE;
                req.content.cr.content.combined.type = SQ_TYPE;
                req.content.cr.content.combined.nbtasks = 0;
                optind--;
                break;
            case 'p':
                if (createopt) {
                    write(STDERR_FILENO, errmsg[2], strlen(errmsg[2]));
                    return 1;
                } else if (consultopt) {
                    write(STDERR_FILENO, errmsg[3], strlen(errmsg[3]));
                    return 1;
                }
                createopt = 1;
                req.opcode = CB_OPCODE;
                req.content.cr.content.combined.type = PL_TYPE;
                req.content.cr.content.combined.nbtasks = 0;
                optind--;
                break;
            case 'i':
                if (createopt) {
                    write(STDERR_FILENO, errmsg[2], strlen(errmsg[2]));
                    return 1;
                } else if (consultopt) {
                    write(STDERR_FILENO, errmsg[3], strlen(errmsg[3]));
                    return 1;
                }
                createopt = 1;
                req.opcode = CB_OPCODE;
                req.content.cr.content.combined.type = IF_TYPE;
                req.content.cr.content.combined.nbtasks = 0;
                optind--;
                break;
            case 'm':
                if (!createopt) {
                    write(STDERR_FILENO, errmsg[4], strlen(errmsg[4]));
                    return 1;
                }
                if (ropt) return 1;
                freestring(&minutes);
                struct string m = {strlen(optarg), (uint8_t*) strdup(optarg)};
                catstring(&minutes, m);
                break;
            case 'H':
                if (!createopt) {
                    write(STDERR_FILENO, errmsg[4], strlen(errmsg[4]));
                    return 1;
                }
                if (ropt) return 1;
                freestring(&hours);
                struct string h = {strlen(optarg), (uint8_t*) strdup(optarg)};
                catstring(&hours, h);
                break;
            case 'd':
                if (!createopt) {
                    write(STDERR_FILENO, errmsg[4], strlen(errmsg[4]));
                    return 1;
                }
                if (ropt) return 1;
                freestring(&daysofweek);
                struct string d = {strlen(optarg), (uint8_t*) strdup(optarg)};
                catstring(&daysofweek, d);
                break;
            case 'n':
                if (!createopt) {
                    write(STDERR_FILENO, errmsg[4], strlen(errmsg[4]));
                    return 1;
                }
                if (ropt) return 1;
                freestring(&minutes);
                freestring(&hours);
                freestring(&daysofweek);
                struct string n = {1, (uint8_t*) "-"};
                catstring(&minutes, n);
                catstring(&hours, n);
                catstring(&daysofweek, n);
                break;
            case 'r':
                if (createopt) {
                    write(STDERR_FILENO, errmsg[2], strlen(errmsg[2]));
                    return 1;
                } else if (consultopt) {
                    write(STDERR_FILENO, errmsg[3], strlen(errmsg[3]));
                    return 1;
                }
                createopt = 1;
                ropt = 1;
                req.opcode = RM_OPCODE;
                if (string_to_uint64((struct string) {strlen(optarg), (uint8_t*) strdup(optarg)} , &(req.content.taskid))) {
                    write(STDERR_FILENO, errmsg[1], strlen(errmsg[1]));
                    return 1;
                }
                break;
            case 'l':
                if (consultopt && !lopt) {
                    write(STDERR_FILENO, errmsg[0], strlen(errmsg[0]));
                    return 1;
                } else if (createopt) {
                    write(STDERR_FILENO, errmsg[3], strlen(errmsg[3]));
                    return 1;
                }
                consultopt = 1;
                lopt = 1;
                req.opcode = LS_OPCODE;
                break;
            case 'x':
                if (consultopt && !xopt) {
                    write(STDERR_FILENO, errmsg[0], strlen(errmsg[0]));
                    return 1;
                } else if (createopt) {
                    write(STDERR_FILENO, errmsg[3], strlen(errmsg[3]));
                    return 1;
                }
                consultopt = 1;
                xopt = 1;
                req.opcode = TX_OPCODE;
                if (string_to_uint64((struct string) {strlen(optarg), (uint8_t*) strdup(optarg)} , &(req.content.taskid))) {
                    write(STDERR_FILENO, errmsg[1], strlen(errmsg[1]));
                    return 1;
                }
                break;
            case 'o':
                if (consultopt && !oopt) {
                    write(STDERR_FILENO, errmsg[0], strlen(errmsg[0]));
                    return 1;
                } else if (createopt) {
                    write(STDERR_FILENO, errmsg[3], strlen(errmsg[3]));
                    return 1;
                }
                consultopt = 1;
                oopt = 1;
                req.opcode = SO_OPCODE;
                if (string_to_uint64((struct string) {strlen(optarg), (uint8_t*) strdup(optarg)} , &(req.content.taskid))) {
                    write(STDERR_FILENO, errmsg[1], strlen(errmsg[1]));
                    return 1;
                }
                break;
            case 'e':
                if (consultopt && !eopt) {
                    write(STDERR_FILENO, errmsg[0], strlen(errmsg[0]));
                    return 1;
                } else if (createopt) {
                    write(STDERR_FILENO, errmsg[3], strlen(errmsg[3]));
                    return 1;
                }
                consultopt = 1;
                eopt = 1;
                req.opcode = SE_OPCODE;
                if (string_to_uint64((struct string) {strlen(optarg), (uint8_t*) strdup(optarg)} , &(req.content.taskid))) {
                    write(STDERR_FILENO, errmsg[1], strlen(errmsg[1]));
                    return 1;
                }
                break;
            case 'P':
                popt = 1;
                strcpy(pipes, optarg); 
                break;
            case 'q':
                qopt = 1;
                req.opcode = TM_OPCODE;
                break;
        }

        if (createopt && optind < argc) {
            if (req.opcode == CR_OPCODE) {
                if (('a' <= argv[optind][0] && argv[optind][0] <= 'z') || ('A' <= argv[optind][0] && argv[optind][0] <= 'Z')) {
                    break;
                }
            } else if (req.opcode == CB_OPCODE && req.content.cr.content.combined.nbtasks == 0) {
                req.content.cr.content.combined.tasksid = NULL;
                while (optind < argc) {
                    uint64_t taskid;
                    struct string taskid_string = {strlen(argv[optind]), (uint8_t*) strdup(argv[optind])};
                    if (string_to_uint64(taskid_string, &taskid) == 0) {
                        req.content.cr.content.combined.nbtasks++;
                        req.content.cr.content.combined.tasksid = realloc(req.content.cr.content.combined.tasksid, req.content.cr.content.combined.nbtasks * sizeof(uint64_t));
                        req.content.cr.content.combined.tasksid[req.content.cr.content.combined.nbtasks - 1] = taskid;
                        optind++;
                    } else {
                        break;
                    }
                }
                if (minutes.length > 0 || hours.length > 0 || daysofweek.length > 0) break;
            }
        }

        if (qopt) {
            break;
        }
    }

    if (req.opcode == 0) {
        fprintf(stderr, "Erreur : vous devez choisir une commande (-l, -x, -o, -e ou -q).\n");
        return 1;
    }

    if (createopt && !ropt) {
        if (minutes.length == 0) {
        catstring(&minutes, (struct string) {1, (uint8_t*) "*"});
        }
        if (hours.length == 0) {
            catstring(&hours, (struct string) {1, (uint8_t*) "*"});
        }
        if (daysofweek.length == 0) {
            catstring(&daysofweek, (struct string) {1, (uint8_t*) "*"});
        }
        req.content.cr.task_timing.minutes = 0;
        req.content.cr.task_timing.hours = 0;
        req.content.cr.task_timing.daysofweek = 0;
        string_to_timing(minutes, hours, daysofweek, &req.content.cr.task_timing);
        if (req.opcode == CR_OPCODE) {
            if (argc <= optind) {
                write(STDERR_FILENO, errmsg[5], strlen(errmsg[5]));
                return 1;
            }
            req.content.cr.content.args.argc = 0;
            req.content.cr.content.args.argv = NULL;
            for (int i = optind; i < argc; i++) {
                struct string arg = {strlen(argv[i]), (uint8_t*) strdup(argv[i])};
                req.content.cr.content.args.argc++;
                req.content.cr.content.args.argv = realloc(req.content.cr.content.args.argv, req.content.cr.content.args.argc * sizeof(struct string));
                req.content.cr.content.args.argv[req.content.cr.content.args.argc - 1] = arg;
            }
        } else {
            if (req.content.cr.content.combined.nbtasks == 0 && optind < argc) {
                req.content.cr.content.combined.tasksid = NULL;
                while (optind < argc) {
                    uint64_t taskid;
                    struct string taskid_string = {strlen(argv[optind]), (uint8_t*) strdup(argv[optind])};
                    if (string_to_uint64(taskid_string, &taskid) == 0) {
                        req.content.cr.content.combined.nbtasks++;
                        req.content.cr.content.combined.tasksid = realloc(req.content.cr.content.combined.tasksid, req.content.cr.content.combined.nbtasks * sizeof(uint64_t));
                        req.content.cr.content.combined.tasksid[req.content.cr.content.combined.nbtasks - 1] = taskid;
                        optind++;
                    }
                }
            }
            if (req.content.cr.content.combined.type == IF_TYPE && (req.content.cr.content.combined.nbtasks < 2 || req.content.cr.content.combined.nbtasks > 3)) {
                write(STDERR_FILENO, errmsg[5], strlen(errmsg[5]));
                return 1;
            }
        }
    }

    if (!popt) {
        snprintf(pipes, sizeof(pipes), "/tmp/%s/erraid/pipes", getenv("USER"));
    }
    int len = strlen(pipes); 

    strcat(pipes, "/erraid-request-pipe");
    int fdreq = open(pipes, O_WRONLY | O_NONBLOCK);
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