#ifndef _GRAPHICS_H
#define _GRAPHICS_H

void draw_piece(char type, color_t c, int x, int y);

void draw_move(cord_t beg, cord_t end, color_t mv_color, char mv_type);

void get_graphic_window(cord_t length);

#endif
