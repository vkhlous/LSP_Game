// #include <stdio.h> // ncurses already has it
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ncurses.h>  // for gui
// #include <csignal>  // for signals

//TODO: Remove C
int port;
struct hostent * host;
int sock;
int ch;
int map_height = 0;
int map_width = 0;
// char ** game_map;
char game_map[31][121];
int map_line;
// int start_countdown;
int GAME_IN_PROGRESS = 0;
int CONNECTED_TO_SERVER = 0;
// char player_names[8][17];

typedef struct client
{
    char* segvards;
    char symbol;
    int active;
    int points;
    int x;
    int y;

} client_t;

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

#define A_CLIENT_CHAR 'A'
#define A_CLIENT_NR 1

#define B_CLIENT_CHAR 'B'
#define B_CLIENT_NR 2

#define C_CLIENT_CHAR 'C'
#define C_CLIENT_NR 3

#define D_CLIENT_CHAR 'D'
#define D_CLIENT_NR 4

#define PLAYERS_COUNT 4

struct client players[8];

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
void initialize_players();
void set_players_coordinats();

int main(int argc, char* argv[])
{ 

	process_arg(argc, argv);

	connect_to_server();

	initscr();
	cbreak();
	noecho();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	endwin();
	
	run();

	endwin();

	//when close connection

    return 0;
}

void run()
{
	//1. Set segvards
	initialize_players();

	try_to_join_game();

	wait_signal();

	while(1) {

	}

	return;
}

void initialize_players()
{
    int i = 0;
    for (; i < PLAYERS_COUNT; i++)
    {
        if (i+1 == A_CLIENT_NR)
        {
            players[i].symbol = A_CLIENT_CHAR;
            players[i].active = 0;
        }
        else if (i+1 == B_CLIENT_NR)
        {
            players[i].symbol = B_CLIENT_CHAR;
            players[i].active = 0;
        }
        else if (i+1 == C_CLIENT_NR)
        {
            players[i].symbol = C_CLIENT_CHAR;
            players[i].active = 0;
        }
        else if (i+1 == D_CLIENT_NR)
        {
            players[i].symbol = D_CLIENT_CHAR;
            players[i].active = 0;
        }
    }
}

void wait_signal()
{
	char msg_buffer[SIZE_MSG_JOIN_GAME];
	char * rsp_buffer;
	int len;
	pthread_t thread;

	while(CONNECTED_TO_SERVER == 1)
	{
		printf("I AM IN SERVER PART\n");
		read (sock, &len, sizeof(int));
		if (len > 0)
		{
			rsp_buffer = (char *)malloc((len+1)*sizeof(char));
			rsp_buffer[len] = 0;

			printf("rsp_buffer is %d\n", rsp_buffer[0] - '0');

			if (read(sock, rsp_buffer, len) == -1)
			{
				fprintf(stderr, ": error receiving data \n");
				perror("Error description: ");
			} else {

				if ((rsp_buffer[0] - '0') == C_LOBBY_INFO)
				{
					printf("LOBBY INFO: %s\n", rsp_buffer);
					lobby_info(rsp_buffer, strlen(rsp_buffer));
				} 
				else if ((rsp_buffer[0] - '0') == C_GAME_IN_PROGRESS)
				{
					printf("GAME IN PROGRESS: %s\n", rsp_buffer);
				}
				else if ((rsp_buffer[0] - '0') == C_GAME_START)
				{
					printf("GAME START %s\n", rsp_buffer);
					int counter = 2;
					while (rsp_buffer[counter] != '}')
					{
						counter++;
					}
					counter += 2;
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
					printf("height: %d, width: %d\n", map_height, map_width);
					read_map(map_height, map_width);
					GAME_IN_PROGRESS = 1;
					// pthread_create(&thread, 0, playing_thread, NULL);
					// pthread_detach(playing_thread);
				}
				else if ((rsp_buffer[0] - '0') == C_GAME_UPDATE)
				{
					printf("GAME UPDATE %s\n", rsp_buffer);
					update_map(rsp_buffer, strlen(rsp_buffer));
				}
				else if ((rsp_buffer[0] - '0') == C_PLAYER_DEAD)
				{
					printf("PLAYER DEAD %s\n", rsp_buffer);
				}
				else if ((rsp_buffer[0] - '0') == C_GAME_END)
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

void lobby_info(char * rsp_buffer, int size) {
	// 2<pievienojušos_spēlētāju_skaits>{<ntā_spēlētāja_segvārds>}
	printf("Started in lobby_info\n");
	char get_buffer[size];
	strcpy(get_buffer, rsp_buffer);
	char name[17];
	int player_count = 0;
	int counter = 2;
	printf("buffer in lobby_info: %s\n", get_buffer);
	while(get_buffer[counter] !=  '>')
	{
		player_count *= 10;
		player_count += get_buffer[counter] - '0';
		counter++;
	}
	counter += 3;
	for (int i = 0; i < player_count; i++)
	{
		int temp = 0;
		while(get_buffer[counter] != '>')
		{
			name[temp] = get_buffer[counter];
			temp++;
			counter++;
		}
		name[temp+1] = '\0';
		// strncpy(player_names[i], name, temp);
		players[i].active = 1;
	    players[i].segvards = (char *)malloc(16);
	    printf("player segvards: %s\n", players[i].segvards);
	    players[i].segvards = strdup(name);
		counter += 3;
	}
	if (player_count == 1)
	{
		printf("Connected %d player: %s", player_count, players[0].segvards);
	}
	else
	{
		printf("Connected %d players: ", player_count);
		for (int i = 0; i < player_count-1; i++)
		{
			printf(" %s,", players[i].segvards);
		}
		printf(" %s", players[player_count-1].segvards);
	}
	printf("\n");
}

void read_map(int map_height, int map_width)
{
	map_line = 0;
	int map_row = 0;
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
				printf("rsp_buffer size: %d\n", strlen(rsp_buffer));
				printf("rsp_buffer: %s\n", rsp_buffer);
				if ((rsp_buffer[0] - '0') == C_MAP_ROW)
				{
					printf("MAP ROW: %s\n", rsp_buffer);
					map_row = 0;
					int counter = 2;
					printf("MAP_ROW rsp_buffer: %c\n", rsp_buffer[counter]);
					while(rsp_buffer[counter] !=  '>')
					{
						map_row *= 10;
						map_row += rsp_buffer[counter] - '0';
						printf("map_row: %d\n", map_row);
						counter++;
					}
					counter += 2;
					int temp = 0;
					printf("MAP line: %c\n", rsp_buffer[counter]);
					while (rsp_buffer[counter] != '>') {
						printf("map_row: %d\t", map_row);
						printf("counter: %d\n", counter);
						printf("temp: %d\n", temp);
						printf("rsp_buffer row: %c\n", rsp_buffer[counter]);
						game_map[map_row][temp] = rsp_buffer[counter];
						printf("game_map row: %c\n", game_map[map_row][temp]);
						counter++;
						temp++;
					}
				} else {
					printf("ERROR: inconsequent game map data\n");
					force_quit();
				}
				printf("map_line: %d\n", map_line);
				map_line++;
			}
		}
	}
	draw_map();
}

void draw_map()
{
	// initscr();
	// cbreak();
	// noecho();
	// nodelay(stdscr, TRUE);
	// keypad(stdscr, TRUE);

    refresh();

	for (int i = 0; i < map_height; i++)
	{
		for (int j = 0; j < map_width; j++)
		{
			printf("%c", game_map[i][j]);
		}
		printf("\n");
		move(i,0);
	}
}


void * playing_thread ()
{
	int len;
	char msg_buffer[5];
    while (GAME_IN_PROGRESS)
	{
		// read from console	     
    	switch (ch = getch()) {
	     
	    	case KEY_UP:
	    	    printw("Key pressed: UP. ");
	    	    printw("1<U>\n");
	    	    
	    	    strcpy(msg_buffer, "1<U>");

	    	    len = strlen(msg_buffer);

	    	    send(sock, &len, sizeof(int), 0);
	    	    send(sock, msg_buffer, strlen(msg_buffer), 0);
	    	    break;
	     
	    	case KEY_DOWN:
	    	    printw("Key pressed: DOWN. ");
	    	    printw("1<D>\n");

	    	    strcpy(msg_buffer, "1<D>");

	    	    len = strlen(msg_buffer);

	    	    send(sock, &len, sizeof(int), 0);
	    	    send(sock, msg_buffer, strlen(msg_buffer), 0);
	    	    break;
	     
	    	case KEY_LEFT:
	    	    printw("Key pressed: LEFT. ");
	    	    printw("1<L>\n");

	    	    strcpy(msg_buffer, "1<L>");

	    	    len = strlen(msg_buffer);

	    	    send(sock, &len, sizeof(int), 0);
	    	    send(sock, msg_buffer, strlen(msg_buffer), 0);
	    	    break;
	     
	    	case KEY_RIGHT:
	    	    printw("Key pressed: RIGHT. ");
	    	    printw("1<R>\n");

	    	    strcpy(msg_buffer, "1<R>");

	    	    len = strlen(msg_buffer);

	    	    send(sock, &len, sizeof(int), 0);
	    	    send(sock, msg_buffer, strlen(msg_buffer), 0);
	    	    break;
    	}
	}
}


void update_map(char * rsp_buffer, int size)
{
	// 7<spēlētāju_skaits>{<ntā_spēlētāja_x_koordināta><ntā_spēlētāja_y_koordināta><ntā_spēlētāja_punkti>}<ēdienu_skaits>{<ntā_ēdiena_x_koordināta><ntā_ēdiena_y_koordināta>}
	// 7<2>{<1><1><0>,<119><29><0>}<0>{(null)}
	char get_buffer[size];
	strcpy(get_buffer, rsp_buffer);
	int player_count = 0;
	int counter = 2;
	int position;
	int points = 0;
	printf("buffer in lobby_info: %s\n", get_buffer);
	while(get_buffer[counter] !=  '>')
	{
		player_count *= 10;
		player_count += get_buffer[counter] - '0';
		counter++;
	}
	counter += 3;
	for (int i = 0; i < player_count; i++)
	{
		position = 0;
		while(get_buffer[counter] !=  '>')
		{
			position *= 10;
			position += get_buffer[counter] - '0';
			counter++;
		}
		players[i].x = position;
		printf("player %s x position: %d\n", players[i].segvards, position);
		counter += 2;
		position = 0;
		while(get_buffer[counter] !=  '>')
		{
			position *= 10;
			position += get_buffer[counter] - '0';
			counter++;
		}
		players[i].y = position;
		printf("player %s y position: %d\n", players[i].segvards, position);
		counter += 2;
		while(get_buffer[counter] !=  '>')
		{
			points *= 10;
			points += get_buffer[counter] - '0';
			counter++;
		}
		players[i].points = points;
		printf("player %s points: %d\n", players[i].segvards, points);
		counter += 3;
	}
	counter += 3;
	set_players_coordinats();
}

void set_players_coordinats() {
    int i = 0;
    for (; i < PLAYERS_COUNT; i++)
    {
        printf("Set for active client: %c, %d. Coordinats: x - %d; y - %d\n", players[i].symbol, players[i].active, players[i].x, players[i].y);
        if (players[i].active == 1)
        {
           // printf("Game map: %d, {%d}{%d}\n", game_map[30][120], players[i].y, players[i].x);
            game_map[players[i].y][players[i].x] = players[i].symbol;
        }
    }
    draw_map();
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
	send(sock, msg_buffer, strlen(msg_buffer), 0);

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
			printf("bout to send lobby_info\n");
			lobby_info(rsp_buffer, strlen(rsp_buffer));
			/*
			if ((rsp_buffer[0] - '0') == C_LOBBY_INFO)
			{
				printf("lobby indo 1\n");
				printf("LOBBY INFO: %s\n", rsp_buffer);
			} 
			else if ((rsp_buffer[0] - '0') == C_USERNAME_TAKEN)
			{
				printf("USERNAME TAKEN: %s\n", rsp_buffer);
			}
			*/
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


    printf("Connecting to the server on port %d ...\n", port);
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