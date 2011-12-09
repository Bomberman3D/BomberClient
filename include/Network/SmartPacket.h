/*                                         *
*            Bomberman Server              *
*                                          *
*    Developed by: Cmaranec (Kennny)       *
*                                          *
*    Copyright (c) 2011                    *
*                                          */

#ifndef __SMARTPACKET_H_
#define __SMARTPACKET_H_

#include <iostream>
#include <vector>

#ifndef _WIN32
 #include <string.h>
#endif

#include "Opcodes.h"

class SmartPacket
{
public:
    SmartPacket(unsigned int opcode = MSG_NONE)
    {
        opcodeId = opcode;
        storage.clear();
        size = 0;
        pos = 0;
    }

    void SetOpcode(unsigned int opcode) { opcodeId = opcode; }
    unsigned int GetOpcode() { return opcodeId; }

    void SetPos(size_t newpos) { pos = newpos; }
    unsigned int GetPos() { return pos; }

    size_t GetSize() { return size; }

    SmartPacket &operator<<(unsigned char value)
    {
        storage.resize(size + sizeof(unsigned char));
        memcpy(&storage[size],&value,sizeof(unsigned char));
        size += sizeof(unsigned char);
        return *this;
    }
    SmartPacket &operator<<(unsigned short value)
    {
        storage.resize(size + sizeof(unsigned short));
        memcpy(&storage[size],&value,sizeof(unsigned short));
        size += sizeof(unsigned short);
        return *this;
    }
    SmartPacket &operator<<(unsigned int value)
    {
        storage.resize(size + sizeof(unsigned int));
        memcpy(&storage[size],&value,sizeof(unsigned int));
        size += sizeof(unsigned int);
        return *this;
    }
    SmartPacket &operator<<(float value)
    {
        storage.resize(size + sizeof(float));
        memcpy(&storage[size],&value,sizeof(float));
        size += sizeof(float);
        return *this;
    }
    SmartPacket &operator<<(const char* value)
    {
        for(size_t i = 0; i < strlen(value); i++)
        {
            storage.resize(size + sizeof(unsigned char));
            storage[size] = value[i];
            size += sizeof(unsigned char);
        }
        // String must be null terminated
        storage.resize(size + sizeof(unsigned char));
        storage[size] = 0;
        size += sizeof(unsigned char);
        return *this;
    }

    SmartPacket &operator>>(char &value)
    {
        value = storage[pos];
        pos += sizeof(char);
        return *this;
    }
    SmartPacket &operator>>(unsigned char &value)
    {
        value = storage[pos];
        pos += sizeof(unsigned char);
        return *this;
    }
    SmartPacket &operator>>(unsigned short &value)
    {
        memcpy(&value,&storage[pos],sizeof(unsigned short));
        pos += sizeof(unsigned short);
        return *this;
    }
    SmartPacket &operator>>(unsigned int &value)
    {
        memcpy(&value,&storage[pos],sizeof(unsigned int));
        pos += sizeof(unsigned int);
        return *this;
    }
    SmartPacket &operator>>(float &value)
    {
        memcpy(&value,&storage[pos],sizeof(float));
        pos += sizeof(float);
        return *this;
    }
    const char* readstr(size_t size)
    {
        char* temp = new char[size+sizeof(unsigned char)];
        for(size_t i = 0; i < size; i++)
        {
            temp[i] = storage[pos];
            pos += sizeof(unsigned char);
        }
        temp[size] = '\0';

        return temp;
    }
    const char* readstr()
    {
        char* temp = new char[size+sizeof(unsigned char)];
        for(size_t i = 0; i < storage.size(); i++)
        {
            temp[i] = storage[pos];
            pos += sizeof(unsigned char);
            if (storage[pos-1] == '\0')
                return temp;
        }
        temp[size] = '\0';

        return temp;
    }

private:
    unsigned int opcodeId;
    std::vector<unsigned char> storage;
    size_t size, pos;
};

#endif
