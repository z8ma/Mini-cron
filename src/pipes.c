#include "pipes.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>

static int create_directory_recursive(const char *path)
{
    char tmp[PATH_MAX];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
    {
        tmp[len - 1] = 0;
    }

    for (p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = 0;
            if (mkdir(tmp, 0755) == -1 && errno != EEXIST)
            {
                return -1;
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, 0755) == -1 && errno != EEXIST)
    {
        return -1;
    }
    return 0;
}

int create_named_pipes(const char *pipes_dir)
{
    char request_pipe_path[PATH_MAX];
    char reply_pipe_path[PATH_MAX];

    if (create_directory_recursive(pipes_dir) == -1)
    {
        perror("create_directory_recursive");
        return -1;
    }

    get_pipe_path(pipes_dir, REQUEST_PIPE_NAME, request_pipe_path, sizeof(request_pipe_path));
    get_pipe_path(pipes_dir, REPLY_PIPE_NAME, reply_pipe_path, sizeof(reply_pipe_path));

    if (mkfifo(request_pipe_path, 0666) == -1 && errno != EEXIST)
    {
        perror("mkfifo request pipe");
        return -1;
    }

    if (mkfifo(reply_pipe_path, 0666) == -1 && errno != EEXIST)
    {
        perror("mkfifo reply pipe");
        return -1;
    }

    return 0;
}

void get_pipe_path(const char *pipes_dir, const char *pipe_name,
                   char *path_buffer, size_t buffer_size)
{
    snprintf(path_buffer, buffer_size, "%s/%s", pipes_dir, pipe_name);
}

char *get_default_pipes_dir(const char *run_directory)
{
    char *pipes_dir = malloc(PATH_MAX);
    if (!pipes_dir)
    {
        return NULL;
    }

    if (run_directory)
    {
        snprintf(pipes_dir, PATH_MAX, "%s/%s", run_directory, DEFAULT_PIPES_SUBDIR);
    }
    else
    {
        const char *user = getenv("USER");
        if (!user)
        {
            free(pipes_dir);
            return NULL;
        }
        snprintf(pipes_dir, PATH_MAX, "/tmp/%s/erraid/%s", user, DEFAULT_PIPES_SUBDIR);
    }

    return pipes_dir;
}