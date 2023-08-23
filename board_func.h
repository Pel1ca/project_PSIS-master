#ifndef _BOARD_FUNC_H
#define _BOARD_FUNC_H

#include "coms/serial.h"
typedef struct board_s board_t;



board_t** matrix_from_file(char* file_name, int* lin, int* col, int* not_brick);

void update_board(char type, color_t color, char id, cord_t beg, cord_t end);

void swap_board(cord_t beg, cord_t end);

cord_t rand_pos(cord_t length, int seed, cord_t lock_cord, pthread_mutex_t **cord, char search1, char search2);

void unlock_cords(cord_t beg, cord_t end, cord_t empty, cord_t length, cord_t lock_cord, pthread_mutex_t **cord);

void draw_board(board_t **board_l, cord_t limits);

int is_brick(cord_t cord);

cord_t bounce(cord_t impossible, cord_t beg);

char get_type(cord_t cord);

char get_id(cord_t cord);

color_t get_color(cord_t cord);

void free_board(board_t **board_l, cord_t length);

void deserialize_board_line(unsigned char *buffer, board_t *board, int size);

void serialize_board_line(board_t *board_line, unsigned char * buffer, int size);

unsigned char *serialize_board_p(board_t board, unsigned char* buffer);

unsigned char *deserialize_board_p(unsigned char *buffer, board_t *board);

board_t *get_board_line(int size);

#endif
