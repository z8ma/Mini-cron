#ifndef COMMUNICATION_H
#define COMMUNICATION_H

int handle_request(int fdrequest, int fdreply);
int handle_reply(int fdreply);

#endif