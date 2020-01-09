#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define M_JOIN_GAME "0<%s>"

#define SIZE_M_JOIN_GAME 19

#define SIZE_SEGVARDS 16

int main(int argc, char* argv[])
{ 
	
	char name[SIZE_SEGVARDS];
	memset(name, '0', 16);

	char m_buffer[SIZE_M_JOIN_GAME];

	printf("Provide segvards. Note: only first 16 symbols will be read.\n");
	scanf("%16s", name);

	snprintf(m_buffer, SIZE_M_JOIN_GAME, M_JOIN_GAME, name);

	printf("Segavrds: %s\n", m_buffer);

    return 0;
}