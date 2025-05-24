# 💡 Projet Puissance 4 

Ce projet implémente un jeu Puissance 4 en réseau à l’aide de sockets TCP en C.  
Il permet à deux joueurs de se connecter à un serveur et de jouer en temps réel dans un terminal.  
La grille s’affiche dynamiquement côté client et côté serveur.

---

## ⚙️ Compilation & 🚀 Exécution

### Lancer le serveur

```bash
make clean && make && ./server_puissance4 -p 6000 -L 2 -H 1
# Hauteur choisie ici : 1, Longueur choisie ici : 2, Port : 6000
```

### Lancer le client (joueurs)

```bash
make client && ./client 127.0.0.1 6000
# Port : 6000 — Adresse locale du serveur : 127.0.0.1
```

### Connexion SSH (si besoin)

```bash
ssh icham@192.168.200.13                      # Se connecter
scp -r nomdudossier icham@192.168.200.13:     # Copier le dossier
# Mot de passe : Puissance4

# Depuis Ubuntu (Windows Terminal) :
nc 192.168.200.13 7500
```

---

## 📁 Structure des fichiers

| Fichier          | Rôle                                                        |
|------------------|-------------------------------------------------------------|
| `main.c`         | Lance le serveur, gère les connexions et les commandes      |
| `server.c`       | Gère les sockets, la liste des utilisateurs et les connexions |
| `protocole.c`    | Gère les commandes `/login`, `/play` et la logique du jeu   |
| `grille.c`       | Affiche la grille dans le terminal du serveur               |
| `client.c`       | Côté client : envoie les commandes, reçoit les réponses     |
| `server.h`       | Déclarations des structures et constantes pour le serveur   |
| `protocole.h`    | Interface des fonctions du protocole de jeu                 |
| `grille.h`       | Prototype pour `afficher_grille()`                          |

---
