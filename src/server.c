#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/server.h"
#include "../include/protocole.h"

// Compteur global pour numéroter les joueurs à l'ajout
static int compteur_joueur = 1;

// Fonction pour compter les joueurs actifs (pseudo différent de "inconnu")
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

// Initialise un socket serveur écoutant sur le port donné
int init_server_socket(int port)
{
    int socketEcoute;
    struct sockaddr_in pointDeRencontreLocal;

    // Création du socket TCP
    socketEcoute = socket(PF_INET, SOCK_STREAM, 0);
    if (socketEcoute < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Préparation de l'adresse locale pour le bind
    socklen_t longueurAdresse = sizeof(struct sockaddr_in);
    memset(&pointDeRencontreLocal, 0x00, longueurAdresse);
    pointDeRencontreLocal.sin_family = PF_INET;
    pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY); // toutes les interfaces
    pointDeRencontreLocal.sin_port = htons(port);              // port spécifié

    // Association de l'adresse au socket
    if (bind(socketEcoute, (struct sockaddr *)&pointDeRencontreLocal, longueurAdresse) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Mise en écoute du socket
    if (listen(socketEcoute, 5) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Confirmation que le serveur est lancé
    printf("Serveur lancé sur le port %d\n", port);
    return socketEcoute;
}

// Fonction d'ajout d'un nouveau client à la liste chaînée
void ajouter_client(struct user **list, int clientSocket, struct sockaddr_in *addr)
{
    int count = 0;
    struct user *tmp = *list;
    while (tmp)
    {
        count++;
        tmp = tmp->next;
    }

    // Refus de connexion si 2 joueurs déjà présents
    if (count >= 2)
    {
        dprintf(clientSocket, "Connexion refusée : 2 joueurs déjà connectés.\n");
        close(clientSocket);
        return;
    }

    // Allocation et initialisation du nouveau joueur
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
    nouveau->symbole = (nouveau->numero == 1) ? 'X' : 'O'; // premier joueur X, second O
    nouveau->next = *list;
    *list = nouveau;

    // Message d'identification au client
    char msg[64];
    snprintf(msg, sizeof(msg), "/info ID:Icham Puissance 4 v2.1\n");
    dprintf(nouveau->socket, "%s", msg);
    printf("S: %s", msg);

    // Demande de login
    dprintf(nouveau->socket, "/login\n");
    printf("S: /login\n");

    // Vérifie si la partie peut être lancée
    verifier_lancement_partie(*list);
}

// Fonction de suppression d'un client déconnecté de la liste
void supprimer_client(struct user **list, int client_fd)
{
    struct user *cur = *list, *prev = NULL;
    while (cur != NULL)
    {
        if (cur->socket == client_fd)
        {
            // Suppression du client de la liste chaînée
            if (prev)
                prev->next = cur->next;
            else
                *list = cur->next;

            // Fermeture du socket
            close(cur->socket);

            // Message de log de déconnexion
            char deco_msg[64];
            snprintf(deco_msg, sizeof(deco_msg), "S: Joueur %s déconnecté.\n", cur->pseudo);
            printf("%s", deco_msg);

            // Libération mémoire
            free(cur);

            // Vérifie s’il reste des joueurs actifs, sinon termine le serveur
            struct user *check = *list;
            int actifs = 0;
            while (check)
            {
                if (strcmp(check->pseudo, "inconnu") != 0)
                    actifs++;
                check = check->next;
            }

            if (actifs == 0)
            {
                printf("S: Tous les joueurs sont déconnectés. Fermeture du serveur.\n");
                exit(EXIT_SUCCESS);
            }

            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

// Accepte une nouvelle connexion entrante et appelle ajouter_client
void handle_new_connection(int server_socket, struct user **user_list)
{
    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);

    // Acceptation de la nouvelle connexion
    int clientSocket = accept(server_socket, (struct sockaddr *)&clientAddr, &addrlen);

    if (clientSocket < 0)
    {
        perror("accept");
        return;
    }

    // Ajout du client à la liste
    ajouter_client(user_list, clientSocket, &clientAddr);
}
