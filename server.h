#ifndef _SERVER_H
#define _SERVER_H

#define FIFO_NAME "./tmp2/fifo"

void create_fifo();
void *fifo_thread();
void init_mutexs();
void free_mutexs();
void free_memory();

void simple_move(unsigned char type, unsigned char id, color_t color, cord_t beg, cord_t end);

void eaten_move(unsigned char type, unsigned char id, color_t color, cord_t beg);

void super_to_normal(unsigned char type, unsigned char id, color_t color, cord_t beg, cord_t end);

void swap_move(unsigned char beg_type, unsigned char beg_id, color_t beg_color, cord_t beg, cord_t end);

void eat_move(unsigned char player_type, unsigned char player_id, color_t player_color, cord_t beg, cord_t end);

void *client_thread(void *var);
void *clock_thread();
int  fruit_controler(int n_clients, int prev_f);
void init_client_fruits();
void *handle_connections(void *fd_server);
void free_allocs();

void disconnect(unsigned char *id, int fd);

#endif
