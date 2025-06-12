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
scp -r /mnt/c/Puissance4/ icham@192.168.200.13:     # Copier le dossier
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

## ✨ Fonctionnalités

- Jeu **Puissance 4** en temps réel entre 2 joueurs via le réseau (TCP)
- Gestion de la grille côté serveur et envoi de l'état de la grille aux clients
- Gestion des tours de jeu, détection de victoire, détection de match nul
- Commandes `/login`, `/play`, `/info MATRIX`, `/info END`, etc.
- Affichage visuel de la grille dans le terminal (serveur et clients)
- Gestion des connexions et déconnexions des clients
- Architecture serveur multi-clients basée sur `poll()`

---

## 📜 Protocole de communication

Le serveur utilise un mini-protocole textuel basé sur des commandes simples :

| Commande envoyée | Description |
|------------------|-------------|
| `/login <pseudo>` | Connexion du joueur avec son pseudo |
| `/play <colonne>` | Joue dans la colonne spécifiée |
| `/info ID:` | ID du serveur envoyé au client |
| `/info LOGIN:` | Informations sur la connexion du joueur |
| `/info MATRIX:` | Grille de jeu envoyée à tous les clients |
| `/play` | Demande au joueur de jouer |
| `/info END:WIN:<pseudo>` | Fin de partie : joueur gagnant |
| `/info END:DRAW:NONE` | Fin de partie : match nul |
| `/ret LOGIN:...` | Codes de retour pour la commande `/login` |
| `/ret PLAY:...` | Codes de retour pour la commande `/play` |

---