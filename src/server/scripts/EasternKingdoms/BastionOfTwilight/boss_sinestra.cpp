
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"
#include "bastion_of_twilight.h"

enum SinestraYells
{
    SAY_INTRO               = 0,
    SAY_SUMMON              = 1,
    SAY_SPECIAL_1           = 2,
    SAY_SPECIAL_2           = 3,
    SAY_SPECIAL_3           = 4,
    SAY_PHASE_2_START       = 5,
    SAY_PHASE_2_BEAM_START  = 6,
    SAY_WINNING             = 7,
    SAY_LOSING              = 8,
    SAY_PHASE_3             = 9,
    SAY_SLAY_1              = 10,
    SAY_SLAY_2              = 11,
    SAY_DEATH               = 12
};

enum CalenYells
{
    SAY_CALEN_SUMMON        = 0,
    SAY_CALEN_BATTLE_START  = 1,
    SAY_CALEN_RECHARGE      = 2,
    SAY_CALEN_WINNING       = 3,
    SAY_CALEN_LOSING        = 4,
    SAY_CALEN_DIE           = 5,
    SAY_CALEN_PHASE_3       = 6
};

#define EMOTE_WHELP_REVIVE 0

enum Spells
{
    // sinestra
    SPELL_HITTIN_YA                   = 94132, // tank exclusion
    SPELL_HITTIN_YA_PLAYER            = 94131, // tank exclusion
    SPELL_REBIRTH                     = 45464, // LoS intro
    SPELL_DRAINED                     = 89350,
    SPELL_CALL_FLAMES                 = 95855,
    SPELL_TWILIGHT_BLAST              = 89280,
    SPELL_TWILIGHT_EXTINCTION         = 86227, // phase 2 start
    SPELL_FLAME_BREATH                = 90125,
    SPELL_TWILIGHT_POWER              = 87220, // phase 2 beam
    SPELL_SUMMON_TWILIGHT_DRAGON      = 92142, // phase 2
    SPELL_WRACK_INITIAL               = 89421, // Sinestra causes a random player to suffer Shadow damage every 2 sec. for 1 min. with the damage taken increasing rapidly. When a player dispels Wrack, it bounces to two nearby allies of the afflicted player. Wrack retains its remaining duration but returns to its original damage values.
    SPELL_WRACK_JUMP                  = 89435,
    SPELL_MANA_BARRIER                = 87299, // phase 2 - A powerful barrier envelops Sinestra, continually replenishing any lost health at the expense of her mana.
    SPELL_BERSERK                     = 26662, // meh meh players are to bad

    // Calen - phase 2 event
    SPELL_FIERY_BARRIER_INITIAL       = 87229,
    SPELL_FIERY_BARIER_VISUAL         = 95791,
    SPELL_FIERY_BARRIER_GROUND_EFFECT = 96431,
    SPELL_ESSENCE_OF_THE_RED          = 87946, // Description: The blessing of Alexstrasza is applied to all allies nearby. - p3 start
    SPELL_FIERY_RESOLVE               = 87221, // phase 2 beam
    SPELL_PYRRHIC_FOCUS               = 87323, // Calen burns his own health reserves to restore his mana. His focus also increases healing done to Calen by 500%.

    // twilight orb
    SPELL_TWILIGHT_PULSE_PERIODIC     = 92957,
    SPELL_TWILIGHT_SLICER_BEAM        = 92851,
    SPELL_TWILIGHT_BEAM               = 95564,

    // twilight whelps
    SPELL_TWILIGHT_SPIT               = 89299,
    SPELL_TWILIGHT_ESSENCE            = 89284, // death aura
    SPELL_TWILIGHT_ESSENCE_INCREASE   = 89288,
    SPELL_FEIGN_DEATH                 = 29266,

    // twilight egg
    SPELL_TWILIGHT_CARAPACE           = 87654, // damage absorb shield
    SPELL_TWILIGHT_FLAME              = 95823,

    // twilight spitecaller
    SPELL_UNLASH_ESSENCE              = 90028, // Releases a torrent of twilight energy that afflicts all nearby enemies, dealing damage equal to $s1% of their maximum health every $t1 sec.
    SPELL_INDOMITABLE                 = 90045, // Regains control through a surge of sheer will, becoming immune to all bonds and restraints until stopped.  Deals $s2 Shadow damage to enemies within $A2 yards and knocks them back.
    SPELL_INDOMITABLE_VISUAL          = 90044,
    // twilight drake
    SPELL_TWILIGHT_BREATH             = 76817,
    SPELL_ABSORB_ESSENCE              = 90107 // If a Twilight Drake comes into contact with a pool of Twilight Essence, it siphons that energy and causes the pool to shrink and eventually vanish entirely. Each charge of Essence absorbed increases the Drake's damage by 10%, and its health by 10%. This effect stacks.
};

enum Events
{
    EVENT_FLAME_BREATH        = 1,
    EVENT_TWILIGHT_ORB        = 2,
    EVENT_SUMMON_HATCHLING    = 3,
    EVENT_SUMMON_DRAGON       = 4,
    EVENT_SUMMON_SPITECALLER  = 5,
    EVENT_START_BEAM_EVENT    = 6,
    EVENT_WRACK               = 7,
    EVENT_SUMMON_SHADOW_ORBS  = 8,
    EVENT_ESSENCE_OF_THE_RED  = 10,
    EVENT_SUMMON_CALEN        = 11,
    EVENT_CHANNEL_START       = 12,
    EVENT_RECAST_PROTECTION   = 13,
    EVENT_RESET_CHECK         = 14,
    EVENT_TWILIGHT_EXTINCTION = 15,
    EVENT_REGENERATE_MANA     = 16,
    EVENT_HEALTH_DEGENERATE   = 17,

    // whelps
    EVENT_SPIT                = 18,
    EVENT_ESSENCE_INCRAESE    = 19,
    EVENT_REVIVE              = 20,

    // twilight spitecaller
    EVENT_UNLASH_ESSENCE      = 21
};

enum Npc
{
    NPC_TWILIGHT_HATCHLING   = 48049,
    NPC_TWILIGHT_DRAGON      = 48436,
    NPC_TIWLIGHT_SPITECALLER = 48415,
    NPC_PULSING_TWILIGHT_EGG = 46842,
    NPC_BEAM_CHANNEL_TARGET  = 46835,
    NPC_BEAM_CHANNEL_VEHICLE = 46907,
    NPC_SINESTRA_CONTROLLER  = 46834,
    NPC_WHELP_SPAWNER        = 48052,
    NPC_EGG_STALKER          = 51609,
    NPC_SHADOW_ORB_1         = 49862,
    NPC_SHADOW_ORB_2         = 49863,
    NPC_FIERY_BARRIER        = 51608,
    NPC_FIRE_STALKER         = 43781
};

enum Phases
{
    PHASE_ALL       = 0,
    PHASE_ONE       = 1,
    PHASE_TWO_INTRO = 2,
    PHASE_TWO       = 3,
    PHASE_THREE     = 4
};

enum Actions
{
    ACTION_PHASE_2_WIN  = 1,
    ACTION_PHASE_2_FAIL = 2,
    ACTION_WHELP_REVIVE = 3,
    ACTION_RESTART_BEAM = 4
};

enum Objects
{
    GOB_CACHE_OF_THE_BROODMOTHER_10 = 208044,
    GOB_CACHE_OF_THE_BROODMOTHER_25 = 208045
};

Position const TwilightEggPos[2] =
{
    { -897.065f, -767.644f, 441.882f, 3.383f },
    { -993.588f, -668.238f, 440.249f, 4.511f }
};

Position const whelpSpawnerPos[5] =
{
    {  -892.680f, -803.110f, 459.358f, 3.119f },
    {  -960.062f, -859.287f, 460.877f, 2.099f },
    { -1089.101f, -765.012f, 455.267f, 6.266f },
    { -1024.698f, -701.151f, 459.659f, 5.068f },
    {  -949.012f, -725.393f, 459.659f, 3.990f }
};

Position const fireTriggers[6] =
{
    { -916.001f, -770.773f, 440.206f, 0.233f },
    { -997.433f, -724.154f, 438.415f, 1.662f },
    { -902.476f, -768.835f, 441.141f, 0.233f },
    { -998.209f, -687.032f, 440.657f, 6.184f },
    { -932.122f, -774.599f, 439.697f, 0.233f },
    { -998.840f, -705.729f, 440.121f, 3.227f }
};

Position const spiteCallerPos[3] =
{
    { -1117.1098f, -830.493f, 465.746f, 5.9237f },
    { -1058.2380f, -847.424f, 448.815f, 0.6382f },
    { -1016.9870f, -810.272f, 438.593f, 0.8103f }
};

Position const CalenPos = { -1015.425f, -810.174f, 438.593f, 0.886497f };
Position const BeamTarget = { -992.334f, -786.917f, 443.772f, 3.898f };
Position const ChestPos = { -959.285f, -749.841f, 438.593f, 4.04007f };

class TankAndBeamCheck : public std::unary_function<Unit*, bool>
{
public:
    TankAndBeamCheck(Unit* boss) : _boss(boss) { }

    bool operator()(Unit* unit) const
    {
        if (!unit || unit->GetTypeId() != TYPEID_PLAYER || unit->ToPlayer()->HasTankSpec() || unit->HasAura(SPELL_HITTIN_YA_PLAYER) || unit->HasAura(SPELL_TWILIGHT_BEAM) || _boss->getVictim() == unit)
            return false;
        return true;
    }

private:
    Unit* _boss;
};

class ShadowOrbChannelEvent : public BasicEvent
{
public:
    ShadowOrbChannelEvent(Creature* owner, Creature* target) : orb1(owner), orb2(target) { }

    bool Execute(uint64 /*execTime*/, uint32 /*diff*/) override
    {
        orb1->AddAura(SPELL_TWILIGHT_PULSE_PERIODIC, orb1);
        orb2->AddAura(SPELL_TWILIGHT_PULSE_PERIODIC, orb2);
        orb1->CastSpell(orb2, SPELL_TWILIGHT_SLICER_BEAM, true);
        orb2->CastSpell(orb1, SPELL_TWILIGHT_SLICER_BEAM, true);
        orb1->DespawnOrUnsummon(11500);
        orb2->DespawnOrUnsummon(11500);
        orb1->ClearUnitState(UNIT_STATE_CASTING);
        orb2->ClearUnitState(UNIT_STATE_CASTING);
        return true;
    }

private:
    Creature* orb1;
    Creature* orb2;
};

class ShadowOrbStartFollow : public BasicEvent
{
public:
    ShadowOrbStartFollow(Unit* owner, Unit* target, bool first = true) : _owner(owner), _target(target), _first(first) {}

    bool Execute(uint64 execTime, uint32 /*diff*/)
    {
        if (!_first)
        {
            _owner->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            _owner->ToCreature()->SetReactState(REACT_AGGRESSIVE);
        }
        if (_target && _target->IsInWorld() && _target->isAlive())
        {
            if (!_first)
            {
                _owner->GetMotionMaster()->MoveChase(_target, 0.1f, M_PI);
                _owner->AddThreat(_target, 1000000.0f);
            }
            else
                _owner->CastSpell(_target, SPELL_TWILIGHT_BEAM, true);
        }
        else
            if (Unit* newTarget = _owner->SelectNearbyTarget())
            {
                _target = newTarget;
                if (!_first)
                {
                    _owner->GetMotionMaster()->MoveChase(newTarget, 0.1f, M_PI);
                    _owner->AddThreat(_target, 1000000.0f);
                }
                else
                    _owner->CastSpell(_target, SPELL_TWILIGHT_BEAM, true);
            }

        if (_first)
        {
            _first = false;
            _owner->m_Events.AddEvent(this, execTime + 2000);
            return false;
        }
        return true;
    }

private:
    Unit* _owner;
    Unit* _target;
    bool  _first;
};


class BeemMovements : public BasicEvent
{
public:
    BeemMovements(Creature* beam, Unit *sinestra, uint64 calenGUID) : _beam(beam), _sinestra(sinestra), _calenGUID(calenGUID) {}

    bool Execute(uint64 execTime, uint32 /*diff*/)
    {
        if (Unit *_calen = Unit::GetUnit(*_sinestra, _calenGUID))
        {
            if (!_beam->isMoving())
            {
                float manaPct = _sinestra->GetPowerPct(POWER_MANA);
                float maxDist = _sinestra->GetDistance2d(_beam->GetHomePosition().GetPositionX(), _beam->GetHomePosition().GetPositionY());
                float currentDist = (maxDist * manaPct) / 100.0f;
                float healthPct = _calen->GetHealthPct();
                maxDist = _calen->GetDistance2d(_beam->GetHomePosition().GetPositionX(), _beam->GetHomePosition().GetPositionY());
                currentDist += maxDist - (maxDist * healthPct) / 100.0f;
                float x, y;
                _sinestra->GetNearPoint2D(x, y, currentDist, _sinestra->GetAngle(_beam));
                if (_beam->GetDistance2d(x, y) > 0.5f)
                    _beam->GetMotionMaster()->MovePoint(0, x, y, _beam->GetPositionZ(), false);
            }
            _beam->m_Events.AddEvent(this, execTime + 500);
            return false;
        }
        return true;
    }

private:
    Creature* _beam;
    Unit* _sinestra;
    uint64 _calenGUID;
};

class boss_sinestra : public CreatureScript
{
public:
    boss_sinestra() : CreatureScript("boss_sinestra") { }

    struct boss_sinestraAI: public BossAI
    {
        boss_sinestraAI(Creature* creature) : BossAI(creature, DATA_SINESTRA)
        {
            introDone = false;
            me->SetVisible(false);
        }

        void prepareFight()
        {
            me->SetHealth(me->GetMaxHealth() / 100 * 60);
            DoCast(me, SPELL_DRAINED, true);

            for (int i = 0; i < 2; ++i)
                me->SummonCreature(NPC_PULSING_TWILIGHT_EGG, TwilightEggPos[i].GetPositionX(), TwilightEggPos[i].GetPositionY(), TwilightEggPos[i].GetPositionZ(), TwilightEggPos[i].GetOrientation(), TEMPSUMMON_DEAD_DESPAWN);

            for (int i = 0; i < 5; ++i)
                me->SummonCreature(NPC_WHELP_SPAWNER, whelpSpawnerPos[i].GetPositionX(), whelpSpawnerPos[i].GetPositionY(), whelpSpawnerPos[i].GetPositionZ(), whelpSpawnerPos[i].GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);

            for (int i = 0; i < 6; ++i)
                me->SummonCreature(NPC_FIRE_STALKER, fireTriggers[i].GetPositionX(), fireTriggers[i].GetPositionY(), fireTriggers[i].GetPositionZ(), fireTriggers[i].GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
        }

        void Reset()
        {
            _Reset();
            prepareFight();
            eggCounter = 0;
            isInEggPhase = false;
            me->SetReactState(REACT_PASSIVE);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        }


        void EnterCombat(Unit* /*who*/)
        {
            me->SetReactState(REACT_AGGRESSIVE);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            events.SetPhase(PHASE_ONE);
            events.ScheduleEvent(EVENT_WRACK, 15000, 0, PHASE_ONE);
            events.ScheduleEvent(EVENT_SUMMON_HATCHLING, 16000, 0, PHASE_ONE);
            events.ScheduleEvent(EVENT_FLAME_BREATH, 21000, 0, PHASE_ONE);
            events.ScheduleEvent(EVENT_SUMMON_SHADOW_ORBS, 29000, 0, PHASE_ONE);
            events.ScheduleEvent(EVENT_RESET_CHECK, 2000, 0, PHASE_ALL);
            DoCast(me, SPELL_HITTIN_YA, true);
            DoCast(SPELL_CALL_FLAMES);
            _EnterCombat();
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);

            switch (summon->GetEntry())
            {
                case NPC_PULSING_TWILIGHT_EGG:
                    summon->CastSpell(summon, SPELL_TWILIGHT_CARAPACE, true);
                    summon->SetReactState(REACT_PASSIVE);
                    break;
                case NPC_WHELP_SPAWNER:
                    summon->SetCanFly(true);
                    summon->SetDisableGravity(true);
                    summon->SetReactState(REACT_PASSIVE);
                    break;
                case NPC_BEAM_CHANNEL_TARGET:
                    summon->SetCanFly(true);
                    summon->SetDisableGravity(true);
                    summon->SetReactState(REACT_PASSIVE);
                    summon->m_Events.AddEvent(new BeemMovements(summon, me, instance->GetData64(NPC_CALEN)), summon->m_Events.CalculateTime(3000));
                    break;
                case NPC_TWILIGHT_DRAGON:
                    summon->SetSpeed(MOVE_FLIGHT, 4.0f, true, true);
                    summon->SetSpeed(MOVE_WALK, 4.0f, true, true);
                    summon->SetInCombatWithZone();
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 200.0f, true))
                        summon->AI()->AttackStart(target);
                    //summon->GetMotionMaster()->MovePoint(0, -984.030f, -776.368f, 438.594f, true);
                    summon->SetReactState(REACT_AGGRESSIVE);
                    break;
                case NPC_TIWLIGHT_SPITECALLER:
                    summon->SetReactState(REACT_PASSIVE);
                    break;
                case NPC_SHADOW_ORB_1:
                case NPC_SHADOW_ORB_2:
                    summon->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
                    summon->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_THREAT, true);
                    summon->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                    break;
                default:
                    break;
            }
        }

        void SummonedCreatureDies(Creature* summon, Unit* killer)
        {
            switch (summon->GetEntry())
            {
                case NPC_PULSING_TWILIGHT_EGG:
                    eggCounter++;
                    if (eggCounter == 2)
                    {
                        if (Creature* calen = me->FindNearestCreature(NPC_CALEN, 300.0f, true))
                            calen->AI()->DoAction(ACTION_PHASE_2_WIN);
                        me->RemoveAurasDueToSpell(SPELL_DRAINED);
                        me->RemoveAurasDueToSpell(SPELL_MANA_BARRIER);
                        events.SetPhase(PHASE_THREE);
                        events.ScheduleEvent(EVENT_WRACK, 3000, 0, PHASE_THREE);
                        events.ScheduleEvent(EVENT_SUMMON_HATCHLING, 16000, 0, PHASE_THREE);
                        events.ScheduleEvent(EVENT_FLAME_BREATH, 21000, 0, PHASE_THREE);
                        events.ScheduleEvent(EVENT_SUMMON_SHADOW_ORBS, 29000, 0, PHASE_THREE);
                        me->CastStop();
                        me->RemoveAurasDueToSpell(SPELL_TWILIGHT_POWER);
                        me->CastWithDelay(1000, me, SPELL_CALL_FLAMES, true);
                        summons.DespawnEntry(NPC_BEAM_CHANNEL_TARGET);
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                            me->AI()->AttackStart(target);
                        me->resetAttackTimer();
                    }
                    break;
                default:
                    break;
            }
        }

        void JustDied(Unit* killer)
        {
            killer->SummonGameObject(Is25ManRaid() ? GOB_CACHE_OF_THE_BROODMOTHER_25 : GOB_CACHE_OF_THE_BROODMOTHER_10, ChestPos.GetPositionX(), ChestPos.GetPositionY(), ChestPos.GetPositionZ(), ChestPos.GetOrientation(), 0, 0, 0, 0, -1);
            Talk(SAY_DEATH);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            _JustDied();
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(urand(SAY_SLAY_1, SAY_SLAY_2));
        }

        void DamageTaken(Unit* attacker, uint32& damage)
        {
            if (HealthBelowPct(31) && events.IsInPhase(PHASE_ONE))
            {
                Talk(SAY_PHASE_2_START);
                events.SetPhase(PHASE_TWO_INTRO);
                me->SetReactState(REACT_PASSIVE);
                me->CastStop();
                me->AttackStop();
                DoCast(me, SPELL_MANA_BARRIER);
                if (Creature *orb1 = me->FindNearestCreature(NPC_SHADOW_ORB_1, 100.0f))
                    if (Creature *orb2 = me->FindNearestCreature(NPC_SHADOW_ORB_2, 100.0f))
                    {
                        orb1->DespawnOrUnsummon(1000);
                        orb2->DespawnOrUnsummon(1000);
                    }
                events.ScheduleEvent(EVENT_TWILIGHT_EXTINCTION, 8000, 0, PHASE_TWO_INTRO);
            }

            if (events.IsInPhase(PHASE_TWO))
                me->SetHealth(me->GetMaxHealth());

            if (!isInEggPhase && events.IsInPhase(PHASE_TWO) && me->GetPowerPct(POWER_MANA) > 25)
            {
                me->SetPower(POWER_MANA, me->GetPower(POWER_MANA) - damage);

                if (!isInEggPhase && me->GetPowerPct(POWER_MANA) <= 25)
                {
                    isInEggPhase = true;
                    for (SummonList::iterator i = summons.begin(); i != summons.end(); ++i)
                    {
                        Creature* summon = ObjectAccessor::GetCreature(*me, *i);
                        if (summon && summon->GetEntry() == NPC_PULSING_TWILIGHT_EGG)
                        {
                            summon->SetInCombatWithZone();
                            summon->RemoveAurasDueToSpell(SPELL_TWILIGHT_CARAPACE);
                            summon->CastSpell(me, SPELL_TWILIGHT_BEAM, true);
                            summon->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1);
                        }
                    }
                    Talk(urand(SAY_SPECIAL_1, SAY_SPECIAL_3));
                    events.ScheduleEvent(EVENT_REGENERATE_MANA, 1000, 0, PHASE_TWO);
                    events.ScheduleEvent(EVENT_RECAST_PROTECTION, 30000, 0, PHASE_TWO);
                }
            }
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (me->IsWithinDistInMap(who, 80.0f) && who->GetTypeId() == TYPEID_PLAYER && !introDone)
            {
                if (instance->IsDoneInHeroic(DATA_CHOGALL))
                {
                    introDone = true;
                    Talk(SAY_INTRO);
                    me->SetVisible(true);
                    DoCast(me, SPELL_REBIRTH);
                }
            }
        }

        void DoMeleeAttackIfReady()
        {
            if (me->getVictim())
            {
                if (me->isAttackReady() && !me->IsNonMeleeSpellCasted(false))
                {
                    if (me->IsWithinMeleeRange(me->getVictim()))
                    {
                        me->AttackerStateUpdate(me->getVictim());
                        me->resetAttackTimer();
                    }
                    else if (events.IsInPhase(PHASE_ONE) || events.IsInPhase(PHASE_THREE))
                    {
                        if (!me->HasUnitState(UNIT_STATE_CASTING))
                        {
                            me->resetAttackTimer();
                            DoCastVictim(SPELL_TWILIGHT_BLAST);
                        }
                    }
                }
            }
        }

        void DoAction(int32 const act)
        {
            switch (act)
            {
                case ACTION_PHASE_2_FAIL:
                    Talk(SAY_WINNING);
                    me->CastStop();
                    DoCast(me, SPELL_TWILIGHT_EXTINCTION, false);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING) && !events.IsInPhase(PHASE_TWO))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_WRACK:
                        DoCastRandom(SPELL_WRACK_INITIAL, 200.0f);
                        if (events.IsInPhase(PHASE_ONE) || events.IsInPhase(PHASE_THREE))
                            events.ScheduleEvent(EVENT_WRACK, 61000, 0, events.IsInPhase(PHASE_ONE) ? PHASE_ONE : PHASE_THREE);
                        break;
                    case EVENT_SUMMON_HATCHLING:
                    {
                        Talk(SAY_SUMMON);
                        for (SummonList::iterator i = summons.begin(); i != summons.end(); ++i)
                        {
                            Creature* summon = ObjectAccessor::GetCreature(*me, *i);
                            if (summon && summon->GetEntry() == NPC_WHELP_SPAWNER)
                                me->SummonCreature(NPC_TWILIGHT_HATCHLING, summon->GetPositionX(), summon->GetPositionY(), summon->GetPositionZ(), summon->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                        }

                        if (events.IsInPhase(PHASE_ONE) || events.IsInPhase(PHASE_THREE))
                            events.ScheduleEvent(EVENT_SUMMON_HATCHLING, 50000, 0, events.IsInPhase(PHASE_ONE) ? PHASE_ONE : PHASE_THREE);
                        break;
                    }
                    case EVENT_FLAME_BREATH:
                        DoCastVictim(SPELL_FLAME_BREATH);
                        if (events.IsInPhase(PHASE_ONE) || events.IsInPhase(PHASE_THREE))
                            events.ScheduleEvent(EVENT_FLAME_BREATH, 21000, 0, events.IsInPhase(PHASE_ONE) ? PHASE_ONE : PHASE_THREE);
                        break;
                    case EVENT_SUMMON_SHADOW_ORBS:
                    {
                        Position pos;
                        Creature* ShadowOrb;
                        Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, TankAndBeamCheck(me));

                        if (!target)
                            target = SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0f, true);

                        if (target)
                        {
                            target->GetNearPosition(pos, 3.0f, 0.0f);
                            ShadowOrb = me->SummonCreature(NPC_SHADOW_ORB_1, pos, TEMPSUMMON_TIMED_DESPAWN, 30000);
                            if (ShadowOrb)
                            {
                                ShadowOrb->SetReactState(REACT_PASSIVE);
                                ShadowOrb->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                                ShadowOrb->SetInCombatWithZone();
                                ShadowOrb->m_Events.AddEvent(new ShadowOrbStartFollow(ShadowOrb, target), ShadowOrb->m_Events.CalculateTime(2000));
                            }
                        }

                        Position pos2;
                        Creature* ShadowOrb2;
                        Unit* target2 = SelectTarget(SELECT_TARGET_RANDOM, 0, TankAndBeamCheck(me));

                        if (!target2 || target == target2)
                            target2 = SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0f, true);

                        if (target2)
                        {
                            target2->GetNearPosition(pos2, 3.0f, 0.0f);
                            ShadowOrb2 = me->SummonCreature(NPC_SHADOW_ORB_2, pos2, TEMPSUMMON_TIMED_DESPAWN, 30000);
                            if (ShadowOrb2)
                            {
                                ShadowOrb2->SetReactState(REACT_PASSIVE);
                                ShadowOrb->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                                ShadowOrb2->SetInCombatWithZone();
                                ShadowOrb2->m_Events.AddEvent(new ShadowOrbStartFollow(ShadowOrb2, target2), ShadowOrb2->m_Events.CalculateTime(2000));
                            }
                        }

                        if (ShadowOrb && ShadowOrb2)
                            ShadowOrb->m_Events.AddEvent(new ShadowOrbChannelEvent(ShadowOrb, ShadowOrb2), ShadowOrb->m_Events.CalculateTime(3900));

                        if (events.IsInPhase(PHASE_ONE) || events.IsInPhase(PHASE_THREE))
                            events.ScheduleEvent(EVENT_SUMMON_SHADOW_ORBS, 28000, 0, events.IsInPhase(PHASE_ONE) ? PHASE_ONE : PHASE_THREE);
                        break;
                    }
                    case EVENT_TWILIGHT_EXTINCTION:
                        for (SummonList::iterator i = summons.begin(); i != summons.end(); ++i)
                            if (Creature* summon = ObjectAccessor::GetCreature(*me, *i))
                                if (summon->GetEntry() == NPC_FIRE_STALKER)
                                    summon->RemoveAllAuras();

                        events.SetPhase(PHASE_TWO);
                        DoCast(me, SPELL_TWILIGHT_EXTINCTION, false);
                        events.ScheduleEvent(EVENT_SUMMON_CALEN, 9000, 0, PHASE_TWO);
                        events.ScheduleEvent(EVENT_CHANNEL_START, 19500, 0, PHASE_TWO);
                        break;
                    case EVENT_SUMMON_CALEN:
                        me->SummonCreature(NPC_CALEN, CalenPos.GetPositionX(), CalenPos.GetPositionY(), CalenPos.GetPositionZ(), CalenPos.GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                        me->SummonCreature(NPC_BEAM_CHANNEL_TARGET, BeamTarget.GetPositionX(), BeamTarget.GetPositionY(), BeamTarget.GetPositionZ(), BeamTarget.GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                        break;
                    case EVENT_CHANNEL_START:
                        me->SetReactState(REACT_AGGRESSIVE);
                        if (Creature* channelTarget = me->FindNearestCreature(NPC_BEAM_CHANNEL_TARGET, 100.0f, true))
                            DoCast(channelTarget, SPELL_TWILIGHT_POWER);
                        for (SummonList::iterator i = summons.begin(); i != summons.end(); ++i)
                        {
                            Creature* summon = ObjectAccessor::GetCreature(*me, *i);
                            if (summon && summon->GetEntry() == NPC_CALEN)
                                summon->AI()->Talk(SAY_CALEN_BATTLE_START);
                        }
                        events.ScheduleEvent(EVENT_SUMMON_DRAGON, urand(12000, 18000), 0, PHASE_TWO);
                        events.ScheduleEvent(EVENT_SUMMON_SPITECALLER, urand(12000, 18000), 0, PHASE_TWO);
                        break;
                    case EVENT_SUMMON_DRAGON:
                        me->SummonCreature(NPC_TWILIGHT_DRAGON, -1024.697f, -701.151f, 459.658f, 5.06800f, TEMPSUMMON_DEAD_DESPAWN);
                        events.ScheduleEvent(EVENT_SUMMON_DRAGON, 30000, 0, PHASE_TWO);
                        break;
                    case EVENT_SUMMON_SPITECALLER:
                        me->SummonCreature(NPC_TIWLIGHT_SPITECALLER, spiteCallerPos[0].GetPositionX(), spiteCallerPos[0].GetPositionY(), spiteCallerPos[0].GetPositionZ(), spiteCallerPos[0].GetOrientation(), TEMPSUMMON_DEAD_DESPAWN);
                        events.ScheduleEvent(EVENT_SUMMON_SPITECALLER, 30000, 0, PHASE_TWO);
                        break;
                    case EVENT_RECAST_PROTECTION:
                        for (SummonList::iterator i = summons.begin(); i != summons.end(); ++i)
                            if (Creature* summon = ObjectAccessor::GetCreature(*me, *i))
                                if (summon && summon->GetEntry() == NPC_PULSING_TWILIGHT_EGG)
                                {
                                    summon->CastStop();
                                    summon->RemoveAurasDueToSpell(SPELL_TWILIGHT_BEAM);
                                    summon->CastSpell(summon, SPELL_TWILIGHT_CARAPACE, true);
                                    summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1);
                                }
                        me->SetPower(POWER_MANA, me->GetMaxPower(POWER_MANA));
                        isInEggPhase = false;
                        break;
                    case EVENT_RESET_CHECK:
                    {
                        bool playerFound = false;
                        std::list<WorldObject*> targetList;
                        Map::PlayerList const& players = me->GetMap()->GetPlayers();
                        if (!players.isEmpty())
                        {
                            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                            {
                                if (Player* target = itr->getSource())
                                {
                                    if (target->isAlive() && me->GetDistance2d(target) < 100.0f && !target->isGameMaster())
                                    {
                                        playerFound = true;
                                        break;
                                    }
                                }
                            }
                        }
                        events.ScheduleEvent(EVENT_RESET_CHECK, 2000);
                        if (!playerFound)
                            EnterEvadeMode();
                        break;
                    }
                    case EVENT_REGENERATE_MANA:
                        if (isInEggPhase)
                        {
                            me->SetPower(POWER_MANA, me->GetPower(POWER_MANA) + me->GetMaxPower(POWER_MANA) * 0.025f);
                            events.ScheduleEvent(EVENT_REGENERATE_MANA, 1000, 0, PHASE_TWO);
                        }
                        break;
                    default:
                        break;
                }
            }
            if (events.IsInPhase(PHASE_ONE) || events.IsInPhase(PHASE_THREE))
                DoMeleeAttackIfReady();
        }
        private:
            uint8 eggCounter;
            bool introDone, isInEggPhase;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_sinestraAI(creature);
    }
};

class BarrierGrowUpEvent : public BasicEvent
{
public:
    BarrierGrowUpEvent(Creature* owner) : _owner(owner) { }

    bool Execute(uint64 execTime, uint32 /*diff*/) override
    {
        _owner->ToCreature()->SetObjectScale(1.0f);
        return true;
    }

private:
    Creature* _owner;

};

class npc_calen : public CreatureScript
{
public:
    npc_calen() : CreatureScript("npc_calen") { }

    struct npc_calenAI : public ScriptedAI
    {
        npc_calenAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
            warned = false;
            die = false;
        }

        void IsSummonedBy(Unit* summoner)
        {
            Talk(SAY_CALEN_SUMMON);
            DoCast(me, SPELL_FIERY_BARRIER_INITIAL, true);
            DoCast(me, SPELL_PYRRHIC_FOCUS, true);
            healthToRemove = me->GetMaxHealth() * 0.0083f;

        }

        void JustDied(Unit* /*killer*/)
        {
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            summons.DespawnAll();
        }

        void JustSummoned(Creature* summon)
        {
            if (summon->GetEntry() == NPC_FIERY_BARRIER)
            {
                summon->SetObjectScale(0.1f);
                summon->CastWithDelay(2000, summon, SPELL_FIERY_BARIER_VISUAL, true);
                summon->CastWithDelay(4000, summon, SPELL_FIERY_BARRIER_GROUND_EFFECT, true);
                summon->m_Events.AddEvent(new BarrierGrowUpEvent(summon), summon->m_Events.CalculateTime(1500));
                events.ScheduleEvent(EVENT_CHANNEL_START, 25000);
                summon->DespawnOrUnsummon(27000);
            }
        }

        void DamageTaken(Unit* attacker, uint32& damage)
        {

        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_PHASE_2_WIN:
                    Talk(SAY_CALEN_WINNING);
                    events.ScheduleEvent(EVENT_ESSENCE_OF_THE_RED, 24000);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_HEALTH_DEGENERATE:
                        me->SetHealth(me->GetHealth() - healthToRemove);
                        if (HealthBelowPct(50) && !warned)
                        {
                            warned = true;
                            Talk(SAY_CALEN_LOSING);
                        }

                        if (HealthBelowPct(25) && !die)
                        {
                            die = true;
                            Talk(SAY_CALEN_DIE);
                            if (Creature* sinestra = Creature::GetCreature(*me, instance->GetData64(NPC_SINESTRA)))
                            {
                                sinestra->AI()->DoAction(ACTION_PHASE_2_FAIL);
                                sinestra->Kill(me, false);
                                return;
                            }
                        }
                        events.ScheduleEvent(EVENT_HEALTH_DEGENERATE, 1000);
                        break;
                    case EVENT_ESSENCE_OF_THE_RED:
                        Talk(SAY_CALEN_PHASE_3);
                        DoCastAOE(SPELL_ESSENCE_OF_THE_RED, true);
                        me->DespawnOrUnsummon(100);
                        break;
                    case EVENT_CHANNEL_START:
                        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                        if (Creature* channelTarget = me->FindNearestCreature(NPC_BEAM_CHANNEL_TARGET, 100.0f, true))
                            DoCast(channelTarget, SPELL_FIERY_RESOLVE);
                        me->RemoveAurasDueToSpell(SPELL_FIERY_BARRIER_INITIAL);
                        events.ScheduleEvent(EVENT_HEALTH_DEGENERATE, 1000);
                        break;
                }
            }
        }

    private:
        InstanceScript* instance;
        EventMap events;
        SummonList summons;
        bool warned, die;
        float healthToRemove;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_calenAI(creature);
    }
};

class npc_sinestra_twilight_whelps : public CreatureScript
{
public:
    npc_sinestra_twilight_whelps() : CreatureScript("npc_sinestra_twilight_whelps") { }

    struct npc_sinestra_twilight_whelpsAI : public ScriptedAI
    {
        npc_sinestra_twilight_whelpsAI(Creature* creature) : ScriptedAI(creature)
        {
            reviveInProgress = false;
            me->SetCanFly(true);
            me->SetDisableGravity(true);
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->SetMaxHealth(RAID_MODE(200488, 641560, 200488, 641560));
            me->SetHealth(me->GetMaxHealth());
            me->SetReactState(REACT_AGGRESSIVE);
            me->SetInCombatWithZone();
            if (Unit* target = summoner->ToCreature()->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                me->AI()->AttackStart(target);
        }

        void EnterCombat(Unit* attacker)
        {
            events.ScheduleEvent(EVENT_SPIT, 5000);
        }

        void DoAction(int32 const action)
        {
            if (reviveInProgress || !me->HasAura(SPELL_FEIGN_DEATH))
                return;

            if (action == ACTION_WHELP_REVIVE)
            {
                reviveInProgress = true;
                events.Reset();
                events.ScheduleEvent(EVENT_REVIVE, 3000);
                Talk(EMOTE_WHELP_REVIVE);
            }
        }

       void EnterEvadeMode() {}

       void DamageTaken(Unit* attacker, uint32& damage)
        {
            if (damage >= me->GetHealth())
            {
                damage = 0;
                me->RemoveAllAuras();
                me->SetReactState(REACT_PASSIVE);
                me->SetHealth(me->GetMaxHealth());
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE);
                me->SetDisplayId(11686);
                DoCast(me, SPELL_FEIGN_DEATH, true);
                events.ScheduleEvent(EVENT_ESSENCE_INCRAESE, 30000);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            if (me->HasAura(SPELL_FEIGN_DEATH) && !me->HasAura(SPELL_TWILIGHT_ESSENCE))
                me->AddAura(SPELL_TWILIGHT_ESSENCE, me);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SPIT:
                        DoCastVictim(SPELL_TWILIGHT_SPIT);
                        events.ScheduleEvent(EVENT_SPIT, 5000);
                        break;
                    case EVENT_ESSENCE_INCRAESE:
                        me->AddAura(SPELL_TWILIGHT_ESSENCE_INCREASE, me);
                        events.ScheduleEvent(EVENT_ESSENCE_INCRAESE, 15000);
                        break;
                    case EVENT_REVIVE:
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->SetDisplayId(me->GetCreatureTemplate()->Modelid1);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE);
                        me->SetInCombatWithZone();
                        me->RemoveAurasDueToSpell(SPELL_FEIGN_DEATH);
                        me->RemoveAurasDueToSpell(SPELL_TWILIGHT_ESSENCE);
                        me->RemoveAurasDueToSpell(SPELL_TWILIGHT_ESSENCE_INCREASE);
                        break;
                }
            }
        }

    private:
        EventMap events;
        bool reviveInProgress;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sinestra_twilight_whelpsAI(creature);
    }
};

class npc_spitecaller : public CreatureScript
{
public:
    npc_spitecaller() : CreatureScript("npc_spitecaller") { }

    struct npc_spitecallerAI : public ScriptedAI
    {
        npc_spitecallerAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            interruptCount = 0;
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC);
            DoCast(me, SPELL_INDOMITABLE_VISUAL, true);
            me->SetSpeed(MOVE_RUN, 3.0f, true, true);
            me->SetSpeed(MOVE_WALK, 3.0f, true, true);
            me->GetMotionMaster()->MovePoint(1, spiteCallerPos[1].GetPositionX(), spiteCallerPos[1].GetPositionY(), spiteCallerPos[1].GetPositionZ(), true);
        }

        void SpellHit(Unit* attacker, SpellInfo const* spellInfo)
        {
            bool castEnrage = false;
            bool isInterrupt = false;
            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (spellInfo->Effects[i].Mechanic == MECHANIC_INTERRUPT || spellInfo->Mechanic == MECHANIC_INTERRUPT)
                    castEnrage = true;
                else if (spellInfo->Effects[i].Mechanic == MECHANIC_DISORIENTED && spellInfo->Mechanic == MECHANIC_NONE || spellInfo->Effects[i].Mechanic == MECHANIC_SILENCE && spellInfo->Mechanic == MECHANIC_NONE
                         || spellInfo->Effects[i].Mechanic == MECHANIC_STUN && spellInfo->Mechanic == MECHANIC_NONE || spellInfo->Effects[i].Mechanic == MECHANIC_KNOCKOUT && spellInfo->Mechanic == MECHANIC_NONE
                         || spellInfo->Effects[i].Mechanic == MECHANIC_GRIP && spellInfo->Mechanic == MECHANIC_NONE || spellInfo->Effects[i].Mechanic == MECHANIC_FEAR && spellInfo->Mechanic == MECHANIC_NONE)
                {
                    interruptCount++;
                    isInterrupt = true;
                    me->InterruptNonMeleeSpells(false);
                }
            }

            // special check some spells only have a mechanic but not effect mechanic
            if (spellInfo->Mechanic == MECHANIC_DISORIENTED || spellInfo->Mechanic == MECHANIC_SILENCE
                || spellInfo->Mechanic == MECHANIC_STUN || spellInfo->Mechanic == MECHANIC_KNOCKOUT
                || spellInfo->Mechanic == MECHANIC_GRIP || spellInfo->Mechanic == MECHANIC_FEAR)
            {
                interruptCount++;
                isInterrupt = true;
                me->InterruptNonMeleeSpells(false);
            }

            if (interruptCount > 1 && isInterrupt || castEnrage)
                DoCast(me, SPELL_INDOMITABLE, true);
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            if (id == 1)
                me->GetMotionMaster()->MovePoint(2, spiteCallerPos[2].GetPositionX(), spiteCallerPos[2].GetPositionY(), spiteCallerPos[2].GetPositionZ(), true);
            else if (id == 2)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC);
                me->SetSpeed(MOVE_RUN, 1.1f, true, true);
                me->SetSpeed(MOVE_WALK, 1.1f, true, true);
                me->SetReactState(REACT_AGGRESSIVE);
                me->SetInCombatWithZone();
                events.ScheduleEvent(EVENT_UNLASH_ESSENCE, 8000);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_UNLASH_ESSENCE:
                        interruptCount = 0; // reset interrupt count
                        me->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                        DoCast(me, SPELL_UNLASH_ESSENCE);
                        events.ScheduleEvent(EVENT_UNLASH_ESSENCE, 8000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
        uint8 interruptCount;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_spitecallerAI(creature);
    }
};

class WrackAuraCheck
{
public:
    bool operator()(WorldObject* target) const
    {
        if (Unit* unit = target->ToUnit())
            if (unit->HasAura(SPELL_WRACK_INITIAL) || unit->HasAura(SPELL_WRACK_JUMP))
                return true;

        return false;
    }
};

class spell_sinestra_wrack : public SpellScriptLoader
{
public:
    spell_sinestra_wrack() : SpellScriptLoader("spell_sinestra_wrack") { }

    class spell_sinestra_wrack_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sinestra_wrack_AuraScript);


        void OnUpdate(AuraEffect* aurEff)
        {
            if (aurEff->GetTickNumber() > 1)
                aurEff->SetAmount(1.5f * aurEff->GetAmount());

            GetAura()->GetEffect(EFFECT_1)->SetAmount(GetDuration());

            if (Unit* target = GetTarget())
                if (InstanceScript* instance = target->GetInstanceScript())
                    instance->SetData(DATA_WRACK_DURATION, GetDuration());
        }

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            {
                if (Unit* target = GetTarget())
                {
                    int32 damage = 2000;
                    int32 duration = GetAura()->GetEffect(EFFECT_1)->GetAmount();

                    target->CastCustomSpell(target, SPELL_WRACK_JUMP, &damage, &duration, NULL, TRIGGERED_CAST_DIRECTLY, NULL, aurEff, GetCasterGUID());
                }
            }
        }

        void Register()
        {
            OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_sinestra_wrack_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            OnEffectRemove += AuraEffectRemoveFn(spell_sinestra_wrack_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_sinestra_wrack_AuraScript();
    }
};

class spell_sinestra_wrack_jump : public SpellScriptLoader
{
public:
    spell_sinestra_wrack_jump() : SpellScriptLoader("spell_sinestra_wrack_jump") {}

    class spell_sinestra_wrack_jump_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_sinestra_wrack_jump_SpellScript);

        void FilterTargetsInitial(std::list<WorldObject*>& targets)
        {
            targets.remove_if(WrackAuraCheck());
            Trinity::Containers::RandomResizeList(targets, 2);
            sharedTargets = targets;
        }

        void SetTargets(std::list<WorldObject*>& targets)
        {
            targets = sharedTargets;
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sinestra_wrack_jump_SpellScript::FilterTargetsInitial, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sinestra_wrack_jump_SpellScript::SetTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
        }

    private:
        std::list<WorldObject*> sharedTargets;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_sinestra_wrack_jump_SpellScript();
    }

    class spell_sinestra_wrack_jump_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sinestra_wrack_jump_AuraScript);

        void OnUpdate(AuraEffect* aurEff)
        {
            if (aurEff->GetTickNumber() > 1)
                aurEff->SetAmount(1.5f * aurEff->GetAmount());

            GetAura()->GetEffect(EFFECT_1)->SetAmount(GetDuration());

            if (Unit* target = GetTarget())
                if (InstanceScript* instance = target->GetInstanceScript())
                    instance->SetData(DATA_WRACK_DURATION, GetDuration());
        }

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            {
                if (Unit* target = GetTarget())
                {
                    int32 damage = 2000;
                    int32 duration = GetAura()->GetEffect(EFFECT_1)->GetAmount();

                    target->CastCustomSpell(target, SPELL_WRACK_JUMP, &damage, &duration, NULL, TRIGGERED_CAST_DIRECTLY, NULL, aurEff, GetCasterGUID());
                }
            }
        }

        void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            SetMaxDuration(aurEff->GetAmount());
            SetDuration(aurEff->GetAmount());

            if (GetDuration() >= 59000)
            {
                if (Unit* target = GetTarget())
                {
                    if (InstanceScript* instance = target->GetInstanceScript())
                    {
                        uint32 duration = instance->GetData(DATA_WRACK_DURATION);
                        if (duration)
                        {
                            SetMaxDuration(duration);
                            SetDuration(duration);
                        }
                    }
                }
            }
        }

        void Register()
        {
            OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_sinestra_wrack_jump_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            OnEffectRemove += AuraEffectRemoveFn(spell_sinestra_wrack_jump_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            AfterEffectApply += AuraEffectApplyFn(spell_sinestra_wrack_jump_AuraScript::OnApply, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_sinestra_wrack_jump_AuraScript();
    }
};

class spell_twilight_extinction : public SpellScriptLoader
{
public:
    spell_twilight_extinction() : SpellScriptLoader("spell_twilight_extinction") { }

    class spell_twilight_extinction_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_twilight_extinction_AuraScript);

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
                caster->CastSpell(caster, GetSpellInfo()->Effects[EFFECT_1].BasePoints, true);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_twilight_extinction_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_twilight_extinction_AuraScript();
    }
};

class FlamesTargetFilter
{
public:
    bool operator()(WorldObject* target) const
    {
        if (Unit* unit = target->ToUnit())
            return unit->GetEntry() != NPC_FIRE_STALKER;
        return false;
    }
};

class spell_call_flames : public SpellScriptLoader
{
public:
    spell_call_flames() : SpellScriptLoader("spell_call_flames") { }

    class spell_call_flames_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_call_flames_SpellScript);

        void HandleDummy(SpellEffIndex effIndex)
        {
            GetHitUnit()->CastWithDelay(1000, GetHitUnit(), SPELL_TWILIGHT_FLAME, true);
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(FlamesTargetFilter());
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_call_flames_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_call_flames_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_call_flames_SpellScript();
    }
};

class isBetweenCheck
{
public:
    isBetweenCheck() { }

    bool operator()(WorldObject* object)
    {
        if (Unit* unit = object->ToUnit())
        {
            if (Creature* ShadowOrb = unit->FindNearestCreature(NPC_SHADOW_ORB_1, 200.0f, true))
            {
                if (Creature* ShadowOrb2 = unit->FindNearestCreature(NPC_SHADOW_ORB_2, 200.0f, true))
                {
                    if (unit->IsInBetween(ShadowOrb, ShadowOrb2, 1.5f))
                        return false;
                }
            }
        }
        return true;
    }
};

class spell_twilight_pulse_selector : public SpellScriptLoader
{
public:
    spell_twilight_pulse_selector() : SpellScriptLoader("spell_twilight_pulse_selector") { }

    class spell_twilight_pulse_selector_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_twilight_pulse_selector_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(isBetweenCheck());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_twilight_pulse_selector_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_twilight_pulse_selector_SpellScript();
    }
};

class EntryFilter
{
public:
    EntryFilter(Unit* source, float dist) : _source(source), _dist(dist) {}

    bool operator()(WorldObject* unit)
    {
        if (Unit* target = unit->ToUnit())
            if (target->GetEntry() == NPC_TWILIGHT_HATCHLING || target->GetEntry() == NPC_TWILIGHT_DRAGON)
            {
                if (target->GetEntry() == NPC_TWILIGHT_DRAGON && !target->isAlive())
                    return true;
                if (_source->GetExactDist2d(target) < _dist)
                    return false;
            }

        return true;
    }

private:
    Unit* _source;
    float _dist;
};

class ExactDistanceCheck
{
public:
    ExactDistanceCheck(Unit* source, float dist) : _source(source), _dist(dist) {}

    bool operator()(WorldObject* unit)
    {
        return _source->GetExactDist2d(unit) > _dist;
    }

private:
    Unit* _source;
    float _dist;
};

class spell_twilight_essence_selector : public SpellScriptLoader
{
public:
    spell_twilight_essence_selector() : SpellScriptLoader("spell_twilight_essence_selector") { }

    class spell_twilight_essence_selector_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_twilight_essence_selector_SpellScript);

        void FilterNpcs(std::list<WorldObject*>& targets)
        {
            targets.remove(GetCaster());
            targets.remove_if(EntryFilter(GetCaster(), 5.0f * GetCaster()->GetFloatValue(OBJECT_FIELD_SCALE_X)));
        }

        void FilterDamageTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(ExactDistanceCheck(GetCaster(), 5.0f * GetCaster()->GetFloatValue(OBJECT_FIELD_SCALE_X)));
        }

        void HandleDummy(SpellEffIndex effIndex)
        {
            if (!GetCaster() || !GetHitUnit())
                return;
            if (Creature* target = GetHitUnit()->ToCreature())
            {
                if (target->GetEntry() == NPC_TWILIGHT_HATCHLING)
                    target->AI()->DoAction(ACTION_WHELP_REVIVE);
                else if (target->GetEntry() == NPC_TWILIGHT_DRAGON && target->isAlive())
                {
                    if (Creature* caster = GetCaster()->ToCreature())
                    {
                        if (Aura* essence = caster->GetAura(SPELL_TWILIGHT_ESSENCE_INCREASE, caster->GetGUID()))
                        {
                            uint32 stacks = essence->GetStackAmount();

                            if (stacks <= 4)
                            {
                                caster->DespawnOrUnsummon();
                                return;
                            }

                            essence->SetStackAmount(stacks - 4);
                        }
                        target->CastSpell(target, SPELL_ABSORB_ESSENCE, true);
                    }
                }
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_twilight_essence_selector_SpellScript::FilterNpcs, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_twilight_essence_selector_SpellScript::FilterDamageTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(spell_twilight_essence_selector_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_twilight_essence_selector_SpellScript();
    }
};

// All are verified below to DB, creature_template & spell_script_names
void AddSC_boss_sinestra()
{
    new boss_sinestra();
    new npc_calen();
    new npc_sinestra_twilight_whelps();
    new npc_spitecaller();
    new spell_sinestra_wrack();
    new spell_sinestra_wrack_jump();
    new spell_twilight_extinction();
    new spell_call_flames();
    new spell_twilight_pulse_selector();
    new spell_twilight_essence_selector();
}
