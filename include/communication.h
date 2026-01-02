#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "request.h"
#include "reply.h"

int handle_request(struct request req, struct reply *rep);
int handle_reply(struct reply rep, uint16_t opcode, struct string *msg);
void string_to_uint64(uint64_t *n, char *s);

#endif