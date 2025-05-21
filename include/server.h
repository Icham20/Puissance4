#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>

#define LG_MESSAGE 256
#define DEFAULT_PORT 5000

// États possibles du joueur
#define ETAT_ATTENTE 0
#define ETAT_JEU 1
#define ETAT_TERMINE 2

// Structure d'un utilisateur connecté (client + joueur)
struct user
{
    int socket;                // File descriptor du client
    struct sockaddr_in sockin; // Adresse du client
    int numero;                // Numéro du joueur (ex : 1, 2)
    char pseudo[32];           // Pseudo du joueur
    char symbole;              // 'X' ou 'O'
    int estPret;               // 1 si le joueur est prêt
    int etat;                  // État du joueur (attente, en jeu, terminé)
    struct user *next;         // Pointeur vers le client suivant
};

// Fonctions déclarées pour server.c
int init_server_socket(int port);
void handle_new_connection(int server_socket, struct user **user_list);
void supprimer_client(struct user **list, int client_fd);
void ajouter_client(struct user **list, int clientSocket, struct sockaddr_in *addr);

#endif
