#ifndef BOMB_NETWORK_H
#define BOMB_NETWORK_H

#include <Global.h>
#include <Singleton.h>
#include <SmartPacket.h>

#include <windows.h>
#include <winsock.h>

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 2530

#define BUFFER_LEN 1*1024

class Network
{
    public:
        Network();
        ~Network();
        bool Initialize();
        bool IsInitialized() { return m_initialized; };

        void Connect(std::string host, uint32 port);
        bool IsConnected() { return m_connected; };

        void Worker();

        void SendPacket(SmartPacket *data);
        SmartPacket* BuildPacket(const char *buffer, uint32 size);
        void HandlePacket(SmartPacket *data);

    protected:
        bool m_connected;
        bool m_initialized;

        std::string m_host;
        uint32 m_port;

        int m_mySocket;
        hostent* m_hostent;
        sockaddr_in m_serverSock;
};

#define sNetwork Singleton<Network>::instance()

extern void runNetworkWorker();

#endif
