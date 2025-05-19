#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/server.h"

// Compteur global pour numéroter les joueurs
static int compteur_joueur = 1;

// Initialisation du socket serveur
int init_server_socket(int port) {
    int socketEcoute;
    struct sockaddr_in pointDeRencontreLocal;

    // Création du socket TCP
    socketEcoute = socket(PF_INET, SOCK_STREAM, 0);
    if (socketEcoute < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    socklen_t longueurAdresse = sizeof(struct sockaddr_in);
    memset(&pointDeRencontreLocal, 0x00, longueurAdresse);
    pointDeRencontreLocal.sin_family = PF_INET;
    pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    pointDeRencontreLocal.sin_port = htons(port);

    // Attachement à l'adresse locale
    if (bind(socketEcoute, (struct sockaddr *)&pointDeRencontreLocal, longueurAdresse) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Mise en écoute
    if (listen(socketEcoute, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Serveur lancé sur le port %d\n", port);
    return socketEcoute;
}

// Ajoute un client à la liste chaînée
void ajouter_client(struct user **list, int clientSocket, struct sockaddr_in *addr) {
    struct user *nouveau = malloc(sizeof(struct user));
    if (!nouveau) {
        perror("malloc");
        close(clientSocket);
        return;
    }

    nouveau->socket = clientSocket;
    nouveau->sockin = *addr;
    nouveau->numero = compteur_joueur++;  // Numérotation du joueur
    nouveau->next = *list;
    *list = nouveau;

    printf("[Joueur %d] connecté (fd = %d)\n", nouveau->numero, clientSocket);
}

// Supprime un client de la liste chaînée
void supprimer_client(struct user **list, int client_fd) {
    struct user *cur = *list, *prev = NULL;
    while (cur != NULL) {
        if (cur->socket == client_fd) {
            if (prev) prev->next = cur->next;
            else *list = cur->next;

            printf("[Joueur %d] déconnecté (fd = %d)\n", cur->numero, client_fd);

            close(cur->socket);
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

// Gère une nouvelle connexion entrante
void handle_new_connection(int server_socket, struct user **user_list) {
    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    int clientSocket = accept(server_socket, (struct sockaddr *)&clientAddr, &addrlen);

    if (clientSocket < 0) {
        perror("accept");
        return;
    }

    ajouter_client(user_list, clientSocket, &clientAddr);
}
