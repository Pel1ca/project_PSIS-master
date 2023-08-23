#ifndef _SOCKET_H
#define _SOCKET_H

int create_client(int domain, int type, int protocol, char path[], int port);

int create_server(int domain, int type, int protocol, struct sockaddr_in *server_addr);

void check_error(int check, int err_value, char err_str[]);

#endif
