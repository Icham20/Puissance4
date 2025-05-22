#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "../include/protocole.h"

#define MIN_LOGIN_LEN 3
#define MAX_LOGIN_LEN 16
#define MAX_HAUTEUR 10
#define MAX_LARGEUR 10

void initialiser_grille(void);
int handle_ready(struct user *client, struct user *user_list);

char grille[MAX_HAUTEUR][MAX_LARGEUR];
int hauteur = 6;
int largeur = 7;

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

int handle_login(struct user *client, const char *login, struct user *user_list) {
    int joueur_connectes = 0;
    struct user *tmp_count = user_list;
    while (tmp_count) {
        if (strcmp(tmp_count->pseudo, "inconnu") != 0)
            joueur_connectes++;
        tmp_count = tmp_count->next;
    }

    if (joueur_connectes >= 2) {
        printf("S: /ret LOGIN:106\n");
        dprintf(client->socket, "/ret LOGIN:106\n");
        dprintf(client->socket, "/login\n");
        return 106;
    }

    int len = strlen(login);
    if (len < MIN_LOGIN_LEN || len > MAX_LOGIN_LEN || strchr(login, ':')) {
        printf("S: /ret LOGIN:105\n");
        dprintf(client->socket, "/ret LOGIN:105\n");
        dprintf(client->socket, "/login\n");
        return 105;
    }

    if (login_existe(user_list, login)) {
        printf("S: /ret LOGIN:101\n");
        dprintf(client->socket, "/ret LOGIN:101\n");
        dprintf(client->socket, "/login\n");
        return 101;
    }

    strncpy(client->pseudo, login, sizeof(client->pseudo) - 1);
    client->pseudo[sizeof(client->pseudo) - 1] = '\0';

    printf("S: /ret LOGIN:000\n");
    dprintf(client->socket, "/ret LOGIN:000\n");

    int total = 0;
    struct user *tmp = user_list;
    while (tmp) {
        total++;
        tmp = tmp->next;
    }

    int numero = client->numero;
    char role = (client->symbole == 'X') ? 'r' : 'o';

    char info_msg[128];
    snprintf(info_msg, sizeof(info_msg), "/info LOGIN:%d/%d:%s:%c\n", numero, total, client->pseudo, role);
    printf("S: %s", info_msg);
    dprintf(client->socket, "%s", info_msg);

    verifier_lancement_partie(user_list);
    return 0;
}

void verifier_lancement_partie(struct user *user_list)
{
    struct user *j1 = NULL, *j2 = NULL, *tmp = user_list;
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

    if (j1 && j2)
    {
        initialiser_grille();
        j1->etat = ETAT_JEU;
        j2->etat = ETAT_JEU;
        j1->estSonTour = 1;
        j2->estSonTour = 0;

        send_matrix_to_all(user_list);

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

void initialiser_grille()
{
    for (int i = 0; i < hauteur; i++)
        for (int j = 0; j < largeur; j++)
            grille[i][j] = '_';
}

void send_matrix_to_all(struct user *user_list)
{
    char buffer[1024] = "/info MATRIX:";
    for (int i = hauteur - 1; i >= 0; i--)
    {
        for (int j = 0; j < largeur; j++)
            strncat(buffer, &grille[i][j], 1);
        if (i > 0)
            strcat(buffer, "/");
    }
    strcat(buffer, "\n");

    printf("S: %s\n", buffer);
    struct user *tmp = user_list;
    while (tmp)
    {
        dprintf(tmp->socket, "%s", buffer);
        tmp = tmp->next;
    }
}

int grille_est_pleine()
{
    for (int i = 0; i < hauteur; i++)
        for (int j = 0; j < largeur; j++)
            if (grille[i][j] == '_')
                return 0;
    return 1;
}

int verifier_victoire(char symbole)
{
    for (int i = 0; i < hauteur; i++)
        for (int j = 0; j <= largeur - 4; j++)
            if (grille[i][j] == symbole && grille[i][j + 1] == symbole &&
                grille[i][j + 2] == symbole && grille[i][j + 3] == symbole)
                return 1;
    for (int i = 0; i <= hauteur - 4; i++)
        for (int j = 0; j < largeur; j++)
            if (grille[i][j] == symbole && grille[i + 1][j] == symbole &&
                grille[i + 2][j] == symbole && grille[i + 3][j] == symbole)
                return 1;
    for (int i = 3; i < hauteur; i++)
        for (int j = 0; j <= largeur - 4; j++)
            if (grille[i][j] == symbole && grille[i - 1][j + 1] == symbole &&
                grille[i - 2][j + 2] == symbole && grille[i - 3][j + 3] == symbole)
                return 1;
    for (int i = 0; i <= hauteur - 4; i++)
        for (int j = 0; j <= largeur - 4; j++)
            if (grille[i][j] == symbole && grille[i + 1][j + 1] == symbole &&
                grille[i + 2][j + 2] == symbole && grille[i + 3][j + 3] == symbole)
                return 1;
    return 0;
}

int handle_play(struct user *client, int col, struct user *user_list)
{
    if (client->etat != ETAT_JEU || !client->estSonTour)
    {
        printf("S: /ret PLAY:102\n");
        dprintf(client->socket, "/ret PLAY:102\n");
        return 102;
    }

    if (col < 0 || col >= largeur)
    {
        printf("S: /ret PLAY:103\n");
        dprintf(client->socket, "/ret PLAY:103\n");
        return 103;
    }

    int ligne = -1;
    for (int i = 0; i < hauteur; i++)
    {
        if (grille[i][col] == '_')
        {
            ligne = i;
            break;
        }
    }

    if (ligne == -1)
    {
        printf("S: /ret PLAY:104\n");
        dprintf(client->socket, "/ret PLAY:104\n");
        return 104;
    }

    grille[ligne][col] = client->symbole;
    printf("S: /ret PLAY:000\n");
    dprintf(client->socket, "/ret PLAY:000\n");
    send_matrix_to_all(user_list);

    if (verifier_victoire(client->symbole)) {
        struct user *tmp = user_list;
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "/info END:WIN:%s\n", client->pseudo);
        printf("S: %s\n", buffer);
        while (tmp)
        {
            dprintf(tmp->socket, "%s", buffer);
            tmp = tmp->next;
        }
    } else if (grille_est_pleine()) {
        struct user *tmp = user_list;
        printf("S: /info END:DRAW:NONE\n");
        while (tmp) {
            dprintf(tmp->socket, "/info END:DRAW:NONE\n");
            tmp = tmp->next;
        }
    } else {
        struct user *tmp = user_list;
        while (tmp)
        {
            tmp->estSonTour = !tmp->estSonTour;
            tmp = tmp->next;
        }

        struct user *turn = user_list;
        while (turn)
        {
            if (turn->estSonTour)
            {
                printf("S: /play\n");
                dprintf(turn->socket, "/play\n");
                break;
            }
            turn = turn->next;
        }
    }

    return 0;
}

int handle_ready(struct user *client, struct user *user_list)
{
    // Cette fonction est laissée vide car elle n'est plus utilisée, mais reste déclarée pour compatibilité
    return 0;
}
