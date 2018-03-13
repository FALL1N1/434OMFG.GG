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
#include "blackrock_caverns.h"

enum Texts
{
    TALK_AGGRO              = 0,
    TALK_QUICKSILVER_ARMOR  = 1,
    TALK_SLAY               = 2,
    TALK_DEATH              = 3,
};

enum Spells
{
    SPELL_QUECKSILVER_ARMOR             = 75842,
    SPELL_SUPERHEATED_QUECKSILVER_ARMOR = 75846,
    SPELL_HEAT_WAVE                     = 63677,
    SPELL_BURNING_METAL                 = 76002,
    SPELL_CLEAVE                        = 15284,
    SPELL_LAVA_SPOUT                    = 76007,
    SPELL_LAVA_POOL                     = 90391
};

enum Events
{
    EVENT_CLEAVE                = 1,
    EVENT_CHECK_ARMOR_STATE     = 2,
    EVENT_LAVA_POOL             = 3,
};

#define ACHIEVEMENT_TOO_HOT_TO_HANDLE   5283

Position const boundFlamesPos[3] =
{
    {268.717f, 789.984f, 95.349f, 4.86041f},
    {226.707f, 754.725f, 95.350f, 2.75163f},
    {216.941f, 808.943f, 95.350f, 0.63891f},
};

class boss_karsh_steelbender : public CreatureScript
{
    public:
        boss_karsh_steelbender() : CreatureScript("boss_karsh_steelbender") { }

        struct boss_karsh_steelbenderAI : public BossAI
        {
            boss_karsh_steelbenderAI(Creature* creature) : BossAI(creature, BOSS_KARSH_STEELBENDER) {}

            void Reset()
            {
                _Reset();
                tooHotToHandle = false;
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                Talk(TALK_AGGRO);
                DoCast(me, SPELL_QUECKSILVER_ARMOR);
                events.ScheduleEvent(EVENT_CLEAVE, urand(12000,18000));
                events.ScheduleEvent(EVENT_CHECK_ARMOR_STATE, 1000);
            }

            void JustDied(Unit* /*killer*/)
            {
                if (IsHeroic() && tooHotToHandle)
                    instance->DoCompleteAchievement(ACHIEVEMENT_TOO_HOT_TO_HANDLE);

                Talk(TALK_DEATH);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                _JustDied();
            }

            void KilledUnit(Unit* who)
            {
                if (who->GetTypeId() == TYPEID_PLAYER)
                    Talk(TALK_SLAY);
            }

            void JustSummoned(Creature* summon)
            {
                BossAI::JustSummoned(summon);
            }

            void StartEruption()
            {
                Talk(TALK_QUICKSILVER_ARMOR);
                DoCast(me, SPELL_QUECKSILVER_ARMOR);

                std::list<Creature*> creatures;
                GetCreatureListWithEntryInGrid(creatures, me, NPC_LAVA_SPOUT_TRIGGER, 100.0f);

                if (!creatures.empty())
                    for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                        (*iter)->CastSpell((*iter),SPELL_LAVA_SPOUT, true);

                if (IsHeroic())
                {
                    for (uint8 i = 0; i <= 2; i++)
                        me->SummonCreature(NPC_BOUND_FLAMES, boundFlamesPos[i], TEMPSUMMON_CORPSE_DESPAWN);
                }
            }

            void DoMeleeAttackIfReady()
            {
                if (me->getVictim())
                {
                    if (me->isAttackReady() && !me->IsNonMeleeSpellCasted(false))
                    {
                        if (me->IsWithinMeleeRange(me->getVictim()))
                        {
                            if (me->HasAura(SPELL_SUPERHEATED_QUECKSILVER_ARMOR))
                                DoCastAOE(SPELL_BURNING_METAL);

                            me->AttackerStateUpdate(me->getVictim());
                            me->resetAttackTimer();
                        }
                    }
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if ((!me->HasAura(SPELL_QUECKSILVER_ARMOR)) && (!me->HasAura(SPELL_SUPERHEATED_QUECKSILVER_ARMOR)))
                {
                    StartEruption();
                    return;
                }

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CLEAVE:
                            DoCastVictim(SPELL_CLEAVE);
                            events.ScheduleEvent(EVENT_CLEAVE, urand(12000,18000));
                            break;
                        case EVENT_CHECK_ARMOR_STATE:
                            if (me->GetDistance(237.166f, 785.067f, 95.67f) < 4.5f)
                            {
                                me->RemoveAura(SPELL_QUECKSILVER_ARMOR);
                                if (me->HasAura(SPELL_SUPERHEATED_QUECKSILVER_ARMOR))
                                {
                                    me->SetAuraStack(SPELL_SUPERHEATED_QUECKSILVER_ARMOR, me, me->GetAuraCount(SPELL_SUPERHEATED_QUECKSILVER_ARMOR) + 1);
                                    me->GetAura(SPELL_SUPERHEATED_QUECKSILVER_ARMOR)->RefreshDuration();
                                }
                                else
                                    me->AddAura(SPELL_SUPERHEATED_QUECKSILVER_ARMOR, me);

                                DoCastAOE(SPELL_HEAT_WAVE);

                                if (IsHeroic() && me->GetAuraCount(SPELL_SUPERHEATED_QUECKSILVER_ARMOR) >= 15)
                                    tooHotToHandle = true;
                            }
                            events.ScheduleEvent(EVENT_CHECK_ARMOR_STATE, 1000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
            private:
                bool tooHotToHandle;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_karsh_steelbenderAI(creature);
        }
};

class spell_burning_metal : public SpellScriptLoader
{
public:
    spell_burning_metal() : SpellScriptLoader("spell_burning_metal") { }

    class spell_burning_metal_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_burning_metal_SpellScript);

        void CalculateDamage()
        {
            if (!GetHitUnit())
                return;

            int32 damage = GetHitDamage();
            uint32 stacks = GetCaster()->GetAuraCount(SPELL_SUPERHEATED_QUECKSILVER_ARMOR);

            if (stacks)
                damage *= stacks;

            SetHitDamage(damage);
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_burning_metal_SpellScript::CalculateDamage);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_burning_metal_SpellScript();
    }
};

class spell_quecksilver_cooled : public SpellScriptLoader
{
public:
    spell_quecksilver_cooled() : SpellScriptLoader("spell_quecksilver_cooled") { }

    class spell_quecksilver_cooled_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_quecksilver_cooled_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            GetTarget()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
            GetTarget()->CombatStop(true);
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            GetTarget()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_quecksilver_cooled_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_quecksilver_cooled_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_quecksilver_cooled_AuraScript();
    }
};


class DistanceCheck
{
public:
    explicit DistanceCheck(Unit* _caster) : caster(_caster) { }

    bool operator() (WorldObject* unit) const
    {
        if (caster->GetExactDist2d(unit) <= 5.0f)
            return true;
        return false;
    }

    Unit* caster;
};

class spell_karsh_lava_spout : public SpellScriptLoader
{
public:
    spell_karsh_lava_spout() : SpellScriptLoader("spell_karsh_lava_spout") { }

    class spell_karsh_lava_spout_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_karsh_lava_spout_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(DistanceCheck(GetCaster()));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_karsh_lava_spout_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_karsh_lava_spout_SpellScript();
    }
};


void AddSC_boss_karsh_steelbender()
{
    new boss_karsh_steelbender();
    new spell_burning_metal();
    new spell_quecksilver_cooled();
    new spell_karsh_lava_spout();
}
