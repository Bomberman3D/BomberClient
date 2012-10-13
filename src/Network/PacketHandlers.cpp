#include <Global.h>
#include <Network.h>
#include <PacketHandlers.h>
#include <Spinlock.h>
#include <Application.h>
#include <Storage.h>
#include <Gameplay.h>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>

// Prichozi data (SmartPacket* data) neni nutne kontrolovat - volajici funkce pointer overi

void PacketHandlers::HandleInitiateSessionResponse(SmartPacket* data)
{
    uint32 lock;
    *data >> lock;

    sLockMgr->NeedToken(LOCK_DISPLAYLIST, THREAD_NETWORK);

    while (!sLockMgr->HasToken(LOCK_DISPLAYLIST, THREAD_NETWORK))
        boost::this_thread::yield();

    if (lock > 0)
        sApplication->SetStage(STAGE_MENU, 0);
    sApplication->SetStagePhase(2);

    // Automaticky preda token
    sLockMgr->UnNeedToken(LOCK_DISPLAYLIST, THREAD_NETWORK);

    SmartPacket resp(CMSG_VALIDATE_VERSION);
    resp << VERSION_STR;
    sNetwork->SendPacket(&resp);
}

void PacketHandlers::HandleValidateVersionResponse(SmartPacket* data)
{
    std::string version;
    uint32 matter;

    version = data->readstr();
    *data >> matter;

    // Server ma verzi 'version', 'matter' je priznak toho, zdali muze na tehle verzi hrat
    // 0 = nemuze, 1 = muze

    sApplication->SetStagePhase(3);

    SmartPacket resp(CMSG_REQUEST_INSTANCE_LIST);
    resp << uint32(0);
    sNetwork->SendPacket(&resp);
}

void PacketHandlers::HandleInstanceList(SmartPacket* data)
{
    std::string str;

    sStorage->m_instanceList = data->readstr();

    sApplication->SetStagePhase(4);
}

void PacketHandlers::HandleEnterGame(SmartPacket* data)
{
    uint8 result;
    float startPosX, startPosZ;
    uint32 myId;
    uint32 instanceId;

    *data >> result;
    *data >> startPosX >> startPosZ; //TODO: IMPLEMENT!
    *data >> myId;
    *data >> instanceId;

    if (result != 0)
    {
        //TODO: zobrazit chybu
        //sApplication->SetState(STAGE_MENU);
        return;
    }

    sStorage->m_myId = myId;
    sStorage->m_instanceId = instanceId;

    sStorage->MakeInterThreadRequest(THREAD_NETWORK, REQUEST_MAP_CHANGE, 1);     // TODO: get map id from server
    sStorage->MakeInterThreadRequest(THREAD_NETWORK, REQUEST_GAME_TYPE_CHANGE, GAME_TYPE_MP_CLASSIC); // TODO: get game type from server
    sStorage->MakeInterThreadRequest(THREAD_NETWORK, REQUEST_STAGE_CHANGE, STAGE_LOADING);
    sStorage->MakeInterThreadRequest(THREAD_NETWORK, REQUEST_SUBSTAGE_CHANGE, 0);
}

void PacketHandlers::HandleMapInitialData(SmartPacket* data)
{
    uint32 count;
    uint16 tmpx, tmpy;
    uint8 type;

    uint32 tmp_id;
    float tmp_x, tmp_y;
    uint8 tmp_offset;
    std::string tmp_name;

    for (uint8 i = 0; i < 4; i++)
    {
        *data >> tmp_id;
        *data >> tmp_x >> tmp_y;
        *data >> tmp_offset;
        tmp_name = data->readstr();
    }

    *data >> count;

    ThreadRequestDynamicElement* tmp;
    for (uint32 i = 0; i < count; i++)
    {
        *data >> tmpx;
        *data >> tmpy;
        *data >> type;

        tmp = new ThreadRequestDynamicElement;
        tmp->x = tmpx;
        tmp->y = tmpy;
        tmp->rec.type = type;
        tmp->rec.state = 0;
        tmp->rec.misc = 0;
        tmp->rec.special = NULL;

        sStorage->MakeInterThreadObjectRequest(THREAD_NETWORK, REQUEST_DYNAMIC_MAP_ELEMENT, (void*)tmp);
    }
    tmp = NULL;

    sStorage->MakeInterThreadObjectRequest(THREAD_NETWORK, REQUEST_DYNAMIC_MAP_FILL, NULL);
}
