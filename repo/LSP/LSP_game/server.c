#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

#define A_CLIENT_CHAR 'A'
#define A_CLIENT_NR 1
#define A_CLIENT_X 1
#define A_CLIENT_Y 1

typedef struct
{
    int sock_desc;
    struct sockaddr address;
    int addr_len;
} connection_t;

typedef struct 
{
	int x;
	int y;
	
} position_t;

typedef struct client
{
	connection_t * connection;
	char* segvards;
	char symbol;
	int x;
	int y;

} client_t;

typedef struct client_node
{
	struct client * client;
	struct client_node_t * next;
	
} client_node_t;


int PORT;
struct hostent * HOST;
int SOCK;
int client_count = 0;

void process_arg(int argc, char* argv[]);

void create_socket();

void * process(void * ptr);

void run(client_node_t **head);

int main(int argc, char* argv[])
{

	client_node_t *head = NULL;
	head = (client_node_t *) malloc(sizeof(client_node_t));
	if (head == NULL)
	{
		return 1;
	}

	process_arg(argc, argv);

	create_socket();

	run(&head);

	return 0;
}

void create_socket()
{
	struct sockaddr_in address;
	int result;

	SOCK = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (SOCK <= 0)
    {
        fprintf(stderr, "error: cannot create socket\n");
        perror("Error description: ");
        exit(0);
    }

    printf("Socket created. Socket descriptor: %d\n", SOCK);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    result = bind(SOCK, (struct sockaddr *)&address, sizeof(struct sockaddr_in));

    if (result != 0)
    {
    	fprintf(stderr, "error: cannot bind socket\n");
        perror("Error description: ");
        exit(0);
    }

    printf("Socket binded. \n");

    result = listen(SOCK, 5); // 5 allowed connected clients,

    if (result != 0)
    {
    	fprintf(stderr, "error: cannot mark socket as passive\n");
        perror("Error description: ");
        exit(0);
    }

    printf("Socket is marked as passive.");

    return;
}

void process_arg(int argc, char* argv[])
{
	if (argc == 3)
	{
		HOST = gethostbyname(argv[1]); // Can be mistake?
		if (!HOST)
    	{
	        fprintf(stderr, "error: unknown host %s\n", argv[1]);
	        perror("Error description: ");
        	exit(1);
    	}

		sscanf(argv[2], "%d", &PORT);
		printf("Arguments processed.\n");

	}
	else 
	{
		printf("Not enough arguments.\n");
		exit(1);
	}

	return;
}

void run(client_node_t **head)
{
	connection_t * connection;
	pthread_t thread;
	client_node_t * current = NULL;
	struct client * new = NULL;

    while (1)
    {
        /* accept incoming connections*/
        connection = (connection_t *)malloc(sizeof(connection_t));
        connection->sock_desc = accept(SOCK, &connection->address, &connection->addr_len);
        printf("Connection %d\n", connection->sock_desc);
        if (connection->sock_desc <= 0)
        {
            free(connection);
        }
        else
        {
            /* start a new thread but do not wait for it */
            current = (client_node_t *) malloc(sizeof(client_node_t));
            new = (struct client *) malloc(sizeof(struct client));
            client_count++;

            if (client_count == A_CLIENT_NR)
            {
            	new->connection = connection;
            	new->symbol = A_CLIENT_CHAR;
            	new->x = A_CLIENT_X;
            	new->y = A_CLIENT_Y;
            }

            (*head) -> client = new;
            (*head) -> next = NULL;

            printf("Conncetd to the client with socket descriptor\n");
            pthread_create(&thread, 0, process, (void *)new);
            pthread_detach(thread);
        }
    }

    return;
}

void * process(void * ptr)
{
	char * buffer;
	int len = 0;
	//client_t client;
	connection_t * conn;
	struct client * client;
	long addr = 0;

	client = (struct client *)ptr; // Getting client from the main thread

	conn = (connection_t *)client -> connection; //  Getting connection from thread

	printf("len1: %d\n", len);

	read (conn->sock_desc, &len, sizeof(int));

	printf("len: %d\n", len);

	if (len > 0)
	{
	    buffer = (char *)malloc((len+1)*sizeof(char));
	    buffer[len] = 0;
	    read(conn->sock_desc, buffer, len);

	    if (buffer[0] == '0'){
	  		printf("First char %c\n", buffer[0]);
	    	printf("Client: %s on socket %d.\n", buffer, conn->sock_desc);
	    }

	    len = strlen("2<LOBBY_INFO>");
	    send(conn->sock_desc, &len, sizeof(int), 0);
	    send(conn->sock_desc, "2<LOBBY_INFO>", sizeof("2<LOBBY_INFO>"), 0);
	    
	    free(buffer);
	}

    close(conn->sock_desc);
    free(conn);
    printf("\n");
    pthread_exit(0);
}