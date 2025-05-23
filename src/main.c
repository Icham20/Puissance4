
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include "../include/server.h"
#include "../include/protocole.h"

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    largeur = 7;
    hauteur = 6;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-L") == 0 && i + 1 < argc) {
            int l = atoi(argv[++i]);
            if (l >= 5 && l <= 10) largeur = l;
        } else if (strcmp(argv[i], "-H") == 0 && i + 1 < argc) {
            int h = atoi(argv[++i]);
            if (h >= 4 && h <= 10) hauteur = h;
        }
    }

    struct user *user_list = NULL;
    int server_socket = init_server_socket(port);

    struct pollfd pollfds[101];
    int nfds;

    while (1) {
        nfds = 0;
        pollfds[nfds].fd = server_socket;
        pollfds[nfds].events = POLLIN;
        nfds++;

        struct user *cur = user_list;
        while (cur && nfds < 101) {
            pollfds[nfds].fd = cur->socket;
            pollfds[nfds].events = POLLIN;
            nfds++;
            cur = cur->next;
        }

        int ready = poll(pollfds, nfds, -1);
        if (ready <= 0) continue;

        if (pollfds[0].revents & POLLIN) {
            handle_new_connection(server_socket, &user_list);
        }

        char buffer[LG_MESSAGE];
        for (int i = 1; i < nfds; i++) {
            if (pollfds[i].revents & POLLIN) {
                int client_fd = pollfds[i].fd;
                int n = read(client_fd, buffer, LG_MESSAGE - 1);
                if (n <= 0) {
                    supprimer_client(&user_list, client_fd);
                } else {
                    buffer[n] = '\0';
                    buffer[strcspn(buffer, "\r\n")] = '\0';

                    struct user *tmp = user_list;
                    while (tmp && tmp->socket != client_fd) {
                        tmp = tmp->next;
                    }

                    if (tmp) {
                        printf("[REÇU] [%s | fd=%d | symbole=%c] : %s\n",
                               tmp->pseudo, tmp->socket, tmp->symbole, buffer);

                        if (strncmp(buffer, "/login", 6) == 0) {
                            char pseudo[32];
                            if (sscanf(buffer, "/login %31s", pseudo) == 1) {
                                handle_login(tmp, pseudo, user_list);
                            } else {
                                dprintf(tmp->socket, "/ret LOGIN:105\n");
                            }
                        } else if (strncmp(buffer, "/play", 5) == 0) {
                            int col = -1;
                            if (sscanf(buffer, "/play %d", &col) == 1) {
                                handle_play(tmp, col, user_list);
                            } else {
                                dprintf(tmp->socket, "/ret PLAY:103\n");
                            }
                        }
                    } else {
                        printf("[AVERTISSEMENT] Client inconnu (fd = %d) : %s\n", client_fd, buffer);
                    }
                }
            }
        }
    }

    close(server_socket);
    return 0;
}
