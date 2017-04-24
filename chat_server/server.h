#ifndef SERVER_H
#define SERVER_H
#include "database.h"

int server_start(int, int);
int login_server(int, int, database*);
int transfer_server(int, database*);
int login_buffer_analysis(char* buffer, char* username, char* password);
int tran_buffer_analysis(char* buffer, char* sorc, char* dest);
int off_reply(int fd, char* sorc, char* dest, sockaddr* addr, socklen_t len);
int transmit(int fd, char* buf, sockaddr* addr, socklen_t len);
int buf_init(char* buf, int length);

#endif // SERVER_H
