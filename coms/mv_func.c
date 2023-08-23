#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include "mv_func.h"

#define SIZE_SIMPLE_BUFF sizeof(int)*4 + sizeof(char) + sizeof(char) * 3 + 2 * sizeof(char)
#define SIZE_FC_BYTES sizeof(char)*2

/*movement info                                                         
struct move_s{								
        cord_t beg;
	cord_t end;                                                               
	color_t color; 								  
        char type; //Pacman->0 Monster->1
*/												
unsigned char buffer[SIZE_SIMPLE_BUFF];
unsigned char fb_buffer[SIZE_FC_BYTES];

/*sends movent->returns number of bytes send*/
int send_mv(int fd, char type, cord_t beg, cord_t end, color_t color)
{
	int nbytes;

	serialize_move(type, beg, end, color);
	nbytes = write(fd, buffer, SIZE_SIMPLE_BUFF);
	if (nbytes != SIZE_SIMPLE_BUFF)
		nbytes = -1;
	return (nbytes);
}

//recives movement info -> returns number of bytes read
int recv_mv(int fd, char *type, cord_t *beg, cord_t *end, color_t *color)
{
	int nbytes= 10;
	char flag, size_score;

	nbytes = read(fd, fb_buffer, SIZE_FC_BYTES);
	deserialize_first_bytes(&size_score, &flag);
	//move
	if(flag == 0)
	{
		nbytes = read(fd, buffer, SIZE_SIMPLE_BUFF-SIZE_FC_BYTES);
		deserialize_mv(type, beg, end, color);
		if (nbytes != SIZE_SIMPLE_BUFF-SIZE_FC_BYTES)
			nbytes = -1;
	}
	else if(flag == 1)
	{
		recv_score_board(fd, 0);
	}
	else
	{
		return (-1);
	}
	return (nbytes);
}

//transforms data into char string
void serialize_move(char type, cord_t beg, cord_t end, color_t color)
{
	unsigned char *buffer_end;
	char flag = 0, score_board = 0;

	buffer_end = serialize_char(buffer, score_board);
	buffer_end = serialize_char(buffer_end, flag);
	buffer_end = serialize_cord(buffer_end, beg);
	buffer_end = serialize_cord(buffer_end, end);
	buffer_end = serialize_color(buffer_end, color);
	buffer_end = serialize_char(buffer_end, type);
}

//transforms char string into sdata
void deserialize_mv(char *type, cord_t *beg, cord_t *end, color_t *color)
{
        unsigned char *buffer_aux = buffer;
	
	buffer_aux = deserialize_cord(buffer_aux, beg);
	buffer_aux = deserialize_cord(buffer_aux, end);
	buffer_aux = deserialize_color(buffer_aux, color);
	buffer_aux = deserialize_char(buffer_aux, type);
}

void deserialize_first_bytes(char *size_score, char *type_flag)
{
	unsigned char *buffer_aux = fb_buffer;

	buffer_aux = deserialize_char(fb_buffer, size_score);
	buffer_aux = deserialize_char(buffer_aux, type_flag);
}

void send_score_board(int fd, int score_board[])
{
	unsigned char buff_score[1026]; //sizeof(int) * 256 + 2*sizeof(char)
	
	serialize_score_board(score_board, buff_score);
	write(fd, buff_score, 256 * sizeof(int));
}


void serialize_score_board(int score_board[256], unsigned char buff_score[])
{
	char flag = 1, size_score = 1;
	int i;

	buff_score = serialize_char(buff_score, size_score);
	buff_score = serialize_char(buff_score, flag);
	for(i = 0; i < 256; i++)
		buff_score = serialize_int(buff_score, score_board[i]);
}

void deserialize_score_board(unsigned char *buff_score, int score_board[])
{
	int i;
	for(i = 0; i < 256; i++)
		buff_score = deserialize_int(buff_score, &score_board[i]);
}

void recv_score_board(int fd, int num_pl)
{
	char buff_score[1024];
	int score_board[256];

	read(fd, buff_score, 256 * sizeof(int));
	deserialize_score_board(buff_score, score_board);
	print_score(score_board);
}

void print_score(int score_board[])
{
	int i;
	
	printf("================SCORE BOARD===============\n");
	for(i = 0; i < 256; i++)
	{
		if(score_board[i] >= 0)
		{
			printf("Jogador %d -> %d pontos\n", i, score_board[i]);
		}
	}
	printf("===========================================\n");
}
