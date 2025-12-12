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

int readcmd_path(char *filename, struct command *cbuf) {
    struct stat st;
    ssize_t fd;
    char path[PATH_MAX];
    
    if (lstat(filename, &st) < 0) return 1;
    if ((st.st_mode & S_IFMT) != S_IFDIR) return 1;

    snprintf(path, strlen(filename) + 6, "%s/type", filename);
    fd = open(path, O_RDONLY);
    if (fd < 0) return 1;
    if (read(fd, &(cbuf->type), sizeof(uint16_t)) < 0) return 1;
    close(fd);

    if (be16toh(cbuf->type) == SI_TYPE) {
        snprintf(path, strlen(filename) + 6, "%s/argv", filename);
        fd = open(path, O_RDONLY);
        if (fd < 0) return 1;
        int ret = readarguments(fd, &(cbuf->content.args));
        close(fd);
        return ret;
    } else if (be16toh(cbuf->type) == SQ_TYPE) {
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
            if (readcmd_path(path, (cbuf->content.combined.cmds) + i) != 0) {
                free(names);
                closedir(dirp);
                return 1;
            }
        }
        cbuf->content.combined.nbcmds = htobe32(cbuf->content.combined.nbcmds);
        free(names);
        closedir(dirp);
        return 0;
    }
    return 0;
}

int readcmd_fd(int fd, struct command *cbuf) {
    if (read(fd, &(cbuf->type), sizeof(uint16_t))!= sizeof(uint16_t)) return 1;
    cbuf->type = be16toh(cbuf->type);
    if (cbuf->type== SI_TYPE) {
        if (readarguments(fd, &(cbuf->content.args)) == 1) return 1;

    } else if (cbuf->type == SQ_TYPE) {

        uint32_t nbcmds_be;
        if (read(fd, &nbcmds_be, sizeof(uint32_t))!= sizeof(uint32_t)) return 1;
        cbuf->content.combined.nbcmds = be32toh(nbcmds_be);
        uint32_t host_nbcmds = cbuf->content.combined.nbcmds;
        cbuf->content.combined.cmds = malloc(host_nbcmds * sizeof(struct command));

        for (int i = 0; i < host_nbcmds; i++) {
            if (readcmd_fd(fd, cbuf->content.combined.cmds + i) == 1) return 1;
        }
    }
    return 0;
}

int writecmd(int fd, struct command *cbuf) {
    uint16_t type_be = htobe16(cbuf->type);
    if (write(fd, &type_be, sizeof(uint16_t))!= sizeof(uint16_t)) return 1;
    if (cbuf->type == SI_TYPE) {
        if (writearguments(fd, &(cbuf->content.args)) == 1) return 1;
    } else if (cbuf->type == SQ_TYPE) {
        uint32_t nbcmds_be = htobe32(cbuf->content.combined.nbcmds);
        if (write(fd, &nbcmds_be, sizeof(uint32_t)) !=sizeof(uint32_t)) return 1;

        uint32_t host_nbcmds = cbuf->content.combined.nbcmds;
        for (int i = 0; i < host_nbcmds; i++) {
            if (writecmd(fd, cbuf->content.combined.cmds + i) == 1) return 1;
        }
    }
    return 0;
}

void freecmd(struct command *cbuf) {
    if (cbuf->type == SI_TYPE) {
        freearguments(&(cbuf->content.args));
    } else if (cbuf->type == SQ_TYPE) {
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
        if (cbuf->type == SI_TYPE) {
            exit(executearg(&cbuf->content.args));
        } else if (cbuf->type == SQ_TYPE) {
            uint16_t finalexit = 0;
            uint32_t nbcmds_host = cbuf->content.combined.nbcmds;
            for (uint32_t i = 0; i < nbcmds_host; i++) {
                finalexit = executecmd(cbuf->content.combined.cmds + i);
            }
            exit(finalexit);
        }
        exit(1);
    } else {
        int status;
        wait(&status);
        if (WIFEXITED(status)) {
            return (uint16_t) WEXITSTATUS(status);
        }
        return 0xffff;
    }
}
