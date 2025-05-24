#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>  // Pour struct sockaddr_in et les constantes réseau

// ======================
// Constantes
// ======================

#define LG_MESSAGE 256        // Taille maximale d’un message échangé
#define DEFAULT_PORT 5000     // Port par défaut si aucun n’est précisé

// États possibles d’un joueur
#define ETAT_ATTENTE 0        // Le joueur est connecté mais attend un autre joueur
#define ETAT_JEU     1        // Le joueur est en train de jouer
#define ETAT_TERMINE 2        // La partie est terminée pour ce joueur


// ======================
// Structure de données : utilisateur
// ======================

// Représente un joueur connecté au serveur
struct user {
    int socket;                   // Descripteur de fichier pour la connexion du client
    struct sockaddr_in sockin;   // Informations réseau sur le client
    int numero;                  // Numéro attribué au joueur (1 ou 2)
    char pseudo[32];             // Pseudo du joueur (fourni avec /login)
    char symbole;                // Symbole attribué : 'X' ou 'O'
    int etat;                    // État du joueur (attente, jeu, terminé)
    int estSonTour;             // Booléen : 1 si c'est à ce joueur de jouer
    struct user *next;           // Pointeur vers le joueur suivant (liste chaînée)
};


// ======================
// Variables globales
// ======================

// Dimensions configurables de la grille (modifiées par les arguments -L et -H)
extern int largeur;
extern int hauteur;


// ======================
// Fonctions du serveur
// ======================

// Initialise un socket serveur sur le port spécifié et retourne le descripteur
int init_server_socket(int port);

// Gère une nouvelle connexion entrante (accept) et l’ajoute à la liste des clients
void handle_new_connection(int server_socket, struct user **user_list);

// Supprime un joueur de la liste en cas de déconnexion
void supprimer_client(struct user **list, int client_fd);

// Ajoute un nouveau joueur dans la liste chaînée et initialise ses données
void ajouter_client(struct user **list, int clientSocket, struct sockaddr_in *addr);

#endif 
