#include "command.h"
#include "timing.h"

#include <unistd.h>
#include <limits.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>

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
}