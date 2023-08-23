#include <stdlib.h>

#include "data.h"
#include "UI_library.h"
#include "board_func.h"
#include "graphics.h"


void draw_piece(char type, color_t c, int x, int y)
{
        switch(type)
        {
                case PACMAN:
                        paint_pacman(x, y, c.red, c.green, c.blue);
                        break;
                case MONSTER:
                        paint_monster(x, y, c.red, c.green, c.blue);
                        break;
                case SPACMAN:
                        paint_powerpacman(x, y, c.red, c.green, c.blue);
                        break;
                case LEMON:
                        paint_lemon(x, y);
                        break;
                case CHERRY:
                        paint_cherry(x, y);
                        break;
                case BRICK:
                        paint_brick(x, y);
                        break;
                default:
                        break;
        }
}


void draw_move(cord_t beg, cord_t end, color_t mv_color, char mv_type)
{
        //it wasnt a new piece
        if ((beg.x >= 0) && (beg.y >= 0))
                clear_place(beg.x, beg.y);
        if ((end.x >= 0) && (end.y >= 0))
                draw_piece(mv_type, mv_color, end.x, end.y);
}

void get_graphic_window(cord_t length)
{
	create_board_window(length.x,length.y);
}
