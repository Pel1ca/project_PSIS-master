#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "coms/coms.h"
#include "data.h"
#include "UI_library.h"
#include "board_func.h"
#include "game_rules.h"
#include "server.h"
#include "pac_lib.h"
#include "client_funcs.h"
#include "graphics.h"


#define MAX_CLIENTS 256
//constant
cord_t empty_cord = {-1, -1};
color_t no_colors = {0, 0, 0};
//board variables
cord_t limits;
board_t **board;
//communications variables
int fd[MAX_CLIENTS];
int fd_fifo_w;
int fd_r;
unsigned char max_client_id;
int num_clients;
//client positions
cord_t client_cord[2][MAX_CLIENTS];
//client colors
color_t client_color[MAX_CLIENTS];
//clients moves per second
int client_mps[2][MAX_CLIENTS];
//clients inativity
int afk[2][MAX_CLIENTS];
//MUTEX declarations
pthread_mutex_t mutex_init;
pthread_mutex_t *mutex_color;
pthread_mutex_t **mutex_cord;  
pthread_mutex_t **mutex_time;
pthread_mutex_t mutex_fd;
pthread_mutex_t m_num_clients;
	
void free_memory()
{
	free_mutexs();
	free_board(board, limits);
	close(fd_r);
	close(fd_fifo_w);
	exit(1);
}

void *send_score()
{
	int i;
	int *score_board;

	while(1)
	{
		sleep(60);
		pthread_mutex_lock(&mutex_fd);
		score_board = get_score_board();
		for(i = 0; i < 256; i++)
			send_score_board(fd[i], score_board);
		pthread_mutex_unlock(&mutex_fd);
	}

}

int main(int argc, char *argv[])
{
	int fd_s;
	int not_brick;
	int i,j;
	struct sockaddr_in addr;
	color_t client_color = {0, 230, 0};
	pthread_t id;	

	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, free_memory);
	for(i=0;i<2;i++)
                for(j=0;j<MAX_CLIENTS;j++)
                        afk[i][j]= -1;
	
	num_clients = 0;
	board = matrix_from_file(argv[1], &(limits.y), &(limits.x), &not_brick);	
	init_mutexs();
	init_game_rules(limits, not_brick);
	fd_s = create_server(AF_INET, SOCK_STREAM, 0, &addr);
	create_fifo();
	pthread_create(&id, NULL, clock_thread, NULL);
	pthread_create(&id, NULL, handle_connections, (void *)&fd_s);
	pthread_create(&id, NULL, send_score, NULL);
	new_client(client_color, "127.0.0.1", 8081);
	return 0;
}

//initializes mutexs
void init_mutexs()
{
	int i,j;
	
	pthread_mutex_init(&mutex_init, NULL);
	
	mutex_cord =malloc(sizeof(pthread_mutex_t*)*limits.y);
	for(i=0; i<limits.y; i++)
	{
		mutex_cord[i] = malloc(sizeof(pthread_mutex_t)*limits.x);
		for(j=0 ; j<limits.x ; j++)
			pthread_mutex_init(&mutex_cord[i][j], NULL);
	}

	mutex_time = malloc(sizeof(pthread_mutex_t*)*2);
	for(i = 0 ; i<2 ; i++)
        {
		mutex_time[i] = malloc(sizeof(pthread_mutex_t)*MAX_CLIENTS);
        	for(j = 0; j < MAX_CLIENTS ; j++)
      			pthread_mutex_init(&mutex_time[i][j], NULL);
	}
	pthread_mutex_init(&mutex_fd, NULL);
	pthread_mutex_init(&m_num_clients, NULL);
}

//frees mutexs
void free_mutexs()
{
	int i;

	for(i = 0; i < limits.y; i++)
		free(mutex_cord[i]);
	for(i = 0; i < 2; i++)
		free(mutex_time[i]);

        free(mutex_cord);
	free(mutex_time);
}

//initializes a fifo
void create_fifo()
{
	pthread_t thread_id;
	fd_fifo_w = -1;

	unlink(FIFO_NAME);
	//Creating the named file(FIFO)
	if(mkfifo(FIFO_NAME, 0666) == -1)
	{
		perror("error creating fifo");
		exit(-1);
	}
	//creates thread that sends information
	pthread_create(&thread_id, NULL, fifo_thread, NULL);
	//opens fifo to write
	if((fd_fifo_w = open(FIFO_NAME, O_WRONLY)) == -1)
	{
		perror("error opening fifo");
		exit(-1);
	}
}
//thread that will received move info from other clients threads
//and foward them to the clients
void *fifo_thread()
{	
	int i;
	unsigned char buffer[SIZE_BUFF];
	
	if((fd_r = open(FIFO_NAME, O_RDONLY)) == -1)
	{
		perror("error opening fifo");
		exit(-1);
	}
	//receives serialized data
	while( read(fd_r, buffer, SIZE_BUFF) > 0 )
	{
		pthread_mutex_lock(&mutex_fd);
		//finds all valid file descriptors
		for(i = 0 ; i <= max_client_id; i++)
		{	
			//sends serialized data
			if(fd[i] != -1)
			{
				if(write(fd[i], buffer, SIZE_BUFF) < 0)
				{
					fd[i] = -1;
				}
			}
		}
		pthread_mutex_unlock(&mutex_fd);
	}
	close(fd_r);
	return (NULL);
}
//thread that initiates the clock and handles mps and afk vectors
void *clock_thread()
{
	int i,j,curr_f,prev_f = 0;	
	cord_t rand_cord;
	
	while(1)
	{
		sleep(1);
		
		//mutex that doenst allow a new connection to happend assyc
		pthread_mutex_lock(&m_num_clients);
		prev_f = curr_f;
		curr_f = fruit_controler(num_clients, prev_f);
		for(i = 0 ; i<2 ; i++)
                {
			for(j = 0; j<=max_client_id  ;j++)
                        {
				//if disconected
				if(afk[i][j]==-1)
					continue;
				//carefully locking mutex_time
                                pthread_mutex_lock(&mutex_time[i][j]);
                                client_mps[i][j] = 0;
				afk[i][j]++;
				if(afk[i][j] == 30)
				{
					rand_cord = rand_pos(limits, (i+1)*13,empty_cord ,mutex_cord,EMPTY,EMPTY);
					simple_move(get_type(client_cord[i][j]), j, client_color[j],client_cord[i][j],rand_cord);
					afk[i][j]=0;
				}
				pthread_mutex_unlock(&mutex_time[i][j]);
		       	}
                }
		pthread_mutex_unlock(&m_num_clients);
        }
}

//Controls the live number of fruits on the board
int fruit_controler(int n_clients, int prev_f)
{
	int i,fruit_type,curr_f;
	cord_t rand_cord;


	//determinates the number of fruits we need to add on the board	(on this or the next second)
	curr_f = fruits_to_add(n_clients);
	
	//if there is none to add on this second
	if(prev_f == 0)
		return curr_f;	
	if(curr_f < 0)
		return (0);
	//Add fruit for this second
	for(i=0;i<prev_f;i++)
	{
           	rand_cord = rand_pos(limits, (i+1)*17,empty_cord ,mutex_cord,EMPTY,EMPTY);
              	fruit_type = wich_fruit();
                simple_move(fruit_type, -1, no_colors,empty_cord , rand_cord);
                add_num_fruits(1);
        }
	//Return the fruit_numb to add next second
	if(curr_f < prev_f)
		return curr_f;	
	if(curr_f > prev_f)
		return curr_f - prev_f;
	if(curr_f == prev_f)
		return 0;	
	
	return 0;
}

//thread that will handle new clients connecting
void *handle_connections(void *fd_server)
{	
	int *fd_s = (int *)fd_server;
	int fd_c = 0;
	int i;
	char *id;
	color_t color;
	pthread_t thread_id;
	
	//Initialize fd
	for(i=0; i< 256; i++)
		fd[i]=-1;
	max_client_id = 0;
	//Accept connections loop
	while((fd_c = accept(*fd_s, NULL, NULL))!=-1)
	{
		//Find if there are enough spaces on the board for new client
		if(board_full(num_clients+1))
		{
			printf("Not enough space on the board for new client\n");
			close(fd_c);
			continue;	
		}

		//Find an empty spot in the fd vector
		for(i = 0 ; i<MAX_CLIENTS ; i++)
		{
			if(fd[i] == -1)
			{
				break;
			}
		}
		//receives info from client
		recv_color(fd_c, &color);
		client_color[i] = color;
		client_cord[0][i] = rand_pos(limits, 2, empty_cord, mutex_cord,EMPTY,EMPTY);//locks
		update_board(0, color, i, empty_cord, client_cord[0][i]);
		send_mv(fd_fifo_w, 0, empty_cord, client_cord[0][i], color);
		pthread_mutex_unlock(&mutex_cord[client_cord[0][i].y][client_cord[0][i].x]);
		
		client_cord[1][i] = rand_pos(limits, 4, empty_cord, mutex_cord,EMPTY,EMPTY);
		update_board(1, color, i, empty_cord, client_cord[1][i]);
		//send new client info to the other clients
		send_mv(fd_fifo_w, 1, empty_cord, client_cord[1][i], color);
		pthread_mutex_unlock(&mutex_cord[client_cord[1][i].y][client_cord[1][i].x]);
		//send info to client
		pthread_mutex_lock(&m_num_clients);
		pthread_mutex_lock(&mutex_fd);
		send_fc(board, limits, i, fd_c);
		//updates max_client_id
		fd[i] = fd_c;
		set_score_board(i);
		if(i > max_client_id)
			max_client_id = i;
		//increments number of clients in the server
		num_clients++;
		//adds more fruits
		init_client_fruits();
		pthread_mutex_unlock(&mutex_fd);
		pthread_mutex_unlock(&m_num_clients);
		//Init data and creates new client thread
		id = malloc(sizeof(char));
		*id = i;
		//inits afk
		afk[0][i]=0;
		afk[1][i]=0;
		pthread_create(&thread_id, NULL, client_thread, (void*)id);
	}
	return (NULL);
}

//Adds fruits with a new connect
void init_client_fruits()
{
	int i,f,fruit_type;
        cord_t rand_cord;
	
	f=fruits_to_add(num_clients);
	for(i=0;i<f;i++)
	{
		rand_cord = rand_pos(limits, (i+1)*17,empty_cord ,mutex_cord,EMPTY,EMPTY);
		fruit_type = wich_fruit();
		simple_move(fruit_type, -1, no_colors,empty_cord , rand_cord);
		add_num_fruits(1);
	}
}
//thread that will handle one client
void *client_thread(void *var)
{
	unsigned char *id = (unsigned char *)var;
	int c_fd = fd[*id];
	//variables that will change with movement
	unsigned char type, in_type;
	color_t color;
	cord_t aux, end;
	int nbytes, move_type;

	do
	{
		//receives info from the client
		nbytes = recv_mv(c_fd, (char *)&in_type, &aux, &end, &color);
		//checks for user erros
		if (nbytes < 0)
			continue ;
		if (in_type != PACMAN && in_type != MONSTER)
			continue ;
		//resets afk (inativity) and verifies moves per second
		pthread_mutex_lock(&mutex_time[in_type][*id]);		
		if(client_mps[in_type][*id] == 2)
		{
			afk[in_type][*id] = 0;
			pthread_mutex_unlock(&mutex_time[in_type][*id]);			
			continue;
		}
		client_mps[in_type][*id]++;
		afk[in_type][*id] = 0;
		pthread_mutex_unlock(&mutex_time[in_type][*id]);
		//checks the user mensage and transforms end into a valid cord
		pthread_mutex_lock(&mutex_init);
		pthread_mutex_lock(&mutex_cord[client_cord[in_type][*id].y][client_cord[in_type][*id].x]);
		//garante que end está no board e que não é brick
		if(get_move_cord(in_type, client_cord[in_type][*id], &end))
		{
			pthread_mutex_unlock(&mutex_init);
			pthread_mutex_unlock(&mutex_cord[client_cord[in_type][*id].y][client_cord[in_type][*id].x]);
			continue ;
		}
		aux = end;
		pthread_mutex_lock(&mutex_cord[end.y][end.x]);
		//checks if the end cord is valid until it produces a valid cord
		type = in_type;
		move_type = check_end_cord(&type, *id, &end, client_cord[(int)in_type][*id]);
		if(move_type == 2)
		{
			pthread_mutex_unlock(&mutex_cord[aux.y][aux.x]);
			pthread_mutex_lock(&mutex_cord[end.y][end.x]);
		}
		pthread_mutex_unlock(&mutex_init);
		//evaluates and executes the moves
		switch(move_type)
		{
			case 1:
				simple_move(type, *id, color, client_cord[in_type][*id], end);
				break ;
			case 2:
				pthread_mutex_unlock(&mutex_cord[end.y][end.x]);
				eaten_move(type, *id, color, client_cord[in_type][*id]);
				break ;
			case 3:
				swap_move(type, *id, color, client_cord[in_type][*id], end);
				break ;
			case 4:
				eat_move(type, *id, color, client_cord[in_type][*id], end);
				break ;
			case 5:
				super_to_normal(type, *id, color, client_cord[in_type][*id], end);
				break ;
			default:
				break ;
		}
	} while( nbytes > 0);	
	
	disconnect(id, c_fd);	
	pthread_exit(NULL);
}

//makes simple move -> move to an empty end position
void simple_move(unsigned char type, unsigned char id, color_t color, cord_t beg, cord_t end)
{
	unsigned char index_type = type;
	
	if(type == SPACMAN)
		index_type = PACMAN;
	//updates board
	update_board(type, color, id, beg, end);
	//updates client cord info
	if(index_type >= 0 && index_type <= 1)
		client_cord[index_type][id] = end;
	//sends move
	send_mv(fd_fifo_w, type, beg, end, color);
	//unlocks
	if(end.x >= 0 && end.y >= 0)
		pthread_mutex_unlock(&mutex_cord[end.y][end.x]);
	if (beg.x >= 0 && beg.y >=0)
		pthread_mutex_unlock(&mutex_cord[beg.y][beg.x]);
}

void eaten_move(unsigned char type, unsigned char id, color_t color, cord_t beg)
{
	cord_t new_cord;
	
	new_cord = rand_pos(limits, 8, beg, mutex_cord,EMPTY,EMPTY);
	//updates board
	update_board(type, color, id, beg, new_cord);
	//updates client cord info
	client_cord[type][id] = new_cord;
	//sends move
	send_mv(fd_fifo_w, type, beg, new_cord, color);
	pthread_mutex_unlock(&mutex_cord[beg.y][beg.x]);
	pthread_mutex_unlock(&mutex_cord[new_cord.y][new_cord.x]);
}

void super_to_normal(unsigned char type, unsigned char id, color_t color, cord_t beg, cord_t end)
{
	color_t pac_color;	
	unsigned char pac_id;
	
	pac_id = get_id(end);
	pac_color = get_color(end);
	//super pacman to normal
	simple_move(PACMAN, pac_id, pac_color, empty_cord, end);
	//monster goes to rand cord
	eaten_move(type, id, color, beg);
}

//makes a swap move-> swaps info from beg cord with end cord
void swap_move(unsigned char beg_type, unsigned char beg_id, color_t beg_color, cord_t beg, cord_t end)
{
	color_t end_color;
	unsigned char end_type, end_id, index_end_type, index_beg_type;
	
	//gets position of the client cord vect
	beg_type = get_type(beg);
	end_id = get_id(end);
	end_type = get_type(end);
	end_color = get_color(end);
	//swaps positions in the board
	swap_board(beg, end);
	index_end_type = end_type;
	index_beg_type = beg_type;
	if(end_type == SPACMAN)
		index_end_type = PACMAN;
	if(beg_type == SPACMAN)
		index_beg_type = PACMAN;
	//swaps cord
	client_cord[index_beg_type][beg_id] = end;
	client_cord[index_end_type][end_id] = beg;
	//returns swaped cord
	send_mv(fd_fifo_w, beg_type, empty_cord, end, beg_color);
	send_mv(fd_fifo_w, end_type, empty_cord, beg, end_color);
	pthread_mutex_unlock(&mutex_cord[end.y][end.x]);
	pthread_mutex_unlock(&mutex_cord[beg.y][beg.x]);
}

//player eats a piece -> piece at beg eats the piece at end
void eat_move(unsigned char player_type, unsigned char player_id, color_t player_color, cord_t beg, cord_t end)
{
	unsigned char eat_type, eat_id;
	color_t eat_color;
	cord_t eat_cord;

	//gets end cord info
	eat_id = get_id(end);
	eat_type = get_type(end);
	eat_color = get_color(end);
	//makes simple move
	simple_move(player_type, player_id, player_color, beg, end);
	eat_cord = rand_pos(limits, 2, empty_cord, mutex_cord,EMPTY,EMPTY);
	//checks if there are valid postions in the board
	if (eat_cord.x == empty_cord.x && eat_cord.y == empty_cord.y)
		return ;
	//updates board
	update_board(eat_type, eat_color, eat_id, empty_cord, eat_cord);
	//updates clientes positions
	client_cord[eat_type][eat_id] = eat_cord;
	//sends move
	send_mv(fd_fifo_w, eat_type, empty_cord, eat_cord, eat_color);
	pthread_mutex_unlock(&mutex_cord[eat_cord.y][eat_cord.x]);
}

	//remove monster
	//remove pacman
	//reset s_pacman
	//reset score_board
	//remove fd
void disconnect(unsigned char *id, int c_fd)
{
	int f,i;
	cord_t rand_cord;

	//remove monster and pacman
	for(i=0;i<2;i++)
	{
		pthread_mutex_lock(&mutex_cord[client_cord[i][*id].y][client_cord[i][*id].x]);
		if(i == 0)
			reset_spacman(*id);
		if(i == 1)
			reset_score_board(*id);
		pthread_mutex_lock(&mutex_time[i][*id]);
		afk[i][*id] = -1;
		pthread_mutex_unlock(&mutex_time[i][*id]);
		simple_move(i, *id, no_colors, client_cord[i][*id], empty_cord);
	} 
	pthread_mutex_lock(&m_num_clients);
	num_clients = num_clients-1;
	//remove excxess fruits
	f=fruits_to_add(num_clients);
	for(i=f;i<0;i++)
        {
                rand_cord = rand_pos(limits, (i+1)*17,empty_cord ,mutex_cord,LEMON,CHERRY);
                if (rand_cord.x < 0 && rand_cord.y < 0)
			break ;
		simple_move(30, -1, no_colors, rand_cord, empty_cord);
 		add_num_fruits(-1);
	}
	pthread_mutex_unlock(&m_num_clients);
	free(id);
	close(c_fd);
}




////////CLIENT
cord_t c_limits;
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
	char c_id;
	
	fd = create_client(AF_INET, SOCK_STREAM, 0, ip, port);

	send_color(fd, pl_color);
	nbytes = recv_fc(fd, &c_limits, &c_id, &board);
	if (nbytes == -1)
	{
		printf("Error getting board info\n");
		exit(-1);
	}
	draw_board(board,c_limits);
	free_board(board, limits);	
	return (fd);
}

