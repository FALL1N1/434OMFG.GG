/*
 * Copyright (C) 2013-2014 OMFG.GG Network <http://www.omfg.gg/>
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
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

#ifndef BATTLEFIELD_TB_
#define BATTLEFIELD_TB_

#include "Battlefield.h"
#include "Group.h"
#include "ObjectMgr.h"
#include "WorldPacket.h"
#include "World.h"
#include "Player.h"
#include "ObjectMgr.h"

const uint32 TBClockWorldState[2] = {5333, 5332};
const uint32 TolBaradFaction[3] = {2354, 2355, 35};

class Group;
class BattlefieldTB;
class BfCapturePointTB;

struct BfTBGameObjectBuilding;
struct BfTBWorkShopData;

typedef std::set<GameObject*> TBGameObjectSet;
typedef std::set<BfTBGameObjectBuilding*> TBGameObjectBuilding;
typedef std::set<BfTBWorkShopData*> TBWorkShop;
typedef std::set<BfCapturePointTB*> TBCapturePointSet;
typedef std::set<Group*> TBGroupSet;

enum eTBpell
{
    SPELL_SPIRITUAL_IMMUNITY_TB                     = 95332,
    SPELL_ALLIANCE_FLAG_TB                          = 14268,
    SPELL_HORDE_FLAG_TB                             = 14267,

    // Reward spells
    SPELL_VICTORY_REWARD_ALLIANCE_TB                = 89789,
    SPELL_VICTORY_REWARD_HORDE_TB                   = 89791,
    SPELL_DEFEAT_REWARD_TB                          = 89793,
    SPELL_DAMAGED_TOWER_TB                          = 89795,
    SPELL_DESTROYED_TOWER_TB                        = 89796,
    SPELL_DEFENDED_TOWER_TB                         = 89794,

    SPELL_VETERAN                                   = 84655,
    SPELL_SLOW_FALL_TB                              = 88473,

    SPELL_TOL_BARAD_VICTORY_ALLIANCE                = 89789,
    SPELL_TOL_BARAD_VICTORY_HORDE                   = 89791,
    SPELL_TOL_BARAD_DEFEAT                          = 89793,
    SPELL_TOL_BARAD_TOWER_DEFENDED                  = 89794,
    SPELL_TOL_BARAD_TOWER_DAMAGED                   = 89795,
    SPELL_TOL_BARAD_TOWER_DESTROYED                 = 89796,

    SPELL_TOL_BARAD_QUEST_ALLIANCE                  = 94665,
    SPELL_TOL_BARAD_QUEST_HORDE                     = 94763,

    SPELL_TOL_BARAD_THE_HOLE_DESACTIVATE_TELEPORT   = 89035,
    SPELL_TOL_BARAD_D_BLOCK_DESACTIVATE_TELEPORT    = 89037,
    SPELL_TOL_BARAD_DEPTH_DESACTIVATE_TELEPORT      = 89038,
};

enum eTBData32
{
    BATTLEFIELD_TB_DATA_DAMAGED_TOWER_DEF,
    BATTLEFIELD_TB_DATA_BROKEN_TOWER_DEF,
    BATTLEFIELD_TB_DATA_DAMAGED_TOWER_ATT,
    BATTLEFIELD_TB_DATA_BROKEN_TOWER_ATT,
    BATTLEFIELD_TB_DATA_CAPTURED_FORT_DEF,
    BATTLEFIELD_TB_DATA_CAPTURED_FORT_ATT,
    BATTLEFIELD_TB_DATA_MAX,
};

/*#########################
*####### Graveyards ######*
#########################*/

class BfGraveYardTB : public BfGraveyard
{
public:
    BfGraveYardTB(BattlefieldTB* Bf);
    void SetTextId(uint32 textid){m_GossipTextId = textid;}
    uint32 GetTextId(){return m_GossipTextId;}
protected:
    uint32 m_GossipTextId;
};

enum eTBGraveyardId
{
    BATTLEFIELD_TB_GY_KEEP,
    BATTLEFIELD_TB_GY_WARDENS_VIGIL,
    BATTLEFIELD_TB_GY_IRONGLAD,
    BATTLEFIELD_TB_GY_SLAGWORKS,
    BATTLEFIELD_TB_GY_WEST_SPIRE,
    BATTLEFIELD_TB_GY_SOUTH_SPIRE,
    BATTLEFIELD_TB_GY_EAST_SPIRE,
    //    BATTLEFIELD_TB_GY_HORDE,
    //  BATTLEFIELD_TB_GY_ALLIANCE,
    BATTLEFIELD_TB_GY_MAX,
};

enum eTBGossipText
{
    BATTLEFIELD_TB_GOSSIPTEXT_GY_KEEP             = -1732000,
    BATTLEFIELD_TB_GOSSIPTEXT_GY_WARDENS_VIGIL    = -1732001,
    BATTLEFIELD_TB_GOSSIPTEXT_GY_IRONGLAD         = -1732002,
    BATTLEFIELD_TB_GOSSIPTEXT_GY_WEST_SPIRE       = -1732003,
    BATTLEFIELD_TB_GOSSIPTEXT_GY_SLAGWORKS        = -1732004,
    BATTLEFIELD_TB_GOSSIPTEXT_GY_SOUTH_SPIRE      = -1732005,
    BATTLEFIELD_TB_GOSSIPTEXT_GY_EAST_SPIRE       = -1732006,
    BATTLEFIELD_TB_GOSSIPTEXT_GY_HORDE            = -1732007,
    BATTLEFIELD_TB_GOSSIPTEXT_GY_ALLIANCE         = -1732008,
};

enum eTBFortType
{
    BATTLEFIELD_TB_FORT_WARDENS_VIGIL,
    BATTLEFIELD_TB_FORT_IRONGLAD,
    BATTLEFIELD_TB_FORT_SLAGWORKS,
    BATTLEFIELD_TB_FORT_MAX,
};

enum eTBNpc
{
    BATTLEFIELD_TB_NPC_GUARD_H   = 51166,
    BATTLEFIELD_TB_NPC_GUARD_A   = 51165,
    BATTLEFIELD_TB_NPC_HUNTER_A  = 47595,
    BATTLEFIELD_TB_NPC_MAGE_A    = 47598,
    BATTLEFIELD_TB_NPC_WARRIOR_A = 47599,
    BATTLEFIELD_TB_NPC_PALADIN_A = 47600,
    BATTLEFIELD_TB_NPC_DRUID_H   = 47607,
    BATTLEFIELD_TB_NPC_MAGE_H    = 47608,
    BATTLEFIELD_TB_NPC_ROGUE_H   = 47609,
    BATTLEFIELD_TB_NPC_SHAMAN_H  = 47610,
    BATTLEFIELD_TB_VEHICLE       = 45344,

    BATTLEFIELD_TB_NPC_PROBLIM = 47593,
    BATTLEFIELD_TB_NPC_CROCO = 47591,

    ALLIANCE_SPRIT_GUIDE = 45066,
    HORDE_SPIRIT_GUIDE = 45079,
};

enum ePath
{
    NPC_PROBLIM_PATH = 370099,
};

enum TolBaradWorldStates
{
    BATTLEFIELD_TB_WORLD_STATE_ACTIVE            = 5387,
    BATTLEFIELD_TB_WORLD_STATE_DEFENDER          = 5384,
    BATTLEFIELD_TB_WORLD_STATE_ATTACKER          = 5385,
    BATTLEFIELD_TB_WORLD_STATE_SHOW_WORLDSTATE   = 5346,


    TB_WS_GARRISON_HORDE_CONTROLLED = 5418,
    TB_WS_GARRISON_HORDE_CAPTURING = 5419,
    TB_WS_GARRISON_ALLIANCE_CAPTURING = 5421,
    TB_WS_GARRISON_ALLIANCE_CONTROLLED = 5422,

    TB_WS_VIGIL_HORDE_CONTROLLED = 5423,
    TB_WS_VIGIL_HORDE_CAPTURING = 5424,
    TB_WS_VIGIL_ALLIANCE_CAPTURING = 5426,
    TB_WS_VIGIL_ALLIANCE_CONTROLLED = 5427,

    TB_WS_SLAGWORKS_HORDE_CONTROLLED = 5428,
    TB_WS_SLAGWORKS_HORDE_CAPTURING = 5429,
    TB_WS_SLAGWORKS_ALLIANCE_CAPTURING = 5431,
    TB_WS_SLAGWORKS_ALLIANCE_CONTROLLED = 5432,

    TB_WS_WEST_INTACT_HORDE = 5433,
    TB_WS_WEST_DAMAGED_HORDE = 5434,
    TB_WS_WEST_DESTROYED_NEUTRAL = 5435,
    TB_WS_WEST_INTACT_ALLIANCE = 5436,
    TB_WS_WEST_DAMAGED_ALLIANCE = 5437,
    TB_WS_WEST_INTACT_NEUTRAL = 5453,
    TB_WS_WEST_DAMAGED_NEUTRAL = 5454,

    TB_WS_SOUTH_INTACT_HORDE = 5438,
    TB_WS_SOUTH_DAMAGED_HORDE = 5439,
    TB_WS_SOUTH_DESTROYED_NEUTRAL = 5440,
    TB_WS_SOUTH_INTACT_ALLIANCE = 5441,
    TB_WS_SOUTH_DAMAGED_ALLIANCE = 5442,
    TB_WS_SOUTH_INTACT_NEUTRAL = 5455,
    TB_WS_SOUTH_DAMAGED_NEUTRAL = 5456,

    TB_WS_EAST_INTACT_HORDE = 5443,
    TB_WS_EAST_DAMAGED_HORDE = 5444,
    TB_WS_EAST_DESTROYED_NEUTRAL = 5445,
    TB_WS_EAST_INTACT_ALLIANCE = 5446,
    TB_WS_EAST_DAMAGED_ALLIANCE = 5447,
    TB_WS_EAST_INTACT_NEUTRAL = 5451,
    TB_WS_EAST_DAMAGED_NEUTRAL = 5452,


    TB_WS_TOWERS_DESTROYED = 5347,
    TB_WS_BUILDINGS_CAPTURED = 5348,
    TB_WS_TOWERS_DESTROYED_SHOW = 5350,
    TB_WS_BUILDINGS_CAPTURED_SHOW = 5349,
};

struct BfTBCoordGY
{
    float x;
    float y;
    float z;
    float o;
    uint32 guid;
    uint32 type;
    int textid; // for gossip menu
    TeamId startcontrol;
};

const BfTBCoordGY TBGraveYard[BATTLEFIELD_TB_GY_MAX]=
{
    {-1244.58f, 981.233f, 155.425f, 0.733038f, 1789, BATTLEFIELD_TB_GY_KEEP,              BATTLEFIELD_TB_GOSSIPTEXT_GY_KEEP, TEAM_NEUTRAL},
    {-1572.14f, 1169.94f, 159.501f, 5.317802f, 1785, BATTLEFIELD_TB_GY_WARDENS_VIGIL,     BATTLEFIELD_TB_GOSSIPTEXT_GY_WARDENS_VIGIL, TEAM_NEUTRAL},
    {-970.465f, 1088.33f, 132.992f, 6.268125f, 1783, BATTLEFIELD_TB_GY_IRONGLAD,          BATTLEFIELD_TB_GOSSIPTEXT_GY_IRONGLAD, TEAM_NEUTRAL},
    {-1343.65f, 568.823f, 139.158f, 4.819249f, 1787, BATTLEFIELD_TB_GY_SLAGWORKS,         BATTLEFIELD_TB_GOSSIPTEXT_GY_SLAGWORKS, TEAM_NEUTRAL},
    {-1052.1f,  1490.65f, 191.407f, 0.008500f, 1784, BATTLEFIELD_TB_GY_WEST_SPIRE,        BATTLEFIELD_TB_GOSSIPTEXT_GY_WEST_SPIRE, TEAM_NEUTRAL},
    {-1600.28f, 869.21f,  193.948f, 4.819249f, 1786, BATTLEFIELD_TB_GY_SOUTH_SPIRE,       BATTLEFIELD_TB_GOSSIPTEXT_GY_SOUTH_SPIRE, TEAM_NEUTRAL},
    {-944.34f,  576.111f, 157.543f, 4.819249f, 1788, BATTLEFIELD_TB_GY_EAST_SPIRE,        BATTLEFIELD_TB_GOSSIPTEXT_GY_EAST_SPIRE, TEAM_NEUTRAL},
    //    {-540.168f, 1331.29f, 22.8201f, 3.971623f, 1807, BATTLEFIELD_TB_GY_HORDE,          BATTLEFIELD_TB_GOSSIPTEXT_GY_HORDE, TEAM_HORDE},
    //  {-456.946f, 1168.55f, 15.7554f, 1.972220f, 1808, BATTLEFIELD_TB_GY_ALLIANCE,       BATTLEFIELD_TB_GOSSIPTEXT_GY_ALLIANCE, TEAM_ALLIANCE},
};

/*#########################
* BfCapturePointTB       *
#########################*/

class BfCapturePointTB: public BfCapturePoint
{
    public:
    BfCapturePointTB(BattlefieldTB* bf, TeamId control);
    void LinkToWorkShop(BfTBWorkShopData* ws) {m_Workshop = ws;}

    void ChangeTeam(TeamId oldteam);
    TeamId GetTeam() const { return m_team; }

protected:
    BfTBWorkShopData* m_Workshop;
};

/*#########################
* TolBarad Battlefield    *
#########################*/

class BattlefieldTB: public Battlefield
{
    public:
    ~BattlefieldTB();
    void OnBattleStart();
    void OnBattleEnd(bool endbytimer);
    void OnStartGrouping();
    void OnPlayerJoinWar(Player* player);
    void OnPlayerLeaveWar(Player* player);
    void OnPlayerLeaveZone(Player* player);
    void OnPlayerEnterZone(Player* player);
    bool Update(uint32 diff);
    void AddBrokenTower(TeamId team);
    void DoCompleteOrIncrementAchievement(uint32 achievement, Player* player, uint8 incrementNumber = 1);
    void AddPlayerToResurrectQueue(uint64 npc_guid, uint64 player_guid);
    bool SetupBattlefield();

    void FillInitialWorldStates(WorldPacket& data);
    void SendInitWorldStatesTo(Player* player);
    void SendInitWorldStatesToAll();

    void ProcessEvent(WorldObject* obj, uint32 eventId);

    void StartDynamicQuests();

    void OnGameObjectCreate(GameObject* go);
    void HandleKill(Player* killer, Unit* victim);

    void AnimateCreature(Creature* creature);

protected:
    TBGameObjectBuilding BuildingsInZone;
    GuidSet BlocDCreature;
    GuidSet TrouCreature;
    GuidSet DepthCreature;
    GuidSet DynamicQuestCreatre[2];
    GuidSet KeepCreature[2];
    GuidSet WarCreature;
    TBWorkShop WorkShopList;
    TBGameObjectSet m_KeepGameObject[2];
    GuidSet m_vehicles[2];
    GuidSet m_PlayersIsSpellImu;  // Player is dead
    uint32 m_saveTimer;
    uint32 _dynamicQuestCounter;
    uint64 uiDoodadDoor;
    uint64 uiDepthDoor;
    uint64 uiHallDoor;
    uint64 uiBlocDDoor;
};

enum eTBGameObjectBuildingType
{
    BATTLEFIELD_TB_OBJECTTYPE_TOWER,
};

enum eTBGameObjectState
{
    BATTLEFIELD_TB_OBJECTSTATE_NONE,
    BATTLEFIELD_TB_OBJECTSTATE_NEUTRAL_INTACT, // Not sure
    BATTLEFIELD_TB_OBJECTSTATE_NEUTRAL_DAMAGE, // Not sure
    BATTLEFIELD_TB_OBJECTSTATE_NEUTRAL_DESTROY, // Not sure
    BATTLEFIELD_TB_OBJECTSTATE_HORDE_INTACT,
    BATTLEFIELD_TB_OBJECTSTATE_HORDE_DAMAGE,
    BATTLEFIELD_TB_OBJECTSTATE_HORDE_DESTROY,
    BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_INTACT,
    BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_DAMAGE,
    BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_DESTROY,
};

enum eTBTeamControl
{
    BATTLEFIELD_TB_TEAM_ALLIANCE,
    BATTLEFIELD_TB_TEAM_HORDE,
    BATTLEFIELD_TB_TEAM_NEUTRAL,
};

enum eTBObject
{
    // horde (used for both faction)
    BATTLEFIELD_TB_GAMEOBJECT_CAPTURE_NORTH     = 205096,
    BATTLEFIELD_TB_GAMEOBJECT_CAPTURE_WEST      = 205139,
    BATTLEFIELD_TB_GAMEOBJECT_CAPTURE_EAST      = 205103,

    //alliance (not used)
    BATTLEFIELD_TB_GAMEOBJECT_CAPTURE_NORTH_ALLIANCE     = 205068,
    BATTLEFIELD_TB_GAMEOBJECT_CAPTURE_WEST_ALLIANCE      = 205101,
    BATTLEFIELD_TB_GAMEOBJECT_CAPTURE_EAST_ALLIANCE      = 205138,

    BATTLEFIELD_TB_GAMEOBJECT_BANNER_ALLY       = 207391,
    BATTLEFIELD_TB_GAMEOBJECT_BANNER_HORDE      = 207400,

    BATTLEFIELD_TB_GAMEOBJECT_DOODAD_DOOR       = 206576, // instance door

    BATTLEFIELD_TB_GAMEOBJECT_DEPTH_DOOR        = 206843, // depth
    BATTLEFIELD_TB_GAMEOBJECT_THE_HALL_DOOR     = 206845, // le trou
    BATTLEFIELD_TB_GAMEOBJECT_BLOCD_DOOR        = 206844, // bloc D
};

enum eTBText
{
    BATTLEFIELD_TB_TEXT_FORT_TAKEN              = 12080,
    BATTLEFIELD_TB_TEXT_FORT_ATTACK             = 12081,
    BATTLEFIELD_TB_TEXT_WARDENS_VIGIL_NAME      = 12082,
    BATTLEFIELD_TB_TEXT_SLAGWORKS_NAME          = 12083,
    BATTLEFIELD_TB_TEXT_IRONCLAD_GARRISON_NAME  = 12084,
    BATTLEFIELD_TB_TEXT_HORDE                   = 12085,
    BATTLEFIELD_TB_TEXT_ALLIANCE                = 12086,
    BATTLEFIELD_TB_TEXT_WILL_START              = 12087,
    BATTLEFIELD_TB_TEXT_FIRSTRANK               = 12088,
    BATTLEFIELD_TB_TEXT_SECONDRANK              = 12089,
    BATTLEFIELD_TB_TEXT_TOWER_DAMAGE            = 12090,
    BATTLEFIELD_TB_TEXT_TOWER_DESTROY           = 12091,
    BATTLEFIELD_TB_TEXT_START                   = 12092,
    BATTLEFIELD_TB_TEXT_DEFEND_KEEP             = 12093,
    BATTLEFIELD_TB_TEXT_TOWER_NAME_SOUTH        = 12094,
    BATTLEFIELD_TB_TEXT_TOWER_NAME_EAST         = 12095,
    BATTLEFIELD_TB_TEXT_TOWER_NAME_WEST         = 12096,
    BATTLEFIELD_TB_TEXT_WIN_KEEP                = 12097,
};

struct BfTBObjectPosition
{
    float x;
    float y;
    float z;
    float o;
    uint32 entryh;
    uint32 entrya;
};

//*********************************************************
//************Destructible (Wall, Tower..)******************
//*********************************************************

struct BfTBBuildingSpawnData
{
    uint32 entry;
    uint32 WorldState;
    float x;
    float y;
    float z;
    float o;
    uint32 type;
    uint32 nameid;
};

#define TB_MAX_OBJ 3
const BfTBBuildingSpawnData TBGameObjectBuillding[TB_MAX_OBJ] =
{
    //South tower
    {204588, 5453, -950.41f, 1469.1f, 176.596f, -2.10312f,   BATTLEFIELD_TB_OBJECTTYPE_TOWER, BATTLEFIELD_TB_TEXT_TOWER_NAME_WEST},
    {204590, 3705, -1618.91f, 954.542f, 168.601f, 0.069812f, BATTLEFIELD_TB_OBJECTTYPE_TOWER, BATTLEFIELD_TB_TEXT_TOWER_NAME_SOUTH},
    {204589, 3706, -1013.28f, 529.538f, 146.427f, 1.97222f,  BATTLEFIELD_TB_OBJECTTYPE_TOWER, BATTLEFIELD_TB_TEXT_TOWER_NAME_EAST},
};

//*********************************************************
//**********Keep Element(GameObject, Creature)**************
//*********************************************************

#define TB_MAX_WAR_NPC 10
const BfTBObjectPosition TBWarNPC[TB_MAX_WAR_NPC] =
{
    // X        Y         Z         O          horde                       alliance
    {-817.969f,  1195.67f, 112.003f, 6.23603f,  BATTLEFIELD_TB_NPC_GUARD_A, BATTLEFIELD_TB_NPC_GUARD_H}, // Guard - bridge
    {-722.403f,  1193.26f, 103.641f, 6.25959f,  BATTLEFIELD_TB_NPC_GUARD_A, BATTLEFIELD_TB_NPC_GUARD_H}, // Guard - bridge
    {-722.751f, 1178.52f, 103.649f, 6.25959f,   BATTLEFIELD_TB_NPC_GUARD_A, BATTLEFIELD_TB_NPC_GUARD_H}, // Guard - bridge
    {-818.092f, 1180.13f, 111.991f, 6.2753f,    BATTLEFIELD_TB_NPC_GUARD_A, BATTLEFIELD_TB_NPC_GUARD_H}, // Guard - bridge
    {-1438.67f, 1140.87f, 123.549f, 4.57729f, BATTLEFIELD_TB_VEHICLE, BATTLEFIELD_TB_VEHICLE}, // Abandoned Siege Engine
    {-1442.58f, 1094.62f, 120.946f, 5.9321f, BATTLEFIELD_TB_VEHICLE, BATTLEFIELD_TB_VEHICLE}, // Abandoned Siege Engine
    {-1274.5f, 798.2f, 120.436f, 1.95014f, BATTLEFIELD_TB_VEHICLE, BATTLEFIELD_TB_VEHICLE},   // Abandoned Siege Engine
    {-1206.68f, 793.54f, 121.123f, 4.78543f, BATTLEFIELD_TB_VEHICLE, BATTLEFIELD_TB_VEHICLE}, // Abandoned Siege Engine
    {-1096.55f, 1086.82f, 124.975f, 2.88555f, BATTLEFIELD_TB_VEHICLE, BATTLEFIELD_TB_VEHICLE}, // Abandoned Siege Engine
    {-1109.07f, 1201.8f, 122.709f, 4.32284f, BATTLEFIELD_TB_VEHICLE, BATTLEFIELD_TB_VEHICLE}, // Abandoned Siege Engine
};

//Here there is all npc keeper spawn point
#define TB_MAX_KEEP_NPC 70
const BfTBObjectPosition TBKeepNPC[TB_MAX_KEEP_NPC] =
{ // TODO - separate some npcs - only bridge npcs should be visible when battle is on. Also need to add vehicles visible at battle start.
    // X        Y         Z         O          horde                       alliance
    // Guards
    {-1265.24f, 1063.95f, 120.313f, 1.42004f,  BATTLEFIELD_TB_NPC_GUARD_H, BATTLEFIELD_TB_NPC_GUARD_A},
    {-1218.90f, 960.533f, 119.728f, 0.51290f,  BATTLEFIELD_TB_NPC_GUARD_H, BATTLEFIELD_TB_NPC_GUARD_A},
    {-1311.70f, 981.560f, 120.541f, 3.07855f,  BATTLEFIELD_TB_NPC_GUARD_H, BATTLEFIELD_TB_NPC_GUARD_A},
    {-1198.08f, 986.733f, 119.728f, 1.66918f,  BATTLEFIELD_TB_NPC_GUARD_H, BATTLEFIELD_TB_NPC_GUARD_A},
    {-1205.72f, 1000.74f, 119.645f, 3.30315f,  BATTLEFIELD_TB_NPC_GUARD_H, BATTLEFIELD_TB_NPC_GUARD_A},
    {-1225.92f, 978.631f, 119.647f, 3.08560f,  BATTLEFIELD_TB_NPC_GUARD_H, BATTLEFIELD_TB_NPC_GUARD_A},
    {-1267.84f, 968.484f, 119.728f, 0.10029f,  BATTLEFIELD_TB_NPC_GUARD_H, BATTLEFIELD_TB_NPC_GUARD_A},
    {-1285.33f, 994.304f, 119.728f, 3.09188f,  BATTLEFIELD_TB_NPC_GUARD_H, BATTLEFIELD_TB_NPC_GUARD_A},
    {-1267.63f, 994.651f, 119.654f, 6.24918f,  BATTLEFIELD_TB_NPC_GUARD_H, BATTLEFIELD_TB_NPC_GUARD_A},
    // trash
    {-1439.92f, 1221.96f, 135.696f, 0.581853f, BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-1478.77f, 1329.18f, 152.962f, 5.75364f,  BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-1416.58f, 1289.43f, 133.603f, 2.08975f,  BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-1432.16f, 1294.02f, 133.629f, 5.44811f,  BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-1441.82f, 1290.96f, 134.240f, 2.69137f,  BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-1422.22f, 693.073f, 123.421f, 0.74358f,  BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-1443.31f, 763.837f, 123.423f, 4.90855f,  BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-1484.08f, 730.777f, 123.422f, 2.30417f,  BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-1537.87f, 662.519f, 123.421f, 6.14083f,  BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-1486.03f, 636.883f, 123.422f, 0.815835f, BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-1428.00f, 600.756f, 123.430f, 1.47871f,  BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-1371.76f, 707.025f, 123.462f, 3.80349f,  BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-1396.22f, 703.874f, 124.116f, 2.28374f,  BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-1421.63f, 728.246f, 124.311f, 5.51566f,  BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-957.513f, 993.60f,  121.779f, 4.68706f,  BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-826.279f, 1031.25f, 121.441f, 3.81762f,  BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-833.906f, 1040.28f, 121.507f, 4.59516f,  BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-967.523f, 911.479f, 121.706f, 1.35225f,  BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-959.251f, 997.010f, 121.921f, 4.76559f,  BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-1000.38f, 979.831f, 122.954f, 3.13589f,  BATTLEFIELD_TB_NPC_WARRIOR_A, BATTLEFIELD_TB_NPC_DRUID_H},
    {-1454.06f, 1270.45f, 133.584f, 0.0f,      BATTLEFIELD_TB_NPC_HUNTER_A,  BATTLEFIELD_TB_NPC_DRUID_H},
    {-1536.95f, 1276.91f, 133.585f, 0.0f,      BATTLEFIELD_TB_NPC_PALADIN_A, BATTLEFIELD_TB_NPC_ROGUE_H},
    {-1490.08f, 1238.24f, 133.584f, 0.0f,      BATTLEFIELD_TB_NPC_PALADIN_A, BATTLEFIELD_TB_NPC_SHAMAN_H},
    {-1474.11f, 643.198f, 123.422f, 0.0f,      BATTLEFIELD_TB_NPC_PALADIN_A, BATTLEFIELD_TB_NPC_SHAMAN_H},
    {-1414.45f, 711.190f, 123.422f, 0.0f,      BATTLEFIELD_TB_NPC_PALADIN_A, BATTLEFIELD_TB_NPC_ROGUE_H},
    {-1446.55f, 756.924f, 123.422f, 0.0f,      BATTLEFIELD_TB_NPC_MAGE_A,    BATTLEFIELD_TB_NPC_ROGUE_H},
    {-1518.74f, 680.033f, 123.422f, 0.0f,      BATTLEFIELD_TB_NPC_MAGE_A,    BATTLEFIELD_TB_NPC_MAGE_H},
    {-1474.18f, 721.503f, 123.423f, 0.0f,      BATTLEFIELD_TB_NPC_MAGE_A,    BATTLEFIELD_TB_NPC_MAGE_H},
    {-945.342f, 1026.22f, 121.441f, 0.0f,      BATTLEFIELD_TB_NPC_MAGE_A,    BATTLEFIELD_TB_NPC_MAGE_H},
    {-832.920f, 969.778f, 121.441f, 0.0f,      BATTLEFIELD_TB_NPC_HUNTER_A,  BATTLEFIELD_TB_NPC_MAGE_H},
    {-883.944f, 976.156f, 121.441f, 5.41424f,  BATTLEFIELD_TB_NPC_PALADIN_A, BATTLEFIELD_TB_NPC_SHAMAN_H},
    {-834.767f, 924.951f, 121.441f, 0.782656f, BATTLEFIELD_TB_NPC_PALADIN_A, BATTLEFIELD_TB_NPC_SHAMAN_H},
    {-837.744f, 1027.21f, 121.441f, 3.95564f,  BATTLEFIELD_TB_NPC_PALADIN_A, BATTLEFIELD_TB_NPC_SHAMAN_H},

    {-1286.42f, 1268.82f, 119.045f, 5.33753f, 47591, 47591},
    {-1266.48f, 1239.15f, 121.776f, 4.40213f, 47591, 47591},
    {-1313.39f, 1198.10f, 119.208f, 3.06852f, 47591, 47591},
    {-1232.17f, 1213.85f, 120.740f, 0.21359f, 47591, 47591},
    {-1165.34f, 1117.91f, 120.876f, 4.11388f, 47591, 47591},
    {-1111.85f, 1212.44f, 123.681f, 2.22893f, 47591, 47591},
    {-1119.85f, 1254.53f, 118.924f, 2.58236f, 47591, 47591},
    {-1140.25f, 1253.44f, 119.248f, 2.58236f, 47591, 47591},
    {-1145.87f, 1270.07f, 120.573f, 2.58236f, 47591, 47591},
    {-1170.93f, 1311.07f, 118.857f, 3.19183f, 47591, 47591},
    {-1210.33f, 1305.95f, 122.211f, 3.45258f, 47591, 47591},
    {-1212.54f, 1293.06f, 119.601f, 3.45258f, 47591, 47591},
    {-1250.69f, 1309.29f, 119.624f, 3.45258f, 47591, 47591},
    {-1292.01f, 1289.27f, 119.669f, 5.33753f, 47591, 47591},
    {-1301.18f, 1086.43f, 118.987f, 3.20518f, 47591, 47591},
    {-1309.24f, 1133.72f, 118.849f, 0.75787f, 47591, 47591},
    {-1262.82f, 1135.22f, 119.079f, 0.75787f, 47591, 47591},
    {-1237.74f, 1127.75f, 118.984f, 0.75787f, 47591, 47591},
    {-1221.59f, 1151.49f, 118.894f, 0.75787f, 47591, 47591},
    {-1199.13f, 1165.23f, 119.172f, 5.05244f, 47591, 47591},
    {-1185.46f, 1152.88f, 119.073f, 5.05244f, 47591, 47591},
    {-1208.29f, 1105.06f, 120.684f, 5.05244f, 47591, 47591},
    {-1137.06f, 1076.74f, 120.011f, 1.22126f, 47591, 47591},
    {-1122.39f, 1110.79f, 119.003f, 0.80342f, 47591, 47591},
    {-1129.72f, 1140.90f, 119.770f, 1.04141f, 47591, 47591},
    {-1157.86f, 1190.91f, 121.026f, 1.77026f, 47591, 47591},
    {-1352.12f, 770.742f, 123.420f, 5.64049f, 47593, 47593},
    {-1235.60f, 982.568f, 119.362f, 0.09596f, 48069, 48066},  // Commander Zanoth / Sergeant Parker
};


#define TB_MAX_TROU_NPC 58
const BfTBObjectPosition TBTrouNPC[TB_MAX_KEEP_NPC] =
{
    // X        Y         Z         O          horde                       alliance
    {-1444.11f, 1153.93f, 125.186f, 1.99232f,  47550, 47550},
    {-1424.77f, 1141.60f, 128.321f, 6.12665f,  47550, 47550},
    {-1438.36f, 1117.79f, 123.420f, 0.01782f,  47550, 47550},
    {-1463.70f, 1097.98f, 122.102f, 4.10897f,  47550, 47550},
    {-1428.46f, 1097.73f, 120.254f, 1.17943f,  47550, 47550},
    {-1474.29f, 1088.73f, 132.811f, 5.78186f,  47550, 47550},
    {-1495.90f, 1104.55f, 130.659f, 2.53032f,  47550, 47550},
    {-1501.02f, 1177.14f, 135.637f, 2.97015f,  47552, 47552},
    {-1504.97f, 1175.97f, 136.077f, 1.41114f,  47552, 47552},
    {-1504.30f, 1181.69f, 135.600f, 5.09073f,  47552, 47552},
    {-1528.56f, 1206.33f, 136.433f, 0.44981f,  47552, 47552},
    {-1526.54f, 1210.76f, 136.116f, 4.90302f,  47552, 47552},
    {-1523.71f, 1206.63f, 136.326f, 2.74317f,  47552, 47552},
    {-1469.83f, 1169.92f, 133.389f, 0.47258f,  47552, 47552},
    {-1463.43f, 1172.94f, 133.854f, 3.84195f,  47552, 47552},
    {-1468.44f, 1176.32f, 134.387f, 4.74515f,  47552, 47552},
    {-1604.06f, 1135.56f, 95.0152f, 2.72668f,  47552, 47552},
    {-1619.41f, 1135.35f, 95.0152f, 5.87613f,  47552, 47552},
    {-1618.72f, 1117.88f, 95.5207f, 2.69134f,  47552, 47552},
    {-1636.40f, 1113.68f, 95.0387f, 5.91932f,  47552, 47552},
    {-1609.99f, 1098.55f, 95.6289f, 2.79736f,  47552, 47552},
    {-1615.02f, 1095.00f, 95.0161f, 2.07087f,  47552, 47552},
    {-1621.64f, 1103.73f, 95.0249f, 2.34183f,  47552, 47552},
    {-1643.82f, 1082.74f, 95.0154f, 1.04278f,  47552, 47552},
    {-1659.50f, 1093.35f, 95.0154f, 0.35948f,  47552, 47552},
    {-1645.70f, 1095.20f, 95.0154f, 0.88098f,  47552, 47552},
    {-1595.92f, 1151.51f, 95.1590f, 3.86157f,  47552, 47552},
    {-1658.59f, 1192.20f, 101.802f, 6.18635f,  47552, 47552},
    {-1659.44f, 1183.36f, 101.806f, 6.18635f,  47552, 47552},
    {-1633.26f, 1183.68f, 95.0148f, 6.26096f,  47552, 47552},
    {-1629.59f, 1190.71f, 95.0148f, 6.26096f,  47552, 47552},
    {-1606.91f, 1211.82f, 95.0158f, 4.14824f,  47552, 47552},
    {-1614.81f, 1212.00f, 95.0158f, 5.11428f,  47552, 47552},
    {-1615.63f, 1263.74f, 101.801f, 5.28706f,  47552, 47552},
    {-1603.50f, 1263.86f, 101.801f, 4.27390f,  47552, 47552},
    {-1603.86f, 1252.38f, 101.801f, 2.34968f,  47552, 47552},
    {-1615.82f, 1252.45f, 101.801f, 0.80244f,  47552, 47552},
    {-1576.87f, 1261.18f, 101.801f, 4.68623f,  47552, 47552},
    {-1577.19f, 1254.65f, 101.801f, 1.63103f,  47552, 47552},
    {-1520.11f, 1280.21f, 101.797f, 4.69880f,  47552, 47552},
    {-1523.72f, 1280.27f, 101.797f, 4.69880f,  47552, 47552},
    {-1521.17f, 1234.03f, 101.797f, 1.58862f,  47552, 47552},
    {-1524.12f, 1233.98f, 101.797f, 1.58470f,  47552, 47552},
    {-1513.40f, 1251.23f, 104.103f, 2.82170f,  47552, 47552},
    {-1513.34f, 1263.57f, 104.103f, 3.41860f,  47552, 47552},
    {-1536.7f,  1252.13f, 101.802f, 3.79951f,  47590, 47590},
    {-1545.84f, 1264.09f, 101.802f, 2.97170f,  47590, 47590},
    {-1612.05f, 1171.45f, 95.0146f, 4.12624f,  47590, 47590},
    {-1609.45f, 1148.99f, 95.1174f, 3.88198f,  47590, 47590},
    {-1613.52f, 1123.60f, 95.0154f, 4.96662f,  47590, 47590},
    {-1580.26f, 1140.19f, 95.1597f, 5.96800f,  47590, 47590},
    {-1577.37f, 1151.31f, 95.1714f, 0.91788f,  47590, 47590},
    {-1448.82f, 1133.07f, 123.422f, 2.16589f,  47590, 47590},
    {-1465.20f, 1099.35f, 122.387f, 3.53641f,  47590, 47590},
    {-1460.92f, 1090.65f, 120.897f, 4.32573f,  47590, 47590},
    {-1509.84f, 1257.28f, 104.763f, 3.13978f,  48036, 48036},
    {-1498.05f, 1256.14f, 133.945f, 2.20095f,  48036, 48036},
    {-1519.92f, 1281.45f, 133.585f, 4.56499f,  48036, 48036},
};

#define TB_MAX_DEPTH_NPC 41
const BfTBObjectPosition TBDeapthNPC[TB_MAX_DEPTH_NPC] =
{
    // X        Y         Z         O          horde                       alliance
    {-1225.61f, 858.577f, 119.856f, 4.78601f, 47531, 47531},
    {-1240.12f, 897.945f, 119.962f, 1.67583f, 47531, 47531},
    {-1217.47f, 915.890f, 119.728f, 1.67583f, 47531, 47531},
    {-1237.31f, 926.871f, 119.728f, 1.67583f, 47531, 47531},
    {-1176.18f, 844.604f, 123.089f, 5.43789f, 47531, 47531},
    {-1185.86f, 797.661f, 122.983f, 5.43789f, 47531, 47531},
    {-1227.69f, 791.049f, 119.433f, 3.87809f, 47531, 47531},
    {-1247.85f, 793.777f, 119.827f, 3.87809f, 47531, 47531},
    {-1292.42f, 794.127f, 119.833f, 3.56471f, 47531, 47531},
    {-1315.69f, 814.443f, 119.442f, 1.51718f, 47531, 47531},
    {-1304.45f, 855.005f, 120.856f, 1.51718f, 47531, 47531},
    {-1302.95f, 882.924f, 119.739f, 1.51718f, 47531, 47531},
    {-1273.68f, 888.015f, 121.084f, 0.05241f, 47531, 47531},
    {-1231.50f, 845.094f, 119.628f, 5.17085f, 47531, 47531},
    {-1234.76f, 809.308f, 119.692f, 4.59437f, 47531, 47531},
    {-1233.06f, 752.813f, 107.039f, 4.64149f, 47531, 47531},
    {-1247.49f, 762.188f, 96.4466f, 3.01179f, 47531, 47531},
    {-1258.15f, 779.274f, 90.8649f, 5.93347f, 47531, 47531},
    {-1238.88f, 816.774f, 120.196f, 1.23286f, 47534, 47534},
    {-1225.87f, 804.315f, 120.332f, 5.17949f, 47534, 47534},
    {-1239.96f, 790.035f, 119.422f, 4.34069f, 47534, 47534},
    {-1231.00f, 748.408f, 104.568f, 3.72022f, 47534, 47534},
    {-1254.07f, 747.459f, 98.8339f, 2.86414f, 47534, 47534},
    {-1249.63f, 789.392f, 89.5293f, 1.47398f, 47534, 47534},
    {-1258.69f, 797.665f, 89.5264f, 1.47398f, 47534, 47534},
    {-1253.81f, 816.938f, 89.5301f, 1.47398f, 47534, 47534},
    {-1239.47f, 826.927f, 89.5301f, 1.47398f, 47534, 47534},
    {-1238.83f, 841.504f, 89.5301f, 0.52836f, 47534, 47534},
    {-1228.60f, 837.749f, 91.0647f, 6.07955f, 47534, 47534},
    {-1219.50f, 831.950f, 91.0647f, 6.07955f, 47534, 47534},
    {-1203.54f, 841.026f, 91.1796f, 6.07955f, 47534, 47534},
    {-1243.01f, 850.984f, 89.5316f, 1.59886f, 47534, 47534},
    {-1256.18f, 824.391f, 89.5308f, 2.45101f, 47534, 47534},
    {-1273.68f, 835.365f, 89.5101f, 2.49814f, 47534, 47534},
    {-1282.76f, 838.237f, 89.5372f, 3.05577f, 47534, 47534},
    {-1297.50f, 829.270f, 89.6896f, 2.70862f, 47534, 47534},
    {-1309.44f, 837.615f, 89.5240f, 0.17492f, 47534, 47534},
    {-1303.74f, 853.948f, 89.5252f, 1.23207f, 47534, 47534},
    {-1291.47f, 855.175f, 89.5190f, 1.18574f, 47534, 47534},
    {-1281.70f, 853.414f, 89.5193f, 3.03535f, 47534, 47534},
    {-1295.99f, 879.974f, 89.5285f, 4.62970f, 47537, 47537},
};

#define TB_MAX_BLOCD_NPC 61
const BfTBObjectPosition TBBlocDNPC[TB_MAX_BLOCD_NPC] =
{
    // X        Y         Z         O          horde                       alliance
    {-995.482f, 1221.62f, 119.329f, 4.57627f, 47540, 47540},
    {-891.576f, 1154.34f, 117.567f, 1.26975f, 47540, 47540},
    {-893.775f, 1115.49f, 122.648f, 4.24641f, 47540, 47540},
    {-941.071f, 1087.30f, 123.152f, 0.72546f, 47540, 47540},
    {-965.630f, 1142.16f, 123.640f, 1.34593f, 47540, 47540},
    {-1024.94f, 1114.78f, 129.316f, 2.43371f, 47540, 47540},
    {-1045.75f, 1096.94f, 123.184f, 3.62752f, 47540, 47540},
    {-1080.88f, 1101.08f, 121.724f, 4.49931f, 47540, 47540},
    {-1061.62f, 1152.22f, 132.515f, 6.23504f, 47540, 47540},
    {-1044.97f, 1208.95f, 124.027f, 1.19514f, 47540, 47540},
    {-1082.51f, 1064.72f, 122.053f, 0.84800f, 47542, 47542},
    {-1085.74f, 1103.17f, 122.618f, 1.51952f, 47542, 47542},
    {-1093.99f, 1147.89f, 120.448f, 1.40171f, 47542, 47542},
    {-1086.38f, 1176.98f, 121.461f, 0.53934f, 47542, 47542},
    {-1064.45f, 1196.09f, 122.280f, 5.60674f, 47542, 47542},
    {-1013.49f, 1180.77f, 122.850f, 6.07091f, 47542, 47542},
    {-988.270f, 1182.65f, 120.095f, 0.10109f, 47542, 47542},
    {-1005.16f, 1154.03f, 122.318f, 3.90007f, 47542, 47542},
    {-1063.14f, 1111.03f, 124.121f, 4.72474f, 47542, 47542},
    {-1046.12f, 1082.57f, 123.343f, 4.72474f, 47542, 47542},
    {-1036.24f, 1073.09f, 125.422f, 4.72474f, 47542, 47542},
    {-1006.52f, 1024.95f, 125.798f, 4.72474f, 47542, 47542},
    {-1075.08f, 1023.32f, 120.036f, 2.75496f, 47542, 47542},
    {-1078.73f, 1021.78f, 120.553f, 2.75496f, 47542, 47542},
    {-1111.18f, 1035.00f, 120.385f, 1.28783f, 47542, 47542},
    {-1110.74f, 1131.87f, 120.942f, 1.13782f, 47542, 47542},
    {-1112.91f, 1163.72f, 120.942f, 1.13782f, 47542, 47542},
    {-1036.75f, 1157.75f, 123.421f, 5.18655f, 47542, 47542},
    {-971.761f, 1053.70f, 84.5823f, 1.49752f, 47542, 47542},
    {-975.982f, 1069.77f, 84.5814f, 1.42683f, 47542, 47542},
    {-965.467f, 1091.92f, 85.8183f, 1.42683f, 47542, 47542},
    {-965.199f, 1116.49f, 84.5834f, 2.27113f, 47542, 47542},
    {-931.258f, 1123.53f, 84.5597f, 2.71881f, 47542, 47542},
    {-947.891f, 1130.65f, 84.5823f, 2.59393f, 47542, 47542},
    {-989.592f, 1212.18f, 84.5820f, 6.07325f, 47542, 47542},
    {-943.286f, 1210.99f, 84.5819f, 3.34399f, 47542, 47542},
    {-968.525f, 1186.84f, 84.5819f, 4.76163f, 47542, 47542},
    {-969.996f, 1147.28f, 84.5824f, 4.67524f, 47542, 47542},
    {-1021.69f, 1132.70f, 91.3227f, 3.10837f, 47542, 47542},
    {-1025.02f, 1130.14f, 91.3227f, 3.10837f, 47542, 47542},
    {-1043.07f, 1130.74f, 94.0270f, 3.10837f, 47542, 47542},
    {-1055.50f, 1151.01f, 106.325f, 3.21361f, 47542, 47542},
    {-974.183f, 1018.92f, 84.8929f, 1.52893f, 47544, 47544},
    {-1054.22f, 1095.44f, 123.421f, 1.27917f, 47549, 47549},
    {-1022.27f, 1162.36f, 123.329f, 0.96580f, 47549, 47549},
    {-1029.22f, 1188.42f, 123.422f, 0.96580f, 47549, 47549},
    {-986.759f, 1209.20f, 119.459f, 6.00334f, 47549, 47549},
    {-979.825f, 1219.18f, 119.119f, 6.00334f, 47549, 47549},
    {-922.692f, 1213.14f, 117.454f, 6.00334f, 47549, 47549},
    {-894.055f, 1234.37f, 117.052f, 5.41429f, 47549, 47549},
    {-874.262f, 1227.55f, 113.735f, 5.41429f, 47549, 47549},
    {-904.387f, 1185.26f, 116.948f, 2.98113f, 47549, 47549},
    {-947.231f, 1168.63f, 119.662f, 2.98113f, 47549, 47549},
    {-1072.14f, 1212.86f, 124.715f, 2.98113f, 47549, 47549},
    {-1106.65f, 1166.13f, 119.000f, 5.02474f, 47549, 47549},
    {-1096.35f, 1137.88f, 119.849f, 4.48281f, 47549, 47549},
    {-1090.55f, 1122.53f, 122.217f, 4.48281f, 47549, 47549},
    {-1123.90f, 1088.07f, 120.670f, 3.72883f, 47549, 47549},
    {-1123.99f, 1060.34f, 123.973f, 3.72883f, 47549, 47549},
    {-1140.82f, 1042.83f, 120.150f, 4.43333f, 47549, 47549},
    {-1121.22f, 1015.01f, 120.613f, 6.25231f, 47549, 47549},
};


#define TB_MAX_DYNAMIC_QUEST_GIVER 3
const BfTBObjectPosition TBQuestNPC[TB_MAX_BLOCD_NPC] =
{
    // X        Y         Z         O          horde                       alliance
    {-1236.46f, 974.88f, 119.61f, 5.87627f, 48062, 48074},
    {-1236.46f, 974.88f, 119.61f, 5.87627f, 48071, 48039},
    {-1236.46f, 974.88f, 119.61f, 5.87627f, 48070, 48061},
};
//*********************************************************
//*****************WorkShop Data & Element*****************
//*********************************************************

struct BfTBCapturePointDataBase
{
    uint32 entry;
    uint32 type;
    uint32 nameid;
    BfTBObjectPosition CapturePoint;
    uint8 nbgob;
    BfTBObjectPosition GameObjectData[2];
};

// TODO Add flags.
#define TB_MAX_WORKSHOP  3
const BfTBCapturePointDataBase TBCapturePointDataBase[TB_MAX_WORKSHOP]=
{
    // Ironclad
    {192031, BATTLEFIELD_TB_FORT_IRONGLAD, BATTLEFIELD_TB_TEXT_IRONCLAD_GARRISON_NAME,
        {-877.823f, 980.493f, 121.441f, 3.05341f, BATTLEFIELD_TB_GAMEOBJECT_CAPTURE_NORTH, BATTLEFIELD_TB_GAMEOBJECT_CAPTURE_NORTH},
        2,
        //gameobject
        {
            {-989.841f, 964.901f, 121.564f, 2.88383f, BATTLEFIELD_TB_GAMEOBJECT_BANNER_HORDE, BATTLEFIELD_TB_GAMEOBJECT_BANNER_ALLY},
            {-989.134f, 993.65f, 121.674f, 3.40062f, BATTLEFIELD_TB_GAMEOBJECT_BANNER_HORDE, BATTLEFIELD_TB_GAMEOBJECT_BANNER_ALLY},
        }
    },

    // Warden's
    {192030, BATTLEFIELD_TB_FORT_WARDENS_VIGIL, BATTLEFIELD_TB_TEXT_WARDENS_VIGIL_NAME,
        {-1492.85f, 1310.62f, 152.962f, 5.4646f, BATTLEFIELD_TB_GAMEOBJECT_CAPTURE_WEST, BATTLEFIELD_TB_GAMEOBJECT_CAPTURE_WEST},
        2,
        //gameobject
        {
            {-1445.56f, 1236.89f, 133.822f, 5.92265f, BATTLEFIELD_TB_GAMEOBJECT_BANNER_HORDE, BATTLEFIELD_TB_GAMEOBJECT_BANNER_ALLY},
            {-1420.72f, 1262.17f, 133.659f, 5.35717f, BATTLEFIELD_TB_GAMEOBJECT_BANNER_HORDE, BATTLEFIELD_TB_GAMEOBJECT_BANNER_ALLY},
        }
    },
    // Slagworks
    {192033, BATTLEFIELD_TB_FORT_SLAGWORKS, BATTLEFIELD_TB_TEXT_SLAGWORKS_NAME,
        {-1438.86f, 685.013f, 123.422f, 0.795405f, BATTLEFIELD_TB_GAMEOBJECT_CAPTURE_EAST, BATTLEFIELD_TB_GAMEOBJECT_CAPTURE_EAST},
        2,
        //gameobject
        {
            {-1378.03f, 726.273f, 124.3f, 0.906316f, BATTLEFIELD_TB_GAMEOBJECT_BANNER_HORDE, BATTLEFIELD_TB_GAMEOBJECT_BANNER_ALLY},
            {-1402.82f, 747.984f, 123.332f, 0.252082f, BATTLEFIELD_TB_GAMEOBJECT_BANNER_HORDE, BATTLEFIELD_TB_GAMEOBJECT_BANNER_ALLY},
        }
    },
};

/********************************************************************
*                Structs using for Building, Graveyard, Workshop      *
********************************************************************/

// Structure for different building witch can be destroy during battle
struct BfTBGameObjectBuilding
{
    BfTBGameObjectBuilding(BattlefieldTB* TB)
    {
        m_TB = TB;
        m_Team = 0;
        m_Build = NULL;
        m_Type = 0;
        m_WorldState = 0;
        m_State = 0;
        m_NameId = 0;
    }
    // Team witch control this point
    uint8 m_Team;

    // TB object
    BattlefieldTB* m_TB;

    // Linked gameobject
    GameObject* m_Build;

    // eTBGameObjectBuildingType
    uint32 m_Type;

    // WorldState
    uint32 m_WorldState;

    // eTBGameObjectState
    uint32 m_State;

    // Name id for warning text
    uint32 m_NameId;

    // GameObject associate
    TBGameObjectSet m_GameObjectList[2];

    // Creature associate
    GuidSet m_CreatureBottomList[2];
    GuidSet m_CreatureTopList[2];
    GuidSet m_TurretBottomList;
    GuidSet m_TurretTopList;

    bool isDamage;
    bool isDestroy;

    void changeWorldState(uint32 newState)
    {
        m_TB->SendUpdateWorldState(m_WorldState, 0);
        m_State = newState;
        ReinitWorldStateFromNameID();
        m_TB->SendUpdateWorldState(m_WorldState, 1);
    }

    void ReinitWorldStateFromNameID()
    {
        switch (m_NameId)
        {
            case BATTLEFIELD_TB_OBJECTSTATE_NONE:
            {
                m_WorldState = TB_WS_WEST_INTACT_NEUTRAL;
                break;
            }
            case BATTLEFIELD_TB_TEXT_TOWER_NAME_WEST:
            {
                switch (m_State)
                {
                    case BATTLEFIELD_TB_OBJECTSTATE_NONE:
                        m_WorldState = TB_WS_WEST_INTACT_NEUTRAL;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_NEUTRAL_INTACT:
                        m_WorldState = TB_WS_WEST_INTACT_NEUTRAL;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_NEUTRAL_DAMAGE:
                        m_WorldState = TB_WS_WEST_DAMAGED_NEUTRAL;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_NEUTRAL_DESTROY:
                        m_WorldState = TB_WS_WEST_DESTROYED_NEUTRAL;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_HORDE_INTACT:
                        m_WorldState = TB_WS_WEST_INTACT_HORDE;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_HORDE_DAMAGE:
                        m_WorldState = TB_WS_WEST_DAMAGED_HORDE;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_HORDE_DESTROY:
                        m_WorldState = TB_WS_WEST_DESTROYED_NEUTRAL;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_INTACT:
                        m_WorldState = TB_WS_WEST_INTACT_ALLIANCE;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_DAMAGE:
                        m_WorldState = TB_WS_WEST_DAMAGED_ALLIANCE;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_DESTROY:
                        m_WorldState = TB_WS_WEST_DESTROYED_NEUTRAL;
                        break;
                }
                break;
            }
            case BATTLEFIELD_TB_TEXT_TOWER_NAME_SOUTH:
            {
                switch (m_State)
                {
                    case BATTLEFIELD_TB_OBJECTSTATE_NONE:
                        m_WorldState = TB_WS_SOUTH_INTACT_NEUTRAL;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_NEUTRAL_INTACT:
                        m_WorldState = TB_WS_SOUTH_INTACT_NEUTRAL;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_NEUTRAL_DAMAGE:
                        m_WorldState = TB_WS_SOUTH_DAMAGED_NEUTRAL;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_NEUTRAL_DESTROY:
                        m_WorldState = TB_WS_SOUTH_DESTROYED_NEUTRAL;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_HORDE_INTACT:
                        m_WorldState = TB_WS_SOUTH_INTACT_HORDE;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_HORDE_DAMAGE:
                        m_WorldState = TB_WS_SOUTH_DAMAGED_HORDE;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_HORDE_DESTROY:
                        m_WorldState = TB_WS_SOUTH_DESTROYED_NEUTRAL;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_INTACT:
                        m_WorldState = TB_WS_SOUTH_INTACT_ALLIANCE;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_DAMAGE:
                        m_WorldState = TB_WS_SOUTH_DAMAGED_ALLIANCE;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_DESTROY:
                        m_WorldState = TB_WS_SOUTH_DESTROYED_NEUTRAL;
                        break;
                }
                break;
            }
            case BATTLEFIELD_TB_TEXT_TOWER_NAME_EAST:
            {
                switch (m_State)
                {
                    case BATTLEFIELD_TB_OBJECTSTATE_NONE:
                        m_WorldState = TB_WS_EAST_INTACT_NEUTRAL;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_NEUTRAL_INTACT:
                        m_WorldState = TB_WS_EAST_INTACT_NEUTRAL;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_NEUTRAL_DAMAGE:
                        m_WorldState = TB_WS_EAST_DAMAGED_NEUTRAL;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_NEUTRAL_DESTROY:
                        m_WorldState = TB_WS_EAST_DESTROYED_NEUTRAL;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_HORDE_INTACT:
                        m_WorldState = TB_WS_EAST_INTACT_HORDE;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_HORDE_DAMAGE:
                        m_WorldState = TB_WS_EAST_DAMAGED_HORDE;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_HORDE_DESTROY:
                        m_WorldState = TB_WS_EAST_DESTROYED_NEUTRAL;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_INTACT:
                        m_WorldState = TB_WS_EAST_INTACT_ALLIANCE;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_DAMAGE:
                        m_WorldState = TB_WS_EAST_DAMAGED_ALLIANCE;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_DESTROY:
                        m_WorldState = TB_WS_EAST_DESTROYED_NEUTRAL;
                        break;
                }
                break;
            }
        }
    }

    void Rebuild()
    {
        isDamage = false;
        isDestroy = false;
        m_Team = m_TB->GetDefenderTeam();
        // Rebuild gameobject
        m_Build->SetDestructibleState(GO_DESTRUCTIBLE_REBUILDING, NULL, true);
        // Updating worldstate
        changeWorldState(m_Team == TEAM_HORDE ? BATTLEFIELD_TB_OBJECTSTATE_HORDE_INTACT : BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_INTACT);
        // UpdateCreatureAndGo();
        m_Build->SetUInt32Value(GAMEOBJECT_FACTION, m_TB->GetAttackerTeam() == TEAM_HORDE ? 12 : 85);
    }

    // Called when associate gameobject is damaged
    void Damaged()
    {
        isDamage = true;
        // Updating worldstate
        changeWorldState(m_TB->GetDefenderTeam() == TEAM_HORDE ? BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_DAMAGE : BATTLEFIELD_TB_OBJECTSTATE_HORDE_DAMAGE);
        // Send warning message
        if (m_NameId)
            m_TB->SendWarningToAllInWar(BATTLEFIELD_TB_TEXT_TOWER_DAMAGE, sObjectMgr->GetTrinityStringForDBCLocale(m_NameId));
    }

    // Called when associate gameobject is destroy
    void Destroyed()
    {
        isDamage = false;
        isDestroy = true;
        // Updating worldstate
        changeWorldState(m_TB->GetDefenderTeam() == TEAM_HORDE ? BATTLEFIELD_TB_OBJECTSTATE_HORDE_DESTROY : BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_DESTROY);
        // Warning
        if (m_NameId)
            m_TB->SendWarningToAllInWar(BATTLEFIELD_TB_TEXT_TOWER_DESTROY, sObjectMgr->GetTrinityStringForDBCLocale(m_NameId));

        switch (m_Type)
        {
            // If destroy tower, inform TB script of it (using for reward calculation and event with south towers)
            case BATTLEFIELD_TB_OBJECTTYPE_TOWER:
                m_TB->AddBrokenTower(TeamId(m_Team));
                break;
        }
    }

    void Init(GameObject* go, uint32 type, uint32 nameid)
    {
        isDamage = false;
        isDestroy = false;
        // GameObject associate to object
        m_Build = go;
        // Type of building (WALL/TOWER/DOOR)
        m_Type = type;
        // NameId for Warning text
        m_NameId = nameid;
        switch (m_Type)
        {
            case BATTLEFIELD_TB_OBJECTTYPE_TOWER:
                m_Team = m_TB->GetDefenderTeam();//Tower in south are for attacker
                break;
            default:
                m_Team = TEAM_NEUTRAL;
                break;
        }
        changeWorldState(m_TB->GetDefenderTeam() == TEAM_HORDE ? BATTLEFIELD_TB_OBJECTSTATE_HORDE_INTACT : BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_INTACT);
        switch (m_State)
        {
            case BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_INTACT:
            case BATTLEFIELD_TB_OBJECTSTATE_HORDE_INTACT:
                if (m_Build)
                    m_Build->SetDestructibleState(GO_DESTRUCTIBLE_REBUILDING, NULL, true);
                break;
            case BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_DESTROY:
            case BATTLEFIELD_TB_OBJECTSTATE_HORDE_DESTROY:
                if (m_Build)
                {
                    m_Build->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_DAMAGED);
                    m_Build->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_DESTROYED);
                    m_Build->SetUInt32Value(GAMEOBJECT_DISPLAYID, m_Build->GetGOInfo()->building.destroyedDisplayId);
                }
                break;
            case BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_DAMAGE:
            case BATTLEFIELD_TB_OBJECTSTATE_HORDE_DAMAGE:
                if (m_Build)
                {
                    m_Build->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_DAMAGED);
                    m_Build->SetUInt32Value(GAMEOBJECT_DISPLAYID, m_Build->GetGOInfo()->building.damagedDisplayId);
                }
                break;
        }
    }

    void UpdateCreatureAndGo()
    {
        for (GuidSet::const_iterator itr = m_CreatureTopList[m_TB->GetDefenderTeam()].begin(); itr != m_CreatureTopList[m_TB->GetDefenderTeam()].end(); ++itr)
              if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
                if (Creature* creature = unit->ToCreature())
                    m_TB->HideNpc(creature);

        for (GuidSet::const_iterator itr = m_CreatureTopList[m_TB->GetAttackerTeam()].begin(); itr != m_CreatureTopList[m_TB->GetAttackerTeam()].end(); ++itr)
            if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
                if (Creature* creature = unit->ToCreature())
                    m_TB->ShowNpc(creature, true);

        for (GuidSet::const_iterator itr = m_CreatureBottomList[m_TB->GetDefenderTeam()].begin(); itr != m_CreatureBottomList[m_TB->GetDefenderTeam()].end(); ++itr)
            if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
                if (Creature* creature = unit->ToCreature())
                    m_TB->HideNpc(creature);

        for (GuidSet::const_iterator itr = m_CreatureBottomList[m_TB->GetAttackerTeam()].begin(); itr != m_CreatureBottomList[m_TB->GetAttackerTeam()].end(); ++itr)
            if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
                if (Creature* creature = unit->ToCreature())
                    m_TB->ShowNpc(creature, true);

        for (TBGameObjectSet::const_iterator itr = m_GameObjectList[m_TB->GetDefenderTeam()].begin(); itr != m_GameObjectList[m_TB->GetDefenderTeam()].end(); ++itr)
            (*itr)->SetRespawnTime(RESPAWN_ONE_DAY);

        for (TBGameObjectSet::const_iterator itr = m_GameObjectList[m_TB->GetAttackerTeam()].begin(); itr != m_GameObjectList[m_TB->GetAttackerTeam()].end(); ++itr)
            (*itr)->SetRespawnTime(RESPAWN_IMMEDIATELY);
    }

    void Save()
    {
        sWorld->setWorldState(m_WorldState, 1);
    }

    bool IsDamage()
    {
        return isDamage;
    }

    bool IsDestroy()
    {
        return isDestroy;
    }

    bool IsIntact()
    {
        return !IsDamage() && !IsDestroy();
    }
};

// Structure for the 6 workshop
struct BfTBWorkShopData
{
    BattlefieldTB* m_TB;                      // Object du joug
    GameObject* m_Build;
    uint32 m_Type;
    uint32 m_State;                           // For worldstate
    uint32 m_TeamControl;                     // Team witch control the workshop
    GuidSet m_CreatureOnPoint[2];             // Contain all Creature associate to this point
    TBGameObjectSet m_GameObjectOnPoint[2];   // Contain all Gameobject associate to this point
    uint32 m_NameId;                          // Id of skyfire_string witch contain name of this node, using for alert message
    uint32 m_WorldState;

    BfTBWorkShopData(BattlefieldTB* TB)
    {
        m_TB            = TB;
        m_Build         = NULL;
        m_Type          = 0;
        m_State         = 0;
        m_TeamControl   = 0;
        m_NameId        = 0;
        m_WorldState = 0;
    }

    void changeWorldState(uint32 newState)
    {
        m_TB->SendUpdateWorldState(m_WorldState, 0);
        m_State = newState;
        ReinitWorldStateFromNameID();
        m_TB->SendUpdateWorldState(m_WorldState, 1);
    }

    void ReinitWorldStateFromNameID()
    {
        switch (m_NameId)
        {
            case BATTLEFIELD_TB_TEXT_IRONCLAD_GARRISON_NAME:
            {
                switch (m_State)
                {
                    case BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_INTACT:
                        m_WorldState = TB_WS_GARRISON_ALLIANCE_CONTROLLED;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_HORDE_INTACT:
                        m_WorldState = TB_WS_GARRISON_HORDE_CONTROLLED;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_NEUTRAL_INTACT:
                        m_WorldState = (m_TeamControl == TEAM_HORDE ? TB_WS_GARRISON_ALLIANCE_CAPTURING : TB_WS_GARRISON_HORDE_CAPTURING);
                        break;
                }
                break;
            }
            case BATTLEFIELD_TB_TEXT_SLAGWORKS_NAME:
            {
                switch (m_State)
                {
                    case BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_INTACT:
                        m_WorldState = TB_WS_SLAGWORKS_ALLIANCE_CONTROLLED;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_HORDE_INTACT:
                        m_WorldState = TB_WS_SLAGWORKS_HORDE_CONTROLLED;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_NEUTRAL_INTACT:
                        m_WorldState = (m_TeamControl == TEAM_HORDE ? TB_WS_SLAGWORKS_ALLIANCE_CAPTURING : TB_WS_SLAGWORKS_HORDE_CAPTURING);
                        break;
                }
                break;
            }
            case BATTLEFIELD_TB_TEXT_WARDENS_VIGIL_NAME:
            {
                switch (m_State)
                {
                    case BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_INTACT:
                        m_WorldState = TB_WS_VIGIL_ALLIANCE_CONTROLLED;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_HORDE_INTACT:
                        m_WorldState = TB_WS_VIGIL_HORDE_CONTROLLED;
                        break;
                    case BATTLEFIELD_TB_OBJECTSTATE_NEUTRAL_INTACT:
                        m_WorldState = (m_TeamControl == TEAM_HORDE ? TB_WS_VIGIL_ALLIANCE_CAPTURING : TB_WS_VIGIL_HORDE_CAPTURING);
                        break;
                }
                break;
            }
        }
    }

    // Spawning associate creature and store them
    void AddCreature(BfTBObjectPosition obj)
    {
        if (Creature* creature = m_TB->SpawnCreature(obj.entryh, obj.x, obj.y, obj.z, obj.o, TEAM_HORDE))
            m_CreatureOnPoint[TEAM_HORDE].insert(creature->GetGUID());

        if (Creature* creature = m_TB->SpawnCreature(obj.entrya, obj.x, obj.y, obj.z, obj.o, TEAM_ALLIANCE))
            m_CreatureOnPoint[TEAM_ALLIANCE].insert(creature->GetGUID());
    }

    // Spawning Associate gameobject and store them
    void AddGameObject(BfTBObjectPosition obj)
    {
        if (GameObject* gameobject = m_TB->SpawnGameObject(obj.entryh, obj.x, obj.y, obj.z, obj.o, 732))
            m_GameObjectOnPoint[TEAM_HORDE].insert(gameobject);

        if (GameObject* gameobject = m_TB->SpawnGameObject(obj.entrya, obj.x, obj.y, obj.z, obj.o, 732))
            m_GameObjectOnPoint[TEAM_ALLIANCE].insert(gameobject);
    }

    // Init method, setup variable
    void Init(uint32 type, uint32 nameid, uint8 team)
    {
        m_Type        = type;
        m_NameId      = nameid;
        changeWorldState(team == TEAM_ALLIANCE ? BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_INTACT : BATTLEFIELD_TB_OBJECTSTATE_HORDE_INTACT);
    }

    void capturePoint(uint8 team)
    {
        switch (team)
        {
            case TEAM_NEUTRAL:
                if (m_TeamControl == m_TB->GetAttackerTeam())
                    if (m_TB->GetData(BATTLEFIELD_TB_DATA_CAPTURED_FORT_ATT) > 0)
                        m_TB->UpdateData(BATTLEFIELD_TB_DATA_CAPTURED_FORT_ATT, -1);
                m_TB->SendUpdateWorldState(TB_WS_BUILDINGS_CAPTURED, m_TB->GetData(BATTLEFIELD_TB_DATA_CAPTURED_FORT_ATT));
                break;
            case TEAM_ALLIANCE:
            case TEAM_HORDE:
                if (team == m_TB->GetAttackerTeam())
                    m_TB->UpdateData(BATTLEFIELD_TB_DATA_CAPTURED_FORT_ATT, 1);
                m_TB->SendUpdateWorldState(TB_WS_BUILDINGS_CAPTURED, m_TB->GetData(BATTLEFIELD_TB_DATA_CAPTURED_FORT_ATT));
                break;
        }
        if (m_TB->GetData(BATTLEFIELD_TB_DATA_CAPTURED_FORT_ATT) == 3)
            m_TB->EndBattle(false);
    }

    void ChangeGraveyardControl(TeamId team)
    {
        // Found associate graveyard and update it
        int type = -1;
        switch (m_Type)
        {
            case BATTLEFIELD_TB_FORT_IRONGLAD:
                type = 2;
                break;
            case BATTLEFIELD_TB_FORT_WARDENS_VIGIL:
                type = 1;
                break;
            case BATTLEFIELD_TB_FORT_SLAGWORKS:
                type = 3;
                break;
            default:
                type = -1;
                break;
        }
        if (type >= 0)
            if (m_TB && m_TB->GetGraveyardById(type))
                m_TB->GetGraveyardById(type)->GiveControlTo(team);
    }

    // Called on change faction in CapturePoint class
    void GiveControlTo(uint8 team, bool init /* for first call in setup*/)
    {
        capturePoint(team);
        switch (team)
        {
            case TEAM_NEUTRAL:
            {
                // Send warning message to all player for inform a faction attack a workshop
                changeWorldState(BATTLEFIELD_TB_OBJECTSTATE_NEUTRAL_INTACT);
                if (!init)
                    m_TB->SendWarningToAllInWar(BATTLEFIELD_TB_TEXT_FORT_ATTACK, sObjectMgr->GetTrinityStringForDBCLocale(m_NameId),
                                                sObjectMgr->GetTrinityStringForDBCLocale(m_TeamControl ? BATTLEFIELD_TB_TEXT_ALLIANCE : BATTLEFIELD_TB_TEXT_HORDE));
                ChangeGraveyardControl(TEAM_NEUTRAL);
                break;
            }
            case TEAM_ALLIANCE:
            {
                // Show Alliance gameobject
                for (TBGameObjectSet::const_iterator itr = m_GameObjectOnPoint[TEAM_ALLIANCE].begin(); itr != m_GameObjectOnPoint[TEAM_ALLIANCE].end(); ++itr)
                    (*itr)->SetRespawnTime(RESPAWN_IMMEDIATELY);
                // Hide Horde gameobject
                for (TBGameObjectSet::const_iterator itr = m_GameObjectOnPoint[TEAM_HORDE].begin(); itr != m_GameObjectOnPoint[TEAM_HORDE].end(); ++itr)
                    (*itr)->SetRespawnTime(RESPAWN_ONE_DAY);
                // Updating worldstate
                changeWorldState(BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_INTACT);
                // Warning message
                if (!init)
                    m_TB->SendWarningToAllInWar(BATTLEFIELD_TB_TEXT_FORT_TAKEN, sObjectMgr->GetTrinityStringForDBCLocale(m_NameId),
                                                sObjectMgr->GetTrinityStringForDBCLocale(BATTLEFIELD_TB_TEXT_ALLIANCE));
                ChangeGraveyardControl(TEAM_ALLIANCE);
                break;
            }
            case TEAM_HORDE:
            {
                // Hide Alliance gameobject
                for (TBGameObjectSet::const_iterator itr = m_GameObjectOnPoint[TEAM_ALLIANCE].begin(); itr != m_GameObjectOnPoint[TEAM_ALLIANCE].end(); ++itr)
                    (*itr)->SetRespawnTime(RESPAWN_ONE_DAY);
                // Show Horde gameobject
                for (TBGameObjectSet::const_iterator itr = m_GameObjectOnPoint[TEAM_HORDE].begin(); itr != m_GameObjectOnPoint[TEAM_HORDE].end(); ++itr)
                    (*itr)->SetRespawnTime(RESPAWN_IMMEDIATELY);
                // Update worlstate
                changeWorldState(BATTLEFIELD_TB_OBJECTSTATE_HORDE_INTACT);
                // Warning message
                if (!init)
                    m_TB->SendWarningToAllInWar(BATTLEFIELD_TB_TEXT_FORT_TAKEN, sObjectMgr->GetTrinityStringForDBCLocale(m_NameId),
                                                sObjectMgr->GetTrinityStringForDBCLocale(BATTLEFIELD_TB_TEXT_HORDE));
                // Update graveyard control
                ChangeGraveyardControl(TEAM_HORDE);
                break;
            }
        }
        m_TeamControl = team;
    }

    // Called on change faction in CapturePoint class
    void ReinitControl(uint8 team)
    {
        switch (team)
        {
            case TEAM_ALLIANCE:
            {
                // Show Alliance gameobject
                for (TBGameObjectSet::const_iterator itr = m_GameObjectOnPoint[TEAM_ALLIANCE].begin(); itr != m_GameObjectOnPoint[TEAM_ALLIANCE].end(); ++itr)
                    (*itr)->SetRespawnTime(RESPAWN_IMMEDIATELY);
                // Hide Horde gameobject
                for (TBGameObjectSet::const_iterator itr = m_GameObjectOnPoint[TEAM_HORDE].begin(); itr != m_GameObjectOnPoint[TEAM_HORDE].end(); ++itr)
                    (*itr)->SetRespawnTime(RESPAWN_ONE_DAY);
                // Updating worldstate
                changeWorldState(BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_INTACT);
                ChangeGraveyardControl(TEAM_ALLIANCE);
                break;
            }
            case TEAM_HORDE:
            {
                // Hide Alliance gameobject
                for (TBGameObjectSet::const_iterator itr = m_GameObjectOnPoint[TEAM_ALLIANCE].begin(); itr != m_GameObjectOnPoint[TEAM_ALLIANCE].end(); ++itr)
                    (*itr)->SetRespawnTime(RESPAWN_ONE_DAY);
                // Show Horde gameobject
                for (TBGameObjectSet::const_iterator itr = m_GameObjectOnPoint[TEAM_HORDE].begin(); itr != m_GameObjectOnPoint[TEAM_HORDE].end(); ++itr)
                    (*itr)->SetRespawnTime(RESPAWN_IMMEDIATELY);
                // Update worlstate
                changeWorldState(BATTLEFIELD_TB_OBJECTSTATE_HORDE_INTACT);
                // Update graveyard control
                ChangeGraveyardControl(TEAM_HORDE);
                break;
            }
        }
        m_TeamControl = team;
    }

    void UpdateGraveYardAndWorkshop()
    {
        if (m_Type < BATTLEFIELD_TB_FORT_MAX)
            ChangeGraveyardControl((TeamId)m_TeamControl);
        else
            ReinitControl(m_TB->GetDefenderTeam());
        m_TB->SetData(BATTLEFIELD_TB_DATA_CAPTURED_FORT_ATT, 0);
    }

    void Save()
    {
        sWorld->setWorldState(m_WorldState, 1);
    }
};

#endif
