#include "server.hpp"

int CServer::Start()
{
    LOG("starting smith server v.%i", 0);
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }

    SetAddress(ENET_HOST_ANY, 1337);

    const int maxClients = 8, numChannels = 2, limit_in = 0, limit_out = 0;
    server = enet_host_create(&address,maxClients, numChannels, limit_in, limit_out );
    if(server == nullptr){
        Error("failed to create ENET server host %s", "error");
        return EXIT_FAILURE;
    }


//https://github.com/kbirk/enet-example/blob/master/include/enet/ENetClient.h


    return EXIT_SUCCESS;
}

void CServer::SetAddress(const char* addr, int port)
{
    address.host = ENET_HOST_ANY;
    if(addr != 0){
        enet_address_set_host(&address, addr);
        LOG("set host address to %s", addr);
    }
    address.port = port;
    LOG("port: %i", port);
}

void CServer::Shutdown()
{
    LOG("Stopping Server (%s)", "now");
    enet_host_destroy(server);
}
