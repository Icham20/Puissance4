# 💡 Projet Puissance 4 


## ⚙️ Compilation & 🚀 Exécution

### Lancer le serveur
```bash
make clean && make && ./server_puissance4 -p 6000 -L 2 -H 1
(Hauteur choisi ici : 1, Longueur choisi ici : 2, Port choisi ici : 6000)
```

### Lancer le client (joueurs)

```bash
make client && ./client 127.0.0.1 6000
(Port choisi ici : 7000), (127.0.0.1 = Adresse locale du serveur)
```

### SSH
```bash
ssh icham@192.168.200.13 (pour se connecter)
scp -r nomdudossier icham@192.168.200.13:   (pour copier le dossier)
password : Puissance4
Dans le terminal windows (ubuntu) : nc 192.168.200.13 7500 (ici le port choisi est 7500)
```
