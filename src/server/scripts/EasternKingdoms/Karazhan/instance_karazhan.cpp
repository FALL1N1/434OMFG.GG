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

/* ScriptData
SDName: Instance_Karazhan
SD%Complete: 70
SDComment: Instance Script for Karazhan to help in various encounters. TODO: GameObject visibility for Opera event.
SDCategory: Karazhan
EndScriptData */

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "karazhan.h"

#define MAX_ENCOUNTER      12

/*
0  - Attumen + Midnight (optional)
1  - Moroes
2  - Maiden of Virtue (optional)
3  - Hyakiss the Lurker /  Rokad the Ravager  / Shadikith the Glider
4  - Opera Event
5  - Curator
6  - Shade of Aran (optional)
7  - Terestian Illhoof (optional)
8  - Netherspite (optional)
9  - Chess Event
10 - Prince Malchezzar
11 - Nightbane
*/

void instance_karazhan::instance_karazhan_InstanceMapScript::Initialize()
{
    SetHeaders(DataHeader);
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

    // 1 - OZ, 2 - HOOD, 3 - RAJ, this never gets altered.
    m_uiOperaEvent      = urand(1, 3);
    m_uiOzDeathCount    = 0;

    m_uiTeam = 0;
    m_uiChessResetTimer = 0;
    m_uiAllianceStalkerCount = 0;
    m_uiHordeStalkerCount = 0;
    m_bFriendlyGame = false;

    m_uiCurtainGUID         = 0;
    m_uiStageDoorLeftGUID   = 0;
    m_uiStageDoorRightGUID  = 0;

    m_uiKilrekGUID      = 0;
    m_uiTerestianGUID   = 0;
    m_uiMoroesGUID      = 0;

    m_uiLibraryDoor         = 0;
    m_uiMassiveDoor         = 0;
    m_uiSideEntranceDoor    = 0;
    m_uiGamesmansDoor       = 0;
    m_uiGamesmansExitDoor   = 0;
    m_uiNetherspaceDoor     = 0;
    MastersTerraceDoor[0]= 0;
    MastersTerraceDoor[1]= 0;
    ImageGUID = 0;
    DustCoveredChest    = 0;

    m_uiMedivhGUID = 0;
    m_uiChessVicConGUID = 0;

    m_uiChessDamages = 0;
}

bool instance_karazhan::instance_karazhan_InstanceMapScript::IsFriendlyGameReady()
{
    return m_bFriendlyGame;
}

void instance_karazhan::instance_karazhan_InstanceMapScript::OnPlayerEnter(Player* player)
{
    if (!m_uiTeam)
        m_uiTeam = player->GetTeam();
}

bool instance_karazhan::instance_karazhan_InstanceMapScript::IsEncounterInProgress() const
{
    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
        if (m_auiEncounter[i] == IN_PROGRESS)
            return true;

    return false;
}

bool instance_karazhan::instance_karazhan_InstanceMapScript::IsCaseOccupied(Creature* creature)
{
    for (std::list<uint64 >::const_iterator itr = m_lChessPiecesAlliance.begin(); itr != m_lChessPiecesAlliance.end(); ++itr)
        if (Creature* pChessPiece = instance->GetCreature(*itr))
            if (pChessPiece->isAlive())
                if (pChessPiece->AI()->GetGUID() == creature->GetGUID())
                    return true;

    for (std::list<uint64 >::const_iterator itr = m_lChessPiecesHorde.begin(); itr != m_lChessPiecesHorde.end(); ++itr)
        if (Creature* pChessPiece = instance->GetCreature(*itr))
            if (pChessPiece->isAlive())
                if (pChessPiece->AI()->GetGUID() == creature->GetGUID())
                    return true;
    return false;
}

void instance_karazhan::instance_karazhan_InstanceMapScript::OnCreatureCreate(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_HUMAN_CHARGER:
        case NPC_HUMAN_CLERIC:
        case NPC_HUMAN_CONJURER:
        case NPC_HUMAN_FOOTMAN:
        case NPC_CONJURED_WATER_ELEMENTAL:
        case NPC_KING_LLANE:
            m_lChessPiecesAlliance.push_back(creature->GetGUID());
            break;
        case NPC_ORC_GRUNT:
        case NPC_ORC_NECROLYTE:
        case NPC_ORC_WARLOCK:
        case NPC_ORC_WOLF:
        case NPC_SUMMONED_DAEMON:
        case NPC_WARCHIEF_BLACKHAND:
            m_lChessPiecesHorde.push_back(creature->GetGUID());
            break;
        case NPC_NIGHTBANE_HELPER:
            if (creature->GetPositionZ() < 100.0f)
                m_lNightbaneGroundTriggers.push_back(creature->GetGUID());
            else
                m_lNightbaneAirTriggers.push_back(creature->GetGUID());
            break;
        case NPC_INVISIBLE_STALKER:
            if (creature->GetPositionY() < -1870.0f)
                m_lChessHordeStalkerList.push_back(creature->GetGUID());
            else
                m_lChessAllianceStalkerList.push_back(creature->GetGUID());
            break;
        case NPC_CHESS_STATUS_BAR:
            if (creature->GetPositionY() < -1870.0f)
                m_HordeStatusGuid = creature->GetGUID();
            else
                m_AllianceStatusGuid = creature->GetGUID();
            break;
        case NPC_ECHO_MEDIVH:
            m_uiMedivhGUID = creature->GetGUID();
            break;
        case NPC_CHESS_VICTORY_CONTROLLER:
            m_uiChessVicConGUID = creature->GetGUID();
            //m_mNpcEntryGuidStore[creature->GetEntry()] = creature->GetGUID();
            break;
        case 17229:
            m_uiKilrekGUID = creature->GetGUID();
            break;
        case 15688:
            m_uiTerestianGUID = creature->GetGUID();
            break;
        case 15687:
            m_uiMoroesGUID = creature->GetGUID();
            break;

    }
}

void instance_karazhan::instance_karazhan_InstanceMapScript::SetData(uint32 type, uint32 uiData)
{
    switch (type)
    {
        case TYPE_ATTUMEN:              m_auiEncounter[0] = uiData; break;
        case TYPE_MOROES:
            if (m_auiEncounter[1] == DONE)
                break;
            m_auiEncounter[1] = uiData;
            break;
        case TYPE_MAIDEN:               m_auiEncounter[2] = uiData; break;
        case TYPE_OPTIONAL_BOSS:        m_auiEncounter[3] = uiData; break;
        case TYPE_OPERA:
            m_auiEncounter[4] = uiData;
            if (uiData == DONE)
                UpdateEncounterState(ENCOUNTER_CREDIT_KILL_CREATURE, 16812, NULL);
            break;
        case TYPE_CURATOR:              m_auiEncounter[5] = uiData; break;
        case TYPE_ARAN:                 m_auiEncounter[6] = uiData; break;
        case TYPE_TERESTIAN:            m_auiEncounter[7] = uiData; break;
        case TYPE_NETHERSPITE:          m_auiEncounter[8] = uiData; break;
        case TYPE_CHESS:
        {
            if (uiData == DONE)
            {
                // doors and loot are not handled for friendly games
                if (GetData(TYPE_CHESS) != SPECIAL)
                {
                    //                            DoUseDoorOrButton(GO_GAMESMANS_HALL_EXIT_DOOR);
                    DoRespawnGameObject(DustCoveredChest, DAY);
                    if (GameObject* loot = instance->GetGameObject(DustCoveredChest))
                        loot->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                }

                // cast game end spells
                if (Creature* pMedivh =  instance->GetCreature(GetData64(NPC_ECHO_MEDIVH)))
                {
                    pMedivh->CastSpell(pMedivh, SPELL_FORCE_KILL_BUNNY, true);
                    pMedivh->CastSpell(pMedivh, SPELL_GAME_OVER, true);
                    pMedivh->CastSpell(pMedivh, SPELL_CLEAR_BOARD, true);
                }
                if (Creature* pController = instance->GetCreature(GetData64(NPC_CHESS_VICTORY_CONTROLLER)))
                    pController->CastSpell(pController, SPELL_VICTORY_VISUAL, true);
                DoRemoveAurasDueToSpellOnPlayers(SPELL_GAME_IN_SESSION);

                m_bFriendlyGame = false;
                m_uiChessResetTimer = 35000;
            }
            else if (uiData == FAIL)
            {
                // clean the board for reset
                if (Creature* pMedivh =  instance->GetCreature(GetData64(NPC_ECHO_MEDIVH)))
                {
                    pMedivh->CastSpell(pMedivh, SPELL_GAME_OVER, true);
                    pMedivh->CastSpell(pMedivh, SPELL_CLEAR_BOARD, true);
                }
                DoRemoveAurasDueToSpellOnPlayers(SPELL_GAME_IN_SESSION);
                m_uiChessResetTimer = 35000;
            }
            else if (uiData == IN_PROGRESS || uiData == SPECIAL)
                DoPrepareChessEvent();
            m_auiEncounter[9]  = uiData;
            break;
        }
        case TYPE_MALCHEZZAR:           m_auiEncounter[10] = uiData; break;
        case TYPE_NIGHTBANE:
            if (m_auiEncounter[11] != DONE)
                m_auiEncounter[11] = uiData;
            break;
        case DATA_OPERA_OZ_DEATHCOUNT:
            if (uiData == SPECIAL)
                ++m_uiOzDeathCount;
            else if (uiData == IN_PROGRESS)
                m_uiOzDeathCount = 0;
            break;
        case DATA_CHESS_DAMAGE:
            m_uiChessDamages += uiData;
            break;
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << ' ' << m_auiEncounter[1] << ' ' << m_auiEncounter[2] << ' '
                   << m_auiEncounter[3] << ' ' << m_auiEncounter[4] << ' ' << m_auiEncounter[5] << ' ' << m_auiEncounter[6] << ' '
                   << m_auiEncounter[7] << ' ' << m_auiEncounter[8] << ' ' << m_auiEncounter[9] << ' ' << m_auiEncounter[10] << ' ' << m_auiEncounter[11];

        strSaveData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

void instance_karazhan::instance_karazhan_InstanceMapScript::DoPrepareChessEvent()
{
    m_uiChessDamages = 0;

    if (Creature* pMedivh =  instance->GetCreature(GetData64(NPC_ECHO_MEDIVH)))
        pMedivh->AI()->DoAction(AI_EVENT_CUSTOM_B);

    // Allow all the chess pieces to init start position
    for (std::list<uint64 >::const_iterator itr = m_lChessPiecesAlliance.begin(); itr != m_lChessPiecesAlliance.end(); ++itr)
        if (Creature* pChessPiece = instance->GetCreature(*itr))
        {
            Creature* pSquare = GetClosestCreatureWithEntry(pChessPiece, NPC_SQUARE_BLACK, 2.0f);
            if (!pSquare)
                pSquare = GetClosestCreatureWithEntry(pChessPiece, NPC_SQUARE_WHITE, 2.0f);
            else
            {
                Creature* pSquare2 = GetClosestCreatureWithEntry(pChessPiece, NPC_SQUARE_WHITE, 2.0f);
                if (pSquare2)
                    if (pChessPiece->GetExactDist2d(pSquare2->GetPositionX(), pSquare2->GetPositionY()) < pChessPiece->GetExactDist2d(pSquare->GetPositionX(), pSquare->GetPositionY()))
                        pSquare = pSquare2;
            }
            if (!pSquare)
                return;

            // send event which will prepare the current square
            pChessPiece->AI()->SetGUID(pSquare->GetGUID());
            pChessPiece->AI()->DoAction(AI_EVENT_CUSTOM_B);
        }

    for (std::list<uint64 >::const_iterator itr = m_lChessPiecesHorde.begin(); itr != m_lChessPiecesHorde.end(); ++itr)
        if (Creature* pChessPiece = instance->GetCreature(*itr))
        {
            Creature* pSquare = GetClosestCreatureWithEntry(pChessPiece, NPC_SQUARE_BLACK, 2.0f);
            if (!pSquare)
                pSquare = GetClosestCreatureWithEntry(pChessPiece, NPC_SQUARE_WHITE, 2.0f);
            else
            {
                Creature* pSquare2 = GetClosestCreatureWithEntry(pChessPiece, NPC_SQUARE_WHITE, 2.0f);
                if (pSquare2)
                    if (pChessPiece->GetExactDist2d(pSquare2->GetPositionX(), pSquare2->GetPositionY()) < pChessPiece->GetExactDist2d(pSquare->GetPositionX(), pSquare->GetPositionY()))
                        pSquare = pSquare2;
            }

            if (!pSquare)
                return;

            // send event which will prepare the current square
            pChessPiece->AI()->SetGUID(pSquare->GetGUID());
            pChessPiece->AI()->DoAction(AI_EVENT_CUSTOM_B);
        }

    // add silence debuff
    DoCastSpellOnPlayers(SPELL_GAME_IN_SESSION);
    m_uiAllianceStalkerCount = 0;
    m_uiHordeStalkerCount = 0;
    m_vHordeStalkers.clear();
    m_vAllianceStalkers.clear();

    // sort stalkers depending on side
    std::list<Creature*> lStalkers;
    for (std::list<uint64 >::const_iterator itr = m_lChessHordeStalkerList.begin(); itr != m_lChessHordeStalkerList.end(); ++itr)
        if (Creature* pTemp = instance->GetCreature(*itr))
            lStalkers.push_back(pTemp);

    if (lStalkers.empty())
        return;

    // get the proper statusBar npc
    Creature* pStatusBar = instance->GetCreature(m_HordeStatusGuid);
    if (!pStatusBar)
        return;

    lStalkers.sort(Trinity::ObjectDistanceOrderPred(pStatusBar));
    for (std::list<Creature*>::const_iterator itr = lStalkers.begin(); itr != lStalkers.end(); ++itr)
        m_vHordeStalkers.push_back((*itr)->GetGUID());

    lStalkers.clear();
    for (std::list<uint64 >::const_iterator itr = m_lChessAllianceStalkerList.begin(); itr != m_lChessAllianceStalkerList.end(); ++itr)
        if (Creature* pTemp = instance->GetCreature(*itr))
            lStalkers.push_back(pTemp);

    if (lStalkers.empty())
        return;

    // get the proper statusBar npc
    pStatusBar = instance->GetCreature(m_AllianceStatusGuid);
    if (!pStatusBar)
        return;

    lStalkers.sort(Trinity::ObjectDistanceOrderPred(pStatusBar));
    for (std::list<Creature*>::const_iterator itr = lStalkers.begin(); itr != lStalkers.end(); ++itr)
        m_vAllianceStalkers.push_back((*itr)->GetGUID());
}

void instance_karazhan::instance_karazhan_InstanceMapScript::DoMoveChessPieceToSides(uint32 uiSpellId, uint32 uiFaction, bool bGameEnd /*= false*/)
{
    // assign proper faction variables
    std::vector<uint64 >& vStalkers = uiFaction == FACTION_ID_CHESS_ALLIANCE ? m_vAllianceStalkers : m_vHordeStalkers;
    uint32 uiCount = uiFaction == FACTION_ID_CHESS_ALLIANCE ? m_uiAllianceStalkerCount : m_uiHordeStalkerCount;

    // get the proper statusBar npc
    Creature* pStatusBar = instance->GetCreature(uiFaction == FACTION_ID_CHESS_ALLIANCE ? m_AllianceStatusGuid : m_HordeStatusGuid);
    if (!pStatusBar)
        return;

    if (vStalkers.size() < uiCount + 1)
        return;

    // handle stalker transformation
    if (Creature* pStalker = instance->GetCreature(vStalkers[uiCount]))
    {
        // need to provide specific target, in order to ensure the logic of the event
        pStatusBar->CastSpell(pStalker, uiSpellId, true);
        uiFaction == FACTION_ID_CHESS_ALLIANCE ? ++m_uiAllianceStalkerCount : ++m_uiHordeStalkerCount;
    }

    // handle emote on end game
    if (bGameEnd)
    {
        // inverse factions
        vStalkers.clear();
        vStalkers = uiFaction == FACTION_ID_CHESS_ALLIANCE ? m_vHordeStalkers : m_vAllianceStalkers;

        for (std::vector<uint64 >::const_iterator itr = vStalkers.begin(); itr != vStalkers.end(); ++itr)
        {
            if (Creature* pStalker = instance->GetCreature(*itr))
                pStalker->HandleEmoteCommand(EMOTE_STATE_APPLAUD);
        }
    }
}

void instance_karazhan::instance_karazhan_InstanceMapScript::Update(uint32 uiDiff)
{
    if (m_uiChessResetTimer)
    {
        // respawn all chess pieces and side stalkers on the original position
        if (m_uiChessResetTimer <= uiDiff)
        {
            for (std::list<uint64 >::const_iterator itr = m_lChessPiecesAlliance.begin(); itr != m_lChessPiecesAlliance.end(); ++itr)
            {
                if (Creature* pTemp = instance->GetCreature(*itr))
                {
                    if (!pTemp->isAlive())
                        pTemp->Respawn();
                    else
                    {
                        float x, y, z, o;
                        pTemp->GetHomePosition(x, y, z, o);
                        pTemp->NearTeleportTo(x, y, z, o);
                    }
                    pTemp->SetVisible(true);
                }
            }
            for (std::list<uint64 >::const_iterator itr = m_lChessPiecesHorde.begin(); itr != m_lChessPiecesHorde.end(); ++itr)
            {
                if (Creature* pTemp = instance->GetCreature(*itr))
                {
                    if (!pTemp->isAlive())
                        pTemp->Respawn();
                    else
                    {
                        float x, y, z, o;
                        pTemp->GetHomePosition(x, y, z, o);
                        pTemp->NearTeleportTo(x, y, z, o);
                    }
                    pTemp->SetVisible(true);
                }
            }

            for (std::list<uint64 >::const_iterator itr = m_lChessAllianceStalkerList.begin(); itr != m_lChessAllianceStalkerList.end(); ++itr)
            {
                if (Creature* pTemp = instance->GetCreature(*itr))
                {
                    if (!pTemp->isAlive())
                        pTemp->Respawn();
                    else
                    {
                        float x, y, z, o;
                        pTemp->GetHomePosition(x, y, z, o);
                        pTemp->NearTeleportTo(x, y, z, o);
                    }
                    pTemp->SetVisible(true);
                    pTemp->HandleEmoteCommand(EMOTE_STATE_NONE);
                }
            }
            for (std::list<uint64 >::const_iterator itr = m_lChessHordeStalkerList.begin(); itr != m_lChessHordeStalkerList.end(); ++itr)
            {
                if (Creature* pTemp = instance->GetCreature(*itr))
                {
                    if (!pTemp->isAlive())
                        pTemp->Respawn();
                    else
                    {
                        float x, y, z, o;
                        pTemp->GetHomePosition(x, y, z, o);
                        pTemp->NearTeleportTo(x, y, z, o);
                    }
                    pTemp->SetVisible(true);
                    pTemp->HandleEmoteCommand(EMOTE_STATE_NONE);
                }
            }

            for (std::vector<uint64 >::const_iterator itr = m_vHordeStalkers.begin(); itr != m_vHordeStalkers.end(); ++itr)
                if (Creature* pStalker = instance->GetCreature(*itr))
                    pStalker->RemoveAllAuras();

            for (std::vector<uint64 >::const_iterator itr = m_vAllianceStalkers.begin(); itr != m_vAllianceStalkers.end(); ++itr)
                if (Creature* pStalker = instance->GetCreature(*itr))
                    pStalker->RemoveAllAuras();

            if (GetData(TYPE_CHESS) == FAIL || GetData(TYPE_CHESS) == IN_PROGRESS)
                SetData(TYPE_CHESS, NOT_STARTED);
            else if (GetData(TYPE_CHESS) == DONE)
                m_bFriendlyGame = true;

            m_uiChessDamages = 0;
            m_uiChessResetTimer = 0;
            if (Creature* pMedivh =  instance->GetCreature(GetData64(NPC_ECHO_MEDIVH)))
                pMedivh->AI()->Reset();
        }
        else
            m_uiChessResetTimer -= uiDiff;
    }
}

void instance_karazhan::instance_karazhan_InstanceMapScript::SetData64(uint32 identifier, uint64 data)
{
    switch (identifier)
    {
    case DATA_IMAGE_OF_MEDIVH: ImageGUID = data;
    }
}

void instance_karazhan::instance_karazhan_InstanceMapScript::OnGameObjectCreate(GameObject* go)
{
    switch (go->GetEntry())
    {
        case 183932:   m_uiCurtainGUID          = go->GetGUID();         break;
        case 184278:
            m_uiStageDoorLeftGUID = go->GetGUID();
            if (m_auiEncounter[4] == DONE)
                go->SetGoState(GO_STATE_ACTIVE);
            break;
        case 184279:
            m_uiStageDoorRightGUID = go->GetGUID();
            if (m_auiEncounter[4] == DONE)
                go->SetGoState(GO_STATE_ACTIVE);
            break;
        case 184517:   m_uiLibraryDoor          = go->GetGUID();         break;
        case 185521:   m_uiMassiveDoor          = go->GetGUID();         break;
        case 184276:   m_uiGamesmansDoor        = go->GetGUID();         break;
        case 184277:   m_uiGamesmansExitDoor    = go->GetGUID();         break;
        case 185134:   m_uiNetherspaceDoor      = go->GetGUID();         break;
        case 184274:    MastersTerraceDoor[0] = go->GetGUID();  break;
        case 184280:    MastersTerraceDoor[1] = go->GetGUID();  break;
        case 184275:
            m_uiSideEntranceDoor = go->GetGUID();
            if (m_auiEncounter[4] == DONE)
                go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED);
            else
                go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED);
            break;
        case 185119: DustCoveredChest = go->GetGUID(); break;

    }

    switch (m_uiOperaEvent)
    {
        //TODO: Set Object visibilities for Opera based on performance
        case EVENT_OZ:
            break;

        case EVENT_HOOD:
            break;

        case EVENT_RAJ:
            break;
    }
}

std::string instance_karazhan::instance_karazhan_InstanceMapScript::GetSaveData()
{
    return strSaveData;
}

uint32 instance_karazhan::instance_karazhan_InstanceMapScript::GetData(uint32 uiData) const
{
    switch (uiData)
    {
        case TYPE_ATTUMEN:              return m_auiEncounter[0];
        case TYPE_MOROES:               return m_auiEncounter[1];
        case TYPE_MAIDEN:               return m_auiEncounter[2];
        case TYPE_OPTIONAL_BOSS:        return m_auiEncounter[3];
        case TYPE_OPERA:                return m_auiEncounter[4];
        case TYPE_CURATOR:              return m_auiEncounter[5];
        case TYPE_ARAN:                 return m_auiEncounter[6];
        case TYPE_TERESTIAN:            return m_auiEncounter[7];
        case TYPE_NETHERSPITE:          return m_auiEncounter[8];
        case TYPE_CHESS:                return m_auiEncounter[9];
        case TYPE_MALCHEZZAR:           return m_auiEncounter[10];
        case TYPE_NIGHTBANE:            return m_auiEncounter[11];
        case DATA_OPERA_PERFORMANCE:    return m_uiOperaEvent;
        case DATA_OPERA_OZ_DEATHCOUNT:  return m_uiOzDeathCount;
        case DATA_TEAM_IN_INSTANCE:     return m_uiTeam;
        case DATA_CHESS_DAMAGE:         return m_uiChessDamages;
    }

    return 0;
}

uint64 instance_karazhan::instance_karazhan_InstanceMapScript::GetData64(uint32 uiData) const
{
    switch (uiData)
    {
        case DATA_KILREK:                   return m_uiKilrekGUID;
        case DATA_TERESTIAN:                return m_uiTerestianGUID;
        case DATA_MOROES:                   return m_uiMoroesGUID;
        case DATA_GO_STAGEDOORLEFT:         return m_uiStageDoorLeftGUID;
        case DATA_GO_STAGEDOORRIGHT:        return m_uiStageDoorRightGUID;
        case DATA_GO_CURTAINS:              return m_uiCurtainGUID;
        case DATA_GO_LIBRARY_DOOR:          return m_uiLibraryDoor;
        case DATA_GO_MASSIVE_DOOR:          return m_uiMassiveDoor;
        case DATA_GO_SIDE_ENTRANCE_DOOR:    return m_uiSideEntranceDoor;
        case DATA_GO_GAME_DOOR:             return m_uiGamesmansDoor;
        case DATA_GO_GAME_EXIT_DOOR:        return m_uiGamesmansExitDoor;
        case DATA_GO_NETHER_DOOR:           return m_uiNetherspaceDoor;
        case DATA_MASTERS_TERRACE_DOOR_1:   return MastersTerraceDoor[0];
        case DATA_MASTERS_TERRACE_DOOR_2:   return MastersTerraceDoor[1];
        case DATA_IMAGE_OF_MEDIVH:          return ImageGUID;
        case NPC_ECHO_MEDIVH:               return m_uiMedivhGUID;
        case NPC_CHESS_VICTORY_CONTROLLER:  return m_uiChessVicConGUID;
    }

    return 0;
}

void instance_karazhan::instance_karazhan_InstanceMapScript::Load(char const* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);
    std::istringstream loadStream(chrIn);

    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3]
               >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6] >> m_auiEncounter[7]
               >> m_auiEncounter[8] >> m_auiEncounter[9] >> m_auiEncounter[10] >> m_auiEncounter[11];
    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
        if (m_auiEncounter[i] == IN_PROGRESS)                // Do not load an encounter as "In Progress" - reset it instead.
            m_auiEncounter[i] = NOT_STARTED;
    OUT_LOAD_INST_DATA_COMPLETE;
}

void AddSC_instance_karazhan()
{
    new instance_karazhan();
}
