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

#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "baradin_hold.h"

enum Texts
{
    SAY_INTRO           = 1,
    SAY_AGGRO           = 2,
    SAY_HATE            = 3,
    SAY_SKEWER          = 4,
    SAY_SKEWER_ANNOUNCE = 5,
    SAY_BLADE_STORM     = 6,
    SAY_SLAY            = 10,
    SAY_DEATH           = 12,
};

enum Spells
{
    SPELL_BLADE_DANCE        = 105726,
    SPELL_BLADE_DANCE_VISUAL = 105828,
    SPELL_SEETHING_HATE      = 105067,
    SPELL_SKEWER             = 104936,
    SPELL_BERSERK            = 47008,
};

enum Misc
{
    ACTION_INTRO    = 1,
    GROUP_SPECIAL   = 2
};

enum Events
{
    EVENT_SEETHING_HATE   = 1,
    EVENT_SKEWER          = 2,
    EVENT_MOVE_DANCE      = 3,
    EVENT_BLADE_DANCE     = 4,
    EVENT_BERSERK         = 5
};

class at_alizabal_intro : public AreaTriggerScript
{
    public:
        at_alizabal_intro() : AreaTriggerScript("at_alizabal_intro") { }

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*areaTrigger*/)
        {
            if (InstanceScript* instance = player->GetInstanceScript())
                if (Creature* alizabal = ObjectAccessor::GetCreature(*player, instance->GetData64(DATA_ALIZABAL)))
                    alizabal->AI()->DoAction(ACTION_INTRO);
            return true;
        }
};

class boss_alizabal : public CreatureScript
{
    public:
        boss_alizabal() : CreatureScript("boss_alizabal") { }

        struct boss_alizabalAI : public BossAI
        {
            boss_alizabalAI(Creature* creature) : BossAI(creature, DATA_ALIZABAL)
            {
                introDone = false;
            }

            void Reset()
            {
                _Reset();
                firstEvent = true;
                moveCount = 0;
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(SAY_AGGRO);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                events.ScheduleEvent(RAND(EVENT_SKEWER, EVENT_SEETHING_HATE), 5500, GROUP_SPECIAL);
                events.ScheduleEvent(EVENT_BLADE_DANCE, 26000);
                events.ScheduleEvent(EVENT_BERSERK, 5 * MINUTE * IN_MILLISECONDS);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }

            void KilledUnit(Unit* who)
            {
                if (who->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_SLAY);
            }

            void EnterEvadeMode()
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                me->GetMotionMaster()->MoveTargetedHome();
                _DespawnAtEvade();
            }

            void DoAction(int32 const action)
            {
                if (introDone)
                    return;

                introDone = true;
                Talk(SAY_INTRO);
            }

            void UpdateAI(uint32 const diff)
            {
                events.Update(diff);

                if (!me->HasAura(SPELL_BLADE_DANCE_VISUAL) && !UpdateVictim())
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SKEWER:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO, 0))
                            {
                                DoCast(target, SPELL_SKEWER, true);
                                Talk(SAY_SKEWER);
                                Talk(SAY_SKEWER_ANNOUNCE, target->GetGUID());
                            }

                            if (firstEvent)
                            {
                                firstEvent = false;
                                events.ScheduleEvent(EVENT_SEETHING_HATE, 9000, GROUP_SPECIAL);
                            }
                            events.ScheduleEvent(EVENT_SKEWER, 20500, GROUP_SPECIAL);
                            break;
                        case EVENT_SEETHING_HATE:
                            Talk(SAY_HATE);
                            DoCastRandom(SPELL_SEETHING_HATE, 100.00f);
                            if (firstEvent)
                            {
                                firstEvent = false;
                                events.ScheduleEvent(EVENT_SKEWER, 9000, GROUP_SPECIAL);
                            }
                            events.ScheduleEvent(EVENT_SEETHING_HATE, 20500, GROUP_SPECIAL);
                            break;
                        case EVENT_BLADE_DANCE:
                            Talk(SAY_BLADE_STORM);
                            DoCast(me, SPELL_BLADE_DANCE_VISUAL);
                            DoCastRandom(SPELL_BLADE_DANCE, 150.00f, false);
                            firstEvent = true;
                            lastDance = false;
                            events.CancelEventGroup(GROUP_SPECIAL);
                            events.ScheduleEvent(RAND(EVENT_SKEWER, EVENT_SEETHING_HATE), 21000, GROUP_SPECIAL);
                            events.ScheduleEvent(EVENT_MOVE_DANCE, 4300);
                            events.ScheduleEvent(EVENT_BLADE_DANCE, 60000);
                            break;
                        case EVENT_MOVE_DANCE:
                            DoCastRandom(SPELL_BLADE_DANCE, 150.00f, false);
                            if (!lastDance)
                            {
                                lastDance = true;
                                events.ScheduleEvent(EVENT_MOVE_DANCE, 4300);
                            }
                            break;
                        case EVENT_BERSERK:
                            DoCast(me, SPELL_BERSERK, true);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
            private:
                bool introDone;
                bool firstEvent;
                bool lastDance;
                uint8 moveCount;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_alizabalAI(creature);
        }
};

void AddSC_boss_alizabal()
{
    new boss_alizabal();
    new at_alizabal_intro();
}
