#include <Global.h>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>

#include <Network.h>
#include <PacketHandlers.h>
#include <Opcodes.h>
#include <SmartPacket.h>

Network::Network()
{
    m_initialized = false;
    m_connected = false;
}

Network::~Network()
{
    closesocket(m_mySocket);
    WSACleanup();
}

bool Network::Initialize()
{
    m_initialized = false;

    WORD wVersionRequested = MAKEWORD(1,1);
    WSADATA data;

    if (WSAStartup(wVersionRequested, &data) != 0)
        return false;

    if ((m_mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        return false;

    m_initialized = true;
    return true;
}

void Network::Connect(std::string host, uint32 port)
{
    if ((m_hostent = gethostbyname(host.c_str())) == NULL)
    {
        m_connected = false;
        return;
    }

    m_serverSock.sin_family = AF_INET;
    m_serverSock.sin_port = htons(port);
    memcpy(&(m_serverSock.sin_addr), m_hostent->h_addr, m_hostent->h_length);
    if (connect(m_mySocket, (sockaddr*)&m_serverSock, sizeof(m_serverSock)) == -1)
    {
        m_connected = false;
        return;
    }

    m_connected = true;
}

void Network::Worker()
{
    char *buf = new char[BUFFER_LEN];
    int result;

    while(1)
    {
        if (!m_connected)
        {
            boost::this_thread::yield();
            continue;
        }

        result = recv(m_mySocket, buf, BUFFER_LEN, 0);
        if (result > 0)
        {
            HandlePacket(BuildPacket(buf, result));
        }
        else if (result == 0)
        {
            // disconnected
        }
        else
        {
            // some error
        }

        boost::this_thread::yield();
    }
}

void runNetworkWorker()
{
    sNetwork->Worker();
}

void Network::SendPacket(SmartPacket *data)
{
    //Total storage size + 4 bytes for opcode + 4 bytes for size
    size_t psize = data->GetSize() + 4 + 4;

    char* buff = new char[psize];
    unsigned int opcode = data->GetOpcode();
    memcpy(&buff[0],&opcode,sizeof(unsigned int));

    unsigned int size = (unsigned int)data->GetSize();
    memcpy(&buff[4],&size,sizeof(unsigned int));

    for(size_t i = 0; i < size; i++)
    {
        *data >> buff[8+i];
    }

    send(m_mySocket, buff, psize, 0);
}

SmartPacket* Network::BuildPacket(const char *buffer, uint32 size)
{
    if (size < 8)
    {
        return NULL;
    }

    unsigned int opcode, psize;

    //at first, parse opcode ID
    memcpy(&opcode,&buffer[0],sizeof(unsigned int));

    if (opcode >= MAX_OPCODE)
        return NULL;

    SmartPacket* packet = new SmartPacket(opcode);

    //next parse size
    memcpy(&psize,&buffer[4],sizeof(unsigned int));

    //and parse the body of packet
    for(size_t i = 0; i < psize; i++)
        *packet << (unsigned char)buffer[8+i];

    return packet;
}

void Network::HandlePacket(SmartPacket *data)
{
    if (!data)
        return;

    for (uint32 i = 0; i < sizeof(PacketHandlerAssign)/sizeof(OpcodeHandler); i++)
        if (PacketHandlerAssign[i].opcode == data->GetOpcode())
            PacketHandlerAssign[i].Handler(data);
}
