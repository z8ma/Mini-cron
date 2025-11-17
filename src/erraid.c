#include "command.h"
#include "timing.h"

#include <unistd.h>
#include <limits.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{

    char run_directory[PATH_MAX];
    int opt;

    if ((opt = getopt(argc, argv, "r")) != -1)
    {
        switch (opt)
        {
        case 'r':
            if (optind < argc)
            {
                strncpy(run_directory, argv[optind], PATH_MAX - 1);
                run_directory[PATH_MAX - 1] = '\0';
            }
            break;
        case '?':
            exit(1);
            break;
        }
    }
    else
    {
        char *user = getlogin();
        snprintf(run_directory, strlen(user) + 12, "/tmp/%s/erraid", user);
        mkdir(user, 0644);
        mkdir(run_directory, 0644);
    }
    if (chdir(run_directory) < 0)
    {
        if (errno == ENOENT)
        {
            perror("Ce dossier n'existe pas.");
            return 1;
        }
    }
    mkdir("tasks", 0644);

    pid_t p1 = fork();
    if (p1 < 0)
    {
        exit(1);
    }

    if (p1 == 0)
    {
        pid_t parent_pid = getppid();
        while (getppid() == parent_pid)
        {
            sleep(1);
        }
    }
    else
    {
        exit(0);
    }
    DIR *dirp = opendir("tasks");
    struct dirent *entry;
    while (1)
    {
        while (entry = readdir(dirp))
        {
            if (entry->d_type == DT_DIR)
            {
                chdir(entry->d_name);
                int fd_timing = open("timing", O_RDONLY);
                struct timing *time;
                if (readtiming(fd_timing, time))
                {
                    exit(1);
                }
                if (is_it_time(time))
                {
                    struct command *cmd;
                    if (readcmd("cmd", cmd))
                    {
                        exit(1);
                    }
                    else
                    {
                        pid_t p2 = fork();
                        if (p2 == 0)
                        {
                            int fd_out = open("stdout", O_WRONLY | O_CREAT | O_APPEND | O_EXCL, 0644);
                            int fd_err = open("stderr", O_WRONLY | O_CREAT | O_APPEND | O_EXCL, 0644);
                            int fd_exit_code = open("times-exitcodes", O_WRONLY | O_CREAT | O_TRUNC | O_EXCL, 0644);
                            dup2(fd_out, STDOUT_FILENO);
                            dup2(fd_err, STDERR_FILENO);
                            dup2(fd_exit_code, STDERR_FILENO);
                            dup2(fd_exit_code, STDOUT_FILENO);
                            executecmd(cmd);
                            exit(0);
                        }
                        else
                        {
                            wait(NULL);
                        }
                    }
                }
            }
        }
        sleep(60);
    }
}