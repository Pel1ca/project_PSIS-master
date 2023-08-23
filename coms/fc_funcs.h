#ifndef _FC_FUNCS_H
#define _FC_FUNS_H

#include "serial.h"

void send_color(int fd, color_t color);

int recv_color(int fd, color_t *color);

void send_fc(board_t **board, cord_t limits, char id, int fd_c);

unsigned char *serialize_fc(cord_t limits,char id);

int recv_fc(int fd, cord_t *limits,char *id, board_t*** board);

void deserialize_fc(unsigned char *buffer, cord_t *limits, char *id);


void print_matrix(board_t **b, int l, int c);
#endif
