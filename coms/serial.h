#ifndef _SERIAL_H
#define _SERIAL_H

#include "../data.h"

unsigned char *serialize_int(unsigned char *buffer, int a);

unsigned char *serialize_char(unsigned char *buffer, char c);

unsigned char *serialize_cord(unsigned char *buffer, cord_t cord);

unsigned char *serialize_color(unsigned char *buffer, color_t color);

unsigned int power(int base, int exp);

unsigned char *deserialize_int(unsigned char *buffer, int *res);

unsigned char *deserialize_char(unsigned char *buffer, char *c);

unsigned char *deserialize_cord(unsigned char *buffer, cord_t *cord);

unsigned char *deserialize_color(unsigned char *buffer, color_t *color);

#endif
