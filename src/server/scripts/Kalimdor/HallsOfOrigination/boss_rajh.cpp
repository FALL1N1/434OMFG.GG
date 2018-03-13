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

#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "halls_of_origination.h"

enum Texts
{
    SAY_AGGRO       = 0,
    SAY_ENERGIZE    = 1,
    SAY_SLAY        = 2,
    SAY_DEATH       = 3
};

enum Spells
{
    //Rajh
    SPELL_SUN_STRIKE                = 73872,
    SPELL_INFERNO_JUMP              = 87653,
    SPELL_INFERNO_JUMP_EXPLOSION    = 87647,
    SPELL_INFERNO_JUMP_VISUAL       = 87645,
    SPELL_INFERNO_JUMP_TRIGGER      = 87650,
    SPELL_SUMMON_SOLAR_WIND         = 74104,
    SPELL_SUMMON_SUN_ORB            = 80352,
    SPELL_BLESSING_OF_THE_SUN       = 76352,
    //Solar Wind
    SPELL_FIRE_VORTEX               = 74109,
    SPELL_SOLAR_FIRE_AURA           = 74107,
    SPELL_SUMMON_SOLAR_FIRE         = 89130,
    //Solar Fire Mob
    SPELL_SOLAR_FIRE                = 89131,
    //Sun Orb
    SPELL_SUN                       = 73658,
    SPELL_SUN_SUN_STRIKE            = 73874,
    SPELL_CHARGE                    = 82856,
    //Sun-Touched Servant
    SPELL_DISPERSION                = 88097,
    //Sun-Touched Speaker
    SPELL_STOMP                     = 75241,
    SPELL_FIRE_STORM                = 73861
};

enum Events
{
    // Rajh
    EVENT_SUN_STRIKE            = 1,
    EVENT_INFERNO_JUMP          = 2,
    EVENT_BLESSING_OF_THE_SUN   = 3,
    EVENT_SUMMON_SUN_ORB        = 4,
    EVENT_SUMMON_SOLAR_WIND     = 5,
    EVENT_ENERGIZE              = 6,
    // Inferno Leap
    EVENT_LEAP_EXPLOSION        = 7,
    // Solar Wind
    EVENT_MOVE_ARROUND          = 8,
    EVENT_SUMMON_FIRE           = 9,
    // Sun Orb
    EVENT_MOVE_UP               = 10,
    EVENT_MOVE_DOWN             = 11,
    EVENT_DESPAWN               = 12,
    // Fire Elementars
    EVENT_SUMMON_2              = 13,
    EVENT_SUMMON_3              = 14,
    EVENT_KILL_2                = 15,
    EVENT_FLAME_WAVE            = 16,
    EVENT_SEARING_FLAME         = 17,
    // Sun Touched Speaker
    EVENT_STOMP                 = 18,
    EVENT_FIRE_STORM            = 19
};

enum Points
{
    POINT_CENTER    = 1,
    POINT_UP        = 2,
    POINT_SUN_EXP   = 3
};

Position const CenterPos    = {-319.455f, 193.440f, 343.946f, 3.133f};
Position const SunPoint     = {-317.685f, 192.550f, 379.702f, 0.0f};

class boss_rajh : public CreatureScript
{
public:
    boss_rajh() : CreatureScript("boss_rajh") { }

    struct boss_rajhAI : public BossAI
    {
        boss_rajhAI(Creature* creature) : BossAI(creature, DATA_RAJH)
        {
            Achievement = true;
            Energized   = true;
            me->Respawn(true);
        }

        bool Achievement;
        bool Energized;

        void Reset()
        {
            _Reset();
            me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            me->ModifyPower(POWER_ENERGY, 100);
            Achievement = true;
            Energized   = true;

            std::list<Creature*> creatureList;
            me->GetCreatureListWithEntryInGrid(creatureList, 47922, 100.0f);
            for (std::list<Creature*>::iterator itr = creatureList.begin(); itr != creatureList.end(); ++itr)
                (*itr)->DespawnOrUnsummon();
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            Talk(SAY_AGGRO);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

            events.ScheduleEvent(EVENT_SUN_STRIKE, 20000);       //sniffed
            events.ScheduleEvent(EVENT_INFERNO_JUMP, 26000);     //guessed
            events.ScheduleEvent(EVENT_SUMMON_SUN_ORB, 9800);    //sniffed
            events.ScheduleEvent(EVENT_SUMMON_SOLAR_WIND, 5800); //sniffed
        }

        void JustSummoned(Creature* summon)
        {
            BossAI::JustSummoned(summon);

            if (summon->GetEntry() == NPC_INFERNO_TRIGGER)
            {
                summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
                summon->CastSpell(summon, SPELL_INFERNO_JUMP_VISUAL, true);
                summon->DespawnOrUnsummon(3000);
            }
            else if (summon->GetEntry() == NPC_SOLAR_FIRE)
            {
                summon->CastSpell(summon, 74106, true);
            }
        }

        void SpellHit(Unit* /*caster*/, const SpellInfo* spell)
        {
            Spell* curSpell = me->GetCurrentSpell(CURRENT_GENERIC_SPELL);
            if(curSpell && curSpell->m_spellInfo->Id == SPELL_INFERNO_JUMP)
                for (uint8 i = 0; i < 3; ++i)
                    if(spell && spell->Effects[i].Effect == SPELL_EFFECT_INTERRUPT_CAST)
                    {
                        me->InterruptSpell(CURRENT_GENERIC_SPELL, false);
                        if (Creature* trigger = me->FindNearestCreature(NPC_INFERNO_TRIGGER, 200.0f))
                            trigger->DespawnOrUnsummon();
                    }
        }

        void SpellHitTarget(Unit* /*target*/, SpellInfo const* spellinfo)
        {
            if (spellinfo->Id == SPELL_INFERNO_JUMP)
                DoCast(SPELL_INFERNO_JUMP_EXPLOSION);
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_SLAY);
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type != POINT_MOTION_TYPE)
                return;
            
            switch (pointId)
            {
                case POINT_CENTER:
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                    events.ScheduleEvent(EVENT_ENERGIZE, 1);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (me->GetPower(POWER_ENERGY) <= 10 && Energized)
            {
                Energized = false;
                me->GetMotionMaster()->MovePoint(POINT_CENTER, CenterPos);
                events.Reset();
            }

            if (me->GetPower(POWER_ENERGY) == 100 && !Energized)
            {
                if (Unit* target = me->getVictim())
                {
                    me->GetMotionMaster()->MoveChase(target);
                    me->Attack(target, false);
                }
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                events.ScheduleEvent(EVENT_SUN_STRIKE, 20000);       //sniffed
                events.ScheduleEvent(EVENT_INFERNO_JUMP, 26000);     //guessed
                events.ScheduleEvent(EVENT_SUMMON_SUN_ORB, 9800);    //sniffed
                events.ScheduleEvent(EVENT_SUMMON_SOLAR_WIND, 5800); //sniffed
                Energized   = true;
                Achievement = false;
            }

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_INFERNO_JUMP:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        {
                            DoCast(target, SPELL_INFERNO_JUMP_TRIGGER, false);
                            if (Creature* trigger = me->FindNearestCreature(NPC_INFERNO_TRIGGER, 200.0f))
                                DoCast(trigger, SPELL_INFERNO_JUMP);
                        }
                        events.ScheduleEvent(EVENT_INFERNO_JUMP, 35000);
                        break;
                    case EVENT_SUMMON_SUN_ORB:
                        DoCast(SPELL_SUMMON_SUN_ORB);
                        events.ScheduleEvent(EVENT_SUMMON_SUN_ORB, 35000);  //sniffed
                        break;
                    case EVENT_SUMMON_SOLAR_WIND:
                        DoCast(SPELL_SUMMON_SOLAR_WIND);
                            events.ScheduleEvent(EVENT_SUMMON_SOLAR_WIND, 35000);
                        break;
                    case EVENT_SUN_STRIKE:
                        DoCastVictim(SPELL_SUN_STRIKE);
                        events.ScheduleEvent(EVENT_SUN_STRIKE, 35000);
                        break;
                    case EVENT_ENERGIZE:
                        Talk(SAY_ENERGIZE);
                        if (!Energized)
                            DoCastAOE(SPELL_BLESSING_OF_THE_SUN);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

        void JustDied(Unit* /*who*/)
        {
            _JustDied();
            Talk(SAY_DEATH);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            instance->DoCompleteAchievement(4841);
            if (IsHeroic())
            {
                instance->DoCompleteAchievement(5065);
                if (Achievement)
                    instance->DoCompleteAchievement(5295);
            }
            std::list<Creature*> creatureList;
            me->GetCreatureListWithEntryInGrid(creatureList, 47922, 100.0f);
            for (std::list<Creature*>::iterator itr = creatureList.begin(); itr != creatureList.end(); ++itr)
                (*itr)->DespawnOrUnsummon();

            if (IsHeroic())
                instance->FinishLfgDungeon(me);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_rajhAI(creature);
    }
};

class npc_solar_wind_vortex : public CreatureScript
{
public:
    npc_solar_wind_vortex() : CreatureScript("npc_solar_wind_vortex") {}

    struct npc_solar_wind_vortexAI : public ScriptedAI
    {
        npc_solar_wind_vortexAI(Creature* creature) : ScriptedAI(creature) 
        {
            instance = me->GetInstanceScript();
        }
        
        void JustSummoned(Creature* summon)
        {
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_DISABLE_MOVE);
            summon->AI()->DoCast(SPELL_SOLAR_FIRE);
        }
        
        void IsSummonedBy(Unit* /*summoner*/)
        {
            me->SetDisplayId(me->GetCreatureTemplate()->Modelid1);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC);
            me->SetReactState(REACT_PASSIVE);
            DoCastAOE(SPELL_FIRE_VORTEX);
            DoCastAOE(SPELL_SOLAR_FIRE_AURA);
            events.ScheduleEvent(EVENT_MOVE_ARROUND, 100);
            events.ScheduleEvent(EVENT_SUMMON_FIRE, 3000);
            me->SetSpeed(MOVE_WALK, 0.5f, false);
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveRandom(50.0f);
            if (Creature* rajh = Creature::GetCreature(*me, instance->GetData64(DATA_RAJH_GUID)))
                rajh->AI()->JustSummoned(me);
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_MOVE_ARROUND:
                        me->GetMotionMaster()->MoveRandom(50.0f);
                        events.ScheduleEvent(EVENT_MOVE_ARROUND, 10000);
                        break;
                    default:
                        break;
                }
            }
        }
    private:
        EventMap events;
        InstanceScript* instance;
    };
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_solar_wind_vortexAI(creature);
    }
};

class npc_sunball : public CreatureScript
{
public:
    npc_sunball() : CreatureScript("npc_sunball") {}

    struct npc_sunballAI : public ScriptedAI
    {
        npc_sunballAI(Creature* creature) : ScriptedAI(creature) { }

        void IsSummonedBy(Unit* /*summoner*/)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_NPC);
            DoCast(me, SPELL_SUN);   
            me->SetHover(true);
            events.ScheduleEvent(EVENT_MOVE_DOWN, 5000);
            events.ScheduleEvent(EVENT_DESPAWN, 11000);
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_MOVE_DOWN:
                        // me->SetHover(false);
                        DoCastRandom(SPELL_CHARGE, 0.0f, true);
                        me->DespawnOrUnsummon(10000);
                        break;
                    default:
                        break;
                }
            }
        }
    private:
        EventMap events;
    };
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sunballAI(creature);
    }
};

class npc_elementar_1 : public CreatureScript
{
public:
    npc_elementar_1() : CreatureScript("npc_elementar_1") { }

    struct npc_elementar_1AI : public ScriptedAI
    {
        npc_elementar_1AI(Creature* creature) : ScriptedAI(creature) { }

        void EnterCombat(Unit * /*who*/)
        {
            events.ScheduleEvent(EVENT_SUMMON_3, 15000);
            events.ScheduleEvent(EVENT_FLAME_WAVE, 9000);
            events.ScheduleEvent(EVENT_SEARING_FLAME, 6000);
        }

        void Reset()
        {
            events.Reset();
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (!UpdateVictim())
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SUMMON_3:
                        DoCastAOE(SPELL_DISPERSION);
                        events.ScheduleEvent(EVENT_SUMMON_3, 60000);
                        break;
                    case EVENT_FLAME_WAVE:
                        if (IsHeroic())
                        {
                            DoCastAOE(89852);
                            events.ScheduleEvent(EVENT_FLAME_WAVE, 9000);
                        }
                        else
                        {
                            DoCastAOE(76160);
                            events.ScheduleEvent(EVENT_FLAME_WAVE, 9000);
                        }
                        break;
                    case EVENT_SEARING_FLAME:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                            if(IsHeroic())
                            {
                                DoCast(target, 89849);
                                events.ScheduleEvent(EVENT_SEARING_FLAME, 6000);
                            }
                            else
                            {
                                DoCast(target, 74101);
                                events.ScheduleEvent(EVENT_SEARING_FLAME, 6000);
                            }
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

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_elementar_1AI(creature);
    }
};

class npc_elementar_2 : public CreatureScript
{
public:
    npc_elementar_2() : CreatureScript("npc_elementar_2") { }

    struct npc_elementar_2AI : public ScriptedAI
    {
        npc_elementar_2AI(Creature* creature) : ScriptedAI(creature) { }

        void EnterCombat(Unit * /*who*/)
        {
            events.ScheduleEvent(EVENT_SUMMON_3, 15000);
            events.ScheduleEvent(EVENT_FLAME_WAVE, 9000);
            events.ScheduleEvent(EVENT_SEARING_FLAME, 6000);
            DoCastAOE(76158);
        }

        void Reset()
        {
            events.Reset();
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (!UpdateVictim())
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SUMMON_3:
                        DoCastAOE(88100);
                        events.ScheduleEvent(EVENT_SUMMON_3, 60000);
                        break;
                    case EVENT_FLAME_WAVE:
                        if (IsHeroic())
                        {
                            DoCastAOE(89852);
                            events.ScheduleEvent(EVENT_FLAME_WAVE, 9000);
                        }
                        else
                        {
                            DoCastAOE(76160);
                            events.ScheduleEvent(EVENT_FLAME_WAVE, 9000);
                        }
                        break;
                    case EVENT_SEARING_FLAME:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                            if(IsHeroic())
                            {
                                DoCast(target, 89849);
                                events.ScheduleEvent(EVENT_SEARING_FLAME, 6000);
                            }
                            else
                            {
                                DoCast(target, 74101);
                                events.ScheduleEvent(EVENT_SEARING_FLAME, 6000);
                            }
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

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_elementar_2AI(creature);
    }
};

class npc_suntouched_speaker : public CreatureScript
{
public:
    npc_suntouched_speaker() : CreatureScript("npc_suntouched_speaker") { }

    struct npc_suntouched_speakerAI : public ScriptedAI
    {
        npc_suntouched_speakerAI(Creature* creature) : ScriptedAI(creature) { }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_FIRE_STORM, 10000);
            events.ScheduleEvent(EVENT_STOMP, 6000);
        }

        void Reset()
        {
            events.Reset();
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (!UpdateVictim())
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_FIRE_STORM:
                        DoCastVictim(SPELL_FIRE_STORM);
                        events.ScheduleEvent(SPELL_FIRE_STORM, 15000);
                        break;
                    case EVENT_STOMP:
                        DoCastVictim(SPELL_STOMP);
                        events.ScheduleEvent(EVENT_STOMP, 9000);
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
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_suntouched_speakerAI(creature);
    }
};

void AddSC_boss_rajh()
{
    new boss_rajh();
    new npc_sunball();
    new npc_solar_wind_vortex();
    new npc_elementar_1();
    new npc_elementar_2();
    new npc_suntouched_speaker();
}