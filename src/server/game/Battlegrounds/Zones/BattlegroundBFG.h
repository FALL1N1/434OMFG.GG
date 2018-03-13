/*
* Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

#ifndef __BATTLEGROUNDBG_H
#define __BATTLEGROUNDBG_H

#include "Battleground.h"

enum BG_BFG_Rewards
{
    BG_BFG_MAP_COMPLETE,
    BG_BFG_REWARD_NUM
};

enum BG_BFG_WorldStates
{
    BG_BFG_OP_OCCUPIED_BASES_HORDE  = 1778,
    BG_BFG_OP_OCCUPIED_BASES_ALLY   = 1779,
    BG_BFG_OP_RESOURCES_ALLY        = 1776,
    BG_BFG_OP_RESOURCES_HORDE       = 1777,
    BG_BFG_OP_RESOURCES_MAX         = 1780,
    BG_BFG_OP_RESOURCES_WARNING     = 1955
};

const uint32 BG_BFG_OP_NODESTATES[3]    = { 1767, 1782, 1772 };

const uint32 BG_BFG_OP_NODEICONS[3]     = { 1842, 1846, 1845 };

enum BG_BFG_NodeObjectId
{
    BG_BFG_OBJECTID_NODE_BANNER_0     = 208779, // Lighthouse banner
    BG_BFG_OBJECTID_NODE_BANNER_1     = 208782, // Mine banner
    BG_BFG_OBJECTID_NODE_BANNER_2     = 208785, // Waterworks banner

    BG_BFG_RBG_OBJECTID_NODE_BANNER_0 = 208779, //406009, // Lighthouse banner
    BG_BFG_RBG_OBJECTID_NODE_BANNER_1 = 208782, //406010, // Mine banner
    BG_BFG_RBG_OBJECTID_NODE_BANNER_2 = 208785, //406011  // Waterworks banner
};

enum BG_BG_ObjectType
{
    BG_BFG_OBJECT_BANNER_NEUTRAL            = 0,
    BG_BFG_OBJECT_BANNER_CONT_A             = 1,
    BG_BFG_OBJECT_BANNER_CONT_H             = 2,
    BG_BFG_OBJECT_BANNER_ALLY               = 3,
    BG_BFG_OBJECT_BANNER_HORDE              = 4,
    BG_BFG_OBJECT_AURA_ALLY                 = 5,
    BG_BFG_OBJECT_AURA_HORDE                = 6,
    BG_BFG_OBJECT_AURA_CONTESTED            = 7,
    //gates
    BG_BFG_OBJECT_GATE_A                    = 25,
    BG_BFG_OBJECT_GATE_H                    = 26,
    //buffs
    BG_BFG_OBJECT_SPEEDBUFF_LIGHTHOUSE      = 27,
    BG_BFG_OBJECT_REGENBUFF_LIGHTHOUSE      = 28,
    BG_BFG_OBJECT_BERSERKBUFF_LIGHTHOUSE    = 29,
    BG_BFG_OBJECT_SPEEDBUFF_WATERWORKS      = 30,
    BG_BFG_OBJECT_REGENBUFF_WATERWORKS      = 31,
    BG_BFG_OBJECT_BERSERKBUFF_WATERWORKS    = 32,
    BG_BFG_OBJECT_SPEEDBUFF_MINE            = 33,
    BG_BFG_OBJECT_REGENBUFF_MINE            = 34,
    BG_BFG_OBJECT_BERSERKBUFF_MINE          = 35,
    BG_BFG_OBJECT_MAX                       = 36
};

/* Object id templates from DB */
enum BG_BFG_ObjectTypes
{
    BG_BFG_OBJECTID_BANNER_A            = 208673,
    BG_BFG_OBJECTID_BANNER_CONT_A       = 208763,
    BG_BFG_OBJECTID_BANNER_H            = 208748,
    BG_BFG_OBJECTID_BANNER_CONT_H       = 208733,

    BG_BFG_RBG_OBJECTID_BANNER_A        = 208673, //406012,
    BG_BFG_RBG_OBJECTID_BANNER_CONT_A   = 208763, //406013,
    BG_BFG_RBG_OBJECTID_BANNER_H        = 208748, //406014,
    BG_BFG_RBG_OBJECTID_BANNER_CONT_H   = 208733, //406015,

    BG_BFG_OBJECTID_AURA_A              = 180100,
    BG_BFG_OBJECTID_AURA_H              = 180101,
    BG_BFG_OBJECTID_AURA_C              = 180102,

    BG_BFG_OBJECTID_GATE_A              = 207177,
    BG_BFG_OBJECTID_GATE_H              = 207178
};

enum BG_BFG_Timers
{
    BG_BFG_FLAG_CAPTURING_TIME = 60000
};

enum BG_BFG_Score
{
    BG_BFG_WARNING_NEAR_VICTORY_SCORE = 1800,
    BG_BFG_MAX_TEAM_SCORE = 2000
};

/* Do not change below, or this thing will go boom! */
enum BG_BG_BattlegroundNodes
{
    BG_BFG_NODE_LIGHTHOUSE = 0,
    BG_BFG_NODE_WATERWORKS = 1,
    BG_BFG_NODE_MINE = 2,

    BG_BFG_DYNAMIC_NODES_COUNT = 3, // Dynamic nodes that can be captured

    BG_BFG_SPIRIT_ALIANCE = 3,
    BG_BFG_SPIRIT_HORDE = 4,

    BG_BFG_ALL_NODES_COUNT = 5 // All nodes (dynamic and static)
};

enum BG_BFG_NodeStatus
{
    BG_BFG_NODE_TYPE_NEUTRAL = 0,
    BG_BFG_NODE_TYPE_CONTESTED = 1,
    BG_BFG_NODE_STATUS_ALLY_CONTESTED = 1,
    BG_BFG_NODE_STATUS_HORDE_CONTESTED = 2,
    BG_BFG_NODE_TYPE_OCCUPIED = 3,
    BG_BFG_NODE_STATUS_ALLY_OCCUPIED = 3,
    BG_BFG_NODE_STATUS_HORDE_OCCUPIED = 4
};

enum BG_BFG_Sounds
{
    BG_BFG_SOUND_NODE_CLAIMED = 8192,
    BG_BFG_SOUND_NODE_CAPTURED_ALLIANCE = 8173,
    BG_BFG_SOUND_NODE_CAPTURED_HORDE = 8213,
    BG_BFG_SOUND_NODE_ASSAULTED_ALLIANCE = 8212,
    BG_BFG_SOUND_NODE_ASSAULTED_HORDE = 8174,
    BG_BFG_SOUND_NEAR_VICTORY = 8456
};

enum BG_BFG_Objectives
{
    BG_OBJECTIVE_ASSAULT_BASE = 122,
    BG_OBJECTIVE_DEFEND_BASE = 123,
    BFG_OBJECTIVE_ASSAULT_BASE = 370,
    BFG_OBJECTIVE_DEFEND_BASE = 371,
};

#define BG_BFG_NotBGBGWeekendHonorTicks 330
#define BG_BFG_BGBGWeekendHonorTicks 200

#define BG_EVENT_START_BATTLE 9158 // Achievement: Newbs to Plowshares

// x, y, z, o
const float BG_BFG_NodePositions[BG_BFG_DYNAMIC_NODES_COUNT][4] =
{
    {1057.856934f, 1278.269165f, 2.91f, 1.903034f}, // Lighthouse
    {980.0635f, 948.7772f, 12.868f, 5.9015f}, // Waterwork
    {1251.010f, 958.3939f, 5.680f, 2.7698f} // Mine
};

// x, y, z, o, rot0, rot1, rot2, rot3
const float BG_BFG_DoorPositions[2][8] =
{
    {918.876f, 1336.56f, 27.6195f, 2.77481f, 0.0f, 0.0f, 0.983231f, 0.182367f},
    {1396.15f, 977.014f, 7.43169f, 6.27043f, 0.0f, 0.0f, 0.006378f, -0.99998f}
};

// Tick intervals and given points: case 0,1,2,3 captured nodes
const uint32 BG_BFG_TickIntervals[4] = {0, 12000, 6000, 1000};
const uint32 BG_BFG_TickPoints[4] = {0, 10, 10, 30};

// WorldSafeLocs ids for 3 nodes, and for ally, and horde starting location
const uint32 BG_BFG_GraveyardIds[BG_BFG_ALL_NODES_COUNT] = {1736, 1738, 1735, 1740, 1739};

// x, y, z, o
const float BG_BFG_BuffPositions[BG_BFG_DYNAMIC_NODES_COUNT][4] =
{
    {1063.57f, 1313.42f, 4.91f, 4.14f}, // Lighthouse
    {961.830f, 977.03f, 14.15f, 4.55f}, // Waterworks
    {1193.09f, 1017.46f, 7.98f, 0.24f}, // Mine
};

// x, y, z, o
const float BG_BFG_SpiritGuidePos[BG_BFG_ALL_NODES_COUNT][4] =
{
    {1034.82f, 1335.58f, 12.0095f, 5.15f}, // Lighthouse
    {887.578f, 937.337f, 23.7737f, 0.45f}, // Waterworks
    {1252.23f, 836.547f, 27.7895f, 1.60f}, // Mine
    {908.274f, 1338.6f, 27.6449f, 5.95f}, // Alliance
    {1401.38f, 977.125f, 7.44215f, 3.04f}, // Horde
};

struct BG_BFG_BannerTimer
{
    uint32 timer;
    uint8 type;
    uint8 teamIndex;
};

struct BattlegroundBFGScore : public BattlegroundScore
{
    BattlegroundBFGScore(): BasesAssaulted(0), BasesDefended(0) { }
    ~BattlegroundBFGScore() { }
    uint32 BasesAssaulted;
    uint32 BasesDefended;
    uint32 GetAttr1() const final override { return BasesAssaulted; }
    uint32 GetAttr2() const final override { return BasesDefended; }
};

class BattlegroundBFG : public Battleground
{
public:
    BattlegroundBFG();
    ~BattlegroundBFG();

    void AddPlayer(Player* player);
    void StartingEventCloseDoors();
    void StartingEventOpenDoors();
    void RemovePlayer(Player* player, uint64 guid, uint32 team);
    void HandleAreaTrigger(Player* Source, uint32 Trigger);
    bool SetupBattleground();
    void Reset();
    void EndBattleground(uint32 winner);
    WorldSafeLocsEntry const* GetClosestGraveYard(Player* player);

    /* Score-keeping */
    void UpdatePlayerScore(Player* Source, uint32 type, uint32 value, bool doAddHonor = true);

    void FillInitialWorldStates(WorldPacket& data);

    /* Nodes occupying */
    void EventPlayerClickedOnFlag(Player* source, GameObject* target_obj);

    /* achievement req. */
    bool IsAllNodesConrolledByTeam(uint32 team) const;
    bool IsTeamScores500Disadvantage(uint32 team) const { return m_TeamScores500Disadvantage[GetTeamIndexByTeamId(team)]; }

    uint32 GetPrematureWinner();
private:
    void PostUpdateImpl(uint32 diff);
    /* GameObject spawning/removing */
    void _CreateBanner(uint8 node, uint8 type, uint8 teamIndex, bool delay);
    void _DelBanner(uint8 node, uint8 type, uint8 teamIndex);
    void _SendNodeUpdate(uint8 node);

    /* Creature spawning/removing */
    // TODO: need to get the peons spawns scripted
    void _NodeOccupied(uint8 node, Team team);
    void _NodeDeOccupied(uint8 node);

    int32 _GetNodeNameId(uint8 node);

    /* Nodes info:
    0: neutral
    1: ally contested
    2: horde contested
    3: ally occupied
    4: horde occupied */
    uint8 m_Nodes[BG_BFG_DYNAMIC_NODES_COUNT];
    uint8 m_prevNodes[BG_BFG_DYNAMIC_NODES_COUNT];
    BG_BFG_BannerTimer m_BannerTimers[BG_BFG_DYNAMIC_NODES_COUNT];
    uint32 m_NodeTimers[BG_BFG_DYNAMIC_NODES_COUNT];
    uint32 m_lastTick[BG_TEAMS_COUNT];
    uint32 m_HonorScoreTics[BG_TEAMS_COUNT];
    bool m_IsInformedNearVictory;
    uint32 m_HonorTics;
    // need for achievements
    bool m_TeamScores500Disadvantage[BG_TEAMS_COUNT];
};
#endif
