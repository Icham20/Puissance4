CC = gcc
CFLAGS = -Wall -g
INCLUDES = -Iinclude
SERVER_SRC = src/main.c src/server.c src/protocole.c src/grille.c
SERVER_BIN = server_puissance4
CLIENT_SRC = src/client.c
CLIENT_BIN = client

all: $(SERVER_BIN)

$(SERVER_BIN): $(SERVER_SRC)
	$(CC) $(CFLAGS) $(SERVER_SRC) $(INCLUDES) -o $(SERVER_BIN)

client:
	$(CC) $(CFLAGS) $(CLIENT_SRC) $(INCLUDES) -o $(CLIENT_BIN)

clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN)