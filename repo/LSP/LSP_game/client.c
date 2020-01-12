#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
// #include <ncurses.h>  // for gui. will be used later
// #include <csignal>  // for signals

//TODO: Remove C
int port;
struct hostent * host;
int sock;
int map_height = 31;
int map_width = 122;
char ** game_map;
int map_line;
int start_countdown;
int GAME_IN_PROGRESS = 0;
int CONNECTED_TO_SERVER = 0;

#define MSG_JOIN_GAME "0<%s>"
#define SIZE_MSG_JOIN_GAME 20
#define SIZE_SEGVARDS 16
#define C_LOBBY_INFO 2
#define C_GAME_IN_PROGRESS 3
#define C_USERNAME_TAKEN 4
#define C_GAME_START 5
#define C_MAP_ROW 6
#define C_GAME_UPDATE 7
#define C_PLAYER_DEAD 8
#define C_GAME_END 9

void process_arg(int argc, char* argv[]);
void connect_to_server();
void try_to_join_game();
void run();
void wait_signal();
void read_map(int map_height, int map_width);
void draw_map();
void update_map(char * rsp_buffer, int rsp_size);
void lobby_info();
void game_end();
void force_quit();
void * playing_thread();

int main(int argc, char* argv[])
{ 
	initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

	process_arg(argc, argv);

	connect_to_server();

	run();

	endwin();

	//when close connection

    return 0;
}

void run()
{
	//1. Set segvards  
	try_to_join_game();

	wait_signal();

	while(1) {

	}

	return;
}

void wait_signal()
{
	char msg_buffer[SIZE_MSG_JOIN_GAME];
	char * rsp_buffer;
	int len;
	pthread_t thread;

	while(CONNECTED_TO_SERVER == 1)
	{
		read (sock, &len, sizeof(int));
		if (len > 0)
		{
			rsp_buffer = (char *)malloc((len+1)*sizeof(char));
			rsp_buffer[len] = 0;

			if (read(sock, rsp_buffer, len) == -1)
			{
				fprintf(stderr, ": error receiving data \n");
				perror("Error description: ");
			} else {

				if (rsp_buffer[0] == C_LOBBY_INFO)
				{
					printf("LOBBY INFO: %s\n", rsp_buffer);
					lobby_info(rsp_buffer);
				} 
				else if (rsp_buffer[0] == C_GAME_IN_PROGRESS)
				{
					printf("GAME IN PROGRESS: %s\n", rsp_buffer);
				}
				else if (rsp_buffer[0] == C_GAME_START)
				{
					printf("GAME START %s\n", rsp_buffer);
					int counter = 2;
					while(rsp_buffer[counter] !=  '>')
					{
						map_height *= 10;
						map_height += rsp_buffer[counter] - '0';
						counter++;
					}
					counter += 2;
					while(rsp_buffer[counter] !=  '>')
					{
						map_width *= 10;
						map_width += rsp_buffer[counter] - '0';
						counter++;
					}
					read_map(map_height, map_width);
					GAME_IN_PROGRESS = 1;
					pthread_create(&thread, 0, playing_thread, NULL);
					pthread_detach(countdown_thread, NULL);
				}
				else if (rsp_buffer[0] == C_GAME_UPDATE)
				{
					printf("GAME UPDATE %s\n", rsp_buffer);
					update_map(rsp_buffer, sizeof(rsp_buffer));
				}
				else if (rsp_buffer[0] == C_PLAYER_DEAD)
				{
					printf("PLAYER DEAD %s\n", rsp_buffer);
				}
				else if (rsp_buffer[0] == C_GAME_END)
				{
					printf("GAME END %s\n", rsp_buffer);
					game_end();
					//force_quit(); //mby should be used in game_end()
				}
			}
			free(rsp_buffer);
		}
	}
}

void lobby_info() {

}

void read_map(int map_height, int map_width)
{
	map_line = 0;
	// game_map[map_height][map_width];
	char * rsp_buffer;
	int len;
	
	while (map_line < map_height) {

		read (sock, &len, sizeof(int));
		if (len > 0)
		{
			rsp_buffer = (char *)malloc((len+1)*sizeof(char));
		    rsp_buffer[len] = 0;

			if (read(sock, rsp_buffer, len) == -1)
			{
				fprintf(stderr, ": error receiving data \n");
			    perror("Error description: ");
			} else {

				if (rsp_buffer[0] == C_MAP_ROW)
				{
					printf("MAP ROW: %s\n", rsp_buffer);

					int counter = 2;
					map_line = 0;
					while(rsp_buffer[counter] !=  '>')
					{
						map_line *= 10;
						map_line += rsp_buffer[counter] - '0';
						counter++;
					}
					counter += 2;
					while (rsp_buffer[counter] != '>') {
						for (int j = 0; j < map_width; j++) {
							game_map[map_line][j] = rsp_buffer[counter];
						}
						counter++;
					}
				} else {
					printf("ERROR: inconsequent game map data\n");
					force_quit();
				}
			}
		}
	}
	draw_map();
}

void draw_map()
{
	for (int i = 0; i < map_height; i++)
	{
		for (int j = 0; j < map_width; j++)
		{
			printf("%c", game_map[i][j]);
		}
		printf("\n");
	}
}


void * playing_thread ()
{
	int len;
	char msg_buffer[4];
    while (game_in_process)
	{
		// read from console	     
    	switch ( ch ) {
	     
	    	case KEY_UP:
	    	    printw("Key pressed: UP. ");
	    	    printw("1<U>\n");
	    	    
	    	    msg_buffer = "1<U>";

	    	    len = strlen(msg_buffer);

	    	    send(sock, &len, sizeof(int), 0);
	    	    send(sock, msg_buffer, sizeof(msg_buffer), 0);
	    	    break;
	     
	    	case KEY_DOWN:
	    	    printw("Key pressed: DOWN. ");
	    	    printw("1<D>\n");

	    	    msg_buffer = "1<D>";

	    	    len = strlen(msg_buffer);

	    	    send(sock, &len, sizeof(int), 0);
	    	    send(sock, msg_buffer, sizeof(msg_buffer), 0);
	    	    break;
	     
	    	case KEY_LEFT:
	    	    printw("Key pressed: LEFT. ");
	    	    printw("1<L>\n");

	    	    msg_buffer = "1<L>";

	    	    len = strlen(msg_buffer);

	    	    send(sock, &len, sizeof(int), 0);
	    	    send(sock, msg_buffer, sizeof(msg_buffer), 0);
	    	    break;
	     
	    	case KEY_RIGHT:
	    	    printw("Key pressed: RIGHT. ");
	    	    printw("1<R>\n");

	    	    msg_buffer = "1<R>";

	    	    len = strlen(msg_buffer);

	    	    send(sock, &len, sizeof(int), 0);
	    	    send(sock, msg_buffer, sizeof(msg_buffer), 0);
	    	    break;
    	}
	}
}


void update_map(char * rsp_buffer, int rsp_size)
{
	char * rsp_buffer[rsp_size] = rsp_buffer;

}

void force_quit()
{
	// raise(SIGABRT);
}

void game_end()
{

}


void try_to_join_game()
{

	char user_name[SIZE_SEGVARDS];
	char msg_buffer[SIZE_MSG_JOIN_GAME];

	char * rsp_buffer;
	int len;

	printf("Provide segvards. Note: only first 16 symbols will be read.\n");
	scanf("%16s", user_name);

	snprintf(msg_buffer, SIZE_MSG_JOIN_GAME, MSG_JOIN_GAME, user_name);

	printf("Segavrds: %s\n", msg_buffer);

	len = strlen(msg_buffer);

	send(sock, &len, sizeof(int), 0);
	send(sock, msg_buffer, sizeof(msg_buffer), 0);

	//LOBBY_INFO - veiksmīga pievienošanās.
	//GAME_IN_PROGRESS - spēle jau notiek.
	//USERNAME_TAKEN - spēlētāja lietotais segvārds jau ir aizņemts.
	read (sock, &len, sizeof(int));
	if (len > 0)
	{
		rsp_buffer = (char *)malloc((len+1)*sizeof(char));
	    rsp_buffer[len] = 0;

		if (read(sock, rsp_buffer, len) == -1)
		{
			fprintf(stderr, ": error receiving data \n");
		    perror("Error description: ");
		} else {

			if (rsp_buffer[0] == C_LOBBY_INFO)
			{
				printf("LOBBY INFO: %s\n", rsp_buffer);
			} 
			else if (rsp_buffer[0] == C_USERNAME_TAKEN)
			{
				printf("USERNAME TAKEN: %s\n", rsp_buffer);
			}
		}

		free(rsp_buffer);
	}

	return;
}

void process_arg(int argc, char* argv[])
{
	if (argc == 3)
	{
		host = gethostbyname(argv[1]); // Can be mistake?
		if (!host)
    	{
	        fprintf(stderr, "%s: error: unknown host \n", argv[1]);
	        perror("Error description: ");
        	exit(1);
    	}

		sscanf(argv[2], "%d", &port);
		printf("Arguments processed.\n");

	}
	else 
	{
		printf("Not enough arguments.\n");
		exit(1);
	}

	return;
}

void connect_to_server()
{
	struct sockaddr_in address;
	int result;

	printf("Creating socket...\n");

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sock <= 0)
    {
        fprintf(stderr, "error: cannot create socket\n");
        perror("Error description: ");
        exit(0);
    }

    printf("Socket created. Socket descriptor: %d\n", sock);


    printf("Connecting to the server on port %d ...", port);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    memcpy(&address.sin_addr, host->h_addr_list[0], host->h_length); // Copying to sin_addr

    result = connect(sock, (struct sockaddr *)&address, sizeof(address)); // Connecting to server by socket descriptor and address 

   	if (result == -1)
   	{
   		fprintf(stderr, "error: cannot connect to host\n");
        perror("Error description: ");
        exit(0);
   	}

   	printf("Connected to server.\n");
   	CONNECTED_TO_SERVER = 1;
   	return;
}