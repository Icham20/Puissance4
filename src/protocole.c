#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "../include/protocole.h"
#include "../include/grille.h"

// Constantes pour les limites de login et dimensions de la grille
#define MIN_LOGIN_LEN 3
#define MAX_LOGIN_LEN 16
#define MAX_HAUTEUR 10
#define MAX_LARGEUR 10

// Définition de la grille et ses dimensions
char grille[MAX_HAUTEUR][MAX_LARGEUR];
int hauteur = 6;
int largeur = 7;

// Vérifie si un login existe déjà dans la liste des utilisateurs
int login_existe(struct user *list, const char *login) {
    while (list) {
        if (strcmp(list->pseudo, login) == 0)
            return 1;
        list = list->next;
    }
    return 0;
}

// Gère la commande /login envoyée par un client
int handle_login(struct user *client, const char *login, struct user *user_list) {
    // Compte les joueurs connectés
    int joueur_connectes = 0;
    struct user *tmp_count = user_list;
    while (tmp_count) {
        if (strcmp(tmp_count->pseudo, "inconnu") != 0)
            joueur_connectes++;
        tmp_count = tmp_count->next;
    }

    // Refuse si deux joueurs sont déjà connectés
    if (joueur_connectes >= 2) {
        printf("S: /ret LOGIN:106\n");
        dprintf(client->socket, "/ret LOGIN:106\n");
        dprintf(client->socket, "/login\n");
        return 106;
    }

    // Vérifie la validité du login (longueur et caractères interdits)
    int len = strlen(login);
    if (len < MIN_LOGIN_LEN || len > MAX_LOGIN_LEN || strchr(login, ':')) {
        printf("S: /ret LOGIN:105\n");
        dprintf(client->socket, "/ret LOGIN:105\n");
        dprintf(client->socket, "/login\n");
        return 105;
    }

    // Vérifie l’unicité du login
    if (login_existe(user_list, login)) {
        printf("S: /ret LOGIN:101\n");
        dprintf(client->socket, "/ret LOGIN:101\n");
        dprintf(client->socket, "/login\n");
        return 101;
    }

    // Enregistre le login
    strncpy(client->pseudo, login, sizeof(client->pseudo) - 1);
    client->pseudo[sizeof(client->pseudo) - 1] = '\0';

    // Réponse positive au client
    printf("S: /ret LOGIN:000\n");
    dprintf(client->socket, "/ret LOGIN:000\n");

    // Compte total d’utilisateurs connectés
    int total = 0;
    struct user *tmp = user_list;
    while (tmp) {
        total++;
        tmp = tmp->next;
    }

    // Envoie un message d’info avec le rôle du joueur
    int numero = client->numero;
    char role = (client->symbole == 'X') ? 'x' : 'o';
    char info_msg[128];
    snprintf(info_msg, sizeof(info_msg), "/info LOGIN:%d/%d:%s:%c\n", numero, total, client->pseudo, role);
    printf("S: %s\n", info_msg);
    dprintf(client->socket, "%s", info_msg);

    // Vérifie si la partie peut être lancée
    verifier_lancement_partie(user_list);
    return 0;
}

// Vérifie si deux joueurs sont prêts, et lance la partie
void verifier_lancement_partie(struct user *user_list) {
    struct user *j1 = NULL, *j2 = NULL, *tmp = user_list;
    while (tmp) {
        if (strcmp(tmp->pseudo, "inconnu") != 0) {
            if (!j1)
                j1 = tmp;
            else if (!j2) {
                j2 = tmp;
                break;
            }
        }
        tmp = tmp->next;
    }

    if (j1 && j2) {
        initialiser_grille();
        j1->etat = ETAT_JEU;
        j2->etat = ETAT_JEU;
        j1->estSonTour = 1;
        j2->estSonTour = 0;

        send_matrix_to_all(user_list);

        if (j1->estSonTour) {
            printf("S: /play\n");
            dprintf(j1->socket, "/play\n");
        } else {
            printf("S: /play\n");
            dprintf(j2->socket, "/play\n");
        }
    }
}

// Initialise la grille avec des cases vides ('_')
void initialiser_grille() {
    for (int i = 0; i < hauteur; i++)
        for (int j = 0; j < largeur; j++)
            grille[i][j] = '_';
}

// Envoie l’état actuel de la grille à tous les clients
void send_matrix_to_all(struct user *user_list) {
    char buffer[1024] = "/info MATRIX:";

    for (int i = 0; i < hauteur; i++) {  // DU BAS VERS LE HAUT
        for (int j = 0; j < largeur; j++) {
            strncat(buffer, &grille[i][j], 1);
        }
        if (i < hauteur - 1)
            strcat(buffer, "/");
    }

    strcat(buffer, "\n");

    afficher_grille();  // Affiche la grille côté serveur

    struct user *tmp = user_list;
    while (tmp) {
        dprintf(tmp->socket, "%s", buffer);
        tmp = tmp->next;
    }
}


// Vérifie si la grille est pleine
int grille_est_pleine() {
    for (int i = 0; i < hauteur; i++)
        for (int j = 0; j < largeur; j++)
            if (grille[i][j] == '_')
                return 0;
    return 1;
}

// Vérifie s’il y a une victoire (4 symboles alignés)

int verifier_victoire(char symbole) {
    // On prépare deux versions du symbole : une minuscule pour les pions normaux, une majuscule pour les gagnants
    char gagnant = toupper(symbole);
    char normal = tolower(symbole);

    // Convertir toute la grille en minuscule avant vérification
    for (int i = 0; i < hauteur; i++)
        for (int j = 0; j < largeur; j++)
            if (grille[i][j] == symbole || grille[i][j] == gagnant)
                grille[i][j] = normal;

    // Horizontal
    for (int i = 0; i < hauteur; i++) {
        for (int j = 0; j <= largeur - 4; j++) {
            if (grille[i][j] == normal && grille[i][j + 1] == normal &&
                grille[i][j + 2] == normal && grille[i][j + 3] == normal) {
                grille[i][j] = gagnant;
                grille[i][j + 1] = gagnant;
                grille[i][j + 2] = gagnant;
                grille[i][j + 3] = gagnant;
                return 1;
            }
        }
    }

    // Vertical
    for (int i = 0; i <= hauteur - 4; i++) {
        for (int j = 0; j < largeur; j++) {
            if (grille[i][j] == normal && grille[i + 1][j] == normal &&
                grille[i + 2][j] == normal && grille[i + 3][j] == normal) {
                grille[i][j] = gagnant;
                grille[i + 1][j] = gagnant;
                grille[i + 2][j] = gagnant;
                grille[i + 3][j] = gagnant;
                return 1;
            }
        }
    }

    // Diagonale montante
    for (int i = 3; i < hauteur; i++) {
        for (int j = 0; j <= largeur - 4; j++) {
            if (grille[i][j] == normal && grille[i - 1][j + 1] == normal &&
                grille[i - 2][j + 2] == normal && grille[i - 3][j + 3] == normal) {
                grille[i][j] = gagnant;
                grille[i - 1][j + 1] = gagnant;
                grille[i - 2][j + 2] = gagnant;
                grille[i - 3][j + 3] = gagnant;
                return 1;
            }
        }
    }

    // Diagonale descendante
    for (int i = 0; i <= hauteur - 4; i++) {
        for (int j = 0; j <= largeur - 4; j++) {
            if (grille[i][j] == normal && grille[i + 1][j + 1] == normal &&
                grille[i + 2][j + 2] == normal && grille[i + 3][j + 3] == normal) {
                grille[i][j] = gagnant;
                grille[i + 1][j + 1] = gagnant;
                grille[i + 2][j + 2] = gagnant;
                grille[i + 3][j + 3] = gagnant;
                return 1;
            }
        }
    }

    return 0;
}



// Gère le coup joué par un client
int handle_play(struct user *client, int col, struct user *user_list) {
    if (client->etat != ETAT_JEU || !client->estSonTour) {
        printf("S: /ret PLAY:102\n");
        dprintf(client->socket, "/ret PLAY:102\n");
        return 102;
    }

    if (col < 0 || col >= largeur) {
        printf("S: /ret PLAY:103\n");
        dprintf(client->socket, "/ret PLAY:103\n");
        return 103;
    }

    // Trouve la première ligne vide dans la colonne
    int ligne = -1;
    for (int i = 0; i < hauteur; i++) {
        if (grille[i][col] == '_') {
            ligne = i;
            break;
        }
    }

    if (ligne == -1) {
        printf("S: /ret PLAY:104\n");
        dprintf(client->socket, "/ret PLAY:104\n");
        return 104;
    }

    // Place le symbole du joueur
    grille[ligne][col] = client->symbole;
    printf("S: /ret PLAY:000\n");
    dprintf(client->socket, "/ret PLAY:000\n");

    // ⛔️ Ne pas envoyer la grille ici (encore sans minuscule)
    // send_matrix_to_all(user_list);

    // Vérifie s’il y a un gagnant
    if (verifier_victoire(client->symbole)) {
        send_matrix_to_all(user_list);  // ✅ Envoie de la grille modifiée avec minuscules

        struct user *tmp = user_list;
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "/info END:WIN:%s\n", client->pseudo);
        printf("S: 🎉 Victoire de %s (%c)\n", client->pseudo, client->symbole);
        while (tmp) {
            dprintf(tmp->socket, "%s", buffer);
            tmp = tmp->next;
        }
    } else if (grille_est_pleine()) {
        send_matrix_to_all(user_list);  // ✅ Affiche la grille finale
        struct user *tmp = user_list;
        printf("S: ⚖️  Match nul !\n");
        while (tmp) {
            dprintf(tmp->socket, "/info END:DRAW:NONE\n");
            tmp = tmp->next;
        }
    } else {
        send_matrix_to_all(user_list);  // ✅ Affiche la grille normale s'il n'y a pas de fin de partie

        // Passe le tour à l’autre joueur
        struct user *tmp = user_list;
        while (tmp) {
            tmp->estSonTour = !tmp->estSonTour;
            tmp = tmp->next;
        }

        // Envoie la commande /play au prochain joueur
        struct user *turn = user_list;
        while (turn) {
            if (turn->estSonTour) {
                printf("🔁 En attente du joueur %s (%c)...\n", turn->pseudo, turn->symbole);
                printf("S: /play\n");
                dprintf(turn->socket, "/play\n");
                break;
            }
            turn = turn->next;
        }
    }

    return 0;
}
