/*
 * Copyright (C) 2013-2014 OMFG.GG Network <http://www.omfg.gg/>
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
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

#include "InstanceScript.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "deadmines.h"

DoorData const doorData[] =
{
    {GO_GLUBTOK_DOOR,                        BOSS_GLUBTOK_DATA,          DOOR_TYPE_PASSAGE,       BOUNDARY_N   },
    {GO_HELIX_DOOR_ENTRANCE,                 BOSS_HELIX_DATA,            DOOR_TYPE_ROOM,          BOUNDARY_N   },
    {GO_HELIX_DOOR_EXIT,                     BOSS_HELIX_DATA,            DOOR_TYPE_PASSAGE,       BOUNDARY_N   },
    {GO_FOE_ENTRANCE,                        BOSS_FOE_REAPER_5000_DATA,  DOOR_TYPE_ROOM,          BOUNDARY_N   },
    {GO_FOE_EXIT,                            BOSS_FOE_REAPER_5000_DATA,  DOOR_TYPE_PASSAGE,       BOUNDARY_N   },
    {0,                                      0,                          DOOR_TYPE_PASSAGE,       BOUNDARY_NONE}, // END
};

class instance_deadmines : public InstanceMapScript
{
    public:
        instance_deadmines() : InstanceMapScript("instance_deadmines", 36){}

        struct instance_deadmines_InstanceMapScript : public InstanceScript
        {
            instance_deadmines_InstanceMapScript(InstanceMap* map) : InstanceScript(map) {}

            void Initialize()
            {
                SetHeaders(DataHeader);
                SetBossNumber(MAX_ENCOUNTER);
                LoadDoorData(doorData);
                GlubtokGUID = 0;
                GlubtokGateGUID = 0;
                HelixGUID = 0;
                CaptainCookieGUID = 0;
                VanessaVanCleefGUID = 0;

                // vancleef event
                timerEvent = 0;
                eventStep = 0;
                MagmaVehicleGUID = 0;
                GlubtokEventGUID = 0;
                doorFOEExitGUID = 0;
                doorDefiasCanonGUID = 0;
                VanCleefEventGUID = 0;
                HelixEventGUID = 0;
                ReeperEventGUID = 0;

                dataFireWall = 1;
                dataProdigy = 1;
                achFailed = true;
				TeamInInstance = 0;
                _events.Reset();
                _events.SetPhase(PHASE_EVENT_NOT_STARTED);
            }

			void OnCreatureCreate(Creature* creature)
			{
				Map::PlayerList const &players = instance->GetPlayers();
				if (!players.isEmpty())
				{
					if (Player* player = players.begin()->getSource())
						TeamInInstance = player->GetTeam();
				}
				switch (creature->GetEntry())
				{
				case 46889: // Kagtha
					if (TeamInInstance == ALLIANCE)
						creature->UpdateEntry(42308, ALLIANCE); // Lieutenant Horatio Laine
					break;
				case 46902: // Miss Mayhem
					if (TeamInInstance == ALLIANCE)
						creature->UpdateEntry(491, ALLIANCE); // Quartermaster Lewis <Quartermaster>
					break;
				case 46903: // Mayhem Reaper Prototype
					if (TeamInInstance == ALLIANCE)
						creature->UpdateEntry(1, ALLIANCE); // GM WAYPOINT
					break;
				case 46906: // Slinky Sharpshiv
					if (TeamInInstance == ALLIANCE)
						creature->UpdateEntry(46612, ALLIANCE); // Lieutenant Horatio Laine
					break;
				case 46613: // Crime Scene Alarm-O-Bot
					if (TeamInInstance == HORDE)
						creature->UpdateEntry(1, HORDE); // GM WAYPOINT
					break;
				case 50595: // Stormwind Defender
					if (TeamInInstance == HORDE)
						creature->UpdateEntry(46890, HORDE); // Shattered Hand Assassin
					break;
				case 46614: // Stormwind Investigator
					if (TeamInInstance == HORDE)
						creature->UpdateEntry(1, HORDE); // GM WAYPOINT
					break;
                    case NPC_GLUBTOK:
                        GlubtokGUID = creature->GetGUID();
                        break;
                    case NPC_HELIX:
                        HelixGUID = creature->GetGUID();
                        break;
                    case NPC_CAPTAIN_COOKIE:
                        CaptainCookieGUID = creature->GetGUID();
                        break;
                    case NPC_VANCLEEF:
                        VanessaVanCleefGUID = creature->GetGUID();
                        break;
                    case NPC_MAGMA_VEHICLE:
                        MagmaVehicleGUID = creature->GetGUID();
                        break;
                    case NPC_NIGHTMARE_FLAMES:
                        if (_events.IsInPhase(PHASE_EVENT_GLUBTOK))
                            creature->CastSpell(creature, SPELL_NIGHTMARE_FLAME, true);
                        else
                            creature->RemoveAura(SPELL_NIGHTMARE_FLAME);
                        break;
                    case NPC_DARKWEB_DEVOURER:
                    case NPC_CHATTERING_HORROR:
                    case NPC_NIGHTMARE_SKITTERLING:
                        if (!_events.IsInPhase(PHASE_EVENT_HELIX))
                        {
                            creature->SetReactState(REACT_PASSIVE);
                            creature->SetVisible(false);
                        }
                        break;
                    case NPC_GLUBTOK_EVENT:
                        GlubtokEventGUID = creature->GetGUID();
                        break;
                    case NPC_VANCLEEF_EVENT_2:
                       VanCleefEventGUID = creature->GetGUID();
                       break;
                    case NPC_HELIX_EVENT:
                        HelixEventGUID = creature->GetGUID();
                        break;
                    case NPC_REEPER_EVENT:
                        ReeperEventGUID = creature->GetGUID();
                        break;
                   case NPC_VAPOR_VALVE:
                       creature->AddAura(94557, creature);
                       break;
                }
                CheckRecquirements();
            }

            void OnGameObjectCreate(GameObject* go)
            {
                switch (go->GetEntry())
                {
                    case GO_GLUBTOK_DOOR:
                    case GO_HELIX_DOOR_ENTRANCE:
                    case GO_HELIX_DOOR_EXIT:
                    case GO_FOE_ENTRANCE:
                        // this/21260/ door has to be open always as there is no guards
                        if (go->GetGUID() != 21260)
                        {
                            AddDoor(go, true);
                            // @hack @todo :: as the door state in db does not work (god knows why) let's hardcode it here
                            go->SetLootState(GO_READY);
                            go->UseDoorOrButton(10000, false);
                        }
                        break;
                    case GO_FOE_EXIT:
                        doorFOEExitGUID = go->GetGUID();
                        AddDoor(go, true);
                        // @hack @todo :: as the door state in db does not work (god knows why) let's hardcode it here
                        go->SetLootState(GO_READY);
                        go->UseDoorOrButton(10000, false);
                        break;
                    case GO_IRON_CLAD_DOOR:
                        doorDefiasCanonGUID = go->GetGUID();
                        AddDoor(go, true);
                        // @hack @todo :: as the door state in db does not work (god knows why) let's hardcode it here
                        go->SetLootState(GO_READY);
                        go->UseDoorOrButton(10000, false);
                        break;

                }
            }

            uint64 GetData64(uint32 data) const
            {
                switch (data)
                {
                    case BOSS_GLUBTOK_DATA:
                        return GlubtokGUID;
                    case BOSS_HELIX_DATA:
                        return HelixGUID;
                    case BOSS_CAPTAIN_COOKIE:
                        return CaptainCookieGUID;
                    case BOSS_GLUBTOK_EVENT_DATA:
                        return GlubtokEventGUID;
                    case BOSS_EVENT_VANCLEEF:
                        return VanCleefEventGUID;
                    default:
                        return 0;
                }
            }

            uint32 GetData(uint32 data) const
            {
                switch (data)
                {
                    case DATA_FIREWALL:
                        return dataFireWall;
                    case DATA_PRODIGY:
                        return dataProdigy;
                    default:
                        return 0;
                }
            }

            bool SetBossState(uint32 type, EncounterState state)
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;
                CheckRecquirements();
                if (!instance->IsHeroic() || (type == BOSS_EVENT_VANCLEEF && (state == DONE || state == DONE_HM)))
                    InitVancleefEvent(false);
                if (type == BOSS_VANCLEEF && state == IN_PROGRESS && !achFailed)
                    DoCompleteAchievement(ACHIEVEMENT_VIGOROUS_VANCLEEF_VINDICATOR);

                return true;
            }

            void CheckRecquirements()
            {
                if (Creature *captainCookie = instance->GetCreature(CaptainCookieGUID))
                    captainCookie->SetVisible(IsDone(BOSS_ADMIRAL_RISNARL));
                if (Creature *vvcleef = instance->GetCreature(VanessaVanCleefGUID))
                {
                    vvcleef->SetVisible(instance->IsHeroic() && IsDone(BOSS_EVENT_VANCLEEF) && IsDone(BOSS_CAPTAIN_COOKIE));
                    vvcleef->SetReactState((instance->IsHeroic() && IsDone(BOSS_EVENT_VANCLEEF) && IsDone(BOSS_CAPTAIN_COOKIE)) ? REACT_AGGRESSIVE : REACT_PASSIVE);
                }
            }

            void SetData(uint32 type, uint32 data)
            {
                switch (type)
                {
                    case DATA_PREPARE_VANCLEEF_EVENT:
                    {
                        _events.SetPhase(PHASE_EVENT_GLUBTOK);
                        instance->SummonCreature(NPC_VANCLEEF_EVENT, vancleefEventSpawn);
                        SetBossState(BOSS_EVENT_VANCLEEF, IN_PROGRESS);
                        _events.ScheduleEvent(CHECK_PLAYERS_ALIVE, 0);
                        _events.ScheduleEvent(CHECK_ACH_FAILED, 300000);
                        achFailed = false;
                        break;
                    }
                    case DATA_START_VANCLEEF_EVENT:
                    {
                        if (GameObject *go = instance->GetGameObject(doorFOEExitGUID))
                            go->SetGoState(GO_STATE_READY);
                        if (GameObject *go = instance->GetGameObject(doorDefiasCanonGUID))
                            go->SetGoState(GO_STATE_READY);
                        _events.ScheduleEvent(EVENT_VANCLEEF_EVENT, 0);
                        eventStep = 0;
                        timerEvent = 0;
                        break;
                    }
                    case DATA_END_VANCLEEF_EVENT:
                    {
                        if (GameObject *go = instance->GetGameObject(doorDefiasCanonGUID))
                            go->SetGoState(GO_STATE_ACTIVE);
                        if (GameObject *go = instance->GetGameObject(doorFOEExitGUID))
                            go->SetGoState(GO_STATE_ACTIVE);
                        break;
                    }
                    case DATA_GLUBTOK_EVENT:
                    {
                        switch (data)
                        {
                            case IN_PROGRESS:
                            {
                                break;
                            }
                            case DONE:
                            {
                                DoRemoveAurasDueToSpellOnPlayers(SPELL_AURA_NIGHTMARE_2);
                                InitVancleefEventFlames(false);
                                if (Creature *vc = instance->GetCreature(VanCleefEventGUID))
                                    vc->NearTeleportTo(Part1EventPos.GetPositionX(), Part1EventPos.GetPositionY(), Part1EventPos.GetPositionZ(), Part1EventPos.GetOrientation());
                                eventStep++;
                                timerEvent = 5000;
                                break;
                            }
                        }
                        break;
                    }
                    case DATA_HELIX_EVENT:
                    {
                        switch (data)
                        {
                            case IN_PROGRESS:
                            {
                                _events.SetPhase(PHASE_EVENT_HELIX);
                                InitVancleefEventSpiders(true);
                                break;
                            }
                            case DONE:
                            {
                                DoRemoveAurasDueToSpellOnPlayers(SPELL_AURA_NIGHTMARE_3);
                                InitVancleefEventLighting(true);
                                eventStep++;
                                timerEvent = 5000;
                                InitVancleefEventSpiders(false);
                                break;
                            }
                        }
                        break;
                    }
                    case DATA_REEPER_EVENT:
                    {
                        switch (data)
                        {
                            case IN_PROGRESS:
                            {
                                _events.SetPhase(PHASE_REEPER);
                                break;
                            }
                            case DONE:
                            {
                                DoRemoveAurasDueToSpellOnPlayers(SPELL_AURA_NIGHTMARE_4);
                                InitVancleefEventLighting(false);
                                eventStep++;
                                timerEvent = 5000;
                                break;
                            }
                        }
                        break;
                    }
                    case DATA_WORGEN_EVENT:
                    {
                        switch (data)
                        {
                            case IN_PROGRESS:
                            {
                                _events.SetPhase(PHASE_WORGEN);
                                break;
                            }
                            case DONE:
                            {
                                eventStep++;
                                timerEvent = 5000;
                                break;
                            }
                            case FAIL:
                            {
                                Map::PlayerList const &PlayerList = instance->GetPlayers();
                                if (!PlayerList.isEmpty())
                                    for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                                        if (Player *player = i->getSource())
                                            if (player->isAlive())
                                                player->Kill(player);
                                break;
                            }
                            default:
                                break;
                        }
                    }
                    case DATA_TRAP_COMPLETE:
                    {
                        switch (data)
                        {
                            case DONE:
                            {
                                break;
                            }
                            case FAIL:
                            {
                                Map::PlayerList const &PlayerList = instance->GetPlayers();
                                if (!PlayerList.isEmpty())
                                    for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                                        if (Player *player = i->getSource())
                                            if (player->isAlive())
                                                player->Kill(player);
                                break;
                            }
                            default:
                                break;
                        }
                    }
                    case DATA_FIREWALL:
                        dataFireWall = data;
                        break;
                    case DATA_PRODIGY:
                        dataProdigy = data;
                    default:
                        break;
                }
            }

            void InitVancleefEvent(bool start)
            {
                if (!start)
                {
                    DoRemoveAurasDueToSpellOnPlayers(SPELL_NIGHTMARE);
                    DoRemoveAurasDueToSpellOnPlayers(SPELL_NIGHTMARE_1);
                    DoRemoveAurasDueToSpellOnPlayers(SPELL_AURA_NIGHTMARE_2);
                    DoRemoveAurasDueToSpellOnPlayers(SPELL_AURA_NIGHTMARE_3);
                    DoRemoveAurasDueToSpellOnPlayers(SPELL_AURA_NIGHTMARE_4);
                    DoRemoveAurasDueToSpellOnPlayers(SPELL_AURA_NIGHTMARE_5);
                    DoRemoveAurasDueToSpellOnPlayers(SPELL_ADRENALINE);
                }
                InitVancleefEventFlames(start);
                InitVancleefEventSpiders(start);
                InitVancleefEventLighting(start);
            }

            void InitVancleefEventFlames(bool start)
            {
                // NPC_NIGHTMARE_FLAMES
                if (Creature *magmaVeh = instance->GetCreature(MagmaVehicleGUID))
                {
                    if (!magmaVeh->isAlive())
                        magmaVeh->Respawn(true);
                    std::list<Creature* > triggerList;
                    magmaVeh->GetCreatureListWithEntryInGrid(triggerList, NPC_NIGHTMARE_FLAMES, 500.0f);
                    for (std::list<Creature* >::iterator itr = triggerList.begin(); itr != triggerList.end(); itr++)
                        if (Creature *shadowFlames = *itr)
                        {
                            if (start)
                                shadowFlames->CastSpell(shadowFlames, SPELL_NIGHTMARE_FLAME, true);
                            else
                                shadowFlames->RemoveAura(SPELL_NIGHTMARE_FLAME);
                        }
                }
            }

            void InitVancleefEventSpiders(bool start)
            {
                // NPC_NIGHTMARE_FLAMES
                if (Creature *helix = instance->GetCreature(HelixEventGUID))
                {
                    std::list<Creature* > triggerList;
                    helix->GetCreatureListWithEntryInGrid(triggerList, NPC_DARKWEB_DEVOURER, 500.0f);
                    helix->GetCreatureListWithEntryInGrid(triggerList, NPC_CHATTERING_HORROR, 500.0f);
                    helix->GetCreatureListWithEntryInGrid(triggerList, NPC_NIGHTMARE_SKITTERLING, 500.0f);
                    for (std::list<Creature* >::iterator itr = triggerList.begin(); itr != triggerList.end(); itr++)
                        if (Creature *spider = *itr)
                        {
                            if (start)
                            {
                                spider->SetVisible(true);
                                spider->SetReactState(REACT_PASSIVE);
                            }
                            else
                            {
                                spider->SetVisible(false);
                                spider->SetReactState(REACT_PASSIVE);
                                if (!spider->IsInEvadeMode())
                                {
                                    spider->CombatStop();
                                    spider->AI()->EnterEvadeMode();
                                }
                            }
                        }
                }
            }

            void InitVancleefEventLighting(bool start)
            {
                if (Creature *vc = instance->GetCreature(VanCleefEventGUID))
                {
                    std::list<Creature* > triggerList;
                    vc->GetCreatureListWithEntryInGrid(triggerList, NPC_VANESSA_LIGHTING_PLATTER, 500.0f);
                    for (std::list<Creature* >::iterator itr = triggerList.begin(); itr != triggerList.end(); itr++)
                        if (Creature *light = *itr)
                            light->AI()->DoAction(start ? ACTION_ACTIVATE : ACTION_DESACTIVATE);
                    triggerList.clear();
                    if (!start)
                    {
                        vc->GetCreatureListWithEntryInGrid(triggerList, NPC_VANESSA_LIGHTING_PLATTER_PASSENGER, 500.0f);
                        for (std::list<Creature* >::iterator itr = triggerList.begin(); itr != triggerList.end(); itr++)
                            if (Creature *light = *itr)
                                light->DespawnOrUnsummon();
                    }
                }
            }

            void Update(uint32 diff)
            {
                _events.Update(diff);

                if (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_VANCLEEF_EVENT:
                        {
                            switch (eventStep)
                            {
                                case 0:
                                {
                                    Map::PlayerList const &PlayerList = instance->GetPlayers();
                                    if (!PlayerList.isEmpty())
                                        for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                                            if (Player *player = i->getSource())
                                                if (player->isAlive())
                                                    player->NearTeleportTo(platformJump.GetPositionX(), platformJump.GetPositionY(), platformJump.GetPositionZ(), platformJump.GetOrientation());
                                    timerEvent = 5000;
                                    break;
                                }
                                case 1:
                                {
                                    if (Creature *magmaVeh = instance->GetCreature(MagmaVehicleGUID))
                                        magmaVeh->AI()->DoAction(ACTION_INIT_EVENT);
                                    timerEvent = 4000;
                                    break;
                                }
                                case 2:
                                {
                                    if (Creature *magmaVeh = instance->GetCreature(MagmaVehicleGUID))
                                        magmaVeh->AI()->DoAction(ACTION_EMOTE_TRAP);
                                    timerEvent = 8000;
                                    break;
                                }
                                case 3:
                                {
                                    if (Creature *magmaVeh = instance->GetCreature(MagmaVehicleGUID))
                                        magmaVeh->AI()->DoAction(ACTION_EJECT_PASSENGERS);
                                    timerEvent = 1000;
                                    break;
                                }
                                case 4:
                                {
                                    if (Creature *me = instance->SummonCreature(49185, platformJump))
                                    {
                                        std::list<Creature* > triggerList;
                                        me->GetCreatureListWithEntryInGrid(triggerList, NPC_VAPOR_VALVE, 50.0f);
                                        for (std::list<Creature* >::iterator itr = triggerList.begin(); itr != triggerList.end(); itr++)
                                            if (Creature *valve = *itr)
                                                if (!valve->HasAura(SPELL_AURA_VAPOR_EVENT))
                                                {
                                                    SetData(DATA_TRAP_COMPLETE, FAIL);
                                                    break;
                                                }
                                    }
                                    DoCastSpellOnPlayers(SPELL_FORECAST_MAGMA_TRAP_THROW);
                                    timerEvent = 1000;
                                    break;
                                }
                                case 5:
                                {
                                    if (Creature *vc = instance->SummonCreature(NPC_VANCLEEF_EVENT_2, platformJump))
                                        vc->AI()->Talk(0);

                                    timerEvent = 6000;
                                    break;
                                }
                                case 6:
                                {
                                    DoCastSpellOnPlayers(SPELL_NIGHTMARE_SLOW);
                                    DoCastSpellOnPlayers(SPELL_NIGHTMARE);
                                    if (Creature *vc = instance->GetCreature(VanCleefEventGUID))
                                        vc->AI()->Talk(1);
                                    instance->SummonCreature(NPC_GLUBTOK_EVENT, Part1EventPos);
                                    timerEvent = 10000;
                                    break;
                                }
                                case 7:
                                {
                                    DoRemoveAurasDueToSpellOnPlayers(SPELL_NIGHTMARE);
                                    DoCastSpellOnPlayers(SPELL_AURA_NIGHTMARE_2);
                                    if (Creature *vc = instance->GetCreature(VanCleefEventGUID))
                                    {
                                        vc->AI()->Talk(2);
                                        vc->AI()->Talk(3);
                                    }
                                    InitVancleefEventFlames(true);
                                    timerEvent = 5000;
                                    break;
                                }
                                case 8:
                                {
                                    eventStep--;
                                    timerEvent = 5000;
                                    break;
                                }
                                case 9:
                                {
                                    DoCastSpellOnPlayers(SPELL_NIGHTMARE);
                                    Position pos =  Part1EventPos;
                                    pos.m_positionX += 1.0f;
                                    pos.m_positionY += 1.0f;
                                    instance->SummonCreature(NPC_HELIX_EVENT, pos);
                                    if (Creature *vc = instance->GetCreature(VanCleefEventGUID))
                                        vc->AI()->Talk(4);
                                    timerEvent = 7000;
                                    break;
                                }
                                case 10:
                                {
                                    if (Creature *vc = instance->GetCreature(VanCleefEventGUID))
                                        vc->AI()->Talk(5);
                                    timerEvent = 11000;
                                    break;
                                }
                                case 11:
                                {
                                    DoRemoveAurasDueToSpellOnPlayers(SPELL_NIGHTMARE);
                                    DoCastSpellOnPlayers(SPELL_AURA_NIGHTMARE_3);
                                    if (Creature *vc = instance->GetCreature(VanCleefEventGUID))
                                        vc->NearTeleportTo(Part2EventPos.GetPositionX(), Part2EventPos.GetPositionY(), Part2EventPos.GetPositionZ(), Part2EventPos.GetOrientation());
                                    if (Creature *hl = instance->GetCreature(HelixEventGUID))
                                        hl->AI()->DoAction(ACTION_START_FIGHT);
                                    timerEvent = 5000;
                                    break;
                                }
                                case 12:
                                {
                                    eventStep--;
                                    timerEvent = 5000;
                                    break;
                                }
                                case 13:
                                {
                                    if (GameObject *go = instance->GetGameObject(doorFOEExitGUID))
                                        go->SetGoState(GO_STATE_ACTIVE);
                                    DoCastSpellOnPlayers(SPELL_AURA_NIGHTMARE_4);
                                    Position pos =  Part3EventPos;
                                    pos.m_positionX += 1.0f;
                                    pos.m_positionY += 1.0f;
                                    instance->SummonCreature(NPC_REEPER_EVENT, pos);
                                    if (Creature *vc = instance->GetCreature(VanCleefEventGUID))
                                        vc->AI()->Talk(6);
                                    timerEvent = 8000;
                                    break;
                                }
                                case 14:
                                {
                                    if (Creature *vc = instance->GetCreature(VanCleefEventGUID))
                                        vc->AI()->Talk(7);
                                    timerEvent = 8000;
                                    break;
                                }
                                case 15:
                                {
                                    if (Creature *vc = instance->GetCreature(VanCleefEventGUID))
                                        vc->NearTeleportTo(Part3EventPos.GetPositionX(), Part3EventPos.GetPositionY(), Part3EventPos.GetPositionZ(), Part3EventPos.GetOrientation());
                                    if (Creature *vc = instance->GetCreature(VanCleefEventGUID))
                                        vc->AI()->Talk(8);
                                    if (Creature *rp = instance->GetCreature(ReeperEventGUID))
                                        rp->AI()->DoAction(ACTION_START_FIGHT);
                                    timerEvent = 5000;
                                    break;
                                }
                                case 16:
                                {
                                    eventStep--;
                                    timerEvent = 5000;
                                    break;
                                }
                                case 17:
                                {
                                    DoCastSpellOnPlayers(SPELL_NIGHTMARE);
                                    if (Creature *vc = instance->GetCreature(VanCleefEventGUID))
                                        vc->AI()->Talk(9);
                                    timerEvent = 10000;
                                    break;
                                }
                                case 18:
                                {
                                    if (Creature *vc = instance->GetCreature(VanCleefEventGUID))
                                        vc->AI()->Talk(10);
                                    timerEvent = 10000;
                                    break;
                                }
                                case 19:
                                {
                                    DoRemoveAurasDueToSpellOnPlayers(SPELL_NIGHTMARE);
                                    DoCastSpellOnPlayers(SPELL_AURA_NIGHTMARE_5);
                                    if (GameObject *go = instance->GetGameObject(doorDefiasCanonGUID))
                                        go->SetGoState(GO_STATE_ACTIVE);
                                    if (Creature *vc = instance->GetCreature(VanCleefEventGUID))
                                    {
                                        vc->AI()->Talk(11);
                                        vc->CastSpell(vc, SPELL_ADRENALINE, true);
                                        vc->DespawnOrUnsummon(1000);
                                    }
                                    instance->SummonCreature(NPC_EMMA_HARRINGTOM, PartHarringtom[0]);
                                    timerEvent = 1000;
                                    break;
                                }
                                case 20:
                                {
                                    eventStep--;
                                    timerEvent = 5000;
                                    break;
                                }
                                case 21:
                                {
                                    instance->SummonCreature(NPC_HERIC_HARRINGTOM, PartHarringtom[1]);
                                    timerEvent = 1000;
                                    break;
                                }
                                case 22:
                                {
                                    eventStep--;
                                    timerEvent = 5000;
                                    break;
                                }
                                case 23:
                                {
                                    instance->SummonCreature(NPC_CALLISA_HARRINGTOM, PartHarringtom[2]);
                                    timerEvent = 1000;
                                    break;
                                }
                                case 24:
                                {
                                    eventStep--;
                                    timerEvent = 5000;
                                    break;
                                }
                                default:
                                {
                                    _events.SetPhase(PHASE_EVENT_DONE);
                                    InitVancleefEvent(false);
                                    SetBossState(BOSS_EVENT_VANCLEEF, DONE);
                                    return;
                                }
                            }
                            eventStep++;
                            _events.ScheduleEvent(EVENT_VANCLEEF_EVENT, timerEvent);
                            break;
                        }
                        case CHECK_PLAYERS_ALIVE:
                        {
                            bool failed = true;
                            Map::PlayerList const &PlayerList = instance->GetPlayers();
                            if (!PlayerList.isEmpty())
                                for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                                    if (Player *player = i->getSource())
                                        if (player->isAlive())
                                        {
                                            failed = false;
                                            break;
                                        }
                            if (failed)
                            {
                                InitVancleefEvent(false);
                                _events.CancelEvent(EVENT_VANCLEEF_EVENT);
                                SetBossState(BOSS_EVENT_VANCLEEF, DONE);
                                return;
                            }
                            _events.ScheduleEvent(CHECK_PLAYERS_ALIVE, 1000);
                            break;
                        }
                        case CHECK_ACH_FAILED:
                        {
                            achFailed = true;
                        }
                        default:
                            break;
                    }
                }
            }

        private:
            uint64 GlubtokGUID;
            uint64 GlubtokGateGUID;
            uint64 HelixGUID;
            uint64 CaptainCookieGUID;
            uint64 VanessaVanCleefGUID;
            uint64 MagmaVehicleGUID;
            uint32 timerEvent;
            uint32 eventStep;
            uint64 GlubtokEventGUID;
            uint64 doorDefiasCanonGUID;
            uint64 doorFOEExitGUID;
            uint64 VanCleefEventGUID;
            uint64 HelixEventGUID;
            uint64 ReeperEventGUID;
            EventMap _events;

            uint32 dataFireWall;
            uint32 dataProdigy;
			uint32 TeamInInstance;
            bool achFailed;
        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_deadmines_InstanceMapScript(map);
        }
};

void AddSC_instance_deadmines()
{
    new instance_deadmines();
}
