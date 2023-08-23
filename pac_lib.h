#ifndef _PACK_LIB_H
#define _PACK_LIB_H

#include "UI_library.h"
#include "data.h"

int init_client(color_t color, char *id, int port);
void game_loop(int fd, color_t color);
void move_player(int dim_x, int dim_y, int type);
void handle_key_pres(SDL_Keycode k_pressed, cord_t *end, int  *ptype);
int rand_num(int num, int seed);

void new_client(color_t color, char *id, int port);


#endif
