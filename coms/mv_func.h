#ifndef _FUNC_MOVE_H
#define _FUNC_MOVE_H

#include "serial.h"

int send_mv(int fd, char type, cord_t beg, cord_t end, color_t color);

int recv_mv(int fd, char *type, cord_t *beg, cord_t *end, color_t *color);

int send_sup_mv(int fd, char type1, char type2, cord_t beg1, cord_t end1, cord_t beg2, cord_t end2, color_t color1, color_t color2);

void serialize_move(char type, cord_t beg, cord_t end, color_t color);

void deserialize_mv(char *type, cord_t *beg, cord_t *end, color_t *color);

void send_score_board(int fd, int score_board[]);

void deserialize_first_bytes(char *size_score, char *type_flag);

void serialize_score_board(int score_board[256], unsigned char buff_score[]);


void deserialize_score_board(unsigned char *buff_score, int score_board[]);


void print_score(int score_board[]);

void recv_score_board(int fd, int num_pl);
#endif
