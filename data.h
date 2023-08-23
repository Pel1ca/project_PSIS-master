#ifndef _DATA_H
#define _DATA_H


enum board_pieces{PACMAN, MONSTER, SPACMAN, LEMON, CHERRY, BRICK, PLACE, EMPTY};

typedef struct cord_s
{
	int x;
	int y;
}		cord_t;

typedef struct color_s
{
	unsigned char red;
	unsigned char green;
	unsigned char blue;
}		color_t;


#endif
