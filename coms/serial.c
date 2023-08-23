#include "serial.h"
#include <stdio.h>
//serialize: puts data in char buffer to send through sockets
//deserialize: rcvs char buffer and returns data

unsigned char *serialize_int(unsigned char *buffer, int a)
{
	//number of bits
	int size_int = sizeof(int) * 8;
	int size_char = sizeof(char) * 8;
	int shift_value;
	int i;
	
	//stores most significant bits in the beginning of the buffer
	for (i = 0; i < size_int / size_char; i++)
	{
		shift_value = size_char * (size_int / size_char - 1 - i);
		buffer[i] = a >> shift_value;
	}
	//return new buffer position
	return (buffer + (size_int / size_char));
}

unsigned char *serialize_char(unsigned char *buffer, char c)
{
	buffer[0] = c;
	return (buffer + sizeof(char));
}

//serialize cord_t
unsigned char *serialize_cord(unsigned char *buffer, cord_t cord)
{
	
	buffer = serialize_int(buffer, cord.x);
	buffer = serialize_int(buffer, cord.y);
	return(buffer);
}


//serialize color_t
unsigned char *serialize_color(unsigned char *buffer, color_t color)
{
	buffer = serialize_char(buffer, color.red);
	buffer = serialize_char(buffer, color.green);
	buffer = serialize_char(buffer, color.blue);
	return(buffer);
}

unsigned int power(int base, int exp)
{
	unsigned int res = 1;
	while (exp > 0)
	{
		res = base * res;
		exp--;
	}
	return (res);
}

unsigned char *deserialize_int(unsigned char *buffer, int *res)
{
	int n = sizeof(int) / sizeof(char);
	int base;
	int i;
	*res = 0;

	base = power(2, sizeof(char) * 8);
	for (i = 0; i < n; i++)
	{
		*res = *res + buffer[i] * power(base, n - i -1);
	}
	return (buffer + sizeof(int)/sizeof(char));		
}

unsigned char *deserialize_char(unsigned char *buffer, char *c)
{
	*c = *buffer;
	return(buffer + sizeof(char));
}

unsigned char *deserialize_cord(unsigned char *buffer, cord_t *cord)
{
	buffer = deserialize_int(buffer, &(cord->x));
	buffer = deserialize_int(buffer, &(cord->y));
	return (buffer);
}

unsigned char *deserialize_color(unsigned char *buffer, color_t *color)
{
	buffer = deserialize_char(buffer, &(color->red));
	buffer = deserialize_char(buffer, &(color->green));
	buffer = deserialize_char(buffer, &(color->blue));
	return(buffer);
}
