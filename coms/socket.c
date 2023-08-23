#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "socket.h"

int	create_client(int domain, int type, int protocol, char path[], int port)
{
	int fd;
	int err;
	struct sockaddr_in server_addr;

	fd = socket(AF_INET, type, protocol);
	check_error(fd, -1, "Socket");
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	if(inet_pton(AF_INET, path, &server_addr.sin_addr) < 0)
	{
		printf("erro no bind\n");
		exit(0);
	}	
	err = connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	check_error(err, -1, "Connect");
	return (fd);
}


int 	create_server(int domain, int type, int protocol, 
struct sockaddr_in *server_addr)
{
	int server_fd;
	int err;

	
	server_fd = socket(AF_INET, type, protocol);
	check_error(server_fd, -1, "Socket");
	server_addr->sin_family = AF_INET;
	server_addr->sin_addr.s_addr = INADDR_ANY;
	server_addr->sin_port = htons(8081);
	
	err = bind(server_fd, (struct sockaddr *)server_addr, sizeof(*server_addr));
	check_error(err, -1, "Bind");
	err = listen(server_fd, 256);
	check_error(err, -1, "Listen");
	printf("socket created and binded\n");
	
	return (server_fd);
}

void	check_error(int check, int err_value, char err_str[])
{
	if (check == err_value)
	{
		perror(err_str);
		exit(-1);
	}
}
