#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "time.h"
#include "data.h"
#include "board_func.h"
#include "graphics.h"

struct  board_s 
{   
        char    p_type;
        color_t p_color;
        char    p_id;
};

board_t** board;
color_t no_color = {0, 0, 0};
cord_t  no_cord = {-1, -1};



char get_id(cord_t cord)
{
	return(board[cord.y][cord.x].p_id);
}

char get_type(cord_t cord)
{
	return(board[cord.y][cord.x].p_type);
}

color_t get_color(cord_t cord)
{
	return(board[cord.y][cord.x].p_color);
}

board_t** matrix_from_file(char* file_name, int* lin,int* col, int *not_brick)
{
	FILE* fp;
	int i, j, n = 0;
	char buffer[200];
	
	*not_brick = 0;
	*lin=0;
	*col=0;
	*not_brick = 0;
	
	fp = fopen(file_name,"r");
	if(fp==NULL){
		printf("Error opening file\n");
		exit(-1);
	}
	
	fgets(buffer, sizeof(buffer),fp);
	if(sscanf(buffer,"%d %d%d",&(*col), &(*lin),&n) != 2){
		printf("Invalid board entry values (lin,col))\n");
		exit(-1);
	}
	board = (board_t **)malloc(sizeof(board_t *)*(*lin));
        for(i = 0; i < *lin; i++){
                board[i] = (board_t *)malloc(sizeof(board_t)*(*col));
                fgets(buffer,sizeof(buffer),fp);
		for(j = 0; j < *col; j++){
			if(buffer[j] ==' ')
			{
                        	(*not_brick)++;
				board[i][j].p_type = EMPTY;
			}
			else if(buffer[j] == 'B')
				board[i][j].p_type = BRICK;
			else {
				printf("Invalid char on board\n");
        			exit(-1);
			}
			board[i][j].p_id = 0;
			board[i][j].p_color.red = 0;
			board[i][j].p_color.blue = 0;
			board[i][j].p_color.green = 0;
		}
	}
	fclose(fp);
	return (board);
}

//makes a movemment - if beg cord is invalid adds piece
void update_board(char type, color_t color, char id, cord_t beg, cord_t end)
{	
	
	if(beg.x >= 0 && beg.y >= 0)
	{
		board[beg.y][beg.x].p_type = EMPTY;
		board[beg.y][beg.x].p_color = no_color;
		board[beg.y][beg.x].p_id = 0;
	}
	if(end.x >= 0 && end.y >= 0)
	{
		board[end.y][end.x].p_type = type;
		board[end.y][end.x].p_color = color;
		board[end.y][end.x].p_id = id;
	}
}

void swap_board(cord_t beg, cord_t end)
{
	board_t temp;

	temp = board[beg.y][beg.x];
	board[beg.y][beg.x] = board[end.y][end.x];
	board[end.y][end.x] = temp;
}


cord_t rand_pos(cord_t length, int seed, cord_t lock_cord, pthread_mutex_t **cord, char search1, char search2)
{
	int i,j,n,rnumb;
	int aux_j;
	time_t t;
	int temp;
	cord_t empty;
	cord_t beg_iteration, end_iteration;

	srand(((unsigned) time(&t))*seed);

	rnumb = (rand()%((length.x)*(length.y)));
	temp = rnumb / length.x;
	rnumb = rnumb - temp*length.x;
	beg_iteration.x = rnumb;
	beg_iteration.y = temp;
	aux_j = rnumb;
	
	for(i = temp; i < length.y ; i++)
	{
		for(j = aux_j; j < length.x ; j++)
		{
			if (i == lock_cord.y && j == lock_cord.x)
				continue ;
			end_iteration.x = j;
			end_iteration.y = i;
			pthread_mutex_lock(&cord[i][j]);
			if(board[i][j].p_type == search1 || board[i][j].p_type == search2)
			{
				empty.x=j;
				empty.y=i;
				unlock_cords(beg_iteration, end_iteration, empty, length, lock_cord, cord);
				return empty;		
			}
		}
		aux_j = 0;
	}
	aux_j = rnumb - 1;
	for(i = temp; i >= 0; i--)
	{
		for(j = aux_j; j >= 0; j--)
		{
			if (i == lock_cord.y && j == lock_cord.x)
				continue ;
			beg_iteration.x = j;
			beg_iteration.y = i;
			pthread_mutex_lock(&cord[i][j]);
			if(board[i][j].p_type == search1 || board[i][j].p_type == search2)
			{
				empty.x=j;
				empty.y=i;
				unlock_cords(beg_iteration, end_iteration, empty, length, lock_cord, cord);
				return empty;			
			}
		}
		aux_j = length.x -1;
	}
	empty.x = -1;
	empty.y = -1;
	unlock_cords(beg_iteration, end_iteration, empty, length, lock_cord, cord);
	return empty;
}

void unlock_cords(cord_t beg, cord_t end, cord_t empty, cord_t length, cord_t lock_cord, pthread_mutex_t **cord)
{	
	int i, j;
	int temp;

	temp = beg.x;
	for(i = beg.y; i < length.y; i++)
	{
		for(j = temp; j < length.x; j++)
		{
			if(i == end.y && j == end.x)
			{
				if(end.x != empty.x || end.y != empty.y)
				{
					pthread_mutex_unlock(&cord[i][j]);
				}
				return ;
			}
			if (i == lock_cord.y && j == lock_cord.x)
				continue ;
			if (i == empty.y && j == empty.x)
				continue ;
			pthread_mutex_unlock(&cord[i][j]);
		}
		temp = 0;
	}
}

int is_brick(cord_t cord)
{
	if(board[cord.y][cord.x].p_type == BRICK)
		return(1);
	else
		return(0);
}

void draw_board(board_t **board_l, cord_t length)
{
	int i, j;
	color_t c;
	
	//creates an empty board
	get_graphic_window(length);
	//goes through board and draw
	for(i = 0; i < length.y; i++)
		for(j = 0; j < length.x; j++)
			draw_piece(board_l[i][j].p_type, board_l[i][j].p_color, j, i);
}

void free_board(board_t **board_l, cord_t length)
{
	int i;
	
	for(i = 0; i < length.y; i++)
		free(board_l[i]);
	free(board_l);
}

board_t *get_board_line(int size)
{
	board_t *b_line;

	b_line = malloc(sizeof(board_t)*size);
	return (b_line);
}

//Serialize and Deserialize funcs
unsigned char *serialize_board_p(board_t board, unsigned char* buffer)
{
        buffer = serialize_char(buffer, board.p_type);
        buffer = serialize_color(buffer, board.p_color);
        buffer = serialize_char(buffer, board.p_id);
        return (buffer);
}

unsigned char *deserialize_board_p(unsigned char *buffer, board_t *board)
{
        buffer = deserialize_char(buffer, &(board->p_type));
        buffer = deserialize_color(buffer, &(board->p_color));
        buffer = deserialize_char(buffer, &(board->p_id));
        return (buffer);                
}

void deserialize_board_line(unsigned char *buffer, board_t *board, int size)
{
        int i;

        for(i = 0; i < size; i++)
	       	buffer = deserialize_board_p(buffer, &(board[i]));
}


void serialize_board_line(board_t *board_line, unsigned char * buffer, int size)
{
        int i;

        for(i = 0; i < size; i++)
                buffer = serialize_board_p(board_line[i], buffer);
}
