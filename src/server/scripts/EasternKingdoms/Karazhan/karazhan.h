/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

#ifndef DEF_KARAZHAN_H
#define DEF_KARAZHAN_H

#define DataHeader "KZ"

enum DataTypes
{
    TYPE_ATTUMEN                    = 1,
    TYPE_MOROES                     = 2,
    TYPE_MAIDEN                     = 3,
    TYPE_OPTIONAL_BOSS              = 4,
    TYPE_OPERA                      = 5,
    TYPE_CURATOR                    = 6,
    TYPE_ARAN                       = 7,
    TYPE_TERESTIAN                  = 8,
    TYPE_NETHERSPITE                = 9,
    TYPE_CHESS                      = 10,
    TYPE_MALCHEZZAR                 = 11,
    TYPE_NIGHTBANE                  = 12,

    DATA_OPERA_PERFORMANCE          = 13,
    DATA_OPERA_OZ_DEATHCOUNT        = 14,

    DATA_KILREK                     = 15,
    DATA_TERESTIAN                  = 16,
    DATA_MOROES                     = 17,
    DATA_GO_CURTAINS                = 18,
    DATA_GO_STAGEDOORLEFT           = 19,
    DATA_GO_STAGEDOORRIGHT          = 20,
    DATA_GO_LIBRARY_DOOR            = 21,
    DATA_GO_MASSIVE_DOOR            = 22,
    DATA_GO_NETHER_DOOR             = 23,
    DATA_GO_GAME_DOOR               = 24,
    DATA_GO_GAME_EXIT_DOOR          = 25,

    DATA_IMAGE_OF_MEDIVH            = 26,
    DATA_MASTERS_TERRACE_DOOR_1     = 27,
    DATA_MASTERS_TERRACE_DOOR_2     = 28,
    DATA_GO_SIDE_ENTRANCE_DOOR      = 29,

    DATA_TEAM_IN_INSTANCE           = 30,
    DATA_CHESS_DAMAGE               = 31,
};

enum OperaEvents
{
    EVENT_OZ                        = 1,
    EVENT_HOOD                      = 2,
    EVENT_RAJ                       = 3
};

enum eNpcsAndGos
{
    NPC_NIGHTBANE                   = 17225,
    NPC_NIGHTBANE_HELPER            = 17260,
    NPC_NETHERSPITE                 = 15689,
    NPC_ECHO_MEDIVH                 = 16816,
    NPC_INVISIBLE_STALKER           = 22519,                    // placeholder for dead chess npcs
    NPC_CHESS_STATUS_BAR            = 22520,                    // npc that controlls the transformation of dead pieces
    NPC_CHESS_VICTORY_CONTROLLER    = 22524,

    // Chess event
    NPC_ORC_GRUNT                   = 17469,                    // pawn
    NPC_ORC_WOLF                    = 21748,                    // knight
    NPC_ORC_WARLOCK                 = 21750,                    // queen
    NPC_ORC_NECROLYTE               = 21747,                    // bishop
    NPC_SUMMONED_DAEMON             = 21726,                    // rook
    NPC_WARCHIEF_BLACKHAND          = 21752,                    // king
    NPC_HUMAN_FOOTMAN               = 17211,                    // pawn
    NPC_HUMAN_CHARGER               = 21664,                    // knight
    NPC_HUMAN_CONJURER              = 21683,                    // queen
    NPC_HUMAN_CLERIC                = 21682,                    // bishop
    NPC_CONJURED_WATER_ELEMENTAL    = 21160,                    // rook
    NPC_KING_LLANE                  = 21684,                    // king
    NPC_SQUARE_WHITE                = 17208,                    // chess white square
    NPC_SQUARE_BLACK                = 17305,                    // chess black square

    GO_STAGE_CURTAIN                = 183932,
    GO_STAGE_DOOR_LEFT              = 184278,
    GO_STAGE_DOOR_RIGHT             = 184279,
    GO_PRIVATE_LIBRARY_DOOR         = 184517,
    GO_MASSIVE_DOOR                 = 185521,
    GO_GAMESMANS_HALL_DOOR          = 184276,
    GO_GAMESMANS_HALL_EXIT_DOOR     = 184277,
    GO_NETHERSPACE_DOOR             = 185134,
    GO_SIDE_ENTRANCE_DOOR           = 184275,
    GO_DUST_COVERED_CHEST           = 185119,
    GO_MASTERS_TERRACE_DOOR_1       = 184274,
    GO_MASTERS_TERRACE_DOOR_2       = 184280,
};

enum eSpells
{
    // Chess event spells
    SPELL_CLEAR_BOARD               = 37366,                    // spell cast to clear the board at the end of the event
    SPELL_GAME_IN_SESSION           = 39331,                    // debuff on players received while the game is in session
    SPELL_FORCE_KILL_BUNNY          = 45260,                    // triggers 45259
    SPELL_GAME_OVER                 = 39401,                    // cast by Medivh on game end
    SPELL_VICTORY_VISUAL            = 39395,                    // cast by the Victory controller on game end

};

enum Miscs
{
    FACTION_ID_CHESS_HORDE          = 1689,
    FACTION_ID_CHESS_ALLIANCE       = 1690
};

enum Actions
{
    AI_EVENT_CUSTOM_A = 1,
    AI_EVENT_CUSTOM_B = 2,
};


#define MAX_ENCOUNTER      12

class instance_karazhan : public InstanceMapScript
{
public:
    instance_karazhan() : InstanceMapScript("instance_karazhan", 532) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_karazhan_InstanceMapScript(map);
    }

    struct instance_karazhan_InstanceMapScript : public InstanceScript
    {
        instance_karazhan_InstanceMapScript(Map* map) : InstanceScript(map) {}

        void Initialize();
        void OnPlayerEnter(Player* player);
        bool IsEncounterInProgress() const;
        void OnCreatureCreate(Creature* creature);
        void SetData(uint32 type, uint32 uiData);
        void DoPrepareChessEvent();
        void DoMoveChessPieceToSides(uint32 uiSpellId, uint32 uiFaction, bool bGameEnd = false);
        void Update(uint32 uiDiff);
        void SetData64(uint32 identifier, uint64 data);
        void OnGameObjectCreate(GameObject* go);
        std::string GetSaveData();
        uint32 GetData(uint32 uiData) const;
        uint64 GetData64(uint32 uiData) const;
        void Load(char const* chrIn);
        bool IsFriendlyGameReady();
        void GetChessPiecesByFaction(std::list<uint64 >& lList, uint32 uiFaction) { lList = uiFaction == FACTION_ID_CHESS_ALLIANCE ? m_lChessPiecesAlliance : m_lChessPiecesHorde; }
        bool IsCaseOccupied(Creature* creature);

    private:
        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string strSaveData;

        uint32 m_uiOperaEvent;
        uint32 m_uiOzDeathCount;

        uint64 m_uiCurtainGUID;
        uint64 m_uiStageDoorLeftGUID;
        uint64 m_uiStageDoorRightGUID;
        uint64 m_uiKilrekGUID;
        uint64 m_uiTerestianGUID;
        uint64 m_uiMoroesGUID;
        uint64 m_uiLibraryDoor;                                     // Door at Shade of Aran
        uint64 m_uiMassiveDoor;                                     // Door at Netherspite
        uint64 m_uiSideEntranceDoor;                                // Side Entrance
        uint64 m_uiGamesmansDoor;                                   // Door before Chess
        uint64 m_uiGamesmansExitDoor;                               // Door after Chess
        uint64 m_uiNetherspaceDoor;                                // Door at Malchezaar
        uint64 MastersTerraceDoor[2];
        uint64 ImageGUID;
        uint64 DustCoveredChest;

        uint32 m_uiTeam;
        uint32 m_uiChessResetTimer;
        uint32 m_uiAllianceStalkerCount;
        uint32 m_uiHordeStalkerCount;
        bool m_bFriendlyGame;
        uint64 m_HordeStatusGuid;
        uint64 m_AllianceStatusGuid;

        std::list<uint64 > m_lNightbaneGroundTriggers;
        std::list<uint64 > m_lNightbaneAirTriggers;

        std::list<uint64 > m_lChessHordeStalkerList;
        std::list<uint64 > m_lChessAllianceStalkerList;
        std::list<uint64 > m_lChessPiecesAlliance;
        std::list<uint64 > m_lChessPiecesHorde;
        std::vector<uint64 > m_vHordeStalkers;
        std::vector<uint64 > m_vAllianceStalkers;

        uint64 m_uiMedivhGUID;
        uint64 m_uiChessVicConGUID;

        uint32 m_uiChessDamages;
    };
};

#endif
