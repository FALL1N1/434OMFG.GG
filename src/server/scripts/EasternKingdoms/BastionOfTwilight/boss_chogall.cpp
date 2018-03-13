
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "bastion_of_twilight.h"

enum Yells
{
    TALK_INTRO = 0,
    TALK_AGGRO = 2,
    TALK_CONVERTION = 1,
    TALK_SUMMON_C = 3,
    TALK_PHASE_2 = 4,
    TALK_DARKNESS_CREATION = 5,
    TALK_PLAYER_DIE = 9,
    TALK_RAID_WIPE = 8,
    TALK_DEATH_NM = 6,
    TALK_DEATH_HM = 7,
};

enum Spells
{
    SPELL_CHOGAL_SITTING_ON_THE_THRONE     = 88648,
    SPELL_CHOGAL_VICIATED_BLOOD            = 93104,
    SPELL_CHOGAL_VICIATED_BLOOD_TRIGGER    = 93103,
    SPELL_CHOGAL_BOSS_TAPE                 = 73879,
    SPELL_CHOGAL_ORDER_OF_FLAME            = 81171,
    SPELL_CHOGAL_ORDER_OF_FLAME_TRIGGER    = 81188,
    SPELL_CHOGAL_FIRE_ABSORPTION           = 81196,
    SPELL_CHOGAL_FIRE_ABSORPTION_TRIGGER   = 81194,
    SPELL_CHOGAL_FIRE_DESTRUCTION          = 81528,
    SPELL_CHOGAL_FIRE_DESTRUCTION_2        = 82722,
    SPELL_CHOGAL_FIRE_DESTRUCTION_DAMAGE   = 81527,
    SPELL_CHOGAL_FIRE_DESTRUCTION_TRIGGER  = 81532,
    SPELL_CHOGAL_ORDER_OF_SHADOW           = 81556,
    SPELL_CHOGAL_ORDER_OF_SHADOW_TRIGGER   = 81557,
    SPELL_CHOGAL_SHADOW_ABSORPTION         = 81566,
    SPELL_CHOGAL_SHADOW_OVERPOWER          = 81572,
    SPELL_CHOGAL_SHADOW_DECHAINEES         = 81571,
    SPELL_CHOGAL_FURY                      = 82524,
    SPELL_CHOGAL_EXPLOSION                 = 82518,
    SPELL_CHOGAL_DEFLAGRATION              = 82523,
    SPELL_CHOGAL_SUMMON_CORRUPTING_ADHERENT_SCEFFECT     = 81628,
    SPELL_CHOGAL_SUMMON_CORRUPTING_ADHERENT_MISSILE_LEFT = 81611,
    SPELL_CHOGAL_SUMMON_CORRUPTING_ADHERENT_MISSILE_RIGHT = 81618,
    SPELL_CHOGAL_SUMMON_CORRUPTING_ADHERENT = 82712,
    SPELL_CHOGAL_BLOOD_SUPPURATION         = 82299,
    SPELL_CHOGAL_BLOOD_SUPPURATION_1       = 82337,
    SPELL_CHOGAL_BLOOD_SUPPURATION_2       = 82914,
    SPELL_CHOGAL_VAPOR_CORRUPTION          = 82919,
    SPELL_CHOGAL_CONSUME_OF_THE_OLD_GOD    = 82630,
    SPELL_CHOGAL_CONSUME_OF_THE_OLD_GOD_TRIGGER = 82634,
    SPELL_CHOGAL_CONVERTION                = 91303,
    SPELL_CHOGALL_VENERATION               = 91317,
    SPELL_CHOGALL_LINK_VENERATION          = 92314,
    SPELL_CHOGAL_CORRUPTION_OF_THE_OLD_GOD = 82361,
    SPELL_CHOGAL_CORRUPTION_OF_THE_OLD_GOD_TRIGGER = 82363,
    SPELL_CHOGAL_VISUAL_CORRUPTION         = 82356,
    SPELL_CHOGAL_DARKNESS_CREATION         = 82414,
    SPELL_SUMMON_DARKNESS_CREATION         = 82433,
    SPELL_BERSERK                          = 64238,
    SPELL_FIRE_SHIELD                      = 93276,
    SPELL_FE_ELEMENTAL_GROWTH              = 81215,
    SPELL_ABSORB_FIRE_REVERSE_CAST         = 81233,
    SPELL_FE_RIDE_VEHICLE                  = 43671,
    SPELL_BRASERA_PERIODIC                 = 81536,
    SPELL_BRASERA_PERIODIC_TRIGGER         = 81538,
    SPELL_SHADOW_ORDER_STALKER_VISUAL      = 81559,
    SPELL_SHADOW_ORDER                     = 81558,
    SPELL_SHADOW_SHIELD                    = 93311,
    SPELL_SL_ELEMENTAL_GROWTH              = 81215,
    SPELL_ABSORB_SHADOW_REVERSE_CAST       = 81565,
    SPELL_SL_RIDE_VEHICLE                  = 43671,
    SPELL_TRIGGER_BOSS_TAPE                = 73878,
    SPELL_CORRUPTING_CRASH                 = 81685,
    SPELL_CORRUPTING_CRASH_TRIGGER         = 81689,
    SPELL_DEPRAVATION                      = 81713,
    SPELL_SPILLED_BLOOD_OF_THE_OLD_GOD     = 81771,
    SPELL_SPILLED_BLOOD_OF_THE_OLD_GOD_PERIDOIC = 81757,
    SPELL_SPILLED_BLOOD_OF_THE_OLD_GOD_TRIGGER = 81761,
    SPELL_FESTERED_BLOOD                   = 82333,
    SPELL_FESTER_BLOOD                     = 82337,
    SPELL_SUMMON_VISUAL                    = 82452,
    SPELL_TRANSFORM_EYE_TENTACLE           = 82451,
    SPELL_VOID_ZONE_VISUAL                 = 82397,
    SPELL_DEBILLITATING_RAY                = 82411,
    SPELL_CORRUPTED_BITE                   = 81777,
    SPELL_CORRUPTED_BITE_TRIGGER           = 81775,
    SPELL_FLAMES_ORDER_STALKER_VISUAL      = 81172,
    SPELL_FLAMES_ORDER_SUMMON              = 81186,

    SPELL_HEROIC_TENTACLE_SUMMON_PRE_EFF   = 93315,
    SPELL_HEROIC_TENTACLE_SUMMON           = 93317,
    SPELL_HEROIC_TENTACLE_EMERGE           = 93319
};

enum corruptions_spells
{
    VICIATED_BLOOD                         = 81701,
    CORRUPTION_ACCELEREE                   = 81836,
    CORRUPTION_ILLNESS                     = 81829,
    CORRUPTION_MALFORMATION_SET_VEHICLEID  = 82125,
    CORRUPTION_MALFORMATION_DUMMY          = 82167,
    RIDE_VEH                               = 46598,
    SHADOW_BOLT                            = 82151,
    CORRUPTION_ULTIME_TRANSFORMATION       = 82193,
    CORRUPTION_ULTIME                      = 82170,
};

enum Npc
{
    NPC_FIRE_ELEMENTAL                     = 43406,
    NPC_FIRE_PORTAL                        = 43393,
    NPC_BRASERO                            = 43585,
    NPC_SHADOW_PORTAL                      = 43603,
    NPC_SHADOW_LORD                        = 43592,
    NPC_CORRUPTING_ADHERENT                = 43622,

    NPC_DARKNESS_CREATION                  = 44045,
    NPC_HEROIC_TENTACLE_ANIMATION          = 50265,
    NPC_HEROIC_TENTACLE                    = 50264,

    NPC_BLOOD_OF_THE_OLD_GOD               = 43707,
    NPC_CORRUPTION                         = 43999,
    NPC_MALFORMATION                       = 43888,
};

enum Phases
{
    PHASE_INTRO   = 1,
    PHASE_1,
    PHASE_2
};

enum Events
{
    // Cho'gall

    // phase 1
    EVENT_CONVERSION = 1,
    EVENT_FURIOUS,
    EVENT_ORDER_OF_FIRE,
    EVENT_ORDER_OF_SHADOW,
    EVENT_SUMMON_CORRUPTING_ADHERENT,
    EVENT_FIRE_DESTRUCTION,
    EVENT_SHADOW_DESTRUCTION,
    EVENT_FESTER_BLOOD,

    // phase 2
    EVENT_CORRUPTION_OLD_GOD,
    EVENT_DARKNESS_CREATION,
    EVENT_CHANGE_PHASE,
    EVENT_FIRE_ABSORB,
    EVENT_SHADOW_ABSORB,
    EVENT_HEROIC_TENTACLE,

    // all phases
    EVENT_BERSERK,

    // corrupted adherent
    EVENT_CORRUPTING_CRASH,
    EVENT_DEPRAVATION,
    EVENT_SPILLED_BLOOD,
    EVENT_FESTERED_BLOOD,

    // shadow creation
    EVENT_DEBILITATING_RAY,
    EVENT_TRANSFORM,
};

enum choDatas
{
    DATA_ELEMNTARY_CHARGE = 2,
};

enum eActions
{
    ACTION_HEROIC_DIE = 1,
};

enum ePoints
{
    POINT_DEATH = 42,
};

Position const CenterPosition     = {-1162.0982f, -798.975f, 835.6469f, 0.0f};

class AbsorbEvent : public BasicEvent
{
public:
    AbsorbEvent(Unit* owner, Unit *boss) : _summon(owner), _boss(boss)
    {
    }

    bool Execute(uint64 /*execTime*/, uint32 /*diff*/)
    {
        _summon->RemoveAurasDueToSpell(SPELL_SL_ELEMENTAL_GROWTH);
        _summon->CastSpell(_boss, SPELL_SL_RIDE_VEHICLE, true);
        return true;
    }

private:
    Unit* _summon;
    Unit *_boss;
};

class ElementaryFireEvent : public BasicEvent
{
public:
    ElementaryFireEvent(Unit* owner, Unit *boss) : summon(owner), me(boss)
    {
    }

    bool Execute(uint64 /*execTime*/, uint32 /*diff*/)
    {
        summon->CastSpell(summon, SPELL_FIRE_SHIELD, true);
        summon->CastSpell(summon, SPELL_FE_ELEMENTAL_GROWTH, true);
        summon->m_Events.AddEvent(new AbsorbEvent(summon, me), summon->m_Events.CalculateTime(1400));
        return true;
    }

private:
    Unit* summon;
    Unit *me;
};

class ElementaryShadowEvent : public BasicEvent
{
public:
    ElementaryShadowEvent(Unit* owner, Unit *boss) : summon(owner), me(boss)
    {
    }

    bool Execute(uint64 /*execTime*/, uint32 /*diff*/)
    {
        summon->CastSpell(summon, SPELL_SHADOW_SHIELD, true);
        summon->CastSpell(summon, SPELL_SL_ELEMENTAL_GROWTH, true);
        summon->m_Events.AddEvent(new AbsorbEvent(summon, me), summon->m_Events.CalculateTime(1400));
        return true;
    }

private:
    Unit* summon;
    Unit *me;
};

class boss_chogall : public CreatureScript
{
public:
    boss_chogall() : CreatureScript("boss_chogall") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_chogallAI (creature);
    }

    struct boss_chogallAI: public BossAI
    {
        boss_chogallAI(Creature* creature) : BossAI(creature, DATA_CHOGALL)
        {
        }

        void Reset()
        {
            events.Reset();
            dieEvent = false;
            _elementaryCharge = 0;
            me->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
            phase = PHASE_INTRO;
            if (Creature *corruption = me->FindNearestCreature(NPC_CORRUPTION, 100))
                corruption->RemoveAllAuras();
            me->RemoveAllAuras();
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CHOGAL_VICIATED_BLOOD_TRIGGER);
            instance->DoRemoveAurasDueToSpellOnPlayers(CORRUPTION_ACCELEREE);
            instance->DoRemoveAurasDueToSpellOnPlayers(CORRUPTION_ILLNESS);
            instance->DoRemoveAurasDueToSpellOnPlayers(CORRUPTION_MALFORMATION_SET_VEHICLEID);
            instance->DoRemoveAurasDueToSpellOnPlayers(CORRUPTION_ULTIME_TRANSFORMATION);
            instance->DoRemoveAurasDueToSpellOnPlayers(CORRUPTION_ULTIME);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            DoCast(SPELL_CHOGAL_SITTING_ON_THE_THRONE);
            _Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.Reset();
            Talk(TALK_AGGRO);
            phase = PHASE_1;
            events.SetPhase(PHASE_1);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

            events.ScheduleEvent(EVENT_FURIOUS, 33000, 0, phase);
            events.ScheduleEvent(EVENT_ORDER_OF_FIRE, 5000, 0, PHASE_1);
            events.ScheduleEvent(EVENT_SUMMON_CORRUPTING_ADHERENT, 60000, 0, PHASE_1);
            events.ScheduleEvent(EVENT_CONVERSION, 9000, 0, PHASE_1);

            events.ScheduleEvent(EVENT_BERSERK, 10 * MINUTE * IN_MILLISECONDS);
            DoCast(SPELL_CHOGAL_VICIATED_BLOOD);
            DoCast(SPELL_TRIGGER_BOSS_TAPE);
            _EnterCombat();
        }

        void EnterEvadeMode()
        {
            // if we passed the 3 % Chogall shouldn´t reset anymore
            if (dieEvent)
                return;

            BossAI::EnterEvadeMode();
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(TALK_PLAYER_DIE);
        }

        void JustDied(Unit* /*killer*/)
        {
            instance->SetBossState(DATA_CHOGALL, IsHeroic() ? DONE_HM : DONE);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me); // Remove
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CHOGAL_VICIATED_BLOOD_TRIGGER);
            instance->DoRemoveAurasDueToSpellOnPlayers(CORRUPTION_ACCELEREE);
            instance->DoRemoveAurasDueToSpellOnPlayers(CORRUPTION_ILLNESS);
            instance->DoRemoveAurasDueToSpellOnPlayers(CORRUPTION_MALFORMATION_SET_VEHICLEID);
            instance->DoRemoveAurasDueToSpellOnPlayers(CORRUPTION_ULTIME_TRANSFORMATION);
            instance->DoRemoveAurasDueToSpellOnPlayers(CORRUPTION_ULTIME);
            if (!IsHeroic())
                Talk(TALK_DEATH_NM);
            _JustDied();
            if (IsHeroic())
                me->GetMotionMaster()->MoveFall();
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            summon->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
            switch (summon->GetEntry())
            {
                case NPC_FIRE_PORTAL:
                    summon->CastSpell(summon, SPELL_FLAMES_ORDER_STALKER_VISUAL, false);
                    summon->CastWithDelay(3000, summon, SPELL_FLAMES_ORDER_SUMMON, true);
                    summon->DespawnOrUnsummon(11000);
                    break;
                case NPC_FIRE_ELEMENTAL:
                    summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                    summon->SetReactState(REACT_PASSIVE);
                    events.ScheduleEvent(EVENT_FIRE_ABSORB, RAID_MODE(0, 0, 10000, 10000), 0, PHASE_1);
                    summon->m_Events.AddEvent(new ElementaryFireEvent(summon, me), summon->m_Events.CalculateTime(RAID_MODE(0, 0, 10000, 10000)));
                    break;
                case NPC_SHADOW_PORTAL:
                    summon->CastSpell(summon, SPELL_SHADOW_ORDER_STALKER_VISUAL, false);
                    summon->CastWithDelay(3000, summon, SPELL_SHADOW_ORDER, true);
                    summon->DespawnOrUnsummon(11000);
                    break;
                case NPC_SHADOW_LORD:
                    summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                    summon->SetReactState(REACT_PASSIVE);
                    summon->m_Events.AddEvent(new ElementaryShadowEvent(summon, me), summon->m_Events.CalculateTime(RAID_MODE(0, 0, 10000, 10000)));
                    events.ScheduleEvent(EVENT_SHADOW_ABSORB, RAID_MODE(0, 0, 10000, 10000), 0, PHASE_1);
                    break;
                case NPC_BRASERO:
                    summon->CastSpell(summon, SPELL_BRASERA_PERIODIC, true);
                    break;
                case NPC_CORRUPTING_ADHERENT:
                    summon->CastSpell(summon, SPELL_TRIGGER_BOSS_TAPE, true);
                    break;
                case NPC_HEROIC_TENTACLE_ANIMATION:
                    summon->CastSpell(summon, SPELL_SUMMON_VISUAL, true);
                    break;
            }
            summon->AI()->DoZoneInCombat();
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            if (summon->GetEntry() == NPC_HEROIC_TENTACLE_ANIMATION)
            {
                summon->CastSpell(summon, SPELL_HEROIC_TENTACLE_SUMMON, true);
                summon->CastSpell((Unit*)NULL, SPELL_HEROIC_TENTACLE_EMERGE, true);
            }
        }

        uint32 GetData(uint32 data) const
        {
            if (data == DATA_ELEMNTARY_CHARGE)
                return _elementaryCharge;
            return 0;
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool /*apply*/)
        {
            if (Creature* creature = who->ToCreature())
            {
                switch (who->GetEntry())
                {
                    case NPC_FIRE_ELEMENTAL:
                        _elementaryCharge = (uint32)who->GetHealthPct();
                        events.ScheduleEvent(EVENT_FIRE_DESTRUCTION, 1000);
                        break;
                    case NPC_SHADOW_LORD:
                        _elementaryCharge = (uint32)who->GetHealthPct();
                        events.ScheduleEvent(EVENT_SHADOW_DESTRUCTION, 1000);
                        break;
                }
                creature->DespawnOrUnsummon(100);
            }
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/)
        {
            switch (summon->GetEntry())
            {
                case NPC_FIRE_ELEMENTAL:
                    _elementaryCharge = 10;
                    events.ScheduleEvent(EVENT_FIRE_DESTRUCTION, 1000);
                    break;
                case NPC_SHADOW_LORD:
                    _elementaryCharge = 10;
                    events.ScheduleEvent(EVENT_SHADOW_DESTRUCTION, 1000);
                    break;
                default:
                    break;
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE)
            {
                switch (id)
                {
                    case POINT_DEATH:
                        me->Kill(me);
                        break;
                }
            }
        }

        void DoAction(int32 const act)
        {
            switch (act)
            {
                case ACTION_HEROIC_DIE:
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC);
                    Talk(TALK_DEATH_HM);
                    me->HandleEmoteCommand(417);
                    me->SetSpeed(MOVE_WALK, 1.0f);
                    me->SetSpeed(MOVE_RUN, 1.0f);
                    me->GetMotionMaster()->MovePoint(POINT_DEATH, CenterPosition);
                    break;
                default:
                    break;
            }
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage)
        {
            if (phase == PHASE_1 && me->GetHealthPct() < 21)
            {
                phase = PHASE_2;
                events.SetPhase(PHASE_2);
                events.ScheduleEvent(EVENT_CHANGE_PHASE, 0, 0, PHASE_2);
                Talk(TALK_PHASE_2);
            }

            if (IsHeroic() && me->HealthBelowPct(3))
            {
                if (!dieEvent)
                    DoAction(ACTION_HEROIC_DIE);
                dieEvent = true;
                damage = 0;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CONVERSION:
                    {
                        Talk(TALK_CONVERTION);
                        me->CastCustomSpell(SPELL_CHOGAL_CONVERTION, SPELLVALUE_MAX_TARGETS, 2, me, false);
                        events.ScheduleEvent(EVENT_CONVERSION, 34000, 0, PHASE_1);
                        break;
                    }
                    case EVENT_FURIOUS:
                        DoCastVictim(SPELL_CHOGAL_FURY);
                        events.ScheduleEvent(EVENT_FURIOUS, urand(40000, 50000), 0, phase);
                        break;
                    case EVENT_ORDER_OF_FIRE:
                        DoCast(SPELL_CHOGAL_ORDER_OF_FLAME);
                        events.ScheduleEvent(EVENT_ORDER_OF_SHADOW, urand(20000, 25000), 0, PHASE_1);
                        break;
                    case EVENT_FIRE_DESTRUCTION:
                    {
                        if (IsHeroic())
                        {
                            int8 stacks = round(_elementaryCharge / 10);
                            if (stacks < 1)
                                stacks = 1;
                            me->CastCustomSpell(SPELL_CHOGAL_FIRE_ABSORPTION_TRIGGER, SPELLVALUE_AURA_STACK, stacks, me, TRIGGERED_FULL_MASK);
                        }
                        else
                            me->CastSpell(me, SPELL_CHOGAL_FIRE_ABSORPTION_TRIGGER, true);
                        break;
                    }
                    case EVENT_ORDER_OF_SHADOW:
                        DoCast(SPELL_CHOGAL_ORDER_OF_SHADOW);
                        events.ScheduleEvent(EVENT_ORDER_OF_FIRE, urand(20000, 25000), 0, PHASE_1);
                        break;
                    case EVENT_SHADOW_DESTRUCTION:
                    {
                        if (IsHeroic())
                        {
                            int8 stacks = round(_elementaryCharge / 10);
                            if (stacks < 1)
                                stacks = 1;
                            me->CastCustomSpell(SPELL_CHOGAL_SHADOW_OVERPOWER, SPELLVALUE_AURA_STACK, stacks, me, TRIGGERED_FULL_MASK);
                        }
                        else
                            me->CastSpell(me, SPELL_CHOGAL_SHADOW_OVERPOWER, true);
                        break;
                    }
                    case EVENT_FIRE_ABSORB:
                        DoCast(SPELL_CHOGAL_FIRE_ABSORPTION);
                        break;
                    case EVENT_SHADOW_ABSORB:
                        DoCast(SPELL_CHOGAL_SHADOW_ABSORPTION);
                        break;
                    case EVENT_SUMMON_CORRUPTING_ADHERENT:
                    {
                        Talk(TALK_SUMMON_C);
                        if (Is25ManRaid() && IsHeroic())
                        {
                            me->CastSpell(me, SPELL_CHOGAL_SUMMON_CORRUPTING_ADHERENT_MISSILE_LEFT, false);
                            me->CastSpell(me, SPELL_CHOGAL_SUMMON_CORRUPTING_ADHERENT_MISSILE_RIGHT, false);
                        }
                        else
                        {
                            if (urand(0, 1))
                                me->CastSpell(me, SPELL_CHOGAL_SUMMON_CORRUPTING_ADHERENT_MISSILE_LEFT, false);
                            else
                                me->CastSpell(me, SPELL_CHOGAL_SUMMON_CORRUPTING_ADHERENT_MISSILE_RIGHT, false);
                        }
                        events.ScheduleEvent(EVENT_FESTER_BLOOD, 30000, 0, PHASE_1);
                        events.ScheduleEvent(EVENT_SUMMON_CORRUPTING_ADHERENT, 90000, 0, PHASE_1);
                        break;
                    }
                    case EVENT_FESTER_BLOOD:
                        DoCast(SPELL_CHOGAL_BLOOD_SUPPURATION);
                        break;
                    case EVENT_CHANGE_PHASE:
                        events.Reset();
                        DoCast(SPELL_CHOGAL_CONSUME_OF_THE_OLD_GOD);
                        events.ScheduleEvent(EVENT_CORRUPTION_OLD_GOD, 5000, 0, PHASE_2);
                        events.ScheduleEvent(EVENT_DARKNESS_CREATION, 5300, 0, PHASE_2);
                        if (IsHeroic())
                            events.ScheduleEvent(EVENT_HEROIC_TENTACLE, 5300, 0, PHASE_2);
                        break;
                    case EVENT_CORRUPTION_OLD_GOD:
                        DoCast(SPELL_CHOGAL_CORRUPTION_OF_THE_OLD_GOD);
                        break;
                    case EVENT_DARKNESS_CREATION:
                        Talk(TALK_DARKNESS_CREATION);
                        me->CastCustomSpell(SPELL_CHOGAL_DARKNESS_CREATION, SPELLVALUE_MAX_TARGETS, RAID_MODE(4, 10, 4, 10), me, false);
                        events.ScheduleEvent(EVENT_DARKNESS_CREATION, 30000, 0, PHASE_2);
                        break;
                    case EVENT_HEROIC_TENTACLE:
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 100.0f, true))
                            me->CastCustomSpell(SPELL_HEROIC_TENTACLE_SUMMON_PRE_EFF, SPELLVALUE_MAX_TARGETS, 1, me, false);
                        events.ScheduleEvent(EVENT_HEROIC_TENTACLE, urand(12000, 15000), 0, PHASE_2);
                        break;
                    case EVENT_BERSERK:
                        DoCast(me, SPELL_BERSERK);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private:
        Phases phase;
        uint32 _elementaryCharge;
        bool dieEvent;
    };
};

class npc_chogall_fire_elemental : public CreatureScript
{
public:
    npc_chogall_fire_elemental() : CreatureScript("npc_chogall_fire_elemental") { }

    struct npc_chogall_fire_elementalAI : public ScriptedAI
    {
        npc_chogall_fire_elementalAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage)
        {
            if (me->GetHealth() - damage <= 1000)
                damage = 0;
        }

        void IsSummonedBy(Unit* /*summoner*/)
        {
            if (instance)
            {
                if (Creature* chogall = Creature::GetCreature(*me, instance->GetData64(NPC_CHOGALL)))
                {
                    chogall->AI()->JustSummoned(me);
                    me->ToTempSummon()->SetSummonerGUID(chogall->GetGUID());
                }
            }
        }
    private:
        InstanceScript* instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_chogall_fire_elementalAI(creature);
    }
};

class npc_chogall_shadow_lord : public CreatureScript
{
public:
    npc_chogall_shadow_lord() : CreatureScript("npc_chogall_shadow_lord") { }

    struct npc_chogall_shadow_lordAI : public ScriptedAI
    {
        npc_chogall_shadow_lordAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage)
        {
            if (me->GetHealth() - damage <= 1000)
                damage = 0;
        }

        void IsSummonedBy(Unit* /*summoner*/)
        {
            if (instance)
            {
                if (Creature* chogall = Creature::GetCreature(*me, instance->GetData64(NPC_CHOGALL)))
                {
                    chogall->AI()->JustSummoned(me);
                    me->ToTempSummon()->SetSummonerGUID(chogall->GetGUID());
                }
            }
        }
    private:
        InstanceScript* instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_chogall_shadow_lordAI(creature);
    }
};

class npc_corrupted_adherent: public CreatureScript
{
public:
    npc_corrupted_adherent () : CreatureScript("npc_corrupted_adherent") { }

    struct npc_corrupted_adherentAI: public ScriptedAI
    {
        npc_corrupted_adherentAI (Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            disolve = false;
        }

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_DEPRAVATION, 8000);
            events.ScheduleEvent(EVENT_CORRUPTING_CRASH, 6000);
        }

        void JustDied(Unit* /*killer*/)
        {

        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage) override
        {
            if (damage > me->GetHealth())
            {
                damage = 0;
                if (!disolve)
                {
                    disolve = true;
                    events.Reset();
                    me->CastWithDelay(3000, me, SPELL_SPILLED_BLOOD_OF_THE_OLD_GOD_PERIDOIC, true);
                    DoStopAttack();
                    me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                    me->SetReactState(REACT_PASSIVE);
                    me->GetMotionMaster()->Clear();
                    me->StopMoving();
                }
            }
        }

        void SpellHit(Unit* /*target*/, SpellInfo const* spellInfo) override
        {
            if (spellInfo->Id == SPELL_CHOGAL_BLOOD_SUPPURATION && disolve)
            {
                me->RemoveAurasDueToSpell(SPELL_CHOGAL_BLOOD_SUPPURATION_1);
                me->RemoveAurasDueToSpell(SPELL_CHOGAL_BLOOD_SUPPURATION_2);
                DoCast(SPELL_FESTERED_BLOOD);
            }
            else if (spellInfo->Id == SPELL_CHOGAL_CONSUME_OF_THE_OLD_GOD && !disolve)
                me->DespawnOrUnsummon(5100);
        }

        void JustSummoned(Creature* summon)
        {
            summon->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
            summon->AI()->DoZoneInCombat();
            switch (summon->GetEntry())
            {
                case NPC_BLOOD_OF_THE_OLD_GOD:
                {
                    summon->NearTeleportTo(summon->GetPositionX(), summon->GetPositionY(), me->GetPositionZ() + 1, 0.0f);
                    summon->CastSpell(summon, SPELL_CORRUPTED_BITE, true);
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                    {
                        summon->AddThreat(target, 5000000.0f);
                        summon->AI()->AttackStart(target);
                    }
                    break;
                }
                default:
                    break;
            }
        }

        void UpdateAI (const uint32 diff)
        {
            if (disolve)
            {
                if (me->HasAura(SPELL_CHOGAL_BLOOD_SUPPURATION_1))
                    me->RemoveAurasDueToSpell(SPELL_CHOGAL_BLOOD_SUPPURATION_1);
                if (me->HasAura(SPELL_CHOGAL_BLOOD_SUPPURATION_2))
                    me->RemoveAurasDueToSpell(SPELL_CHOGAL_BLOOD_SUPPURATION_2);
            }

            if (!UpdateVictim() || disolve)
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_CORRUPTING_CRASH:
                        DoCastRandom(SPELL_CORRUPTING_CRASH, 0.0f);
                        events.ScheduleEvent(EVENT_CORRUPTING_CRASH, 6000);
                        break;
                    case EVENT_DEPRAVATION:
                        DoCast(SPELL_DEPRAVATION);
                        events.ScheduleEvent(EVENT_DEPRAVATION, 11000);
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
        bool disolve;
    };


    CreatureAI* GetAI (Creature* creature) const
    {
        return new npc_corrupted_adherentAI(creature);
    }
};

class npc_darkness_creation: public CreatureScript
{
public:
    npc_darkness_creation () : CreatureScript("npc_darkness_creation") { }

    struct npc_darkness_creationAI: public ScriptedAI
    {
        npc_darkness_creationAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void IsSummonedBy(Unit* /*summoner*/)
        {
            if (Creature* chogall = Creature::GetCreature(*me, instance->GetData64(NPC_CHOGALL)))
                chogall->AI()->JustSummoned(me);
        }

        void Reset()
        {
            events.Reset();
            DoCast(SPELL_SUMMON_VISUAL);
            events.ScheduleEvent(EVENT_TRANSFORM, 3000);
            events.ScheduleEvent(EVENT_DEBILITATING_RAY, 5000);
        }

        void UpdateAI (const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_DEBILITATING_RAY:
                        DoCastRandom(SPELL_DEBILLITATING_RAY, 0.0f);
                        events.ScheduleEvent(EVENT_DEBILITATING_RAY, 10000);
                        break;
                    case EVENT_TRANSFORM:
                        DoCast(SPELL_TRANSFORM_EYE_TENTACLE);
                        DoCast(SPELL_VOID_ZONE_VISUAL);
                        break;
                    default:
                        break;
                }
            }

        }
    private:
        InstanceScript* instance;
        EventMap events;
    };


    CreatureAI* GetAI (Creature* creature) const
    {
        return new npc_darkness_creationAI(creature);
    }
};

class CheckControlEvent : public BasicEvent
{
public:
    CheckControlEvent(Player* owner) : _owner(owner) { }

    bool Execute(uint64 execTime, uint32 /*diff*/) override
    {
        uint32 spellId = sSpellMgr->GetSpellIdForDifficulty(SPELL_CHOGALL_VENERATION, _owner);
        if (!_owner->HasAura(spellId))
            _owner->SetClientControl(_owner, 1);
        else
        {
            _owner->m_Events.AddEvent(this, execTime + 2000);
            return false;
        }
        return true;
    }

private:
    Player* _owner;
};

class spell_chogall_convertion : public SpellScriptLoader
{
public:
    spell_chogall_convertion() : SpellScriptLoader("spell_chogall_convertion") { }

    class spell_chogall_convertion_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_chogall_convertion_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (Player* player = GetHitPlayer())
            {
                player->SetClientControl(player, 0);
                player->CastWithDelay(500, player, SPELL_CHOGALL_VENERATION, true);
                player->m_Events.AddEvent(new CheckControlEvent(player), player->m_Events.CalculateTime(2000));
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_chogall_convertion_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_chogall_convertion_SpellScript();
    }
};

class spell_chogall_veneration : public SpellScriptLoader
{
public:
    spell_chogall_veneration() : SpellScriptLoader("spell_chogall_veneration") { }

    class spell_chogall_veneration_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_chogall_veneration_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            return true;
        }

        void Apply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (!GetCaster())
                return;
            if (Player *player = GetCaster()->ToPlayer())
                player->CastSpell(player, SPELL_CHOGALL_LINK_VENERATION, true);
        }

        void Remove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (!GetCaster())
                return;
            if (Player *player = GetCaster()->ToPlayer())
            {
                player->RemoveAurasDueToSpell(SPELL_CHOGALL_LINK_VENERATION);
                player->SetClientControl(player, 1);
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_chogall_veneration_AuraScript::Apply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_chogall_veneration_AuraScript::Remove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_chogall_veneration_AuraScript();
    }
};

class ShadowBoltEvent : public BasicEvent
{
public:
    ShadowBoltEvent(Creature* owner) : _owner(owner) { }

    bool Execute(uint64 execTime, uint32 /*diff*/) override
    {
        if (_owner->isAlive())
        {
            if (!_owner->GetVehicle())
                _owner->SetVisible(false);
            else
            {
                _owner->AI()->DoCastRandom(SHADOW_BOLT, 0.0f);
                _owner->m_Events.AddEvent(this, execTime + 2000);
                return false;
            }
        }
        return true;
    }

private:
    Creature* _owner;
};

class spell_corruption_energize : public SpellScriptLoader
{
public:
    spell_corruption_energize() : SpellScriptLoader("spell_corruption_energize") { }

    class spell_corruption_energize_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_corruption_energize_SpellScript);

        void AddPower(Player *player, int32 nb)
        {
            int32 currPower = player->GetPower(POWER_ALTERNATE_POWER);
            if (currPower < 25 && currPower + nb >= 25)
                player->CastSpell(player, CORRUPTION_ACCELEREE, true);
            else if (currPower < 50  && currPower + nb >= 50)
                player->CastSpell(player, CORRUPTION_ILLNESS, true);
            else if (currPower < 75 && currPower + nb >= 75)
            {
                player->CastSpell(player, CORRUPTION_MALFORMATION_SET_VEHICLEID, true);
                Position pos;
                player->GetPosition(&pos);
                if (Creature *c = player->SummonCreature(NPC_MALFORMATION, pos))
                {
                    c->m_Events.AddEvent(new ShadowBoltEvent(c), c->m_Events.CalculateTime(2000));
                    c->CastSpell(c, CORRUPTION_MALFORMATION_DUMMY, true);
                    c->CastSpell(player, RIDE_VEH, true);
                }
            }
            else if (currPower + nb > 100 && !player->HasAura(CORRUPTION_ULTIME))
            {
                player->CastSpell(player, CORRUPTION_ULTIME_TRANSFORMATION, true);
                player->CastSpell(player, CORRUPTION_ULTIME, true);
            }
            if (currPower + nb > 100)
                player->SetPower(POWER_ALTERNATE_POWER, 100);
            else
                player->SetPower(POWER_ALTERNATE_POWER, nb + currPower);
        }

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (Player* player = GetHitPlayer())
            {
                switch (GetSpellInfo()->Id)
                {
                    // vapor corruption
                    case 82919:
                    case 93108:
                    case 93109:
                    case 93110:
                    // fester blood ?
                    case 81761:
                    case 93172:
                    case 93173:
                    case 93174:
                        AddPower(player, 5);
                        break;
                    // corrupted crash
                    case 81689:
                    case 93184:
                    case 93185:
                    case 93186:
                    // depravation
                    case 81713:
                    case 93175:
                    case 93176:
                    case 93177:
                        AddPower(player, 10);
                        break;
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_corruption_energize_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_corruption_energize_SpellScript();
    }
};

class spell_corrupted_beat : public SpellScriptLoader
{
public:
    spell_corrupted_beat() : SpellScriptLoader("spell_corrupted_beat") { }

    class spell_corrupted_beat_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_corrupted_beat_SpellScript);

        void AddPower(Player *player, int32 nb)
        {
            int32 currPower = player->GetPower(POWER_ALTERNATE_POWER);
            if (currPower < 25 && currPower + nb >= 25)
                player->CastSpell(player, CORRUPTION_ACCELEREE, true);
            else if (currPower < 50  && currPower + nb >= 50)
                player->CastSpell(player, CORRUPTION_ILLNESS, true);
            else if (currPower < 75 && currPower + nb >= 75)
            {
                player->CastSpell(player, CORRUPTION_MALFORMATION_SET_VEHICLEID, true);
                Position pos;
                player->GetPosition(&pos);
                if (Creature *c = player->SummonCreature(NPC_MALFORMATION, pos))
                {
                    c->m_Events.AddEvent(new ShadowBoltEvent(c), c->m_Events.CalculateTime(2000));
                    c->CastSpell(c, CORRUPTION_MALFORMATION_DUMMY, true);
                    c->CastSpell(player, RIDE_VEH, true);
                }
            }
            else if (currPower + nb > 100 && !player->HasAura(CORRUPTION_ULTIME))
            {
                player->CastSpell(player, CORRUPTION_ULTIME_TRANSFORMATION, true);
                player->CastSpell(player, CORRUPTION_ULTIME, true);
            }
            if (currPower + nb > 100)
                player->SetPower(POWER_ALTERNATE_POWER, 100);
            else
                player->SetPower(POWER_ALTERNATE_POWER, nb + currPower);
        }

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (!GetCaster())
                return;
            if (Player* player = GetHitPlayer())
            {
                if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    GetCaster()->ToCreature()->DespawnOrUnsummon(600);
                AddPower(player, 2);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_corrupted_beat_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_corrupted_beat_SpellScript();
    }
};

class spell_debilitating_ray : public SpellScriptLoader
{
public:
    spell_debilitating_ray() : SpellScriptLoader("spell_debilitating_ray") { }

    class spell_debilitating_ray_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_debilitating_ray_AuraScript);

        void AddPower(Player *player, int32 nb)
        {
            int32 currPower = player->GetPower(POWER_ALTERNATE_POWER);
            if (currPower < 25 && currPower + nb >= 25)
                player->CastSpell(player, CORRUPTION_ACCELEREE, true);
            else if (currPower < 50  && currPower + nb >= 50)
                player->CastSpell(player, CORRUPTION_ILLNESS, true);
            else if (currPower < 75 && currPower + nb >= 75)
            {
                player->CastSpell(player, CORRUPTION_MALFORMATION_SET_VEHICLEID, true);
                Position pos;
                player->GetPosition(&pos);
                if (Creature *c = player->SummonCreature(NPC_MALFORMATION, pos))
                {
                    c->m_Events.AddEvent(new ShadowBoltEvent(c), c->m_Events.CalculateTime(2000));
                    c->CastSpell(c, CORRUPTION_MALFORMATION_DUMMY, true);
                    c->CastSpell(player, RIDE_VEH, true);
                }
            }
            else if (currPower + nb > 100 && !player->HasAura(CORRUPTION_ULTIME))
            {
                player->CastSpell(player, CORRUPTION_ULTIME_TRANSFORMATION, true);
                player->CastSpell(player, CORRUPTION_ULTIME, true);
            }
            if (currPower + nb > 100)
                player->SetPower(POWER_ALTERNATE_POWER, 100);
            else
                player->SetPower(POWER_ALTERNATE_POWER, nb + currPower);
        }

        void HandleEffectPeriodicUpdate(AuraEffect* /*aurEff*/)
        {
            if (WorldObject* play = GetOwner())
                if (Player *player = play->ToPlayer())
                {
                    AddPower(player, 2);
                }
        }

        void Register()
        {
            OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_debilitating_ray_AuraScript::HandleEffectPeriodicUpdate, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_debilitating_ray_AuraScript();
    }
};

class spell_corrupted_accelerating : public SpellScriptLoader
{
public:
    spell_corrupted_accelerating() : SpellScriptLoader("spell_corrupted_accelerating") { }

    class spell_corrupted_accelerating_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_corrupted_accelerating_SpellScript);

        void AddPower(Player *player, int32 nb)
        {
            int32 currPower = player->GetPower(POWER_ALTERNATE_POWER);
            if (currPower < 25 && currPower + nb >= 25)
                player->CastSpell(player, CORRUPTION_ACCELEREE, true);
            else if (currPower < 50  && currPower + nb >= 50)
                player->CastSpell(player, CORRUPTION_ILLNESS, true);
            else if (currPower < 75 && currPower + nb >= 75)
            {
                player->CastSpell(player, CORRUPTION_MALFORMATION_SET_VEHICLEID, true);
                Position pos;
                player->GetPosition(&pos);
                if (Creature *c = player->SummonCreature(NPC_MALFORMATION, pos))
                {
                    c->m_Events.AddEvent(new ShadowBoltEvent(c), c->m_Events.CalculateTime(2000));
                    c->CastSpell(c, CORRUPTION_MALFORMATION_DUMMY, true);
                    c->CastSpell(player, RIDE_VEH, true);
                }
            }
            else if (currPower + nb > 100 && !player->HasAura(CORRUPTION_ULTIME))
            {
                player->CastSpell(player, CORRUPTION_ULTIME_TRANSFORMATION, true);
                player->CastSpell(player, CORRUPTION_ULTIME, true);
            }
            if (currPower + nb > 100)
                player->SetPower(POWER_ALTERNATE_POWER, 100);
            else
                player->SetPower(POWER_ALTERNATE_POWER, nb + currPower);
        }

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (!GetCaster())
                return;
            if (Unit* player = GetCaster())
            {
                if (player->ToPlayer())
                    AddPower(player->ToPlayer(), 2);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_corrupted_accelerating_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_corrupted_accelerating_SpellScript();
    }
};

class spell_chogall_illness : public SpellScriptLoader
{
public:
    spell_chogall_illness() : SpellScriptLoader("spell_chogall_illness") { }

    class spell_chogall_illness_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_chogall_illness_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            return true;
        }

        void AddPower(Player *player, int32 nb)
        {
            int32 currPower = player->GetPower(POWER_ALTERNATE_POWER);
            if (currPower < 25 && currPower + nb >= 25)
                player->CastSpell(player, CORRUPTION_ACCELEREE, true);
            else if (currPower < 50  && currPower + nb >= 50)
                player->CastSpell(player, CORRUPTION_ILLNESS, true);
            else if (currPower < 75 && currPower + nb >= 75)
            {
                player->CastSpell(player, CORRUPTION_MALFORMATION_SET_VEHICLEID, true);
                Position pos;
                player->GetPosition(&pos);
                if (Creature *c = player->SummonCreature(NPC_MALFORMATION, pos))
                {
                    c->m_Events.AddEvent(new ShadowBoltEvent(c), c->m_Events.CalculateTime(2000));
                    c->CastSpell(c, CORRUPTION_MALFORMATION_DUMMY, true);
                    c->CastSpell(player, RIDE_VEH, true);
                }
            }
            else if (currPower + nb > 100 && !player->HasAura(CORRUPTION_ULTIME))
            {
                player->CastSpell(player, CORRUPTION_ULTIME_TRANSFORMATION, true);
                player->CastSpell(player, CORRUPTION_ULTIME, true);
            }
            if (currPower + nb > 100)
                player->SetPower(POWER_ALTERNATE_POWER, 100);
            else
                player->SetPower(POWER_ALTERNATE_POWER, nb + currPower);
        }

        void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
        {
            if (Unit *player = GetCaster())
                if (player->ToPlayer())
                    AddPower(player->ToPlayer(), 5);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_chogall_illness_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_chogall_illness_AuraScript();
    }
};

// 81556
class spell_chogall_portals_missile : public SpellScriptLoader
{
public:
    spell_chogall_portals_missile() : SpellScriptLoader("spell_chogall_portals_missile") { }

    class spell_chogall_portals_missile_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_chogall_portals_missile_SpellScript);

        void ModDestHeight(SpellEffIndex /*effIndex*/)
        {
            WorldLocation *offset = GetHitDest();
            const_cast<WorldLocation*>(GetExplTargetDest())->Relocate(offset->GetPositionX(), offset->GetPositionY(), 836.0f);
            GetHitDest()->Relocate(offset->GetPositionX(), offset->GetPositionY(), 836.0f);
        }

        void Register()
        {
            OnEffectLaunch += SpellEffectFn(spell_chogall_portals_missile_SpellScript::ModDestHeight, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_chogall_portals_missile_SpellScript();
    }
};

class spell_chogall_portals_summon : public SpellScriptLoader
{
public:
    spell_chogall_portals_summon() : SpellScriptLoader("spell_chogall_portals_summon") { }

    class spell_chogall_portals_summon_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_chogall_portals_summon_SpellScript);

        void ModDestHeight(SpellEffIndex /*effIndex*/)
        {
            WorldLocation *offset = GetHitDest();
            const_cast<WorldLocation*>(GetExplTargetDest())->Relocate(offset->GetPositionX(), offset->GetPositionY(), 836.0f);
            GetHitDest()->Relocate(offset->GetPositionX(), offset->GetPositionY(), 836.0f);
        }

        void Register()
        {
            OnEffectLaunch += SpellEffectFn(spell_chogall_portals_summon_SpellScript::ModDestHeight, EFFECT_0, SPELL_EFFECT_SUMMON);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_chogall_portals_summon_SpellScript();
    }
};

// 6644
class at_chogall_room : public AreaTriggerScript
{
public:
    at_chogall_room() : AreaTriggerScript("at_chogall_room") { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* /*areaTrigger*/)
    {
        if (InstanceScript* instance = player->GetInstanceScript())
            if (Creature* chogall = Creature::GetCreature(*player, instance->GetData64(NPC_CHOGALL)))
                chogall->AI()->Talk(TALK_INTRO);
        return true;
    }
};

class spell_elementary_damage : public SpellScriptLoader // 93227 93228
{
    class spell_elementary_damage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_elementary_damage_SpellScript);

        void HandleEffect(SpellEffIndex /*effIndex*/)
        {
            if (Unit *caster = GetCaster())
                if (Creature *chogall = caster->ToCreature())
                {
                    int32 damage = GetHitDamage();
                    SetHitDamage(damage + (damage * chogall->AI()->GetData(DATA_ELEMNTARY_CHARGE) / 100));
                }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_elementary_damage_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };
public:
    spell_elementary_damage() : SpellScriptLoader("spell_elementary_damage") { }

    SpellScript* GetSpellScript() const
    {
        return new spell_elementary_damage_SpellScript();
    }
};

// 82363
class spell_chogall_corruption_of_the_ancient_god : public SpellScriptLoader
{
    class spell_chogall_corruption_of_the_ancient_god_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_chogall_corruption_of_the_ancient_god_SpellScript);

        void HandleEffect(SpellEffIndex /*effIndex*/)
        {
            if (Unit *caster = GetCaster())
                if (Unit *target = GetHitUnit())
                {
                    int32 damage = GetHitDamage();
                    int32 currPower = target->GetPower(POWER_ALTERNATE_POWER);
                    SetHitDamage(damage + 0.03 * damage * currPower);
                }
        }

        void AddPower(Player *player, int32 nb)
        {
            int32 currPower = player->GetPower(POWER_ALTERNATE_POWER);
            if (currPower < 25 && currPower + nb >= 25)
                player->CastSpell(player, CORRUPTION_ACCELEREE, true);
            else if (currPower < 50  && currPower + nb >= 50)
                player->CastSpell(player, CORRUPTION_ILLNESS, true);
            else if (currPower < 75 && currPower + nb >= 75)
            {
                player->CastSpell(player, CORRUPTION_MALFORMATION_SET_VEHICLEID, true);
                Position pos;
                player->GetPosition(&pos);
                if (Creature *c = player->SummonCreature(NPC_MALFORMATION, pos))
                {
                    c->m_Events.AddEvent(new ShadowBoltEvent(c), c->m_Events.CalculateTime(2000));
                    c->CastSpell(c, CORRUPTION_MALFORMATION_DUMMY, true);
                    c->CastSpell(player, RIDE_VEH, true);
                }
            }
            else if (currPower + nb > 100 && !player->HasAura(CORRUPTION_ULTIME))
            {
                player->CastSpell(player, CORRUPTION_ULTIME_TRANSFORMATION, true);
                player->CastSpell(player, CORRUPTION_ULTIME, true);
            }
            if (currPower + nb > 100)
                player->SetPower(POWER_ALTERNATE_POWER, 100);
            else
                player->SetPower(POWER_ALTERNATE_POWER, nb + currPower);
        }

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (Player* player = GetHitPlayer())
                AddPower(player, 1);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_chogall_corruption_of_the_ancient_god_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            OnEffectHitTarget += SpellEffectFn(spell_chogall_corruption_of_the_ancient_god_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };
public:
    spell_chogall_corruption_of_the_ancient_god() : SpellScriptLoader("spell_chogall_corruption_of_the_ancient_god") { }

    SpellScript* GetSpellScript() const
    {
        return new spell_chogall_corruption_of_the_ancient_god_SpellScript();
    }
};

class npc_chogall_heroic_tentacl_pre_eff : public CreatureScript
{
public:
    npc_chogall_heroic_tentacl_pre_eff() : CreatureScript("npc_chogall_heroic_tentacl_pre_eff") { }

    struct npc_chogall_heroic_tentacl_pre_effAI : public ScriptedAI
    {
        npc_chogall_heroic_tentacl_pre_effAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void IsSummonedBy(Unit* /*summoner*/) override
        {
            if (instance)
                if (Creature* chogall = Creature::GetCreature(*me, instance->GetData64(NPC_CHOGALL)))
                {
                    chogall->AI()->JustSummoned(me);
                    me->ToTempSummon()->SetSummonerGUID(instance->GetData64(NPC_CHOGALL));
                }
        }
    private:
        InstanceScript* instance;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_chogall_heroic_tentacl_pre_effAI(creature);
    }
};

// All are verified below to DB, creature_template & spell_script_names
void AddSC_boss_chogall()
{
    new boss_chogall();
    new npc_chogall_fire_elemental();
    new npc_chogall_shadow_lord();
    new npc_corrupted_adherent();
    new npc_darkness_creation();
    new spell_chogall_convertion();
    new spell_chogall_veneration();
    new spell_corruption_energize();
    new spell_corrupted_beat();
    new spell_debilitating_ray();
    new spell_corrupted_accelerating();
    new spell_chogall_illness();
    new spell_chogall_portals_summon();
    new spell_chogall_portals_missile();
    new at_chogall_room();
    new spell_elementary_damage();
    new spell_chogall_corruption_of_the_ancient_god();
    new npc_chogall_heroic_tentacl_pre_eff();
}
