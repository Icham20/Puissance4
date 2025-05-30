#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>

#define LG_MESSAGE 512  // Taille maximale d'un message échangé entre client et serveur

// =============================================
// AFFICHAGE DE LA GRILLE DE JEU
// =============================================
void afficher_grille_matrix(const char *matrix) {
    printf("📩 Réponse : /info MATRIX:\n");

    // Compte le nombre de lignes en fonction des séparateurs '/'
    int lignes = 1;
    for (const char *p = matrix; *p; p++) {
        if (*p == '/') lignes++;
    }

    // Décomposition de la chaîne en une matrice 2D
    char matrice[10][10];
    int row = 0, col = 0, max_col = 0;
    for (int i = 0; matrix[i]; i++) {
        if (matrix[i] == '/') {
            if (col > max_col) max_col = col;
            row++;
            col = 0;
        } else {
            matrice[row][col++] = matrix[i];
        }
    }
    if (col > max_col) max_col = col;

    int hauteur = lignes;
    int largeur = max_col;

    // Affichage ligne par ligne, du bas vers le haut (aligné avec la logique du jeu)
    for (int r = 0; r < hauteur; r++) {
        for (int c = 0; c < largeur; c++) printf("+---");
        printf("+\n");
        for (int c = 0; c < largeur; c++) printf("| %c ", matrice[r][c]);
        printf("|\n");
    }

    // Affiche la ligne du bas et les numéros de colonnes
    for (int c = 0; c < largeur; c++) printf("+---");
    printf("+\n");
    for (int c = 0; c < largeur; c++) printf("  %d ", c);
    printf("\n");
}

// Fonction utilitaire : efface la ligne de commande en cours
void nettoyer_prompt() {
    printf("\r%*s\r", 80, "");  // Efface la ligne avec des espaces
}

// =============================================
// AFFICHAGE DES RÉPONSES DU SERVEUR
// =============================================
int afficher_reponses(int sock) {
    char response[LG_MESSAGE];
    int n = recv(sock, response, sizeof(response) - 1, MSG_DONTWAIT);
    if (n > 0) {
        response[n] = '\0';  // Ajout du caractère de fin de chaîne
        char *ligne = strtok(response, "\n");  // Découpe par ligne

        while (ligne) {
            nettoyer_prompt();  // Nettoyage de l'affichage

            // Affichage formaté selon le type de message
            if (strncmp(ligne, "/info MATRIX:", 13) == 0) {
                afficher_grille_matrix(ligne + 13);  // Affiche la grille proprement
            } else if (strcmp(ligne, "/play") == 0) {
                printf("📩 Réponse : /play → C'est votre tour de jouer !\n");
            } else {
                printf("📩 Réponse : %s\n", ligne);  // Réponse standard
            }

            // Gestion des refus de connexion
            if (strstr(ligne, "Connexion refusée")) {
                printf("⛔ Connexion refusée par le serveur. Fermeture du client.\n");
                return -1;
            }

            ligne = strtok(NULL, "\n");
        }

        // Réaffiche le prompt de saisie utilisateur
        printf("Vous : ");
        fflush(stdout);
    }
    return 0;
}

// =============================================
// FONCTION PRINCIPALE DU CLIENT
// =============================================
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <IP> <PORT>\n", argv[0]);
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    // Création du socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    // Configuration de l'adresse du serveur
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0) {
        perror("inet_pton");
        return 1;
    }

    // Connexion au serveur distant
    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connect");
        return 1;
    }

    printf("Connecté au serveur %s:%d\n", ip, port);

    usleep(100 * 1000);  // Pause pour laisser le serveur répondre
    if (afficher_reponses(sock) < 0) {
        close(sock);
        return 0;
    }

    char buffer[LG_MESSAGE];

    // Boucle principale : attente d'activité serveur ou saisie utilisateur
    while (1) {
        struct pollfd fds[2];
        fds[0].fd = STDIN_FILENO;   // Entrée clavier
        fds[0].events = POLLIN;
        fds[1].fd = sock;           // Socket serveur
        fds[1].events = POLLIN;

        int ret = poll(fds, 2, -1);  // Attente indéfinie d'événement
        if (ret > 0) {
            // Traitement d'un message reçu du serveur
            if (fds[1].revents & POLLIN) {
                if (afficher_reponses(sock) < 0) break;
            }

            // Lecture et envoi d'une commande utilisateur
            if (fds[0].revents & POLLIN) {
                printf("Vous : ");
                fflush(stdout);

                if (!fgets(buffer, sizeof(buffer), stdin)) break;

                // Envoi du message au serveur
                if (send(sock, buffer, strlen(buffer), 0) < 0) {
                    perror("send");
                    break;
                }

                usleep(100 * 1000);  // Délai pour laisser la réponse arriver
                if (afficher_reponses(sock) < 0) break;
            }
        }
    }

    // Fermeture propre du socket
    close(sock);
    return 0;
}