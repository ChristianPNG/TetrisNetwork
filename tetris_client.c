#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#define SERVERPORT 8080
#define HOSTNAME "127.0.0.1"

typedef struct{
	int seq;
	int data;
} Packet;

int simulate_loss(float packet_loss_ratio){
	return ((double)rand() / (double) RAND_MAX) < packet_loss_ratio;
}


int main(int argc, char const *argv[]){
	initscr();
	timeout(1);

	//socket creation
	socklen_t addr_size;

	int sockid = socket(AF_INET, SOCK_DGRAM, 0);

	if (sockid < 0){
		perror("socket() failed");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr = {0};

	memset(&serveraddr, '\0', sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVERPORT);
	serveraddr.sin_addr.s_addr = inet_addr(HOSTNAME);

	char res[1024];
	char cpy[1024] = "";
	char similar[1024] = "";

	bool terminate = false;
	bool gameon = true;

	int sequence = 0;
	int ack;
	int c = -1; //default getch

	int cnt_packets = 0;
	int cnt_transmission = 0;
	int cnt_retransmission = 0;
	int cnt_dropped = 0;
	int cnt_success = 0;
	int cnt_ack = 0;
	int cnt_timeout = 0;

	struct timeval timeout;
	struct timeval timeout_end;
	fd_set readfds;
	timeout_end.tv_sec = 3;
	timeout.tv_sec = 0;
	timeout.tv_usec = 100000;

	while(gameon){
	/*
	In this loop: we getch input with timeout(1) if no input getch auto returns -1 or ERR.
	Send the input back to the server and receive the res (result) string from the server
	containing the game display. If res is not empty, clear and update the ncurses terminal
	with the new string.
	*/
		if (strcmp(similar, cpy)!=0){
			//sometimes while loop is restarted despite cpy not being updated
			//if checks if that occurred and if it did, skip this block
			printw("%s", cpy); //prints the main game display
			strcpy(similar, cpy);
		}
		Packet packet;
		packet.seq = sequence;
		c = getch();
		packet.data = c;
		if (c == 'q'){
		//terminate on q press
			terminate = true;
		}
		printw("packet %d generated for transmission\n", packet.seq);
		cnt_transmission++;

		addr_size = sizeof(serveraddr);
		bzero(res, 1024);

		bool wait = true;
		while(wait){
		/*
		This while loop simulates packet loss, there is a timeout operating on .1 seconds that detects if it
		receives the ACK or not. If not the loop is executed again.
		*/
			int loss = simulate_loss(0); //pass in packet loss ratio 0-1.
			if (!loss){
				int len = sendto(sockid, &packet, sizeof(packet), 0, (struct sockaddr*) 
						&serveraddr,addr_size);
				wait = false;
				if (len != sizeof(packet)){
					perror("failed to send");
					break;
				}
			}
			FD_ZERO(&readfds);
			FD_SET(sockid, &readfds);
			int activity = select(sockid + 1, &readfds, 0, 0, &timeout); //timeout function
			if (activity == 0){ 
				//0 = timeout expired
				printw("timeout expired for packet %d\n", packet.seq);
				printw("packet %d lost\n", packet.seq);
				printw("packet %d generated for re-transmission\n", packet.seq);
				cnt_retransmission++;
				cnt_dropped++;
				cnt_timeout++;
			}else{
				wait = false;
			}
		}
		cnt_packets++;
		printw("packet %d transmitted\n", cnt_packets);
		//receive the ACK
		int bytes_received = recvfrom(sockid, &ack, sizeof(ack), 0,
			(struct sockaddr*) &serveraddr, &addr_size);
		if (bytes_received < 0){
			printf("recv failure\n");
			break;
		}
		else{
			cnt_packets++;
			printw("packet %d transmitted\n", cnt_packets);
			printw("ack %d received\n", ack);
			printw("packet %d successfully transmitted with %d data bytes\n", packet.seq, sizeof(packet));
			cnt_success++;
			cnt_ack++;
			if (ack == 0){
				break;
			}
			wait = false;
		}
		sequence++;

		//receive the game logic + display in a string
		bytes_received = recvfrom(sockid, res, sizeof(res), 0,
			(struct sockaddr*) &serveraddr, &addr_size);


                if (bytes_received < 0) {
                    perror("recv error");
		    gameon = false;
		    break;
		} else {
			cnt_packets++;
			printw("packet %d transmitted\n", cnt_packets);
			if (res[0] != '\0'){ //check if res is not empty
				strcpy(cpy, res);
				clear();
			}
		}
	}
	clear();
	endwin();
	printf("%s", cpy);
	printf("game over\n");
	printf("total packets (generated + received): %d\n", cnt_packets);
	printf("total packets generated for transmission: %d\n", cnt_transmission);
	printf("total packets generated for retransmission: %d\n", cnt_retransmission);
	printf("total packets dropped: %d\n", cnt_dropped);
	printf("total packets successfully transmitted: %d\n", cnt_success);
	printf("total ACKs received: %d\n", cnt_ack);
	printf("total timeout expirations: %d\n", cnt_timeout);
	if (terminate){
		printf("Connection terminated by client!\n");
	}
	close(sockid);
	return 0;
}
