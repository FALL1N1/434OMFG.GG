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
SDName: boss_Halazzi
SD%Complete: 80
SDComment:
SDCategory: Zul'Aman
EndScriptData */

#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "zulaman.h"

enum says
{
    YELL_AGGRO   = 0,
    YELL_SABER   = 1,
    YELL_SPLIT   = 2,
    YELL_MERGE   = 3,
    YELL_KILL    = 4,
    YELL_DEATH   = 5,
    YELL_BERSERK = 6,
};

enum Spells
{
    // 22:22:48.149

    // Merge
    SPELL_WATER_TOTEM               = 97500, // 22:22:57.337 22:23:16.619 -- 22:24:10.892
    SPELL_ENRAGE                    = 43139, // 22:23:05.777 22:23:21.455 -- 22:24:15.728

    SPELL_REFRESHING_STREAM         = 97502,
    SPELL_HALAZZI_TRANSFORM         = 43271, // 22:24:53.495

    //Split
    SPELL_TRANSFIGURE               = 97484, // 22:23:22.672
    SPELL_SUMMON_LYNX               = 43143, // 22:23:25.074
    SPELL_GROUND_HORION             = 97491, // 22:23:32.297 22:23:40.752 22:23:51.610
    SPELL_FLAME_SHOCK               = 97490, // 22:23:33.514 22:23:46.774 22:24:02.468
    SPELL_LIGHTNING_TOTEM           = 97492, // 22:23:35.916 22:23:52.811

    SPELL_HALAZI_TRANSFORM_0        = 43145, // 22:24:02.468

    SPELL_TRANSFIGURE_FORM          = 43573,

    // Lynx
    SPELL_FIXATE                    = 78617,
    SPELL_SHRED_ARMOR               = 43243,
    SPELL_LYNX_FLURRY               = 43290,

    SPELL_BERSERK                   = 45078
};

enum Hal_CreatureIds
{
    NPC_SPIRIT_LYNX                 = 24143,
    NPC_WATER_TOTEM                 = 52755,
    NPC_LIGHTNING_TOTEM             = 24224
};

enum Events
{
    EVENT_ENRAGE = 1,
    EVENT_WATER_TOTEM,
    EVENT_SPLIT,
    EVENT_MERGE,
    EVENT_LIGHTNING_TOTEM,
    EVENT_FLAME_SHOCK,
    EVENT_FIXATE,
    EVENT_SHRED_ARMOR,
    EVENT_LYNX_FLURRY,
    EVENT_BERSERK,
    EVENT_GROUND_HORION,
    EVENT_SUMMON_LYNX,
};

class boss_halazzi : public CreatureScript
{
    public:
        boss_halazzi() : CreatureScript("boss_halazzi") { }

        struct boss_halazziAI : public BossAI
        {
            boss_halazziAI(Creature* creature) : BossAI(creature, DATA_HALAZZIEVENT)
            {
            }

            void Reset()
            {
                _Reset();
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FIXATE);
                back = false;
                split = 0;
                me->SetMaxHealth(4149700);
                me->SetHealth(me->GetMaxHealth());
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                DoZoneInCombat();
                Talk(YELL_AGGRO);

                events.ScheduleEvent(EVENT_WATER_TOTEM, 9000);
                events.ScheduleEvent(EVENT_ENRAGE, 17000);
                events.ScheduleEvent(EVENT_BERSERK, 600000);
            }

            void JustSummoned(Creature* summon)
            {
                switch (summon->GetEntry())
                {
                    case NPC_WATER_TOTEM:
                    {
                        summon->SetReactState(REACT_PASSIVE);
                        summon->CastSpell(summon, SPELL_REFRESHING_STREAM, true);
                        summons.Summon(summon);
                        return;
                    }
                    case NPC_SPIRIT_LYNX:
                    {
                        lynxGUID = summon->GetGUID();
                        summon->SetReactState(REACT_AGGRESSIVE);
                        break;
                    }
                    case NPC_LIGHTNING_TOTEM:
                        summon->SetReactState(REACT_AGGRESSIVE);
                        summon->SetInCombatWithZone();
                        break;
                    default:
                        break;
                }
                BossAI::JustSummoned(summon);
            }

            void SummonedCreatureDespawn(Creature* summon)
            {
                if (summon->GetEntry() == NPC_SPIRIT_LYNX && !back)
                {
                    events.ScheduleEvent(EVENT_MERGE, 1);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FIXATE);
                }
            }

            void DamageTaken(Unit* who, uint32 &damage)
            {
                if (damage > 0)
                {
                    if (me->HasAura(SPELL_TRANSFIGURE_FORM) && me->HealthBelowPct(20) && !back)
                    {
                        back = true;
                        me->RemoveAura(SPELL_TRANSFIGURE_FORM);
                        events.ScheduleEvent(EVENT_MERGE, 1);
                    }
                    else if (!me->HasAura(SPELL_TRANSFIGURE_FORM) && !back)
                    {
                        if (me->HealthBelowPct(60) && split == 0)
                        {
                            split++;
                            events.ScheduleEvent(EVENT_SPLIT, 1);
                        }
                        else if (me->HealthBelowPct(30) && split == 1)
                        {
                            split++;
                            events.ScheduleEvent(EVENT_SPLIT, 1);
                        }
                        else if (me->HealthBelowPct(25) && split == 2)
                        {
                            events.ScheduleEvent(EVENT_LIGHTNING_TOTEM, 5000);
                            split++;
                        }
                    }
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ENRAGE:
                            me->CastSpell(me, SPELL_ENRAGE, true);
                            events.ScheduleEvent(EVENT_ENRAGE, 16000);
                            break;
                        case EVENT_WATER_TOTEM:
                            DoCast(SPELL_WATER_TOTEM);
                            events.ScheduleEvent(EVENT_WATER_TOTEM, 19000);
                            break;
                        case EVENT_SPLIT:
                            me->RemoveAura(SPELL_HALAZZI_TRANSFORM);
                            me->CastSpell(me, SPELL_TRANSFIGURE, true);
                            me->CastSpell(me, SPELL_TRANSFIGURE_FORM, true);
                            me->SetMaxHealth(2074850);
                            Talk(YELL_SPLIT);
                            events.ScheduleEvent(EVENT_SUMMON_LYNX, 3000);
                            events.ScheduleEvent(EVENT_LIGHTNING_TOTEM, 13000);
                            events.ScheduleEvent(EVENT_FLAME_SHOCK, 11000);
                            events.ScheduleEvent(EVENT_GROUND_HORION, 10000);
                            events.CancelEvent(EVENT_ENRAGE);
                            events.CancelEvent(EVENT_WATER_TOTEM);
                            break;
                        case EVENT_SUMMON_LYNX:
                            me->CastSpell(me, SPELL_SUMMON_LYNX, true);
                            break;
                        case EVENT_LIGHTNING_TOTEM:
                            me->CastSpell(me, SPELL_LIGHTNING_TOTEM, false);
                            events.ScheduleEvent(EVENT_LIGHTNING_TOTEM, 17000);
                            break;
                        case EVENT_FLAME_SHOCK:
                            DoCastRandom(SPELL_FLAME_SHOCK, 0.0f);
                            events.ScheduleEvent(EVENT_FLAME_SHOCK, urand(13000, 16000));
                            break;
                        case EVENT_GROUND_HORION:
                            DoCastRandom(SPELL_GROUND_HORION, 0.0f);
                            events.ScheduleEvent(EVENT_GROUND_HORION, urand(8000, 11000));
                            break;
                        case EVENT_MERGE:
                            me->RemoveAura(SPELL_TRANSFIGURE_FORM);
                            me->CastSpell(me, SPELL_HALAZZI_TRANSFORM, true);
                            me->SetMaxHealth(4149700);
                            Talk(YELL_MERGE);
                            if(Creature* lynx = me->GetCreature(*me, lynxGUID))
                                lynx->DespawnOrUnsummon();
                            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FIXATE);
                            if (split == 1)
                                health = me->CountPctFromMaxHealth(60);
                            else
                                health = me->CountPctFromMaxHealth(30);
                            me->SetHealth(health);
                            events.ScheduleEvent(EVENT_WATER_TOTEM, 8000);
                            events.ScheduleEvent(EVENT_ENRAGE, 13000);
                            events.CancelEvent(EVENT_LIGHTNING_TOTEM);
                            events.CancelEvent(EVENT_FLAME_SHOCK);
                            events.CancelEvent(EVENT_GROUND_HORION);
                            back = false;
                            break;
                        case EVENT_BERSERK:
                            Talk(YELL_BERSERK);
                            me->CastSpell(me, SPELL_BERSERK, true);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(YELL_KILL);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FIXATE);
                Talk(YELL_DEATH);
            }

        private:
            uint64 lynxGUID;
            int8 split;
            uint32 health;
            bool back;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_halazziAI(creature);
        }
};

// Spirits Lynx
class mob_halazzi_lynx : public CreatureScript
{
    public:
        mob_halazzi_lynx() : CreatureScript("mob_halazzi_lynx") { }

        struct mob_halazzi_lynxAI : public ScriptedAI
        {
            mob_halazzi_lynxAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            void Reset()
            {
                despawn = false;
                events.Reset();
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
            }

            void DamageTaken(Unit* /*done_by*/, uint32 &damage)
            {
                if (me->HealthBelowPct(20) && !despawn)
                {
                    me->DespawnOrUnsummon();
                    despawn = true;
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                DoZoneInCombat();

                events.ScheduleEvent(EVENT_FIXATE, 1);
                events.ScheduleEvent(EVENT_SHRED_ARMOR, 4000);
                events.ScheduleEvent(EVENT_LYNX_FLURRY, 7000);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FIXATE:
                            pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FIXATE);
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            {
                                DoResetThreat();
                                me->CastSpell(target, SPELL_FIXATE, true);
                                me->AddThreat(target, 50000000.0f);
                            }
                            events.ScheduleEvent(EVENT_FIXATE, urand(5000, 10000));
                            break;
                        case EVENT_SHRED_ARMOR:
                            DoCastVictim(SPELL_SHRED_ARMOR);
                            events.ScheduleEvent(EVENT_SHRED_ARMOR, 4000);
                            break;
                        case EVENT_LYNX_FLURRY:
                            me->CastSpell(me, SPELL_LYNX_FLURRY, true);
                            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SNARE, true);
                            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SLEEP, true);
                            events.ScheduleEvent(EVENT_LYNX_FLURRY, 18000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* pInstance;
            EventMap events;
            bool despawn;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_halazzi_lynxAI(creature);
        }
};

void AddSC_boss_halazzi()
{
    new boss_halazzi();
    new mob_halazzi_lynx();
}
