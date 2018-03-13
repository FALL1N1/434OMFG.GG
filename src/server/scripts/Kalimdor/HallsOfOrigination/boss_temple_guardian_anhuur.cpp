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

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "GridNotifiers.h"
#include "Player.h"
#include "halls_of_origination.h"

enum Texts
{
    SAY_AGGRO                    = 0,
    SAY_SHIELD                   = 1,
    EMOTE_SHIELD                 = 2,
    EMOTE_UNSHIELD               = 3,
    SAY_KILL                     = 4,
    SAY_DEATH                    = 5
};

enum Spells
{
    SPELL_DIVINE_RECKONING       = 75592,
    SPELL_BURNING_LIGHT          = 75115,
    SPELL_REVERBERATING_HYMN     = 75322,
    SPELL_SHIELD_OF_LIGHT        = 74938,
    SPELL_ACTIVATE_BEACONS       = 76599,
    SPELL_TELEPORT               = 74969,
    SPELL_SHIELD_VISUAL_RIGHT    = 83698,
    SPELL_BEAM_OF_LIGHT_RIGHT    = 76573,
    SPELL_SHIELD_VISUAL_LEFT     = 83697,
    SPELL_BEAM_OF_LIGHT_LEFT     = 74930,
    SPELL_SEARING_LIGHT          = 75194,
    SPELL_VIPER_POISON           = 74538
};

enum Events
{
    EVENT_DIVINE_RECKONING       = 1,
    EVENT_BURNING_LIGHT          = 2,
    EVENT_SEAR                   = 3
};

enum Phases
{
    PHASE_SHIELDED               = 0,
    PHASE_FIRST_SHIELD           = 1, // Ready to be shielded for the first time
    PHASE_SECOND_SHIELD          = 2, // First shield already happened, ready to be shielded a second time
    PHASE_FINAL                  = 3  // Already shielded twice, ready to finish the encounter normally.
};

enum Actions
{
    ACTION_CHECK_BEACONS = 1,
};

class boss_temple_guardian_anhuur : public CreatureScript
{
public:
    boss_temple_guardian_anhuur() : CreatureScript("boss_temple_guardian_anhuur") { }

    struct boss_temple_guardian_anhuurAI : public BossAI
    {
        boss_temple_guardian_anhuurAI(Creature* creature) : BossAI(creature, DATA_TEMPLE_GUARDIAN_ANHUUR) { }

        void CleanStalkers()
        {
            std::list<Creature*> stalkers;
            GetCreatureListWithEntryInGrid(stalkers, me, NPC_CAVE_IN_STALKER, 100.0f);
            for (std::list<Creature*>::iterator itr = stalkers.begin(); itr != stalkers.end(); ++itr)
            {
                (*itr)->RemoveAurasDueToSpell(SPELL_BEAM_OF_LIGHT_RIGHT);
                (*itr)->RemoveAurasDueToSpell(SPELL_BEAM_OF_LIGHT_LEFT);
            }
        }

        void Reset()
        {
            _phase = PHASE_FIRST_SHIELD;
            _oldPhase = PHASE_FIRST_SHIELD;
            _Reset();
            CleanStalkers();
            me->RemoveAurasDueToSpell(SPELL_SHIELD_OF_LIGHT);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage)
        {
            if ((me->HealthBelowPctDamaged(66, damage) && _phase == PHASE_FIRST_SHIELD) ||
                (me->HealthBelowPctDamaged(33, damage) && _phase == PHASE_SECOND_SHIELD))
            {
                _phase++; // Increase the phase
                _oldPhase = _phase;

                _phase = PHASE_SHIELDED;

                me->InterruptNonMeleeSpells(true);
                me->AttackStop();
                DoCast(me, SPELL_TELEPORT);

                DoCast(me, SPELL_SHIELD_OF_LIGHT);
                me->SetFlag(UNIT_FIELD_FLAGS, uint32(UNIT_FLAG_UNK_31));

                DoCastAOE(SPELL_ACTIVATE_BEACONS);

                std::list<Creature*> stalkers;
                GameObject* door = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_ANHUUR_DOOR));
                GetCreatureListWithEntryInGrid(stalkers, me, NPC_CAVE_IN_STALKER, 100.0f);

                stalkers.remove_if(Trinity::HeightDifferenceCheck(door, 0.0f, false)); // Target only the bottom ones
                for (std::list<Creature*>::iterator itr = stalkers.begin(); itr != stalkers.end(); ++itr)
                {
                    if ((*itr)->GetPositionX() > door->GetPositionX())
                    {
                        (*itr)->CastSpell((*itr), SPELL_SHIELD_VISUAL_LEFT, true);
                        (*itr)->CastSpell((*itr), SPELL_BEAM_OF_LIGHT_LEFT, true);
                    }
                    else
                    {
                        (*itr)->CastSpell((*itr), SPELL_SHIELD_VISUAL_RIGHT, true);
                        (*itr)->CastSpell((*itr), SPELL_BEAM_OF_LIGHT_RIGHT, true);
                    }
                }

                std::list<Creature*> snakes;
                GetCreatureListWithEntryInGrid(snakes, me, NPC_SNAKE, 100.0f);
                for (std::list<Creature*>::iterator itr = snakes.begin(); itr != snakes.end(); ++itr)
                {
                    if (!(*itr)->isAlive())
                        (*itr)->Respawn(true);
                }

                // must be delayed because of the teleport
                me->CastWithDelay(1000, me, SPELL_REVERBERATING_HYMN);

                Talk(EMOTE_SHIELD);
                Talk(SAY_SHIELD);
            }
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_CHECK_BEACONS)
            {
                bool activeStalkerFound = false;
                std::list<Creature*> stalkers;
                GameObject* door = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_ANHUUR_DOOR));
                GetCreatureListWithEntryInGrid(stalkers, me, NPC_CAVE_IN_STALKER, 100.0f);
                stalkers.remove_if(Trinity::HeightDifferenceCheck(door, 0.0f, false));
                for (std::list<Creature*>::iterator itr = stalkers.begin(); itr != stalkers.end(); ++itr)
                {
                    if ((*itr)->HasAura(SPELL_BEAM_OF_LIGHT_LEFT) || (*itr)->HasAura(SPELL_BEAM_OF_LIGHT_RIGHT))
                    {
                        activeStalkerFound = true;
                        break;
                    }
                }

                if (!activeStalkerFound)
                {
                    me->RemoveAurasDueToSpell(SPELL_SHIELD_OF_LIGHT);
                    Talk(EMOTE_UNSHIELD);
                    _phase = _oldPhase;
                }
            }
        }

        void EnterCombat(Unit* /*who*/)
        {
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
            Talk(SAY_AGGRO);
            _EnterCombat();

            events.ScheduleEvent(EVENT_DIVINE_RECKONING, urand(10000, 12000));
            events.ScheduleEvent(EVENT_BURNING_LIGHT, 12000);
        }

        void JustDied(Unit* /*killer*/)
        {
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            Talk(SAY_DEATH);
            _JustDied();
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_KILL);
        }

        void JustReachedHome()
        {
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            _JustReachedHome();
            instance->SetBossState(DATA_TEMPLE_GUARDIAN_ANHUUR, FAIL);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim() || !CheckInRoom() || me->GetCurrentSpell(CURRENT_CHANNELED_SPELL) || _phase == PHASE_SHIELDED)
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_DIVINE_RECKONING:
                        DoCastRandom(SPELL_DIVINE_RECKONING, 0.0f);
                        events.ScheduleEvent(EVENT_DIVINE_RECKONING, urand(10000, 12000));
                        break;
                    case EVENT_BURNING_LIGHT:
                    {
                        Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, NonTankTargetSelector(me));
                        if (!target)
                            DoCastRandom(SPELL_BURNING_LIGHT, 0.0f);
                        else
                            DoCast(target, SPELL_BURNING_LIGHT);
                        events.ScheduleEvent(EVENT_SEAR, 2000);
                        events.ScheduleEvent(EVENT_BURNING_LIGHT, 12000);
                        break;
                    }
                    case EVENT_SEAR:
                    {
                        Unit* target = me->FindNearestCreature(NPC_SEARING_LIGHT, 100.0f);
                        if (!target)
                            break;

                        std::list<Creature*> stalkers;
                        GetCreatureListWithEntryInGrid(stalkers, me, NPC_CAVE_IN_STALKER, 100.0f);
                        stalkers.remove_if(Trinity::HeightDifferenceCheck(ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_ANHUUR_DOOR)), 5.0f, true));

                        if (stalkers.size() < 2)
                            break;

                        stalkers.sort(Trinity::ObjectDistanceOrderPred(target));

                        // Get the closest statue face (any of its eyes)
                        Creature* eye1 = stalkers.front();
                        stalkers.remove(eye1); // Remove the eye.
                        stalkers.sort(Trinity::ObjectDistanceOrderPred(eye1)); // Find the second eye.
                        Creature* eye2 = stalkers.front();

                        eye1->CastSpell(eye1, SPELL_SEARING_LIGHT, true);
                        eye2->CastSpell(eye2, SPELL_SEARING_LIGHT, true);
                        break;
                    }
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        uint8 _phase;
        uint8 _oldPhase;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return GetHallsOfOriginationAI<boss_temple_guardian_anhuurAI>(creature);
    }
};

class spell_anhuur_shield_of_light : public SpellScriptLoader
{
    public:
        spell_anhuur_shield_of_light() : SpellScriptLoader("spell_anhuur_shield_of_light") { }

        class spell_anhuur_shield_of_light_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_anhuur_shield_of_light_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (InstanceMap* instance = GetCaster()->GetMap()->ToInstanceMap())
                {
                    if (InstanceScript* const script = instance->GetInstanceScript())
                    {
                        if (GameObject* go = ObjectAccessor::GetGameObject(*GetCaster(), script->GetData64(DATA_ANHUUR_DOOR)))
                        {
                            targets.remove_if(Trinity::HeightDifferenceCheck(go, 5.0f, false));
                            targets.remove(GetCaster());
                            targets.sort(Trinity::ObjectDistanceOrderPred(GetCaster()));
                            targets.resize(2);
                        }
                    }
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_anhuur_shield_of_light_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_anhuur_shield_of_light_SpellScript();
        }
};

class spell_anhuur_disable_beacon_beams : public SpellScriptLoader
{
    public:
        spell_anhuur_disable_beacon_beams() : SpellScriptLoader("spell_anhuur_disable_beacon_beams") { }

        class spell_anhuur_disable_beacon_beams_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_anhuur_disable_beacon_beams_SpellScript);

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                GetHitUnit()->RemoveAurasDueToSpell(GetEffectValue());
            }

            void CheckPhase()
            {
                if (InstanceScript* instance = GetCaster()->GetInstanceScript())
                    if (Creature* anhuur = ObjectAccessor::GetCreature(*GetCaster(), instance->GetData64(DATA_ANHUUR_GUID)))
                        anhuur->AI()->DoAction(ACTION_CHECK_BEACONS);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_anhuur_disable_beacon_beams_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
                AfterCast += SpellCastFn(spell_anhuur_disable_beacon_beams_SpellScript::CheckPhase);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_anhuur_disable_beacon_beams_SpellScript();
        }
};

class spell_anhuur_activate_beacons : public SpellScriptLoader
{
    public:
        spell_anhuur_activate_beacons() : SpellScriptLoader("spell_anhuur_activate_beacons") { }

        class spell_anhuur_activate_beacons_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_anhuur_activate_beacons_SpellScript);

            void Activate(SpellEffIndex index)
            {
                PreventHitDefaultEffect(index);
                GetHitGObj()->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_anhuur_activate_beacons_SpellScript::Activate, EFFECT_0, SPELL_EFFECT_ACTIVATE_OBJECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_anhuur_activate_beacons_SpellScript();
        }
};

class spell_anhuur_divine_reckoning : public SpellScriptLoader
{
public:
    spell_anhuur_divine_reckoning() : SpellScriptLoader("spell_anhuur_divine_reckoning") { }

    class spell_anhuur_divine_reckoning_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_anhuur_divine_reckoning_AuraScript);

        void OnPeriodic(AuraEffect const* aurEff)
        {
            if (Unit* caster = GetCaster())
            {
                CustomSpellValues values;
                values.AddSpellMod(SPELLVALUE_BASE_POINT0, aurEff->GetAmount());
                caster->CastCustomSpell(GetSpellInfo()->Effects[EFFECT_0].TriggerSpell, values, GetTarget());
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_anhuur_divine_reckoning_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_anhuur_divine_reckoning_AuraScript();
    }
};

class npc_pit_viper : public CreatureScript
{
public:
    npc_pit_viper() : CreatureScript("npc_pit_viper") { }

    struct npc_pit_viperAI : public ScriptedAI
    {
        npc_pit_viperAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            castTimer = 0;
        }

        void EnterCombat(Unit* /*who*/)
        {
            castTimer = 5000;
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (castTimer <= diff)
            {
                DoCastVictim(SPELL_VIPER_POISON);
                castTimer = 8000;
            } else castTimer -= diff;
        }
    private:
        uint32 castTimer;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_pit_viperAI(creature);
    }
};

void AddSC_boss_temple_guardian_anhuur()
{
    new boss_temple_guardian_anhuur();
    new spell_anhuur_shield_of_light();
    new spell_anhuur_disable_beacon_beams();
    new spell_anhuur_activate_beacons();
    new spell_anhuur_divine_reckoning();
}
