# 💡 Projet Serveur Puissance 4 

## ⚙️ Compilation

```bash
make          # Compile le serveur (génère l'exécutable ./serveur_puissance4)
make client   # Compile le client de test (génère l'exécutable ./client)
make clean    # Supprime les exécutables générés
```
## 🚀 Exécution

### Lancer le serveur

```bash
./serveur_puissance4 -p 7000 (ici le port choisi est 7000 mais on peut mettre ce que l'on veut)
```

### Lancer un client (dans un autre terminal)

```bash
./client 127.0.0.1 7000 (ici le port choisi est 7000 mais on peut mettre ce que l'on veut)
```
