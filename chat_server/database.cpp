#include <sqlite3.h>
#include <string.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include "database.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_USERNAME 20
#define MAX_PASSWORD 20

using namespace std;

int getcount(void* count, int n_column, char** p_value, char** p_name)
{
    *((int*)count) = atoi(p_value[0]);
    return 0;
}

int getid(void* id, int n_column, char** p_value, char** p_name)
{
    *((int*)id) = atoi(p_value[0]);
    return 0;
}

int getstring(void* password, int n_column, char** p_value, char** p_name)
{
    strcpy((char*)password, p_value[0]);
    return 0;
}

int getsockaddr(void* sockaddr, int n_column, char** p_value, char** p_name)
{
    ((struct sockaddr_in*)sockaddr)->sin_port = htons(atoi(p_value[1]));
    ((struct sockaddr_in*)sockaddr)->sin_addr.s_addr = inet_addr(p_value[0]);
    return 0;
}

string itoa(int num)
{
    char *temp = (char*)malloc(sizeof(char[10]));
    sprintf(temp, "%d", num);
    string result(temp);
    free(temp);
    return result;
}



database::database(char * path)
{
    if(sqlite3_open(path, &db) != 0)
    {
        std::cerr << "fail in open db\n";
    }
    sqlite3_exec(db, "select count(*) from user;", getcount, &count, &dbErr);
    if(dbErr != nullptr)
    {
        cout << dbErr << endl;
    }
}

database::~database()
{
    sqlite3_close(db);
}

int database::regist(char *username, char *password)
{
    if(user_id_search(username) != -1)
    {
        return -1;
    }

    string sql_sen;
    sql_sen = sql_sen + "insert into user(id, username, password) values(" + itoa(count) + ", \""
                          + username + "\", \"" + password  + "\");";

    sqlite3_exec(db, sql_sen.c_str(), NULL, NULL, &dbErr);
    count++ ;
    return 0;
}

int database::user_id_search(char* username)
{
    string sql_sen;
    sql_sen = sql_sen + "select id from user where username=\"" + username + "\";";

    int get_id = -1;
    sqlite3_exec(db, sql_sen.c_str(), getid, &get_id, &dbErr);

    return get_id;
}

int database::login(char *username, char *processed_password)
{
    string sql_sen;
    sql_sen = sql_sen + "select password from user where username=\"" + username + "\";";

    char sql_password[20];
    sqlite3_exec(db, sql_sen.c_str(), getstring, &sql_password, &dbErr);


    if(strcmp(sql_password, processed_password))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int database::online(char* username, char *key, char* ip, const char* port)
{
    if(is_online(user_id_search(username)))
    {
        return -1;
    }
    string sql_sen;
    sql_sen = sql_sen + "insert into verify(id, key, ip, port) values(" + itoa(user_id_search(username)) + ", \"" + key + "\", \"" + ip + "\", \"" + port + "\");";

    sqlite3_exec(db, sql_sen.c_str(), NULL, NULL, &dbErr);
    if(dbErr != NULL)
    {
        cout << dbErr << endl;
    }
    return 0;
}

int database::offline(char *username)
{
    string sql_sen;
    sql_sen = sql_sen + "delete from verify where id = " + itoa(user_id_search(username)) + ";";

    sqlite3_exec(db, sql_sen.c_str(), NULL, NULL, &dbErr);
    if(dbErr != NULL)
    {
        cout << dbErr << endl;
    }
    return 0;
}

int database::is_online(int uid)
{
    string sql_sen;
    sql_sen = sql_sen + "select count(*) from verify where id = " + itoa(uid) + ";";

    int get_tag = 0;
    sqlite3_exec(db, sql_sen.c_str(), getcount, &get_tag, &dbErr);
    if(dbErr != NULL)
    {
        cout << dbErr << endl;
    }
    return get_tag;
}

int database::dest_addr(char* username, sockaddr* dest)
{
    string sql_sen;
    sql_sen = sql_sen + "select ip, port from verify where id = " + itoa(user_id_search(username)) + ";";

    sqlite3_exec(db, sql_sen.c_str(), getsockaddr, dest, &dbErr);
    return 0;
}