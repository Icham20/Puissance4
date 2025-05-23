// fichier client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define LG_MESSAGE 512

int afficher_reponses(int sock) {
    char response[LG_MESSAGE];
    int n = recv(sock, response, sizeof(response) - 1, MSG_DONTWAIT);
    if (n > 0) {
        response[n] = '\0';
        char *ligne = strtok(response, "\n");
        while (ligne) {
            printf("📩 Réponse : %s\n", ligne);
            if (strstr(ligne, "Connexion refusée")) {
                printf("⛔ Connexion refusée par le serveur. Fermeture du client.\n");
                return -1;  // Signale au main() de fermer
            }
            ligne = strtok(NULL, "\n");
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <IP> <PORT>\n", argv[0]);
        return 1;
    }

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

    printf("Connecté au serveur %s:%d\n", ip, port);

    // Messages initiaux (ex : /info ID, /login)
    usleep(100 * 1000);  // Petite pause pour laisser le serveur répondre
    if (afficher_reponses(sock) < 0) {
        close(sock);
        return 0;
    }

    char buffer[LG_MESSAGE];

    while (1) {
        printf("Vous : ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;

        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            perror("send");
            break;
        }

        usleep(100 * 1000);  // Laisse le temps au serveur d’envoyer toutes les lignes

        if (afficher_reponses(sock) < 0) break;
    }

    close(sock);
    return 0;
}
