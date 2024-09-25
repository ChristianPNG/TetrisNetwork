#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define ROWS 20 // you can change height and width of table with ROWS and COLS 
#define COLS 15
#define TRUE 1
#define FALSE 0

#define PORT 8080
#define HOSTNAME "127.0.0.1"

char Table[ROWS][COLS] = {0};
int score = 0;
char GameOn = TRUE;
suseconds_t timer = 400000; // decrease this to make it faster
int decrease = 1000;

typedef struct {
    char **array;
    int width, row, col;
} Shape;

typedef struct {
	int seq;
	int data;

} Packet;

Shape current;

const Shape ShapesArray[7]= {
	{(char *[]){(char []){0,1,1},(char []){1,1,0}, (char []){0,0,0}}, 3},                           //S shape     
	{(char *[]){(char []){1,1,0},(char []){0,1,1}, (char []){0,0,0}}, 3},                           //Z shape     
	{(char *[]){(char []){0,1,0},(char []){1,1,1}, (char []){0,0,0}}, 3},                           //T shape     
	{(char *[]){(char []){0,0,1},(char []){1,1,1}, (char []){0,0,0}}, 3},                           //L shape     
	{(char *[]){(char []){1,0,0},(char []){1,1,1}, (char []){0,0,0}}, 3},                           //flipped L shape    
	{(char *[]){(char []){1,1},(char []){1,1}}, 2},                                                 //square shape
	{(char *[]){(char []){0,0,0,0}, (char []){1,1,1,1}, (char []){0,0,0,0}, (char []){0,0,0,0}}, 4} //long bar shape
	// you can add any shape like it's done above. Don't be naughty.
};

Shape CopyShape(Shape shape){
	Shape new_shape = shape;
	char **copyshape = shape.array;
	new_shape.array = (char**)malloc(new_shape.width*sizeof(char*));
    int i, j;
    for(i = 0; i < new_shape.width; i++){
		new_shape.array[i] = (char*)malloc(new_shape.width*sizeof(char));
		for(j=0; j < new_shape.width; j++) {
			new_shape.array[i][j] = copyshape[i][j];
		}
    }
    return new_shape;
}

void DeleteShape(Shape shape){
    int i;
    for(i = 0; i < shape.width; i++){
		free(shape.array[i]);
    }
    free(shape.array);
}

int CheckPosition(Shape shape){ //Check the position of the copied shape
	char **array = shape.array;
	int i, j;
	for(i = 0; i < shape.width;i++) {
		for(j = 0; j < shape.width ;j++){
			if((shape.col+j < 0 || shape.col+j >= COLS || shape.row+i >= ROWS)){ //Out of borders
				if(array[i][j]) //but is it just a phantom?
					return FALSE;
				
			}
			else if(Table[shape.row+i][shape.col+j] && array[i][j])
				return FALSE;
		}
	}
	return TRUE;
}

void SetNewRandomShape(){ //updates [current] with new shape
	Shape new_shape = CopyShape(ShapesArray[rand()%7]);

    new_shape.col = rand()%(COLS-new_shape.width+1);
    new_shape.row = 0;
    DeleteShape(current);
	current = new_shape;
	if(!CheckPosition(current)){
		GameOn = FALSE;
	}
}

void RotateShape(Shape shape){ //rotates clockwise
	Shape temp = CopyShape(shape);
	int i, j, k, width;
	width = shape.width;
	for(i = 0; i < width ; i++){
		for(j = 0, k = width-1; j < width ; j++, k--){
				shape.array[i][j] = temp.array[k][i];
		}
	}
	DeleteShape(temp);
}

void WriteToTable(){
	int i, j;
	for(i = 0; i < current.width ;i++){
		for(j = 0; j < current.width ; j++){
			if(current.array[i][j])
				Table[current.row+i][current.col+j] = current.array[i][j];
		}
	}
}

void RemoveFullRowsAndUpdateScore(){
	int i, j, sum, count=0;
	for(i=0;i<ROWS;i++){
		sum = 0;
		for(j=0;j< COLS;j++) {
			sum+=Table[i][j];
		}
		if(sum==COLS){
			count++;
			int l, k;
			for(k = i;k >=1;k--)
				for(l=0;l<COLS;l++)
					Table[k][l]=Table[k-1][l];
			for(l=0;l<COLS;l++)
				Table[k][l]=0;
			timer-=decrease--;
		}
	}
	score += 100*count;
}

char* PrintTable(){
	/*
	modified PrintTable() function, instead of printw any calls that use to do that now just append to a string called res (result).
	*/
	char Buffer[ROWS][COLS] = {0};
	char *res = malloc(1024*sizeof(char*));
	int i, j;
	for(i = 0; i < current.width ;i++){
		for(j = 0; j < current.width ; j++){
			if(current.array[i][j])
				Buffer[current.row+i][current.col+j] = current.array[i][j];
		}
	}
	clear();
	for(i=0; i<COLS-9; i++)
		//printw(" ");
		strcat(res, " ");
	//printw("Covid Tetris\n");
	strcat(res, "Covid Tetris\n");
	for(i = 0; i < ROWS ;i++){
		for(j = 0; j < COLS ; j++){
			//printw("%c ", (Table[i][j] + Buffer[i][j])? '#': '.');
			char tmp[2] = {(Table[i][j] + Buffer[i][j])? '#': '.', '\0'};
			strcat(res, tmp);
		}
		//printw("\n");
		strcat(res, "\n");
	}
	//printw("\nScore: %d\n", score);
	//strcat(res, "\nScore: %d\n");
	return(res);
}

char* ManipulateCurrent(int action){
	Shape temp = CopyShape(current);
	switch(action){
		case 's':
			temp.row++;  //move down
			if(CheckPosition(temp))
				current.row++;
			else {
				WriteToTable();
				RemoveFullRowsAndUpdateScore();
                SetNewRandomShape();
			}
			break;
		case 'd':
			temp.col++;  //move right
			if(CheckPosition(temp))
				current.col++;
			break;
		case 'a':
			temp.col--;  //move left
			if(CheckPosition(temp))
				current.col--;
			break;
		case 'w':
			RotateShape(temp); // rotate clockwise
			if(CheckPosition(temp))
				RotateShape(current);
			break;
	}
	DeleteShape(temp);
	return PrintTable();
}

struct timeval before_now, now;
int hasToUpdate(){
	return ((suseconds_t)(now.tv_sec*1000000 + now.tv_usec) -((suseconds_t)before_now.tv_sec*1000000 + before_now.tv_usec)) > timer;
}

int main() {
    //UDP setup
    int sockid;
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t clientAddrLen;
    int buffer;
    char test[1024];
    //socket creation
    sockid = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockid < 0){
        perror("socket creation error");
        return -1;
    }

    memset(&serveraddr, '\0', sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = inet_addr(HOSTNAME);

    //bind
    int bindid = bind(sockid, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if(bindid < 0){
        perror("server bind error\n");
        return -1;
    }
    //game logic
    srand(time(0));
    score = 0;
    int c = 99;
    int ack = 1;
	int packetcount = 0;
	SetNewRandomShape();
	char* res = PrintTable();
	while(GameOn){
	/*
	res (result) and buffer (control input) will be the main things being communicated between client and server.
	res is a string that contains all the characters needed to display the game and will
	be sent to the client. Buffer are int ascii values representing the input from the client.
	If no calls to manipulateCurrent() is done, we will send a res of "" which will be skipped on the client side.
	*/
		Packet packet;
		clientAddrLen = sizeof(clientaddr);
		res = "";
		int bytes_received = recvfrom(sockid, &packet, sizeof(packet),
			 0, (struct sockaddr*)&clientaddr, &clientAddrLen); // Receive integer input value
		printf("int recv: %d", packet.data);

		packetcount++;
		printf("packet %d transmitted\n", packetcount);
		if (bytes_received < 0) {
		    perror("recv error");
		} else {
			if (c == 'q'){
				break;
			}
			sendto(sockid, &ack, sizeof(ack), 0, 
				(struct sockaddr*)&clientaddr, clientAddrLen);
		    	c = packet.data; // Assign received value to a variable
			packetcount++;
			printf("packet %d transmitted\n", packetcount);
	                if(c != -1) { //if input was actually received.
			  	res = ManipulateCurrent(c);
			}
			gettimeofday(&now, NULL);
			if (hasToUpdate()) { //time difference in microsec accuracy
				res = ManipulateCurrent('s');
				gettimeofday(&before_now, NULL);
			}
			sendto(sockid, res, strlen(res), 0, 
				(struct sockaddr*)&clientaddr, clientAddrLen);
			packetcount++;
			printf("packet %d transmitted\n", packetcount);
		}

	}
	Packet packet;
        clientAddrLen = sizeof(clientaddr);

	if (c != 'q'){
		int bytes_received = recvfrom(sockid, &packet, sizeof(packet),
		0, (struct sockaddr*)&clientaddr, &clientAddrLen); // Receive integer input value
		printf("int recv: %d", packet.data);

		packetcount++;
		printf("packet %d transmitted\n", packetcount);
	}
	ack = 0;
	sendto(sockid, &ack, sizeof(ack), 0, 
	(struct sockaddr*)&clientaddr, clientAddrLen);
	c = packet.data; // Assign received value to a variable
	packetcount++;
	printf("packet %d transmitted\n", packetcount);



	close(sockid);
	DeleteShape(current);
	endwin();
	printf("\nGame ouvre!\n");
	printf("\nScore: %d\n", score);
	printf("Total number of data packets transmitted: %d\n", packetcount);
	return 0;
}
