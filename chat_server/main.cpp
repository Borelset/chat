#include <iostream>
#include <string>
#include "database.h"
#include "server.h"
#include "sys/socket.h"

using namespace std;

int main(int argc, char *argv[])
{
    //database user("./database.db");
    //struct sockaddr test = user.dest_addr("sadfyui");
    //cout << user.regist("xdnzxy12323", "123456") << endl;
    //cout << user.login("xdnzxy12323", "123456") << endl;
    server_start(1234,1235);
}
