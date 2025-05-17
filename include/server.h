#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>

#define LG_MESSAGE 256
#define DEFAULT_PORT 5000

// Structure d'un utilisateur connecté (client)
struct user {
    int socket;                    // File descriptor du client
    struct sockaddr_in sockin;    // Adresse du client
    struct user *next;            // Pointeur vers le client suivant dans la liste chaînée
};

// Fonctions déclarées pour server.c
int init_server_socket(int port);
void handle_new_connection(int server_socket, struct user **user_list);
void supprimer_client(struct user **list, int client_fd);
void ajouter_client(struct user **list, int clientSocket, struct sockaddr_in *addr);

#endif