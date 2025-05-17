CC = gcc
CFLAGS = -Wall -g
INCLUDES = -Iinclude
SERVER_SRC = src/main.c src/server.c
SERVER_BIN = server_puissance4
CLIENT_SRC = src/client.c
CLIENT_BIN = client

all: $(SERVER_BIN)

$(SERVER_BIN):
	$(CC) $(CFLAGS) $(SERVER_SRC) $(INCLUDES) -o $(SERVER_BIN)

client:
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT_BIN)

clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN)
