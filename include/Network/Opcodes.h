/*                                         *
*            Bomberman Server              *
*                                          *
*    Developed by: Cmaranec (Kennny)       *
*                                          *
*    Copyright (c) 2011                    *
*                                          */

#ifndef __OPCODES_H_
#define __OPCODES_H_

#define VERSION_STR "0001-dev"

#define FAIL 0
#define OK 1

enum Opcodes
{
    MSG_NONE                       = 0x00,
    CMSG_INITIATE_SESSION          = 0x01,
    SMSG_INITIATE_SESSION_RESPONSE = 0x02,
    SMSG_SERVER_BROADCAST          = 0x03,
    CMSG_VALIDATE_VERSION          = 0x04,
    SMSG_VALIDATE_VERSION_RESPONSE = 0x05,
    CMSG_REQUEST_INSTANCE_LIST     = 0x06,
    SMSG_INSTANCE_LIST             = 0x07,
    CMSG_ENTER_GAME                = 0x08,
    SMSG_ENTER_GAME_RESULT         = 0x09,
    CMSG_MOVE_START                = 0x0A,
    CMSG_MOVE_STOP                 = 0x0B,
    CMSG_MOVE_HEARTBEAT            = 0x0C,
    SMSG_MOVE_START                = 0x0D,
    SMSG_MOVE_STOP                 = 0x0E,
    SMSG_MOVE_HEARTBEAT            = 0x0F,
    SMSG_MAP_INITIAL_DATA          = 0x10,
    SMSG_NEW_PLAYER                = 0x11,
    CMSG_PLANT_BOMB                = 0x12,
    SMSG_PLANT_BOMB                = 0x13,
    SMSG_PLAYER_DIED               = 0x14,
    SMSG_BOX_DESTROYED             = 0x15,
    SMSG_NEW_BONUS                 = 0x16,
    CMSG_BONUS_TAKEN               = 0x17,
    SMSG_BONUS_TAKEN               = 0x18,
    SMSG_SCOREBOARD                = 0x19,
    CMSG_RESPAWN_REQUEST           = 0x1A,
    SMSG_RESPAWN                   = 0x1B,
    CMSG_CHAT                      = 0x1C,
    SMSG_CHAT                      = 0x1D,
    MAX_OPCODE
};

#endif
