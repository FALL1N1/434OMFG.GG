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

enum spells
{
    // rock borer
    SPELL_ROCK_BORE          = 80028,
    SPELL_ROCK_BORE_HC       = 92630,
    // millhouse manastorm
    SPELL_BLUR              = 81216,
    SPELL_FEAR              = 81442,
    SPELL_FROSTBOLT_VOLLEY  = 81440,
    SPELL_IMPENDING_DOOM    = 86830,
    SPELL_SHADOW_BOLT       = 81439,
    SPELL_SHADOWFURY        = 81441,
    SPELL_TIGULE            = 81220,
    SPELL_IMPEMDING_DOOM_E  = 86838,
    //rift conjurer
    SPELL_SHADOW_BOLT_CJ    = 80279,
    SPELL_SUM_IMP           = 80308,
};

enum events
{
    EVENT_MILL_FEAR         = 1,
    EVENT_FROSTBOLT_VOLLEY,
    EVENT_IMPENDING_DOOM,
    EVENT_SHADOW_BOLT,
    EVENT_SHADOWFURY,
    EVENT_SHADOW_BOLT_CJ,
    EVENT_SUM_IMP,
};

Position const millhouseWP [3] =
{
    {985.726f, 889.674f, 304.323f, 2.140f},
    {1057.546f, 864.379f, 293.793f, 2.799f},
    {1149.005f, 883.497f, 284.963f, 3.322f}
};

// Rock Borer AI
class mob_rock_borer : public CreatureScript
{
public:
    mob_rock_borer() : CreatureScript("mob_rock_borer") { }

    struct mob_rock_borerAI : public ScriptedAI
    {
        mob_rock_borerAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            rockboreTimer = urand(5000, 7000);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (rockboreTimer <= diff)
            {
                if (Unit* victim = me->getVictim())
                {
                    if (Aura* aur = victim->GetAura(IsHeroic() ? SPELL_ROCK_BORE_HC : SPELL_ROCK_BORE))
                    {
                        aur->ModStackAmount(1);
                        aur->RefreshDuration();
                    }
                    else
                        DoCastVictim(SPELL_ROCK_BORE);
                }
                rockboreTimer = urand(10000, 12000);
            }else rockboreTimer -= diff;

            DoMeleeAttackIfReady();
        }
    private:
        uint32 rockboreTimer;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_rock_borerAI(creature);
    }
};

// Millhouse Manastorm AI
class mob_millhouse_manastorm : public CreatureScript
{
public:
    mob_millhouse_manastorm() : CreatureScript("mob_millhouse_manastorm") { }

    struct mob_millhouse_manastormAI : public ScriptedAI
    {
        mob_millhouse_manastormAI(Creature* creature) : ScriptedAI(creature)
        {
            phase = 0;
        }

        void Reset()
        {
            events.Reset();
            changePhase = false;
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            end = false;
        }

        void JustReachedHome()
        {
            if (phase == 2)
                DoCast(SPELL_IMPEMDING_DOOM_E);
            else
                DoCast(SPELL_TIGULE);
            changePhase = false;
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_MILL_FEAR, 10000);
            events.ScheduleEvent(EVENT_FROSTBOLT_VOLLEY, urand(7000, 17000));
            events.ScheduleEvent(EVENT_IMPENDING_DOOM, urand(25000, 35000));
            events.ScheduleEvent(EVENT_SHADOW_BOLT, 2000);
            events.ScheduleEvent(EVENT_SHADOWFURY, urand(10000, 15000));
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (end || who->GetEntry() != BOSS_CORBORUS || me->GetDistance(who) > 5.0f)
                return;
            end = true;
            std::list<Creature*> eventCr;
            me->GetCreatureListWithEntryInGrid(eventCr, 43430, 150.0f);
            for (std::list<Creature*>::iterator itr = eventCr.begin(); itr != eventCr.end(); ++itr)
                if (Creature *c = (*itr))
                {
                    c->GetMotionMaster()->MoveJump(1169.36f, 789.21f, 312.0f, 20.0f, 20.0f);
                    c->DespawnOrUnsummon(3000);
                }

            me->GetCreatureListWithEntryInGrid(eventCr, 42696, 150.0f);
            for (std::list<Creature*>::iterator itr = eventCr.begin(); itr != eventCr.end(); ++itr)
                if (Creature *c = (*itr))
                {
                    c->GetMotionMaster()->MoveJump(1169.36f, 789.21f, 312.0f, 20.0f, 20.0f);
                    c->DespawnOrUnsummon(3000);
                }

            me->GetCreatureListWithEntryInGrid(eventCr, 43537, 150.0f);
            for (std::list<Creature*>::iterator itr = eventCr.begin(); itr != eventCr.end(); ++itr)
                if (Creature *c = (*itr))
                {
                    c->GetMotionMaster()->MoveJump(1169.36f, 789.21f, 312.0f, 20.0f, 20.0f);
                    c->DespawnOrUnsummon(3000);
                }

            me->GetMotionMaster()->MoveJump(1169.36f, 789.21f, 312.0f, 20.0f, 20.0f);
            me->DespawnOrUnsummon(3000);
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE && (id == 1 || id == 2))
                me->AI()->EnterEvadeMode();
        }

        void DamageTaken(Unit* attacker, uint32& damage)
        {
            if (changePhase)
                return;
            if (me->GetHealthPct() <= 50)
            {
                changePhase = true;
                switch (phase)
                {
                    case 0:
                    {
                        Talk(0);
                        Position pos = { 984.0f, 890.0f, 304.47f, 0.0f };
                        me->SetHomePosition(pos);
                        me->GetMotionMaster()->MovePoint(1, pos);
                        break;
                    }
                    case 1:
                    {
                        Talk(1);
                        Position pos = { 1147.33f, 882.51f, 284.97f, 0.0f };
                        me->SetHomePosition(pos);
                        me->GetMotionMaster()->MovePoint(2, pos);
                        break;
                    }
                }
                phase++;
                damage = 0;
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim() || changePhase)
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_MILL_FEAR:
                        DoCastRandom(SPELL_FEAR, 0.0f);
                        events.ScheduleEvent(EVENT_MILL_FEAR, 10000);
                        return;
                    case EVENT_SHADOW_BOLT_CJ:
                        DoCastVictim(SPELL_SHADOW_BOLT_CJ);
                        events.ScheduleEvent(EVENT_SHADOW_BOLT_CJ, 2000);
                        return;
                    case EVENT_FROSTBOLT_VOLLEY:
                        DoCast(SPELL_FROSTBOLT_VOLLEY);
                        events.ScheduleEvent(EVENT_FROSTBOLT_VOLLEY, urand(8000, 15000));
                        return;
                    case EVENT_IMPENDING_DOOM:
                        DoCast(SPELL_IMPENDING_DOOM);
                        DoCast(me, SPELL_IMPEMDING_DOOM_E, true);
                        events.ScheduleEvent(EVENT_IMPENDING_DOOM, urand(60000, 65000));
                        return;
                    case EVENT_SHADOWFURY:
                        DoCastRandom(SPELL_SHADOWFURY, 0.0f);
                        events.ScheduleEvent(EVENT_SHADOWFURY, urand(8000, 20000));
                        return;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
        uint32 phase;
        bool changePhase;
        bool end;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_millhouse_manastormAI(creature);
    }
};

class mob_stonecore_rift_conjurer : public CreatureScript
{
public:
    mob_stonecore_rift_conjurer() : CreatureScript("mob_stonecore_rift_conjurer") { }

    struct mob_stonecore_rift_conjurerAI : public ScriptedAI
    {
        mob_stonecore_rift_conjurerAI(Creature* creature) : ScriptedAI(creature), _summons(creature) { }

        void Reset()
        {
            events.Reset();
            _summons.DespawnAll();
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_SHADOW_BOLT, urand(5000, 7000));
            events.ScheduleEvent(EVENT_SUM_IMP, 15000);
        }

        void JustSummoned(Creature* summoned)
        {
            _summons.Summon(summoned);
        }

        void JustDied(Unit* /*killer*/)
        {
            _summons.DespawnAll();
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
                    case EVENT_SHADOW_BOLT:
                        DoCastRandom(SPELL_SHADOW_BOLT, 0.0f);
                        events.ScheduleEvent(EVENT_SHADOW_BOLT, urand(3000, 7000));
                        break;
                    case EVENT_SUM_IMP:
                        DoCast(SPELL_SUM_IMP);
                        events.ScheduleEvent(EVENT_SUM_IMP, 30000);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
        SummonList _summons;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_stonecore_rift_conjurerAI(creature);
    }
};

enum giantEvent
{
    EVENT_SECOUSSE = 1,
    EVENT_CRISTAL_ECLAT,
};

enum giantSpell
{
    SPELL_SECOUSSE = 81008,
    SPELL_CRISTAL_ECLAT = 81015,
    SPELL_CRISTAL_ECLAT_TRIG_1 = 92165, // summon 49473
    SPELL_CRISTAL_ECLAT_TRIG_2 = 92176,
    SPELL_CRISTAL_AURA = 80919,
    SPELL_CRISTAL_SUMMON = 92150,

    SPELL_RANDOM_AGGRO = 92111,
    SPELL_CRISTAL_EXPLOSION = 80913,
};

class CristalEvent : public BasicEvent
{
public:
    CristalEvent(Creature* caster) : _caster(caster) {}

    bool Execute(uint64 execTime, uint32 /*diff*/)
    {
        _caster->CastSpell(_caster, SPELL_CRISTAL_SUMMON, true);
        _caster->DespawnOrUnsummon(100);
        return true;
    }

private:
    Creature* _caster;
};

class mob_stonecore_crystalspawn_giant : public CreatureScript
{
public:
    mob_stonecore_crystalspawn_giant() : CreatureScript("mob_stonecore_crystalspawn_giant") { }

    struct mob_stonecore_crystalspawn_giantAI : public ScriptedAI
    {
        mob_stonecore_crystalspawn_giantAI(Creature* creature) : ScriptedAI(creature), _summons(creature) { }

        void Reset()
        {
            events.Reset();
            _summons.DespawnAll();
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_CRISTAL_ECLAT, urand(5000, 7000));
            events.ScheduleEvent(EVENT_SECOUSSE, 5000);
        }

        void JustSummoned(Creature* summon)
        {
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
            summon->CastSpell(summon, SPELL_CRISTAL_AURA, true);
            summon->m_Events.AddEvent(new CristalEvent(summon), summon->m_Events.CalculateTime(4000));
            _summons.Summon(summon);
        }

        void JustDied(Unit* /*killer*/)
        {
            _summons.DespawnAll();
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
                    case EVENT_CRISTAL_ECLAT:
                        DoCastRandom(SPELL_CRISTAL_ECLAT, 0.0f);
                        events.ScheduleEvent(EVENT_CRISTAL_ECLAT, urand(5000, 11000));
                        break;
                    case EVENT_SECOUSSE:
                        DoCast(SPELL_SECOUSSE);
                        events.ScheduleEvent(EVENT_SECOUSSE, 30000);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
        SummonList _summons;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_stonecore_crystalspawn_giantAI(creature);
    }
};

class mob_stonecore_cristal : public CreatureScript
{
public:
    mob_stonecore_cristal() : CreatureScript("mob_stonecore_cristal") { }

    struct mob_stonecore_cristalAI : public ScriptedAI
    {
        mob_stonecore_cristalAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            mui_check_dist = 3000; // safe timer
            mui_check_aggro = 100;
            me->SetInCombatWithZone();
            me->SetSpeed(MOVE_WALK, 0.9f);
            me->SetSpeed(MOVE_RUN, 0.9f);
        }


        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (mui_check_aggro <= diff)
            {
                DoCastAOE(SPELL_RANDOM_AGGRO);
                mui_check_aggro = 300000;
            }
            else
                mui_check_aggro -= diff;

            if (mui_check_dist <= diff)
            {
                if (Unit *c = me->getVictim())
                    if (me->GetDistance(c) <= 1.5f)
                    {
                        me->CastSpell(c, SPELL_CRISTAL_EXPLOSION, true);
                        me->DespawnOrUnsummon(500);
                    }
                mui_check_dist = 500;
            }
            else
                mui_check_dist -= diff;
        }

    private:
        uint32 mui_check_dist;
        uint32 mui_check_aggro;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_stonecore_cristalAI(creature);
    }
};

class spell_cristal_giant_secousse : public SpellScriptLoader
{
    class SecousseCheck
    {
    public:
        SecousseCheck()
        {
        }

        bool operator() (WorldObject* obj)
        {
            if (Unit *target = obj->ToUnit())
                if (target->HasUnitMovementFlag(MOVEMENTFLAG_FALLING))
                    return true;
            return false;
        }
    };

public:
    spell_cristal_giant_secousse() : SpellScriptLoader("spell_cristal_giant_secousse")
    {
    }

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        bool Load()
        {
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(SecousseCheck());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }
};


void AddSC_the_stonecore()
{
    new mob_rock_borer();
    new mob_millhouse_manastorm();
    new mob_stonecore_rift_conjurer();
    new mob_stonecore_crystalspawn_giant();
    new spell_cristal_giant_secousse();
    new mob_stonecore_cristal();
}
