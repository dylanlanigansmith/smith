#include <common.hpp>
#include <svglobal.hpp>
#include <enet/enet.h>
class CServer
{
    public:
    int Start();
    void SetAddress(const char* addr, int port);
    void Shutdown();
    ENetAddress address;
    ENetHost *server;

};

extern CServer* server;