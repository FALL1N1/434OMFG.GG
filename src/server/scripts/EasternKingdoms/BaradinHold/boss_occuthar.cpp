/*
 * Copyright (C) 2013-2014 OMFG.GG Network <http://www.omfg.gg/>
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
#include "baradin_hold.h"

enum Spells
{
    // Occu'thar
    SPELL_SEARING_SHADOWS           = 101007,
    SPELL_FOCUSED_FIRE_CHANNEL      = 96886,
    SPELL_FOCUSED_FIRE              = 96884,
    SPELL_FOCUSED_FIRE_SUMMON       = 96873,
    SPELL_EYES_OF_OCCUTHAR          = 96920,
    SPELL_RIDE_VEHICLE              = 84495,
    SPELL_BERSERK                   = 47008,

    // Eye
    SPELL_GAZE_OF_OCCUTHAR          = 97028,
    SPELL_EYE_EXPLODE               = 96968,
    SPELL_EYE_VISUAL                = 96995,
    SPELL_RIDE_VEHICLE_PLAYER       = 96812,
};

enum Events
{
    EVENT_SEARING_SHADOWS   = 1,
    EVENT_FOCUSED_FIRE      = 2,
    EVENT_EYE_OF_OCCUTHAR   = 3,
    EVENT_BERSERK           = 4,
    EVENT_EYE_EXPLOSION     = 5,
};

class boss_occuthar : public CreatureScript
{
public:
    boss_occuthar() : CreatureScript("boss_occuthar") { }

    struct boss_occutharAI : public BossAI
    {
        boss_occutharAI(Creature* creature) : BossAI(creature, DATA_OCCUTHAR), vehicle(creature->GetVehicleKit())
        {
            ASSERT(me->GetVehicleKit());
        }

        void Reset()
        {
            _Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            for (int i = 0; i <= 6; ++i)
                if (Creature* eye = me->SummonCreature(NPC_OCCUTHAR_EYE, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_MANUAL_DESPAWN))
                    eye->EnterVehicle(me, i);
                   // eye->CastSpell(me, SPELL_RIDE_VEHICLE);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            events.ScheduleEvent(EVENT_SEARING_SHADOWS, 6000);
            events.ScheduleEvent(EVENT_FOCUSED_FIRE, 16000);
            events.ScheduleEvent(EVENT_EYE_OF_OCCUTHAR, 23000);
            events.ScheduleEvent(EVENT_BERSERK, 300000);
        }

        void EnterEvadeMode()
        {
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            me->GetMotionMaster()->MoveTargetedHome();
            _EnterEvadeMode();
            Reset();
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);

            if (summon->GetEntry() == NPC_FOCUS_FIRE_DUMMY)
            {
                for (uint8 i = 0; i < 7; ++i)
                    if (Unit* seat = me->GetVehicleKit()->GetPassenger(i))
                        seat->CastSpell(summon, SPELL_FOCUSED_FIRE_CHANNEL, false);
                DoCast(summon, SPELL_FOCUSED_FIRE);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_EYE_OF_OCCUTHAR:
                        DoCastVictim(SPELL_EYES_OF_OCCUTHAR);
                        events.ScheduleEvent(EVENT_EYE_OF_OCCUTHAR, 57500);
                        events.RescheduleEvent(EVENT_FOCUSED_FIRE, 16000);
                        break;
                    case EVENT_FOCUSED_FIRE:
                        DoCastRandom(SPELL_FOCUSED_FIRE_SUMMON, 100.0f);
                        events.ScheduleEvent(EVENT_FOCUSED_FIRE, 16000);
                        break;
                    case EVENT_SEARING_SHADOWS:
                        DoCastVictim(SPELL_SEARING_SHADOWS);
                        events.ScheduleEvent(EVENT_SEARING_SHADOWS, 24000);
                        break;
                    case EVENT_BERSERK:
                        DoCast(me, SPELL_BERSERK, true);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        Vehicle* vehicle;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_occutharAI(creature);
    }
};

class npc_eye_of_occuthar : public CreatureScript
{
public:
    npc_eye_of_occuthar() : CreatureScript("npc_eye_of_occuthar") { }

    struct npc_eye_of_occutharAI : public ScriptedAI
    {
        npc_eye_of_occutharAI(Creature* creature) : ScriptedAI(creature) {}

        void IsSummonedBy(Unit* /*owner*/)
        {
            events.Reset();
            me->SetInCombatWithZone();
            events.ScheduleEvent(EVENT_EYE_EXPLOSION, 10000);
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_EYE_EXPLOSION:
                        DoCastAOE(SPELL_EYE_EXPLODE);
                        me->DespawnOrUnsummon(500);
                        break;
                }
            }
        }
    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_eye_of_occutharAI(creature);
    }
};

class spell_eyes_of_occuthar : public SpellScriptLoader
{
public:
    spell_eyes_of_occuthar() : SpellScriptLoader("spell_eyes_of_occuthar") { }

    class spell_eyes_of_occuthar_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_eyes_of_occuthar_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (Unit* target = GetHitUnit())
                if (target->GetTypeId() == TYPEID_PLAYER)
                    GetCaster()->CastSpell(target, uint32(GetEffectValue()), true);
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_eyes_of_occuthar_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_eyes_of_occuthar_SpellScript();
    }
};

class spell_eyes_of_occuthar_summon : public SpellScriptLoader
{
public:
    spell_eyes_of_occuthar_summon() : SpellScriptLoader("spell_eyes_of_occuthar_summon") { }

    class spell_eyes_of_occuthar_summon_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_eyes_of_occuthar_summon_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* target = GetTarget())
            {
                if (Unit *caster = GetCaster())
                    if (Creature* eye = caster->SummonCreature(NPC_EYE_OF_OCCUTHAR, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 12000))
                    {
                        eye->AddAura(SPELL_EYE_VISUAL, eye);
                        eye->CastSpell(target, SPELL_RIDE_VEHICLE_PLAYER, true);
                        eye->CastSpell(target, SPELL_GAZE_OF_OCCUTHAR);
                    }
            }
        }

        void Register()
        {
            AfterEffectApply += AuraEffectApplyFn(spell_eyes_of_occuthar_summon_AuraScript::OnApply, EFFECT_1, SPELL_AURA_SET_VEHICLE_ID, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_eyes_of_occuthar_summon_AuraScript();
    }
};

void AddSC_boss_occuthar()
{
    new boss_occuthar();
    new npc_eye_of_occuthar();
    new spell_eyes_of_occuthar();
    new spell_eyes_of_occuthar_summon();
}
