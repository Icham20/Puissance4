# 💡 Projet Serveur Puissance 4 — Jalon 1

Ce projet implémente un serveur TCP multiclient en C, basé sur une architecture modulaire.  
Il utilise `poll()` pour gérer les connexions simultanées et une liste chaînée pour suivre les clients connectés.

## ⚙️ Compilation

```bash
make          # Compile le serveur (génère l'exécutable ./serveur_puissance4)
make client   # Compile le client de test (génère l'exécutable ./client)
make clean    # Supprime les exécutables générés
