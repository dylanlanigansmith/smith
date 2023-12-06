
#include <common.hpp>
#include <iostream>
#include <server/server.hpp>

int main(int argc, char **argv)
{
    server = new CServer();
    atexit(enet_deinitialize);
    return server->Start();
   
}

CServer* server;