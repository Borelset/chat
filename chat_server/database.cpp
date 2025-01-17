#include <sqlite3.h>
#include <string.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "database.h"
#include "log.h"

#define MAX_USERNAME 20
#define MAX_PASSWORD 20

using namespace std;
extern _log logout;

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
    ((struct sockaddr_in*)sockaddr)->sin_port = atoi(p_value[1]);
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
        logout << "fail in open db" << logout;
    }
    sqlite3_exec(db, "select count(*) from user;", getcount, &count, &dbErr);
    if(dbErr != NULL)
    {
        logout << dbErr << logout;
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
    if(dbErr != NULL)
    {
        logout << dbErr << logout;
    }
    count++ ;
    return 0;
}

int database::user_id_search(char* username)
{
    string sql_sen;
    sql_sen = sql_sen + "select id from user where username=\"" + username + "\";";

    int get_id = -1;
    sqlite3_exec(db, sql_sen.c_str(), getid, &get_id, &dbErr);
    if(dbErr != NULL)
    {
        logout << dbErr << logout;
    }

    return get_id;
}

int database::login(char *username, char *processed_password)
{
    string sql_sen;
    sql_sen = sql_sen + "select password from user where username=\"" + username + "\";";

    char sql_password[20];
    sqlite3_exec(db, sql_sen.c_str(), getstring, &sql_password, &dbErr);
    if(dbErr != NULL)
    {
        logout << dbErr << logout;
    }

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
    sql_sen = sql_sen + "insert into verify(id, key, ip, port, tag) values(" + itoa(user_id_search(username)) + ", \"" + key + "\", \"" + ip + "\", \"" + port + "\", 0);";

    sqlite3_exec(db, sql_sen.c_str(), NULL, NULL, &dbErr);
    if(dbErr != NULL)
    {
        logout << dbErr << logout;
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
        logout << dbErr << logout;
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
        logout << dbErr << logout;
    }
    return get_tag;
}

int database::dest_addr(char* username, sockaddr* dest)
{
    string sql_sen;
    sql_sen = sql_sen + "select ip, port from verify where id = " + itoa(user_id_search(username)) + ";";

    sqlite3_exec(db, sql_sen.c_str(), getsockaddr, dest, &dbErr);
    if(dbErr != NULL)
    {
        logout << dbErr << logout;
    }
    return 0;
}

int database::is_exist(char *username)
{
    string sql_sen;
    sql_sen = sql_sen + "select id from verify where id = " + itoa(user_id_search(username)) + ";";

    int get_count = -1;
    sqlite3_exec(db, sql_sen.c_str(), getcount, &get_count, &dbErr);
    if(dbErr != NULL)
    {
        logout << dbErr << logout;
    }
    if(get_count != -1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int _cmp(sockaddr_in* addr1, sockaddr_in* addr2)
{
    return !strcmp(inet_ntoa(addr1->sin_addr), inet_ntoa(addr2->sin_addr)) && (addr1->sin_port == addr2->sin_port);
}

int database::addr_cmp(sockaddr_in* sorc_addr, char *sorc)
{
    string sql_sen;
    sql_sen = sql_sen + "select ip, port from verify where id = " + itoa(user_id_search(sorc)) + ";";

    struct sockaddr_in sorc_database;
    memset(&sorc_database, 0, sizeof(sockaddr));
    sqlite3_exec(db, sql_sen.c_str(), getsockaddr, &sorc_database, &dbErr);
    if(dbErr != NULL)
    {
        logout << dbErr << logout;
    }

    if(_cmp(&sorc_database, sorc_addr))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int database::confirm_port(char *username, int port)
{
    int tag_database = -1;
    string sql_sen;
    sql_sen = sql_sen + "select tag from verify where id = " + itoa(user_id_search(username)) + ";";
    sqlite3_exec(db, sql_sen.c_str(), getcount, &tag_database, &dbErr);
    if(dbErr != NULL)
    {
        logout << dbErr << logout;
    }

    if(tag_database == 1)
    {
        return -1;
    }
    else
    {
        sql_sen = "";
        sql_sen = sql_sen + "update verify set tag = 1, port = " + itoa(port) + " where id = " + itoa(user_id_search(username)) + ";";
        sqlite3_exec(db, sql_sen.c_str(), NULL, NULL, &dbErr);
        if(dbErr != NULL)
        {
            logout << dbErr << logout;
        }
        return 0;
    }
}
