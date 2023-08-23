#include <stdlib.h> //abs(int)
#include <stdio.h>
#include <pthread.h>

#include "game_rules.h"
#include "board_func.h"

int score_board[256];

cord_t empty_cor = {-1, -1};
cord_t limits;

int num_fruits;
int next_fruit;
int empty_space;
pthread_mutex_t m_fruits;

int s_pacman[256];

void init_game_rules(cord_t bounds, int not_brick)
{
	int i;

	empty_space = not_brick;
	limits = bounds;
	num_fruits = 0;
	for (i = 0; i < 256; i++)
	{
		s_pacman[i] = 0;
		score_board[i] = -1;
	}
	pthread_mutex_init(&m_fruits, NULL);
	//var that controles next fruit to be printed
	next_fruit = (bounds.x + bounds.y) % 2;
}


//Returns number of fruits to place in a board
int fruits_to_add(int num_players)
{
	int max_fruits = (num_players - 1) * 2;
	
	if(num_players > 0)
	{
		pthread_mutex_lock(&m_fruits);
		max_fruits = max_fruits - num_fruits;
		pthread_mutex_unlock(&m_fruits);
		return (max_fruits);
	}
	return(0);
}

int wich_fruit()
{
	if (next_fruit == 1)
	{
		next_fruit = 0;
		return (LEMON);
	}
	else
	{
		next_fruit = 1;
		return (CHERRY);
	}
}

void add_num_fruits(int add)
{
	pthread_mutex_lock(&m_fruits);
	num_fruits += add;
	pthread_mutex_unlock(&m_fruits);
}


int board_full(int n_clients)
{
	int f, not_empty_space;
	
	f = fruits_to_add(n_clients);
	not_empty_space = (n_clients-1) * 2 + n_clients * 2; 
	if(not_empty_space > empty_space)
		return 1;
	return 0;
}


int reset_spacman(char id)
{
	s_pacman[id] = 0;
}

void reset_score_board(char id)
{
	score_board[id] = -1;
}

void set_score_board(char id)
{
	score_board[id] = 0;
}

//receveis input from client, creates a valid end cord and does first evalauation
//checks types and if there was a move checks border
//at the end of this function => end is garanted to be a positive cordinate
int get_move_cord(char type, cord_t beg, cord_t *end)
{
	if (type == PACMAN)
	{
		//checks pac cord
		if ((check_move_pac(*end)) == 0)
			return (1); //NO MOVE
		end->x = end->x + beg.x;
		end->y = end->y + beg.y;
	}
	else if(type == MONSTER)
	{
		if(!update_monster_pos(beg, end))
			return (1);//NO MOVE
	}
	else
	{
		return(2); //INVALID MOVE
	}
	check_walls(end);
	char e_type = get_type(*end);	
	cord_t temp;
	if(e_type == BRICK)
	{
		temp = bounce(*end, beg);
		//invalid bounce => no move
		if(!is_in_board(temp))
			return(1);//NO MOVE
		//piece trapped
		else if(is_brick(temp))
			return(1);//NO MOVE
		else
		{	
			*end = temp;
			return(0);//needs to re-check
		}
	}
	return (0); //POSSIBLE VALID MOVE
}


//if mv is not in the correct format returns 0
int check_move_pac(cord_t mv)
{
	if(mv.x == 0)
	{
		if(mv.y == 1 || mv.y == -1)
			return (1);
	}
	else if(mv.y == 0)
	{
		if(mv.x == 1 || mv.x == -1)
			return (1);
	}
	return (0);
}

int update_monster_pos(cord_t beg, cord_t *end)
{
	cord_t mv = {0, 0};
	//mouse cord => end
	int diff_x = end->x - beg.x;
	int diff_y = end->y - beg.y;
	//no move
	if((diff_x == 0) && (diff_y == 0))
	{
		return (0);//NO MOVE
	}
	//mouse to far way from pacman
	if((abs(diff_x) > 1) || (abs(diff_y) > 1))
		return (0);
	else if(abs(diff_x) >= abs(diff_y))
	{
		if(diff_x > 0)
			mv = move_right();
		else if(diff_x < 0)
			mv = move_left();
	}
	else if(abs(diff_x) <= abs(diff_y)) 
	{	
		if(diff_y > 0)
			mv = move_up();
		else if(diff_y < 0)
			mv = move_down();
	}
	end->x = beg.x + mv.x;
	end->y = beg.y + mv.y;
	return(1);
}

//checks borders and bounces if possible
void check_walls(cord_t *end)
{
	//bate na parede a direita
	if(end->x >= limits.x)
	{
		if(limits.x - 2 >= 0)
			end->x = limits.x - 2;
		else
			end->x = limits.x - 1;
	}
	if(end->x < 0)
	{
		if(limits.x > 1)
			end->x = 1;
		else
			end->x = 0;
	}
	if(end->y >= limits.y)
	{
		if(limits.y - 2 >= 0)
			end->y = limits.y - 2;
		else
			end->y = limits.y - 1;
	}
	if(end->y < 0)
	{
		if(limits.y > 1)
			end->y = 1;
		else
			end->y = 0;
	}
	
}

//address of a board piece at the end cordinates
int check_end_cord(unsigned char *type, unsigned char id, cord_t *end, cord_t beg)
{	
	unsigned char e_id;
       	unsigned char e_type, aux_type;
	cord_t temp;
	
	e_id = get_id(*end);
	e_type = get_type(*end); //get_type(board);
	aux_type = get_type(beg);
	if(e_type == SPACMAN)
		e_type = PACMAN;
	if(e_type == EMPTY)
	{
		*type = aux_type;
		return (1); //SIMPLE MOVE
	}
	else if(e_type == MONSTER)
	{
		if(*type == PACMAN && id != e_id)
		{
			if(s_pacman[id] <= 0)
			{
				score_board[e_id]++;
				return(2); //Player PACMAN is eaten 
			}
			else
			{	//super_power pacman eats monster
				score_board[id]++;
				s_pacman[id]--;
				if(s_pacman[id] <= 0)
					*type = PACMAN;
				else
					*type = SPACMAN;
				return(4);//Player1 eats Player 2
			}
		}
		else if(*type == PACMAN && id == e_id)
		{
			*type = aux_type;
			return(3); //SWAP
		}
		else if(*type == MONSTER)
			return(3); //SWAP
	}
	else if(e_type == PACMAN)
	{
		if(*type == PACMAN || (*type == MONSTER && id == e_id))
			return(3); //SWAP
		else if(*type == MONSTER && id != e_id)
		{
			if (s_pacman[e_id] <= 0)
			{
				score_board[id]++;
				return(4);//Player Monster eats
			}
			else if(s_pacman[e_id] > 0)
			{
				score_board[e_id]++;
				s_pacman[e_id]--;
				if(s_pacman[e_id] > 0)
					return (2); //still s_pacman
				else
				{
					//*type = PACMAN;
					return (5); //pacman changed
				}
			}
		}
	}
	else if(e_type == LEMON || e_type == CHERRY)
	{
		score_board[id]++;//Precisa de sinc
		pthread_mutex_lock(&m_fruits);
		num_fruits--;
		pthread_mutex_unlock(&m_fruits);
		if (*type == PACMAN)
		{
			s_pacman[id] = 2;
			*type = SPACMAN;
		}
		return (1); //Simple move
	}
}
//makes the bounce when hitting a brick
//the bounce could be invalid so we need to check
cord_t bounce(cord_t impossible, cord_t beg)
{
	cord_t temp;
	int diff_x = impossible.x - beg.x;
	int diff_y = impossible.y - beg.y;
	
	//initialize temp
	temp = impossible;
	if(diff_x > 0)
		temp.x = impossible.x - 2;
	else if(diff_x < 0)
		temp.x = impossible.x + 2;
	else if(diff_y > 0)
		temp.y = impossible.y - 2;
	else if(diff_y < 0)
		temp.y = impossible.y + 2;
	return(temp);
}

cord_t move_right()
{
	cord_t mv;

	mv.x = 1;
	mv.y = 0;
	return (mv);
}


cord_t move_left()
{
	cord_t mv;

	mv.x = -1;
	mv.y = 0;
	return (mv);
}

cord_t move_up()
{
	cord_t mv;

	mv.x = 0;
	mv.y = 1;
	return (mv);
}

cord_t move_down()
{
	cord_t mv;

	mv.x = 0;
	mv.y = -1;
	return (mv);
}

int is_in_board(cord_t cord)
{
	if(cord.x < 0 || cord.x >= limits.x || cord.y < 0 || cord.y >= limits.y)
		return (0);
	return (1);
}

int *get_score_board()
{
	return (score_board);
}
