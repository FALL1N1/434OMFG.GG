/*
 * Copyright (C) 2010 - 2012 ProjectSkyfire <http://www.projectskyfire.org/>
 *
 * Copyright (C) 2011 - 2012 ArkCORE <http://www.arkania.net/>
 * Copyright (C) 2008 - 2012 TrinityCore <http://www.trinitycore.org/>
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

#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "the_vortex_pinnacle.h"

Position sleapStreamPos[13] = {
    {-775.517f, -70.9323f, 640.3123f, 0.0f}, // 1 first boss
    {-848.227f, -68.724f, 654.2203f, 0.0f}, // 2 first boss
    {-844.885f, -205.135f, 660.7083f, 0.0f}, // 3 first boss
    {-901.36f, -177.33f, 664.6f, 0.0f}, //first arrival
    {-310.4583f, -29.74479f, 625.0833f, 0.0f}, // entrence left
    {-1190.88f, 125.203f, 737.6243f, 0.0f}, // 1 second boss
    {-1138.55f, 178.524f, 711.4943f, 0.0f}, // 2 second boss
    {-1245.21f, 230.986f, 690.6083f, 0.0f}, // 3 second boss
    {-1282.07f, 344.856f, 660.9873f, 0.0f}, // 4 second boss
    {-1229.64f, 412.26f, 641.2933f, 0.0f}, // 5 second boss
    {-1188.7f, 475.9f, 635.0f, 0.0f}, //second arrival
    {-382.441f, 42.31597f, 625.0833f, 0.0f}, // entrence right
    {-746.9566f, 529.1406f, 644.8316f, 0.0f} // 1 last boss
};

enum SleepStream
{
    SLEAPSTREAM_ERTAN_1 = 0,
    SLEAPSTREAM_ERTAN_2 = 1,
    SLEAPSTREAM_ERTAN_3 = 2,
    SLEAPSTREAM_ERTAN_ARRIVAL = 3,
    SLEAPSTREAM_ERTAN_INSTANCE_BEGIN = 4,
    SLEAPSTREAM_ALTAIRUS_1 = 5,
    SLEAPSTREAM_ALTAIRUS_2 = 6,
    SLEAPSTREAM_ALTAIRUS_3 = 7,
    SLEAPSTREAM_ALTAIRUS_4 = 8,
    SLEAPSTREAM_ALTAIRUS_5 = 9,
    SLEAPSTREAM_ALTAIRUS_ARRIVAL = 10,
    SLEAPSTREAM_ALTAIRUS_INSTANCE_BEGIN = 11,
    SLEAPSTREAM_ASAAD_1 = 12,
    MAX_SLIPSTREAM = 13,
};

#define NB_STARS 8
#define NPC_STAR 45932 // 45981
#define UNDER_MAP_ARRIVAL 42

class npc_slipstream : public CreatureScript
{
public:
    npc_slipstream() : CreatureScript("npc_slipstream") { }

    void generateSlipStreamMap(Player *player, Creature *me)
    {
        std::list<Creature*> slipStreams;
        player->GetCreatureListWithEntryInGrid(slipStreams, NPC_SLIPSTREAM, 533.0f);
        slipStreams.sort(Trinity::ObjectDistanceOrderPred(player));
        for (std::list<Creature*>::iterator itr = slipStreams.begin(); itr != slipStreams.end(); ++itr)
        {
            for (int i = 0; i < MAX_SLIPSTREAM; i++)
                if (CAST_AI(npc_slipstream::npc_slipstreamAI, (*itr)->AI())->isSleapStream((SleepStream)i))
                {
                    CAST_AI(npc_slipstream::npc_slipstreamAI, me->AI())->_slipStream[i] = (*itr)->GetGUID();
                    break;
                }
        }
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (InstanceScript* instance = creature->GetInstanceScript())
        {
            if (instance->IsDone(BOSS_ALTAIRUS))
            {
                if (CAST_AI(npc_slipstream::npc_slipstreamAI, creature->AI())->isSleapStream(SLEAPSTREAM_ALTAIRUS_1))
                {
                    generateSlipStreamMap(player, creature);
                    // There's an issue where it will get "stuck" at the fourth whirlwind, I'm not sure why it's happening so for now we use HACKY METHOD ! / VERADEV
                    player->TeleportTo(657, -1203.778320f, 467.539001f, 632.912109f, 0.559989f); // Hack until we figure out why it gets stuck at fourth whirlwind
                    // CAST_AI(npc_slipstream::npc_slipstreamAI, creature->AI())->AddSleapStreamEvent(player->GetGUID(), SLEAPSTREAM_ALTAIRUS_1);
                }
                else if (creature->GetPositionX() > -400 && creature->GetPositionX() < -300)
                {
                    player->NearTeleportTo(sleapStreamPos[10]);
                    return true;
                }
            }
            if (instance->IsDone(BOSS_GRAND_VIZIER_ERTAN))
            {
                if (CAST_AI(npc_slipstream::npc_slipstreamAI, creature->AI())->isSleapStream(SLEAPSTREAM_ERTAN_1))
                {
                    generateSlipStreamMap(player, creature);
                    CAST_AI(npc_slipstream::npc_slipstreamAI, creature->AI())->AddSleapStreamEvent(player->GetGUID(), SLEAPSTREAM_ERTAN_1);
                }
                else if (creature->GetPositionX() > -400 && creature->GetPositionX() < -300)
                {
                    player->NearTeleportTo(sleapStreamPos[3]);
                    return true;
                }
            }
        }
        player->PlayerTalkClass->ClearMenus();
        return true;
    }

    struct playerAction {
        uint64 _playerGUID;
        SleepStream _action;
        uint32 _sleapStreamTimer;
        bool _lastMove;
    };

    class MoveEnded {
    public :
        MoveEnded(Creature *c) : me(c) {}
        bool operator() (const playerAction &pa)
        {
            Unit *player = Unit::GetUnit(*me, pa._playerGUID);
            if (!player)
                return true;
            if (pa._lastMove)
                return false;
            return CAST_AI(npc_slipstream::npc_slipstreamAI, me->AI())->isSleapStreamWayEnded(pa._action);
        }

    private:
        Creature *me;
    };

    struct npc_slipstreamAI : public ScriptedAI
    {
        npc_slipstreamAI(Creature* creature) : ScriptedAI(creature), instance(creature->GetInstanceScript()) { }

        void Reset()
        {
            sleapStreamRefresh = 250;
        }

        void AddSleapStreamEvent(uint64 guid, SleepStream ident)
        {
            playerAction act;
            act._playerGUID = guid;
            act._action = ident;
            act._sleapStreamTimer = 2000;
            act._lastMove = false;
            bool canPush = true;
            for (std::list<playerAction >::iterator itr = passengersGUID.begin(); itr != passengersGUID.end(); itr++)
                if ((*itr)._playerGUID == guid)
                {
                    canPush = false;
                    break;
                }
            if (canPush)
                passengersGUID.push_back(act);
        }

        bool isSleapStream(SleepStream ident)
        {
            Position pos = sleapStreamPos[ident];
            if (me->GetDistance2d(pos.m_positionX, pos.m_positionY) < 2.0f)
                return true;
            return false;
        }

        bool isSleapStreamWayEnded(SleepStream ident)
        {
            switch (ident)
            {
                case SLEAPSTREAM_ERTAN_ARRIVAL:
                case SLEAPSTREAM_ALTAIRUS_ARRIVAL:
                case SLEAPSTREAM_ASAAD_1:
                case SLEAPSTREAM_ERTAN_INSTANCE_BEGIN:
                case SLEAPSTREAM_ALTAIRUS_INSTANCE_BEGIN:
                    return true;
                default:
                    break;
            }
            return false;
        }

        void UpdateSleapStreamEvent(const uint32 diff)
        {
            passengersGUID.remove_if(MoveEnded(me));
            for (std::list<playerAction >::iterator itr = passengersGUID.begin(); itr != passengersGUID.end(); itr++)
            {
                if ((*itr)._sleapStreamTimer <= diff)
                {
                    if (Unit *player = Unit::GetUnit(*me, (*itr)._playerGUID))
                    {
                        if ((*itr)._action != SLEAPSTREAM_ERTAN_ARRIVAL && (*itr)._action != SLEAPSTREAM_ALTAIRUS_ARRIVAL)
                        {
                            if (Creature *sl = Unit::GetCreature(*me, _slipStream[(int)(*itr)._action]))
                            {
                                if (sl->GetVehicleKit())
                                    if (sl->GetVehicleKit()->GetPassenger(0) != NULL)
                                    {
                                        (*itr)._sleapStreamTimer = 1000;
                                        continue;
                                    }
                                player->ExitVehicle();
                                player->CastSpell(sl, 84988, true);
                            }
                        }
                        else
                        {
                            player->ExitVehicle(&sleapStreamPos[(*itr)._action]);
                        }
                    }
                    int act = (*itr)._action;
                    act++;
                    if (isSleapStreamWayEnded((SleepStream)act))
                    {
                        (*itr)._lastMove = !(*itr)._lastMove;
                        if ((*itr)._lastMove)
                            (*itr)._action = (SleepStream)act;
                    }
                    else
                        (*itr)._action = (SleepStream)act;
                    (*itr)._sleapStreamTimer = 2000;
                }
                else
                    (*itr)._sleapStreamTimer -= diff;
            }
        }

        void MovementInform(uint32 , uint32 point)
        {
            if (point == UNDER_MAP_ARRIVAL)
            {
                me->DespawnOrUnsummon(1000);
            }
        }

        void SetGUID(uint64 guid, int32 )
        {
            me->RemoveAllAuras();
            me->SetSpeed(MOVE_FLIGHT, 3.0f);
            if (Player *player = Unit::GetPlayer(*me, guid))
                player->CastSpell(me, 84988, true);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void UpdateAI(const uint32 diff)
        {
            if (sleapStreamRefresh <= diff)
            {
                if (me->GetVehicleKit())
                    if (Unit* passenger = me->GetVehicleKit()->GetPassenger(0))
                    {
                        Position pos(*me);
                        passenger->Relocate(&pos);
                    }

                UpdateSleapStreamEvent(diff + 250);
                sleapStreamRefresh = 500;
            }
            else
                sleapStreamRefresh -= diff;
        }

        std::list<playerAction > passengersGUID;
        std::map<int, uint64 > _slipStream;
    private :
        uint32 sleapStreamRefresh;
        InstanceScript *instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_slipstreamAI (creature);
    }
};

class mob_vortex_wind_rotation_manager : public CreatureScript
{
public:
    mob_vortex_wind_rotation_manager() : CreatureScript("mob_vortex_wind_rotation_manager") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_vortex_wind_rotation_managerAI(pCreature);
    }

    struct mob_vortex_wind_rotation_managerAI : public ScriptedAI
    {
        mob_vortex_wind_rotation_managerAI(Creature* pCreature) : ScriptedAI(pCreature),  Summons(me)
        {
            instance = pCreature->GetInstanceScript();
        }

        void Reset()
        {
            m_timer = 0;
            radius = 14;
            own_stars.clear();
            orient = 0.0f;
            Summons.DespawnAll();
            for (int i = 0; i < NB_STARS; i++)
            {
                orient += 2 * M_PI / NB_STARS;
                float x, y;
                me->GetNearPoint2D(x, y, radius, orient);
                me->SummonCreature(NPC_STAR, x, y, me->GetPositionZ(), 0.0f, TEMPSUMMON_MANUAL_DESPAWN);
            }
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            start = false;
            me->AddAura(85719, me);
        }

        void JustDied(Unit* /*who*/)
        {
            Summons.DespawnAll();
        }

        void DamageTaken(Unit* pDone_by, uint32& uiDamage)
        {
            uiDamage = 0;
        }

        void AttackStart(Unit *who)
        {
            return;
        }

        void JustSummoned(Creature* summoned)
        {
            own_stars.push_back(summoned->GetGUID());
            Summons.Summon(summoned);
        }

        void checkForDespawn(const uint32 diff)
        {
            if (mui_timer_despawn <= diff)
            {
                bool despawn = true;
                for (std::list<uint64 >::iterator itr = own_stars.begin(); itr != own_stars.end(); itr++)
                    if (Creature *vortex = Unit::GetCreature(*me, (*itr)))
                        if (vortex->isAlive())
                        {
                            despawn = false;
                            break;
                        }
                if (despawn)
                    me->DespawnOrUnsummon();
                mui_timer_despawn = 1000;
            }
            else
                mui_timer_despawn -= diff;
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (me->GetDistance2d(who) < 25.0f)
                start = true;
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!start)
                return;

            checkForDespawn(uiDiff);

            if (m_timer <= uiDiff)
            {
                orient = fmod(orient + 0.1f, 2.0f * static_cast<float >(M_PI));
                for (std::list<uint64 >::iterator itr = own_stars.begin(); itr != own_stars.end(); itr++)
                {
                    if (Creature *vortex = Unit::GetCreature(*me, (*itr)))
                    {
                        float x,y;
                        orient +=  2 * M_PI / NB_STARS;
                        me->GetNearPoint2D(x, y, radius, orient);
                        vortex->GetMotionMaster()->Clear();
                        vortex->GetMotionMaster()->MovePoint(0, x, y,  me->GetPositionZ());
                    }
                }
                m_timer = 200;
            }
            else m_timer -= uiDiff;
        }

    private :
        InstanceScript* instance;
        SummonList Summons;
        std::list<uint64 > own_stars;

        uint32 m_timer;
        uint32 radius;
        uint32 mui_timer_despawn;

        float orient;

        bool start;
    };
};

class mob_skyfall_star : public CreatureScript
{
public:
    mob_skyfall_star() : CreatureScript("mob_skyfall_star") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_skyfall_starAI(pCreature);
    }

    struct mob_skyfall_starAI : public ScriptedAI
    {
       mob_skyfall_starAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
        }

        void Reset()
        {
            m_timer = 0;
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim())
                return;

            if (m_timer <= uiDiff)
            {
                DoCastRandom(87854, 0.0f);
                m_timer = urand(3000, 10000);
            }
            else m_timer -= uiDiff;
        }

    private :
        InstanceScript* instance;
        uint32 m_timer;
    };
};

class mob_wild_vortex : public CreatureScript
{
public:
    mob_wild_vortex() : CreatureScript("mob_wild_vortex") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_wild_vortexAI(pCreature);
    }

    struct mob_wild_vortexAI : public ScriptedAI
    {
       mob_wild_vortexAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
        }

        void Reset()
        {
            m_timer = 0;
            m_timer1 = 2000;
            m_timer2 = 4000;
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (m_timer <= uiDiff)
            {
                DoCastVictim(88032);
                m_timer = urand(2000, 4000);
            }
            else m_timer -= uiDiff;


            if (m_timer1 <= uiDiff)
            {
                DoCastRandom(88029, 0.0f);
                m_timer1 = urand(5000, 6000);
            }
            else m_timer1 -= uiDiff;


            if (m_timer2 <= uiDiff)
            {
                if (Unit* unit = SelectTarget(SELECT_TARGET_RANDOM, 0, NonTankTargetSelector(me)))
                    me->CastSpell(unit, 88010);
                m_timer2 = urand(3000, 10000);
            }
            else m_timer2 -= uiDiff;

            DoMeleeAttackIfReady();
        }

    private :
        InstanceScript* instance;
        uint32 m_timer;
        uint32 m_timer1;
        uint32 m_timer2;
    };
};

class mob_golden_orb : public CreatureScript
{
public:
    mob_golden_orb() : CreatureScript("mob_golden_orb") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_golden_orbAI(pCreature);
    }

    struct mob_golden_orbAI : public ScriptedAI
    {
        mob_golden_orbAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
        }

        void Reset()
        {
            updateAch = false;
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (updateAch || !IsHeroic() || me->GetDistance2d(who) > 1.0f)
                return;
            instance->SetData(DATA_GOLDEN_ORB, DONE);
            me->DespawnOrUnsummon();
            updateAch = true;
        }

        void UpdateAI(const uint32 uiDiff)
        {
        }

    private :
        InstanceScript* instance;
        bool updateAch;
    };
};


void AddSC_vortex_pinnacle()
{
    new npc_slipstream();
    new mob_vortex_wind_rotation_manager();
    new mob_skyfall_star();
    new mob_wild_vortex();
    new mob_golden_orb();
}
