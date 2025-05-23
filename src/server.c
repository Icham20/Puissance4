#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/server.h"
#include "../include/protocole.h"

// Compteur global pour numéroter les joueurs
static int compteur_joueur = 1;

// Compte les clients connectés avec pseudo ≠ "inconnu"
int joueurs_actifs(struct user *list)
{
    int count = 0;
    while (list)
    {
        if (strcmp(list->pseudo, "inconnu") != 0)
            count++;
        list = list->next;
    }
    return count;
}

// Initialisation du socket serveur
int init_server_socket(int port)
{
    int socketEcoute;
    struct sockaddr_in pointDeRencontreLocal;

    socketEcoute = socket(PF_INET, SOCK_STREAM, 0);
    if (socketEcoute < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    socklen_t longueurAdresse = sizeof(struct sockaddr_in);
    memset(&pointDeRencontreLocal, 0x00, longueurAdresse);
    pointDeRencontreLocal.sin_family = PF_INET;
    pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    pointDeRencontreLocal.sin_port = htons(port);

    if (bind(socketEcoute, (struct sockaddr *)&pointDeRencontreLocal, longueurAdresse) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(socketEcoute, 5) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Serveur lancé sur le port %d\n", port);
    return socketEcoute;
}

void ajouter_client(struct user **list, int clientSocket, struct sockaddr_in *addr)
{
    int count = 0;
    struct user *tmp = *list;
    while (tmp)
    {
        count++;
        tmp = tmp->next;
    }
    if (count >= 2)
    {
        dprintf(clientSocket, "Connexion refusée : 2 joueurs déjà connectés.\n");
        close(clientSocket);
        return;
    }

    struct user *nouveau = malloc(sizeof(struct user));
    if (!nouveau)
    {
        perror("malloc");
        close(clientSocket);
        return;
    }

    nouveau->socket = clientSocket;
    nouveau->sockin = *addr;
    nouveau->numero = compteur_joueur++;
    nouveau->etat = ETAT_ATTENTE;
    strcpy(nouveau->pseudo, "inconnu");
    nouveau->symbole = (nouveau->numero == 1) ? 'X' : 'O';
    nouveau->next = *list;
    *list = nouveau;

    // NOUVEL AFFICHAGE MODIFIÉ
    char msg[64];
    snprintf(msg, sizeof(msg), "/info ID: Joueur %d connecté.\n", nouveau->numero);
    dprintf(nouveau->socket, "%s", msg);
    printf("S: %s", msg);

    dprintf(nouveau->socket, "/login\n");
    printf("S: /login\n");

    verifier_lancement_partie(*list);
}

void supprimer_client(struct user **list, int client_fd)
{
    struct user *cur = *list, *prev = NULL;
    while (cur != NULL)
    {
        if (cur->socket == client_fd)
        {
            if (prev)
                prev->next = cur->next;
            else
                *list = cur->next;
            close(cur->socket);
            // Message de déconnexion
    char deco_msg[64];
    snprintf(deco_msg, sizeof(deco_msg), "S: ❌ Joueur %d déconnecté.\n", cur->numero);
    printf("%s", deco_msg);

    free(cur);

    // Vérifie s'il reste des joueurs actifs
    struct user *check = *list;
    int actifs = 0;
    while (check) {
        if (strcmp(check->pseudo, "inconnu") != 0)
            actifs++;
        check = check->next;
    }

    if (actifs == 0) {
        printf("S: 🛑 Tous les joueurs sont déconnectés. Fermeture du serveur.\n");
        exit(EXIT_SUCCESS);
    }
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

void handle_new_connection(int server_socket, struct user **user_list)
{
    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    int clientSocket = accept(server_socket, (struct sockaddr *)&clientAddr, &addrlen);

    if (clientSocket < 0)
    {
        perror("accept");
        return;
    }

    ajouter_client(user_list, clientSocket, &clientAddr);
}
