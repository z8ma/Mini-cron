#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include "serialisation.h"

int readstring(int fd, struct string *sbuf) {
    if (read(fd, sbuf, sizeof(uint32_t)) < 0) return 1;

    sbuf->length = be32toh(sbuf->length);
    sbuf->data = malloc(((sbuf->length) + 1) * sizeof(uint8_t));
    if(!sbuf->data) return 1;

    if (read(fd, sbuf->data, (sbuf->length) * sizeof(uint8_t)) < 0) return 1;
    sbuf->data[sbuf->length] ='\0';
    return 0;
}

void freestring(struct string *sbuf) {
    free(sbuf->data);
}

int readtiming(int fd, struct timing *tbuf) {
    if (read(fd, tbuf, sizeof(struct timing)) < 0) return 1;

    tbuf->minutes = be64toh(tbuf->minutes);
    tbuf->hours = be32toh(tbuf->hours);
    return 0;
}

int readarguments(int fd, struct arguments *abuf) {
    if (read(fd, abuf, sizeof(uint32_t)) < 0) return 1;

    abuf->argc = be32toh(abuf->argc);
    abuf->argv = malloc((abuf->argc) * sizeof(struct string));
    for (size_t i = 0; i < abuf->argc; i++) {
        if (readstring(fd, &abuf->argv[i]) < 0) return 1;
    }

    return 0;
}

void freearguments(struct arguments *abuf) {
    for(int i = 0; i < abuf->argc; i++) {
        freestring(&(abuf->argv[i]));
    }
    free((abuf)->argv);
}

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

int readcmd(char *filename, struct command *cbuf) {
    struct stat st;
    ssize_t fd;
    char typename[2];
    char path[PATH_MAX];
    
    if (lstat(filename, &st) < 0) return 1;
    if ((st.st_mode & S_IFMT) != S_IFDIR) return 1;

    snprintf(path, strlen(filename) + 6, "%s/type", filename);
    fd = open(path, O_RDONLY);
    if (fd < 0) return 1;
    if (read(fd, typename, 2) < 0) return 1;
    cbuf->type = (typename[0]<<8) + typename[1];
    close(fd);

    if (cbuf->type == SI_TYPE) {
        snprintf(path, strlen(filename) + 6, "%s/argv", filename);
        fd = open(path, O_RDONLY);
        if (fd < 0) return 1;
        int ret = readarguments(fd, &(cbuf->args));
        close(fd);
        return ret;
    } else if (cbuf->type == SQ_TYPE) {
        struct dirent *entry;
        cbuf->nbcmds = 0;
        char **names = NULL;

        DIR *dirp = opendir(filename);
        if (!dirp) return 1;

        while ((entry = readdir(dirp))) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && (entry->d_type == DT_DIR)) {
                names = realloc(names, (cbuf->nbcmds + 1) * sizeof(char *));
                names[cbuf->nbcmds++] = (entry->d_name);
            }
        }

        if (cbuf->nbcmds == 0) return 1;
        cbuf->cmd = malloc(cbuf->nbcmds * sizeof(struct command));
        if (!cbuf->cmd) return 1;
        insertion_sort(names, cbuf->nbcmds);

        for (int i = 0; i < cbuf->nbcmds; i++) {
            snprintf(path, strlen(filename) + strlen(names[i]) + 2, "%s/%s", filename, names[i]);
            if (readcmd(path, (cbuf->cmd) + i) != 0) {
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

void freecmd(struct command *cbuf) {
    if (cbuf->type == SI_TYPE) {
        freearguments(&(cbuf->args));
    } else if (cbuf->type == SQ_TYPE) {
        for(int i = 0; i < cbuf->nbcmds; i++) {
            freecmd(&(cbuf->cmd[i]));
        }
        free(cbuf->cmd);
    }
}

int executecmd(struct command *cbuf) {
    pid_t p = fork();
    if (p < 0) {
        exit(1);
    } else if (p == 0) {
        if (cbuf->type == SI_TYPE) {
            char **exec_argv = malloc((cbuf->args.argc + 1) * sizeof(char *));
            for (uint32_t i = 0; i < cbuf->args.argc; i++) {
                exec_argv[i] = (char *)cbuf->args.argv[i].data;
            }
            exec_argv[cbuf->args.argc] = NULL;
            execvp(exec_argv[0], exec_argv);

            perror("execvp");
            free(exec_argv);
            exit(1);
        } else if (cbuf->type == SQ_TYPE) {
            for (uint32_t i = 0; i < cbuf->nbcmds; i++) {
                executecmd(cbuf->cmd + i);
            }
            exit(0);
        }
        return 1;
    } else {
        wait(NULL);
    }
    return 0;
}