#include "task.h"

#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>

void sleep_until_next_minute()
{
    time_t t = time(NULL);
    unsigned long seconds = difftime(t, 0);
    unsigned long sleeptime = 60 - (seconds % 60);
    sleep(sleeptime);
}

int main(int argc, char *argv[])
{
    int opt;
    if ((opt = getopt(argc, argv, "r")) != -1)
    {
        switch (opt)
        {
        case 'r':
            if (optind < argc)
            {
                if (chdir(argv[optind]) < 0)
                {
                    perror("chdir");
                    return 1;
                }
            }
            break;
        case '?':
            exit(1);
            break;
        }
    }
    else
    {
        char run_directori[PATH_MAX] = "/tmp";
        size_t len = strlen(run_directori);
        snprintf(run_directori + len, sizeof(run_directori) - len, "/%s", getenv("USER"));
        len = strlen(run_directori);

        if (mkdir(run_directori, 0744) < 0)
        {
            if (errno != EEXIST)
            {
                perror("mkdir");
                return 1;
            }
        }
        snprintf(run_directori + len, sizeof(run_directori) - len, "/%s", "erraid");
        if (mkdir(run_directori, 0744) < 0)
        {
            if (errno != EEXIST)
            {
                perror("mkdir");
                return 1;
            }
        }
        if (chdir(run_directori) < 0)
        {
            perror("chdir");
            return 1;
        }
    }

    if (mkdir("tasks", 0744) < 0)
    {
        if (errno != EEXIST)
        {
            perror("mkdir");
            return 1;
        }
    }

    ssize_t fd = open("/dev/null", O_RDWR);
    dup2(fd, STDIN_FILENO);
    close(fd);

    struct dirent *entry;
    char path_task[PATH_MAX];
    struct stat st;
    struct task t;
    while (1)
    {
        sleep_until_next_minute();
        DIR *dirp = opendir("tasks");
        if (!dirp)
        {
            perror("opendir");
            return 1;
        }
        while ((entry = readdir(dirp)))
        {
            snprintf(path_task, PATH_MAX, "%s/%s", "tasks", entry->d_name);
            if (stat(path_task, &st) < 0)
            {
                perror("stat");
                return 1;
            }
            if (S_ISDIR(st.st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            {
                readtask(entry->d_name, &t);
                executetask(&t);
                freetask(&t);
            }
        }
        closedir(dirp);
    }
    return 0;
}