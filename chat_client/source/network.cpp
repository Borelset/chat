#include <QSettings>
#include <string>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "network.h"

#include <iostream>

//#define SERVER_IP "123.206.206.120"
//#define SERVER_LOGIN_PORT 1234

using namespace std;

char SERVER_IP[20] = "123.206.206.120";
char SERVER_LOGIN_PORT[5] = "1234";

network::network()
{
    QSettings *configIniRead = new QSettings("./config.ini", QSettings::IniFormat);
    QString ipResult = configIniRead->value("/server_address/ip").toString();
    QString portResult = configIniRead->value("/server_address/port").toString();
    strcpy(SERVER_IP, ipResult.toStdString().c_str());
    strcpy(SERVER_LOGIN_PORT, portResult.toStdString().c_str());

    logfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&login_server, 0, sizeof(sockaddr));
    ((struct sockaddr_in*)&login_server)->sin_family = AF_INET;
    ((struct sockaddr_in*)&login_server)->sin_addr.s_addr = inet_addr(SERVER_IP);
    ((struct sockaddr_in*)&login_server)->sin_port = htons(atoi(SERVER_LOGIN_PORT));
}

network::~network()
{
    if(log_tag == 1)
    {
        logout();
    }
    close(logfd);
    close(transfd);
}

int network::login(const char *username, const char *password)
{
    string sendbuf;
    char recvbuf[50];
    sendbuf = sendbuf + username + " " + password + "#";
    if(connect(logfd, &login_server, sizeof(sockaddr)) != 0)
    {
        perror("login connect ");
        close(logfd);
        logfd = socket(AF_INET, SOCK_STREAM, 0);
        return -90;
    }

    send(logfd, sendbuf.c_str(), strlen(sendbuf.c_str()), 0);
    recv(logfd, &recvbuf, 50, 0);
    close(logfd);
    logfd = socket(AF_INET, SOCK_STREAM, 0);
    cout << recvbuf << endl;

    char state[5];
    char tranport[10];
    char myip[20];
    char myport[10];
    recv_analysis(recvbuf, state, tranport, myip, myport);

    if(atoi(state) == 200)
    {
        strcpy(login_username, username);
        /*
        memset(&native, 0, sizeof(sockaddr));
        ((struct sockaddr_in*)&native)->sin_family = AF_INET;
        ((struct sockaddr_in*)&native)->sin_addr.s_addr = inet_addr(myip);
        ((struct sockaddr_in*)&native)->sin_port = htons(atoi(myport));

        memset(&trans_server, 0, sizeof(sockaddr));
        ((struct sockaddr_in*)&trans_server)->sin_family = AF_INET;
        ((struct sockaddr_in*)&trans_server)->sin_addr.s_addr = inet_addr(SERVER_IP);
        ((struct sockaddr_in*)&trans_server)->sin_port = htons(atoi(tranport));
        */
        QString q_ip(SERVER_IP);
        q_trans_server.setAddress(q_ip);
        q_native.setAddress(myip);
        q_trans_sock.bind(atoi(myport), QUdpSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);

        //transfd = socket(AF_INET, SOCK_DGRAM, 0);
        //bind(transfd, (struct sockaddr*)&native, sizeof(sockaddr));
        log_tag = 1;

        trans_server_port = atoi(tranport);
        my_port = atoi(myport);

        return 1;
    }
    else if(atoi(state) == 201)
    {
        return -1;
    }
    else if(atoi(state) == 403)
    {
        return -2;
    }
    else
    {
        return -99;
    }
}

int network::confirm_port()
{
    string sendbuf ;
    sendbuf = sendbuf + login_username + ">server confirm";

    q_trans_sock.writeDatagram(sendbuf.c_str(), strlen(sendbuf.c_str()), q_trans_server, trans_server_port);
}

int network::logout()
{
    logfd = socket(AF_INET, SOCK_STREAM, 0);
    string sendbuf;
    sendbuf = sendbuf + login_username + " 1@";

    connect(logfd, &login_server, sizeof(sockaddr));
    send(logfd, sendbuf.c_str(), strlen(sendbuf.c_str()), 0);
    close(logfd);
    close(transfd);

    return 0;
}

int network::regist(const char *username, const char *password)
{
    char recvbuf[50];
    string sendbuf;
    sendbuf = sendbuf + username + " " + password + "$";

    connect(logfd, &login_server, sizeof(sockaddr));
    send(logfd, sendbuf.c_str(), strlen(sendbuf.c_str()), 0);
    recv(logfd, &recvbuf, 50, 0);
    close(logfd);
    logfd = socket(AF_INET, SOCK_STREAM, 0);

    char state[5];
    char tranport[10];
    char myip[20];
    char myport[10];
    recv_analysis(recvbuf, state, tranport, myip, myport);

    if(atoi(state) == 203)
    {
        return 1;
    }
    else if(atoi(state) == 204)
    {
        return -1;
    }
    else
    {
        return -2;
    }
}

int network::sendmsg(const char *dest, const char *msg)
{
    string sendbuf;
    sendbuf = sendbuf + login_username + ">" + dest + " " + msg;

    cout << dest << endl;
    cout << msg << endl;
    //sendto(transfd, sendbuf.c_str(), strlen(sendbuf.c_str()), 0, &trans_server, sockaddr_length);
    q_trans_sock.writeDatagram(sendbuf.c_str(), strlen(sendbuf.c_str()), q_trans_server, trans_server_port);
}

string network::recvmsg()
{
    char recv[300];
    memset(recv, '\0', 300);
    q_trans_sock.readDatagram(recv, 300);

    string result = recv;
    return result;
}

int recv_analysis(char* buffer, char* state, char* transport, char* myip, char* myport)
{
    int ptr1 = 0;
    int ptr2 = 0;
    int count = 0;
    for(int i=0;i<50;i++)
    {
        if(buffer[i] == ' ' && count == 0)
        {
            ptr2 = i;
            buffer_init(state, 5);
            memcpy(state, buffer, ptr2-ptr1);
            ptr1 = i+1;
            count++;
        }
        else if(buffer[i] == ' ' && count == 1)
        {
            ptr2 = i;
            buffer_init(myip, 20);
            memcpy(myip, &buffer[ptr1], ptr2-ptr1);
            ptr1 = i+1;
            count++;
        }
        else if(buffer[i] == ' ' && count == 2)
        {
            ptr2 = i;
            buffer_init(myport, 10);
            memcpy(myport, &buffer[ptr1], ptr2-ptr1);
            ptr1 = i+1;
            count++;
        }
        else if(buffer[i] == ' ' && count == 3)
        {
            ptr2 = i;
            buffer_init(transport, 10);
            memcpy(transport, &buffer[ptr1], ptr2-ptr1);
            ptr1 = i+1;
            count++;
        }
    }
    return 0;
}

int buffer_init(char* buffer, int length)
{
    for(int i=0;i<length;i++)
    {
        buffer[i] = 0;
    }
    return 0;
}
