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

#include "GroupMgr.h"
#include "MapManager.h"
#include "SpellAuras.h"
#include "BattlefieldTB.h"

BattlefieldTB::~BattlefieldTB()
{
    for (TBWorkShop::const_iterator itr = WorkShopList.begin(); itr != WorkShopList.end(); ++itr)
        delete *itr;

    for (TBGameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
        delete *itr;
}

bool BattlefieldTB::SetupBattlefield()
{
    m_TypeId                     = BATTLEFIELD_TB;    //View enum BattlefieldTypes
    m_BattleId                   = BATTLEFIELD_BATTLEID_TB;
    m_Guid                       = 2238289014803267605;
    m_ZoneId                     = 5095; // Tol Barad
    m_MapId                      = 732;  // Map X
    m_Map                        = sMapMgr->FindMap(m_MapId, 0);
    if (m_Map == NULL)
        m_Map = sMapMgr->CreateBaseMap(m_MapId);
    m_MaxPlayer                  = sWorld->getIntConfig(CONFIG_TOL_BARAD_PLR_MAX);
    m_IsEnabled                  = sWorld->getBoolConfig(CONFIG_TOL_BARAD_ENABLE);
    m_MinPlayer                  = sWorld->getIntConfig(CONFIG_TOL_BARAD_PLR_MIN);
    m_MinLevel                   = sWorld->getIntConfig(CONFIG_TOL_BARAD_PLR_MIN_LVL);
    m_BattleTime                 = sWorld->getIntConfig(CONFIG_TOL_BARAD_BATTLETIME) * MINUTE * IN_MILLISECONDS;
    m_NoWarBattleTime            = sWorld->getIntConfig(CONFIG_TOL_BARAD_NOBATTLETIME) * MINUTE * IN_MILLISECONDS;
    m_RestartAfterCrash          = sWorld->getIntConfig(CONFIG_WINTERGRASP_RESTART_AFTER_CRASH) * MINUTE * IN_MILLISECONDS;

    m_TimeForAcceptInvite        = 20; // in second
    m_StartGroupingTimer         = 15 * MINUTE * IN_MILLISECONDS;
    m_StartGrouping              = false;

    // doors
    uiDoodadDoor                 = 0;
    uiDepthDoor                  = 0;
    uiHallDoor                   = 0;
    uiBlocDDoor                  = 0;

    RegisterZone(m_ZoneId);

    m_Data32.resize(BATTLEFIELD_TB_DATA_MAX);

    m_saveTimer                  = 60000;

    // Init GraveYards
    SetGraveyardNumber(BATTLEFIELD_TB_GY_MAX);

    _dynamicQuestCounter = 0;

    //Load from db
    if((sWorld->getWorldState(BATTLEFIELD_TB_WORLD_STATE_ACTIVE) == 0 ) &&
       (sWorld->getWorldState(BATTLEFIELD_TB_WORLD_STATE_DEFENDER) == 0) &&
       (sWorld->getWorldState(TBClockWorldState[0]) == 0 ))
    {
        sWorld->setWorldState(BATTLEFIELD_TB_WORLD_STATE_ACTIVE, false);
        sWorld->setWorldState(BATTLEFIELD_TB_WORLD_STATE_DEFENDER, urand(0, 1));
        sWorld->setWorldState(TBClockWorldState[0], m_NoWarBattleTime);
    }

    m_isActive               = bool(sWorld->getWorldState(BATTLEFIELD_TB_WORLD_STATE_ACTIVE));
    m_DefenderTeam           = (TeamId)sWorld->getWorldState(BATTLEFIELD_TB_WORLD_STATE_DEFENDER);

    m_Timer                  = sWorld->getWorldState(TBClockWorldState[0]);
    if (m_isActive)
    {
        m_isActive = false;
        m_Timer = m_RestartAfterCrash;
    }

    for (uint8 i = 0; i < BATTLEFIELD_TB_GY_MAX; i++)
    {
        BfGraveYardTB* gy = new BfGraveYardTB(this);

        // When between games, the graveyard is controlled by the defending team
        if (TBGraveYard[i].startcontrol == TEAM_NEUTRAL)
            gy->Initialize(m_DefenderTeam, TBGraveYard[i].guid);
        else
            gy->Initialize(TBGraveYard[i].startcontrol, TBGraveYard[i].guid);

        gy->SetTextId(TBGraveYard[i].textid);
        if (Creature* creature = SpawnCreature(ALLIANCE_SPRIT_GUIDE, TBGraveYard[i].x, TBGraveYard[i].y, TBGraveYard[i].z, TBGraveYard[i].o, TEAM_ALLIANCE))
            gy->SetSpirit(creature, TEAM_ALLIANCE);
        if (Creature* creature = SpawnCreature(HORDE_SPIRIT_GUIDE, TBGraveYard[i].x, TBGraveYard[i].y, TBGraveYard[i].z, TBGraveYard[i].o, TEAM_HORDE))
            gy->SetSpirit(creature, TEAM_HORDE);
        m_GraveyardList[i] = gy;
    }

    // Spawn workshop creatures and gameobjects
    for (uint8 i = 0; i < TB_MAX_WORKSHOP; i++)
    {
        BfTBWorkShopData* ws = new BfTBWorkShopData(this); // Create new object
        // Init:setup variable
        ws->Init(TBCapturePointDataBase[i].type, TBCapturePointDataBase[i].nameid, GetDefenderTeam());
        // Spawn associate gameobject on this point (Horde/Alliance flags)
        for (int g = 0; g<TBCapturePointDataBase[i].nbgob; g++)
            ws->AddGameObject(TBCapturePointDataBase[i].GameObjectData[g]);

        // Create PvPCapturePoint
        ws->GiveControlTo(GetDefenderTeam(), true);
        if (TBCapturePointDataBase[i].type < BATTLEFIELD_TB_FORT_MAX)
        {
            // Create Object
            BfCapturePointTB* TBWorkShop = new BfCapturePointTB(this, GetDefenderTeam());
            // Spawn gameobject associate (see in OnGameObjectCreate, of OutdoorPvP for see association)
            if (TBWorkShop->SetCapturePointData(TBCapturePointDataBase[i].CapturePoint.entryh, 732,
                                                TBCapturePointDataBase[i].CapturePoint.x,
                                                TBCapturePointDataBase[i].CapturePoint.y,
                                                TBCapturePointDataBase[i].CapturePoint.z,
                                                0))
            {
                TBWorkShop->LinkToWorkShop(ws);   // Link our point to the capture point (for faction changement)
                AddCapturePoint(TBWorkShop);      // Add this capture point to list for update this (view in Update() of OutdoorPvP)
            }
        }
        WorkShopList.insert(ws);
    }

    // Spawning npc in keep
    for (uint8 i = 0; i < TB_MAX_KEEP_NPC; i++)
    {
        // Horde npc
        if (Creature* creature = SpawnCreature(TBKeepNPC[i].entryh, TBKeepNPC[i].x, TBKeepNPC[i].y, TBKeepNPC[i].z, TBKeepNPC[i].o, TEAM_HORDE))
            KeepCreature[TEAM_HORDE].insert(creature->GetGUID());
        // Alliance npc
        if (Creature* creature = SpawnCreature(TBKeepNPC[i].entrya, TBKeepNPC[i].x, TBKeepNPC[i].y, TBKeepNPC[i].z, TBKeepNPC[i].o, TEAM_ALLIANCE))
            KeepCreature[TEAM_ALLIANCE].insert(creature->GetGUID());
    }


    // Spawwning quest trou creatures
    for (uint8 i = 0; i < TB_MAX_TROU_NPC; i++)
        if (Creature* creature = SpawnCreature(TBTrouNPC[i].entryh, TBTrouNPC[i].x, TBTrouNPC[i].y, TBTrouNPC[i].z, TBTrouNPC[i].o, TEAM_HORDE))
            TrouCreature.insert(creature->GetGUID());

    // Spawwning quest trou creatures
    for (uint8 i = 0; i < TB_MAX_DEPTH_NPC; i++)
        if (Creature* creature = SpawnCreature(TBDeapthNPC[i].entryh, TBDeapthNPC[i].x, TBDeapthNPC[i].y, TBDeapthNPC[i].z, TBDeapthNPC[i].o, TEAM_HORDE))
            DepthCreature.insert(creature->GetGUID());

    // Spawwning quest trou creatures
    for (uint8 i = 0; i < TB_MAX_BLOCD_NPC; i++)
        if (Creature* creature = SpawnCreature(TBBlocDNPC[i].entryh, TBBlocDNPC[i].x, TBBlocDNPC[i].y, TBBlocDNPC[i].z, TBBlocDNPC[i].o, TEAM_HORDE))
            BlocDCreature.insert(creature->GetGUID());

    // Spawwning dynamic quest giver
    for (uint8 i = 0; i < TB_MAX_DYNAMIC_QUEST_GIVER; i++)
    {
        if (Creature* creature = SpawnCreature(TBQuestNPC[i].entryh, TBQuestNPC[i].x, TBQuestNPC[i].y, TBQuestNPC[i].z, TBQuestNPC[i].o, TEAM_HORDE))
            DynamicQuestCreatre[TEAM_HORDE].insert(creature->GetGUID());
        if (Creature* creature = SpawnCreature(TBQuestNPC[i].entrya, TBQuestNPC[i].x, TBQuestNPC[i].y, TBQuestNPC[i].z, TBQuestNPC[i].o, TEAM_ALLIANCE))
            DynamicQuestCreatre[TEAM_ALLIANCE].insert(creature->GetGUID());
    }
    for (GuidSet::const_iterator itr = TrouCreature.begin(); itr != TrouCreature.end(); ++itr)
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    for (GuidSet::const_iterator itr = DepthCreature.begin(); itr != DepthCreature.end(); ++itr)
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    for (GuidSet::const_iterator itr = BlocDCreature.begin(); itr != BlocDCreature.end(); ++itr)
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    for (GuidSet::const_iterator itr = DynamicQuestCreatre[GetAttackerTeam()].begin(); itr != DynamicQuestCreatre[GetAttackerTeam()].end(); ++itr)
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    for (GuidSet::const_iterator itr = DynamicQuestCreatre[GetDefenderTeam()].begin(); itr != DynamicQuestCreatre[GetDefenderTeam()].end(); ++itr)
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    // Hide keep npc
    for (GuidSet::const_iterator itr = KeepCreature[GetAttackerTeam()].begin(); itr != KeepCreature[GetAttackerTeam()].end(); ++itr)
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    // show keep npc
    for (GuidSet::const_iterator itr = KeepCreature[GetDefenderTeam()].begin(); itr != KeepCreature[GetDefenderTeam()].end(); ++itr)
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
            {
                ShowNpc(creature, true);
                AnimateCreature(creature);
            }
    //Spawning Buiding
    for (uint8 i = 0; i < TB_MAX_OBJ; i++)
    {
        GameObject* go = SpawnGameObject(TBGameObjectBuillding[i].entry, TBGameObjectBuillding[i].x, TBGameObjectBuillding[i].y, TBGameObjectBuillding[i].z, TBGameObjectBuillding[i].o, 732);
        if (go)
        {
            // 5453
            BfTBGameObjectBuilding* b = new BfTBGameObjectBuilding(this);
            b->Init(go, TBGameObjectBuillding[i].type, TBGameObjectBuillding[i].nameid);
            BuildingsInZone.insert(b);
        }
    }
    StartDynamicQuests();
    return true;
}

void BattlefieldTB::AnimateCreature(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case BATTLEFIELD_TB_NPC_PROBLIM:
            creature->GetMotionMaster()->MovePath(NPC_PROBLIM_PATH, true);
            break;
        case BATTLEFIELD_TB_NPC_GUARD_H:
        case BATTLEFIELD_TB_NPC_GUARD_A:
            creature->GetMotionMaster()->MoveRandom(30.0f);
            break;
        case BATTLEFIELD_TB_NPC_HUNTER_A:
        case BATTLEFIELD_TB_NPC_MAGE_A:
        case BATTLEFIELD_TB_NPC_WARRIOR_A:
        case BATTLEFIELD_TB_NPC_PALADIN_A:
        case BATTLEFIELD_TB_NPC_DRUID_H:
        case BATTLEFIELD_TB_NPC_MAGE_H:
        case BATTLEFIELD_TB_NPC_ROGUE_H:
        case BATTLEFIELD_TB_NPC_SHAMAN_H:
            creature->GetMotionMaster()->MoveRandom(15.0f);
            break;
        case BATTLEFIELD_TB_NPC_CROCO:
            creature->GetMotionMaster()->MoveRandom(10.0f);
            break;
    }
}

void BattlefieldTB::OnGameObjectCreate(GameObject* go)
{
    switch (go->GetEntry())
    {
        case BATTLEFIELD_TB_GAMEOBJECT_DOODAD_DOOR:
            uiDoodadDoor = go->GetGUID();
            if (CanQueue() || IsWarTime())
                go->SetGoState(GO_STATE_READY);
            else
                go->SetGoState(GO_STATE_ACTIVE);
            break;
        case BATTLEFIELD_TB_GAMEOBJECT_DEPTH_DOOR:
            uiDepthDoor = go->GetGUID();
            if (_dynamicQuestCounter == 1)
                go->SetGoState(GO_STATE_ACTIVE);
            else
                go->SetGoState(GO_STATE_READY);
            break;
        case BATTLEFIELD_TB_GAMEOBJECT_THE_HALL_DOOR:
            uiHallDoor = go->GetGUID();
            if (_dynamicQuestCounter == 0)
                go->SetGoState(GO_STATE_ACTIVE);
            else
                go->SetGoState(GO_STATE_READY);
            break;
        case BATTLEFIELD_TB_GAMEOBJECT_BLOCD_DOOR:
            uiBlocDDoor = go->GetGUID();
            if (_dynamicQuestCounter == 2)
                go->SetGoState(GO_STATE_ACTIVE);
            else
                go->SetGoState(GO_STATE_READY);
            break;
        default:
            return;
    }
}

bool BattlefieldTB::Update(uint32 diff)
{
    bool m_return = Battlefield::Update(diff);
    if (m_saveTimer <= diff)
    {
        sWorld->setWorldState(BATTLEFIELD_TB_WORLD_STATE_ACTIVE, m_isActive);
        sWorld->setWorldState(BATTLEFIELD_TB_WORLD_STATE_DEFENDER, m_DefenderTeam);
        sWorld->setWorldState(TBClockWorldState[0], m_Timer );
        m_saveTimer = 60 * IN_MILLISECONDS;
    }
    else
        m_saveTimer -= diff;

    for (GuidSet::const_iterator itr = m_PlayersIsSpellImu.begin(); itr != m_PlayersIsSpellImu.end(); ++itr)
        if (Player* player = ObjectAccessor::FindPlayer((*itr)))
        {
            if (player->HasAura(SPELL_SPIRITUAL_IMMUNITY_TB))
            {
                const WorldSafeLocsEntry* graveyard = GetClosestGraveYard(player);
                if (graveyard)
                {
                    if (player->GetDistance2d(graveyard->x, graveyard->y) > 6.0f)
                    {
                        player->RemoveAurasDueToSpell(SPELL_SPIRITUAL_IMMUNITY_TB);
                        m_PlayersIsSpellImu.erase(player->GetGUID());
                    }
                }
            }
        }
    return m_return;
}

void BattlefieldTB::AddPlayerToResurrectQueue(uint64 npc_guid, uint64 player_guid)
{
    Battlefield::AddPlayerToResurrectQueue(npc_guid, player_guid);
    if (IsWarTime())
    {
        if (Player* player = ObjectAccessor::FindPlayer(player_guid))
        {
            if (!player->HasAura(SPELL_SPIRITUAL_IMMUNITY_TB))
            {
                player->CastSpell(player, SPELL_SPIRITUAL_IMMUNITY_TB, true);
                m_PlayersIsSpellImu.insert(player->GetGUID());
            }
        }
    }
}

void BattlefieldTB::OnBattleStart()
{
    m_Data32[BATTLEFIELD_TB_DATA_BROKEN_TOWER_DEF] = 0;
    m_Data32[BATTLEFIELD_TB_DATA_CAPTURED_FORT_DEF] = 3;
    m_Data32[BATTLEFIELD_TB_DATA_CAPTURED_FORT_ATT] = 0;

    if (GameObject *go = m_Map->GetGameObject(uiDoodadDoor))
        go->SetGoState(GO_STATE_READY);
    if (GameObject *go = m_Map->GetGameObject(uiDepthDoor))
        go->SetGoState(GO_STATE_READY);
    if (GameObject *go = m_Map->GetGameObject(uiHallDoor))
        go->SetGoState(GO_STATE_READY);
    if (GameObject *go = m_Map->GetGameObject(uiBlocDDoor))
        go->SetGoState(GO_STATE_READY);

    // Update graveyard (in no war time all graveyard is to deffender, in war time, depend of base)
    for (TBWorkShop::const_iterator itr = WorkShopList.begin(); itr != WorkShopList.end(); ++itr)
        if ((*itr))
            (*itr)->UpdateGraveYardAndWorkshop();

    // Reset capture points - should be done with workshop update (linked) i dont know why it doesnt work...
    for (BfCapturePointMap::iterator itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
        itr->second->ResetCapturePointData(itr->second->GetCapturePointGo(), GetDefenderTeam());

    //Hide keep npc
    for (GuidSet::const_iterator itr = KeepCreature[GetAttackerTeam()].begin(); itr != KeepCreature[GetAttackerTeam()].end(); ++itr)
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    for (GuidSet::const_iterator itr = KeepCreature[GetDefenderTeam()].begin(); itr != KeepCreature[GetDefenderTeam()].end(); ++itr)
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    // Spawning battle npcs
    for (uint8 i = 0; i < TB_MAX_WAR_NPC; i++)
        if (Creature* creature = SpawnCreature(TBWarNPC[i].entryh, TBWarNPC[i].x, TBWarNPC[i].y, TBWarNPC[i].z, TBWarNPC[i].o, TEAM_HORDE))
            WarCreature.insert(creature->GetGUID());

    //Show battle npcs
    for (GuidSet::const_iterator itr = WarCreature.begin(); itr != WarCreature.end(); ++itr)
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
            {
                ShowNpc(creature, true);
                if (creature->GetEntry() == BATTLEFIELD_TB_VEHICLE)
                    creature->setFaction(GetAttackerTeam() == TEAM_HORDE ? 85 : 12);
            }

    for (TBGameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
        if ((*itr))
            (*itr)->Rebuild();

    for (GuidSet::const_iterator itr = TrouCreature.begin(); itr != TrouCreature.end(); ++itr)
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    for (GuidSet::const_iterator itr = DepthCreature.begin(); itr != DepthCreature.end(); ++itr)
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    for (GuidSet::const_iterator itr = BlocDCreature.begin(); itr != BlocDCreature.end(); ++itr)
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    for (GuidSet::const_iterator itr = DynamicQuestCreatre[GetAttackerTeam()].begin(); itr != DynamicQuestCreatre[GetAttackerTeam()].end(); ++itr)
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    for (GuidSet::const_iterator itr = DynamicQuestCreatre[GetDefenderTeam()].begin(); itr != DynamicQuestCreatre[GetDefenderTeam()].end(); ++itr)
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    m_GraveyardList[0]->GiveControlTo(GetAttackerTeam());
    //Warnin message
    SendWarningToAllInZone(BATTLEFIELD_TB_TEXT_START);

}

void BattlefieldTB::OnBattleEnd(bool endbytimer)
{
    if (GameObject *go = m_Map->GetGameObject(uiDoodadDoor))
        go->SetGoState(GO_STATE_ACTIVE);

    for (GuidSet::const_iterator itr = m_PlayersInWar[GetDefenderTeam()].begin(); itr != m_PlayersInWar[GetDefenderTeam()].end(); ++itr)
    {
        if (Player* player = ObjectAccessor::FindPlayer((*itr)))
        {
            if (player->GetTeamId() == TEAM_HORDE)
            {
                player->CastSpell(player, SPELL_TOL_BARAD_VICTORY_HORDE, true);
                player->CastSpell(player, SPELL_TOL_BARAD_QUEST_HORDE, true);
            }
            else
            {
                player->CastSpell(player, SPELL_TOL_BARAD_VICTORY_ALLIANCE, true);
                player->CastSpell(player, SPELL_TOL_BARAD_QUEST_ALLIANCE, true);
            }
            for (TBGameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
                if ((*itr))
                {
                    if (!endbytimer)
                    {
                        if ((*itr)->IsDamage())
                            player->CastSpell(player, SPELL_TOL_BARAD_TOWER_DAMAGED, true);
                        else if ((*itr)->IsDestroy())
                            player->CastSpell(player, SPELL_TOL_BARAD_TOWER_DESTROYED, true);
                    }
                    else if ((*itr)->IsIntact())
                        player->CastSpell(player, SPELL_TOL_BARAD_TOWER_DEFENDED, true);
                }
            // Victory, TB TODO
            //DoCompleteOrIncrementAchievement(ACHIEVEMENTS_WIN_TB, player);
            // Ployez sous notre joug
            /*if (GetTimer() <= 10000 && !endbytimer)
                DoCompleteOrIncrementAchievement(ACHIEVEMENTS_WIN_TB_TIMER_10, player);*/
        }
    }

    for (GuidSet::const_iterator itr = m_PlayersInWar[GetAttackerTeam()].begin(); itr != m_PlayersInWar[GetAttackerTeam()].end(); ++itr)
        if (Player* player = ObjectAccessor::FindPlayer((*itr)))
            player->CastSpell(player, SPELL_TOL_BARAD_DEFEAT, true);

    //Change all npc in keep
    for (GuidSet::const_iterator itr = KeepCreature[GetAttackerTeam()].begin(); itr != KeepCreature[GetAttackerTeam()].end(); ++itr)
    {
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);
    }
    for (GuidSet::const_iterator itr = KeepCreature[GetDefenderTeam()].begin(); itr != KeepCreature[GetDefenderTeam()].end(); ++itr)
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
            {
                ShowNpc(creature, true);
                AnimateCreature(creature);
            }
    for (GuidSet::const_iterator itr = DynamicQuestCreatre[GetAttackerTeam()].begin(); itr != DynamicQuestCreatre[GetAttackerTeam()].end(); ++itr)
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    // Hide creatures that should be visible only when battle is on.
    for (GuidSet::const_iterator itr = WarCreature.begin(); itr != WarCreature.end(); ++itr)
        if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
            if (Creature* creature = unit->ToCreature())
                creature->RemoveFromWorld();
    WarCreature.clear();

    for (TBGameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
        if ((*itr))
            (*itr)->Rebuild();

    for (TBWorkShop::const_iterator itr = WorkShopList.begin(); itr != WorkShopList.end(); ++itr)
        if (*itr)
            (*itr)->GiveControlTo(GetDefenderTeam(), false);

    // Update all graveyard, control is to defender when no wartime
    for (uint8 i = 0; i < BATTLEFIELD_TB_GY_MAX; i++)
    {
        if (GetGraveyardById(i))
        {
            GetGraveyardById(i)->GiveControlTo(GetDefenderTeam());
        }
    }

    for (TBGameObjectSet::const_iterator itr = m_KeepGameObject[GetDefenderTeam()].begin(); itr != m_KeepGameObject[GetDefenderTeam()].end(); ++itr)
        (*itr)->SetRespawnTime(RESPAWN_IMMEDIATELY);

    for (TBGameObjectSet::const_iterator itr = m_KeepGameObject[GetAttackerTeam()].begin(); itr != m_KeepGameObject[GetAttackerTeam()].end(); ++itr)
        (*itr)->SetRespawnTime(RESPAWN_ONE_DAY);

    //Saving data
    for (TBGameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
        (*itr)->Save();
    for (TBWorkShop::const_iterator itr = WorkShopList.begin(); itr != WorkShopList.end(); ++itr)
        (*itr)->Save();

    for (uint8 team = 0; team < 2; ++team)
    {
        for (GuidSet::const_iterator itr = m_PlayersInWar[team].begin(); itr != m_PlayersInWar[team].end(); ++itr)
        {
            if (Player* player = ObjectAccessor::FindPlayer((*itr)))
            {
                player->RemoveAurasDueToSpell(SPELL_VETERAN);
                player->RemoveAurasDueToSpell(SPELL_SPIRITUAL_IMMUNITY_TB);
            }
        }
        m_PlayersInWar[team].clear();

        for (GuidSet::const_iterator itr = m_vehicles[team].begin(); itr != m_vehicles[team].end(); ++itr)
        {
            if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
                if (Creature* creature = unit->ToCreature())
                    if (creature->IsVehicle())
                        creature->GetVehicleKit()->RemoveAllPassengers();
        }
        m_vehicles[team].clear();
    }

    _dynamicQuestCounter++;
    if (_dynamicQuestCounter >= 3)
        _dynamicQuestCounter = 0;
    StartDynamicQuests();
    // Disband
}

void OnCreatureCreate(Creature* /*creature*/)
{

}

/******************************************************
*                    Reward System                    *
******************************************************/

void BattlefieldTB::DoCompleteOrIncrementAchievement(uint32 /*achievement*/, Player* /*player*/, uint8 /*incrementNumber*/)
{
}

void BattlefieldTB::OnStartGrouping()
{
    if (GameObject *go = m_Map->GetGameObject(uiDoodadDoor)) // close instance door 15 minutes before the battle start
        go->SetGoState(GO_STATE_READY);
    SendWarningToAllInZone(BATTLEFIELD_TB_TEXT_WILL_START);
}

void BattlefieldTB::OnPlayerJoinWar(Player* player)
{
    player->RemoveAurasDueToSpell(SPELL_VETERAN);
    player->RemoveAurasDueToSpell(SPELL_SPIRITUAL_IMMUNITY_TB);

    if (player->GetTeamId() == GetDefenderTeam())
    {
        if (player->GetZoneId() != m_ZoneId)
            player->TeleportTo(732, -1244.58f, 981.233f, 155.426f, 0);
        player->CastSpell(player, SPELL_SLOW_FALL_TB, true);
    }
    else
        player->TeleportTo(732, -827.212646f, 1187.948608f, 112.81f, 3.092834f);

    SendInitWorldStatesTo(player);
}

void BattlefieldTB::HandleKill(Player* killer, Unit* victim)
{
    if (killer == victim)
        return;

    TeamId killerTeam = killer->GetTeamId();
    if (victim->GetTypeId() == TYPEID_PLAYER)
    {
        for (GuidSet::const_iterator itr = m_PlayersInWar[killerTeam].begin(); itr != m_PlayersInWar[killerTeam].end(); ++itr)
            if (Player* player = sObjectAccessor->FindPlayer(*itr))
                if (player->GetDistance2d(killer) < 40 && !player->HasAura(SPELL_VETERAN))
                {
                    player->CastSpell(player, SPELL_VETERAN, true);
                    player->RewardHonor(victim, 1, 1);
                }
    }
}

void BattlefieldTB::OnPlayerLeaveWar(Player* player)
{
    if (!player->GetSession()->PlayerLogout())
    {
        player->RemoveAurasDueToSpell(SPELL_VETERAN);
        player->RemoveAurasDueToSpell(SPELL_SPIRITUAL_IMMUNITY_TB);
    }
}

void BattlefieldTB::OnPlayerLeaveZone(Player* player)
{
    if (!m_isActive)
    {
        player->RemoveAurasDueToSpell(SPELL_VETERAN);
        player->RemoveAurasDueToSpell(SPELL_SPIRITUAL_IMMUNITY_TB);
    }
}

void BattlefieldTB::OnPlayerEnterZone(Player* player)
{
    if (!m_isActive)
    {
        player->RemoveAurasDueToSpell(SPELL_VETERAN);
        player->RemoveAurasDueToSpell(SPELL_SPIRITUAL_IMMUNITY_TB);
    }

    // Send worldstate to player
    SendInitWorldStatesTo(player);
}
// Method sending worldsate to player
void BattlefieldTB::FillInitialWorldStates(WorldPacket& /*data*/)
{
}

void BattlefieldTB::SendInitWorldStatesTo(Player* player)
{
    WorldPacket data(SMSG_INIT_WORLD_STATES, (4 + 4 + 4 + 2 + (BuildingsInZone.size() * 8) + (WorkShopList.size() * 8)));

    data << uint32(m_MapId);
    data << uint32(m_ZoneId);
    data << uint32(0);
    data << uint16(4+2+4 + BuildingsInZone.size() + WorkShopList.size()); // Number of fields

    data << uint32(5384) << uint32(!IsWarTime() && GetDefenderTeam() == TEAM_HORDE ? 1 : 0); // Horde Controlled
    data << uint32(5385) << uint32(!IsWarTime() && GetDefenderTeam() == TEAM_ALLIANCE ? 1 : 0); // Alliance Controlled
    data << uint32(5546) << uint32(IsWarTime() && GetAttackerTeam() == TEAM_ALLIANCE ? 1 : 0); // Alliance is Attacking
    data << uint32(5547) << uint32(IsWarTime() && GetAttackerTeam() == TEAM_HORDE ? 1 : 0); // Horde is Attacking

    for (uint32 i = 0; i < 2; ++i)
        data << TBClockWorldState[i] << uint32(time(NULL) + (m_Timer / 1000));

    data << uint32(BATTLEFIELD_TB_WORLD_STATE_SHOW_WORLDSTATE) << uint32(IsWarTime() ? 1 : 0); // Time remaining
    data << uint32(BATTLEFIELD_TB_WORLD_STATE_ACTIVE) << uint32(IsWarTime() ? 0 : 1); // Next Battle

    for (TBGameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
        data << (*itr)->m_WorldState << 1;

    for (TBWorkShop::const_iterator itr = WorkShopList.begin(); itr != WorkShopList.end(); ++itr)
        data << (*itr)->m_WorldState << 1;

    data << uint32(TB_WS_TOWERS_DESTROYED_SHOW) << uint32(IsWarTime() && player->GetTeamId() == GetDefenderTeam() ? 1 : 0); // destroyed show
    data << uint32(TB_WS_BUILDINGS_CAPTURED_SHOW) << uint32(IsWarTime() &&  player->GetTeamId() == GetAttackerTeam() ? 1 : 0); // Captured show
    if (IsWarTime())
    {
        //        data << uint32(TB_WS_TOWERS_DESTROYED) << uint32(3); // destroyed count
        data << uint32(TB_WS_TOWERS_DESTROYED) << uint32(m_Data32[BATTLEFIELD_TB_DATA_BROKEN_TOWER_DEF]); // destroyed count
        data << uint32(TB_WS_BUILDINGS_CAPTURED) << uint32(m_Data32[BATTLEFIELD_TB_DATA_CAPTURED_FORT_ATT]); // Captured count
    }
    player->GetSession()->SendPacket(&data);
}

void BattlefieldTB::SendInitWorldStatesToAll()
{
    for (uint8 team = 0; team<2; team++)
        for (GuidSet::iterator itr = m_players[team].begin(); itr != m_players[team].end(); ++itr)
            if (Player* player = ObjectAccessor::FindPlayer((*itr)))
                SendInitWorldStatesTo(player);
}

void BattlefieldTB::AddBrokenTower(TeamId team)
{
    m_Timer += 5 * 60 * 1000; // Increase battle time by 5 mins.
    SendInitWorldStatesToAll();
}

void BattlefieldTB::ProcessEvent(WorldObject* obj, uint32 eventId)
{
    if (!obj || !IsWarTime())
        return;

    //if destroy or damage event, search the wall/tower and update worldstate/send warning message
    for (TBGameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
    {
        if (obj->GetEntry() == (*itr)->m_Build->GetEntry())
        {
            if ((*itr)->m_Build->GetGOInfo()->building.damagedEvent == eventId)
                (*itr)->Damaged();

            if ((*itr)->m_Build->GetGOInfo()->building.destroyedEvent == eventId)
                (*itr)->Destroyed();

            break;
        }
    }
}

void BattlefieldTB::StartDynamicQuests()
{
    switch (_dynamicQuestCounter)
    {
        case 0:
            if (GameObject *go = m_Map->GetGameObject(uiHallDoor))
                go->SetGoState(GO_STATE_ACTIVE);
            for (GuidSet::const_iterator itr = DynamicQuestCreatre[GetDefenderTeam()].begin(); itr != DynamicQuestCreatre[GetDefenderTeam()].end(); ++itr)
                if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
                    if (Creature* creature = unit->ToCreature())
                        if (creature->GetEntry() == (GetDefenderTeam() == TEAM_HORDE ? 48062 : 48074))
                            ShowNpc(creature, true);
            for (GuidSet::const_iterator itr = TrouCreature.begin(); itr != TrouCreature.end(); ++itr)
                if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
                    if (Creature* creature = unit->ToCreature())
                        ShowNpc(creature, true);
            break;
        case 1:
            if (GameObject *go = m_Map->GetGameObject(uiDepthDoor))
                go->SetGoState(GO_STATE_ACTIVE);
            for (GuidSet::const_iterator itr = DynamicQuestCreatre[GetDefenderTeam()].begin(); itr != DynamicQuestCreatre[GetDefenderTeam()].end(); ++itr)
                if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
                    if (Creature* creature = unit->ToCreature())
                        if (creature->GetEntry() == (GetDefenderTeam() == TEAM_HORDE ? 48071 : 48039))
                            ShowNpc(creature, true);
            for (GuidSet::const_iterator itr = DepthCreature.begin(); itr != DepthCreature.end(); ++itr)
                if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
                    if (Creature* creature = unit->ToCreature())
                        ShowNpc(creature, true);
            break;
        case 2:
            if (GameObject *go = m_Map->GetGameObject(uiBlocDDoor))
                go->SetGoState(GO_STATE_ACTIVE);
            for (GuidSet::const_iterator itr = DynamicQuestCreatre[GetDefenderTeam()].begin(); itr != DynamicQuestCreatre[GetDefenderTeam()].end(); ++itr)
                if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
                    if (Creature* creature = unit->ToCreature())
                        if (creature->GetEntry() == (GetDefenderTeam() == TEAM_HORDE ? 48070 : 48061))
                            ShowNpc(creature, true);
            for (GuidSet::const_iterator itr = BlocDCreature.begin(); itr != BlocDCreature.end(); ++itr)
                if (Unit* unit = ObjectAccessor::FindUnit((*itr)))
                    if (Creature* creature = unit->ToCreature())
                        ShowNpc(creature, true);
            break;
        default:
            break;
    }
}

void BfCapturePointTB::ChangeTeam(TeamId /*oldteam*/)
{
    m_Workshop->GiveControlTo(m_team, false);
}

BfCapturePointTB::BfCapturePointTB(BattlefieldTB* bf, TeamId control) : BfCapturePoint(bf)
{
    m_Bf = bf;
    m_team = control;
}

BfGraveYardTB::BfGraveYardTB(BattlefieldTB* bf) : BfGraveyard(bf)
{
    m_Bf = bf;
}
