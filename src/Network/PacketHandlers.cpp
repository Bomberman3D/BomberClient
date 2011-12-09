#include <Global.h>
#include <Network.h>
#include <PacketHandlers.h>
#include <Spinlock.h>
#include <Application.h>
#include <Storage.h>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>

// Prichozi data (SmartPacket* data) neni nutne kontrolovat - volajici funkce pointer overi

void PacketHandlers::HandleInitiateSessionResponse(SmartPacket* data)
{
    uint32 lock;
    *data >> lock;

    sLockMgr->NeedToken(LOCK_DISPLAYLIST, THREAD_NETWORK);

    while (!sLockMgr->HasToken(LOCK_DISPLAYLIST, THREAD_NETWORK))
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
    }

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

    sApplication->SetStage(STAGE_LOADING);
}

void PacketHandlers::HandleMapInitialData(SmartPacket* data)
{
    uint32 count;
    uint32 tmpx, tmpy;
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

    /*gDisplayStore.NeedDLToken(DL_TOKEN_NETWORKTHREAD);

    while (!gDisplayStore.HasDLToken(DL_TOKEN_NETWORKTHREAD))
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
    }*/

    for (int i = 0; i < count; i++)
    {
        *data >> tmpx >> tmpy >> type;

        // rozbitelna bedna                //if (type == 1)
        //    gDisplay
        //sDisplay->DrawModel(tmpx-0.51f,0.0f,tmpy-0.51f,5,ANIM_IDLE,true,0.6f);
    }
    /*gDisplay.SetGameState(GAME_GAME);
    gDisplay.SetGameStateStage(255);

    gDisplayStore.UnNeedDLToken(DL_TOKEN_NETWORKTHREAD);
    gDisplayStore.NextDLToken();*/
}
