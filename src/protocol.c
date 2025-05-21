#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "../include/protocol.h"

#define MIN_LOGIN_LEN 3
#define MAX_LOGIN_LEN 16

int login_existe(struct user *list, const char *login) {
    while (list) {
        if (strcmp(list->pseudo, login) == 0) {
            return 1;
        }
        list = list->next;
    }
    return 0;
}

int handle_login(struct user *client, const char *login, struct user *user_list) {
    // Vérifie combien de joueurs ont vraiment un pseudo ≠ "inconnu"
    int joueur_connectes = 0;
    struct user *tmp_count = user_list;
    while (tmp_count) {
        if (strcmp(tmp_count->pseudo, "inconnu") != 0) {
            joueur_connectes++;
        }
        tmp_count = tmp_count->next;
    }

    if (joueur_connectes >= 2) {
        dprintf(client->socket, "/ret LOGIN:106\n");
        return 106;
    }

    int len = strlen(login);
    if (len < MIN_LOGIN_LEN || len > MAX_LOGIN_LEN || strchr(login, ':')) {
        dprintf(client->socket, "/ret LOGIN:105\n");
        return 105;
    }

    if (login_existe(user_list, login)) {
        dprintf(client->socket, "/ret LOGIN:101\n");
        return 101;
    }

    strncpy(client->pseudo, login, sizeof(client->pseudo) - 1);
    client->pseudo[sizeof(client->pseudo) - 1] = '\0';

    dprintf(client->socket, "/ret LOGIN:000\n");

    int total = 0;
    struct user *tmp = user_list;
    while (tmp) {
        total++;
        tmp = tmp->next;
    }

    int numero = client->numero;
    char role = (client->symbole == 'X') ? 'r' : 'o';

    dprintf(client->socket, "/info LOGIN:%d/%d:%s:%c\n", numero, total, client->pseudo, role);
    return 0;
}

int handle_ready(struct user *client, struct user *user_list) {
    if (client->etat != ETAT_ATTENTE) {
        dprintf(client->socket, "/ret READY:102\n");
        return 102;
    }

    client->estPret = 1;
    dprintf(client->socket, "/ret READY:000\n");

    struct user *j1 = NULL, *j2 = NULL;
    struct user *tmp = user_list;
    while (tmp) {
        if (tmp->estPret && strcmp(tmp->pseudo, "inconnu") != 0) {
            if (!j1) j1 = tmp;
            else if (!j2) {
                j2 = tmp;
                break;
            }
        }
        tmp = tmp->next;
    }

    if (j1 && j2) {
        j1->etat = ETAT_JEU;
        j2->etat = ETAT_JEU;

        dprintf(j1->socket, "/info START:%s:%s\n", j1->pseudo, j2->pseudo);
        dprintf(j2->socket, "/info START:%s:%s\n", j1->pseudo, j2->pseudo);
    }

    return 0;
}
