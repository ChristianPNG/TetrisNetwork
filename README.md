# TetrisNetwork

Tetris Network project is an implementation of a client-server architecture for
an existing tetris project. Note: I did not write the tetris game itself, that 
was najibghadri, however I did implement splitting the game into both a server and
client that are able to communicate with eachother. 

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


