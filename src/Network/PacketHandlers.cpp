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
    uint32 mapId;
    uint32 instanceId;

    *data >> result;
    *data >> startPosX >> startPosZ; //TODO: IMPLEMENT!
    *data >> myId;
    *data >> mapId;
    *data >> instanceId;

    if (result != 0)
    {
        //TODO: zobrazit chybu
        sStorage->MakeInterThreadRequest(THREAD_NETWORK, REQUEST_STAGE_CHANGE, STAGE_MENU);
        return;
    }

    sStorage->m_myId = myId;
    sStorage->m_instanceId = instanceId;

    sStorage->MakeInterThreadRequest(THREAD_NETWORK, REQUEST_MAP_CHANGE, mapId);
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

        if (tmp_id == 0 || tmp_id == sStorage->m_myId)
            continue;

        Player* pl = new Player;
        pl->id = tmp_id;
        pl->x = tmp_x;
        pl->y = tmp_y;
        pl->speed = 1.0f;
        pl->artkit = tmp_offset;
        pl->rec = NULL;
        pl->name = tmp_name.c_str();

        sStorage->MakeInterThreadObjectRequest(THREAD_NETWORK, REQUEST_PLAYER_ADD, pl);
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

void PacketHandlers::HandleNewPlayer(SmartPacket* data)
{
    uint32 id;
    uint8 offset;
    float x, y;
    std::string nick;

    *data >> id;

    if (id == 0 || id == sStorage->m_myId)
        return;

    *data >> x;
    *data >> y;
    *data >> offset;
    nick = data->readstr();

    Player* pl = new Player;
    pl->id = id;
    pl->name = nick.c_str();
    pl->x = x;
    pl->y = y;
    pl->speed = 1.0f;
    pl->artkit = offset;
    pl->rec = NULL;

    sStorage->MakeInterThreadObjectRequest(THREAD_NETWORK, REQUEST_PLAYER_ADD, pl);
}

void PacketHandlers::HandleMoveStart(SmartPacket* data)
{
    uint32 id;
    float rot;

    *data >> id;
    *data >> rot;

    if (id == 0 || id == sStorage->m_myId)
        return;

    Player* pl = sNetwork->GetPlayerById(id);
    if (!pl)
        return;

    ThreadRequestPlayerAnim* req = new ThreadRequestPlayerAnim;
    req->id = id;
    req->anim = ANIM_WALK;
    req->flags = 0;
    sStorage->MakeInterThreadObjectRequest(THREAD_NETWORK, REQUEST_PLAYER_ANIM, req);

    ThreadRequestPlayerRotation* req2 = new ThreadRequestPlayerRotation;
    req2->id = id;
    req2->rotation = rot;
    sStorage->MakeInterThreadObjectRequest(THREAD_NETWORK, REQUEST_PLAYER_ROTATION, req2);
}

void PacketHandlers::HandleMoveStop(SmartPacket* data)
{
    uint32 id;
    float rot;

    *data >> id;
    *data >> rot;

    if (id == 0 || id == sStorage->m_myId)
        return;

    Player* pl = sNetwork->GetPlayerById(id);
    if (!pl)
        return;

    ThreadRequestPlayerAnim* req = new ThreadRequestPlayerAnim;
    req->id = id;
    req->anim = ANIM_IDLE;
    req->flags = 0;
    sStorage->MakeInterThreadObjectRequest(THREAD_NETWORK, REQUEST_PLAYER_ANIM, req);

    ThreadRequestPlayerRotation* req2 = new ThreadRequestPlayerRotation;
    req2->id = id;
    req2->rotation = rot;
    sStorage->MakeInterThreadObjectRequest(THREAD_NETWORK, REQUEST_PLAYER_ROTATION, req2);
}

void PacketHandlers::HandleMoveHeartbeat(SmartPacket* data)
{
    uint32 id;
    float x, y, z, rot, speed;

    *data >> id;

    if (id == 0 || id == sStorage->m_myId)
        return;

    *data >> x;
    *data >> y;
    *data >> z;
    *data >> rot;
    *data >> speed;

    Player* pl = sNetwork->GetPlayerById(id);
    if (!pl)
        return;

    ThreadRequestPlayerPos* req = new ThreadRequestPlayerPos;
    req->id = id;
    req->x = x;
    req->y = y;
    req->z = z;
    sStorage->MakeInterThreadObjectRequest(THREAD_NETWORK, REQUEST_PLAYER_POS, req);

    ThreadRequestPlayerRotation* req2 = new ThreadRequestPlayerRotation;
    req2->id = id;
    req2->rotation = rot;
    sStorage->MakeInterThreadObjectRequest(THREAD_NETWORK, REQUEST_PLAYER_ROTATION, req2);
}

void PacketHandlers::HandlePlantBomb(SmartPacket* data)
{
    uint32 owner;
    uint32 x, y, reach;

    *data >> owner;
    *data >> x;
    *data >> y;
    *data >> reach;

    ThreadRequestPlantBomb* req = new ThreadRequestPlantBomb;
    req->id = owner;
    req->x = x;
    req->y = y;
    req->reach = reach;
    sStorage->MakeInterThreadObjectRequest(THREAD_NETWORK, REQUEST_PLANT_BOMB, req);
}

void PacketHandlers::HandlePlayerDied(SmartPacket* data)
{
    uint32 id;
    float x, z;

    *data >> id;
    *data >> x;
    *data >> z;

    ThreadRequestPlayerDeath* req = new ThreadRequestPlayerDeath;
    req->id = id;
    req->x = x;
    req->z = z;
    sStorage->MakeInterThreadObjectRequest(THREAD_NETWORK, REQUEST_PLAYER_DEATH, req);
}

void PacketHandlers::HandleBoxDestroyed(SmartPacket* data)
{
    uint32 x, y;

    *data >> x;
    *data >> y;

    ThreadRequestBoxDestroy* req = new ThreadRequestBoxDestroy;
    req->x = x;
    req->y = y;
    sStorage->MakeInterThreadObjectRequest(THREAD_NETWORK, REQUEST_BOX_DESTROY, req);
}
