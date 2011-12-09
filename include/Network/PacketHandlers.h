#ifndef BOMB_PACKETHANDLERS_H
#define BOMB_PACKETHANDLERS_H

#include <Global.h>
#include <SmartPacket.h>

namespace PacketHandlers
{
    void HandleInitiateSessionResponse(SmartPacket* data);
    void HandleValidateVersionResponse(SmartPacket* data);
    void HandleInstanceList(SmartPacket* data);
    void HandleEnterGame(SmartPacket* data);
    void HandleMapInitialData(SmartPacket* data);
};

struct OpcodeHandler
{
    uint32 opcode;
    void (*Handler)(SmartPacket* data);
};

static const OpcodeHandler PacketHandlerAssign[] = {
    {SMSG_INITIATE_SESSION_RESPONSE, &PacketHandlers::HandleInitiateSessionResponse},
    {SMSG_VALIDATE_VERSION_RESPONSE, &PacketHandlers::HandleValidateVersionResponse},
    {SMSG_INSTANCE_LIST,             &PacketHandlers::HandleInstanceList},
    {SMSG_ENTER_GAME_RESULT,         &PacketHandlers::HandleEnterGame}
};

#endif
