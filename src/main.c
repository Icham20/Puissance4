#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include "../include/server.h"

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;

    // Vérification de l'option -p pour choisir le port
    if (argc == 3 && strcmp(argv[1], "-p") == 0) {
        port = atoi(argv[2]);
    }

    struct user *user_list = NULL; // Liste chaînée des clients
    int server_socket = init_server_socket(port); // Création et écoute du socket serveur

    struct pollfd pollfds[101]; // Tableau pour poll (1 pour serveur + 100 max clients)
    int nfds;

    while (1) {
        nfds = 0;

        // Ajouter le socket d'écoute
        pollfds[nfds].fd = server_socket;
        pollfds[nfds].events = POLLIN;
        nfds++;

        // Ajouter tous les sockets clients
        struct user *cur = user_list;
        while (cur && nfds < 101) {
            pollfds[nfds].fd = cur->socket;
            pollfds[nfds].events = POLLIN;
            nfds++;
            cur = cur->next;
        }

        // Attente d'activité sur les sockets
        int ready = poll(pollfds, nfds, -1);
        if (ready <= 0) continue;

        // Nouvelle connexion entrante
        if (pollfds[0].revents & POLLIN) {
            handle_new_connection(server_socket, &user_list);
        }

        // Lecture des messages clients
        char buffer[LG_MESSAGE];
        for (int i = 1; i < nfds; i++) {
            if (pollfds[i].revents & POLLIN) {
                int client_fd = pollfds[i].fd;
                int n = read(client_fd, buffer, LG_MESSAGE - 1);
                if (n <= 0) {
                    supprimer_client(&user_list, client_fd); // Déconnexion client
                } else {
                    buffer[n] = '\0';
                    printf("Message reçu de %d : %s\n", client_fd, buffer); // Affichage brut
                }
            }
        }
    }

    close(server_socket);
    return 0;
}