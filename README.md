# Tetris in 200 C lines

## Files

Tetris.c - The server for the tetris game. All the logic will be ran here.

Tetris_client.c - The client for the tetris game. Controls and gameplay will
be ran here. 

## How to run

1. running the command "make" in the folder will create 2 executables "client"
and "server"

2. Run the server first. "./server"

3. Run the client second "./client"

4. The game should then start on the client side

## port and IP for client

The client has 2 "#define" towards the top currently set to 127.0.0.1 
"localhost" and port 8080. These can be changed although work well as it is.

## How to play

Use W,A,S,D to move the blocks. Q to terminate the game. 

