#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "server.h"

// Gère la commande /login <pseudo>
int handle_login(struct user *client, const char *login, struct user *user_list);
int handle_ready(struct user *client, struct user *user_list);

#endif
