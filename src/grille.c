#include <stdio.h>

extern int hauteur;
extern int largeur;
extern char grille[10][10];
extern char joueur_actuel; // X ou O si défini dans le serveur

void afficher_grille(void) {
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf(" Grille de jeu (%dx%d)\n", largeur, hauteur);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    for (int i = hauteur - 1; i >= 0; i--) {
        printf(" +");
        for (int j = 0; j < largeur; j++) {
            printf("---+");
        }
        printf("\n |");
        for (int j = 0; j < largeur; j++) {
            char c = grille[i][j];
            if (c == '_') c = ' ';
            printf(" %c |", c);
        }
        printf("\n");
    }

    // Ligne inférieure
    printf(" +");
    for (int j = 0; j < largeur; j++) {
        printf("---+");
    }
    printf("\n  ");
    for (int j = 0; j < largeur; j++) {
        printf(" %d  ", j);
    }
    printf("\n");
}
