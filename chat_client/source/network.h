#ifndef NETWORK_H
#define NETWORK_H
#include <sys/socket.h>
#include <QUdpSocket>
#include <string>

class network
{
public:
    sockaddr login_server;
    sockaddr trans_server;
    sockaddr native;

    QUdpSocket q_trans_sock;
    QHostAddress q_trans_server;
    QHostAddress q_native;

    socklen_t sockaddr_length = sizeof(sockaddr);
    int logfd = -1;
    int transfd = -1;
    char login_username[20];
    int my_port = -1;
    int trans_server_port = -1;
    int log_tag = 0;

    network();
    ~network();
    int login(const char* username, const char* password);
    int confirm_port();
    int logout();
    int regist(const char* username, const char* password);
    int sendmsg(const char* dest, const char* msg);
    std::string recvmsg();
};

int recv_analysis(char* buffer, char* state, char* transport, char* myip, char* myport);
int buffer_init(char* buffer, int length);

#endif // NETWORK_H
