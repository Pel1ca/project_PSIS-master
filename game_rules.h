#ifndef _GAME_RULES_H
#define _GAME_RULES_H

#include "data.h"

void init_game_rules(cord_t bounds, int not_brick);

int fruits_to_add(int num_players);

void add_num_fruits(int add);

int board_full(int n_clients);

int reset_spacman(char id);

void reset_score_board(char id);

int wich_fruit();

int get_move_cord(char type, cord_t beg, cord_t *end);

int check_move_pac(cord_t mv);

int update_monster_pos(cord_t beg, cord_t *end);

void check_walls(cord_t *end);

int is_in_board(cord_t cord);

int check_end_cord(unsigned char *type, unsigned char id, cord_t *end, cord_t beg);

cord_t move_up();

cord_t move_left();

cord_t move_right();

cord_t move_down();

int *get_score_board();


void set_score_board(char id);

#endif
