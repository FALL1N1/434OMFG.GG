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

#ifndef __BATTLEGROUNDEYR_H
#define __BATTLEGROUNDEYR_H

#include "Battleground.h"
#include "Language.h"

enum BG_EYR_WorldStates
{
    EYR_ALLIANCE_RESOURCES              = 2749,
    EYR_HORDE_RESOURCES                 = 2750,
    EYR_ALLIANCE_BASE                   = 2752,
    EYR_HORDE_BASE                      = 2753,
    BG_EYR_OP_OCCUPIED_BASES_HORDE      = 2752,
    BG_EYR_OP_OCCUPIED_BASES_ALLY       = 2753,
    BG_EYR_OP_RESOURCES_ALLY            = 1776,
    BG_EYR_OP_RESOURCES_HORDE           = 1777,
    BG_EYR_OP_RESOURCES_MAX             = 1780,
    BG_EYR_OP_RESOURCES_WARNING         = 1955,
    EYR_NETHERSTORM_FLAG                = 2757,
    EYR_NETHERSTORM_FLAG_STATE_ALLIANCE = 2769,
    EYR_NETHERSTORM_FLAG_STATE_HORDE    = 2770,
    EYR_DRAENEI_RUINS_HORDE_CONTROL     = 2733,
    EYR_DRAENEI_RUINS_ALLIANCE_CONTROL  = 2732,
    EYR_DRAENEI_RUINS_UNCONTROL         = 2731,
    EYR_MAGE_TOWER_ALLIANCE_CONTROL     = 2730,
    EYR_MAGE_TOWER_HORDE_CONTROL        = 2729,
    EYR_MAGE_TOWER_UNCONTROL            = 2728,
    EYR_FEL_REAVER_HORDE_CONTROL        = 2727,
    EYR_FEL_REAVER_ALLIANCE_CONTROL     = 2726,
    EYR_FEL_REAVER_UNCONTROL            = 2725,
    EYR_BLOOD_ELF_HORDE_CONTROL         = 2724,
    EYR_BLOOD_ELF_ALLIANCE_CONTROL      = 2723,
    EYR_BLOOD_ELF_UNCONTROL             = 2722,
};

enum EYRBattlegroundPoints
{
    EYR_FEL_REAVER             = 0,
    EYR_BLOOD_ELF              = 1,
    EYR_DRAENEI_RUINS          = 2,
    EYR_MAGE_TOWER             = 3,

    EYR_PLAYERS_OUT_OF_POINTS  = 4,
    EYR_POINTS_MAX             = 4
};

enum EYRBattlegroundPointsTrigger
{
    EYR_TR_BLOOD_ELF_POINT        = 4476,
    EYR_TR_FEL_REAVER_POINT       = 4514,
    EYR_TR_MAGE_TOWER_POINT       = 4516,
    EYR_TR_DRAENEI_RUINS_POINT    = 4518,
    EYR_TR_BLOOD_ELF_BUFF         = 4568,
    EYR_TR_FEL_REAVER_BUFF        = 4569,
    EYR_TR_MAGE_TOWER_BUFF        = 4570,
    EYR_TR_DRAENEI_RUINS_BUFF     = 4571
};

enum BG_EYR_FlagState
{
    BG_EYR_FLAG_STATE_ON_BASE            = 0,
    BG_EYR_FLAG_STATE_WAIT_RESPAWN       = 1,
    BG_EYR_FLAG_STATE_ON_PLAYER          = 2,
    BG_EYR_FLAG_STATE_ON_GROUND          = 3,

    BG_EYR_FLAG_RESPAWN_TIME             = (8*IN_MILLISECONDS),
    BG_EYR_FPOINTS_TICK_TIME             = (2*IN_MILLISECONDS),

    BG_EYR_NETHERSTORM_FLAG_SPELL        = 34976,
    BG_EYR_PLAYER_DROPPED_FLAG_SPELL     = 34991

};

const uint32 BG_EYR_FlagPoints[4]        = { 75, 85, 100, 500 };

const uint32 BG_EYR_OP_NODESTATES[4]     = { 2731, 2728, 2725, 2722 };

const uint32 BG_EYR_OP_NODEICONS[4]      = { 2731, 2728, 2725, 2722 };

/* Note: code uses that these IDs follow each other */
enum BG_EYR_NodeObjectId
{
    BG_EYR_OBJECTID_NODE_BANNER_0        = 180087,       // Stables banner
    BG_EYR_OBJECTID_NODE_BANNER_1        = 180088,       // Blacksmith banner
    BG_EYR_OBJECTID_NODE_BANNER_2        = 180089,       // Farm banner
    BG_EYR_OBJECTID_NODE_BANNER_3        = 180090,       // Lumber mill banner
    BG_EYR_OBJECTID_NODE_BANNER_4        = 180091,        // Gold mine banner

    BG_EYR_RBG_OBJECTID_NODE_BANNER_0    = 406000,       // Stables banner
    BG_EYR_RBG_OBJECTID_NODE_BANNER_1    = 406001,       // Blacksmith banner
    BG_EYR_RBG_OBJECTID_NODE_BANNER_2    = 406002,       // Farm banner
    BG_EYR_RBG_OBJECTID_NODE_BANNER_3    = 406003,       // Lumber mill banner
    BG_EYR_RBG_OBJECTID_NODE_BANNER_4    = 406004        // Gold mine banner
};

enum BG_EYR_ObjectType
{
    // for all 5 node points 8*5=40 objects
    BG_EYR_OBJECT_BANNER_NEUTRAL                 = 0,
    BG_EYR_OBJECT_BANNER_CONT_A                  = 1,
    BG_EYR_OBJECT_BANNER_CONT_H                  = 2,
    BG_EYR_OBJECT_BANNER_ALLY                    = 3,
    BG_EYR_OBJECT_BANNER_HORDE                   = 4,
    BG_EYR_OBJECT_AURA_ALLY                      = 5,
    BG_EYR_OBJECT_AURA_HORDE                     = 6,
    BG_EYR_OBJECT_AURA_CONTESTED                 = 7,
    //gates
    BG_EYR_OBJECT_GATE_A                         = 40,
    BG_EYR_OBJECT_GATE_H                         = 41,
    BG_EYR_OBJECT_FLAG_NETHERSTORM               = 42,
    //buffs
    BG_EYR_OBJECT_SPEEDBUFF_FEL_REAVER           = 43,
    BG_EYR_OBJECT_REGENBUFF_FEL_REAVER           = 44,
    BG_EYR_OBJECT_BERSERKBUFF_FEL_REAVER         = 45,
    BG_EYR_OBJECT_SPEEDBUFF_BLOOD_ELF            = 46,
    BG_EYR_OBJECT_REGENBUFF_BLOOD_ELF            = 47,
    BG_EYR_OBJECT_BERSERKBUFF_BLOOD_ELF          = 48,
    BG_EYR_OBJECT_SPEEDBUFF_DRAENEI_RUINS        = 49,
    BG_EYR_OBJECT_REGENBUFF_DRAENEI_RUINS        = 50,
    BG_EYR_OBJECT_BERSERKBUFF_DRAENEI_RUINS      = 51,
    BG_EYR_OBJECT_SPEEDBUFF_MAGE_TOWER           = 52,
    BG_EYR_OBJECT_REGENBUFF_MAGE_TOWER           = 53,
    BG_EYR_OBJECT_BERSERKBUFF_MAGE_TOWER         = 54,

    BG_EYR_OBJECT_MAX                            = 55
};

/* Object id templates from DB */
enum BG_EYR_ObjectTypes
{
    BG_EYR_OBJECTID_BANNER_A             = 180058,
    BG_EYR_OBJECTID_BANNER_CONT_A        = 180059,
    BG_EYR_OBJECTID_BANNER_H             = 180060,
    BG_EYR_OBJECTID_BANNER_CONT_H        = 180061,

    BG_EYR_RBG_OBJECTID_BANNER_A         = 406005,
    BG_EYR_RBG_OBJECTID_BANNER_CONT_A    = 406006,
    BG_EYR_RBG_OBJECTID_BANNER_H         = 406007,
    BG_EYR_RBG_OBJECTID_BANNER_CONT_H    = 406008,

    BG_EYR_OBJECTID_AURA_A               = 180100,
    BG_EYR_OBJECTID_AURA_H               = 180101,
    BG_EYR_OBJECTID_AURA_C               = 180102,

    BG_OBJECT_A_DOOR_EYR_ENTRY           = 184719,
    BG_OBJECT_H_DOOR_EYR_ENTRY           = 184720
};

enum BG_EYR_Timers
{
    BG_EYR_FLAG_CAPTURING_TIME           = 60000
};

enum BG_EYR_Score
{
    BG_EYR_WARNING_NEAR_VICTORY_SCORE    = 1400,
    BG_EYR_MAX_TEAM_SCORE                = 1600
};

/* do NOT change the order, else wrong behaviour */
enum BG_EYR_BattlegroundNodes
{
    BG_EYR_NODE_FEL_REAVER       = 0,
    BG_EYR_NODE_BLOOD_ELF        = 1,
    BG_EYR_NODE_DRAENEI_RUINS    = 2,
    BG_EYR_NODE_MAGE_TOWER       = 3,

    BG_EYR_DYNAMIC_NODES_COUNT   = 4,                        // dynamic nodes that can be captured

    EYR_SPIRIT_FEL_REAVER        = 0,
    EYR_SPIRIT_BLOOD_ELF         = 1,
    EYR_SPIRIT_DRAENEI_RUINS     = 2,
    EYR_SPIRIT_MAGE_TOWER        = 3,
    EYR_SPIRIT_MAIN_ALLIANCE     = 4,
    EYR_SPIRIT_MAIN_HORDE        = 5,

    BG_EYR_ALL_NODES_COUNT       = 6                         // all nodes (dynamic and static)
};

struct BattlegroundEYRLosingPointStruct
{
    uint32 MessageIdAlliance;
    uint32 MessageIdHorde;
};

const BattlegroundEYRLosingPointStruct m_LosingPointTypesR[BG_EYR_DYNAMIC_NODES_COUNT] =
{
    { LANG_BG_EY_HAS_LOST_A_F_RUINS, LANG_BG_EY_HAS_LOST_H_F_RUINS },
    { LANG_BG_EY_HAS_LOST_A_B_TOWER, LANG_BG_EY_HAS_LOST_H_B_TOWER },
    { LANG_BG_EY_HAS_LOST_A_D_RUINS, LANG_BG_EY_HAS_LOST_H_D_RUINS },
    { LANG_BG_EY_HAS_LOST_A_M_TOWER, LANG_BG_EY_HAS_LOST_H_M_TOWER }
};

struct BattlegroundEYRCapturingPointStruct
{
    uint32 MessageIdAlliance;
    uint32 MessageIdHorde;
};

const BattlegroundEYRCapturingPointStruct m_CapturingPointTypesR[BG_EYR_DYNAMIC_NODES_COUNT] =
{
    { LANG_BG_EY_HAS_TAKEN_A_F_RUINS, LANG_BG_EY_HAS_TAKEN_H_F_RUINS },
    { LANG_BG_EY_HAS_TAKEN_A_B_TOWER, LANG_BG_EY_HAS_TAKEN_H_B_TOWER },
    { LANG_BG_EY_HAS_TAKEN_A_D_RUINS, LANG_BG_EY_HAS_TAKEN_H_D_RUINS },
    { LANG_BG_EY_HAS_TAKEN_A_M_TOWER, LANG_BG_EY_HAS_TAKEN_H_M_TOWER },
};

enum BG_EYR_NodeStatus
{
    BG_EYR_NODE_TYPE_NEUTRAL             = 0,
    BG_EYR_NODE_TYPE_CONTESTED           = 1,
    BG_EYR_NODE_STATUS_ALLY_CONTESTED    = 1,
    BG_EYR_NODE_STATUS_HORDE_CONTESTED   = 2,
    BG_EYR_NODE_TYPE_OCCUPIED            = 3,
    BG_EYR_NODE_STATUS_ALLY_OCCUPIED     = 3,
    BG_EYR_NODE_STATUS_HORDE_OCCUPIED    = 4
};

enum BG_EYR_Sounds
{
    BG_EYR_SOUND_NODE_CLAIMED            = 8192,
    BG_EYR_SOUND_NODE_CAPTURED_ALLIANCE  = 8173,
    BG_EYR_SOUND_NODE_CAPTURED_HORDE     = 8213,
    BG_EYR_SOUND_NODE_ASSAULTED_ALLIANCE = 8212,
    BG_EYR_SOUND_NODE_ASSAULTED_HORDE    = 8174,
    BG_EYR_SOUND_NEAR_VICTORY            = 8456,
    BG_EYR_SOUND_FLAG_RESET              = 8192
};

enum BG_EYR_Objectives
{
    EYR_OBJECTIVE_ASSAULT_BASE           = 122,
    EYR_OBJECTIVE_DEFEND_BASE            = 123,
    EYR_OBJECTIVE_CAPTURE_FLAG           = 183,
};

#define BG_EYR_NotEYRBGWeekendHonorTicks      260
#define BG_EYR_EYRBGWeekendHonorTicks         160
#define BG_EYR_NotEYRBGWeekendReputationTicks 160
#define BG_EYR_EYRBGWeekendReputationTicks    120

#define EYR_EVENT_START_BATTLE                9158 // Achievement: Let's Get This Done

// x, y, z, o
const float BG_EYR_NodePositions[BG_EYR_DYNAMIC_NODES_COUNT][4] =
{
    {2044.28f, 1729.68f, 1189.96f, 4.757453f},  // FEL_REAVER center
    {2048.83f, 1393.65f, 1194.49f, 1.68944f},   // BLOOD_ELF center
    {2286.56f, 1402.36f, 1197.11f, 1.66381f},   // DRAENEI_RUINS center
    {2284.48f, 1731.23f, 1189.99f, 4.92725f}    // MAGE_TOWER center
};

// x, y, z, o, rot0, rot1, rot2, rot3
const float BG_EYR_DoorPositions[2][8] =
{
    { 2527.6f,   1596.91f,  1262.13f, -3.12414f, -0.173642f, -0.001515f, 0.98477f,  -0.008594f  },
    { 1803.21f,  1539.49f,  1261.09f, 3.14159f,  0.173648f,  0.0f,       0.984808f, 0.0f        },
};

// Tick intervals and given points: case 0, 1, 2, 3, 4, 5 captured nodes
const uint32 BG_EYR_TickIntervals[6] = {0, 12000, 9000, 6000, 3000, 1000};
const uint32 BG_EYR_TickPoints[6] = {0, 10, 10, 10, 10, 30};


enum EYRBattlegroundGaveyards
{
    EYR_GRAVEYARD_MAIN_ALLIANCE     = 1103,
    EYR_GRAVEYARD_MAIN_HORDE        = 1104,
    EYR_GRAVEYARD_FEL_REAVER        = 1105,
    EYR_GRAVEYARD_BLOOD_ELF         = 1106,
    EYR_GRAVEYARD_DRAENEI_RUINS     = 1107,
    EYR_GRAVEYARD_MAGE_TOWER        = 1108
};

// WorldSafeLocs ids for 5 nodes, and for ally, and horde starting location
const uint32 BG_EYR_GraveyardIds[BG_EYR_ALL_NODES_COUNT] = {EYR_GRAVEYARD_FEL_REAVER, EYR_GRAVEYARD_BLOOD_ELF, EYR_GRAVEYARD_DRAENEI_RUINS, EYR_GRAVEYARD_MAGE_TOWER, EYR_GRAVEYARD_MAIN_ALLIANCE, EYR_GRAVEYARD_MAIN_HORDE};

// x, y, z, o
const float BG_EYR_BuffPositions[BG_EYR_DYNAMIC_NODES_COUNT][4] =
{
    {2044.28f, 1729.68f, 1189.96f, 0.017453f},  // FEL_REAVER center
    {2048.83f, 1393.65f, 1194.49f, 0.20944f},   // BLOOD_ELF center
    {2286.56f, 1402.36f, 1197.11f, 3.72381f},   // DRAENEI_RUINS center
    {2284.48f, 1731.23f, 1189.99f, 2.89725f}    // MAGE_TOWER center
};

// x, y, z, o
const float BG_EYR_SpiritGuidePos[BG_EYR_ALL_NODES_COUNT][4] =
{
    {2044.28f, 1729.68f, 1189.96f, 0.017453f},  // FEL_REAVER center
    {2048.83f, 1393.65f, 1194.49f, 0.20944f},   // BLOOD_ELF center
    {2286.56f, 1402.36f, 1197.11f, 3.72381f},   // DRAENEI_RUINS center
    {2284.48f, 1731.23f, 1189.99f, 2.89725f}    // MAGE_TOWER center
};

struct BG_EYR_BannerTimer
{
    uint32      timer;
    uint8       type;
    uint8       teamIndex;
};

struct BattlegroundEYRScore : public BattlegroundScore
{
 BattlegroundEYRScore(): BasesAssaulted(0), BasesDefended(0), FlagCaptures(0) { }
    ~BattlegroundEYRScore() { }
    uint32 BasesAssaulted;
    uint32 BasesDefended;
    uint32 FlagCaptures;
};

class BattlegroundEYR : public Battleground
{
    public:
        BattlegroundEYR();
        ~BattlegroundEYR();

        void AddPlayer(Player* player);
        void StartingEventCloseDoors();
        void StartingEventOpenDoors();
        void RemovePlayer(Player* player, uint64 guid, uint32 team);
        void HandleAreaTrigger(Player* Source, uint32 Trigger);
        void HandleKillPlayer(Player* player, Player* killer);
        bool SetupBattleground();
        void Reset();
        void EndBattleground(uint32 winner);
        WorldSafeLocsEntry const* GetClosestGraveYard(Player* player);

        /* Scorekeeping */
        void UpdatePlayerScore(Player* Source, uint32 type, uint32 value, bool doAddHonor = true);

        void FillInitialWorldStates(WorldPacket& data);
        void SetDroppedFlagGUID(uint64 guid, int32 /*TeamID*/ = -1)  { m_DroppedFlagGUID = guid;}
        uint64 GetDroppedFlagGUID() const          { return m_DroppedFlagGUID;}

        /* Nodes occupying */
        void EventPlayerClickedOnFlag(Player* source, GameObject* target_obj);
        void EventPlayerClickedOnMiddleFlag(Player* source, GameObject* target_obj);
        void EventPlayerDroppedFlag(Player* Source);
        void EventPlayerCapturedFlag(Player* Source, uint32 BgObjectType);

        /* achievement req. */
        bool IsAllNodesConrolledByTeam(uint32 team) const;  // overwrited
        bool IsTeamScores500Disadvantage(uint32 team) const { return m_TeamScores500Disadvantage[GetTeamIndexByTeamId(team)]; }

        uint32 GetPrematureWinner();

        /* BG Flags */
        uint64 GetFlagPickerGUID(int32 /*team*/ = -1) const    { return m_FlagKeeper; }
        void SetFlagPicker(uint64 guid)     { m_FlagKeeper = guid; }
        bool IsFlagPickedup() const         { return m_FlagKeeper != 0; }
        uint8 GetFlagState() const          { return m_FlagState; }
        void RespawnFlag(bool send_message);
        void RespawnFlagAfterDrop();

    private:
        void PostUpdateImpl(uint32 diff);
        /* Gameobject spawning/despawning */
        void _CreateBanner(uint8 node, uint8 type, uint8 teamIndex, bool delay);
        void _DelBanner(uint8 node, uint8 type, uint8 teamIndex);
        void _SendNodeUpdate(uint8 node);

        /* Creature spawning/despawning */
        // TODO: working, scripted peons spawning
        void _NodeOccupied(uint8 node, Team team);
        void _NodeDeOccupied(uint8 node);

        void UpdatePointWorldStates();

        /* Nodes info:
            0: neutral
            1: ally contested
            2: horde contested
            3: ally occupied
            4: horde occupied     */
        uint8               m_Nodes[BG_EYR_DYNAMIC_NODES_COUNT];
        uint8               m_prevNodes[BG_EYR_DYNAMIC_NODES_COUNT];
        BG_EYR_BannerTimer   m_BannerTimers[BG_EYR_DYNAMIC_NODES_COUNT];
        uint32              m_NodeTimers[BG_EYR_DYNAMIC_NODES_COUNT];
        uint32              m_lastTick[BG_TEAMS_COUNT];
        uint32              m_HonorScoreTics[BG_TEAMS_COUNT];
        uint32              m_ReputationScoreTics[BG_TEAMS_COUNT];
        bool                m_IsInformedNearVictory;
        uint32              m_HonorTics;
        uint32              m_ReputationTics;
        // need for achievements
        bool                m_TeamScores500Disadvantage[BG_TEAMS_COUNT];
        uint32              m_Points_Trigger[BG_EYR_DYNAMIC_NODES_COUNT];

        uint64 m_FlagKeeper;                                // keepers guid
        uint64 m_DroppedFlagGUID;
        uint32 m_FlagCapturedBgObjectType;                  // type that should be despawned when flag is captured
        uint8 m_FlagState;                                  // for checking flag state
        int32 m_FlagsTimer;
        uint32 mui_flagTimer;
};
#endif
