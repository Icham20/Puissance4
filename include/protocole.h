#ifndef PROTOCOLE_H   
#define PROTOCOLE_H

#include "server.h"   // Inclusion de la structure "user" et autres définitions du serveur

// ======================
// Commandes de base
// ======================

// Gère la commande /login : vérifie la validité du pseudo et met à jour l'état du joueur
int handle_login(struct user *client, const char *login, struct user *user_list);

// Gère la commande /play : vérifie la validité du coup et met à jour la grille
int handle_play(struct user *client, int colonne, struct user *user_list);


// ======================
// Fonctions liées à la grille
// ======================

// Initialise la grille avec des cases vides
void initialiser_grille(void);

// Envoie l’état actuel de la grille à tous les joueurs
void send_matrix_to_all(struct user *user_list);

// Vérifie si un joueur a aligné 4 pions consécutifs (victoire)
int verifier_victoire(char symbole);

// Vérifie si la grille est pleine (match nul)
int grille_est_pleine(void);


// ======================
// Gestion de la partie
// ======================

// Vérifie si deux joueurs sont connectés et lance la partie
void verifier_lancement_partie(struct user *user_list);

#endif 
