#ifndef _SOCKET_LIB_H
#define _SOCKET_LIB_H

#include "../data.h"
#include "../board_func.h"
#include <netinet/in.h>


#define SIZE_BUFF sizeof(int)*4 + sizeof(char) + sizeof(char) * 3 + sizeof(char)*2

/*socket functions */
int create_client(int domain, int type, int protocol, char path[], int port);

int create_server(int domain, int type, int protocol, struct sockaddr_in *server_addr);

void check_error(int check, int err_value, char err_str[]);

/*fc funtions */
void send_color(int fd, color_t color);

int recv_color(int fd, color_t *color);

void send_fc(board_t **board, cord_t limits, char id, int fd_c);

int recv_fc(int fd, cord_t *limits,char *id, board_t*** board);

/*move_t functions*/
int send_mv(int fd, char type, cord_t beg, cord_t end, color_t color);

int recv_mv(int fd, char *type, cord_t *beg, cord_t *end, color_t *color);

int send_sup_mv(int fd, char type1, char type2, cord_t beg1, cord_t end1, cord_t beg2, cord_t end2, color_t color1, color_t color2);

void print_matrix(board_t **b, int l, int col);

void send_score_board(int fd, int score_board[]);

#endif
