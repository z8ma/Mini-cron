#include <cstddef>
#ifndef PIPES_H
#define PIPES_H

#define REQUEST_PIPE_NAME "erraid-request-pipe"
#define REPLY_PIPE_NAME "erraid-reply-pipe"
#define DEFAULT_PIPES_SUBDIR "pipes"

int create_named_pipes(const char *pipes_dir);

void get_pipe_path(const char *pipes_dir, const char *pipe_name, char *path_buffer, size_t buffer_size);

char *get_default_pipes_dir(const char *run_directory);

#endif