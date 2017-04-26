#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "server.h"
#include "database.h"
#include "log.h"

using namespace std;
extern _log logout;

int server_start(int port1, int port2)
{
    logout << "server starting .. " << logout;
    database user("./database.db");

    int pid = fork();
    if(pid == 0)
    {
        transfer_server(port2, &user);
    }
    else
    {
        login_server(port1, port2, &user);
    }
}

int login_server(int port1, int port2, database* db)
{
    struct sockaddr_in log_addr;
    memset(&log_addr, 0, sizeof(sockaddr_in));
    log_addr.sin_port = htons(port1);
    log_addr.sin_family = AF_INET;
    log_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int loginfd = socket(AF_INET, SOCK_STREAM, 0);
    if(loginfd < 0)
    {
        logout << "login socket: " << strerror(errno) << logout;
    }
    if(bind(loginfd, (struct sockaddr*)&log_addr, sizeof(sockaddr)) < 0)
    {
        logout << "login bind: " << strerror(errno) << logout;
    }

    listen(loginfd, 100);
    logout << "login server ready, running at port: " << port1 << " .." << logout;
    int clifd = -1;
    int tag = 0;
    struct sockaddr_in cliaddr;
    socklen_t sockaddr_length = sizeof(sockaddr);
    char recvbuffer[50];
    char username[20];
    char password[20];

    while(1)
    {
        clifd = accept(loginfd, (struct sockaddr*)&cliaddr, &sockaddr_length);
        if(clifd < 0)
        {
            logout << "login accept: " << strerror(errno) << logout;
        }
        else
        {
            logout << "accept a connection from: " << inet_ntoa(cliaddr.sin_addr) << ":" << (int)cliaddr.sin_port << logout;

            recv(clifd, recvbuffer, 50, 0);
            tag = login_buffer_analysis(recvbuffer, username, password);

            if(tag == 1 && db->login(username, password) == 0)
            {

                if(db->online(username, "NULL", inet_ntoa(cliaddr.sin_addr), itoa(cliaddr.sin_port).c_str()) == -1)
                {
                    string sendbuffer = "201 1 1 1 ";
                    send(clifd, sendbuffer.c_str(), strlen(sendbuffer.c_str()) + 4, 0);
                }
                logout << username << " has login.." << logout;
                string sendbuffer = "200 ";
                sendbuffer = sendbuffer + inet_ntoa(cliaddr.sin_addr) + " " + itoa(cliaddr.sin_port) + " " + itoa(port2)+ " ";
                send(clifd, sendbuffer.c_str(), strlen(sendbuffer.c_str()) + 4, 0);
                close(clifd);
            }
            else if(tag == 0)
            {
                if(db->regist(username, password) == 0)
                {
                    logout << username << " has regist.." << logout;
                    send(clifd, "203 1 1 1 ", 11, 0);
                    close(clifd);
                }
                else
                {
                    send(clifd, "204 1 1 1 ", 11, 0);
                    close(clifd);
                }
            }
            else if(tag == 99)
            {
                logout << username << " has logout.." << logout;
                db->offline(username);
                close(clifd);
            }
            else
            {
                send(clifd, "403 1 1 1 ", 11, 0);
                close(clifd);
            }
        }
    }
}

int transfer_server(int port, database* db)
{
    struct sockaddr_in tran_addr;
    memset(&tran_addr, 0, sizeof(sockaddr_in));
    tran_addr.sin_port = htons(port);
    tran_addr.sin_family = AF_INET;
    tran_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int tranfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(tranfd < 0)
    {
        logout << "tran socket: " << strerror(errno) << logout;
    }
    if(bind(tranfd, (struct sockaddr*)&tran_addr, sizeof(sockaddr)) < 0)
    {
        logout << "tran bind: " << strerror(errno) << logout;
    }

    logout << "transfer server ready, running at port: " << port << " .." << logout;
    struct sockaddr_in cliaddr;
    socklen_t sockaddr_length = sizeof(sockaddr);
    char recvbuffer[300];
    char sorc[20];
    char dest[20];

    while(1)
    {
        buf_init(recvbuffer, 300);
        if(recvfrom(tranfd, recvbuffer, 300, 0, (struct sockaddr*)&cliaddr, &sockaddr_length)<0)
        {
            logout << "tran recvfrom: " << strerror(errno) << logout;
        }
        buf_init(sorc, 20);
        buf_init(dest, 20);
        tran_buffer_analysis(recvbuffer, sorc, dest);
        logout << "recv from: " << inet_ntoa(cliaddr.sin_addr) << ":" << (int)cliaddr.sin_port << logout;
        logout << recvbuffer << logout;

        if(!strcmp(dest, "server"))
        {
            db->confirm_port(sorc, cliaddr.sin_port);
        }
        else if(!db->addr_cmp(&cliaddr, sorc))
        {
            bad_verify_reply(tranfd, sorc, dest, (struct sockaddr*)&cliaddr, sockaddr_length);
        }
        else if(!db->is_exist(dest))
        {
            not_exist_reply(tranfd, sorc, dest, (struct sockaddr*)&cliaddr, sockaddr_length);
        }
        else if(!db->is_online(db->user_id_search(dest)))
        {
            off_reply(tranfd, sorc, dest, (struct sockaddr*)&cliaddr, sockaddr_length);
        }
        else
        {
            db->dest_addr(dest, (sockaddr*)&cliaddr);
            transmit(tranfd, recvbuffer, (struct sockaddr*)&cliaddr, sockaddr_length);
        }

    }
}

int login_buffer_analysis(char* buffer, char* username, char* password)
{
    buf_init(username, 20);
    buf_init(password, 20);
    int ptr = 0;
    for(int i=0;i<50;i++)
    {
        if(buffer[i] == ' ')
        {
            memcpy(username, buffer, i);
            ptr = i;
        }
        else if(buffer[i] == '#')
        {
            memcpy(password, &buffer[ptr+1], i-ptr-1);
            return 1;
        }
        else if(buffer[i] == '$')
        {
            memcpy(password, &buffer[ptr+1], i-ptr-1);
            return 0;
        }
        else if(buffer[i] == '@')
        {
            return 99;
        }
    }
}

int tran_buffer_analysis(char* buffer, char* sorc, char* dest)
{
    int ptr = 0;
    for(int i=0;i<300;i++)
    {
        if(buffer[i] == '>')
        {
            memcpy(sorc, buffer, i);
            ptr = i;
        }
        if(buffer[i] == ' ')
        {
            memcpy(dest, &buffer[ptr+1], i-ptr-1);
            ptr = i;
            break;
        }
    }
}

int bad_verify_reply(int fd, char* sorc, char* dest, struct sockaddr* addr, socklen_t len)
{
    string sendbuf;
    sendbuf = sendbuf + "server>" + sorc + " fail in verify";
    sendto(fd, sendbuf.c_str(), strlen(sendbuf.c_str()), 0, addr, len);
    return 0;
}

int not_exist_reply(int fd, char* sorc, char* dest, struct sockaddr* addr, socklen_t len)
{
    string sendbuf;
    sendbuf = sendbuf + "server>" + sorc + " " + dest + " does not exist";
    sendto(fd, sendbuf.c_str(), strlen(sendbuf.c_str()), 0, addr, len);
    return 0;
}

int off_reply(int fd, char* sorc, char* dest, sockaddr* addr, socklen_t len)
{
    string sendbuf;
    sendbuf = sendbuf + "server>" + sorc + " " + dest + " is offline";
    sendto(fd, sendbuf.c_str(), strlen(sendbuf.c_str()), 0, addr, len);
    return 0;
}

int transmit(int fd, char* buf, sockaddr* addr, socklen_t len)
{
    logout << "sendto: " << inet_ntoa(((sockaddr_in*)addr)->sin_addr) << ":" << ((sockaddr_in*)addr)->sin_port << logout;
    sendto(fd, buf, strlen(buf), 0, addr, len);
    return 0;
}

int buf_init(char* buf, int length)
{
    for(int i=0;i<length;i++)
    {
        buf[i] = '\0';
    }
}
