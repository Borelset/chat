#include <iostream>
#include <string>
#include "database.h"
#include "server.h"
#include "sys/socket.h"
#include "log.h"

using namespace std;
_log logout;

int main(int argc, char *argv[])
{
    if(argc == 3)
    {
        logout << "select port " << argv[1] << " & " << argv[2] << logout;
        server_start(atoi(argv[1]),atoi(argv[2]));
    }
    else
    {
        logout << "not enough args, select default port" << logout;
        server_start(1234,1235);
    }
}
