// Inclusion des bibliothèques nécessaires
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "../include/protocole.h"
#include "../include/grille.h"

// Définition des constantes
#define MIN_LOGIN_LEN 3
#define MAX_LOGIN_LEN 16
#define MAX_HAUTEUR 10
#define MAX_LARGEUR 10

// Grille de jeu (globale)
char grille[MAX_HAUTEUR][MAX_LARGEUR];
int hauteur = 6;
int largeur = 7;

/*
 Vérifie si un login est déjà utilisé par un autre joueur.
 Renvoie 1 si le login est présent, 0 sinon.
 */
int login_existe(struct user *list, const char *login)
{
    while (list)
    {
        if (strcmp(list->pseudo, login) == 0)
            return 1;
        list = list->next;
    }
    return 0;
}

/*
 Traite la commande /login d'un client.
 Vérifie la validité du login, l'unicité, affecte le symbole X/O, et lance la partie si 2 joueurs connectés.
 */
int handle_login(struct user *client, const char *login, struct user *user_list)
{
    // Vérification de la validité du login (taille + pas de ':')
    int len = strlen(login);
    if (len < MIN_LOGIN_LEN || len > MAX_LOGIN_LEN || strchr(login, ':'))
    {
        printf("S: /ret LOGIN:105\n");
        dprintf(client->socket, "/ret LOGIN:105\n");
        return 105;
    }

    // Vérification de l'unicité du login
    if (login_existe(user_list, login))
    {
        printf("S: /ret LOGIN:101\n");
        dprintf(client->socket, "/ret LOGIN:101\n");
        return 101;
    }

    // Comptage des joueurs déjà connectés
    int joueurs_connectes = 0;
    struct user *tmp = user_list;
    while (tmp)
    {
        if (strcmp(tmp->pseudo, "inconnu") != 0)
            joueurs_connectes++;
        tmp = tmp->next;
    }

    // Enregistrement du pseudo du client
    strncpy(client->pseudo, login, sizeof(client->pseudo) - 1);
    client->pseudo[sizeof(client->pseudo) - 1] = '\0';

    // Attribution du symbole
    if (joueurs_connectes == 0)
        client->symbole = 'X'; // Premier joueur
    else
        client->symbole = 'O'; // Deuxième joueur

    // Réponse positive au client
    printf("S: /ret LOGIN:000\n");
    dprintf(client->socket, "/ret LOGIN:000\n");

    // Envoi d'information complémentaire
    int numero = (client->symbole == 'X') ? 1 : 2;
    char role = (client->symbole == 'X') ? 'x' : 'o';

    char msg[128];
    snprintf(msg, sizeof(msg), "/info LOGIN:%d/2:%s:%c\n", numero, client->pseudo, role);
    printf("S: %s", msg);
    dprintf(client->socket, "%s", msg);

    // Vérification de lancement de la partie (si 2 joueurs connectés)
    verifier_lancement_partie(user_list);
    return 0;
}

/*
 Vérifie si 2 joueurs sont prêts.
 Si oui, initialise la grille, démarre la partie, et envoie /play au joueur 1.
 */
void verifier_lancement_partie(struct user *user_list)
{
    struct user *j1 = NULL, *j2 = NULL, *tmp = user_list;

    // Recherche des 2 premiers joueurs connectés
    while (tmp)
    {
        if (strcmp(tmp->pseudo, "inconnu") != 0)
        {
            if (!j1)
                j1 = tmp;
            else if (!j2)
            {
                j2 = tmp;
                break;
            }
        }
        tmp = tmp->next;
    }

    // Si les 2 joueurs sont prêts, on lance la partie
    if (j1 && j2)
    {
        initialiser_grille();
        j1->etat = ETAT_JEU;
        j2->etat = ETAT_JEU;
        j1->estSonTour = 1;
        j2->estSonTour = 0;

        send_matrix_to_all(user_list);

        // Envoi de /play au joueur dont c'est le tour
        if (j1->estSonTour)
        {
            printf("S: /play\n");
            dprintf(j1->socket, "/play\n");
        }
        else
        {
            printf("S: /play\n");
            dprintf(j2->socket, "/play\n");
        }
    }
}

/*
 Initialise la grille avec des cases vides ('_').
 */
void initialiser_grille()
{
    for (int i = 0; i < hauteur; i++)
        for (int j = 0; j < largeur; j++)
            grille[i][j] = '_';
}

/*
Envoie l'état actuel de la grille à tous les clients.
La grille est envoyée sous le format /info MATRIX:...
 */
void send_matrix_to_all(struct user *user_list)
{
    char buffer[1024] = "/info MATRIX:";

    for (int i = 0; i < hauteur; i++)
    {
        for (int j = 0; j < largeur; j++)
        {
            strncat(buffer, &grille[i][j], 1);
        }
        if (i < hauteur - 1)
            strcat(buffer, "/");
    }

    strcat(buffer, "\n");

    afficher_grille(); // Affichage côté serveur

    // Envoi à tous les clients
    struct user *tmp = user_list;
    while (tmp)
    {
        dprintf(tmp->socket, "%s", buffer);
        tmp = tmp->next;
    }
}

/*
 Vérifie si la grille est pleine (match nul).
 Retourne 1 si pleine, 0 sinon.
 */
int grille_est_pleine()
{
    for (int i = 0; i < hauteur; i++)
        for (int j = 0; j < largeur; j++)
            if (grille[i][j] == '_')
                return 0;
    return 1;
}

/*
 Vérifie s'il y a une victoire.
 Cherche 4 symboles alignés (horizontale, verticale, diagonales).
 Marque les pions gagnants en majuscules.
 */
int verifier_victoire(char symbole)
{
    char gagnant = toupper(symbole);
    char normal = tolower(symbole);

    // Convertit toute la grille en minuscule avant vérification
    for (int i = 0; i < hauteur; i++)
        for (int j = 0; j < largeur; j++)
            if (grille[i][j] == symbole || grille[i][j] == gagnant)
                grille[i][j] = normal;

    // Recherche horizontale
    for (int i = 0; i < hauteur; i++)
    {
        for (int j = 0; j <= largeur - 4; j++)
        {
            if (grille[i][j] == normal && grille[i][j + 1] == normal &&
                grille[i][j + 2] == normal && grille[i][j + 3] == normal)
            {
                grille[i][j] = gagnant;
                grille[i][j + 1] = gagnant;
                grille[i][j + 2] = gagnant;
                grille[i][j + 3] = gagnant;
                return 1;
            }
        }
    }

    // Recherche verticale
    for (int i = 0; i <= hauteur - 4; i++)
    {
        for (int j = 0; j < largeur; j++)
        {
            if (grille[i][j] == normal && grille[i + 1][j] == normal &&
                grille[i + 2][j] == normal && grille[i + 3][j] == normal)
            {
                grille[i][j] = gagnant;
                grille[i + 1][j] = gagnant;
                grille[i + 2][j] = gagnant;
                grille[i + 3][j] = gagnant;
                return 1;
            }
        }
    }

    // Recherche diagonale montante
    for (int i = 3; i < hauteur; i++)
    {
        for (int j = 0; j <= largeur - 4; j++)
        {
            if (grille[i][j] == normal && grille[i - 1][j + 1] == normal &&
                grille[i - 2][j + 2] == normal && grille[i - 3][j + 3] == normal)
            {
                grille[i][j] = gagnant;
                grille[i - 1][j + 1] = gagnant;
                grille[i - 2][j + 2] = gagnant;
                grille[i - 3][j + 3] = gagnant;
                return 1;
            }
        }
    }

    // Recherche diagonale descendante
    for (int i = 0; i <= hauteur - 4; i++)
    {
        for (int j = 0; j <= largeur - 4; j++)
        {
            if (grille[i][j] == normal && grille[i + 1][j + 1] == normal &&
                grille[i + 2][j + 2] == normal && grille[i + 3][j + 3] == normal)
            {
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

/*
Traite un coup /play d'un joueur.
Vérifie les erreurs, place le pion, met à jour la grille, vérifie victoire ou match nul, et passe le tour.
 */
int handle_play(struct user *client, int col, struct user *user_list)
{
    // Vérification que c'est bien le tour du joueur
    if (client->etat != ETAT_JEU || !client->estSonTour)
    {
        printf("S: /ret PLAY:102\n");
        dprintf(client->socket, "/ret PLAY:102\n");
        return 102;
    }

    // Vérification de la validité de la colonne
    if (col < 0 || col >= largeur)
    {
        printf("S: /ret PLAY:103\n");
        dprintf(client->socket, "/ret PLAY:103\n");
        return 103;
    }

    // Recherche de la première case vide dans la colonne
    int ligne = -1;
    for (int i = 0; i < hauteur; i++)
    {
        if (grille[i][col] == '_')
        {
            ligne = i;
            break;
        }
    }

    // Colonne pleine
    if (ligne == -1)
    {
        printf("S: /ret PLAY:104\n");
        dprintf(client->socket, "/ret PLAY:104\n");
        return 104;
    }

    // Placement du pion
    grille[ligne][col] = client->symbole;
    printf("S: /ret PLAY:000\n");
    dprintf(client->socket, "/ret PLAY:000\n");

    // Vérification victoire
    if (verifier_victoire(client->symbole))
    {
        send_matrix_to_all(user_list);

        struct user *tmp = user_list;
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "/info END:WIN:%s\n", client->pseudo);
        printf("S: Victoire de %s (%c)\n", client->pseudo, client->symbole);
        while (tmp)
        {
            dprintf(tmp->socket, "%s", buffer);
            tmp = tmp->next;
        }
    }
    // Vérification match nul
    else if (grille_est_pleine())
    {
        send_matrix_to_all(user_list);

        struct user *tmp = user_list;
        printf("S: Match nul !\n");
        while (tmp)
        {
            dprintf(tmp->socket, "/info END:DRAW:NONE\n");
            tmp = tmp->next;
        }
    }
    else
    {
        // Sinon : partie en cours → envoie nouvelle grille et passe le tour
        send_matrix_to_all(user_list);

        // Passe le tour à l'autre joueur
        struct user *tmp = user_list;
        while (tmp)
        {
            tmp->estSonTour = !tmp->estSonTour;
            tmp = tmp->next;
        }

        // Envoi /play au prochain joueur
        struct user *turn = user_list;
        while (turn)
        {
            if (turn->estSonTour)
            {
                printf("En attente du joueur %s (%c)...\n", turn->pseudo, turn->symbole);
                printf("S: /play\n");
                dprintf(turn->socket, "/play\n");
                break;
            }
            turn = turn->next;
        }
    }

    return 0;
}
