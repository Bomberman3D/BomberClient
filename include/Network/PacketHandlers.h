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
    void HandleMoveStart(SmartPacket* data);
    void HandleMoveStop(SmartPacket* data);
    void HandleMoveHeartbeat(SmartPacket* data);
    void HandleNewPlayer(SmartPacket* data);
    void HandlePlantBomb(SmartPacket* data);
    void HandleBoxDestroyed(SmartPacket* data);
    void HandlePlayerDied(SmartPacket* data);
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
    {SMSG_ENTER_GAME_RESULT,         &PacketHandlers::HandleEnterGame},
    {SMSG_MAP_INITIAL_DATA,          &PacketHandlers::HandleMapInitialData},
    {SMSG_MOVE_START,                &PacketHandlers::HandleMoveStart},
    {SMSG_MOVE_STOP,                 &PacketHandlers::HandleMoveStop},
    {SMSG_MOVE_HEARTBEAT,            &PacketHandlers::HandleMoveHeartbeat},
    {SMSG_NEW_PLAYER,                &PacketHandlers::HandleNewPlayer},
    {SMSG_PLANT_BOMB,                &PacketHandlers::HandlePlantBomb},
    {SMSG_PLAYER_DIED,               &PacketHandlers::HandlePlayerDied},
    {SMSG_BOX_DESTROYED,             &PacketHandlers::HandleBoxDestroyed}
};

#endif
