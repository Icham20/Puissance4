
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>

#define LG_MESSAGE 512

void afficher_grille_matrix(const char *matrix) {
    printf("📩 Réponse : /info MATRIX:\n");

    int lignes = 1;
    for (const char *p = matrix; *p; p++) {
        if (*p == '/') lignes++;
    }

    char matrice[10][10];
    int row = 0, col = 0, max_col = 0;
    for (int i = 0; matrix[i]; i++) {
        if (matrix[i] == '/') {
            if (col > max_col) max_col = col;
            row++;
            col = 0;
        } else if (strncmp(lignes, "/login", 6) == 0) {
                printf("Veuillez entrer votre pseudo :\n");
            } else {
            matrice[row][col++] = matrix[i];
        }
    }
    if (col > max_col) max_col = col;

    int hauteur = lignes;
    int largeur = max_col;

    for (int r = 0; r < hauteur; r++) {
        for (int c = 0; c < largeur; c++) printf("+---");
        printf("+\n");
        for (int c = 0; c < largeur; c++) {
            char symbole = matrice[r][c];
            if (symbole == 'x' || symbole == 'o') {
                printf("| \033[32m%c\033[0m ", symbole);
            } else if (strncmp(lignes, "/login", 6) == 0) {
                printf("Veuillez entrer votre pseudo :\n");
            } else {
                printf("| %c ", symbole);
            }
        }
        printf("|\n");
    }

    for (int c = 0; c < largeur; c++) printf("+---");
    printf("+\n");
    for (int c = 0; c < largeur; c++) printf("  %d ", c);
    printf("\n");
}

void nettoyer_prompt() {
    printf("\r%*s\r", 80, "");
}

int afficher_reponses(int sock, int debug) {
    char response[LG_MESSAGE];
    int n = recv(sock, response, sizeof(response) - 1, MSG_DONTWAIT);
    if (n > 0) {
        response[n] = '\0';
        char *ligne = strtok(response, "\n");

        while (ligne) {
            nettoyer_prompt();

            if (debug) printf("[DEBUG] %s\n", ligne);
            if (strncmp(ligne, "/info MATRIX:", 13) == 0) {
                afficher_grille_matrix(ligne + 13);
            } else if (strcmp(ligne, "/play") == 0) {
                printf("👉 C'est votre tour ! Choisissez une colonne :\n");
            } else if (strncmp(ligne, "/ret LOGIN:", 11) == 0) {
                printf("🔑 Code de connexion : %s\n", ligne + 5);
            } else if (strncmp(ligne, "/ret PLAY:", 10) == 0) {
                printf("🕹️ Code de coup : %s\n", ligne + 5);
            } else if (strncmp(ligne, "/info END:WIN:", 14) == 0) {
                printf("🎉 Le joueur %s a gagné !\n", ligne + 14);
            } else if (strncmp(ligne, "/info END:DRAW", 14) == 0) {
                printf("⚖️ Match nul !\n");
            } else if (strncmp(ligne, "/info ID:", 9) == 0) {
                printf("👤 %s\n", ligne + 9);
            } else if (strncmp(ligne, "/info LOGIN:", 12) == 0) {
                // ne rien afficher pour info login
                // ignorer
            } else if (strncmp(ligne, "/login", 6) == 0) {
                printf("Veuillez entrer votre pseudo :\n");
            } else {
                printf("📩 %s\n", ligne);
            }

            ligne = strtok(NULL, "\n");
        }

        printf("Vous : ");
        fflush(stdout);
    }
    return 0;
}

int debug = 0;
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <IP> <PORT>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-d") == 0) { debug = 1; argv++; argc--; }
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0) {
        perror("inet_pton");
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connect");
        return 1;
    }

    printf("Connecté à %s:%d\n", ip, port);

    usleep(100 * 1000);
    if (afficher_reponses(sock, debug) < 0) {
        close(sock);
        return 0;
    }

    char buffer[LG_MESSAGE];

    while (1) {
        struct pollfd fds[2];
        fds[0].fd = STDIN_FILENO;
        fds[0].events = POLLIN;
        fds[1].fd = sock;
        fds[1].events = POLLIN;

        int ret = poll(fds, 2, -1);
        if (ret > 0) {
            if (fds[1].revents & POLLIN) {
                if (afficher_reponses(sock, debug) < 0) break;
            }

            if (fds[0].revents & POLLIN) {
                if (strstr(buffer, "/login") != NULL) printf("Veuillez entrer votre pseudo : "); else printf("Vous : ");
                fflush(stdout);

                if (!fgets(buffer, sizeof(buffer), stdin)) break;

                if (send(sock, buffer, strlen(buffer), 0) < 0) {
                    perror("send");
                    break;
                }

                usleep(100 * 1000);
                if (afficher_reponses(sock, debug) < 0) break;
            }
        }
    }

    close(sock);
    return 0;
}
