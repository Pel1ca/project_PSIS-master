#ifndef _CLIENT_FUNCS_H
#define _CLIENT_FUNCS_H

void stop_running();

void game_loop(int fd, color_t pl_color);

void handle_key_pres(SDL_Keycode k_pressed, cord_t *end, int  *ptype);

#endif
