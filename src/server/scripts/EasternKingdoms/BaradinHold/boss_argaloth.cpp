/*
 * Copyright (C) 2013-2014 OMFG.GG Network <http://www.omfg.gg/>
 * Copyright (C) 2011-2013 Devastion Cataclysm <http://www.cataclysm-wow.eu/>
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
#include "baradin_hold.h"

enum Spells
{
    SPELL_BERSERK               = 47008,
    SPELL_CONSUMING_DARKNESS    = 88954,
    SPELL_METEOR_SLASH          = 88942,
    SPELL_FEL_FIRESTORM         = 88972,
};

enum Events
{
    EVENT_BERSERK               = 1,
    EVENT_CONSUMING_DARKNESS    = 2,
    EVENT_METEOR_SLASH          = 3,
};

class boss_argaloth: public CreatureScript
{
public:
    boss_argaloth() : CreatureScript("boss_argaloth") { }

    struct boss_argalothAI: public BossAI
    {
        boss_argalothAI(Creature* creature) : BossAI(creature, DATA_ARGALOTH) { }

        uint8 FirestormCount;

        void Reset()
        {
            _Reset();
            FirestormCount = 0;
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            events.ScheduleEvent(EVENT_BERSERK, 300000);
            events.ScheduleEvent(EVENT_CONSUMING_DARKNESS, urand(20000, 25000));
            events.ScheduleEvent(EVENT_METEOR_SLASH, urand(15000, 20000));
        }

        void JustDied(Unit* /*who*/)
        {
            _JustDied();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        }

        void EnterEvadeMode()
        {
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            BossAI::EnterEvadeMode();
        }

        void DamageTaken(Unit* /*damager*/, uint32& /*damage*/)
        {
            if (me->GetHealthPct() < 66 && FirestormCount == 0)
            {
                DoCast(SPELL_FEL_FIRESTORM);
                events.DelayEvents(3000);
                FirestormCount++;
            }

            if (me->GetHealthPct() < 33 && FirestormCount == 1)
            {
                DoCast(SPELL_FEL_FIRESTORM);
                events.DelayEvents(3000);
                FirestormCount++;
            }
        }

        void UpdateAI(const uint32 diff)
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
                    case EVENT_CONSUMING_DARKNESS:
                        CastCunsumingDarkness();
                        events.RescheduleEvent(EVENT_CONSUMING_DARKNESS, urand(20000, 25000));
                        break;
                    case EVENT_METEOR_SLASH:
                        DoCast(SPELL_METEOR_SLASH);
                        events.RescheduleEvent(EVENT_METEOR_SLASH, urand(15000, 20000));
                        break;
                    case EVENT_BERSERK:
                        DoCast(me, SPELL_BERSERK);
                        break;
                }
            }

            DoMeleeAttackIfReady();
            EnterEvadeIfOutOfCombatArea(diff);
        }

        void CastCunsumingDarkness()
        {
            std::list<Unit*> targets;
            SelectTargetList(targets, Is25ManRaid() ? 8 : 3, SELECT_TARGET_RANDOM, 0.0f, true, -SPELL_CONSUMING_DARKNESS);
            if (targets.size() > 1)
                for (std::list<Unit*>::iterator itr = targets.begin(); itr != targets.end(); ++itr)
                    DoCast(*itr, SPELL_CONSUMING_DARKNESS);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_argalothAI(creature);
    }
};

class spell_argaloth_consuming_darkness : public SpellScriptLoader
{
public:
    spell_argaloth_consuming_darkness() : SpellScriptLoader("spell_argaloth_consuming_darkness") {}

    class spell_argaloth_consuming_darkness_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_argaloth_consuming_darkness_AuraScript);

        void OnUpdate(AuraEffect* aurEff)
        {
            if (aurEff->GetTickNumber() > 1)
                aurEff->SetAmount(1.1f * aurEff->GetAmount());
        }

        void Register()
        {
            OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_argaloth_consuming_darkness_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_argaloth_consuming_darkness_AuraScript();
    }
};

void AddSC_boss_argaloth()
{
    new boss_argaloth();
    new spell_argaloth_consuming_darkness();
}
