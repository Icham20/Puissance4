#ifndef PROTOCOLE_H
#define PROTOCOLE_H

#include "server.h"

// Commandes de base
int handle_login(struct user *client, const char *login, struct user *user_list);
int handle_play(struct user *client, int colonne, struct user *user_list);

// Grille de jeu
void initialiser_grille(void);
void send_matrix_to_all(struct user *user_list);
int verifier_victoire(char symbole);
int grille_est_pleine(void);

// Lancement automatique de la partie
void verifier_lancement_partie(struct user *user_list);

#endif