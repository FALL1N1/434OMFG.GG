/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Common.h"
#include "Transport.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "Path.h"
#include "ScriptMgr.h"
#include "WorldPacket.h"
#include "DBCStores.h"
#include "World.h"
#include "GameObjectAI.h"
#include "Player.h"
#include "TemporarySummon.h"
#include "Totem.h"

void MapManager::LoadTransports()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = WorldDatabase.Query("SELECT guid, entry, name, period, ScriptName FROM transports");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 transports. DB table `transports` is empty!");
        return;
    }

    uint32 count = 0;

    do
    {

        Field* fields = result->Fetch();
        uint32 lowguid = fields[0].GetUInt32();
        uint32 entry = fields[1].GetUInt32();
        std::string name = fields[2].GetString();
        uint32 period = fields[3].GetUInt32();
        uint32 scriptId = sObjectMgr->GetScriptId(fields[4].GetCString());

        GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(entry);

        if (!goinfo)
        {
            TC_LOG_ERROR("sql.sql", "Transport ID:%u, Name: %s, will not be loaded, gameobject_template missing", entry, name.c_str());
            continue;
        }

        if (goinfo->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
        {
            TC_LOG_ERROR("sql.sql", "Transport ID:%u, Name: %s, will not be loaded, gameobject_template type wrong", entry, name.c_str());
            continue;
        }

        // TC_LOG_INFO("server.loading", "Loading transport %d between %s, %s", entry, name.c_str(), goinfo->name);

        std::set<uint32> mapsUsed;

        Transport* t = new Transport(period, scriptId);
        if (!t->GenerateWaypoints(goinfo, mapsUsed))
            // skip transports with empty waypoints list
        {
            TC_LOG_ERROR("sql.sql", "Transport (path id %u) path size = 0. Transport ignored, check DBC files or transport GO data0 field.", goinfo->moTransport.taxiPathId);
            delete t;
            continue;
        }

        TaxiPathNodeEntry const* startNode = t->KeyFrames.begin()->Node;
        uint32 mapid = startNode->mapid;
        float x = startNode->x;
        float y = startNode->y;
        float z = startNode->z;
        float o = t->KeyFrames.begin()->InitialOrientation;

        // creates the Gameobject
        if (!t->Create(lowguid, entry, mapid, x, y, z, o, 255, 0))
        {
            delete t;
            continue;
        }

        m_Transports.insert(t);

        for (std::set<uint32>::const_iterator i = mapsUsed.begin(); i != mapsUsed.end(); ++i)
            m_TransportsByMap[*i].insert(t);

        //If we someday decide to use the grid to track transports, here:
        t->SetMap(sMapMgr->CreateBaseMap(mapid));
        t->SetZoneScript();
        t->AddToWorld();

        ++count;
    }
    while (result->NextRow());

    // check transport data DB integrity
    result = WorldDatabase.Query("SELECT gameobject.guid, gameobject.id, transports.name FROM gameobject, transports WHERE gameobject.id = transports.entry");
    if (result)                                              // wrong data found
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 guid  = fields[0].GetUInt32();
            uint32 entry = fields[1].GetUInt32();
            std::string name = fields[2].GetString();
            TC_LOG_ERROR("sql.sql", "Transport %u '%s' have record (GUID: %u) in `gameobject`. Transports must not have any records in `gameobject` or its behavior will be unpredictable/bugged.", entry, name.c_str(), guid);
        }
        while (result->NextRow());
    }


    TC_LOG_INFO("server.loading", ">> Loaded %u transports in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void MapManager::LoadTransportNPCs()
{
    uint32 oldMSTime = getMSTime();

    //                                                 0       1            2                3             4             5             6        7
    QueryResult result = WorldDatabase.Query("SELECT guid, npc_entry, transport_entry, TransOffsetX, TransOffsetY, TransOffsetZ, TransOffsetO, emote FROM creature_transport");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 transport NPCs. DB table `creature_transport` is empty!");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 guid = fields[0].GetInt32();
        uint32 entry = fields[1].GetInt32();
        uint32 transportEntry = fields[2].GetInt32();
        float tX = fields[3].GetFloat();
        float tY = fields[4].GetFloat();
        float tZ = fields[5].GetFloat();
        float tO = fields[6].GetFloat();
        uint32 anim = fields[7].GetInt32();

        for (MapManager::TransportSet::iterator itr = m_Transports.begin(); itr != m_Transports.end(); ++itr)
        {
            if ((*itr)->GetEntry() == transportEntry)
            {
                (*itr)->AddNPCPassenger(guid, entry, tX, tY, tZ, tO, anim);
                break;
            }
        }

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u transport npcs in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void MapManager::LoadTransportGOs()
{
    uint32 oldMSTime = getMSTime();

    //                                                 0       1            2                3             4             5             6
    QueryResult result = WorldDatabase.Query("SELECT guid, go_entry, transport_entry, TransOffsetX, TransOffsetY, TransOffsetZ, TransOffsetO FROM gameobject_transport");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 transport GOs. DB table `gameobject_transport` is empty!");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 guid = fields[0].GetInt32();
        uint32 entry = fields[1].GetInt32();
        uint32 transportEntry = fields[2].GetInt32();
        float tX = fields[3].GetFloat();
        float tY = fields[4].GetFloat();
        float tZ = fields[5].GetFloat();
        float tO = fields[6].GetFloat();

        for (MapManager::TransportSet::iterator itr = m_Transports.begin(); itr != m_Transports.end(); ++itr)
        {
            if ((*itr)->GetEntry() == transportEntry)
            {
                (*itr)->AddGOPassenger(guid, entry, tX, tY, tZ, tO);
                break;
            }
        }

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u transport gos in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

Transport::Transport(uint32 period, uint32 script) : GameObject(), m_pathTime(0),
                                                     currenttguid(0), ScriptId(script), m_nextNodeTime(0), inInstance(false),
                                                     _isMoving(true), _pendingStop(false), _triggeredArrivalEvent(false), _triggeredDepartureEvent(false),
                                                     _asToBeDeleted(false)
{
    m_updateFlag = (UPDATEFLAG_TRANSPORT | UPDATEFLAG_STATIONARY_POSITION | UPDATEFLAG_ROTATION);
}

Transport::~Transport()
{
    for (CreatureSet::iterator itr = m_NPCPassengerSet.begin(); itr != m_NPCPassengerSet.end(); ++itr)
    {
        (*itr)->SetTransport(NULL);
        if ((*itr)->ToTempSummon())
            (*itr)->DespawnOrUnsummon();
        else
            (*itr)->AddObjectToRemoveList();
    }

    for (GameObjectSet::iterator itr = m_GOPassengerSet.begin(); itr != m_GOPassengerSet.end(); ++itr)
    {
        (*itr)->SetTransport(NULL);
        (*itr)->AddObjectToRemoveList();
    }

    m_NPCPassengerSet.clear();
    m_GOPassengerSet.clear();
    m_passengers.clear();
}

bool Transport::Create(uint32 guidlow, uint32 entry, uint32 mapid, float x, float y, float z, float ang, uint32 animprogress, uint32 dynflags)
{
    Relocate(x, y, z, ang);
    // instance id and phaseMask isn't set to values different from std.

    if (!IsPositionValid())
    {
        TC_LOG_ERROR("entities.transport", "Transport (GUID: %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)",
            guidlow, x, y);
        return false;
    }

    Object::_Create(guidlow, 0, HIGHGUID_MO_TRANSPORT);

    GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(entry);

    if (!goinfo)
    {
        TC_LOG_ERROR("sql.sql", "Transport not created: entry in `gameobject_template` not found, guidlow: %u map: %u  (X: %f Y: %f Z: %f) ang: %f", guidlow, mapid, x, y, z, ang);
        return false;
    }

    m_goInfo = goinfo;


    // initialize waypoints
    _nextFrame = KeyFrames.begin();
    _currentFrame = _nextFrame++;
    _triggeredArrivalEvent = false;
    _triggeredDepartureEvent = false;

    m_goValue.Transport.PathProgress = 0;
    SetObjectScale(goinfo->size);
    SetUInt32Value(GAMEOBJECT_FACTION, goinfo->faction);
    SetUInt32Value(GAMEOBJECT_FLAGS, goinfo->flags);
    SetPeriod(m_pathTime);
    SetEntry(goinfo->entry);
    SetDisplayId(goinfo->displayId);
    SetGoState(!goinfo->moTransport.canBeStopped ? GO_STATE_READY : GO_STATE_ACTIVE);
    SetGoType(GameobjectTypes(goinfo->type));
    SetGoAnimProgress(animprogress);
    if (dynflags)
        SetUInt32Value(GAMEOBJECT_DYNAMIC, MAKE_PAIR32(0, dynflags));
    SetName(goinfo->name);
    UpdateRotationFields(0.0f, 1.0f);
    m_model = GameObjectModel::Create(*this);

    return true;
}

bool Transport::GenerateWaypoints(const GameObjectTemplate* goInfo, std::set<uint32> &mapsUsed)
{
    uint32 pathId = goInfo->moTransport.taxiPathId;
    TaxiPathNodeList const& path = sTaxiPathNodesByPath[pathId];
    if (path.empty())
        return false;

    Movement::PointsArray splinePath, allPoints;
    bool mapChange = false;
    for (size_t i = 0; i < path.size(); ++i)
        allPoints.push_back(G3D::Vector3(path[i].x, path[i].y, path[i].z));

    // Add extra points to allow derivative calculations for all path nodes
    allPoints.insert(allPoints.begin(), allPoints.front().lerp(allPoints[1], -0.2f));
    allPoints.push_back(allPoints.back().lerp(allPoints[allPoints.size() - 2], -0.2f));
    allPoints.push_back(allPoints.back().lerp(allPoints[allPoints.size() - 2], -1.0f));

    SplineRawInitializer initer(allPoints);
    TransportSpline orientationSpline;
    orientationSpline.init_spline_custom(initer);
    orientationSpline.initLengths();

    for (size_t i = 0; i < path.size(); ++i)
    {
        if (!mapChange)
        {
            TaxiPathNodeEntry const& node_i = path[i];
            if (i != path.size() - 1 && (node_i.actionFlag & 1 || node_i.mapid != path[i + 1].mapid))
            {
                KeyFrames.back().Teleport = true;
                mapChange = true;
            }
            else
            {
                KeyFrame k(node_i);
                G3D::Vector3 h;
                orientationSpline.evaluate_derivative(i + 1, 0.0f, h);
                k.InitialOrientation = Position::NormalizeOrientation(atan2(h.y, h.x) + M_PI);

                KeyFrames.push_back(k);
                splinePath.push_back(G3D::Vector3(node_i.x, node_i.y, node_i.z));
                mapsUsed.insert(k.Node->mapid);
            }
        }
        else
            mapChange = false;
    }

    if (splinePath.size() >= 2)
    {
        // Remove special catmull-rom spline points
        if (!KeyFrames.front().IsStopFrame() && !KeyFrames.front().Node->arrivalEventID && !KeyFrames.front().Node->departureEventID)
        {
            splinePath.erase(splinePath.begin());
            KeyFrames.erase(KeyFrames.begin());
        }
        if (!KeyFrames.back().IsStopFrame() && !KeyFrames.back().Node->arrivalEventID && !KeyFrames.back().Node->departureEventID)
        {
            splinePath.pop_back();
            KeyFrames.pop_back();
        }
    }

    if (KeyFrames.empty())
        return false;

    if (mapsUsed.size() > 1)
    {
        for (std::set<uint32>::const_iterator itr = mapsUsed.begin(); itr != mapsUsed.end(); ++itr)
            ASSERT(!sMapStore.LookupEntry(*itr)->Instanceable());

        inInstance = false;
    }
    else
        inInstance = sMapStore.LookupEntry(*mapsUsed.begin())->Instanceable();

    // last to first is always "teleport", even for closed paths
    KeyFrames.back().Teleport = true;

    const float speed = float(goInfo->moTransport.moveSpeed);
    const float accel = float(goInfo->moTransport.accelRate);
    const float accel_dist = 0.5f * speed * speed / accel;

    accelTime = speed / accel;
    accelDist = accel_dist;

    int32 firstStop = -1;
    int32 lastStop = -1;

    // first cell is arrived at by teleportation :S
    KeyFrames[0].DistFromPrev = 0;
    KeyFrames[0].Index = 1;
    if (KeyFrames[0].IsStopFrame())
    {
        firstStop = 0;
        lastStop = 0;
    }

    // find the rest of the distances between key points
    // Every path segment has its own spline
    size_t start = 0;
    for (size_t i = 1; i < KeyFrames.size(); ++i)
    {
        if (KeyFrames[i - 1].Teleport || i + 1 == KeyFrames.size())
        {
            size_t extra = !KeyFrames[i - 1].Teleport ? 1 : 0;
            TransportSpline* spline = new TransportSpline();
            spline->init_spline(&splinePath[start], i - start + extra, Movement::SplineBase::ModeCatmullrom);
            spline->initLengths();
            for (size_t j = start; j < i + extra; ++j)
            {
                KeyFrames[j].Index = j - start + 1;
                KeyFrames[j].DistFromPrev = spline->length(j - start, j + 1 - start);
                if (j > 0)
                    KeyFrames[j - 1].NextDistFromPrev = KeyFrames[j].DistFromPrev;
                KeyFrames[j].Spline = spline;
            }

            if (KeyFrames[i - 1].Teleport)
            {
                KeyFrames[i].Index = i - start + 1;
                KeyFrames[i].DistFromPrev = 0.0f;
                KeyFrames[i - 1].NextDistFromPrev = 0.0f;
                KeyFrames[i].Spline = spline;
            }

            start = i;
        }

        if (KeyFrames[i].IsStopFrame())
        {
            // remember first stop frame
            if (firstStop == -1)
                firstStop = i;
            lastStop = i;
        }
    }

    KeyFrames.back().NextDistFromPrev = KeyFrames.front().DistFromPrev;

    if (firstStop == -1 || lastStop == -1)
        firstStop = lastStop = 0;

    // at stopping keyframes, we define distSinceStop == 0,
    // and distUntilStop is to the next stopping keyframe.
    // this is required to properly handle cases of two stopping frames in a row (yes they do exist)
    float tmpDist = 0.0f;
    for (size_t i = 0; i < KeyFrames.size(); ++i)
    {
        int32 j = (i + lastStop) % KeyFrames.size();
        if (KeyFrames[j].IsStopFrame() || j == lastStop)
            tmpDist = 0.0f;
        else
            tmpDist += KeyFrames[j].DistFromPrev;
        KeyFrames[j].DistSinceStop = tmpDist;
    }

    tmpDist = 0.0f;
    for (int32 i = int32(KeyFrames.size()) - 1; i >= 0; i--)
    {
        int32 j = (i + firstStop) % KeyFrames.size();
        tmpDist += KeyFrames[(j + 1) % KeyFrames.size()].DistFromPrev;
        KeyFrames[j].DistUntilStop = tmpDist;
        if (KeyFrames[j].IsStopFrame() || j == firstStop)
            tmpDist = 0.0f;
    }

    for (size_t i = 0; i < KeyFrames.size(); ++i)
    {
        float total_dist = KeyFrames[i].DistSinceStop + KeyFrames[i].DistUntilStop;
        if (total_dist < 2 * accel_dist) // won't reach full speed
        {
            if (KeyFrames[i].DistSinceStop < KeyFrames[i].DistUntilStop) // is still accelerating
            {
                // calculate accel+brake time for this short segment
                float segment_time = 2.0f * sqrt((KeyFrames[i].DistUntilStop + KeyFrames[i].DistSinceStop) / accel);
                // substract acceleration time
                KeyFrames[i].TimeTo = segment_time - sqrt(2 * KeyFrames[i].DistSinceStop / accel);
            }
            else // slowing down
                KeyFrames[i].TimeTo = sqrt(2 * KeyFrames[i].DistUntilStop / accel);
        }
        else if (KeyFrames[i].DistSinceStop < accel_dist) // still accelerating (but will reach full speed)
        {
            // calculate accel + cruise + brake time for this long segment
            float segment_time = (KeyFrames[i].DistUntilStop + KeyFrames[i].DistSinceStop) / speed + (speed / accel);
            // substract acceleration time
            KeyFrames[i].TimeTo = segment_time - sqrt(2 * KeyFrames[i].DistSinceStop / accel);
        }
        else if (KeyFrames[i].DistUntilStop < accel_dist) // already slowing down (but reached full speed)
            KeyFrames[i].TimeTo = sqrt(2 * KeyFrames[i].DistUntilStop / accel);
        else // at full speed
            KeyFrames[i].TimeTo = (KeyFrames[i].DistUntilStop / speed) + (0.5f * speed / accel);
    }

    // calculate tFrom times from tTo times
    float segmentTime = 0.0f;
    for (size_t i = 0; i < KeyFrames.size(); ++i)
    {
        int32 j = (i + lastStop) % KeyFrames.size();
        if (KeyFrames[j].IsStopFrame() || j == lastStop)
            segmentTime = KeyFrames[j].TimeTo;
        KeyFrames[j].TimeFrom = segmentTime - KeyFrames[j].TimeTo;
    }

    // calculate path times
    KeyFrames[0].ArriveTime = 0;
    float curPathTime = 0.0f;
    if (KeyFrames[0].IsStopFrame())
    {
        curPathTime = float(KeyFrames[0].Node->delay);
        KeyFrames[0].DepartureTime = uint32(curPathTime * IN_MILLISECONDS);
    }

    for (size_t i = 1; i < KeyFrames.size(); ++i)
    {
        curPathTime += KeyFrames[i - 1].TimeTo;
        if (KeyFrames[i].IsStopFrame())
        {
            KeyFrames[i].ArriveTime = uint32(curPathTime * IN_MILLISECONDS);
            KeyFrames[i - 1].NextArriveTime = KeyFrames[i].ArriveTime;
            curPathTime += float(KeyFrames[i].Node->delay);
            KeyFrames[i].DepartureTime = uint32(curPathTime * IN_MILLISECONDS);
        }
        else
        {
            curPathTime -= KeyFrames[i].TimeTo;
            KeyFrames[i].ArriveTime = uint32(curPathTime * IN_MILLISECONDS);
            KeyFrames[i - 1].NextArriveTime = KeyFrames[i].ArriveTime;
            KeyFrames[i].DepartureTime = KeyFrames[i].ArriveTime;
        }
    }

    KeyFrames.back().NextArriveTime = KeyFrames.back().DepartureTime;

    m_pathTime = KeyFrames.back().DepartureTime;
    return true;
}

Transport::WayPointMap::const_iterator Transport::GetNextWayPoint()
{
    WayPointMap::const_iterator iter = m_curr;
    ++iter;
    if (iter == m_WayPoints.end())
        iter = m_WayPoints.begin();
    return iter;
}

void Transport::EnableMovement(bool enabled)
{
    if (!GetGOInfo()->moTransport.canBeStopped)
        return;

    _pendingStop = !enabled;
}

void Transport::MoveToNextWaypoint()
{
    // Clear events flagging
    _triggeredArrivalEvent = false;
    _triggeredDepartureEvent = false;

    // Set frames
    _currentFrame = _nextFrame++;
    if (_nextFrame == KeyFrames.end())
        _nextFrame = KeyFrames.begin();
}

bool Transport::TeleportTransport(uint32 newMapid, float x, float y, float z, float orientation)
{
    Map const* oldMap = GetMap();
    Relocate(x, y, z);
    UpdateModelPosition();
    if (oldMap->GetId() == newMapid)
        return false;

    Map* newMap = sMapMgr->CreateBaseMap(newMapid);
    std::list<Player *> _playersToRemoved;
    for (std::map<uint64, WorldObject *>::const_iterator itr = m_passengers.begin(); itr != m_passengers.end();)
    {
        WorldObject* passenger = ObjectAccessor::GetWorldObject(*this, itr->first);
        ++itr;

        if (!passenger)
            continue;

        switch (passenger->GetTypeId())
        {
            case TYPEID_PLAYER:
            {
                Player *player = passenger->ToPlayer();
                if (player->isDead() && !player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
                    player->ResurrectPlayer(1.0f);
                if (!player->TeleportTo(newMapid, x, y, z, orientation, TELE_TO_NOT_LEAVE_TRANSPORT))
                    _playersToRemoved.push_back(player);
                break;
            }
            case TYPEID_UNIT:
                if (!IS_PLAYER_GUID(passenger->ToUnit()->GetOwnerGUID()))  // pets should be teleported with player
                    passenger->ToCreature()->FarTeleportTo(newMap, x, y, z, passenger->GetOrientation());
                break;
            case TYPEID_DYNAMICOBJECT:
                passenger->AddObjectToRemoveList();
                break;
            case TYPEID_GAMEOBJECT:
            {
                GameObject* go = passenger->ToGameObject();
                go->GetMap()->RemoveFromMap(go, false);
                go->Relocate(x, y, z, orientation);
                go->SetMap(newMap);
                newMap->AddToMap(go);
                break;
            }
            default:
                break;
        }
    }

    for (std::list<Player *>::iterator itr = _playersToRemoved.begin(); itr != _playersToRemoved.end(); itr++)
        RemovePassenger(*itr);

    //we need to create and save new Map object with 'newMapid' because if not done -> lead to invalid Map object reference...
    //player far teleport would try to create same instance, but we need it NOW for transport...

    RemoveFromWorld();
    ResetMap();

    SetMap(newMap);
    ASSERT(GetMap());
    AddToWorld();

    if (oldMap != newMap)
    {
        UpdateForMap(oldMap);
        UpdateForMap(newMap);
    }

    for (CreatureSet::iterator itr = m_NPCPassengerSet.begin(); itr != m_NPCPassengerSet.end(); ++itr)
        (*itr)->FarTeleportTo(newMap, x, y, z, (*itr)->GetOrientation());

    for (GameObjectSet::iterator itr = m_GOPassengerSet.begin(); itr != m_GOPassengerSet.end(); ++itr)
    {
        GameObject* go = *itr;
        go->GetMap()->RemoveFromMap(go, false);
        go->Relocate(x, y, z, orientation);
        go->SetMap(newMap);
        newMap->AddToMap(go);
    }
    return true;
}

bool Transport::AddPassenger(WorldObject* passenger)
{
    if (!passenger)
        return false;

    if (passenger->GetTypeId() == TYPEID_PLAYER && !IsInWorld())
        return false;

    TC_LOG_INFO("entities.transport", "Player %s boarded transport %s.", passenger->GetName().c_str(), GetName().c_str());
    {
        TRINITY_GUARD(ACE_Thread_Mutex, Lock);

        switch (passenger->GetTypeId())
        {
            case TYPEID_PLAYER:
                if (passenger->ToUnit()->GetVehicleKit())
                    passenger->ToUnit()->GetVehicleKit()->RemoveAllPassengers();
                m_passengers[passenger->GetGUID()] = passenger;
                sScriptMgr->OnAddPassenger(this, passenger->ToPlayer());
                break;
            case TYPEID_UNIT:
                if (passenger->ToUnit()->GetVehicleKit())
                    passenger->ToUnit()->GetVehicleKit()->RemoveAllPassengers();
                m_passengers[passenger->GetGUID()] = passenger;
                break;
            case TYPEID_DYNAMICOBJECT:
            case TYPEID_GAMEOBJECT:
                m_passengers[passenger->GetGUID()] = passenger;
                break;
        }
    }

    return true;
}

bool Transport::RemovePassenger(WorldObject* passenger)
{
    bool success = false;
    if (!passenger)
        return success;

    {
        TRINITY_GUARD(ACE_Thread_Mutex, Lock);

        switch (passenger->GetTypeId())
        {
            case TYPEID_PLAYER:
                m_passengers.erase(passenger->GetGUID());
                passenger->SetTransport(NULL);
                sScriptMgr->OnRemovePassenger(this, passenger->ToPlayer());
                success = true;
                break;
            case TYPEID_GAMEOBJECT:
            case TYPEID_DYNAMICOBJECT:
            case TYPEID_UNIT:
                passenger->SetTransport(NULL);
                passenger->m_movementInfo.t_guid = 0;
                passenger->m_movementInfo.t_pos.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
                passenger->m_movementInfo.t_seat = -1;
                passenger->m_movementInfo.t_time = 0;
                passenger->m_movementInfo.t_time2 = 0;
                passenger->m_movementInfo.t_time3 = 0;
                m_passengers.erase(passenger->GetGUID());
                success = true;
                break;
            default:
                success = false;
                break;
        }
    }

    TC_LOG_INFO("entities.transport", "Player %s removed from transport %s.", passenger->GetName().c_str(), GetName().c_str());
    return success;
}

void Transport::CleanupsBeforeDelete(bool finalCleanup /*= true*/)
{
    while (!m_passengers.empty())
    {
        WorldObject* obj = m_passengers.begin()->second;
        if (!RemovePassenger(obj))
            break;
    }

    GameObject::CleanupsBeforeDelete(finalCleanup);
}

void Transport::Update(uint32 p_diff)
{
    uint32 const positionUpdateDelay = 200;

    if (AI())
        AI()->UpdateAI(p_diff);
    else if (!AIM_Initialize())
        TC_LOG_ERROR("entities.transport", "Could not initialize GameObjectAI for Transport");

    if (KeyFrames.size() <= 1)
        return;

    if (IsMoving() || !_pendingStop)
        m_goValue.Transport.PathProgress += p_diff;

    uint32 timer = m_goValue.Transport.PathProgress % GetPeriod();

    for (;;)
    {
        if (timer >= _currentFrame->ArriveTime)
        {
            if (!_triggeredArrivalEvent)
            {
                DoEventIfAny(*_currentFrame, false);
                _triggeredArrivalEvent = true;
            }
            if (timer < _currentFrame->DepartureTime)
            {
                SetMoving(false);
                if (_pendingStop && GetGoState() != GO_STATE_READY)
                {
                    SetGoState(GO_STATE_READY);
                    m_goValue.Transport.PathProgress = (m_goValue.Transport.PathProgress / GetPeriod());
                    m_goValue.Transport.PathProgress *= GetPeriod();
                    m_goValue.Transport.PathProgress += _currentFrame->ArriveTime;
                }
                break;  // its a stop frame and we are waiting
            }
        }

        if (timer >= _currentFrame->DepartureTime && !_triggeredDepartureEvent)
        {
            DoEventIfAny(*_currentFrame, true); // departure event
            _triggeredDepartureEvent = true;
        }

        // not waiting anymore
        SetMoving(true);

        // Enable movement
        if (GetGOInfo()->moTransport.canBeStopped)
            SetGoState(GO_STATE_ACTIVE);

        if (timer >= _currentFrame->DepartureTime && timer < _currentFrame->NextArriveTime)
            break;  // found current waypoint

        MoveToNextWaypoint();

        sScriptMgr->OnRelocate(this, _currentFrame->Node->index, _currentFrame->Node->mapid, _currentFrame->Node->x, _currentFrame->Node->y, _currentFrame->Node->z);

        // Departure event
        if (_currentFrame->IsTeleportFrame())
            if (TeleportTransport(_nextFrame->Node->mapid, _nextFrame->Node->x, _nextFrame->Node->y, _nextFrame->Node->z, _nextFrame->InitialOrientation))
                return; // Update more in new map thread
    }

    // Set position
    _positionChangeTimer.Update(p_diff);
    if (_positionChangeTimer.Passed())
    {
        _positionChangeTimer.Reset(positionUpdateDelay);
        if (IsMoving())
        {
            float t = CalculateSegmentPos(float(timer) * 0.001f);
            G3D::Vector3 pos, dir;
            _currentFrame->Spline->evaluate_percent(_currentFrame->Index, t, pos);
            _currentFrame->Spline->evaluate_derivative(_currentFrame->Index, t, dir);
            UpdatePosition(pos.x, pos.y, pos.z, atan2(dir.y, dir.x) + M_PI);
        }
        else
        {
            /* There are four possible scenarios that trigger loading/unloading passengers:
              1. transport moves from inactive to active grid
              2. the grid that transport is currently in becomes active
              3. transport moves from active to inactive grid
              4. the grid that transport is currently in unloads
            */
            /*            bool gridActive = GetMap()->IsGridLoaded(GetPositionX(), GetPositionY());

            if (_staticPassengers.empty() && gridActive) // 2.
                LoadStaticPassengers();
            else if (!_staticPassengers.empty() && !gridActive)
                // 4. - if transports stopped on grid edge, some passengers can remain in active grids
                //      unload all static passengers otherwise passengers won't load correctly when the grid that transport is currently in becomes active
                UnloadStaticPassengers();*/
        }
    }

    sScriptMgr->OnTransportUpdate(this, p_diff);
}

float Transport::CalculateSegmentPos(float now)
{
    KeyFrame const& frame = *_currentFrame;
    const float speed = float(m_goInfo->moTransport.moveSpeed);
    const float accel = float(m_goInfo->moTransport.accelRate);
    float timeSinceStop = frame.TimeFrom + (now - (1.0f/IN_MILLISECONDS) * frame.DepartureTime);
    float timeUntilStop = frame.TimeTo - (now - (1.0f/IN_MILLISECONDS) * frame.DepartureTime);
    float segmentPos, dist;
    float accelTime = this->accelTime;
    float accelDist = this->accelDist;
    // calculate from nearest stop, less confusing calculation...
    if (timeSinceStop < timeUntilStop)
    {
        if (timeSinceStop < accelTime)
            dist = 0.5f * accel * timeSinceStop * timeSinceStop;
        else
            dist = accelDist + (timeSinceStop - accelTime) * speed;
        segmentPos = dist - frame.DistSinceStop;
    }
    else
    {
        if (timeUntilStop < accelTime)
            dist = 0.5f * accel * timeUntilStop * timeUntilStop;
        else
            dist = accelDist + (timeUntilStop - accelTime) * speed;
        segmentPos = frame.DistUntilStop - dist;
    }

    return segmentPos / frame.NextDistFromPrev;
}

void Transport::UpdateForMap(Map const* targetMap)
{
    Map::PlayerList const& player = targetMap->GetPlayers();
    if (player.isEmpty())
        return;

    if (GetMapId() == targetMap->GetId())
    {
        for (Map::PlayerList::const_iterator itr = player.begin(); itr != player.end(); ++itr)
        {
            if (this != itr->getSource()->GetTransport())
            {
                UpdateData transData(GetMapId());
                BuildCreateUpdateBlockForPlayer(&transData, itr->getSource());
                WorldPacket packet;
                transData.BuildPacket(&packet);
                itr->getSource()->SendDirectMessage(&packet);
            }
        }
    }
    else
    {
        UpdateData transData(targetMap->GetId());
        BuildOutOfRangeUpdateBlock(&transData);
        WorldPacket out_packet;
        transData.BuildPacket(&out_packet);

        for (Map::PlayerList::const_iterator itr = player.begin(); itr != player.end(); ++itr)
            if (this != itr->getSource()->GetTransport())
                itr->getSource()->SendDirectMessage(&out_packet);
    }
}

void Transport::DoEventIfAny(KeyFrame const& node, bool departure)
{
    // 22663 22663 22861 22861 22664 22664 22860 22860 22665 22665 22663 22663 22861
    if (uint32 eventid = departure ? node.Node->departureEventID : node.Node->arrivalEventID)
    {
        TC_LOG_DEBUG("maps.script", "Taxi %s event %u of node %u of %s path", departure ? "departure" : "arrival", eventid, node.Node->index, GetName().c_str());
        GetMap()->ScriptsStart(sEventScripts, eventid, this, this);
        EventInform(eventid);
    }
}

void Transport::BuildStartMovePacket(Map const* targetMap)
{
    SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
    SetGoState(GO_STATE_ACTIVE);
    EnableMovement(true);
    UpdateForMap(targetMap);
}

void Transport::BuildWaitMovePacket(Map const* targetMap)
{
    SetGoState(GO_STATE_READY);
    UpdateForMap(targetMap);
}

void Transport::BuildStopMovePacket(Map const* targetMap)
{
    RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
    SetGoState(GO_STATE_READY);
    EnableMovement(false);
    UpdateForMap(targetMap);
}

uint32 Transport::AddNPCPassenger(uint32 tguid, uint32 entry, float x, float y, float z, float o, uint32 anim)
{
    Map* map = GetMap();
    //make it world object so it will not be unloaded with grid
    Creature* creature = new Creature(true);

    if (!creature->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_UNIT), map, GetPhaseMask(), entry, 0, GetGOInfo()->faction, 0, 0, 0, 0))
    {
        delete creature;
        return 0;
    }

    creature->SetTransport(this);
    creature->m_movementInfo.t_guid = GetGUID();
    creature->m_movementInfo.t_pos.Relocate(x, y, z, o);

    if (anim)
        creature->SetUInt32Value(UNIT_NPC_EMOTESTATE, anim);

    creature->Relocate(
        GetPositionX() + (x * std::cos(GetOrientation()) + y * std::sin(GetOrientation() + float(M_PI))),
        GetPositionY() + (y * std::cos(GetOrientation()) + x * std::sin(GetOrientation())),
        z + GetPositionZ(),
        o + GetOrientation());

    creature->SetHomePosition(creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation());
    creature->SetTransportHomePosition(creature->m_movementInfo.t_pos);

    if (!creature->IsPositionValid())
    {
        TC_LOG_ERROR("entities.transport", "Creature (guidlow %d, entry %d) not created. Suggested coordinates isn't valid (X: %f Y: %f)", creature->GetGUIDLow(), creature->GetEntry(), creature->GetPositionX(), creature->GetPositionY());
        delete creature;
        return 0;
    }

    map->AddToMap(creature);
    m_NPCPassengerSet.insert(creature);

    if (tguid == 0)
    {
        ++currenttguid;
        tguid = currenttguid;
    }
    else
        currenttguid = std::max(tguid, currenttguid);

    creature->SetGUIDTransport(tguid);
    sScriptMgr->OnAddCreaturePassenger(this, creature);
    return tguid;
}

uint32 Transport::AddGOPassenger(uint32 tguid, uint32 entry, float x, float y, float z, float o)
{
    Map* map = GetMap();
    //make it world object so it will not be unloaded with grid
    GameObject* go = new GameObject();

    if (!go->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_GAMEOBJECT), entry, map, GetPhaseMask(), x, y, z, o, 0, 0, 0, 0, 100, GO_STATE_READY))
    {
        delete go;
        return 0;
    }

    go->SetTransport(this);
    go->m_movementInfo.t_guid = GetGUID();
    go->m_movementInfo.t_pos.Relocate(x, y, z, o);
    CalculatePassengerPosition(x, y, z, o);
    go->Relocate(x, y, z, o);

    if (!go->IsPositionValid())
    {
        TC_LOG_ERROR("entities.transport", "GameObject (guidlow %d, entry %d) not created. Suggested coordinates isn't valid (X: %f Y: %f)", go->GetGUIDLow(),
                       go->GetEntry(), go->GetPositionX(), go->GetPositionY());
        delete go;
        return 0;
    }

    if (!map->AddToMap(go))
    {
        delete go;
        return 0;
    }

    m_GOPassengerSet.insert(go);
    return tguid;
}

void Transport::UpdatePosition(float x, float y, float z, float o)
{
    if (!MapManager::IsValidMapCoord(GetMapId(), x, y, z))
        return;
    Relocate(x, y, z, o);
    UpdateModelPosition();
    UpdatePassengersPositions();
}

void Transport::UpdatePassengersPositions()
{
    float x, y, z, o;
    for (CreatureSet::iterator itr = m_NPCPassengerSet.begin(); itr != m_NPCPassengerSet.end(); ++itr)
    {
        Creature* npc = *itr;
        npc->m_movementInfo.t_pos.GetPosition(x, y, z, o);
        CalculatePassengerPosition(x, y, z, o);
        GetMap()->CreatureRelocation(npc, x, y, z, o, false);
        npc->GetTransportHomePosition(x, y, z, o);
        CalculatePassengerPosition(x, y, z, o);
        npc->SetHomePosition(x, y, z, o);
    }

    for (GameObjectSet::iterator itr = m_GOPassengerSet.begin(); itr != m_GOPassengerSet.end(); ++itr)
    {
        GameObject* go = *itr;
        go->m_movementInfo.t_pos.GetPosition(x, y, z, o);
        CalculatePassengerPosition(x, y, z, o);
        GetMap()->GameObjectRelocation(go, x, y, z, o, false);
    }

    {
        TRINITY_GUARD(ACE_Thread_Mutex, Lock);
        std::list<uint64 > cleanUpPassengers;

        for (std::map<uint64, WorldObject *>::iterator itr = m_passengers.begin(); itr != m_passengers.end(); ++itr)
        {
            WorldObject *passenger = ObjectAccessor::GetWorldObject(*this, itr->first);

            if (!passenger || passenger->GetMap() != GetMap())
            {
                cleanUpPassengers.push_back(itr->first);
                continue;
            }

            passenger->m_movementInfo.t_pos.GetPosition(x, y, z, o);
            CalculatePassengerPosition(x, y, z, o);

            switch (passenger->GetTypeId())
            {
                case TYPEID_PLAYER:
                    if (!passenger->ToPlayer()->IsInWorld())
                        cleanUpPassengers.push_back(itr->first);
                    else if (passenger->ToPlayer()->IsInWorld() && passenger->ToUnit()->GetVehicle())
                        cleanUpPassengers.push_back(itr->first);
                    else
                        GetMap()->PlayerRelocation(passenger->ToPlayer(), x, y, z, o);
                    break;
                case TYPEID_UNIT:
                    if (!passenger->ToUnit()->GetVehicle())
                    {
                        Creature* npc = passenger->ToCreature();
                        GetMap()->CreatureRelocation(npc, x, y, z, o, false);
                        npc->GetTransportHomePosition(x, y, z, o);
                        CalculatePassengerPosition(x, y, z, o);
                        npc->SetHomePosition(x, y, z, o);
                    }
                    else
                        cleanUpPassengers.push_back(itr->first);
                    break;
                case TYPEID_DYNAMICOBJECT:
                    GetMap()->DynamicObjectRelocation(passenger->ToDynObject(), x, y, z, o);
                    break;
                case TYPEID_GAMEOBJECT:
                    GetMap()->GameObjectRelocation(passenger->ToGameObject(), x, y, z, o, false);
                    break;
            }
        }

        for (std::list<uint64 >::const_iterator itr = cleanUpPassengers.begin(); itr != cleanUpPassengers.end(); itr++)
        {
            if (WorldObject *passenger = ObjectAccessor::GetWorldObject(*this, *itr))
            {
                passenger->SetTransport(NULL);
                passenger->m_movementInfo.t_guid = 0;
                passenger->m_movementInfo.t_pos.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
                passenger->m_movementInfo.t_seat = -1;
                passenger->m_movementInfo.t_time = 0;
                passenger->m_movementInfo.t_time2 = 0;
                passenger->m_movementInfo.t_time3 = 0;
            }
            m_passengers.erase(*itr);
        }
    }
}

void Transport::CalculatePassengerPosition(float& x, float& y, float& z, float& o)
{
    float inx = x, iny = y, inz = z, ino = o;
    o = Position::NormalizeOrientation(GetOrientation() + ino);
    x = GetPositionX() + inx * std::cos(GetOrientation()) - iny * std::sin(GetOrientation());
    y = GetPositionY() + iny * std::cos(GetOrientation()) + inx * std::sin(GetOrientation());
    z = GetPositionZ() + inz; // this is completly wrong it have to take in consideration the getanimrotation and passeger from gameobject position dist
}

void Transport::CalculatePassengerOffset(float& x, float& y, float& z, float& o)
{
    o -= GetOrientation();
    z -= GetPositionZ();
    y -= GetPositionY();    // y = searchedY * std::cos(o) + searchedX * std::sin(o)
    x -= GetPositionX();    // x = searchedX * std::cos(o) + searchedY * std::sin(o + pi)
    float inx = x, iny = y;
    y = (iny - inx * tan(GetOrientation())) / (cos(GetOrientation()) + std::sin(GetOrientation()) * tan(GetOrientation()));
    x = (inx + iny * tan(GetOrientation())) / (cos(GetOrientation()) + std::sin(GetOrientation()) * tan(GetOrientation()));
}

// instance part.

Transport* MapManager::LoadTransportInMap(Map* instance, uint32 goEntry, uint32 period)
{
    const GameObjectTemplate* goInfo = sObjectMgr->GetGameObjectTemplate(goEntry);

    if (!goInfo)
    {
        TC_LOG_ERROR("sql.sql", "Transport ID: %u will not be loaded, gameobject_template missing", goEntry);
        return NULL;
    }

    if (goInfo->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
    {
        TC_LOG_ERROR("sql.sql", "Transport ID: %u will not be loaded, gameobject_template as wrong type", goEntry);
        return NULL;
    }

    Transport* t = new Transport(period, goInfo->ScriptId);

    std::set<uint32> mapsUsed;
    if (!t->GenerateWaypoints(goInfo, mapsUsed))
    {
        delete t;
        return NULL;
    }

    TaxiPathNodeEntry const* startNode = t->KeyFrames.begin()->Node;
    uint32 mapId = startNode->mapid;
    float x = startNode->x;
    float y = startNode->y;
    float z = startNode->z;
    float o = t->KeyFrames.begin()->InitialOrientation;

    if (!t->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_MO_TRANSPORT), goEntry, mapId, x, y, z, o, 255, 0))
    {
        delete t;
        return NULL;
    }

    t->SetMap(instance);
    t->SetZoneScript();
    t->AddToWorld();

    m_Transports.insert(t);
    m_TransportsByInstanceIdMap[instance->GetInstanceId()].insert(t);

    return t;
}

void MapManager::UnLoadTransportFromMap(Transport* t)
{
    Map* map = t->GetMap();

    for (Transport::CreatureSet::iterator itr = t->m_NPCPassengerSet.begin(); itr != t->m_NPCPassengerSet.end();)
    {
        if (Creature* npc = *itr)
        {
            npc->SetTransport(NULL);
            npc->setActive(false);
            npc->RemoveFromWorld();
        }
        ++itr;
    }

    for (Transport::GameObjectSet::iterator itr = t->m_GOPassengerSet.begin(); itr != t->m_GOPassengerSet.end();)
    {
        if (GameObject* go = *itr)
        {
            go->SetTransport(NULL);
            go->setActive(false);
            go->RemoveFromWorld();
        }
        ++itr;
    }

    UpdateData transData(t->GetMapId());
    t->BuildOutOfRangeUpdateBlock(&transData);
    WorldPacket out_packet;
    transData.BuildPacket(&out_packet);

    for (Map::PlayerList::const_iterator itr = map->GetPlayers().begin(); itr != map->GetPlayers().end(); ++itr)
        if (t != itr->getSource()->GetTransport())
            itr->getSource()->SendDirectMessage(&out_packet);

    t->m_NPCPassengerSet.clear();
    t->m_GOPassengerSet.clear();
    m_TransportsByInstanceIdMap[t->GetInstanceId()].erase(t);
    m_Transports.erase(t);
    t->m_WayPoints.clear();
    t->RemoveFromWorld();
}

void MapManager::LoadTransportForPlayers(Player* player)
{
    MapManager::TransportMap& tmap = sMapMgr->m_TransportsByInstanceIdMap;

    UpdateData transData(player->GetMapId());

    MapManager::TransportSet& tset = tmap[player->GetInstanceId()];

    for (MapManager::TransportSet::const_iterator i = tset.begin(); i != tset.end(); ++i)
        (*i)->BuildCreateUpdateBlockForPlayer(&transData, player);

    WorldPacket packet;
    transData.BuildPacket(&packet);
    player->SendDirectMessage(&packet);
}

void MapManager::UnLoadTransportForPlayers(Player* player)
{
    MapManager::TransportMap& tmap = sMapMgr->m_TransportsByInstanceIdMap;

    UpdateData transData(player->GetMapId());

    MapManager::TransportSet& tset = tmap[player->GetInstanceId()];

    for (MapManager::TransportSet::const_iterator i = tset.begin(); i != tset.end(); ++i)
    {
        for (Transport::CreatureSet::iterator itr = (*i)->m_NPCPassengerSet.begin(); itr != (*i)->m_NPCPassengerSet.end();)
        {
            if (Creature* npc = *itr)
            {
                npc->SetTransport(NULL);
                npc->setActive(false);
                npc->RemoveFromWorld();
            }
            ++itr;
        }

        for (Transport::GameObjectSet::iterator itr = (*i)->m_GOPassengerSet.begin(); itr != (*i)->m_GOPassengerSet.end();)
        {
            if (GameObject* go = *itr)
            {
                go->SetTransport(NULL);
                go->setActive(false);
                go->RemoveFromWorld();
            }
            ++itr;
        }

        (*i)->BuildOutOfRangeUpdateBlock(&transData);
    }

    WorldPacket packet;
    transData.BuildPacket(&packet);
    player->SendDirectMessage(&packet);
}


Creature* Transport::AddNPCPassengerInInstance(uint32 entry, float x, float y, float z, float o, uint32 anim)
{
    Map* map = GetMap();
    Creature* creature = new Creature;

    if (!creature->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_UNIT), map, GetPhaseMask(), entry, 0, GetGOInfo()->faction, 0, 0, 0, 0))
    {
        delete creature;
        return 0;
    }

    creature->SetTransport(this);
    creature->m_movementInfo.guid = GetGUID();
    creature->m_movementInfo.t_pos.Relocate(x, y, z, o);

    creature->Relocate(
                       GetPositionX() + (x * cos(GetOrientation()) + y * sin(GetOrientation() + float(M_PI))),
                       GetPositionY() + (y * cos(GetOrientation()) + x * sin(GetOrientation())),
                       z + GetPositionZ(),
                       o + GetOrientation());


    creature->SetHomePosition(creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation());
    creature->SetTransportHomePosition(creature->m_movementInfo.t_pos);

    if (!creature->IsPositionValid())
    {
        TC_LOG_ERROR("misc", "Creature (guidlow %d, entry %d) not created. Suggested coordinates isn't valid (X: %f Y: %f)", creature->GetGUIDLow(), creature->GetEntry(), creature->GetPositionX(), creature->GetPositionY());
        delete creature;
        return 0;
    }

    map->AddToMap(creature);
    m_NPCPassengerSet.insert(creature);

    creature->setActive(true);
    sScriptMgr->OnAddCreaturePassenger(this, creature);
    return creature;
}

void Transport::BuildPassengersBlockForPlayer(Player *player, UpdateData *data)
{
    //    TRINITY_GUARD(ACE_Thread_Mutex, Lock);
    for (std::map<uint64, WorldObject *>::const_iterator itr = m_passengers.begin(); itr != m_passengers.end(); ++itr)
        if (WorldObject *passenger = ObjectAccessor::GetWorldObject(*this, itr->first))
            if (player != passenger && player->HaveAtClient(passenger))
                passenger->BuildCreateUpdateBlockForPlayer(data, player);
}

void Transport::UpdateVisibilityOf(std::set<Unit*>& i_visibleNow, Player::ClientGUIDs &vis_guids, UpdateData &i_data, Player &i_player)
{
    for (std::map<uint64, WorldObject *>::const_iterator itr = m_passengers.begin(); itr != m_passengers.end(); ++itr)
    {
        if (WorldObject *passenger = ObjectAccessor::GetWorldObject(*this, itr->first))
            if (vis_guids.find(itr->first) != vis_guids.end())
            {
                vis_guids.erase(itr->first);

                switch (passenger->GetTypeId())
                {
                    case TYPEID_GAMEOBJECT:
                        i_player.UpdateVisibilityOf(passenger->ToGameObject(), i_data, i_visibleNow);
                        break;
                    case TYPEID_PLAYER:
                        i_player.UpdateVisibilityOf(passenger->ToPlayer(), i_data, i_visibleNow);
                        if (!passenger->isNeedNotify(NOTIFY_VISIBILITY_CHANGED))
                            passenger->ToPlayer()->UpdateVisibilityOf(&i_player);
                        break;
                    case TYPEID_UNIT:
                        i_player.UpdateVisibilityOf(passenger->ToCreature(), i_data, i_visibleNow);
                        break;
                    case TYPEID_DYNAMICOBJECT:
                        i_player.UpdateVisibilityOf(passenger->ToDynObject(), i_data, i_visibleNow);
                        break;
                    default:
                        break;
                }
            }
    }
}

TempSummon* Transport::SummonPassenger(uint32 entry, Position const& pos, TempSummonType summonType, SummonPropertiesEntry const* properties /*= NULL*/, uint32 duration /*= 0*/, Unit* summoner /*= NULL*/, uint32 spellId /*= 0*/, uint32 vehId /*= 0*/)
{
    Map* map = FindMap();
    if (!map)
        return NULL;

    uint32 mask = UNIT_MASK_SUMMON;
    if (properties)
    {
        switch (properties->Category)
        {
            case SUMMON_CATEGORY_PET:
                mask = UNIT_MASK_GUARDIAN;
                break;
            case SUMMON_CATEGORY_PUPPET:
                mask = UNIT_MASK_PUPPET;
                break;
            case SUMMON_CATEGORY_VEHICLE:
                mask = UNIT_MASK_MINION;
                break;
            case SUMMON_CATEGORY_WILD:
            case SUMMON_CATEGORY_ALLY:
            case SUMMON_CATEGORY_UNK:
            {
                switch (properties->Type)
                {
                    case SUMMON_TYPE_MINION:
                    case SUMMON_TYPE_GUARDIAN:
                    case SUMMON_TYPE_GUARDIAN2:
                        mask = UNIT_MASK_GUARDIAN;
                        break;
                    case SUMMON_TYPE_TOTEM:
                    case SUMMON_TYPE_LIGHTWELL:
                        mask = UNIT_MASK_TOTEM;
                        break;
                    case SUMMON_TYPE_VEHICLE:
                    case SUMMON_TYPE_VEHICLE2:
                        mask = UNIT_MASK_SUMMON;
                        break;
                    case SUMMON_TYPE_MINIPET:
                        mask = UNIT_MASK_MINION;
                        break;
                    default:
                        if (properties->Flags & 512) // Mirror Image, Summon Gargoyle
                            mask = UNIT_MASK_GUARDIAN;
                        break;
                }
                break;
            }
            default:
                return NULL;
        }
    }

    uint32 phase = PHASEMASK_NORMAL;
    uint32 team = 0;
    if (summoner)
    {
        phase = summoner->GetPhaseMask();
        if (summoner->GetTypeId() == TYPEID_PLAYER)
            team = summoner->ToPlayer()->GetTeam();
    }

    TempSummon* summon = NULL;
    switch (mask)
    {
        case UNIT_MASK_SUMMON:
            summon = new TempSummon(properties, summoner, false);
            break;
        case UNIT_MASK_GUARDIAN:
            summon = new Guardian(properties, summoner, false);
            break;
        case UNIT_MASK_PUPPET:
            summon = new Puppet(properties, summoner);
            break;
        case UNIT_MASK_TOTEM:
            summon = new Totem(properties, summoner);
            break;
        case UNIT_MASK_MINION:
            summon = new Minion(properties, summoner, false);
            break;
    }

    float x, y, z, o;
    pos.GetPosition(x, y, z, o);
    CalculatePassengerPosition(x, y, z, o);
    if (!summon->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_UNIT), map, phase, entry, vehId, team, x, y, z, o))
    {
        delete summon;
        return NULL;
    }

    summon->SetUInt32Value(UNIT_CREATED_BY_SPELL, spellId);

    summon->SetTransport(this);
    summon->m_movementInfo.t_guid = GetGUID();
    summon->m_movementInfo.t_pos.Relocate(pos);
    summon->Relocate(x, y, z, o);
    summon->SetHomePosition(x, y, z, o);
    summon->SetTransportHomePosition(pos);

    summon->InitStats(duration);

    if (!map->AddToMap<Creature>(summon))
    {
        delete summon;
        return NULL;
    }

    m_NPCPassengerSet.insert(summon);

    summon->InitSummon();
    summon->SetTempSummonType(summonType);

    return summon;
 }

void Transport::LoadStaticPassengers()
{
    if (uint32 mapId = GetGOInfo()->moTransport.mapID)
    {
        CellObjectGuidsMap const& cells = sObjectMgr->GetMapObjectGuids(mapId, GetMap()->GetSpawnMode());
        CellGuidSet::const_iterator guidEnd;
        for (CellObjectGuidsMap::const_iterator cellItr = cells.begin(); cellItr != cells.end(); ++cellItr)
        {
            // Creatures on transport
            guidEnd = cellItr->second.creatures.end();
            for (CellGuidSet::const_iterator guidItr = cellItr->second.creatures.begin(); guidItr != guidEnd; ++guidItr)
                CreateNPCPassenger(*guidItr, sObjectMgr->GetCreatureData(*guidItr));

            // GameObjects on transport
            guidEnd = cellItr->second.gameobjects.end();
            for (CellGuidSet::const_iterator guidItr = cellItr->second.gameobjects.begin(); guidItr != guidEnd; ++guidItr)
                CreateGOPassenger(*guidItr, sObjectMgr->GetGOData(*guidItr));
        }
    }
}

Creature* Transport::CreateNPCPassenger(uint32 guid, CreatureData const* data)
{
    Map* map = GetMap();
    Creature* creature = new Creature();

    if (!creature->LoadCreatureFromDB(guid, map, false))
    {
        delete creature;
        return NULL;
    }

    float x = data->posX;
    float y = data->posY;
    float z = data->posZ;
    float o = data->orientation;

    creature->SetTransport(this);
    creature->m_movementInfo.t_guid = GetGUID();
    creature->m_movementInfo.t_pos.Relocate(x, y, z, o);
    CalculatePassengerPosition(x, y, z, o);
    creature->Relocate(x, y, z, o);
    creature->SetHomePosition(creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation());
    creature->SetTransportHomePosition(creature->m_movementInfo.t_pos);

    if (!creature->IsPositionValid())
    {
        TC_LOG_ERROR("entities.transport", "Creature (guidlow %d, entry %d) not created. Suggested coordinates aren't valid (X: %f Y: %f)",creature->GetGUIDLow(),creature->GetEntry(),creature->GetPositionX(),creature->GetPositionY());
        delete creature;
        return NULL;
    }

    if (data->phaseMask)
        creature->SetPhaseMask(data->phaseMask, true);

    if (!map->AddToMap(creature))
    {
        delete creature;
        return NULL;
    }

    m_NPCPassengerSet.insert(creature);
    sScriptMgr->OnAddCreaturePassenger(this, creature);
    return creature;
}

GameObject* Transport::CreateGOPassenger(uint32 guid, GameObjectData const* data)
{
    Map* map = GetMap();
    GameObject* go = new GameObject();

    if (!go->LoadGameObjectFromDB(guid, map, false))
    {
        delete go;
        return NULL;
    }

    float x = data->posX;
    float y = data->posY;
    float z = data->posZ;
    float o = data->orientation;

    go->SetTransport(this);
    go->m_movementInfo.t_guid = GetGUID();
    go->m_movementInfo.t_pos.Relocate(x, y, z, o);
    CalculatePassengerPosition(x, y, z, o);
    go->Relocate(x, y, z, o);

    if (!go->IsPositionValid())
    {
        TC_LOG_ERROR("entities.transport", "GameObject (guidlow %d, entry %d) not created. Suggested coordinates aren't valid (X: %f Y: %f)", go->GetGUIDLow(), go->GetEntry(), go->GetPositionX(), go->GetPositionY());
        delete go;
        return NULL;
    }

    if (!map->AddToMap(go))
    {
        delete go;
        return NULL;
    }

    m_GOPassengerSet.insert(go);
    return go;
}
