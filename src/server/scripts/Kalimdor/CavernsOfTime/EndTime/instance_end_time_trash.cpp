/*
* OMFG.GG
*/

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "PassiveAI.h"
#include "SpellScript.h"
#include "MoveSplineInit.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "endTime.h"

enum Spells
{
    SPELL_ARCANE_BLAST  = 101816,
    SPELL_BLINK         = 101812,
    
    SPELL_FACE_KICK     = 101888,
    SPELL_PUTRID_SPIT   = 102063,
    SPELL_WALL_OF_DEAD  = 101891,

    SPELL_SHOOT         = 102410,
    SPELL_MULTISHOT     = 102411,

    SPELL_PW_SHIELD     = 102409,
    SPELL_FNT_O_LIGHT   = 102405,
    SPELL_LIGHT_RAIN    = 102406,

    SPELL_CADAVER_TOSS  = 109952,
    SPELL_CANNIBALIZE   = 109944,
    SPELL_FLESH_RIP     = 102066,

    SPELL_SHIELD_BASH   = 101817,
    SPELL_SHIELD_WALL   = 101811,
    SPELL_THUNDERCLAP   = 101820,

    SPELL_ENRAGE        = 102134,
    SPELL_FLAME_BREATH  = 102135,

    SPELL_ARCANE_WAVE   = 102601,
    SPELL_TEMP_VORTEX   = 102600,

    SPELL_VOID_SHIELD   = 102599,
    SPELL_VOID_STRIKE   = 102598,
};

enum Events
{
    EVENT_PUTRID_SPIT       = 1,
    EVENT_FACE_KICK         = 2,
    EVENT_FOUNTAIN_O_LIGHT  = 3,
    EVENT_PW_SHIELD         = 4,
    EVENT_CADAVER_TOSS      = 5,
    EVENT_CABBABALIZE       = 6,
    EVENT_FLESH_RIP         = 7,
    EVENT_SHIELD_BASH       = 8,
    EVENT_SHIELD_WALL       = 9,
    EVENT_THUNDERCLAP       = 10,
    EVENT_FLAME_BREATH      = 11,
    EVENT_ENRAGE            = 12,
    EVENT_ARCANE_WAVE       = 13,
    EVENT_TEMP_VORTEX       = 14,
    EVENT_VOID_SHIELD       = 15,
    EVENT_VOID_STRIKE       = 16
};

// 54691
class npc_et_time_twisted_sourceress : public CreatureScript
{
public:
    npc_et_time_twisted_sourceress() : CreatureScript("npc_et_time_twisted_sourceress") {}

    struct npc_et_time_twisted_sourceressAI : public ScriptedAI
    {
        npc_et_time_twisted_sourceressAI(Creature * creature) : ScriptedAI(creature) { }

        void UpdateAI(const uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (me->HasAuraWithMechanic((1 << MECHANIC_FREEZE) | (1 << MECHANIC_ROOT)))
                DoCast(SPELL_BLINK);

            DoCastVictim(SPELL_ARCANE_BLAST, false);
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_et_time_twisted_sourceressAI(creature);
    }
};

// 54507
class npc_et_time_twisted_scourge_breast : public CreatureScript
{
public:
    npc_et_time_twisted_scourge_breast() : CreatureScript("npc_et_time_twisted_scourge_breast") {}

    struct npc_et_time_twisted_scourge_breastAI : public ScriptedAI
    {
        npc_et_time_twisted_scourge_breastAI(Creature * creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            events.Reset();
        }

        void EnterCombat(Unit* victim) override
        {
            events.ScheduleEvent(EVENT_FACE_KICK, 5000);
            events.ScheduleEvent(EVENT_PUTRID_SPIT, 2000);
        }

        void UpdateAI(const uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_FACE_KICK:
                        DoCastVictim(SPELL_FACE_KICK);
                        events.ScheduleEvent(EVENT_FACE_KICK, 10000);
                        break;
                    case EVENT_PUTRID_SPIT:
                        DoCastRandom(SPELL_PUTRID_SPIT, 30.0f);
                        events.ScheduleEvent(EVENT_PUTRID_SPIT, 6000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_et_time_twisted_scourge_breastAI(creature);
    }
};

// 54693
class npc_et_time_twisted_rifleman : public CreatureScript
{
public:
    npc_et_time_twisted_rifleman() : CreatureScript("npc_et_time_twisted_rifleman") {}

    struct npc_et_time_twisted_riflemanAI : public ScriptedAI
    {
        npc_et_time_twisted_riflemanAI(Creature * creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            casts = 0;
        }

        void UpdateAI(const uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (casts < 4)
            {
                DoCastRandom(SPELL_SHOOT, 40.0f);
                ++casts;
            }
            else
            {
                DoCastRandom(SPELL_MULTISHOT, 40.0f);
                casts = 0;
            }
        }

    private:
        uint8 casts;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_et_time_twisted_riflemanAI(creature);
    }
};

// 54690
class npc_et_time_twisted_priest : public CreatureScript
{
public:
    npc_et_time_twisted_priest() : CreatureScript("npc_et_time_twisted_priest") {}

    struct npc_et_time_twisted_priestAI : public ScriptedAI
    {
        npc_et_time_twisted_priestAI(Creature * creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            events.Reset();
        }

        void JustSummoned(Creature* summon) override
        {
            summon->SetReactState(REACT_PASSIVE);
            summon->CastSpell(summon, SPELL_LIGHT_RAIN, true);
        }

        void SummonedCreatureDies(Creature* /*summon*/, Unit* /*killer*/) override
        {
            events.ScheduleEvent(EVENT_FOUNTAIN_O_LIGHT, 20000);
        }

        void EnterCombat(Unit* victim) override
        {
            events.ScheduleEvent(EVENT_FOUNTAIN_O_LIGHT, 3000);
            events.ScheduleEvent(EVENT_PW_SHIELD, 5000);
        }

        void UpdateAI(const uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_FOUNTAIN_O_LIGHT:
                        DoCast(SPELL_FNT_O_LIGHT);
                        break;
                    case EVENT_PW_SHIELD:
                        if (Unit* target = DoSelectLowestHpFriendly(40.0f))
                            DoCast(target, SPELL_PW_SHIELD);
                        events.ScheduleEvent(EVENT_PW_SHIELD, 20000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_et_time_twisted_priestAI(creature);
    }
};

// 54511
class npc_et_time_twisted_geist : public CreatureScript
{
public:
    npc_et_time_twisted_geist() : CreatureScript("npc_et_time_twisted_geist") {}

    struct npc_et_time_twisted_geistAI : public ScriptedAI
    {
        npc_et_time_twisted_geistAI(Creature * creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            events.Reset();
        }

        void EnterCombat(Unit* victim) override
        {
            events.ScheduleEvent(EVENT_CADAVER_TOSS, 13000);
            events.ScheduleEvent(EVENT_CABBABALIZE, 5000);
            events.ScheduleEvent(EVENT_FLESH_RIP, 2000);
        }

        void UpdateAI(const uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_CADAVER_TOSS:
                        DoCastRandom(SPELL_CADAVER_TOSS, 45.0f);
                        break;
                    case EVENT_CABBABALIZE:
                        DoCast(SPELL_CANNIBALIZE);
                        events.ScheduleEvent(EVENT_CABBABALIZE, 40000);
                        break;
                    case EVENT_FLESH_RIP:
                        DoCastVictim(SPELL_FLESH_RIP);
                        events.ScheduleEvent(EVENT_FLESH_RIP, 15000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_et_time_twisted_geistAI(creature);
    }
};

// 54687
class npc_et_time_twisted_footman : public CreatureScript
{
public:
    npc_et_time_twisted_footman() : CreatureScript("npc_et_time_twisted_footman") {}

    struct npc_et_time_twisted_footmanAI : public ScriptedAI
    {
        npc_et_time_twisted_footmanAI(Creature * creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            events.Reset();
        }

        void EnterCombat(Unit* victim) override
        {
            events.ScheduleEvent(EVENT_SHIELD_BASH, 3000);
            events.ScheduleEvent(EVENT_SHIELD_WALL, 12000);
            events.ScheduleEvent(EVENT_THUNDERCLAP, 8000);
        }

        void UpdateAI(const uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_SHIELD_BASH:
                        DoCastVictim(SPELL_SHIELD_BASH);
                        events.ScheduleEvent(EVENT_SHIELD_BASH, 12000);
                        break;
                    case EVENT_SHIELD_WALL:
                        DoCast(SPELL_SHIELD_WALL);
                        events.ScheduleEvent(EVENT_SHIELD_WALL, 20000);
                        break;
                    case EVENT_THUNDERCLAP:
                        DoCastVictim(SPELL_THUNDERCLAP);
                        events.ScheduleEvent(EVENT_THUNDERCLAP, 16000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_et_time_twisted_footmanAI(creature);
    }
};

// 54543
class npc_et_time_twisted_drake : public CreatureScript
{
public:
    npc_et_time_twisted_drake() : CreatureScript("npc_et_time_twisted_drake") {}

    struct npc_et_time_twisted_drakeAI : public ScriptedAI
    {
        npc_et_time_twisted_drakeAI(Creature * creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            events.Reset();
        }

        void EnterCombat(Unit* victim) override
        {
            events.ScheduleEvent(EVENT_ENRAGE, 20000);
            events.ScheduleEvent(EVENT_FLAME_BREATH, 15000);
        }

        void UpdateAI(const uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_ENRAGE:
                        DoCast(SPELL_ENRAGE);
                        events.ScheduleEvent(EVENT_ENRAGE, 20000);
                        break;
                    case EVENT_FLAME_BREATH:
                        DoCast(SPELL_FLAME_BREATH);
                        events.ScheduleEvent(EVENT_FLAME_BREATH, 15000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_et_time_twisted_drakeAI(creature);
    }
};

// 54920
class npc_et_infinite_supressor : public CreatureScript
{
public:
    npc_et_infinite_supressor() : CreatureScript("npc_et_infinite_supressor") {}

    struct npc_et_infinite_supressorAI : public ScriptedAI
    {
        npc_et_infinite_supressorAI(Creature * creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            events.Reset();
        }

        void EnterCombat(Unit* victim) override
        {
            events.ScheduleEvent(EVENT_ARCANE_WAVE, 3000);
            events.ScheduleEvent(EVENT_TEMP_VORTEX, 7000);
        }

        void UpdateAI(const uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_ARCANE_WAVE:
                        DoCastRandom(SPELL_ARCANE_WAVE, 45.0f);
                        events.ScheduleEvent(EVENT_ARCANE_WAVE, 20000);
                        break;
                    case EVENT_TEMP_VORTEX:
                        DoCastRandom(SPELL_TEMP_VORTEX, 45.0f);
                        events.ScheduleEvent(EVENT_TEMP_VORTEX, 15000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_et_infinite_supressorAI(creature);
    }
};

// 54923
class npc_et_infinite_warden : public CreatureScript
{
public:
    npc_et_infinite_warden() : CreatureScript("npc_et_infinite_warden") {}

    struct npc_et_infinite_wardenAI : public ScriptedAI
    {
        npc_et_infinite_wardenAI(Creature * creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            events.Reset();
        }

        void EnterCombat(Unit* victim) override
        {
            events.ScheduleEvent(EVENT_VOID_SHIELD, 3000);
            events.ScheduleEvent(EVENT_VOID_STRIKE, 7000);
        }

        void UpdateAI(const uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_VOID_SHIELD:
                        DoCast(SPELL_VOID_SHIELD);
                        events.ScheduleEvent(EVENT_VOID_SHIELD, 20000);
                        break;
                    case EVENT_VOID_STRIKE:
                        DoCastRandom(EVENT_VOID_STRIKE, 45.0f);
                        events.ScheduleEvent(EVENT_VOID_STRIKE, 5000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_et_infinite_wardenAI(creature);
    }
};

void AddSC_end_time_trash()
{
    new npc_et_infinite_supressor();
    new npc_et_time_twisted_drake();
    new npc_et_time_twisted_footman();
    new npc_et_time_twisted_geist();
    new npc_et_time_twisted_priest();
    new npc_et_time_twisted_rifleman();
    new npc_et_time_twisted_scourge_breast();
    new npc_et_time_twisted_sourceress();
    new npc_et_infinite_warden();
}