#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>

#define LG_MESSAGE 256
#define DEFAULT_PORT 5000

// États possibles du joueur
#define ETAT_ATTENTE 0
#define ETAT_JEU     1
#define ETAT_TERMINE 2

// Structure d'un utilisateur connecté (client + joueur)
struct user {
    int socket;                   // Descripteur du socket client
    struct sockaddr_in sockin;   // Adresse du client
    int numero;                  // Numéro du joueur (1 ou 2)
    char pseudo[32];             // Pseudo du joueur
    char symbole;                // 'X' ou 'O'
    int etat;                    // État du joueur (attente, jeu, terminé)
    int estSonTour;             // 1 si c'est son tour de jouer
    struct user *next;          // Pointeur vers le joueur suivant
};

// Dimensions de la grille
extern int largeur;
extern int hauteur;

// Fonctions serveur
int init_server_socket(int port);
void handle_new_connection(int server_socket, struct user **user_list);
void supprimer_client(struct user **list, int client_fd);
void ajouter_client(struct user **list, int clientSocket, struct sockaddr_in *addr);

#endif
