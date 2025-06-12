// Inclusion des bibliothèques nécessaires
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include "../include/server.h"
#include "../include/protocole.h"

/*
  Fonction principale du serveur Puissance 4.
  Gère les connexions des clients et l'échange des messages via poll().
 */
int main(int argc, char *argv[])
{
    // Paramètres par défaut
    int port = DEFAULT_PORT;
    largeur = 7;
    hauteur = 6;

    // Lecture des arguments de la ligne de commande
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
        {
            port = atoi(argv[++i]); // Changer le port
        }
        else if (strcmp(argv[i], "-L") == 0 && i + 1 < argc)
        {
            int l = atoi(argv[++i]); // Largeur de la grille
            if (l >= 5 && l <= 10)
                largeur = l;
        }
        else if (strcmp(argv[i], "-H") == 0 && i + 1 < argc)
        {
            int h = atoi(argv[++i]); // Hauteur de la grille
            if (h >= 4 && h <= 10)
                hauteur = h;
        }
    }

    // Initialisation de la liste des utilisateurs
    struct user *user_list = NULL;

    // Création et initialisation de la socket serveur
    int server_socket = init_server_socket(port);

    // Tableau de descripteurs surveillés par poll()
    struct pollfd pollfds[101];
    int nfds;

    // Boucle principale du serveur
    while (1)
    {
        // Prépare le tableau pollfds pour cette itération
        nfds = 0;

        // Ajoute le serveur (écoute des nouvelles connexions)
        pollfds[nfds].fd = server_socket;
        pollfds[nfds].events = POLLIN;
        nfds++;

        // Ajoute tous les clients actuellement connectés
        struct user *cur = user_list;
        while (cur && nfds < 101)
        {
            pollfds[nfds].fd = cur->socket;
            pollfds[nfds].events = POLLIN;
            nfds++;
            cur = cur->next;
        }

        // Attente d'événements (nouvelle connexion ou message client)
        int ready = poll(pollfds, nfds, -1); // Bloquant
        if (ready <= 0)
            continue; // Ignore les faux signaux

        // Nouvelle connexion entrante ?
        if (pollfds[0].revents & POLLIN)
        {
            handle_new_connection(server_socket, &user_list);
        }

        // Buffer pour réceptionner les messages clients
        char buffer[LG_MESSAGE];

        // Parcours des clients connectés
        for (int i = 1; i < nfds; i++)
        {
            if (pollfds[i].revents & POLLIN)
            {
                int client_fd = pollfds[i].fd;
                int n = read(client_fd, buffer, LG_MESSAGE - 1);

                // Client déconnecté ?
                if (n <= 0)
                {
                    supprimer_client(&user_list, client_fd);
                }
                else
                {
                    // Nettoyage du message
                    buffer[n] = '\0';
                    buffer[strcspn(buffer, "\r\n")] = '\0'; // Retire retour chariot et saut de ligne

                    // Recherche du client correspondant dans la liste
                    struct user *tmp = user_list;
                    while (tmp && tmp->socket != client_fd)
                    {
                        tmp = tmp->next;
                    }

                    // Si client connu
                    if (tmp)
                    {
                        // Log du message reçu
                        printf("[REÇU] [%s | fd=%d | symbole=%c] : %s\n",
                               tmp->pseudo, tmp->socket, tmp->symbole, buffer);

                        // Traitement de la commande reçue

                        // Commande de login
                        if (strncmp(buffer, "/login", 6) == 0)
                        {
                            char pseudo[32];
                            if (sscanf(buffer, "/login %31s", pseudo) == 1)
                            {
                                handle_login(tmp, pseudo, user_list);
                            }
                            else
                            {
                                dprintf(tmp->socket, "/ret PROTO:202\n"); // Erreur de protocole
                            }
                        }
                        // Commande de jeu (jouer un coup)
                        else if (strncmp(buffer, "/play", 5) == 0)
                        {
                            int col = -1;
                            if (sscanf(buffer, "/play %d", &col) == 1)
                            {
                                handle_play(tmp, col, user_list);
                            }
                            else
                            {
                                dprintf(tmp->socket, "/ret PROTO:202\n"); // Erreur de protocole
                            }
                        }
                        // Commande inconnue
                        else
                        {
                            dprintf(tmp->socket, "/ret PROTO:201\n"); // Commande non reconnue
                        }
                    }
                    // Si client non reconnu (cas rare)
                    else
                    {
                        printf("[AVERTISSEMENT] Client inconnu (fd = %d) : %s\n", client_fd, buffer);
                    }
                }
            }
        }
    }

    // Fermeture du socket serveur (normalement jamais atteint)
    close(server_socket);
    return 0;
}
