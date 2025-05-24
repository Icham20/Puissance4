#include <stdio.h>

// Variables globales définies ailleurs (ex: dans protocole.c)
extern int hauteur;                  // Nombre de lignes de la grille
extern int largeur;                  // Nombre de colonnes de la grille
extern char grille[10][10];          // Matrice représentant la grille du jeu
extern char joueur_actuel;           // Symbole du joueur actif (X ou O), si utilisé

// =============================================
// FONCTION : afficher_grille
// Affiche graphiquement la grille de jeu côté serveur
// =============================================
void afficher_grille(void) {
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf(" Grille de jeu (%dx%d)\n", largeur, hauteur);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    // Affichage des lignes de la grille, du haut vers le bas
    for (int i = hauteur - 1; i >= 0; i--) {
        // Bord supérieur de la ligne
        printf(" +");
        for (int j = 0; j < largeur; j++) {
            printf("---+");
        }
        printf("\n |");

        // Contenu des cases
        for (int j = 0; j < largeur; j++) {
            char c = grille[i][j];
            if (c == '_') c = ' ';  // Affiche une case vide comme un espace
            printf(" %c |", c);
        }
        printf("\n");
    }

    // Bordure inférieure
    printf(" +");
    for (int j = 0; j < largeur; j++) {
        printf("---+");
    }
    printf("\n  ");

    // Numérotation des colonnes pour repères visuels
    for (int j = 0; j < largeur; j++) {
        printf(" %d  ", j);
    }
    printf("\n");
}
