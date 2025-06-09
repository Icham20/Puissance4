#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/select.h>

#define LG_MESSAGE 512

int hauteur = 6;
int largeur = 7;

// ✅ Vérifie si la chaîne est uniquement "/login"
int est_login_seul(const char *msg) {
    char copie[LG_MESSAGE];
    strncpy(copie, msg, LG_MESSAGE);
    copie[LG_MESSAGE - 1] = '\0';

    for (int i = strlen(copie) - 1; i >= 0; i--) {
        if (copie[i] == ' ' || copie[i] == '\n' || copie[i] == '\r' || copie[i] == '\t')
            copie[i] = '\0';
        else
            break;
    }

    return strcmp(copie, "/login") == 0;
}

// ✅ Nouvelle version propre de l'affichage de la grille
void afficher_grille_matrix(const char *matrix) {
    char lignes[10][11];  // max 10 lignes, 10 colonnes + \0
    int nb_lignes = 0;
    int nb_colonnes = 0;

    const char *start = matrix;
    while (*start && nb_lignes < 10) {
        const char *slash = strchr(start, '/');
        int len = (slash ? slash - start : strlen(start));
        if (len > 10) len = 10;

        strncpy(lignes[nb_lignes], start, len);
        lignes[nb_lignes][len] = '\0';

        if (nb_lignes == 0)
            nb_colonnes = len;

        nb_lignes++;

        if (!slash)
            break;
        start = slash + 1;
    }

    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf(" Grille de jeu (%dx%d)\n", nb_colonnes, nb_lignes);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    for (int i = nb_lignes - 1; i >= 0; i--) {
        printf(" +");
        for (int j = 0; j < nb_colonnes; j++)
            printf("---+");
        printf("\n |");
        for (int j = 0; j < nb_colonnes; j++) {
            char c = lignes[i][j];
            if (c != 'x' && c != 'o' && c != 'X' && c != 'O')
                c = ' ';
            printf(" %c |", c);
        }
        printf("\n");
    }

    printf(" +");
    for (int j = 0; j < nb_colonnes; j++)
        printf("---+");
    printf("\n  ");
    for (int j = 0; j < nb_colonnes; j++)
        printf(" %d  ", j);
    printf("\n");
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_address;
    char buffer[LG_MESSAGE];
    char temp[LG_MESSAGE];
    int is_logged_in = 0;

    if (argc < 3) {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_address.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("connect");
        return 1;
    }

    printf("Puissance 4 - Client connecté au serveur\n");
    printf("Connecté au serveur [%s:%s]\n", argv[1], argv[2]);

    fd_set readfds;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sock, &readfds);

        if (select(sock + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            memset(buffer, 0, sizeof(buffer));
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = '\0';

            if (!is_logged_in) {
                snprintf(temp, sizeof(temp), "/login %.500s\n", buffer);
                strncpy(buffer, temp, sizeof(buffer));
            } else {
                int col = atoi(buffer);
                if (col >= 0 && col < largeur) {
                    snprintf(temp, sizeof(temp), "/play %d\n", col);
                    strncpy(buffer, temp, sizeof(buffer));
                } else {
                    printf("⚠️ Colonne invalide. Choisissez entre 0 et %d.\n", largeur - 1);
                    printf("> ");
                    fflush(stdout);
                    continue;
                }
            }

            write(sock, buffer, strlen(buffer));
        }

        if (FD_ISSET(sock, &readfds)) {
            memset(buffer, 0, sizeof(buffer));
            int bytes_received = read(sock, buffer, sizeof(buffer) - 1);
            if (bytes_received <= 0) {
                printf("🔌 Déconnecté du serveur.\n");
                break;
            }

            buffer[bytes_received] = '\0';

            if (est_login_seul(buffer) || strlen(buffer) <= 2)
                continue;

            if (strncmp(buffer, "/ret LOGIN:000", 14) == 0) {
                printf("✅ Connexion acceptée.\n");
                is_logged_in = 1;
            } else if (strncmp(buffer, "/ret LOGIN:101", 14) == 0) {
                printf("❌ Pseudo déjà utilisé. Choisissez-en un autre.\n> ");
            } else if (strncmp(buffer, "/ret LOGIN:105", 14) == 0) {
                printf("❌ Pseudo invalide (3-16 caractères, sans ':').\n> ");
            } else if (strncmp(buffer, "/info ID:", 9) == 0) {
                printf("Identifiant serveur : %s\n", strchr(buffer, ':') + 1);
                printf("Entrez votre pseudo : ");
                fflush(stdout);
            } else if (strncmp(buffer, "/info MATRIX:", 13) == 0) {
                afficher_grille_matrix(strchr(buffer, ':') + 1);
            } else if (strncmp(buffer, "/play", 5) == 0) {
                printf("🎯 C’est votre tour ! Entrez une colonne (0 à %d) : ", largeur - 1);
                fflush(stdout);
            } else if (strncmp(buffer, "/info END:WIN:", 14) == 0) {
                char *login_gagnant = strstr(buffer, "WIN:") + 4;
                printf("🏆 %s a gagné la partie ! Bravo 🎉\n", login_gagnant);
                break;
            } else if (strncmp(buffer, "/info END:DRAW:NONE", 19) == 0) {
                printf("\n🤝 Match nul ! Personne n’a gagné cette fois.\n");
                break;
            }
        }
    }

    close(sock);
    return 0;
}
