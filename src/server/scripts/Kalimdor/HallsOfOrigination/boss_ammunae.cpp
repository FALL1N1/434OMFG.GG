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

#include "GridNotifiers.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "halls_of_origination.h"

enum Spells
{
    //Ammunae
    SPELL_WITHER                           = 76043,
    SPELL_CONSUME_LIFE_DAMAGE_EFFECT       = 79767,
    SPELL_CONSUME_LIFE_ENERGIZE_EFFECT     = 75665,
    SPELL_CONSUME_LIFE_POWER_BURN_EFFECT   = 75666,
    SPELL_RAMPANT_GROWTH                   = 75790,
    //Seeding Pod
    SPELL_ENERGIZE                         = 75657,
    SPELL_ENERGIZING_GROWTH                = 89123,
    //Bloodpetal
    SPELL_THORN_SLASH                      = 76044,
    //Spore
    SPELL_NOXIOUS_SPORE                    = 75702,
    SPELL_SPORE_CLOUD                      = 75701,
};

enum AmunaeTexts
{
    SAY_AGGRO                = 0,
    SAY_GROWTH               = 1,
    SAY_SLAY                 = 2,
    SAY_DEATH                = 3,
};

enum Events
{
    //Ammunae
    EVENT_WITHER                = 1,
    EVENT_CONSUME_LIFE          = 2,
    EVENT_RAMPANT_GROWTH        = 3,
    EVENT_SUMMON_POD            = 4,
    EVENT_SUMMON_SPORE          = 5,
    EVENT_ENERGY_TICKER         = 6,
    EVENT_ENERGY_TICKER_STOP    = 7,
    EVENT_COMBAT                = 8,
    //Blossom
    EVENT_THORN_SLASH           = 9,
    EVENT_EMERGE                = 10,
    EVENT_ATTACK                = 11,
    //Seedling Pod
    EVENT_ENERGIZE              = 12,
};

class boss_ammunae : public CreatureScript
{
    public:
        boss_ammunae() : CreatureScript("boss_ammunae") { }

        struct boss_ammunaeAI : public BossAI
        {
            boss_ammunaeAI(Creature* creature) : BossAI(creature, DATA_AMMUNAE)
            {
                me->Respawn(true);
            }

            void Reset()
            {
                _Reset();
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                DoCast(me, 69470, true);
                me->SetPower(POWER_ENERGY, 0);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(SAY_AGGRO);
                events.ScheduleEvent(EVENT_WITHER, 10000);
                events.ScheduleEvent(EVENT_CONSUME_LIFE, 4000);
                events.ScheduleEvent(EVENT_SUMMON_POD, 3000);
                events.ScheduleEvent(EVENT_RAMPANT_GROWTH, 15000);
                events.ScheduleEvent(EVENT_SUMMON_SPORE, 15000);
            }

            void KilledUnit(Unit* victim)
            {
                Talk(SAY_SLAY);
            }

            void RampartSummon(uint32 entry, float distance)
            {
                std::list<Creature*> CreatureList;
                Trinity::AllCreaturesOfEntryInRange checker(me, entry, distance);
                Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange> searcher(me, CreatureList, checker);
                me->VisitNearbyObject(distance, searcher);
                if (CreatureList.empty())
                    return;

                std::list<Creature*>::iterator itr = CreatureList.begin();
                uint32 count = CreatureList.size();
                for (std::list<Creature*>::iterator iter = CreatureList.begin(); iter != CreatureList.end(); ++iter)
                {
                    (*iter)->SummonCreature(NPC_BLOODPETAL_BLOSSOM, (*iter)->GetPositionX(), (*iter)->GetPositionY(), (*iter)->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 4000);
                    (*iter)->DespawnOrUnsummon(0);
                }
            }

            void DoRampartGrowth()
            {
                Talk(SAY_GROWTH);
                DoCast(SPELL_RAMPANT_GROWTH);
                RampartSummon(NPC_SEEDING_POD, 200.0f);
            }

            void UpdateAI(uint32 const diff)
            {
                events.Update(diff);

                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_WITHER:
                            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, false);
                            DoCastRandom(SPELL_WITHER, 0.0f);
                            events.ScheduleEvent(EVENT_WITHER, 18000);
                            break;
                        case EVENT_CONSUME_LIFE:
                            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            {
                                DoCast(target, SPELL_CONSUME_LIFE_ENERGIZE_EFFECT);
                                DoCast(target, SPELL_CONSUME_LIFE_DAMAGE_EFFECT);
                            }
                            events.ScheduleEvent(EVENT_CONSUME_LIFE, 15000);
                            break;
                        case EVENT_RAMPANT_GROWTH:
                            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
                            if (me->GetPower(POWER_ENERGY) == me->GetMaxPower(POWER_ENERGY))
                                DoRampartGrowth();
                            events.ScheduleEvent(EVENT_RAMPANT_GROWTH, 10000);
                            break;
                        case EVENT_SUMMON_POD:
                            me->SummonCreature(NPC_SEEDING_POD, me->GetPositionX()+rand()%20, me->GetPositionY()+rand()%20, me->GetPositionZ());
                            events.ScheduleEvent(EVENT_SUMMON_POD, 13000);
                            break;
                        case EVENT_SUMMON_SPORE:
                            me->SummonCreature(NPC_SPORE, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                            events.ScheduleEvent(EVENT_SUMMON_SPORE, 20000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

            void JustDied(Unit* /*who*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_ammunaeAI(creature);
        }
};

class npc_bloodpetal_blossom : public CreatureScript
{
public:
    npc_bloodpetal_blossom() : CreatureScript("npc_bloodpetal_blossom") { }

    struct npc_bloodpetal_blossomAI : public ScriptedAI
    {
        npc_bloodpetal_blossomAI(Creature* creature) : ScriptedAI(creature)
        {
            active = false;
        }

        EventMap events;

        bool active;

        void IsSummonedBy(Unit* /*creator*/)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_PASSIVE);
            me->AddAura(75768, me);
            me->HandleEmoteCommand(EMOTE_STATE_SUBMERGED);
            events.ScheduleEvent(EVENT_EMERGE, urand(3500, 4500));
        }

        void EnterCombat(Unit* /*who*/)
        {
            //me->AddAura(75768, me);
            events.Reset();
            events.ScheduleEvent(EVENT_THORN_SLASH, 5000);
        }

        void Reset()
        {
            events.Reset();
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (!UpdateVictim())
                if (active)
                    return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_EMERGE:
                        me->HandleEmoteCommand(0);
                        me->HandleEmoteCommand(EMOTE_ONESHOT_EMERGE);
                        events.ScheduleEvent(EVENT_ATTACK, 3000);
                    case EVENT_ATTACK:
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
                        me->SetReactState(REACT_AGGRESSIVE);
                        DoZoneInCombat();
                        events.ScheduleEvent(EVENT_THORN_SLASH, 5000);
                        active = true;
                        break;
                    case EVENT_THORN_SLASH:
                        DoCastVictim(SPELL_THORN_SLASH);
                        events.ScheduleEvent(EVENT_THORN_SLASH, 7500);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodpetal_blossomAI(creature);
    }
};

class npc_seed_pod : public CreatureScript
{
public:
    npc_seed_pod() : CreatureScript("npc_seed_pod") { }

    struct npc_seed_podAI : public Scripted_NoMovementAI
    {
        npc_seed_podAI(Creature* creature) : Scripted_NoMovementAI(creature) { }

        EventMap events;

        void IsSummonedBy(Unit* creator)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            events.ScheduleEvent(EVENT_ENERGIZE, 3000);
            me->SetReactState(REACT_PASSIVE);
            DoCastAOE(75687);
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ENERGIZE:
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    if (Unit* target = me->FindNearestCreature(BOSS_AMMUNAE, 100))
                        DoCast(target, SPELL_ENERGIZE);
                    me->AddAura(96278, me);
                    events.ScheduleEvent(EVENT_ENERGIZE, 3000);
                    if (IsHeroic())
                        me->AddAura(SPELL_ENERGIZING_GROWTH, me);
                    break;
                }
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            me->DespawnOrUnsummon();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_seed_podAI(creature);
    }
};

class npc_spore : public CreatureScript
{
public:
    npc_spore() : CreatureScript("npc_spore") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sporeAI(creature);
    }

    struct npc_sporeAI : public ScriptedAI
    {
        npc_sporeAI(Creature* creature) : ScriptedAI(creature) { }

        void IsSummonedBy(Unit* creator)
        {
            if (Player* victim = me->FindNearestPlayer(50.0f))
                me->Attack(victim, false);
            me->setFaction(16);
        }

        void DamageTaken(Unit* /*who*/, uint32& damage)
        {
            if (damage < me->GetHealth())
                return;

            DoCast(SPELL_SPORE_CLOUD);
            me->SetHealth(10);
            damage = 0;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_PASSIVE);
            me->DespawnOrUnsummon(7000);
        }
    };
};

void AddSC_boss_ammunae()
{
    new boss_ammunae;
    new npc_bloodpetal_blossom;
    new npc_seed_pod;
    new npc_spore;
}
