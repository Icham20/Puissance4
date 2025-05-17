# 💡 Projet Puissance 4 

## ⚙️ Compilation

```bash
make          # Compile le serveur (génère l'exécutable ./server_puissance4) + à faire dans le terminal serveur
make client   # Compile le client de test (génère l'exécutable ./client) + à faire dans le terminal client
make clean    # Supprime les exécutables générés
```
## 🚀 Exécution

### Lancer le serveur

```bash
./server_puissance4 -p 7000 (ici le port choisi est 7000)
```

### Lancer un client (dans un autre terminal)

```bash
./client 127.0.0.1 7000 (ici le port choisi est 7000), (127.0.0.1 = Adresse locale du serveur)
```
