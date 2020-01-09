#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

//TODO: Remove C
int C_PORT;
struct hostent * C_HOST;
int C_SOCK;

#define MSG_JOIN_GAME "0<%s>"
#define SIZE_MSG_JOIN_GAME 20
#define SIZE_SEGVARDS 16

void process_arg(int argc, char* argv[]);
void connect_to_server();
void try_to_join_game();
void run();

int main(int argc, char* argv[])
{ 
	process_arg(argc, argv);

	connect_to_server();

	run();

	//when close connection

    return 0;
}

void run()
{
	//1. Set segvards  
	try_to_join_game();

	while(1)
	{

	}

	return;
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

	send(C_SOCK, &len, sizeof(int), 0);
	send(C_SOCK, msg_buffer, sizeof(msg_buffer), 0);

	//LOBBY_INFO - veiksmīga pievienošanās.
	//GAME_IN_PROGRESS - spēle jau notiek.
	//USERNAME_TAKEN - spēlētāja lietotais segvārds jau ir aizņemts.
	read (C_SOCK, &len, sizeof(int));
	if (len > 0)
	{
		rsp_buffer = (char *)malloc((len+1)*sizeof(char));
	    rsp_buffer[len] = 0;

		if (read(C_SOCK, rsp_buffer, len) == -1)
		{
			fprintf(stderr, ": error receiving data \n");
		    perror("Error description: ");
		} else {

			if (rsp_buffer[0] == '2')
			{
				printf("LOBBY INFO: %s\n", rsp_buffer);
			} 
			else if (rsp_buffer[0] == 3)
			{
				printf("GAME IN PROGRESS: %s\n", rsp_buffer);
			}
			else if (rsp_buffer[0] == 4)
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
		C_HOST = gethostbyname(argv[1]); // Can be mistake?
		if (!C_HOST)
    	{
	        fprintf(stderr, "%s: error: unknown host \n", argv[1]);
	        perror("Error description: ");
        	exit(1);
    	}

		sscanf(argv[2], "%d", &C_PORT);
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

	C_SOCK = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (C_SOCK <= 0)
    {
        fprintf(stderr, "error: cannot create socket\n");
        perror("Error description: ");
        exit(0);
    }

    printf("Socket created. Socket descriptor: %d\n", C_SOCK);


    printf("Connecting to the server on port %d ...", C_PORT);
    address.sin_family = AF_INET;
    address.sin_port = htons(C_PORT);

    memcpy(&address.sin_addr, C_HOST->h_addr_list[0], C_HOST->h_length); // Copying to sin_addr

    result = connect(C_SOCK, (struct sockaddr *)&address, sizeof(address)); // Connecting to server by socket descriptor and address 

   	if (result == -1)
   	{
   		fprintf(stderr, "error: cannot connect to host\n");
        perror("Error description: ");
        exit(0);
   	}

   	printf("Connected to server.\n");
   	return;
}