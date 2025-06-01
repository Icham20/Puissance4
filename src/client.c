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

void afficher_grille_matrix(const char *matrix) {
    char grille[10][10];
    int ligne = 0, colonne = 0;
    int colonnes_ligne = 0;
    hauteur = 0;
    largeur = 0;

    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++)
            grille[i][j] = '_';

    for (int i = 0; matrix[i]; i++) {
        if (matrix[i] == '/') {
            if (ligne == 0) largeur = colonnes_ligne;
            ligne++;
            colonnes_ligne = 0;
            colonne = 0;
        } else {
            if (ligne < 10 && colonne < 10) {
                char c = matrix[i];
                if (c != '_' && c != 'x' && c != 'o' && c != 'X' && c != 'O') c = '_';
                grille[ligne][colonne++] = c;
                colonnes_ligne++;
            }
        }
    }

    if (colonnes_ligne > 0) {
        if (ligne == 0) largeur = colonnes_ligne;
        ligne++;
    }

    hauteur = ligne;

    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf(" Grille de jeu (%dx%d)\n", largeur, hauteur);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    for (int i = hauteur - 1; i >= 0; i--) {
        printf(" +");
        for (int j = 0; j < largeur; j++) printf("---+");
        printf("\n |");
        for (int j = 0; j < largeur; j++) {
            char c = grille[i][j];
            if (c == '_') c = ' ';
            printf(" %c |", c);
        }
        printf("\n");
    }

    printf(" +");
    for (int j = 0; j < largeur; j++) printf("---+");
    printf("\n  ");
    for (int j = 0; j < largeur; j++) printf(" %d  ", j);
    printf("\n");
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_address;
    char buffer[LG_MESSAGE];
    char temp[LG_MESSAGE];
    int debug_mode = 0;
    int is_logged_in = 0;

    if (argc < 3) {
        printf("Usage: %s <IP> <port> [--debug]\n", argv[0]);
        return 1;
    }

    if (argc > 3 && (strcmp(argv[3], "--debug") == 0 || strcmp(argv[3], "-d") == 0)) {
        debug_mode = 1;
        printf("🛠️ Mode debug activé\n");
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

            // Ne pas afficher /login si pas en mode debug
            if (!debug_mode && strncmp(buffer, "/login", 6) == 0) continue;

            if (debug_mode) {
                printf("[DEBUG] %s\n", buffer);
            }

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
            } else if (debug_mode) {
                printf("[DEBUG] Message inconnu reçu : %s\n", buffer);
            }
        }
    }

    close(sock);
    return 0;
}
