#include <SDL2/SDL.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h> 
#include <signal.h>
#include <arpa/inet.h>

#include "data.h"
#include "UI_library.h"
#include "pac_lib.h"
#include "coms/coms.h"
#include "board_func.h"
#include "client_funcs.h"
#include "graphics.h"

cord_t limits;
//receives file descriptor pointer and talks with the sever
void *execute_mov(void *fd_void)
{
	//variables to communicate
	int *fd;
	//variables to make movemment
	cord_t beg, end;
	color_t mv_color;
	char mv_type;
	int nbytes;

	fd = (int *)fd_void;
	do
	{
		nbytes = recv_mv(*fd, &mv_type, &beg, &end, &mv_color);
		draw_move(beg, end, mv_color, mv_type);
	}while(nbytes > 0);
	//handle server disconnects
	stop_running();
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	int i, aux[3];
	color_t pl_color;
	
	if(argc != 6)
	{
		printf("wrong number of argumments, write IP, PORT, COLOR in rgb\n");
		exit(0);
	}
	for(i = 3; i < 6; i++)
	{
		aux[i-3] = atoi(argv[i]);
		if(aux[i-3] < 0 || aux[i-3] > 255)
		{
			printf("Invalid colors, must be less 256 and more than 0\n");
			exit(0);
		}
	}
	pl_color.red = aux[0];
	pl_color.green = aux[1];
	pl_color.blue = aux[2];
	new_client(pl_color, argv[1], atoi(argv[2]));
	printf("Server Disconnected\n");
	return (0);
}



void new_client(color_t color, char *ip, int port)
{
	int connect_fd;
	pthread_t thread_id;
	
	connect_fd = init_client(color, ip, port);
	pthread_create(&thread_id, NULL, execute_mov, (void *)&connect_fd);
	game_loop(connect_fd, color);
	close_board_windows();
}

int init_client(color_t pl_color, char *ip, int port)
{
	//mudar para board_t
	board_t **board;
	int fd;
	int nbytes=0;
	char id;

	fd = create_client(AF_INET, SOCK_STREAM, 0, ip, port);

	send_color(fd, pl_color);
	nbytes = recv_fc(fd, &limits, &id, &board);
	if (nbytes == -1)
	{
		printf("Error getting board info\n");
		exit(-1);
	}
	draw_board(board, limits);	
	free_board(board, limits);
	return (fd);
}

