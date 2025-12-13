#ifndef COMMUNICATION_H
#define COMMUNICATION_H

int handle_request(int fdrequest, int fdreply);
int handle_reply(int fdreply);
void string_to_uint64(uint64_t *n, char *s);

#endif