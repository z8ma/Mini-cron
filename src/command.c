#include "command.h"
#include "arguments.h"
#include "communication.h"
#include "times_exitcodes.h"

#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>


void insertion_sort(char **arr, size_t n) {
    for (size_t i = 1; i < n; i++) {
        char *key = arr[i];
        ssize_t j = i - 1;
        while (j >= 0 && strcmp(arr[j], key) > 0) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

int readdircmd(char *filename, struct command *cbuf) {
    struct stat st;
    int fd;
    char path[PATH_MAX];
    uint16_t type_be;
    
    if (lstat(filename, &st) < 0) return 1;
    if ((st.st_mode & S_IFMT) != S_IFDIR) return 1;

    snprintf(path, strlen(filename) + 6, "%s/type", filename);
    fd = open(path, O_RDONLY);
    if (fd < 0) return 1;
    if (read(fd, &type_be, sizeof(uint16_t)) < 0) return 1;
    cbuf->type = be16toh(type_be);
    close(fd);

    if (cbuf->type == SI_TYPE) {
        snprintf(path, strlen(filename) + 6, "%s/argv", filename);
        fd = open(path, O_RDONLY);
        if (fd < 0) return 1;
        int ret = readarguments(fd, &(cbuf->content.args));
        close(fd);
        return ret;
    } else {
        struct dirent *entry;
        cbuf->content.combined.nbcmds = 0;
        char **names = NULL;

        DIR *dirp = opendir(filename);
        if (!dirp) return 1;

        while ((entry = readdir(dirp))) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && (entry->d_type == DT_DIR)) {
                names = realloc(names, (cbuf->content.combined.nbcmds + 1) * sizeof(char *));
                names[cbuf->content.combined.nbcmds++] = (entry->d_name);
            }
        }

        if (cbuf->content.combined.nbcmds == 0) return 1;
        cbuf->content.combined.cmds = malloc(cbuf->content.combined.nbcmds * sizeof(struct command));
        if (!cbuf->content.combined.cmds) return 1;
        insertion_sort(names, cbuf->content.combined.nbcmds);

        for (int i = 0; i < cbuf->content.combined.nbcmds; i++) {
            snprintf(path, strlen(filename) + strlen(names[i]) + 2, "%s/%s", filename, names[i]);
            if (readdircmd(path, (cbuf->content.combined.cmds) + i) != 0) {
                free(names);
                closedir(dirp);
                return 1;
            }
        }
        free(names);
        closedir(dirp);
        return 0;
    }
    return 0;
}

int readcmd(int fd, struct command *cbuf) {
    uint16_t type_be;
    if (read(fd, &type_be, sizeof(uint16_t))!= sizeof(uint16_t)) return 1;
    cbuf->type = be16toh(type_be);
    if (cbuf->type== SI_TYPE) {
        if (readarguments(fd, &(cbuf->content.args)) == 1) return 1;
    } else {
        uint32_t nbcmds_be;
        if (read(fd, &nbcmds_be, sizeof(uint32_t))!= sizeof(uint32_t)) return 1;
        cbuf->content.combined.nbcmds = be32toh(nbcmds_be);
        cbuf->content.combined.cmds = malloc(cbuf->content.combined.nbcmds * sizeof(struct command));

        for (int i = 0; i < cbuf->content.combined.nbcmds; i++) {
            if (readcmd(fd, cbuf->content.combined.cmds + i) == 1) return 1;
        }
    }
    return 0;
}

int writecmd(int fd, struct command *cbuf) {
    uint16_t type_be = htobe16(cbuf->type);
    if (write(fd, &type_be, sizeof(uint16_t))!= sizeof(uint16_t)) return 1;
    if (cbuf->type == SI_TYPE) {
        if (writearguments(fd, &(cbuf->content.args)) == 1) return 1;
    } else {
        uint32_t nbcmds_be = htobe32(cbuf->content.combined.nbcmds);
        if (write(fd, &nbcmds_be, sizeof(uint32_t)) !=sizeof(uint32_t)) return 1;
        for (int i = 0; i < cbuf->content.combined.nbcmds; i++) {
            if (writecmd(fd, cbuf->content.combined.cmds + i) == 1) return 1;
        }
    }
    return 0;
}

int mkdircmd(char *pathcmd, struct command *cbuf) {
    mkdir(pathcmd, 0744);
    char type[PATH_MAX];
    snprintf(type, sizeof(type), "%s/type", pathcmd);
    int fd_type = creat(type, 0644);
    uint16_t type_be = htobe16(cbuf->type);
    if (write(fd_type, &type_be, sizeof(uint16_t))!= sizeof(uint16_t)) return 1;
    close(fd_type);
    if (cbuf->type == SI_TYPE) {
        char argument[PATH_MAX];
        snprintf(argument, sizeof(argument), "%s/argv", pathcmd);
        int fd_argument = creat(argument, 0644);
        writearguments(fd_argument, &cbuf->content.args);
    } else {
        for (int i = 0; i < cbuf->content.combined.nbcmds; i++) {
            struct string pathsubcmd = {strlen(pathcmd), (uint8_t*) pathcmd};
            uint_to_string(i, &pathsubcmd);
            mkdircmd((char*) pathsubcmd.data, cbuf->content.combined.cmds + i);
        }
    }
    return 0;
}

void freecmd(struct command *cbuf) {
    if (cbuf->type == SI_TYPE) {
        freearguments(&(cbuf->content.args));
    } else {
        uint32_t host_nbcmds = cbuf->content.combined.nbcmds;
        for(int i = 0; i < host_nbcmds; i++) {
            freecmd(&(cbuf->content.combined.cmds[i]));
        }
        free(cbuf->content.combined.cmds);
    }
}


uint16_t executecmd(struct command *cbuf) {
    pid_t p = fork();
    if (p < 0) {
        exit(1);
    } else if (p == 0) {
        switch (cbuf->type) {
            case SI_TYPE :
                exit(executearg(&cbuf->content.args));
                break;
            case SQ_TYPE :
                uint16_t finalexit = 0;
                for (uint32_t i = 0; i < cbuf->content.combined.nbcmds; i++) {
                    finalexit = executecmd(cbuf->content.combined.cmds + i);
                }
                exit(finalexit);
                break;
            case PL_TYPE : 
                finalexit = 0;
                int **fdpipe = malloc((cbuf->content.combined.nbcmds - 1) * sizeof(int*));
                for (uint32_t i = 0; i < cbuf->content.combined.nbcmds - 1; i++) {
                    fdpipe[i] = malloc(2 * sizeof(int));
                    pipe(fdpipe[i]);
                }
                for (uint32_t i = 0; i < cbuf->content.combined.nbcmds; i++) {
                    p = fork();
                    if (p < 0) {
                        exit(1);
                    } 
                    if (p == 0) {
                        if (i > 0) {
                            dup2(fdpipe[i-1][0], STDIN_FILENO);
                        }
                        if (i < cbuf->content.combined.nbcmds - 1) {
                            dup2(fdpipe[i][1], STDOUT_FILENO);
                        }

                        for (int j = 0; j < cbuf->content.combined.nbcmds - 1; j++) {
                            close(fdpipe[j][0]);
                            close(fdpipe[j][1]);
                            free(fdpipe[j]);
                        }
                        free(fdpipe);
                        finalexit = executecmd(cbuf->content.combined.cmds + i);
                        exit(finalexit);
                    }
                }
                for (int i = 0; i < cbuf->content.combined.nbcmds - 1; i++) {
                    close(fdpipe[i][0]);
                    close(fdpipe[i][1]);
                    free(fdpipe[i]);
                }
                free(fdpipe);
                waitpid(p, (int*) &finalexit, 0);
                for (int i = 0; i < cbuf->content.combined.nbcmds - 1; i++) {
                    wait(NULL);
                }
                exit(finalexit);
                break;
            case IF_TYPE :
                finalexit = 0;
                if(executecmd(cbuf->content.combined.cmds) == 0) {
                    finalexit = executecmd(cbuf->content.combined.cmds + 1);
                } else if (cbuf->content.combined.nbcmds > 2) {
                    finalexit = executecmd(cbuf->content.combined.cmds + 2);
                }
                exit(finalexit);
                break;
        }
        exit(1);
    }
    int status;
    wait(&status);
    if (WIFEXITED(status)) {
        return (uint16_t) WEXITSTATUS(status);
    }
    return 0xffff;
    
}

int command_to_string(struct command c,struct string *s) {
    struct string start = {1, (uint8_t*) "("};
    struct string space = {1, (uint8_t*) " "};
    struct string semicolon = {3, (uint8_t*) " ; "};
    struct string end = {1, (uint8_t*) ")"};
    struct string pipe = {3, (uint8_t*) " | "};
    struct string ifs = {2, (uint8_t*) "if"};
    struct string thens = {4, (uint8_t*) "then"};
    struct string elses = {4, (uint8_t*) "else"};
    struct string fis = {2, (uint8_t*) "fi"};
    switch (c.type) {
        case SI_TYPE :
            if (arguments_to_string(c.content.args, s) == 1) return 1;
            break;
        case SQ_TYPE :
            for (int i = 0; i< c.content.combined.nbcmds; i++) {
                if (c.content.combined.cmds[i].type == SQ_TYPE) {
                    if (catstring(s, start) == 1) return 1;
                }
                catstring(s, space);
                if (command_to_string(c.content.combined.cmds[i], s) == 1) return 1;
                catstring(s, space);
                if (c.content.combined.cmds[i].type == SQ_TYPE) {
                    if (catstring(s, end) == 1) return 1;
                }
                if (i != c.content.combined.nbcmds-1) {
                    if (catstring(s, semicolon) == 1) return 1;
                }
            }
            break;
        case PL_TYPE : 
            if (catstring(s, start) == 1) return 1;
            if (catstring(s, space) == 1) return 1;
            for (int i = 0; i< c.content.combined.nbcmds; i++) {
                if (command_to_string(c.content.combined.cmds[i], s) == 1) return 1;
                if (i != c.content.combined.nbcmds-1) {
                    if (catstring(s, pipe) == 1) return 1;
                }
            }
            if (catstring(s, space) == 1) return 1;
            if (catstring(s, end) == 1) return 1;
            break;
        case IF_TYPE : 
            if (catstring(s, start) == 1) return 1;
            if (catstring(s, space) == 1) return 1;
            if (catstring(s, ifs) == 1) return 1;
            if (catstring(s, space) == 1) return 1;
            if (command_to_string(c.content.combined.cmds[0], s) == 1) return 1;
            if (catstring(s, semicolon) == 1) return 1;
            if (catstring(s, thens) == 1) return 1;
            if (catstring(s, space) == 1) return 1;
            if (command_to_string(c.content.combined.cmds[1], s) == 1) return 1;
            if (c.content.combined.nbcmds > 2) {
                if (catstring(s, space) == 1) return 1;
                if (catstring(s, elses) == 1) return 1;
                if (catstring(s, space) == 1) return 1;
                if (command_to_string(c.content.combined.cmds[2], s) == 1) return 1;
            }
            if (catstring(s, semicolon) == 1) return 1;
            if (catstring(s, fis) == 1) return 1;
            if (catstring(s, space) == 1) return 1;
            if (catstring(s, end) == 1) return 1;
            break;
    }
    return 0;
}