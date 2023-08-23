#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>

#include "../board_func.h"
#include "fc_funcs.h"


//Functions that deal  with the first communication between server and client, using the first_contact_t data structure




//Sizes without name of struct
int size_of_fc = sizeof(int) * 2 + sizeof(char);
int size_of_color = sizeof(char) * 3;
int size_of_board = sizeof(char) * 5;

//Send Color
void send_color(int fd, color_t color)
{	
	unsigned char  *buffer_send;
	
	buffer_send = malloc(size_of_color);
	serialize_color(buffer_send, color);	
	send(fd, buffer_send, size_of_color, 0);
	free(buffer_send);
}
//Receive Color
int recv_color(int fd, color_t *color)
{
	int nbytes;
	unsigned char *buffer_rcv;
	
	buffer_rcv = malloc(sizeof(size_of_color));
	nbytes = recv(fd, buffer_rcv, size_of_color, 0);
	buffer_rcv = deserialize_color(buffer_rcv , color);
	if(nbytes != size_of_color)
		nbytes = -1;
	return nbytes;	
}


//Creates and Sends first contact information to the client
void send_fc(board_t **board, cord_t limits, char id, int fd_c)
{
	unsigned char *buffer_send;
	unsigned char *buffer_board;
	int i;
	
	//serializes first part of the data
	buffer_send = serialize_fc(limits,id); 
	//send first part of the message
	send(fd_c, buffer_send, size_of_fc, 0);
	buffer_board = malloc(size_of_board * limits.x);
	//send board
	for (i = 0; i < limits.y; i++)
	{
		//add serialize board
		serialize_board_line(board[i], buffer_board, limits.x);
		send(fd_c, buffer_board, limits.x * size_of_board, 0);
	}
	free(buffer_send);
	free(buffer_board);
}
//Creates buffer and fills it
unsigned char *serialize_fc(cord_t limits,char id)
{
	unsigned char *buffer_beg, *buffer_end;

	buffer_beg = malloc(size_of_fc);
	buffer_end = serialize_cord(buffer_beg, limits);
	buffer_end = serialize_char(buffer_end, id);
	
	return(buffer_beg);
}


//Receives first contact ifo from the server
int recv_fc(int fd, cord_t *limits,char *id, board_t*** board)
{
	unsigned char *buffer_rcv;
	unsigned char *buffer_board;
	board_t **board_temp;
	int i,nbytes;
	

	//Alloc buffer
	buffer_rcv = malloc(size_of_fc);
	//rcvs first part of the data
	nbytes = recv(fd, buffer_rcv, size_of_fc, 0);
	//stores first part of the data
	deserialize_fc(buffer_rcv, limits, id);	
	//rcvs second part of the data (board)
	board_temp = (board_t **)malloc(sizeof(board_t *) * (limits->y));
	buffer_board = malloc(size_of_board * limits->x);
	for(i = 0; i < limits->y; i++)
	{
		board_temp[i] = get_board_line(limits->x);
		recv(fd, buffer_board, (limits->x) * size_of_board, 0);
		//add deserialize board	
		deserialize_board_line(buffer_board, board_temp[i], limits->x);
	}
	*board = board_temp;
	if(nbytes != size_of_fc)
		nbytes = -1;
	free(buffer_board);
	return(nbytes);
}

//receives buffer and converts to variables
void deserialize_fc(unsigned char *buffer, cord_t *limits, char *id){

        int i;
        unsigned char *beg = buffer;

        buffer = deserialize_cord(buffer, limits);
        buffer  = deserialize_char(buffer, id);
        free(beg);
}
