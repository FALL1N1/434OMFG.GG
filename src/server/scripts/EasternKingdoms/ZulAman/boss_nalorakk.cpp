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

#include "GridNotifiers.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "zulaman.h"

//Trash Waves
float NalorakkWay[8][3] =
{
    { 18.569f, 1414.512f, 11.42f}, // waypoint 1
    {-17.264f, 1419.551f, 12.62f},
    {-52.642f, 1419.357f, 27.31f}, // waypoint 2
    {-69.908f, 1419.721f, 27.31f},
    {-79.929f, 1395.958f, 27.31f},
    {-80.072f, 1374.555f, 40.87f}, // waypoint 3
    {-80.072f, 1314.398f, 40.87f},
    {-80.072f, 1295.775f, 48.60f} // waypoint 4
};

// Fight
enum says
{
    YELL_AGGRO          = 0,
    YELL_KILL           = 1,
    YELL_DEATH          = 2,
    YELL_BERSERK        = 3,
    YELL_SURGE          = 4,
    YELL_SHIFTEDTOTROLL = 5,
    YELL_SHIFTEDTOBEAR  = 6,
    YELL_NALORAKK_WAVE1 = 7,
    YELL_NALORAKK_WAVE2 = 8,
    YELL_NALORAKK_WAVE3 = 9,
    YELL_NALORAKK_WAVE4 = 10,
};

enum Spells
{
    // Normal Form
    SPELL_BRUTAL_STRIKE             = 42384,
    SPELL_SURGE                     = 42402,
    SPELL_BEAR_FORM                 = 42594,

    // Bear Form
    SPELL_LACERATING_SLASH          = 42395,
    SPELL_REND_FLESH                = 42397,
    SPELL_DEAFENING_ROAR            = 42398,

    SPELL_BERSERK                   = 45078
};

enum Events
{
    EVENT_SURGE = 1,
    EVENT_BRUTAL_STRIKE,
    EVENT_BEAR_FORM,

    EVENT_LACERATING_SLASH,
    EVENT_REND_FLESH,
    EVENT_DEAFENING_ROAR,
    EVENT_NORMAL_FORM,

    EVENT_BERSERK

};


class boss_nalorakk : public CreatureScript
{
    public:
        boss_nalorakk() : CreatureScript("boss_nalorakk") { }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_nalorakkAI(creature);
        }

        struct boss_nalorakkAI : public BossAI
        {
            boss_nalorakkAI(Creature* creature) : BossAI(creature, DATA_NALORAKKEVENT)
            {
                MoveEvent = true;
                MovePhase = 0;
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;
            EventMap events;

            bool MoveEvent;
            bool inMove;
            uint32 MovePhase;
            uint32 waitTimer;

            void Reset()
            {
                _Reset();
                if (MoveEvent)
                {
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                }
                else
                {
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                }
                inMove = false;
                waitTimer = 0;
                me->SetSpeed(MOVE_RUN, 2);
                me->SetWalk(false);
                MovePhase = 0;
            }

            void SendAttacker(Unit* target)
            {
                std::list<Creature*> templist;
                float x, y, z;
                me->GetPosition(x, y, z);

                {
                    Trinity::AllFriendlyCreaturesInGrid check(me);
                    Trinity::CreatureListSearcher<Trinity::AllFriendlyCreaturesInGrid> searcher(me, templist, check);
                    me->VisitNearbyGridObject(me->GetGridActivationRange(), searcher);
                }

                if (templist.empty())
                    return;

                for (std::list<Creature*>::const_iterator i = templist.begin(); i != templist.end(); ++i)
                {
                    if ((*i) && me->IsWithinDistInMap((*i), 25))
                    {
                        (*i)->SetNoCallAssistance(true);
                        (*i)->AI()->AttackStart(target);
                    }
                }
            }

            void AttackStart(Unit* who)
            {
                if (!MoveEvent)
                    ScriptedAI::AttackStart(who);
            }

            void MoveInLineOfSight(Unit* who)
            {
                if (!MoveEvent)
                {
                    ScriptedAI::MoveInLineOfSight(who);
                }
                else
                {
                    if (me->IsHostileTo(who))
                    {
                        if (!inMove)
                        {
                            switch (MovePhase)
                            {
                                case 0:
                                    if (me->IsWithinDistInMap(who, 50))
                                    {
                                        Talk(YELL_NALORAKK_WAVE1);
                                        me->GetMotionMaster()->MovePoint(1, NalorakkWay[1][0], NalorakkWay[1][1], NalorakkWay[1][2]);
                                        MovePhase ++;
                                        inMove = true;

                                        SendAttacker(who);
                                    }
                                    break;
                                case 2:
                                    if (me->IsWithinDistInMap(who, 40))
                                    {
                                        Talk(YELL_NALORAKK_WAVE2);
                                        me->GetMotionMaster()->MovePoint(3, NalorakkWay[3][0], NalorakkWay[3][1], NalorakkWay[3][2]);
                                        MovePhase ++;
                                        inMove = true;

                                        SendAttacker(who);
                                    }
                                    break;
                                case 5:
                                    if (me->IsWithinDistInMap(who, 40))
                                    {
                                        Talk(YELL_NALORAKK_WAVE3);
                                        me->GetMotionMaster()->MovePoint(6, NalorakkWay[6][0], NalorakkWay[6][1], NalorakkWay[6][2]);
                                        MovePhase ++;
                                        inMove = true;

                                        SendAttacker(who);
                                    }
                                    break;
                                case 7:
                                    if (me->IsWithinDistInMap(who, 50))
                                    {
                                        SendAttacker(who);

                                        Talk(YELL_NALORAKK_WAVE4);
                                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

                                        MoveEvent = false;
                                        me->SetHomePosition(NalorakkWay[6][0], NalorakkWay[6][1], NalorakkWay[6][2], me->GetOrientation());
                                    }
                                    break;
                            }
                        }
                    }
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                events.Reset();
                events.ScheduleEvent(EVENT_SURGE, 8000);
                events.ScheduleEvent(EVENT_BRUTAL_STRIKE, 13000);
                events.ScheduleEvent(EVENT_BEAR_FORM, 30000);
                events.ScheduleEvent(EVENT_BERSERK, 600000);

                Talk(YELL_AGGRO);
                DoZoneInCombat();
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();

                Talk(YELL_DEATH);
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(YELL_KILL);
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (MoveEvent)
                {
                    if (type != POINT_MOTION_TYPE)
                        return;

                    if (!inMove)
                        return;

                    if (MovePhase != id)
                        return;

                    switch (MovePhase)
                    {
                        case 2:
                            me->SetOrientation(3.1415f*2);
                            inMove = false;
                            return;
                        case 1:
                        case 3:
                        case 4:
                        case 6:
                            MovePhase ++;
                            waitTimer = 1;
                            inMove = true;
                            return;
                        case 5:
                            me->SetOrientation(3.1415f*0.5f);
                            inMove = false;
                            return;
                        case 7:
                            me->SetOrientation(3.1415f*0.5f);
                            inMove = false;
                            return;
                    }

                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (waitTimer && inMove)
                {
                    if (waitTimer <= diff)
                    {
                        me->GetMotionMaster()->MovementExpired();
                        me->GetMotionMaster()->MovePoint(MovePhase, NalorakkWay[MovePhase][0], NalorakkWay[MovePhase][1], NalorakkWay[MovePhase][2]);
                        waitTimer = 0;
                    } else waitTimer -= diff;
                }

                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SURGE:
                            if(Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0))
                                me->CastSpell(target, SPELL_SURGE, false);
                            events.ScheduleEvent(EVENT_SURGE, 8000);
                            break;
                        case EVENT_BRUTAL_STRIKE:
                            DoCastVictim(SPELL_BRUTAL_STRIKE);
                            events.ScheduleEvent(EVENT_BRUTAL_STRIKE, 8000);
                            break;
                        case EVENT_BEAR_FORM:
                            me->CastSpell(me, SPELL_BEAR_FORM, false);
                            events.CancelEvent(EVENT_SURGE);
                            events.CancelEvent(EVENT_BRUTAL_STRIKE);
                            events.ScheduleEvent(EVENT_LACERATING_SLASH, 1500);
                            events.ScheduleEvent(EVENT_REND_FLESH, 3000);
                            events.ScheduleEvent(EVENT_DEAFENING_ROAR, 11000);
                            events.ScheduleEvent(EVENT_NORMAL_FORM, 30000);
                            break;
                        case EVENT_LACERATING_SLASH:
                            DoCastVictim(SPELL_LACERATING_SLASH);
                            break;
                        case EVENT_REND_FLESH:
                            DoCastVictim(SPELL_REND_FLESH);
                            events.ScheduleEvent(EVENT_REND_FLESH, 10000);
                            break;
                        case EVENT_DEAFENING_ROAR:
                            me->CastSpell(me, SPELL_DEAFENING_ROAR, false);
                            events.ScheduleEvent(EVENT_DEAFENING_ROAR, 10000);
                            break;
                        case EVENT_NORMAL_FORM:
                            me->RemoveAura(SPELL_BEAR_FORM);
                            events.CancelEvent(EVENT_LACERATING_SLASH);
                            events.CancelEvent(EVENT_REND_FLESH);
                            events.CancelEvent(EVENT_DEAFENING_ROAR);
                            events.ScheduleEvent(EVENT_SURGE, 2000);
                            events.ScheduleEvent(EVENT_BRUTAL_STRIKE, 7000);
                            events.ScheduleEvent(EVENT_BEAR_FORM, 30000); // 22:32:02.015

                            events.ScheduleEvent(EVENT_SURGE, 10000);
                            events.ScheduleEvent(EVENT_BRUTAL_STRIKE, 15000);
                            events.ScheduleEvent(EVENT_BEAR_FORM, 30000);
                            break;
                        case EVENT_BERSERK:
                            me->CastSpell(me, SPELL_BERSERK, true);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

void AddSC_boss_nalorakk()
{
    new boss_nalorakk();
}
