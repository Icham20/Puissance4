// ==========================================
// PROJET PUISSANCE 4 - CLIENT TCP
// Auteur : Houssam
// Description : client qui permet de jouer au Puissance 4 en se connectant à un serveur.
// L'utilisateur tape seulement son pseudo ou une colonne. Le client envoie les commandes au serveur.
// ==========================================

#include <stdio.h>          // Entrées / sorties
#include <stdlib.h>         // Fonctions utilitaires
#include <string.h>         // Manipulations de chaînes
#include <unistd.h>         // Fonctions UNIX
#include <arpa/inet.h>      // Gestion des adresses IP
#include <sys/select.h>     // Pour utiliser select()

#define LG_MESSAGE 512      // Taille maximale des messages

int hauteur = 6;            // Hauteur par défaut de la grille
int largeur = 7;            // Largeur par défaut de la grille

// Fonction qui affiche la grille envoyée par le serveur
void afficher_grille_matrix(const char *matrix) 
{
    char lignes[10][11]; 
    int nb_lignes = 0;
    int nb_colonnes = 0;

    const char *start = matrix;
    while (*start && nb_lignes < 10) // On parcourt la chaîne reçue (matrix), ligne par ligne
    {
        const char *slash = strchr(start, '/'); // On cherche le prochain '/' qui sépare les lignes
        int len = (slash ? slash - start : strlen(start));  // On calcule la longueur de la ligne (jusqu'à '/' ou fin de la chaîne)
        if (len > 10) len = 10;
        if (len == 0) break; // Si la ligne est vide → on arrête

        strncpy(lignes[nb_lignes], start, len); // On copie la ligne dans le tableau lignes[nb_lignes]
        lignes[nb_lignes][len] = '\0';   // On ajoute le caractère de fin '\0' pour en faire une vraie chaîne C

        if (nb_lignes == 0) nb_colonnes = len;  // Si c'est la première ligne → on mémorise le nombre de colonnes

        nb_lignes++; // On passe à la ligne suivante
        if (!slash) break; // Si on a atteint la fin de la chaîne → on arrête
        start = slash + 1; // On place le pointeur start juste après le '/'
        if (*start == '\0') break;  // Si c'est la fin de la chaîne → on arrête
    }

    // Affiche la grille formatée
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf(" Grille de jeu (%dx%d)\n", nb_colonnes, nb_lignes);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    // Affiche les lignes de bas en haut
    for (int i = nb_lignes - 1; i >= 0; i--)
    {
        printf(" +");
        for (int j = 0; j < nb_colonnes; j++)
            printf("---+");
        printf("\n |");
        for (int j = 0; j < nb_colonnes; j++)
        {
            char c = lignes[i][j];
            if (c != 'x' && c != 'o' && c != 'X' && c != 'O')
                c = ' ';
            printf(" %c |", c);
        }
        printf("\n");
    }

    // Affiche les numéros de colonnes
    printf(" +");
    for (int j = 0; j < nb_colonnes; j++)
        printf("---+");
    printf("\n  ");
    for (int j = 0; j < nb_colonnes; j++)
        printf(" %d  ", j);
    printf("\n");
}

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in server_address;
    char buffer[LG_MESSAGE];
    char temp[LG_MESSAGE];
    int is_logged_in = 0; // 0 = pas encore connecté (pas encore envoyé de /login)

    // Vérifie que l'utilisateur a bien donné l'IP et le port
    if (argc < 3)
    {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        return 1;
    }

    // Crée une socket TCP
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        return 1;
    }

    // Prépare l'adresse du serveur
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_address.sin_addr);

    // Se connecte au serveur
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("connect");
        return 1;
    }

    printf("Puissance 4 - Client connecté au serveur\n");
    printf("Connecté au serveur [%s:%s]\n", argv[1], argv[2]);

    fd_set readfds; // Liste des choses à surveiller : clavier et serveur

    // Boucle principale
    while (1)
    {
        FD_ZERO(&readfds);                    // Vide la liste
        FD_SET(STDIN_FILENO, &readfds);       // Surveille le clavier
        FD_SET(sock, &readfds);               // Surveille les messages du serveur

        // Attend une activité (clavier ou serveur)
        if (select(sock + 1, &readfds, NULL, NULL, NULL) < 0)
        {
            perror("select");
            break;
        }

        // Si l'utilisateur a tapé quelque chose
        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            memset(buffer, 0, sizeof(buffer));
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = '\0'; // Enlève le \n final

            // Si l'utilisateur tape une commande → interdit
            if (buffer[0] == '/')
            {
                continue;
            }

            // Si on n'est pas encore connecté → on envoie au serveur un /login avec le pseudo
            if (!is_logged_in)
            {
                snprintf(temp, sizeof(temp), "/login %.500s\n", buffer);
                strncpy(buffer, temp, sizeof(buffer));
            }
            // Si on est connecté → on envoie au serveur un /play avec la colonne
            else
            {
                int col = atoi(buffer); // Convertit en entier
                if (col >= 0 && col < largeur)
                {
                    snprintf(temp, sizeof(temp), "/play %d\n", col);
                    strncpy(buffer, temp, sizeof(buffer));
                }
                else
                {
                    // Colonne invalide → on demande une autre colonne
                    printf("colonne invalide. entrez une autre colonne : ");
                    fflush(stdout);
                    continue;
                }
            }

            // Envoie la commande au serveur
            write(sock, buffer, strlen(buffer));
        }

        // Si un message est reçu du serveur
        if (FD_ISSET(sock, &readfds))
        {
            memset(buffer, 0, sizeof(buffer));
            int bytes_received = read(sock, buffer, sizeof(buffer) - 1);
            if (bytes_received <= 0)
            {
                printf("==> Déconnecté du serveur.\n");
                break;
            }

            buffer[bytes_received] = '\0'; // On termine la chaîne avec '\0' pour pouvoir la traiter comme du texte
            char *line = strtok(buffer, "\n"); // Découpe en lignes

            while (line != NULL) // On parcourt toutes les lignes du message reçu du serveur
            {
                if (strlen(line) <= 2)  // Si la ligne est vide ou trop courte, on l'ignore et on passe à la suivante
                {
                    line = strtok(NULL, "\n"); // Passe à la ligne suivante
                    continue;
                }

                // Si le serveur nous dit que la connexion est OK
                if (strncmp(line, "/ret LOGIN:000", 14) == 0)
                {
                    printf("==> Connexion acceptée.\n");
                    is_logged_in = 1;
                }
                // Si le pseudo est déjà utilisé
                else if (strncmp(line, "/ret LOGIN:101", 14) == 0)
                {
                    printf("==> Pseudo déjà utilisé. Choisissez-en un autre.\n> ");
                }
                // Si le pseudo est invalide
                else if (strncmp(line, "/ret LOGIN:105", 14) == 0)
                {
                    printf("==> Pseudo invalide (3-16 caractères, sans ':').\n> ");
                }
                // Si le coup est valide
                else if (strncmp(line, "/ret PLAY:000", 13) == 0)
                {
                    // Rien à afficher
                }
                // Si ce n'est pas notre tour
                else if (strncmp(line, "/ret PLAY:102", 13) == 0)
                {
                    printf("Ce n'est pas votre tour. Attendez votre tour.\n");
                }
                // Si la colonne n'existe pas
                else if (strncmp(line, "/ret PLAY:103", 13) == 0)
                {
                    printf("colonne invalide. entrez une autre colonne : ");
                    fflush(stdout);
                }
                // Si la colonne est pleine
                else if (strncmp(line, "/ret PLAY:104", 13) == 0)
                {
                    printf("La colonne est pleine. Entrez une autre colonne : ");
                    fflush(stdout);
                }
                // Si le serveur nous donne notre ID
                else if (strncmp(line, "/info ID:", 9) == 0)
                {
                    printf("Identifiant serveur : %s\n", strchr(line, ':') + 1);
                    printf("Entrez votre pseudo : ");
                    fflush(stdout);
                }
                // Si le serveur nous envoie la grille à afficher
                else if (strncmp(line, "/info MATRIX:", 13) == 0)
                {
                    afficher_grille_matrix(strchr(line, ':') + 1);
                }
                // Si c'est notre tour de jouer
                else if (strncmp(line, "/play", 5) == 0)
                {
                    printf("==> C’est votre tour ! Entrez une colonne : ");
                    fflush(stdout);
                }
                // Si un joueur a gagné
                else if (strncmp(line, "/info END:WIN:", 14) == 0)
                {
                    char *login_gagnant = strstr(line, "WIN:") + 4;
                    printf("==> %s a gagné la partie ! Bravo !\n", login_gagnant);
                    break;
                }
                // Si égalité
                else if (strncmp(line, "/info END:DRAW:NONE", 19) == 0)
                {
                    printf("==> Match nul ! Personne n’a gagné cette fois.\n");
                    break;
                }

                line = strtok(NULL, "\n");
            }
        }
    }

    // Ferme la connexion
    close(sock);
    return 0;
}
