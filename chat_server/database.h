#ifndef DATABASE_H
#define DATABASE_H
#include <sqlite3.h>
#include <string>
#include <sys/socket.h>

class database
{
private:
    sqlite3* db;
    char* dbErr;
    int count = 0;
public:
    database(char*);
    ~database();
    int user_id_search(char* username);
    int regist(char* username, char* password);
    int login(char* username, char* processed_password);
    int online(char* username, char* key, char* ip, const char* port);
    int offline(char* username);
    int is_online(int uid);
    int dest_addr(char* username, sockaddr* dest);
    int is_exist(char* username);
    int addr_cmp(struct sockaddr_in* sorc_addr, char* sorc);
    int confirm_port(char *username, int port);
};
std::string itoa(int num);

#endif // DATABASE_H
