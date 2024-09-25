all: server client
server: tetris.c
	gcc -o server tetris.c -lncurses
client: tetris_client.c
	gcc -o client tetris_client.c -lncurses
clean:
	rm -f server client
