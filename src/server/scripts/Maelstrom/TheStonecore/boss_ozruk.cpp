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

#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "the_stonecore.h"

enum Spells
{
    SPELL_SPIKE_SHIELD              = 78835,
    SPELL_ENRAGE                    = 80467,
    SPELL_SHATTER                   = 78807,
    SPELL_GROUND_SLAM               = 78903,
    SPELL_BULWARK                   = 78939,
    SPELL_PARALYZE                  = 92426,
    SPELL_PARALYZE_DUMMY            = 92427,
    SPELL_PARALYZE_INIT             = 92428,

    SPELL_RUPTURE                   = 92383,
    SPELL_RUPTURE_DAM               = 92381,
    SPELL_RUPTURE_DAMAGE            = 95346,
    SPELL_RUPTURE_SUM_CONTROLLER    = 95669,
    SPELL_RUPTURE_PERIODIC          = 92393
};

enum Events
{
    EVENT_BULWARK               = 1,
    EVENT_GROUND_SLAM,
    EVENT_GROUND_SLAM_END,
    EVENT_SHATTER,
    EVENT_PARALYZE,
    EVENT_SPIKE_SHIELD
};

enum Say
{
    SAY_AGGRO       = 0,
    SAY_SLAY        = 1,
    SAY_SHIELD      = 2,
    SAY_DEATH       = 3,
    EMOTE_BULWARK   = 4,
    EMOTE_ENRAGE    = 5
};

class boss_ozruk : public CreatureScript
{
public:
    boss_ozruk() : CreatureScript("boss_ozruk") {}

    struct boss_ozrukAI : public BossAI
    {
        boss_ozrukAI(Creature * creature) : BossAI(creature, DATA_OZRUK) {}

        void Reset()
        {
            enraged = false;
            _Reset();
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void EnterCombat(Unit * /*who*/)
        {
            Talk(SAY_AGGRO);
            events.ScheduleEvent(EVENT_BULWARK, 6000);
            _EnterCombat();
        }

        void JustSummoned(Creature * summon)
        {
            if (summon->GetEntry() == NPC_RUPTURE_CONTROLLER)
            {
                summon->CastSpell(summon, SPELL_RUPTURE_PERIODIC, false);
                summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                summon->DespawnOrUnsummon(5000);
            }
        }

        void JustDied(Unit * /*killer*/)
        {
            Talk(SAY_DEATH);
            _JustDied();
        }

        void KilledUnit(Unit * victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_SLAY);
        }

        void SpellHit(Unit* /*caster*/, SpellInfo const* spell)
        {
            if (spell->Id == SPELL_PARALYZE_DUMMY)
            {
                DoCast(SPELL_PARALYZE);
                events.ScheduleEvent(EVENT_SHATTER, 3000);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_BULWARK:
                        Talk(EMOTE_BULWARK);
                        DoCast(SPELL_BULWARK);
                        events.ScheduleEvent(EVENT_SPIKE_SHIELD, 5000);
                        break;
                    case EVENT_SPIKE_SHIELD:
                        Talk(SAY_SHIELD);
                        DoCast(SPELL_SPIKE_SHIELD);
                        if (IsHeroic())
                            DoCast(SPELL_PARALYZE_INIT);
                        events.ScheduleEvent(EVENT_GROUND_SLAM, 2000);
                        break;
                    case EVENT_GROUND_SLAM:
                        me->SetReactState(REACT_PASSIVE);
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MoveIdle();
                        me->SetUInt64Value(UNIT_FIELD_TARGET, 0);
                        me->SetFacingToObject(me->getVictim());
                        DoCast(SPELL_GROUND_SLAM);
                        events.ScheduleEvent(EVENT_GROUND_SLAM_END, 0);
                        break;
                    case EVENT_GROUND_SLAM_END:
                        DoCast(SPELL_RUPTURE_SUM_CONTROLLER);
                        me->SetReactState(REACT_AGGRESSIVE);
                        if (Unit * victim = me->getVictim())
                        {
                            me->SetUInt64Value(UNIT_FIELD_TARGET, victim->GetGUID());
                            DoStartMovement(victim);
                        }
                        if (!IsHeroic())
                            events.ScheduleEvent(EVENT_BULWARK, 10000);
                        break;
                    case EVENT_SHATTER:
                        DoCast(SPELL_SHATTER);
                        events.ScheduleEvent(EVENT_BULWARK, 3000);
                        break;
                    default:
                        break;
                }
            }

            if (!enraged && me->HealthBelowPct(30))
            {
                Talk(EMOTE_ENRAGE);
                enraged = true;
                DoCast(SPELL_ENRAGE);
            }

            DoMeleeAttackIfReady();
        }
    private:
        bool enraged;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new boss_ozrukAI(creature);
    }
};

class spell_rupture_periodic : public SpellScriptLoader
{
public:
    spell_rupture_periodic() : SpellScriptLoader("spell_rupture_periodic") { }

    class spell_rupture_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_rupture_periodic_AuraScript)

        void HandleEffectPeriodic(AuraEffect const* aurEff)
        {
            Unit * caster = GetCaster();
            if (!caster)
                return;
            Position pos;
            float dist = 1.0f + aurEff->GetTickNumber() * 5.0f;
            caster->GetNearPosition(pos, dist, 0.0f);

            for (int i = 0; i < 3; i++)
            {
                if (i == 1)
                    caster->MovePosition(pos, 3.0f, M_PI/2);
                else if (i == 2)
                    caster->MovePosition(pos, 6.0f, -M_PI/2);

                if (Creature * creature = caster->SummonCreature(NPC_RUPTURE, pos, TEMPSUMMON_TIMED_DESPAWN, 1000))
                {
                    creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE);
                    creature->CastSpell(creature, SPELL_RUPTURE_DAM, false);
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_rupture_periodic_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_rupture_periodic_AuraScript();
    }
};

class spell_rupture_damage : public SpellScriptLoader
{
public:
    spell_rupture_damage() : SpellScriptLoader("spell_rupture_damage") {}

    class spell_rupture_damage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_rupture_damage_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end();)
            {
                if (GetCaster()->isInFront((*itr)))
                    ++itr;
                else
                    targets.erase(itr++);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_rupture_damage_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_rupture_damage_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_rupture_damage_SpellScript();
    }
};


void AddSC_boss_ozruk()
{
    new boss_ozruk();
    new spell_rupture_periodic();
    new spell_rupture_damage();
};
