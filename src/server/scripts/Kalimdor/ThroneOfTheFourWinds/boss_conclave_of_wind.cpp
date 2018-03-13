
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "throne_of_the_four_winds.h"

enum Spells
{
    // Anshal
    SPELL_SOOTHING_BREEZE           = 86205,
    SPELL_SOOTHING_BREEZE_SUMMON    = 86204,
    SPELL_SOOTHING_BREEZE_VISUAL    = 86208,
    SPELL_SOOTHING_BREEZE_HEAL        = 86206,
    SPELL_SOOTHING_BREEZE_SCILENCE    = 86207,

    SPELL_NURTURE                   = 85422,
    SPELL_NURTURE_DUMMY_AURA        = 85428,
    SPELL_NURTURE_CREEPER_SUMMON    = 85429,

    SPELL_ZEPHYR_ULTIMATE           = 84638,

    SPELL_WITHERING_WIND            = 85576,

    // Nezir
    SPELL_ICE_PATCH                 = 86122,
    SPELL_ICE_PATCH_VISUAL          = 86107,
    SPELL_ICE_PATCH_AURA            = 86111,

    SPELL_PERMAFROST                = 86082,
    SPELL_WIND_CHILL                = 84645,
    SPELL_CHILLING_WINDS            = 85578,

    SPELL_SLEET_STORM_ULTIMATE      = 84644,

    // Rohash
    SPELL_SLICING_GALE              = 86182,

    SPELL_WIND_BLAST                = 86193,
    SPELL_WIND_BLAST_EFFECT         = 85483,

    SPELL_SUMMON_TORNADO            = 86192,
    SPELL_TORNADO_EFFECT            = 86134,
    SPELL_HURRICANE_ULTIMATE        = 84643,
    SPELL_DEAFING_WINDS             = 85573,

    // heroic
    SPELL_TEMPEST_SHIELD            = 93059,

    // Generic
    SPELL_GATHER_STRENGH            = 86307,
    SPELL_NO_MANAREG                = 96301,
    SPELL_SLIPSTREAM_BUFF           = 87740,
    SPELL_TOXIC_SPORES              = 86281,

    // damage special abilities
    SPELL_WITHERING_WIND_DAMAGE = 93168,
    SPELL_CHILLING_WINDS_DAMAGE = 93163,
    SPELL_DEAFING_WINDS_DAMAGE = 93166,
};

#define SPELL_WITHERING_WIND_AURA RAID_MODE(85576, 93181, 93182, 93183)
#define SPELL_CHILLING_WINDS_AURA RAID_MODE(85578, 93147, 93148, 93149)
#define SPELL_DEAFING_WINDS_AURA RAID_MODE(85573, 93190, 93191, 93192)

enum Events
{
    // Anshal
    EVENT_SOOTHING_BREEZE           = 1,
    EVENT_NURTURE,
    EVENT_CREEPER_TOXIC_SPORE,

    // Nezir
    EVENT_ICE_PATCH,
    EVENT_PERMAFROST,
    EVENT_WIND_CHILL,
    EVENT_SLEET_STORM_ULTIMATE,

    // Rohash
    EVENT_SLICING_GALE,
    EVENT_WIND_BLAST,
    EVENT_TORNADO,

    // All
    EVENT_PULSE_AGGRO,
    EVENT_REGENERATE,

};

enum Actions
{
    ACTION_DO_TOXIC_SPORE = 1,
};

class boss_anshal : public CreatureScript
{
public:
    boss_anshal() : CreatureScript("boss_anshal") { }

    struct boss_anshalAI : public ScriptedAI
    {
        boss_anshalAI(Creature* creature) : ScriptedAI(creature), summon(me)
        {
            instance = creature->GetInstanceScript();
            creature->setPowerType(POWER_MANA);
            creature->SetMaxPower(POWER_MANA, 90);
        }

        void Reset()
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_POWER_BURN, true);
            me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, false);
            instance->SetBossState(DATA_CONCLAVE_OF_WIND_EVENT, NOT_STARTED);
            instance->SetData(DATA_DEACTIVATE_SLIPSTREAM, 0);
            me->m_Events.KillAllEvents(true);
            events.Reset();
            summon.DespawnAll();
            DoCast(me, SPELL_NO_MANAREG);
            DoCast(me, SPELL_PRE_COMBAT_EFFECT_ANSHAL, true);
            me->RemoveAurasDueToSpell(SPELL_BERSERK);
            uiRegentimer = 0;
            me->SetPower(POWER_MANA, 0);
            isDead = false;
            poisonCounter = false;
        }

        void EnterCombat(Unit* who)
        {
            DoZoneInCombat();
            me->RemoveAurasDueToSpell(SPELL_PRE_COMBAT_EFFECT_ANSHAL);
            me->SetPower(POWER_MANA, 0);
            instance->SetBossState(DATA_CONCLAVE_OF_WIND_EVENT, IN_PROGRESS);
            instance->SetData(DATA_KILLED_BOSSES, NOT_STARTED);
            uiRegentimer = 1000;
            events.ScheduleEvent(EVENT_SOOTHING_BREEZE, 17000);
            events.ScheduleEvent(EVENT_NURTURE, 33000);
            events.ScheduleEvent(EVENT_PULSE_AGGRO, 1000);
            if (Creature *alakir = Unit::GetCreature((*me), instance->GetData64(DATA_ALAKIR)))
                alakir->AI()->DoAction(ACTION_DISABLE_SLIPSTREAM_TRIGGER);
        }

        void JustSummoned(Creature* summons)
        {
            summon.Summon(summons);

            if (summons->GetEntry() == NPC_SOOTHING_BREEZE)
            {
                summons->CastSpell(summons, SPELL_SOOTHING_BREEZE_VISUAL, true);
                summons->CastSpell(summons, SPELL_SOOTHING_BREEZE_HEAL, true);
                summons->CastSpell(summons, SPELL_SOOTHING_BREEZE_SCILENCE, true);
            }
            else if (summons->GetEntry() == NPC_RAVENOUS_CREEPER_TRIGGER)
            {
                summons->CastSpell(summons, SPELL_NURTURE_DUMMY_AURA, true);
                if (!poisonCounter)
                    events.ScheduleEvent(EVENT_CREEPER_TOXIC_SPORE, 20000);
                poisonCounter = true;
            }
        }

        void DamageTaken(Unit* victim, uint32& damage)
        {
            if (HealthBelowPct(3) && !isDead)
            {
                events.Reset();
                events.ScheduleEvent(EVENT_REGENERATE, IsHeroic() ? 60000 : 120000);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, true);
                DoCast(SPELL_GATHER_STRENGH);
                instance->SetData(DATA_KILLED_BOSSES, instance->GetData(DATA_KILLED_BOSSES) +1);
                isDead = true;
            }
        }

        void JustDied(Unit* killer)
        {
            summon.DespawnAll();
            instance->SetBossState(DATA_CONCLAVE_OF_WIND_EVENT, IsHeroic() ? DONE_HM : DONE);
            instance->SetData(DATA_DEACTIVATE_SLIPSTREAM, 0);
        }

        bool canMove()
        {
            if (Unit *victim = me->getVictim())
            {
                if (victim->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) > 60.0f)
                    return false;
                else
                    return true;
            }
            return false;
        }

        void pulseAggro()
        {
            if (!canMove())
            {
                if (Unit *victim = SelectTarget(SELECT_TARGET_NEAREST, 0, 60.0f, true))
                {
                    if (me->HasReactState(REACT_PASSIVE))
                        me->SetReactState(REACT_AGGRESSIVE);
                    float myThreat = me->getThreatManager().getThreat(victim);
                    float itsThreat = 0.0f;
                    if (HostileReference* hostileRef = me->getThreatManager().getCurrentVictim())
                        itsThreat = hostileRef->getThreat();
                    if (itsThreat > myThreat)
                        me->getThreatManager().addThreat(victim, itsThreat - myThreat);
                    if (!me->getThreatManager().getOnlineContainer().empty())
                        if (HostileReference* forcedVictim = me->getThreatManager().getOnlineContainer().getReferenceByTarget(victim))
                            me->getThreatManager().setCurrentVictim(forcedVictim);
                    me->AI()->AttackStart(victim);
                }
                else
                {
                    Position centerPos = me->GetHomePosition();
                    me->SetReactState(REACT_PASSIVE);
                    me->GetMotionMaster()->MoveIdle();
                    me->GetMotionMaster()->MovePoint(0, centerPos);
                }
            }
            else if (me->HasReactState(REACT_PASSIVE))
            {
                me->SetReactState(REACT_AGGRESSIVE);
                if (Unit *victim = me->getVictim())
                    DoStartMovement(victim);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!instance)
                return;

            if (instance->GetBossState(DATA_CONCLAVE_OF_WIND_EVENT) == IN_PROGRESS)
            {
                if (uiRegentimer <= diff)
                {
                    if (me->GetPower(POWER_MANA) == 90 && !isDead)
                    {
                        Position pos = me->GetHomePosition();
                        me->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), false);
                        DoCast(SPELL_ZEPHYR_ULTIMATE);
                        events.CancelEvent(EVENT_SOOTHING_BREEZE);
                        events.ScheduleEvent(EVENT_SOOTHING_BREEZE, 32500);
                        events.ScheduleEvent(EVENT_NURTURE, 48000);
                        poisonCounter = false;
                        uiRegentimer = 15000;
                    }
                    else if (me->GetPower(POWER_MANA) == 85)
                    {
                        instance->SetData(DATA_DEACTIVATE_SLIPSTREAM, 1);
                        me->SetPower(POWER_MANA, me->GetPower(POWER_MANA)+1);
                        uiRegentimer = 1000;
                    }
                    else if (me->GetPower(POWER_MANA) == 79)
                    {
                        events.CancelEvent(EVENT_CREEPER_TOXIC_SPORE);
                        me->SetPower(POWER_MANA, me->GetPower(POWER_MANA)+1);
                        uiRegentimer = 1000;
                    }
                    else if (me->GetPower(POWER_MANA) == 62)
                    {
                        if (!poisonCounter)
                            events.ScheduleEvent(EVENT_CREEPER_TOXIC_SPORE, 10000);
                        poisonCounter = true;
                        me->SetPower(POWER_MANA, me->GetPower(POWER_MANA)+1);
                        uiRegentimer = 1000;
                    }
                    else if (me->GetPower(POWER_MANA) <= 1)
                    {
                        instance->SetData(DATA_DEACTIVATE_SLIPSTREAM, 0);
                        me->SetPower(POWER_MANA, me->GetPower(POWER_MANA)+1);
                        uiRegentimer = 1000;
                    }
                    else
                    {
                        me->SetPower(POWER_MANA, me->GetPower(POWER_MANA)+1);
                        uiRegentimer = 1000;
                    }
                }
                else uiRegentimer -= diff;

                if (!SelectTarget(SELECT_TARGET_NEAREST, 0, 60.0f, true))
                {
                    if (!me->HasAura(SPELL_WITHERING_WIND_AURA))
                        DoCast(me, SPELL_WITHERING_WIND_AURA, true);

                } else if (me->HasAura(SPELL_WITHERING_WIND_AURA))
                    me->RemoveAura(SPELL_WITHERING_WIND_AURA);

            } else if (me->HasAura(SPELL_WITHERING_WIND_AURA))
                me->RemoveAura(SPELL_WITHERING_WIND_AURA);



            if (!UpdateVictim() && !isDead)
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING) && !isDead)
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SOOTHING_BREEZE:
                    summon.DespawnEntry(NPC_SOOTHING_BREEZE);
                    if (canMove())
                    {
                        me->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), false);
                        DoCast(SPELL_SOOTHING_BREEZE);
                    }
                    events.ScheduleEvent(EVENT_SOOTHING_BREEZE, urand(17000, 19000));
                    break;
                case EVENT_NURTURE:
                    if (canMove())
                    {
                        me->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), false);
                        DoCast(me, SPELL_NURTURE);
                    }
                    break;
                case EVENT_PULSE_AGGRO:
                    pulseAggro();
                    events.ScheduleEvent(EVENT_PULSE_AGGRO, 1000);
                    break;
                case EVENT_REGENERATE:
                    events.ScheduleEvent(EVENT_SOOTHING_BREEZE, 32500);
                    events.ScheduleEvent(EVENT_NURTURE, 35000);
                    events.ScheduleEvent(EVENT_PULSE_AGGRO, 1000);
                    instance->SetData(DATA_KILLED_BOSSES, instance->GetData(DATA_KILLED_BOSSES) -1);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                    me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, false);
                    isDead = false;
                    break;
                case EVENT_CREEPER_TOXIC_SPORE:
                {
                    std::list<Creature *> creepers;
                    me->GetCreatureListWithEntryInGrid(creepers, NPC_RAVENOUS_CREEPER, 100);
                    for (std::list<Creature *>::iterator itr = creepers.begin(); itr != creepers.end(); itr++)
                        if (Creature *creeper = *itr)
                            creeper->AI()->DoAction(ACTION_DO_TOXIC_SPORE);
                    events.ScheduleEvent(EVENT_CREEPER_TOXIC_SPORE, 20000);
                    break;
                }
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }

        private:
            InstanceScript* instance;
            EventMap events;
            uint32 uiRegentimer;
            SummonList summon;
            bool isDead;
            bool poisonCounter;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_anshalAI (creature);
    }
};

class boss_nezir : public CreatureScript
{
public:
    boss_nezir() : CreatureScript("boss_nezir") { }

    struct boss_nezirAI : public ScriptedAI
    {
        boss_nezirAI(Creature* creature) : ScriptedAI(creature), summon(me)
        {
            instance = creature->GetInstanceScript();
            creature->setPowerType(POWER_MANA);
            creature->SetMaxPower(POWER_MANA, 90);
        }

        void Reset()
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_POWER_BURN, true);
            me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, false);
            instance->SetBossState(DATA_CONCLAVE_OF_WIND_EVENT, NOT_STARTED);
            instance->SetData(DATA_DEACTIVATE_SLIPSTREAM, 0);
            me->m_Events.KillAllEvents(true);
            summon.DespawnAll();
            events.Reset();
            isDead = false;
            me->RemoveAurasDueToSpell(SPELL_BERSERK);
            DoCast(me, SPELL_NO_MANAREG);
            DoCast(me, SPELL_PRE_COMBAT_EFFECT_NEZIR, true);
            uiRegentimer = 0;
            me->SetPower(POWER_MANA,0);
        }

        void EnterCombat(Unit* who)
        {
            DoZoneInCombat();
            me->RemoveAurasDueToSpell(SPELL_PRE_COMBAT_EFFECT_NEZIR);
            me->SetPower(POWER_MANA,0);
            instance->SetBossState(DATA_CONCLAVE_OF_WIND_EVENT, IN_PROGRESS);
            uiRegentimer = 1000;

            events.ScheduleEvent(EVENT_ICE_PATCH, urand(10000,12000));
            events.ScheduleEvent(EVENT_PERMAFROST, 6000);
            events.ScheduleEvent(EVENT_WIND_CHILL, 1000);
            events.ScheduleEvent(EVENT_PULSE_AGGRO, 1500);
            if (Creature *alakir = Unit::GetCreature((*me), instance->GetData64(DATA_ALAKIR)))
                alakir->AI()->DoAction(ACTION_DISABLE_SLIPSTREAM_TRIGGER);
        }

        void JustDied(Unit* killer)
        {
            summon.DespawnAll();
            instance->SetBossState(DATA_CONCLAVE_OF_WIND_EVENT, IsHeroic() ? DONE_HM : DONE);
        }

        void JustSummoned(Creature* summoned)
        {
            summon.Summon(summoned);

            if (summoned->GetEntry() == NPC_ICE_PATCH)
                summoned->CastSpell(summoned, SPELL_ICE_PATCH_VISUAL, true);
        }

        void DamageTaken(Unit* victim, uint32& damage)
        {
            if (HealthBelowPct(3) && !isDead)
            {
                events.Reset();
                events.ScheduleEvent(EVENT_REGENERATE, IsHeroic() ? 60000 : 120000);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, true);
                DoCast(SPELL_GATHER_STRENGH);
                instance->SetData(DATA_KILLED_BOSSES, instance->GetData(DATA_KILLED_BOSSES) +1);
                isDead = true;
            }
        }

        bool canMove()
        {
            if (Unit *victim = me->getVictim())
            {
                if (victim->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) > 60.0f)
                    return false;
                else
                    return true;
            }
            return false;
        }

        void pulseAggro()
        {
            if (!canMove())
            {
                if (Unit *victim = SelectTarget(SELECT_TARGET_NEAREST, 0, 60.0f, true))
                {
                    if (me->HasReactState(REACT_PASSIVE))
                        me->SetReactState(REACT_AGGRESSIVE);
                    float myThreat = me->getThreatManager().getThreat(victim);
                    float itsThreat = 0.0f;
                    if (HostileReference* hostileRef = me->getThreatManager().getCurrentVictim())
                        itsThreat = hostileRef->getThreat();
                    if (itsThreat > myThreat)
                        me->getThreatManager().addThreat(victim, itsThreat - myThreat);
                    if (!me->getThreatManager().getOnlineContainer().empty())
                        if (HostileReference* forcedVictim = me->getThreatManager().getOnlineContainer().getReferenceByTarget(victim))
                            me->getThreatManager().setCurrentVictim(forcedVictim);
                    me->AI()->AttackStart(victim);
                }
                else
                {
                    Position centerPos = me->GetHomePosition();
                    me->SetReactState(REACT_PASSIVE);
                    me->GetMotionMaster()->MoveIdle();
                    me->GetMotionMaster()->MovePoint(0, centerPos);
                }
            }
            else if (me->HasReactState(REACT_PASSIVE))
            {
                me->SetReactState(REACT_AGGRESSIVE);
                if (Unit *victim = me->getVictim())
                    DoStartMovement(victim);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!instance)
                return;

            if (instance->GetBossState(DATA_CONCLAVE_OF_WIND_EVENT) == IN_PROGRESS)
            {
                if (uiRegentimer <= diff)
                {
                    if (me->GetPower(POWER_MANA) == 90 && !isDead)
                    {
                        Position pos = me->GetHomePosition();
                        me->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), false);
                        DoCast(SPELL_SLEET_STORM_ULTIMATE);
                        uiRegentimer = 15000;
                    }
                    else if (me->GetPower(POWER_MANA) == 85)
                    {
                        instance->SetData(DATA_DEACTIVATE_SLIPSTREAM, 1);
                        me->SetPower(POWER_MANA, me->GetPower(POWER_MANA)+1);
                        uiRegentimer = 1000;
                    }
                    else if (me->GetPower(POWER_MANA) <= 1)
                    {
                        instance->SetData(DATA_DEACTIVATE_SLIPSTREAM, 0);
                        me->SetPower(POWER_MANA, me->GetPower(POWER_MANA)+1);
                        uiRegentimer = 1000;
                    }
                    else
                    {
                        me->SetPower(POWER_MANA, me->GetPower(POWER_MANA)+1);
                        uiRegentimer = 1000;
                    }
                }
                else uiRegentimer -= diff;

                if (!SelectTarget(SELECT_TARGET_NEAREST, 0, 60.0f, true))
                {
                    if (!me->HasAura(SPELL_CHILLING_WINDS_AURA))
                        DoCast(me, SPELL_CHILLING_WINDS_AURA, true);

                }else if (me->HasAura(SPELL_CHILLING_WINDS_AURA))
                    me->RemoveAura(SPELL_CHILLING_WINDS_AURA);

            } else if (me->HasAura(SPELL_CHILLING_WINDS_AURA))
                me->RemoveAura(SPELL_CHILLING_WINDS_AURA);

            if (!UpdateVictim() && !isDead)
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING) && !isDead)
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ICE_PATCH:
                        if (canMove())
                            DoCastRandom(SPELL_ICE_PATCH, 90.0f, false);
                        events.ScheduleEvent(EVENT_ICE_PATCH, 15000);
                        break;
                    case EVENT_PERMAFROST:
                        if (canMove())
                        {
                            me->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), false);
                            DoCastVictim(SPELL_PERMAFROST);
                        }
                        events.ScheduleEvent(EVENT_PERMAFROST, 10000);
                        break;
                    case EVENT_WIND_CHILL:
                        if (canMove())
                            DoCastAOE(SPELL_WIND_CHILL);
                        events.ScheduleEvent(EVENT_WIND_CHILL, 10500);
                        break;
                    case EVENT_PULSE_AGGRO:
                        pulseAggro();
                        events.ScheduleEvent(EVENT_PULSE_AGGRO, 1000);
                        break;
                    case EVENT_REGENERATE:
                        events.ScheduleEvent(EVENT_ICE_PATCH, urand(10000,12000));
                        events.ScheduleEvent(EVENT_PERMAFROST, urand(20000,23000));
                        events.ScheduleEvent(EVENT_WIND_CHILL, 15000);
                        events.ScheduleEvent(EVENT_PULSE_AGGRO, 1000);
                        instance->SetData(DATA_KILLED_BOSSES, instance->GetData(DATA_KILLED_BOSSES) -1);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                        me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, false);
                        isDead = false;
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

        private:
            InstanceScript* instance;
            EventMap events;
            uint32 uiRegentimer;
            SummonList summon;
            bool isDead;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_nezirAI (creature);
    }
};

class boss_rohash : public CreatureScript
{
public:
    boss_rohash() : CreatureScript("boss_rohash") { }

    struct boss_rohashAI : public ScriptedAI
    {
        boss_rohashAI(Creature* creature) : ScriptedAI(creature), summon(me)
        {
            instance = creature->GetInstanceScript();
            creature->setPowerType(POWER_MANA);
            creature->SetMaxPower(POWER_MANA, 90);
            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
        }

        void Reset()
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_POWER_BURN, true);
            me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, false);
            instance->SetBossState(DATA_CONCLAVE_OF_WIND_EVENT, NOT_STARTED);
            instance->SetData(DATA_DEACTIVATE_SLIPSTREAM, 0);
            me->m_Events.KillAllEvents(true);
            DoCast(me, SPELL_NO_MANAREG);
            DoCast(me, SPELL_PRE_COMBAT_EFFECT_ROHASH, true);
            me->RemoveAurasDueToSpell(SPELL_BERSERK);
            events.Reset();
            summon.DespawnAll();
            isDead = false;
            uiRegentimer = 0;
            me->SetPower(POWER_MANA, 0);
        }

        void EnterCombat(Unit* who)
        {
            DoZoneInCombat();
            me->RemoveAurasDueToSpell(SPELL_PRE_COMBAT_EFFECT_ROHASH);
            me->SetPower(POWER_MANA,0);
            instance->SetBossState(DATA_CONCLAVE_OF_WIND_EVENT, IN_PROGRESS);
            uiRegentimer = 1000;

            events.ScheduleEvent(EVENT_WIND_BLAST, 11500);
            events.ScheduleEvent(EVENT_TORNADO, 7000);
            events.ScheduleEvent(EVENT_SLICING_GALE, 2000);
            events.ScheduleEvent(EVENT_PULSE_AGGRO, 1000);
            if (Creature *alakir = Unit::GetCreature((*me), instance->GetData64(DATA_ALAKIR)))
                alakir->AI()->DoAction(ACTION_DISABLE_SLIPSTREAM_TRIGGER);
        }

        void JustSummoned(Creature* summoned)
        {
            summon.Summon(summoned);
            if (summoned->GetEntry() == 46419)
            {
                summoned->SetDisableGravity(true);
                summoned->SetCanFly(true);
            }

            if (summoned->GetEntry() == NPC_WORLD_TRIGGER_2)
            {
                // summoned->SetCanFly(true);
                summoned->SetMaxHealth(1000000);
                summoned->SetFullHealth();
                summoned->setFaction(14);
                summoned->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }

            if (summoned->GetEntry() == 46207)
            {
                summoned->CastSpell(summoned, SPELL_TORNADO_EFFECT, true);
                summoned->setFaction(14);
                summoned->GetMotionMaster()->MoveRandom(35.0f);
            }
        }

        void JustDied(Unit* victim)
        {
            instance->SetBossState(DATA_CONCLAVE_OF_WIND_EVENT, IsHeroic() ? DONE_HM : DONE);
            summon.DespawnAll();
            instance->CompleteGuildCriteriaForGuildGroup(CRITERIA_GUILD_RUN_CONCLAVE);

            std::list<Creature*> creatureList;
            me->GetCreatureListWithEntryInGrid(creatureList, NPC_SLIPSTREAM_ALAKIR, 1000.0f);
            for (std::list<Creature*>::iterator itr = creatureList.begin();itr != creatureList.end(); ++itr)
                if (Creature* slipstream = (*itr)->ToCreature())
                    slipstream->SetVisible(true);
        }

        void DamageTaken(Unit* victim, uint32& damage)
        {
            if (HealthBelowPct(3) && !isDead)
            {
                events.Reset();
                events.ScheduleEvent(EVENT_REGENERATE, IsHeroic() ? 60000 : 120000);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, true);
                DoCast(SPELL_GATHER_STRENGH);
                instance->SetData(DATA_KILLED_BOSSES, instance->GetData(DATA_KILLED_BOSSES) +1);
                isDead = true;
            }
        }

        bool canMove()
        {
            if (Unit *victim = me->getVictim())
            {
                if (victim->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) > 60.0f)
                    return false;
                else
                    return true;
            }
            return false;
        }

        void pulseAggro()
        {
            if (!canMove())
            {
                if (Unit *victim = SelectTarget(SELECT_TARGET_NEAREST, 0, 60.0f, true))
                {
                    if (me->HasReactState(REACT_PASSIVE))
                        me->SetReactState(REACT_AGGRESSIVE);
                    float myThreat = me->getThreatManager().getThreat(victim);
                    float itsThreat = 0.0f;
                    if (HostileReference* hostileRef = me->getThreatManager().getCurrentVictim())
                        itsThreat = hostileRef->getThreat();
                    if (itsThreat > myThreat)
                        me->getThreatManager().addThreat(victim, itsThreat - myThreat);
                    if (!me->getThreatManager().getOnlineContainer().empty())
                        if (HostileReference* forcedVictim = me->getThreatManager().getOnlineContainer().getReferenceByTarget(victim))
                            me->getThreatManager().setCurrentVictim(forcedVictim);
                    me->AI()->AttackStart(victim);
                }
                else
                {
                    Position centerPos = me->GetHomePosition();
                    me->SetReactState(REACT_PASSIVE);
                    me->GetMotionMaster()->MoveIdle();
                    me->GetMotionMaster()->MovePoint(0, centerPos);
                }
            }
            else if (me->HasReactState(REACT_PASSIVE))
            {
                me->SetReactState(REACT_AGGRESSIVE);
                if (Unit *victim = me->getVictim())
                    DoStartMovement(victim);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!instance)
                return;

            if (instance->GetBossState(DATA_CONCLAVE_OF_WIND_EVENT) == IN_PROGRESS)
            {
                if (uiRegentimer <= diff)
                {
                    if (me->GetPower(POWER_MANA) == 90 && !isDead)
                    {
                        Position pos = me->GetHomePosition();
                        me->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), false);
                        events.CancelEvent(EVENT_WIND_BLAST);
                        events.ScheduleEvent(EVENT_WIND_BLAST, 26500);
                        summon.DespawnEntry(46207);
                        DoCast(SPELL_HURRICANE_ULTIMATE);
                        me->SetPower(POWER_MANA,0);
                        uiRegentimer = 15000;
                    }
                    else if (me->GetPower(POWER_MANA) == 85)
                    {
                        instance->SetData(DATA_DEACTIVATE_SLIPSTREAM, 1);
                        me->SetPower(POWER_MANA, me->GetPower(POWER_MANA)+1);
                        uiRegentimer = 1000;
                    }
                    else if (me->GetPower(POWER_MANA) == 30 && IsHeroic() && !isDead)
                    {
                        me->CastSpell(me, SPELL_TEMPEST_SHIELD, true);
                        me->SetPower(POWER_MANA, me->GetPower(POWER_MANA)+1);
                        uiRegentimer = 1000;
                    }
                    else if (me->GetPower(POWER_MANA) <= 1)
                    {
                        instance->SetData(DATA_DEACTIVATE_SLIPSTREAM, 0);
                        me->SetPower(POWER_MANA, me->GetPower(POWER_MANA)+1);
                        uiRegentimer = 1000;
                    }
                    else
                    {
                        me->SetPower(POWER_MANA, me->GetPower(POWER_MANA)+1);
                        uiRegentimer = 1000;
                    }

                }
                else uiRegentimer -= diff;

                if (!SelectTarget(SELECT_TARGET_NEAREST, 0, 60.0f, true))
                {
                    if (!me->HasAura(SPELL_DEAFING_WINDS_AURA))
                        DoCast(me, SPELL_DEAFING_WINDS_AURA, true);

                } else if (me->HasAura(SPELL_DEAFING_WINDS_AURA))
                    me->RemoveAura(SPELL_DEAFING_WINDS_AURA);

            } else if (me->HasAura(SPELL_DEAFING_WINDS_AURA))
                me->RemoveAura(SPELL_DEAFING_WINDS_AURA);

            if (!UpdateVictim() && !isDead)
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING) && !isDead)
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_WIND_BLAST:
                    if (canMove())
                        if (Creature* trigger = DoSummon(NPC_WORLD_TRIGGER_2, me, 20.0f, 12000, TEMPSUMMON_TIMED_DESPAWN))
                        {
                            float x, y;
                            me->GetNearPoint2D(x, y, 25, me->GetAngle(trigger));
                            trigger->NearTeleportTo(x, y, me->GetPositionZ(), trigger->GetOrientation());
                            DoCast(trigger, SPELL_WIND_BLAST);
                        }
                    events.ScheduleEvent(EVENT_WIND_BLAST, 60000);
                    break;
                case EVENT_SLICING_GALE:
                    if (canMove())
                        DoCastRandom(SPELL_SLICING_GALE, 90.0f, false);
                    events.ScheduleEvent(EVENT_SLICING_GALE, 2000);
                    break;
                case EVENT_TORNADO:
                    summon.DespawnEntry(NPC_TORNADO);
                    if (canMove())
                        DoCast(SPELL_SUMMON_TORNADO);
                    events.ScheduleEvent(EVENT_TORNADO, 45000);
                    break;
                case EVENT_PULSE_AGGRO:
                    pulseAggro();
                    events.ScheduleEvent(EVENT_PULSE_AGGRO, 1000);
                    break;
                case EVENT_REGENERATE:
                    events.ScheduleEvent(EVENT_WIND_BLAST, 5000);
                    events.ScheduleEvent(EVENT_TORNADO, 7000);
                    events.ScheduleEvent(EVENT_SLICING_GALE, 2000);
                    events.ScheduleEvent(EVENT_PULSE_AGGRO, 1000);
                    instance->SetData(DATA_KILLED_BOSSES, instance->GetData(DATA_KILLED_BOSSES) -1);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                    me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, false);
                    isDead = false;
                    break;
                default:
                    break;
                }
            }
        }

        private:
            InstanceScript* instance;
            EventMap events;
            uint32 uiRegentimer;
            SummonList summon;
            bool isDead;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_rohashAI (creature);
    }
};

class npc_anshal_creeper : public CreatureScript
{
public:
    npc_anshal_creeper() : CreatureScript("npc_anshal_creeper") { }

    struct npc_anshal_creeperAI : public ScriptedAI
    {
        npc_anshal_creeperAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
            toxicSporesTimer = 0;
        }

        void IsSummonedBy(Unit* /*summoner*/)
        {
            if (Creature* anshal = sObjectAccessor->GetCreature(*me, instance->GetData64(DATA_ANSHAL)))
                anshal->AI()->JustSummoned(me);
            me->SetInCombatWithZone();
            toxicSporesTimer = 10000;
            // should be at the same time
            uiMovementTimer = 1000;
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_DO_TOXIC_SPORE:
                    me->CastSpell(me, SPELL_TOXIC_SPORES, true);
                    break;
            }
        }

        bool canMove()
        {
            if (Unit *victim = me->getVictim())
            {
                if (Creature* anshal = Creature::GetCreature(*me, instance->GetData64(DATA_ANSHAL)))
                    if (victim->GetDistance2d(anshal->GetHomePosition().GetPositionX(), anshal->GetHomePosition().GetPositionY()) > 65.0f)
                        return false;
                    else
                        return true;
            }
            return false;
        }

        void pulseAggro()
        {
            if (!canMove())
            {
                if (Creature* westwind = me->FindNearestCreature(47925, 200.0f, true)) // we should use the westwind trigger for the selection because the trigger is always on center position
                {
                    if (Unit *victim = westwind->AI()->SelectTarget(SELECT_TARGET_NEAREST, 0, 60.0f, true))
                    {
                        if (me->HasReactState(REACT_PASSIVE))
                            me->SetReactState(REACT_AGGRESSIVE);
                        float myThreat = me->getThreatManager().getThreat(victim);
                        float itsThreat = 0.0f;
                        if (HostileReference* hostileRef = me->getThreatManager().getCurrentVictim())
                            itsThreat = hostileRef->getThreat();
                        if (itsThreat > myThreat)
                            me->getThreatManager().addThreat(victim, itsThreat - myThreat);
                        if (!me->getThreatManager().getOnlineContainer().empty())
                            if (HostileReference* forcedVictim = me->getThreatManager().getOnlineContainer().getReferenceByTarget(victim))
                                me->getThreatManager().setCurrentVictim(forcedVictim);
                        me->AI()->AttackStart(victim);
                    }
                    else
                    {
                        Position centerPos = me->GetHomePosition();
                        me->SetReactState(REACT_PASSIVE);
                        me->GetMotionMaster()->MoveIdle();
                        me->GetMotionMaster()->MovePoint(0, centerPos);
                    }
                }
                else
                {
                    Position centerPos = me->GetHomePosition();
                    me->SetReactState(REACT_PASSIVE);
                    me->GetMotionMaster()->MoveIdle();
                    me->GetMotionMaster()->MovePoint(0, centerPos);
                }
            }
            else if (me->HasReactState(REACT_PASSIVE))
            {
                me->SetReactState(REACT_AGGRESSIVE);
                if (Unit *victim = me->getVictim())
                    DoStartMovement(victim);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (uiMovementTimer <= diff)
            {
                pulseAggro();
                uiMovementTimer = 1000;
            }
            else uiMovementTimer -= diff;

            DoMeleeAttackIfReady();
        }

    private:
        InstanceScript* instance;
        uint32 toxicSporesTimer;
        uint32 uiMovementTimer;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_anshal_creeperAI (creature);
    }
};

class spell_nezir_perma_frost : public SpellScriptLoader
{
public:
    spell_nezir_perma_frost() : SpellScriptLoader("spell_nezir_perma_frost") { }

    class spell_nezir_perma_frost_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nezir_perma_frost_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.clear();
            if (Unit *caster = GetCaster())
                if (InstanceScript* instance = caster->GetInstanceScript())
                    if (Creature* nezir = Creature::GetCreature(*caster, instance->GetData64(DATA_NEZIR)))
                    {
                        ThreatContainer::StorageType const &threatList = nezir->getThreatManager().getThreatList();
                        for (ThreatContainer::StorageType::const_iterator itr = threatList.begin(); itr != threatList.end(); ++itr)
                            if (Unit* target = (*itr)->getTarget())
                                if (nezir->isInFrontInMap(target, 90.0f, 100.0f * M_PI / 180.0f))
                                    targets.push_back(target);
                    }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_nezir_perma_frost_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_104);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_nezir_perma_frost_SpellScript();
    }
};

class spell_anshal_nurture : public SpellScriptLoader
{
public:
    spell_anshal_nurture() : SpellScriptLoader("spell_anshal_nurture") { }

    class spell_anshal_nurture_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_anshal_nurture_SpellScript);

        void HandleDummy()
        {
            if (Creature* caster = GetCaster()->ToCreature())
            {
                caster->CastWithDelay(8000, caster, SPELL_NURTURE_CREEPER_SUMMON, true);
                caster->DespawnOrUnsummon(10000);
            }
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_anshal_nurture_SpellScript::HandleDummy);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_anshal_nurture_SpellScript();
    }
};

class at_slipstream_jump : public AreaTriggerScript
{
public:
    at_slipstream_jump() : AreaTriggerScript("at_slipstream_jump") { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* areaTrigger)
    {
        if (InstanceScript* instance = player->GetInstanceScript())
            if (instance->GetData(DATA_DEACTIVATE_SLIPSTREAM))
                return false;

        player->CastSpell(player, SPELL_SLIPSTREAM_BUFF, true);
        switch (areaTrigger->id)
        {
            case 6274:
                player->GetMotionMaster()->MoveJump(-80.842f, 1020.651f, 199.492f, 40.0f, 40.0f);
                break;
            case 6275:
                player->GetMotionMaster()->MoveJump(-255.214f, 849.186f, 199.491f, 40.0f, 40.0f);
                break;
            case 6276:
                player->GetMotionMaster()->MoveJump(155.783f, 846.531f, 199.491f, 40.0f, 40.0f);
                break;
            case 6277:
                player->GetMotionMaster()->MoveJump(-15.109f, 1020.446f, 199.492f, 40.0f, 40.0f);
                break;
            case 6278:
                player->GetMotionMaster()->MoveJump(-18.136f, 609.507f, 199.492f, 40.0f, 40.0f);
                break;
            case 6279:
                player->GetMotionMaster()->MoveJump(155.898f, 780.698f, 199.492f, 40.0f, 40.0f);
                break;
            case 6280:
                player->GetMotionMaster()->MoveJump(-255.230f, 783.763f, 199.492f, 40.0f, 40.0f);
                break;
            case 6290:
                player->GetMotionMaster()->MoveJump(-83.918f, 609.413f, 199.491f, 40.0f, 40.0f);
                break;
            default:
                break;
        }
        return true;
    }
};

class spell_tfw_correct_channel_eff1 : public SpellScriptLoader
{
public:
    spell_tfw_correct_channel_eff1() : SpellScriptLoader("spell_tfw_correct_channel_eff1") { }

    class spell_tfw_correct_channel_eff1_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_tfw_correct_channel_eff1_AuraScript);

        void onPeriodicTick(AuraEffect const* aurEff)
        {
            if (Unit* caster = GetTarget())
            {
                if (caster->GetTypeId() != TYPEID_UNIT || caster->getPowerType() != POWER_MANA)
                    return;

                caster->ModifyPowerPct(POWER_MANA, -6.0f, true);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_tfw_correct_channel_eff1_AuraScript::onPeriodicTick, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_tfw_correct_channel_eff1_AuraScript();
    }
};

class spell_tfw_correct_channel_eff0 : public SpellScriptLoader
{
public:
    spell_tfw_correct_channel_eff0() : SpellScriptLoader("spell_tfw_correct_channel_eff0") { }

    class spell_tfw_correct_channel_eff0_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_tfw_correct_channel_eff0_AuraScript);

        void onPeriodicTick(AuraEffect const* aurEff)
        {
            if (Unit* caster = GetTarget())
            {
                if (caster->GetTypeId() != TYPEID_UNIT || caster->getPowerType() != POWER_MANA)
                    return;

                caster->ModifyPowerPct(POWER_MANA, -6.0f, true);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_tfw_correct_channel_eff0_AuraScript::onPeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_tfw_correct_channel_eff0_AuraScript();
    }
};

class spell_rohash_windblast : public SpellScriptLoader
{
public:
    spell_rohash_windblast() : SpellScriptLoader("spell_rohash_windblast") { }

    class spell_rohash_windblast_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_rohash_windblast_AuraScript);

        bool Load()
        {
            start = false;
            speed = urand(0, 1) == 0 ? 0.2f : -0.2f; // if the speed rotation is too low just change this.
            return true;
        }

        void CalcPeriodic(AuraEffect const* /*effect*/, bool& isPeriodic, int32& amplitude)
        {
            isPeriodic = true;
            amplitude = 500;
        }

        void OnPeriodicTick(AuraEffect const* /*aurEff*/)
        {
            if (Unit* cas = GetCaster())
                if (Creature* caster = cas->ToCreature())
                    if (Unit* target = GetTarget())
                    {
                        float x, y;
                        if (!start)
                        {
                            angle = caster->GetAngle(target);
                            start = true;
                        }
                        else
                            angle += speed;
                        caster->GetNearPoint2D(x, y, 25, angle);
                        target->GetMotionMaster()->Clear();
                        target->GetMotionMaster()->MovePoint(0, x, y, target->GetPositionZ());
                    }
        }

        void Register()
        {
            DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_rohash_windblast_AuraScript::CalcPeriodic, EFFECT_0, SPELL_AURA_DUMMY);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_rohash_windblast_AuraScript::OnPeriodicTick, EFFECT_0, SPELL_AURA_DUMMY);
        }

    private:
        bool start;
        float angle;
        float speed;
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_rohash_windblast_AuraScript();
    }
};

class spell_rohash_hurricane : public SpellScriptLoader
{
public:
    spell_rohash_hurricane() : SpellScriptLoader("spell_rohash_hurricane") { }

    class spell_rohash_hurricane_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_rohash_hurricane_SpellScript);

        void HandleDummy(SpellEffIndex effIndex)
        {
            if (Unit* target = GetHitUnit())
                if (Creature* vehicle = GetCaster()->SummonCreature(46419, -51.4478f, 576.8439f, 250.6340f, 2.3387f, TEMPSUMMON_TIMED_DESPAWN, 30000))
                    target->CastSpell(vehicle, 86481, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_rohash_hurricane_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_rohash_hurricane_SpellScript();
    }
};

class spell_rohash_hurricane_cs_veh : public SpellScriptLoader
{
public:
    spell_rohash_hurricane_cs_veh() : SpellScriptLoader("spell_rohash_hurricane_cs_veh") { }

    class spell_rohash_hurricane_cs_veh_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_rohash_hurricane_cs_veh_AuraScript);

        void onPeriodicTick(AuraEffect const* aurEff)
        {
            PreventDefaultAction();
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_rohash_hurricane_cs_veh_AuraScript::onPeriodicTick, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_rohash_hurricane_cs_veh_AuraScript();
    }
};

class npc_rohash_trigger : public CreatureScript
{
public:
    npc_rohash_trigger() : CreatureScript("npc_rohash_trigger") { }

    struct npc_rohash_triggerAI : public PassiveAI
    {
        npc_rohash_triggerAI(Creature* creature) : PassiveAI(creature) { }

        void PassengerBoarded(Unit* who, int8 seatId, bool apply)
        {
            if (apply)
                if (Player* player = who->ToPlayer())
                {
                    player->AddUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
                    player->SendMovementDisableGravity();
                }

            if (!apply)
                if (Player* player = who->ToPlayer())
                {
                    player->RemoveUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
                    player->SendMovementDisableGravity();
                }
        }

        void Reset()
        {
            moveTimer = 100;
        }

        void UpdateAI(uint32 const diff)
        {
            if (moveTimer <= diff)
            {
                me->SetFacingTo(me->GetOrientation() + 0.1f);
                moveTimer = 100;
            }
            else moveTimer -= diff;
        }

    private:
        uint32 moveTimer;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rohash_triggerAI (creature);
    }
};

class MoveSecondEvent : public BasicEvent
{
public:
    MoveSecondEvent(Unit* player) : _player(player)
    {
    }

    bool Execute(uint64 execTime, uint32 /*diff*/)
    {
        if (Creature* landing = _player->FindNearestCreature(45504, 500.0f, true))
            _player->GetMotionMaster()->MoveJump(landing->GetPositionX(), landing->GetPositionY(), landing->GetPositionZ(), 45.0f, 45.0f, 0);
        return false;
    }

private:
    Unit *_player;
};

class MoveFirstEvent : public BasicEvent
{
public:
    MoveFirstEvent(Unit* player) : _player(player)
    {
    }

    bool Execute(uint64 execTime, uint32 /*diff*/)
    {
        _player->GetMotionMaster()->MoveJump(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ() + 153.0f, 35.0f, 35.0f, 0);
        _player->m_Events.AddEvent(new MoveSecondEvent(_player), execTime + 3000);
        return false;
    }

private:
    Unit *_player;
};

class npc_slipstream_alakir : public CreatureScript
{
public:
    npc_slipstream_alakir() : CreatureScript("npc_slipstream_alakir") { }

    struct npc_slipstream_alakirAI : public PassiveAI
    {
        npc_slipstream_alakirAI(Creature* creature) : PassiveAI(creature)
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }

        void PassengerBoarded(Unit* who, int8 seatId, bool apply)
        {
            if (apply)
                who->CastSpell(who, SPELL_SLIPSTREAM_BUFF, true);

            if (!apply)
                who->m_Events.AddEvent(new MoveFirstEvent(who), who->m_Events.CalculateTime(0));
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_slipstream_alakirAI (creature);
    }
};

class spell_tornado_vehicle : public SpellScriptLoader
{
public:
    spell_tornado_vehicle() : SpellScriptLoader("spell_tornado_vehicle") { }

    class spell_tornado_vehicle_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_tornado_vehicle_SpellScript);

        void prevent(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
        }

        void Register()
        {
            OnEffectHit += SpellEffectFn(spell_tornado_vehicle_SpellScript::prevent, EFFECT_1, SPELL_EFFECT_APPLY_AURA);
            OnEffectHit += SpellEffectFn(spell_tornado_vehicle_SpellScript::prevent, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_tornado_vehicle_SpellScript();
    }
};

class spell_anshal_wing_dummy : public SpellScriptLoader
{
public:
    spell_anshal_wing_dummy() : SpellScriptLoader("spell_anshal_wing_dummy") { }

    class spell_anshal_wing_dummy_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_anshal_wing_dummy_AuraScript);

        void HandleDummy(AuraEffect const* aurEff)
        {
            if (Unit *caster = GetCaster())
            {
                uint32 tick = aurEff->GetTickNumber();
                int32 damage = 2000 + 200 * tick;
                caster->CastCustomSpell(caster, SPELL_WITHERING_WIND_DAMAGE, &damage, NULL, NULL, true);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_anshal_wing_dummy_AuraScript::HandleDummy, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_anshal_wing_dummy_AuraScript();
    }
};

class spell_nezir_wing_dummy : public SpellScriptLoader
{
public:
    spell_nezir_wing_dummy() : SpellScriptLoader("spell_nezir_wing_dummy") { }

    class spell_nezir_wing_dummy_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_nezir_wing_dummy_AuraScript);

        void HandleDummy(AuraEffect const* aurEff)
        {
            if (Unit *caster = GetCaster())
            {
                uint32 tick = aurEff->GetTickNumber();
                int32 damage = 2000 + 200 * tick;
                caster->CastCustomSpell(caster, SPELL_CHILLING_WINDS_DAMAGE, &damage, NULL, NULL, true);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_nezir_wing_dummy_AuraScript::HandleDummy, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_nezir_wing_dummy_AuraScript();
    }
};

class spell_rohash_wing_dummy : public SpellScriptLoader
{
public:
    spell_rohash_wing_dummy() : SpellScriptLoader("spell_rohash_wing_dummy") { }

    class spell_rohash_wing_dummy_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_rohash_wing_dummy_AuraScript);

        void HandleDummy(AuraEffect const* aurEff)
        {
            if (Unit *caster = GetCaster())
            {
                uint32 tick = aurEff->GetTickNumber();
                int32 damage = 2000 + 200 * tick;
                caster->CastCustomSpell(caster, SPELL_DEAFING_WINDS_DAMAGE, &damage, NULL, NULL, true);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_rohash_wing_dummy_AuraScript::HandleDummy, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_rohash_wing_dummy_AuraScript();
    }
};

void AddSC_boss_conclave_of_wind()
{
    new boss_anshal();
    new boss_nezir();
    new boss_rohash();
    new npc_anshal_creeper();
    new spell_nezir_perma_frost();
    new spell_anshal_nurture();
    new at_slipstream_jump();
    new spell_tfw_correct_channel_eff1();
    new spell_tfw_correct_channel_eff0();
    new spell_rohash_windblast();
    new spell_rohash_hurricane();
    new npc_rohash_trigger();
    new npc_slipstream_alakir();
    new spell_tornado_vehicle();
    new spell_anshal_wing_dummy();
    new spell_nezir_wing_dummy();
    new spell_rohash_wing_dummy();
    new spell_rohash_hurricane_cs_veh();
}
