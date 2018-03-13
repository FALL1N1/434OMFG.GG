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

#ifndef TRANSPORTS_H
#define TRANSPORTS_H

#include "GameObject.h"
#include "VehicleDefines.h"
#include "MoveSpline.h"
#include "Player.h"

#include <map>
#include <set>
#include <string>

typedef Movement::Spline<double>                 TransportSpline;

class SplineRawInitializer
{
 public:
 SplineRawInitializer(Movement::PointsArray& points) : _points(points) { }

    void operator()(uint8& mode, bool& cyclic, Movement::PointsArray& points, int& lo, int& hi) const
    {
        mode = Movement::SplineBase::ModeCatmullrom;
        cyclic = false;
        points.assign(_points.begin(), _points.end());
        lo = 1;
        hi = points.size() - 2;
    }

    Movement::PointsArray& _points;
};

struct KeyFrame
{
    explicit KeyFrame(TaxiPathNodeEntry const& _node) : Index(0), Node(&_node), InitialOrientation(0.0f),
        DistSinceStop(-1.0f), DistUntilStop(-1.0f), DistFromPrev(-1.0f), TimeFrom(0.0f), TimeTo(0.0f),
        Teleport(false), ArriveTime(0), DepartureTime(0), Spline(NULL), NextDistFromPrev(0.0f), NextArriveTime(0)
    {
    }

    TaxiPathNodeEntry const* Node;
    float DistSinceStop;
    float DistUntilStop;
    float DistFromPrev;
    float TimeFrom;
    float TimeTo;

    uint32 Index;
    float InitialOrientation;
    bool Teleport;
    uint32 ArriveTime;
    uint32 DepartureTime;
    TransportSpline* Spline;

    // Data needed for next frame
    float NextDistFromPrev;
    uint32 NextArriveTime;

    bool IsTeleportFrame() const { return Teleport; }
    bool IsStopFrame() const { return Node->actionFlag == 2; }
};

typedef std::vector<KeyFrame>                    KeyFrameVec;

class Transport : public GameObject, public TransportBase
{
    public:
        Transport(uint32 period, uint32 script);
        ~Transport();

        bool Create(uint32 guidlow, uint32 entry, uint32 mapid, float x, float y, float z, float ang, uint32 animprogress, uint32 dynflags);
        bool GenerateWaypoints(const GameObjectTemplate* goInfo, std::set<uint32> &mapids);
        void CleanupsBeforeDelete(bool finalCleanup = true) override;
        void Update(uint32 p_time);
        bool AsToBeDeleted() { return _asToBeDeleted; }
        void DeleteInNextUpdate(bool val) { _asToBeDeleted = val; }

        // non static passenger part
        bool AddPassenger(WorldObject* passenger);
        bool RemovePassenger(WorldObject* passenger);
        void RemovePassenger(Creature* passenger) { m_NPCPassengerSet.erase(passenger); }
        void RemovePassenger(GameObject* passenger) { m_GOPassengerSet.erase(passenger); }



        // static passengers part
        void LoadStaticPassengers();
        typedef std::set<Creature*> CreatureSet;
        CreatureSet m_NPCPassengerSet;
        typedef std::set<GameObject*> GameObjectSet;
        GameObjectSet m_GOPassengerSet;
        uint32 AddNPCPassenger(uint32 tguid, uint32 entry, float x, float y, float z, float o, uint32 anim = 0);
        uint32 AddGOPassenger(uint32 tguid, uint32 entry, float x, float y, float z, float o);
        Creature* CreateNPCPassenger(uint32 guid, CreatureData const* data);
        GameObject* CreateGOPassenger(uint32 guid, GameObjectData const* data);

        Creature* AddNPCPassengerInInstance(uint32 entry, float x, float y, float z, float o, uint32 anim = 0);
        void UpdatePosition(float x, float y, float z, float o);
        // void UpdatePosition(MovementInfo* mi);
        void UpdatePassengersPositions();

        /// This method transforms supplied transport offsets into global coordinates
        void CalculatePassengerPosition(float& x, float& y, float& z, float& o);

        /// This method transforms supplied global coordinates into local offsets
        void CalculatePassengerOffset(float& x, float& y, float& z, float& o);

        void BuildStartMovePacket(Map const* targetMap);
        void BuildWaitMovePacket(Map const* targetMap);
        void BuildStopMovePacket(Map const* targetMap);
        uint32 GetScriptId() const { return ScriptId; }
        void BuildPassengersBlockForPlayer(Player *player, UpdateData *data);
        void UpdateVisibilityOf(std::set<Unit*>& i_visibleNow, Player::ClientGUIDs &vis_guids, UpdateData &i_data, Player &i_player);

        TempSummon* SummonPassenger(uint32 entry, Position const& pos, TempSummonType summonType, SummonPropertiesEntry const* properties = NULL, uint32 duration = 0, Unit* summoner = NULL, uint32 spellId = 0, uint32 vehId = 0);
    private:
        struct WayPoint
        {
            WayPoint() : mapid(0), x(0), y(0), z(0), teleport(false), id(0) {}
            WayPoint(uint32 _mapid, float _x, float _y, float _z, bool _teleport, uint32 _id = 0,
                uint32 _arrivalEventID = 0, uint32 _departureEventID = 0)
                : mapid(_mapid), x(_x), y(_y), z(_z), teleport(_teleport), id(_id),
                arrivalEventID(_arrivalEventID), departureEventID(_departureEventID)
            {
            }
            uint32 mapid;
            float x;
            float y;
            float z;
            bool teleport;
            uint32 id;
            uint32 arrivalEventID;
            uint32 departureEventID;
        };

        typedef std::map<uint32, WayPoint> WayPointMap;

        WayPointMap::const_iterator m_curr;
        WayPointMap::const_iterator m_next;
        uint32 m_pathTime;

        std::map<uint64, WorldObject *> m_passengers;

        uint32 currenttguid;
        uint32 ScriptId;

        bool inInstance;

        float accelTime;
        float accelDist;

        KeyFrameVec::const_iterator _currentFrame;
        KeyFrameVec::const_iterator _nextFrame;
        TimeTrackerSmall _positionChangeTimer;

        bool _isMoving;
        bool _pendingStop;

        //! These are needed to properly control events triggering only once for each frame
        bool _triggeredArrivalEvent;
        bool _triggeredDepartureEvent;

        ACE_Thread_Mutex Lock;
        bool _asToBeDeleted;
    public:
        KeyFrameVec KeyFrames;

        WayPointMap m_WayPoints;
        uint32 m_nextNodeTime;
        void EnableMovement(bool enabled);

    private:
        bool TeleportTransport(uint32 newMapid, float x, float y, float z, float orientation);
        void UpdateForMap(Map const* map);
        void DoEventIfAny(KeyFrame const& node, bool departure);

        bool IsMoving() const { return _isMoving; }
        void SetMoving(bool val) { _isMoving = val; }

        WayPointMap::const_iterator GetNextWayPoint();
        void MoveToNextWaypoint();
        float CalculateSegmentPos(float now);

        uint32 GetPeriod() const { return GetUInt32Value(GAMEOBJECT_LEVEL); }
        void SetPeriod(uint32 period) { SetUInt32Value(GAMEOBJECT_LEVEL, period); }
        uint32 GetTimer() const { return GetGOValue()->Transport.PathProgress; }

};
#endif
