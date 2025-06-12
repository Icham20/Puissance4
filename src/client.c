// Inclusion des bibliothèques standards nécessaires
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/select.h>

// Taille maximale des messages échangés
#define LG_MESSAGE 512

// Dimensions par défaut de la grille de jeu
int hauteur = 6;
int largeur = 7;

/*
  Fonction d'affichage de la grille de jeu à partir de la matrice envoyée par le serveur.
  La matrice est transmise sous la forme d'une chaîne de caractères formatée avec des '/' pour séparer les lignes.
*/
void afficher_grille_matrix(const char *matrix) 
{
    char lignes[10][11]; // Jusqu'à 10 lignes, 10 colonnes + '\0' pour chaque ligne
    int nb_lignes = 0;
    int nb_colonnes = 0;

    const char *start = matrix;
    while (*start && nb_lignes < 10)
    {
        const char *slash = strchr(start, '/'); // Cherche le prochain '/'
        int len = (slash ? slash - start : strlen(start)); // Longueur de la sous-chaîne avant le slash
        if (len > 10)
            len = 10;

        if (len == 0) // Ignore les lignes vides
            break;

        strncpy(lignes[nb_lignes], start, len);
        lignes[nb_lignes][len] = '\0';

        if (nb_lignes == 0)
            nb_colonnes = len; // Mémorise le nombre de colonnes à la première ligne

        nb_lignes++;

        if (!slash) // Si on est arrivé à la fin de la chaîne
            break;
        start = slash + 1;

        if (*start == '\0') // Si la fin est atteinte
            break;
    }

    // Affichage de la grille formatée
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf(" Grille de jeu (%dx%d)\n", nb_colonnes, nb_lignes);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    // Affiche la grille du bas vers le haut (comme dans un vrai Puissance 4)
    for (int i = nb_lignes - 1; i >= 0; i--)
    {
        printf(" +");
        for (int j = 0; j < nb_colonnes; j++)
            printf("---+");
        printf("\n |");
        for (int j = 0; j < nb_colonnes; j++)
        {
            char c = lignes[i][j];
            // Vérifie que le caractère est valide, sinon espace vide
            if (c != 'x' && c != 'o' && c != 'X' && c != 'O')
                c = ' ';
            printf(" %c |", c);
        }
        printf("\n");
    }

    // Ligne inférieure avec les numéros de colonnes
    printf(" +");
    for (int j = 0; j < nb_colonnes; j++)
        printf("---+");
    printf("\n  ");
    for (int j = 0; j < nb_colonnes; j++)
        printf(" %d  ", j);
    printf("\n");
}

/**
 * Fonction principale du client Puissance 4
 */
int main(int argc, char *argv[])
{
    int sock; // Descripteur de socket
    struct sockaddr_in server_address; // Adresse du serveur
    char buffer[LG_MESSAGE]; // Buffer de réception/envoi
    char temp[LG_MESSAGE]; // Buffer temporaire pour formater les commandes
    int is_logged_in = 0; // Indicateur de connexion réussie

    // Vérifie les arguments : l'utilisateur doit fournir l'IP et le port
    if (argc < 3)
    {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        return 1;
    }

    // Création de la socket TCP
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        return 1;
    }

    // Préparation de l'adresse du serveur
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_address.sin_addr);

    // Connexion au serveur
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("connect");
        return 1;
    }

    // Message de confirmation de connexion
    printf("Puissance 4 - Client connecté au serveur\n");
    printf("Connecté au serveur [%s:%s]\n", argv[1], argv[2]);

    // Préparation du set de descripteurs pour la fonction select()
    fd_set readfds;

    // Boucle principale du client
    while (1)
    {
        FD_ZERO(&readfds); // Réinitialisation du set
        FD_SET(STDIN_FILENO, &readfds); // Ajoute l'entrée standard (clavier)
        FD_SET(sock, &readfds); // Ajoute la socket réseau

        // Attente d'une activité sur l'un des descripteurs
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
            buffer[strcspn(buffer, "\n")] = '\0'; // Retire le \n final

            // Si l'utilisateur n'est pas encore connecté, on envoie la commande /login
            if (!is_logged_in)
            {
                snprintf(temp, sizeof(temp), "/login %.500s\n", buffer);
                strncpy(buffer, temp, sizeof(buffer));
            }
            // Sinon, on envoie une commande /play avec la colonne choisie
            else
            {
                int col = atoi(buffer);
                if (col >= 0 && col < largeur)
                {
                    snprintf(temp, sizeof(temp), "/play %d\n", col);
                    strncpy(buffer, temp, sizeof(buffer));
                }
                else
                {
                    // Gestion d'erreur : colonne invalide
                    printf("==> Colonne invalide. Choisissez entre 0 et %d.\n", largeur - 1);
                    printf("> ");
                    fflush(stdout);
                    continue;
                }
            }

            // Envoi de la commande au serveur
            write(sock, buffer, strlen(buffer));
        }

        // Si un message est reçu du serveur
        if (FD_ISSET(sock, &readfds))
        {
            memset(buffer, 0, sizeof(buffer));
            int bytes_received = read(sock, buffer, sizeof(buffer) - 1);
            if (bytes_received <= 0)
            {
                // Si la connexion est fermée
                printf("==> Déconnecté du serveur.\n");
                break;
            }

            buffer[bytes_received] = '\0';

            // Traitement des messages ligne par ligne
            char *line = strtok(buffer, "\n");
            while (line != NULL)
            {
                // Ignore les lignes très courtes
                if (strlen(line) <= 2)
                {
                    line = strtok(NULL, "\n");
                    continue;
                }

                // Traitement des différentes réponses du serveur
                if (strncmp(line, "/ret LOGIN:000", 14) == 0)
                {
                    printf("==> Connexion acceptée.\n");
                    is_logged_in = 1;
                }
                else if (strncmp(line, "/ret LOGIN:101", 14) == 0)
                {
                    printf("==> Pseudo déjà utilisé. Choisissez-en un autre.\n> ");
                }
                else if (strncmp(line, "/ret LOGIN:105", 14) == 0)
                {
                    printf("==> Pseudo invalide (3-16 caractères, sans ':').\n> ");
                }
                else if (strncmp(line, "/info ID:", 9) == 0)
                {
                    printf("Identifiant serveur : %s\n", strchr(line, ':') + 1);
                    printf("Entrez votre pseudo : ");
                    fflush(stdout);
                }
                else if (strncmp(line, "/login", 6) == 0)
                {
                    // On ignore l'affichage de /login
                }
                else if (strncmp(line, "/info MATRIX:", 13) == 0)
                {
                    // Affiche la grille reçue
                    afficher_grille_matrix(strchr(line, ':') + 1);
                }
                else if (strncmp(line, "/play", 5) == 0)
                {
                    // Invite à jouer
                    printf("==> C’est votre tour ! Entrez une colonne (0 à %d) : ", largeur - 1);
                    fflush(stdout);
                }
                else if (strncmp(line, "/info END:WIN:", 14) == 0)
                {
                    // Partie gagnée
                    char *login_gagnant = strstr(line, "WIN:") + 4;
                    printf("==> %s a gagné la partie ! Bravo !\n", login_gagnant);
                    break;
                }
                else if (strncmp(line, "/info END:DRAW:NONE", 19) == 0)
                {
                    // Match nul
                    printf("==> Match nul ! Personne n’a gagné cette fois.\n");
                    break;
                }

                // Passage à la ligne suivante
                line = strtok(NULL, "\n");
            }
        }
    }

    // Fermeture de la socket
    close(sock);
    return 0;
}
