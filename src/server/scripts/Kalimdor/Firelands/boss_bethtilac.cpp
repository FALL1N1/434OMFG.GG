/*
*

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

/*
##### Script Info #####
Author: dimiandre
Progress: 90%
*/

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "PassiveAI.h"
#include "SpellScript.h"
#include "MoveSplineInit.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "firelands.h"

enum Spells
{
    // Bethilac
    SPELL_EMBER_FLARE = 98934,
    SPELL_METEOR_BURN = 99073,
    SPELL_CONSUME = 100634,
    SPELL_SMOLDERING_DEVASTATION = 99052,
    SPELL_VENOM_RAIN = 99333,
    SPELL_METEOR_BURN_VISUAL = 99071,
    SPELL_FRENZY = 99497,
    SPELL_KISS = 99476,

    SPELL_THE_WIDOWS_KISS = 99506,

    // Ciderweb Spinner
    SPELL_BURNING_ACID = 98471,
    SPELL_FIERY_WEB_SPIN = 99822,
    SPELL_FIXATE = 99559,

    // Heroic spells
    SPELL_VOLATILE_BURST = 99990,

    //Cinderweb Spiderling
    SPELL_SEEPING_VENOM = 97079,

    // Engorged Broodling
    SPELL_VOLATILE_BURST_H = 99990,

    SPELL_BOILING_SPATTER = 99463,

    SPELL_SPIDERWEB_FILAMENT_1 = 98149, // Graphic of ground web signal
    SPELL_SPIDERWEB_FILAMENT_2 = 98623, // Graphic filament (to any target)
    SPELL_SPIDERWEB_FILAMENT_3 = 98153, // Graphic filamente (pet to summoner)
};

enum Events
{
    EVENT_SUMMON_CINDERWEB_SPINNER = 1,
    EVENT_VENOM_RAIN,
    EVENT_DECREASE_ENERGY,
    EVENT_SMOLDERING_DEVASTATION,
    EVENT_METEOR_BURN,
    EVENT_SUMMON_SPILDERING,
    EVENT_SUMMON_DRONE,
    EVENT_FRENZY,
    EVENT_KISS,
    EVENT_EMBER_FLARE,
    GO_HOME
};

// Grounds
const float groundLow = 75.0f;
const float groundUp = 112.0f;

Position const cinderwebSummonPos[7] =
{
    { 70.372f, 378.78f, 74.07f, 2.47f },
    { 52.038f, 364.16f, 74.07f, 2.47f },
    { 29.274f, 371.97f, 74.07f, 2.47f },
    { 17.162f, 387.65f, 74.07f, 2.47f },
    { 14.612f, 414.66f, 74.07f, 2.47f },
    { 55.102f, 419.37f, 74.07f, 2.47f },
    { 68.198f, 406.28f, 76.07f, 2.47f },
};

Position const spilderingSummonPos[3] =
{
    { 23.119f, 296.747f, 82.76f, 0.85f },
    { 134.86f, 359.464f, 85.50f, 3.50f },
    { 97.986f, 451.187f, 86.21f, 3.73f },
};

Position const droneSummonPos[3] =
{
    { 6.039f, 408.18f, 74.32f, 5.29f },
    { 6.46f, 374.25f, 74.04f, 0.20f },
};

class boss_bethtilac : public CreatureScript
{
public:
    boss_bethtilac() : CreatureScript("boss_bethtilac") {}

    struct boss_bethtilacAI : public BossAI
    {
        boss_bethtilacAI(Creature * creature) : BossAI(creature, DATA_BETHTILAC), summons(me)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        SummonList summons;
        uint32 phase;
        uint8 numDevastation;
        uint8 spinnerCount;

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);

            if (summon->GetEntry() == 53450)
            {
                summon->AddAura(SPELL_METEOR_BURN_VISUAL, summon);
                return;
            }

            DoZoneInCombat(summon);
        }

        void Reset()
        {
            _Reset();
            events.Reset();
            summons.DespawnAll();
            me->SetCanRegen(false);
            me->setPowerType(POWER_ENERGY);
            me->SetMaxPower(POWER_ENERGY, 100);
            me->SetPower(POWER_ENERGY, 100);

            numDevastation = 0;
            phase = 0;

            spinnerCount = 0;

            if (instance)
                instance->SetData(DATA_BETHTILAC_EVENT, NOT_STARTED);
        }

        void EnterCombat(Unit* victim)
        {
            _EnterCombat();
            DoZoneInCombat(me);

            if (instance)
                instance->SetData(DATA_BETHTILAC_EVENT, IN_PROGRESS);

            numDevastation = 0;
            phase = 0;
            spinnerCount = 0;

            // Teleport Bethtilac to up floor
            me->NearTeleportTo(71.83f, 357.11f, 112.0f, 1.73f);
            events.ScheduleEvent(EVENT_VENOM_RAIN, urand(3000, 4000), 0);
            events.ScheduleEvent(EVENT_SUMMON_CINDERWEB_SPINNER, 12000, 0);
            events.ScheduleEvent(EVENT_DECREASE_ENERGY, 1000, 0);
            events.ScheduleEvent(EVENT_METEOR_BURN, urand(15000, 18000), 0);
            events.ScheduleEvent(EVENT_SUMMON_SPILDERING, 14000, 0);
            events.ScheduleEvent(EVENT_SUMMON_DRONE, 45000, 0);
            events.ScheduleEvent(EVENT_EMBER_FLARE, urand(12000, 15000), 1);
        }

        void AttackStart(Unit* who)
        {
            if (!who)
                return;

            if (phase == 0)
                AttackStartNoMove(who);
            else
                ScriptedAI::AttackStart(who);
        }

        void JustDied(Unit* killer)
        {
            _JustDied();
            summons.DespawnAll();
            if (me->GetMap()->IsHeroic())
            {// this will give the achievement to players in heroic difficulty
                Map::PlayerList const& PlayerList = instance->instance->GetPlayers();
                AchievementEntry const* bethtilac = sAchievementStore.LookupEntry(5807);
                for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                {
                    if (Player* player = itr->getSource())
                    {
                        player->CompletedAchievement(bethtilac);
                    }
                }
            }

            if (instance)
                instance->SetData(DATA_BETHTILAC_EVENT, DONE);

            me->NearTeleportTo(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY(), me->GetHomePosition().GetPositionZ() + 0.5f, me->GetHomePosition().GetOrientation());
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                case EVENT_VENOM_RAIN:
                    if (me->getVictim() && me->GetDistance(me->getVictim()) > 3.6f)
                        DoCastAOE(SPELL_VENOM_RAIN, true);

                    events.ScheduleEvent(EVENT_VENOM_RAIN, urand(2000, 3000), 0);
                    break;
                case EVENT_SUMMON_CINDERWEB_SPINNER:
                    me->SummonCreature(53642, cinderwebSummonPos[urand(0, 6)]);
                    spinnerCount++;

                    if (spinnerCount <= 6)
                        events.ScheduleEvent(EVENT_SUMMON_CINDERWEB_SPINNER, 2000, 0);
                    else
                    {
                        spinnerCount = 0;
                    }
                    break;
                case EVENT_DECREASE_ENERGY:
                {
                    int32 energy = me->GetPower(POWER_ENERGY);

                    if (energy == 0)
                    {
                        events.ScheduleEvent(EVENT_SMOLDERING_DEVASTATION, 1000);
                        energy = 100;
                        numDevastation++;
                    }
                    else
                    {
                        energy -= 1;
                    }

                    me->SetPower(POWER_ENERGY, energy);

                    if (numDevastation >= 3 && phase == 0)
                    {
                        events.CancelEventGroup(0);
                        events.CancelEvent(EVENT_VENOM_RAIN);
                        events.CancelEvent(EVENT_SUMMON_CINDERWEB_SPINNER);
                        events.CancelEvent(EVENT_METEOR_BURN);
                        events.CancelEvent(EVENT_SUMMON_SPILDERING);
                        events.CancelEvent(EVENT_SUMMON_DRONE);

                        events.ScheduleEvent(EVENT_FRENZY, 5000, 1);
                        events.ScheduleEvent(EVENT_KISS, urand(12000, 15000), 1);
                        events.ScheduleEvent(GO_HOME, 4000, 1);
                        phase = 1;
                    }

                    if (phase == 0)
                        events.ScheduleEvent(EVENT_DECREASE_ENERGY, 1000, 0);
                    break;
                }
                case GO_HOME:
                    me->NearTeleportTo(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY(), me->GetHomePosition().GetPositionZ(), me->GetHomePosition().GetOrientation(), false);
                    break;
                case EVENT_SMOLDERING_DEVASTATION:
                    DoCastAOE(SPELL_SMOLDERING_DEVASTATION);

                    if (phase == 0)
                    {
                        events.ScheduleEvent(EVENT_SUMMON_CINDERWEB_SPINNER, 20000, 0);
                        events.ScheduleEvent(EVENT_SUMMON_DRONE, urand(30000, 35000), 0);
                    }
                    break;
                case EVENT_METEOR_BURN:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM_AT_THIS_FLOOR, 0, 100.0f, true, 0))
                        me->CastSpell(target, SPELL_METEOR_BURN, true);

                    events.ScheduleEvent(EVENT_METEOR_BURN, urand(15000, 18000), 0);
                    break;
                case EVENT_SUMMON_SPILDERING:
                {
                    uint8 pos = urand(0, 2);
                    for (uint8 i = 0; i < 5; i++)
                    {
                        if (RAID_MODE(0, 1) == 1)
                            pos = urand(0, 2);

                        if (Creature* spider = me->SummonCreature(53631, spilderingSummonPos[pos]))
                        {
                            spider->SetSpeed(MOVE_WALK, 0.3f, true);
                            spider->SetSpeed(MOVE_RUN, 0.3f, true);
                        }
                    }
                    // Spawn Broodling in heroic version
                    if (me->GetMap()->IsHeroic())
                    {
                        for (uint8 i = 0; i < 3; i++)
                        {
                            if (RAID_MODE(0, 1) == 1)
                                pos = urand(0, 2);

                            if (Creature* spider = me->SummonCreature(53745, spilderingSummonPos[pos]))
                            {
                                spider->SetSpeed(MOVE_WALK, 0.3f, true);
                                spider->SetSpeed(MOVE_RUN, 0.3f, true);
                            }
                        }
                    }
                    events.ScheduleEvent(EVENT_SUMMON_SPILDERING, 30000, 0);
                    break;
                }
                case EVENT_SUMMON_DRONE:
                    me->SummonCreature(52581, droneSummonPos[urand(0, 1)]);
                    break;
                case EVENT_EMBER_FLARE:
                    DoCastAOE(SPELL_EMBER_FLARE, true);
                    events.ScheduleEvent(EVENT_EMBER_FLARE, urand(4000, 5000), 1);
                    break;
                case EVENT_FRENZY:
                    me->AddAura(SPELL_FRENZY, me);
                    events.ScheduleEvent(EVENT_FRENZY, 5000, 1);
                    break;
                case EVENT_KISS:
                    me->CastSpell(me->getVictim(), SPELL_KISS, true);
                    events.ScheduleEvent(EVENT_KISS, urand(12000, 15000), 1);
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_bethtilacAI(creature);
    }
};

enum spinnerEvents
{
    EVENT_BURNING_ACID = 1,
    EVENT_FIERY_WEB_SPIN,
};

class npc_cinderweb_spinner : public CreatureScript
{
public:
    npc_cinderweb_spinner() : CreatureScript("npc_cinderweb_spinner")
    { }

    struct npc_cinderweb_spinnerAI : public ScriptedAI
    {
        npc_cinderweb_spinnerAI(Creature * creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void AttackStart(Unit* who)
        {
            if (!who)
                return;

            if (who->GetPositionZ() > 100.0f)
            {
                me->getThreatManager().addThreat(who, -90000000);
                AttackStartNoMove(who);
            }
            else
                ScriptedAI::AttackStart(who);
        }

        void JustDied(Unit* killer)
        {
            Position pos;
            me->GetPosition(&pos);
            if (Creature* filament = me->SummonCreature(53082, pos, TEMPSUMMON_TIMED_DESPAWN, 22000, 0))
            {
                filament->SetReactState(REACT_PASSIVE);

                filament->AddAura(SPELL_SPIDERWEB_FILAMENT_1, filament);
                pos.m_positionZ += 39.0f;

                if (Creature* dest = filament->SummonCreature(53237, pos, TEMPSUMMON_TIMED_DESPAWN, 22000, 0))
                {
                    dest->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NOT_SELECTABLE);
                    filament->CastSpell(dest, SPELL_SPIDERWEB_FILAMENT_2, true);
                }
            }
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_BURNING_ACID, urand(6000, 10000));

            if (me->GetMap()->IsHeroic())
                events.ScheduleEvent(EVENT_FIERY_WEB_SPIN, urand(15000, 17000));
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                case EVENT_BURNING_ACID:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM_AT_THIS_FLOOR, 0, 6.0f, true))
                        me->CastSpell(target, SPELL_BURNING_ACID, true);
                    else
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            me->CastSpell(target, SPELL_BURNING_ACID, true);
                    }
                    events.ScheduleEvent(EVENT_BURNING_ACID, urand(12000, 16000));
                    break;
                case EVENT_FIERY_WEB_SPIN:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM_AT_THIS_FLOOR, 0, 100.0f, true))
                        me->CastSpell(target, SPELL_FIERY_WEB_SPIN, true);
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_cinderweb_spinnerAI(creature);
    }
};


class npc_spirderweb_filament : public CreatureScript
{
public:
    npc_spirderweb_filament() : CreatureScript("npc_spirderweb_filament")
    { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (!player || !creature)
            return false;

        Position pos;
        creature->GetPosition(&pos);
        pos.m_positionZ += 40.0f;

        player->GetMotionMaster()->MoveJump(pos, 13.0f, 13.7f);
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        creature->RemoveAura(SPELL_SPIDERWEB_FILAMENT_1);
        return true;
    }

    bool OnGossipSelect(Player* pPlayer, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
    {
        return true;
    }
};

enum spiderlingEvents
{
    EVENT_GET_TARGET = 1,
    EVENT_VENOM,
};

class npc_cinderweb_spiderling : public CreatureScript
{
public:
    npc_cinderweb_spiderling() : CreatureScript("npc_cinderweb_spiderling")
    { }

    struct npc_cinderweb_spiderlingAI : public ScriptedAI
    {
        npc_cinderweb_spiderlingAI(Creature * creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;
        bool venom;

        void Reset()
        {
            events.Reset();
            venom = false;
        }

        void AttackStart(Unit* who)
        {
            if (!who)
                return;

            if (who->GetPositionZ() > 100.0f)
            {
                me->getThreatManager().addThreat(who, -90000000);
                AttackStartNoMove(who);
            }
            else
                ScriptedAI::AttackStart(who);
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_GET_TARGET, 1);
            me->SetReactState(REACT_PASSIVE);
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!who)
                return;

            if (venom)
                return;

            if (me->GetDistance(who) > 5.0f)
                return;

            venom = true;
            me->CastSpell(who, SPELL_SEEPING_VENOM, true);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                case EVENT_GET_TARGET:
                    if (Creature* drone = me->FindNearestCreature(RAID_MODE(52581, 53582, 53583, 53584), 100.0f, true))
                        me->GetMotionMaster()->MoveChase(drone);
                    else
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM_AT_THIS_FLOOR, 0, 100.0f, true, 0))
                            me->GetMotionMaster()->MoveChase(target);
                    }

                    events.ScheduleEvent(EVENT_GET_TARGET, 1000);
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_cinderweb_spiderlingAI(creature);
    }
};

enum droneEvents
{
    EVENT_BOILING_SPATTER = 1,
    EVENT_BURNIGN_ACID_DRONE,
    EVENT_FIXATE
};
class npc_cinderweb_drone : public CreatureScript
{
public:
    npc_cinderweb_drone() : CreatureScript("npc_cinderweb_drone")
    { }

    struct npc_cinderweb_droneAI : public ScriptedAI
    {
        npc_cinderweb_droneAI(Creature * creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!who)
                return;

            if (!who->ToCreature())
                return;

            if (me->GetDistance(who) > 5.0f)
                return;

            if (who->GetEntry() == RAID_MODE(53631, 53579, 53580, 53581))
            {
                who->ToCreature()->DisappearAndDie();
                me->CastSpell(me, SPELL_CONSUME, true);
            }
        }

        void AttackStart(Unit* who)
        {
            if (!who)
                return;

            if (who->GetPositionZ() > 100.0f)
            {
                me->getThreatManager().addThreat(who, -90000000);
                AttackStartNoMove(who);
            }
            else
                ScriptedAI::AttackStart(who);
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_BOILING_SPATTER, urand(20000, 25000));
            events.ScheduleEvent(EVENT_BURNIGN_ACID_DRONE, urand(6000, 8000));
            events.ScheduleEvent(EVENT_FIXATE, urand(25000, 30000));
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                case EVENT_BURNIGN_ACID_DRONE:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM_AT_THIS_FLOOR, 0, 6.0f, true, -SPELL_SEEPING_VENOM))
                        me->CastSpell(target, SPELL_BURNING_ACID, true);
                    else
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            me->CastSpell(target, SPELL_BURNING_ACID, true);
                    }

                    events.ScheduleEvent(EVENT_BURNIGN_ACID_DRONE, urand(6000, 8000));
                    break;
                case EVENT_BOILING_SPATTER:
                    me->CastSpell(me->getVictim(), SPELL_BOILING_SPATTER, true);

                    events.ScheduleEvent(EVENT_BOILING_SPATTER, urand(20000, 25000));
                    break;
                case EVENT_FIXATE:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM_AT_THIS_FLOOR, 0, 6.0f, true))
                    {
                        me->getThreatManager().addThreat(target, 80000000);
                        AttackStart(target);
                        me->AddAura(SPELL_FIXATE, me);
                    }

                    events.ScheduleEvent(EVENT_FIXATE, urand(30000, 35000));
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_cinderweb_droneAI(creature);
    }
};


enum broodlingEvents
{
    EVENT_ZERO = 1,
};

class npc_engorged_broodling : public CreatureScript
{
public:
    npc_engorged_broodling() : CreatureScript("npc_engorged_broodling")
    { }

    struct npc_engorged_broodlingAI : public ScriptedAI
    {
        npc_engorged_broodlingAI(Creature * creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;
        uint32 fixedTarget;

        void Reset()
        {
            events.Reset();
            fixedTarget = 0;
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!who)
                return;

            if (!who->ToPlayer())
                return;

            if (me->GetDistance2d(who) > 6.0f)
                return;

            if (who->GetGUIDLow() != fixedTarget)
                return;

            me->CastSpell(me, SPELL_VOLATILE_BURST, true);
        }

        void EnterCombat(Unit* who)
        {
            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM_AT_THIS_FLOOR, 0, 6.0f, true))
            {
                me->getThreatManager().addThreat(target, 80000000);
                AttackStart(target);
                fixedTarget = target->GetGUID();
            }
            else
            {
                me->getThreatManager().addThreat(who, 80000000);
                fixedTarget = who->GetGUID();
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_engorged_broodlingAI(creature);
    }
};

void AddSC_boss_bethtilac()
{
    new boss_bethtilac();
    new npc_cinderweb_spinner();
    new npc_spirderweb_filament();
    new npc_cinderweb_spiderling();
    new npc_cinderweb_drone();
    new npc_engorged_broodling();
}