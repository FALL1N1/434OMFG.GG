
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "bastion_of_twilight.h"

#define MAX_DAZZLING_DESTRUCTION_CASTS 3

// TODO: intro is missing

enum ValionaYells
{
  SAY_VA_DEATH = 3,
  SAY_VA_KILL  = 2,
  SAY_VA_BOUT  = 5,
  SAY_VA_DB    = 4
};

enum TheralionYells
{
  SAY_TH_DEATH = 3,
  SAY_TH_KILL  = 2,
  SAY_TH_ENGF  = 6,
  SAY_TH_DAZZ  = 4,
  SAY_TH_DAZZ_2 = 5
};

enum Spells
{
    /***** GENERAL *****/

    SPELL_SELF_ROOT         = 42716,
    SPELL_BERSERK           = 47008, // 10 min.

    // Heroic / Twilight Realm stuff.
    SPELL_TWILIGHT_SHIFT_A  = 93053, // Melee trigger aura for both dragons. 5 stacks get next in one min! HEROIC ONLY!
    SPELL_TWILIGHT_SHIFT    = 93051,  // Spell, 5 stacks get next in one min! HEROIC ONLY!
    SPELL_SHIFTING_REALITY  = 93055, // Sends you to Twilight Realm (see next spell).
    // Portals
    SPELL_COLLAPSING_PORTAL = 86291, // Every minute, grows larger.
    SPELL_SUMMON_PORTAL     = 86289,
    SPELL_TWILIGHT_PORTAL_VISUAL = 95716,
    // HEROIC
    SPELL_TWILIGHT_SENTRY_SUMMON_PERDIODIC = 93029,
    SPELL_TWILIGHT_SENTRY_SUMMON = 93035,
    SPELL_TWILIGHT_SENTRY_SUMMON_TRIGGER = 93034,
    /***** VALIONA *****/

    // Ground
    SPELL_DEVOURING_FLAME_TRACKER_SUMMON = 86833,
    SPELL_DEVOURING_FLAMES  = 86840,
    SPELL_BLACKOUT          = 86788,
    SPELL_BLACKOUT_DAMAGE   = 86825,

    // Air
    SPELL_TW_METEOR_TARGET  = 88518,
    SPELL_TW_METEOR         = 86013,

    SPELL_DEEP_BREATH_DUMMY = 78954, // See Valiona doing magical things with her tongue!
    SPELL_DEEP_BREATH       = 86059, // Does nothing, script effect. Valiona should trigger Twilight Flames three times and fill 1/3 of a room with this before landing.
    SPELL_SUM_TWIL_FLAMES   = 75931, // Summon mob. 75932.
    SPELL_TWILIGHT_FLAMES   = 86199, // Actual damage and effect(86202 for screen eff and 86214 for damage taken in twilight zone, trig by 86210), trig by 86194.
    // This and 86228 need 8 y radius.
    SPELL_TW_FLAMES_TRIGGER = 86194, // Trigger for tw flames every sec.
    SPELL_TWILIGHT_ZONE     = 86202, // Screen eff, phase and make obj visible. check the raid mode here
    SPELL_TWILIGHT_ZONE_2   = 88436,
    SPELL_TWILIGHT_ZONE_AUR = 86210, // Aura triggers 86214 every 2 sec for 5k damage and shadow incr 10% in T.R. HEROIC ONLY!

    /***** THERALION *****/

    // Ground
    SPELL_ENGULFING_MAGIC   = 86607,
    SPELL_ENGULGING_MAGIC_DAMAGE = 86631,
    SPELL_FABULOUS_FLAMES   = 86497,

    // Air
    SPELL_TWILIGHT_BLAST    = 86369,
    SPELL_DAZZ_DESTRUCTION_DUMMY    = 86408,
    SPELL_DAZZ_DESTRUCTION_SUMMON   = 86385,

    /***** MISC *****/
    SPELL_TWIL_PROTECT_BUFF = 86415, // Connected somehow to 86199 (tw flames).

    // Twilight Sentry
    SPELL_TWILIGHT_RIFT     = 93010, // Trigg 93019 dmg spell every 3 secs. - 93019 needs 30 yard radius.
    SPELL_SUM_TW_RIFT_ORBS  = 93017, // Summon some weird orbs.

    // Unstable Twilight
    SPELL_UNSTABLE_TWILIGHT = 86305, // Needs 8 y radius.
    SPELL_UNSTABLE_TWILIGHT_TRIGGER = 86302,

    // Dazzling Destruction Stalker
    SPELL_DAZZLING_VISUAL   = 86383,

    // Fabulous Flames
    SPELL_FABULOUS_VISUAL    = 86506,
};

enum Events
{
    /***** GENERAL *****/
    EVENT_BERSERK = 1,

    /***** VALIONA *****/
    EVENT_DEVOURING_FLAMES,
    EVENT_DEVOURING_FLAMES_END,
    EVENT_BLACKOUT,

    EVENT_TWILIGHT_METEOR,
    EVENT_DEEP_BREATH, // Move 10 sec
    EVENT_BREATH_DUMMY, // Spit flames!

    EVENT_BREATH_1, // Dummy, move, place the shit. 3 times.
    EVENT_BREATH_2,
    EVENT_BREATH_3,

    EVENT_MOVE_BREATH_1,
    EVENT_MOVE_BREATH_2,

    EVENT_SUMMON_MOBS_1,
    EVENT_SUMMON_MOBS_2,
    EVENT_SUMMON_MOBS_3,

    EVENT_SUMMON_PORTAL,

    /***** THERALION *****/
    EVENT_ENGULFING_MAGIC,
    EVENT_FABULOUS_FLAMES,

    EVENT_TWILIGHT_BLAST,
    EVENT_DAZZLING_DESTRUCTION,

    /***** MISC *****/

    EVENT_TWILIGHT_TANK_SHIFT,

    // Ground phase
    EVENT_LIFTOFF,
    EVENT_FLIGHT, // unused solo
    EVENT_AIR, // unused solo

    // Air phase
    EVENT_LAND,
    EVENT_RETURN,
    EVENT_GROUND,

    // Mobs
    EVENT_DESPAWN,
    EVENT_TWILIGHT_RIFT_SUMMON,
    EVENT_TWILIGHT_RIFT,
    EVENT_UNSTABLE_TWILIGHT,
    EVENT_COLLAPSING_PORTAL
};

/*
Valiona Phase: Valiona is on the ground and Theralion is in the air.
Theralion Phase: Theralion is on the ground, and Valiona is in the air.
Valiona Phase lasts 100 seconds, Theralion Phase lasts 140 seconds.
*/

enum Phases
{
    PHASE_ALL = 0,
    PHASE_GROUND,
    PHASE_FLIGHT,
    GROUP_GROUND,
    GROUP_FLIGHT
};

enum Creatures
{
    NPC_TWILIGHT_SENTRY         = 50008, // T. Realm, HEROIC ONLY!
    NPC_DAZZ_DESTRUCTION_STALKER = 46374,
    NPC_UNSTABLE_TWILIGHT       = 46304,
    NPC_TWILIGHT_RIFT           = 50014,
    NPC_COLLAPSING_TW_PORTAL    = 46301,
    NPC_TWILIGHT_FLAME          = 40718,
    NPC_DEVOURING_FLAME_TRACKER = 46588,
    NPC_TWILIGHT_ZONE           = 42844,
};

enum PhaseMaskP
{
    PHASEMASK_NM = 1,
    PHASEMASK_SHADOW = 290,
    PHASEMASK_BOTH_SHADOW_NM = 291,
};

Position const TwilFlamePos[90] = // 15 per row, 2 rows per side, 3 sides.
{
// 1-30 for entrance, first breath.
{-726.185f, -768.217f, 836.6880f},
{-712.927f, -759.297f, 836.688f},
{-729.71f, -750.193f, 836.693f},
{-718.571f, -735.144f, 836.696f},
{-731.519f, -729.622f, 835.106f},
{-720.204f, -715.33f, 832.281f},
{-718.553f, -702.101f, 831.897f},
{-718.308f, -688.416f, 831.891f},
{-719.734f, -676.143f, 831.892f},
{-721.456f, -661.333f, 831.897f},
{-722.821f, -649.582f, 833.176f},
{-719.32f, -637.6f, 836.699f},
{-723.196f, -622.605f, 836.692f},
{-716.657f, -611.672f, 836.688f},
{-715.665f, -600.11f, 836.685f},
{-722.079f, -618.628f, 836.69f},
{-709.057f, -668.355f, 831.899f},
{-707.799f, -701.244f, 831.899f},
{-711.942f, -719.573f, 834.601f},
{-730.262f, -715.065f, 831.901f},
{-731.211f, -703.507f, 831.896f},
{-731.271f, -696.279f, 831.893f},
{-731.779f, -684.027f, 831.889f},
{-730.221f, -670.001f, 831.894f},
{-731.618f, -660.124f, 831.897f},
{-733.132f, -649.415f, 832.364f},
{-733.703f, -636.535f, 835.993f},
{-729.895f, -622.403f, 836.696f},
{-727.821f, -609.092f, 836.692f},
{-725.244f, -632.255f, 836.699f},
// 30-60 for mid
{-738.228f, -774.297f, 836.687f},
{-747.187f, -711.44f, 831.897f},
{-746.532f, -691.806f, 831.891f},
{-746.461f, -681.971f, 831.89f},
{-746.909f, -771.366f, 836.687f},
{-746.978f, -668.307f, 831.893f},
{-746.207f, -646.622f, 833.052f},
{-750.661f, -735.852f, 836.699f},
{-744.774f, -629.426f, 836.697f},
{-740.922f, -601.492f, 836.688f},
{-734.556f, -756.46f, 836.693f},
{-749.818f, -751.318f, 836.694f},
{-742.449f, -759.696f, 836.692f},
{-741.821f, -746.749f, 836.698f},
{-742.275f, -735.698f, 836.699f},
{-739.995f, -725.004f, 833.588f},
{-741.421f, -713.62f, 831.898f},
{-740.577f, -702.188f, 831.893f},
{-739.815f, -691.874f, 831.891f},
{-739.103f, -682.223f, 831.89f},
{-738.46f, -673.514f, 831.893f},
{-737.876f, -665.608f, 831.896f},
{-737.234f, -656.916f, 831.899f},
{-736.475f, -646.637f, 833.048f},
{-735.778f, -637.195f, 835.802f},
{-735.17f, -628.958f, 836.697f},
{-734.6f, -621.244f, 836.695f},
{-749.068f, -724.66f, 833.581f},
{-732.867f, -608.092f, 836.691f},
{-732.764f, -596.374f, 836.687f},
// 60-90 for exit
{-766.844f, -614.003f, 836.689f},
{-751.849f, -608.325f, 836.689f},
{-755.589f, -627.736f, 836.695f},
{-765.266f, -635.142f, 836.698f},
{-755.655f, -640.214f, 835.515f},
{-763.859f, -648.017f, 834.242f},
{-774.417f, -650.346f, 835.237f},
{-767.926f, -656.699f, 832.59f},
{-756.128f, -657.912f, 831.899f},
{-749.862f, -670.31f, 831.896f},
{-759.005f, -666.729f, 831.897f},
{-768.303f, -664.93f, 831.897f},
{-767.196f, -675.072f, 831.892f},
{-754.611f, -678.247f, 831.891f},
{-754.229f, -690.564f, 831.891f},
{-760.611f, -683.775f, 831.889f},
{-774.04f, -682.983f, 831.89f},
{-781.804f, -686.498f, 831.898f},
{-770.858f, -691.417f, 831.893f},
{-761.664f, -694.862f, 831.893f},
{-752.508f, -699.213f, 831.894f},
{-760.785f, -706.285f, 831.898f},
{-771.937f, -701.638f, 831.899f},
{-770.358f, -714.257f, 833.268f},
{-756.803f, -721.699f, 833.392f},
{-773.318f, -730.829f, 836.696f},
{-756.708f, -737.369f, 836.696f},
{-756.39f, -753.955f, 836.691f},
{-759.558f, -765.119f, 836.686f},
{-768.016f, -756.87f, 836.686f}
};

struct Position const portalPos [32] =
{
    { -719.562f, -666.192f, 831.895f, 2.02262f },
    { -714.213f, -683.996f, 831.89f, 1.23722f },
    { -692.711f, -685.066f, 834.69f , 0.0751219f },
    { -737.404f, -729.797f, 834.987f, 2.64511f },
    { -729.6f, -693.74f, 831.893f, 1.29422f },
    { -717.303f, -648.152f, 834.284f, 1.32171f },
    { -729.695f, -600.076f, 836.687f, 4.61052f },
    { -757.238f, -607.654f, 836.688f, 5.12495f },
    { -726.226f, -618.618f, 836.692f, 1.47678f },
    { -738.251f, -671.169f, 831.892f, 2.05012f },
    { -736.309f, -652.2f, 831.899f, 1.42965f },
    { -768.243f, -717.22f, 833.717f, 1.09586f },
    { -745.573f, -690.878f, 831.892f, 1.48856f },
    { -754.572f, -747.532f, 836.692f, 1.44143f },
    { -757.394f, -737.542f, 836.695f, 2.702f },
    { -731.541f, -764.063f, 836.69f, 3.97827f },
    { -748.1f, -765.09f, 836.688f, 2.58812f },
    { -766.265f, -767.635f, 836.685f, 4.04226f },
    { -722.399f, -719.888f, 833.132f, 5.34486f },
    { -725.394f, -742.52f, 836.696f, 4.11964f },
    { -711.445f, -609.107f, 836.686f, 6.09607f },
    { -741.459f, -632.174f, 836.699f, 0.657184f },
    { -772.302f, -641.23f, 836.699f, 1.2855f },
    { -784.96f, -686.984f, 831.897f, 2.87201f },
    { -742.531f, -709.88f, 831.896f, 6.23236f },
    { -726.602f, -637.284f, 836.327f, 1.96764f },
    { -744.503f, -620.119f, 836.697f, 2.65094f },
    { -757.452f, -631.196f, 836.695f, 4.64978f },
    { -758.898f, -650.326f, 832.936f, 4.24923f },
    { -763.597f, -666.212f, 831.898f, 5.94176f },
    { -757.706f, -680.829f, 831.891f, 3.73086f },
    { -764.423f, -694.156f, 831.892f, 5.80039f }
};

// ShadowPhaseCheck
class ShadowPhaseCheck : public std::unary_function<Unit*, bool>
{
public:
    ShadowPhaseCheck(bool nonTank, Unit *boss) : _nonTank(nonTank), _boss(boss)
    {
    }

    bool operator()(Unit* unit) const
    {
        if (!unit || unit->GetTypeId() != TYPEID_PLAYER || unit->GetPhaseMask() == PHASEMASK_SHADOW)
            return false;
        if (_nonTank && unit == _boss->getVictim())
            return false;
        return true;
    }
private :
    bool _nonTank;
    Unit *_boss;
};

class FabulousFlameCheck : public std::unary_function<Unit*, bool>
{
public:
    FabulousFlameCheck(bool nonTank, Unit *boss) : _nonTank(nonTank), _boss(boss)
    {
    }

    bool operator()(Unit* unit) const
    {
        if (!unit || unit->GetTypeId() != TYPEID_PLAYER || unit->GetPhaseMask() == PHASEMASK_SHADOW)
            return false;
        if (_nonTank && unit == _boss->getVictim())
            return false;
        if (_boss->GetDistance2d(unit->GetPositionX(), unit->GetPositionY()) < 15.0f)
            return false;
        return true;
    }
private :
    bool _nonTank;
    Unit *_boss;
};

class boss_valiona_bot : public CreatureScript
{
public:
    boss_valiona_bot() : CreatureScript("boss_valiona_bot") { }

    struct boss_valiona_botAI : public BossAI
    {
        boss_valiona_botAI(Creature* creature) : BossAI(creature, DATA_VALIONA_THERALION)
        {
            me->ApplySpellImmune(0, IMMUNITY_ID, RAID_MODE(86202, 92889, 92890, 92891), true);
        }

        void Reset()
        {
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TWIL_PROTECT_BUFF);
            instance->DoRemoveAurasDueToSpellOnPlayers(RAID_MODE(86202, 92889, 92890, 92891));
            instance->DoRemoveAurasDueToSpellOnPlayers(RAID_MODE(88436, 92892, 92893, 92894));
            if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE)
                me->GetMotionMaster()->MovementExpired();
            instance->SetBossState(DATA_VALIONA_THERALION, NOT_STARTED);
            me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, 0x02);
            me->RemoveUnitMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_FLYING);
            breathSequenz[0] = 0;
            breathSequenz[1] = 1;
            breathSequenz[2] = 2;
            if (IsHeroic())
                me->AddAura(SPELL_TWILIGHT_SHIFT_A, me);
            me->RemoveAurasDueToSpell(SPELL_TWILIGHT_SENTRY_SUMMON_PERDIODIC);
            me->SetPhaseMask(PHASEMASK_BOTH_SHADOW_NM, true);
            DespawnCreatures(NPC_TWILIGHT_SENTRY, 200.0f);
            DespawnCreatures(NPC_TWILIGHT_RIFT, 200.0f);
            _Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            me->LowerPlayerDamageReq(me->GetMaxHealth());
            EnterPhaseGround();
            events.ScheduleEvent(EVENT_SUMMON_PORTAL, 30000, 0, PHASE_ALL);
            events.ScheduleEvent(EVENT_BERSERK, 600000, PHASE_ALL);
            instance->SetBossState(DATA_VALIONA_THERALION, IN_PROGRESS);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me); // Add
            DoCast(me, SPELL_SUMMON_PORTAL, true);
            if (Creature* Theralion = Creature::GetCreature(*me, instance->GetData64(NPC_THERALION_BOT)))
                if (Theralion && !Theralion->isInCombat())
                    Theralion->SetInCombatWithZone();
            if (IsHeroic())
            {
                me->AddAura(SPELL_TWILIGHT_SENTRY_SUMMON_PERDIODIC, me);
                if (Creature *twilightZone = me->FindNearestCreature(NPC_TWILIGHT_ZONE, 200, true))
                    twilightZone->AddAura(SPELL_TWILIGHT_ZONE_AUR, twilightZone);
            }
            SetUnstableInCombat();
            _EnterCombat();
        }

        void DespawnCreatures(uint32 entry, float distance)
        {
            std::list<Creature*> creatures;
            GetCreatureListWithEntryInGrid(creatures, me, entry, distance);

            if (creatures.empty())
                return;

            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                (*iter)->DespawnOrUnsummon();
        }

        void SetUnstableInCombat()
        {
            std::list<Creature*> creatures;
            me->GetCreatureListWithEntryInGrid(creatures, NPC_UNSTABLE_TWILIGHT, 200.0f);

            if (creatures.empty())
               return;

            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                (*iter)->SetInCombatWithZone(); // Set Unstable Twilight orbs in combat.
        }

        void initEvents(bool onGround = true)
        {
            events.CancelEventGroup(onGround ? GROUP_FLIGHT : GROUP_GROUND);
            if (onGround)
            {
                events.ScheduleEvent(EVENT_BLACKOUT, 10000, GROUP_GROUND, PHASE_GROUND); // 40 sec cd
                events.ScheduleEvent(EVENT_DEVOURING_FLAMES, 25500, GROUP_GROUND, PHASE_GROUND); // 40 sec cd
                events.ScheduleEvent(EVENT_LIFTOFF, 100000, GROUP_GROUND, PHASE_GROUND);
            }
            else
            {
                events.ScheduleEvent(EVENT_TWILIGHT_METEOR, 4000, GROUP_FLIGHT, PHASE_FLIGHT); // 15 times, every 6 secs.
                events.ScheduleEvent(EVENT_DEEP_BREATH, 83000, GROUP_FLIGHT, PHASE_FLIGHT); // 10 sec to get there, lasts 45 seconds, 15 per flight on a part, 3 parts : entrance, middle, exit.
                events.ScheduleEvent(EVENT_LAND, 138000, GROUP_FLIGHT, PHASE_FLIGHT);
            }
        }

        void EnterEvadeMode()
        {
            Reset();
            me->GetMotionMaster()->MoveTargetedHome();
            instance->SetBossState(DATA_VALIONA_THERALION, FAIL);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me); // Remove
            _EnterEvadeMode();
        }

        void EnterPhaseGround()
        {
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, 0x02);
            me->RemoveUnitMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_FLYING);
            events.SetPhase(PHASE_GROUND);
            initEvents(true);
        }

        void EnterPhaseAir()
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, 0x02);
            me->AddUnitMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_FLYING);
            events.SetPhase(PHASE_FLIGHT);
            initEvents(false);
        }

        void JustSummoned(Creature* summon)
        {
            switch (summon->GetEntry())
            {
                case NPC_DEVOURING_FLAME_TRACKER:
                {
                    me->CastSpell(summon, SPELL_DEVOURING_FLAMES, false);
                    break;
                }
                case NPC_COLLAPSING_TW_PORTAL:
                {
                    summon->CastSpell(summon, SPELL_TWILIGHT_PORTAL_VISUAL, true);
                    break;
                }
                default:
                {
                    summon->setActive(true);
                    if(me->isInCombat())
                        summon->AI()->DoZoneInCombat();
                    break;
                }
            }
            summons.Summon(summon);
        }

        void JustDied(Unit* killer)
        {
            Talk(SAY_VA_DEATH);
            me->SetLootRecipient(killer);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TWIL_PROTECT_BUFF);
            instance->DoRemoveAurasDueToSpellOnPlayers(RAID_MODE(86202, 92889, 92890, 92891));
            instance->DoRemoveAurasDueToSpellOnPlayers(RAID_MODE(88436, 92892, 92893, 92894));
            instance->SetBossState(DATA_VALIONA_THERALION, IsHeroic() ? DONE_HM : DONE);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            DespawnCreatures(NPC_TWILIGHT_SENTRY, 200.0f);
            DespawnCreatures(NPC_TWILIGHT_RIFT, 200.0f);
            _JustDied();
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_VA_KILL);
        }

        void DamageTaken(Unit* who, uint32& damage)
        {
            Creature* theralion = Creature::GetCreature(*me, instance->GetData64(NPC_THERALION_BOT));
            if (!theralion)
                return;
            if (who && who->GetPhaseMask() == PHASEMASK_SHADOW)
                damage = 0;
            if (damage > me->GetHealth())
            {
                who->Kill(me);
                who->Kill(theralion);
                me->LowerPlayerDamageReq(me->GetMaxHealth());
                me->SetLootRecipient(who);
                theralion->LowerPlayerDamageReq(theralion->GetMaxHealth());
                theralion->SetLootRecipient(who);
                damage = 0;
            }
            else if (theralion->isAlive())
                theralion->SetHealth(theralion->GetHealth() - damage);
        }

        void chooseBreathPoint(int point, bool isNear, float minDist = 40.0f)
        {
            switch (point)
            {
            case 0:
                if ((!isNear && me->GetDistance2d(-725.077f, -613.762f) > minDist) || (isNear && me->GetDistance2d(-725.077f, -613.762f) < minDist))
                    me->GetMotionMaster()->MovePoint(1, -725.077f, -613.762f, me->GetPositionZ());
                else
                    me->GetMotionMaster()->MovePoint(1, -723.525f, -769.260f, me->GetPositionZ());
                break;
            case 1:
                if ((!isNear && me->GetDistance2d(-740.447f, -612.804f) > minDist) || (isNear && me->GetDistance2d(-740.447f, -612.804f) < minDist))
                    me->GetMotionMaster()->MovePoint(1, -740.447f, -612.804f, me->GetPositionZ());
                else
                    me->GetMotionMaster()->MovePoint(1, -738.849f, -769.072f, me->GetPositionZ());
                break;
            case 2:
                if ((!isNear && me->GetDistance2d(-763.181f, -626.995f) > minDist) || (isNear && me->GetDistance2d(-763.181f, -626.995f) < minDist))
                    me->GetMotionMaster()->MovePoint(1, -763.181f, -626.995f, me->GetPositionZ());
                else
                    me->GetMotionMaster()->MovePoint(1, -757.691f, -766.305f, me->GetPositionZ());
                break;
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
                    case EVENT_LIFTOFF:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LIFTOFF);
                        me->SetDisableGravity(true);
                        events.ScheduleEvent(EVENT_FLIGHT, 1500, GROUP_GROUND, PHASE_GROUND);
                        events.ScheduleEvent(EVENT_AIR, 2000, GROUP_GROUND, PHASE_GROUND);
                        break;
                    case EVENT_FLIGHT:
                         me->SetReactState(REACT_PASSIVE);
                         me->AttackStop();
                         me->GetMotionMaster()->MovePoint(1, -741.065f, -684.987f, me->GetPositionZ() + 25.0f);
                         break;
                    case EVENT_AIR:
                        EnterPhaseAir();
                        break;
                    case EVENT_BLACKOUT:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, ShadowPhaseCheck(true, me)))
                            DoCast(target, SPELL_BLACKOUT);
                        Talk(SAY_VA_BOUT);
                        events.ScheduleEvent(EVENT_BLACKOUT, 40000, GROUP_GROUND, PHASE_GROUND);
                        break;
                    case EVENT_DEVOURING_FLAMES:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, ShadowPhaseCheck(true, me)))
                        {
                            if (!target)
                                target = SelectTarget(SELECT_TARGET_RANDOM, 0, ShadowPhaseCheck(false, me));

                            if (target)
                                target->CastSpell(target, SPELL_DEVOURING_FLAME_TRACKER_SUMMON, true);
                        }
                        events.ScheduleEvent(EVENT_DEVOURING_FLAMES, 40000, GROUP_GROUND, PHASE_GROUND);
                        break;
                    }
                    case EVENT_LAND:
                        me->SetSpeed(MOVE_WALK, 1.0f, true);
                        me->SetSpeed(MOVE_RUN, 1.0f, true);
                        me->SetSpeed(MOVE_FLIGHT, 1.0f, true);
                        events.CancelEvent(EVENT_BREATH_DUMMY); // Cancel the breaths.
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
                        me->SetDisableGravity(false);
                        events.ScheduleEvent(EVENT_RETURN, 1000, GROUP_FLIGHT, PHASE_FLIGHT);
                        events.ScheduleEvent(EVENT_GROUND, 1500, GROUP_FLIGHT, PHASE_FLIGHT);
                        break;
                    case EVENT_RETURN:
                        me->SetReactState(REACT_PASSIVE);
                        me->AttackStop();
                        me->GetMotionMaster()->MovePoint(1, -741.065f, -684.987f, me->GetPositionZ() - 25.0f);
                        break;
                    case EVENT_GROUND:
                        EnterPhaseGround();
                        break;
                    case EVENT_TWILIGHT_METEOR:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, ShadowPhaseCheck(false, me)))
                        {
                            me->AddAura(SPELL_TW_METEOR_TARGET, target);
                            DoCast(target, SPELL_TW_METEOR);
                        }
                        events.ScheduleEvent(EVENT_TWILIGHT_METEOR, 6100, GROUP_FLIGHT, PHASE_FLIGHT);
                        break;
                    case EVENT_DEEP_BREATH:
                        Talk(SAY_VA_DB);
                        me->SetSpeed(MOVE_WALK, 2.7f, true);
                        me->SetSpeed(MOVE_RUN, 2.7f, true);
                        me->SetSpeed(MOVE_FLIGHT, 2.7f, true);
                        events.CancelEvent(EVENT_TWILIGHT_METEOR);
                        std::random_shuffle(breathSequenz, breathSequenz + 3);
                        switch (breathSequenz[0])
                        {
                            case 0:
                                me->GetMotionMaster()->MovePoint(1, -723.525f, -769.260f, me->GetPositionZ());
                                break;
                            case 1:
                                me->GetMotionMaster()->MovePoint(1, -740.447f, -612.804f, me->GetPositionZ());
                                break;
                            case 2:
                                me->GetMotionMaster()->MovePoint(1, -757.691f, -766.305f, me->GetPositionZ());
                                break;
                        }
                        events.ScheduleEvent(EVENT_BREATH_1, 10000, GROUP_FLIGHT, PHASE_FLIGHT);
                        break;
                    case EVENT_BREATH_1:
                    {
                        chooseBreathPoint(breathSequenz[0], false);
                        events.ScheduleEvent(EVENT_BREATH_DUMMY, 3000, GROUP_FLIGHT, PHASE_FLIGHT); // lasts 3 sec.
                        pointFlame.clear();
                        int min = 1, max = 1;
                        switch (breathSequenz[0])
                        {
                            case 0:
                                min = 1;
                                max = 31;
                                break;
                            case 1:
                                min = 31;
                                max = 61;
                                break;
                            case 2:
                                min = 61;
                                max = 91;
                                break;
                        }
                        for (int i = min; i < max; i++)
                        {
                            Position point = TwilFlamePos[i];
                            pointFlame.push_back(point);
                        }
                        events.ScheduleEvent(EVENT_SUMMON_MOBS_1, 0, GROUP_FLIGHT, PHASE_FLIGHT); // lasts 3 sec.
                        events.ScheduleEvent(EVENT_MOVE_BREATH_1, 11000, GROUP_FLIGHT, PHASE_FLIGHT); // must move in 13000 long side-side.
                        summonFlameCount = 1;
                        break;
                    }
                    case EVENT_MOVE_BREATH_1:
                    {
                        chooseBreathPoint(breathSequenz[1], true);
                        events.ScheduleEvent(EVENT_BREATH_2, 4000, GROUP_FLIGHT, PHASE_FLIGHT);
                        break;
                    }
                    case EVENT_BREATH_2:
                    {
                        // -738.228f, -774.297f, 836.687f
                        chooseBreathPoint(breathSequenz[1], false);
                        pointFlame.clear();
                        int min = 1, max = 1;
                        switch (breathSequenz[1])
                        {
                            case 0:
                                min = 1;
                                max = 31;
                                break;
                            case 1:
                                min = 31;
                                max = 61;
                                break;
                            case 2:
                                min = 61;
                                max = 91;
                                break;
                        }
                        for (int i = min; i < max; i++)
                        {
                            Position point = TwilFlamePos[i];
                            pointFlame.push_back(point);
                        }
                        summonFlameCount = 31;
                        events.ScheduleEvent(EVENT_SUMMON_MOBS_2, 0, GROUP_FLIGHT, PHASE_FLIGHT); // lasts 3 sec.
                        events.ScheduleEvent(EVENT_MOVE_BREATH_2, 11000, GROUP_FLIGHT, PHASE_FLIGHT); // must move in 13000 long side-side.
                        break;
                    }
                    case EVENT_MOVE_BREATH_2:
                        chooseBreathPoint(breathSequenz[2], true);
                        events.ScheduleEvent(EVENT_BREATH_3, 4000, GROUP_FLIGHT, PHASE_FLIGHT);
                        break;
                    case EVENT_BREATH_3:
                    {
                        chooseBreathPoint(breathSequenz[2], false);
                        summonFlameCount = 61;
                        pointFlame.clear();
                        int min = 1, max = 1;
                        switch (breathSequenz[2])
                        {
                            case 0:
                                min = 1;
                                max = 31;
                                break;
                            case 1:
                                min = 31;
                                max = 61;
                                break;
                            case 2:
                                min = 61;
                                max = 91;
                                break;
                        }
                        for (int i = min; i < max; i++)
                        {
                            Position point = TwilFlamePos[i];
                            pointFlame.push_back(point);
                        }
                        break;
                    }
                    case EVENT_SUMMON_MOBS_1:
                    {
                        for (std::list<Position >::iterator itr = pointFlame.begin(); itr != pointFlame.end(); )
                        {
                            if (me->GetDistance2d(itr->GetPositionX(), itr->GetPositionY()) < 15.0f)
                            {
                                me->SummonCreature(NPC_TWILIGHT_FLAME, itr->GetPositionX(), itr->GetPositionY(), itr->GetPositionZ(),TEMPSUMMON_CORPSE_DESPAWN);
                                summonFlameCount++;
                                itr = pointFlame.erase(itr);
                            }
                            else
                                ++itr;
                        }
                        if (summonFlameCount < 90)
                            events.ScheduleEvent(EVENT_SUMMON_MOBS_1, 100, GROUP_FLIGHT, PHASE_FLIGHT);
                        break;
                    }
                    case EVENT_BREATH_DUMMY:
                        DoCast(me, SPELL_DEEP_BREATH_DUMMY);
                        events.ScheduleEvent(EVENT_BREATH_DUMMY, 3000, GROUP_FLIGHT, PHASE_FLIGHT);
                        break;
                    case EVENT_SUMMON_PORTAL:
                        for(int i = 0; i < 2; i++)
                            DoCast(me, SPELL_SUMMON_PORTAL, true);
                        events.ScheduleEvent(EVENT_SUMMON_PORTAL, 61000, PHASE_ALL);
                        break;
                    case EVENT_BERSERK:
                        DoCast(me, SPELL_BERSERK, true);
                        if (Creature* Theralion = Creature::GetCreature(*me, instance->GetData64(NPC_THERALION_BOT)))
                            Theralion->CastSpell(Theralion, SPELL_BERSERK, true);
                        break;
                }
            }
            if (events.IsInPhase(PHASE_GROUND))
                DoMeleeAttackIfReady();
        }

    private:
        Unit* meteorTarget;
        uint32 summonFlameCount;
        std::list<Position > pointFlame;
        int breathSequenz[3];
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_valiona_botAI(creature);
    }
};

class boss_theralion_bot : public CreatureScript
{
public:
    boss_theralion_bot() : CreatureScript("boss_theralion_bot") { }

    struct boss_theralion_botAI : public BossAI
    {
        boss_theralion_botAI(Creature* creature) : BossAI(creature, DATA_VALIONA_THERALION)
        {
            me->ApplySpellImmune(0, IMMUNITY_ID, RAID_MODE(86202, 92889, 92890, 92891), true);
        }

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE)
                me->GetMotionMaster()->MovementExpired();

            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, 0x02);
            me->RemoveUnitMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_FLYING);
            uiDazzlingDestructionCount = 0;
            me->RemoveAurasDueToSpell(SPELL_TWILIGHT_SENTRY_SUMMON_PERDIODIC);
            if (IsHeroic())
                me->AddAura(SPELL_TWILIGHT_SHIFT_A, me);
            me->SetPhaseMask(PHASEMASK_BOTH_SHADOW_NM, true);
            _Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            me->LowerPlayerDamageReq(me->GetMaxHealth());
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me); // Add
            uiDazzlingDestructionCount = 0;
            if (Creature* Valiona = Creature::GetCreature(*me, instance->GetData64(NPC_VALIONA_BOT)))
                if(Valiona && !Valiona->isInCombat())
                    Valiona->SetInCombatWithZone();

            events.SetPhase(PHASE_GROUND);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            events.ScheduleEvent(EVENT_LIFTOFF, 1000, GROUP_GROUND, PHASE_GROUND);
            _EnterCombat();
        }

        void initEvents(bool onGround = true)
        {
            events.CancelEventGroup(onGround ? GROUP_FLIGHT : GROUP_GROUND);

            if(onGround)
            {
                events.ScheduleEvent(EVENT_FABULOUS_FLAMES, 7000, GROUP_GROUND, PHASE_GROUND); // every 15 sec
                events.ScheduleEvent(EVENT_ENGULFING_MAGIC, 15000, GROUP_GROUND, PHASE_GROUND); // 30 sec cooldown
                events.ScheduleEvent(EVENT_LIFTOFF, 138500, GROUP_GROUND, PHASE_GROUND);
            }else
            {
                events.ScheduleEvent(EVENT_TWILIGHT_BLAST, 5000, GROUP_FLIGHT, PHASE_FLIGHT);
                events.ScheduleEvent(EVENT_DAZZLING_DESTRUCTION, 85000, GROUP_FLIGHT, PHASE_FLIGHT);
                events.ScheduleEvent(EVENT_LAND, 100000, GROUP_FLIGHT, PHASE_FLIGHT);
            }
        }

        void EnterEvadeMode()
        {
            Reset();
            me->GetMotionMaster()->MoveTargetedHome();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me); // Remove
            _EnterEvadeMode();
        }

        void EnterPhaseGround()
        {
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, 0x02);
            me->RemoveUnitMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_FLYING);
            events.SetPhase(PHASE_GROUND);
            initEvents(true);
        }

        void EnterPhaseAir()
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, 0x02);
            me->AddUnitMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_FLYING);
            events.SetPhase(PHASE_FLIGHT);
            initEvents(false);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            if (me->isInCombat())
               summon->AI()->DoZoneInCombat();
        }

        void JustDied(Unit* killer)
        {
            Talk(SAY_TH_DEATH);
            me->SetLootRecipient(killer);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me); // Remove
            _JustDied();
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_TH_KILL);
        }

        void DamageTaken(Unit* who, uint32& damage)
        {
            Creature* valiona = Creature::GetCreature(*me, instance->GetData64(NPC_VALIONA_BOT));
            if (!valiona)
                return;
            if (who && who->GetPhaseMask() == PHASEMASK_SHADOW)
                damage = 0;
            if (damage > me->GetHealth())
            {
                who->Kill(me);
                who->Kill(valiona);
                me->LowerPlayerDamageReq(me->GetMaxHealth());
                me->SetLootRecipient(who);
                valiona->LowerPlayerDamageReq(valiona->GetMaxHealth());
                valiona->SetLootRecipient(who);
                damage = 0;
            }
            else if (valiona->isAlive())
                valiona->SetHealth(valiona->GetHealth() - damage);
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
                    case EVENT_LIFTOFF:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LIFTOFF);
                        me->SetDisableGravity(true);
                        events.ScheduleEvent(EVENT_FLIGHT, 1500, GROUP_GROUND, PHASE_GROUND);
                        events.ScheduleEvent(EVENT_AIR, 2000, GROUP_GROUND, PHASE_GROUND);
                        break;
                    case EVENT_FLIGHT:
                        me->SetReactState(REACT_PASSIVE);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                        me->AttackStop();
                        me->GetMotionMaster()->MovePoint(1, -741.065f, -684.987f, me->GetPositionZ() + 25.0f);
                        break;
                    case EVENT_AIR:
                        EnterPhaseAir();
                        break;
                    case EVENT_ENGULFING_MAGIC:
                        Talk(SAY_TH_ENGF);
                        me->CastSpell(me, SPELL_ENGULFING_MAGIC, true);
                        events.ScheduleEvent(EVENT_ENGULFING_MAGIC, 35000, GROUP_GROUND, PHASE_GROUND);
                        break;
                    case EVENT_FABULOUS_FLAMES:
                    {
                        Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, FabulousFlameCheck(true, me));
                        if (!target)
                            target = SelectTarget(SELECT_TARGET_RANDOM, 0, ShadowPhaseCheck(true, me));
                        if (target)
                            DoCast(target, SPELL_FABULOUS_FLAMES);

                        events.ScheduleEvent(EVENT_FABULOUS_FLAMES, 15000, GROUP_GROUND, PHASE_GROUND);
                        break;
                    }
                    case EVENT_LAND:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
                        me->SetDisableGravity(false);
                        events.ScheduleEvent(EVENT_RETURN, 1000, GROUP_FLIGHT, PHASE_FLIGHT);
                        events.ScheduleEvent(EVENT_GROUND, 1500, GROUP_FLIGHT, PHASE_FLIGHT);
                        break;
                    case EVENT_RETURN:
                        me->SetReactState(REACT_PASSIVE);
                        me->AttackStop();
                        me->GetMotionMaster()->MovePoint(1, -741.065f, -684.987f, me->GetPositionZ() - 25.0f);
                        break;
                    case EVENT_GROUND:
                        EnterPhaseGround();
                        break;
                    case EVENT_TWILIGHT_BLAST:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, ShadowPhaseCheck(false, me)))
                            DoCast(target, SPELL_TWILIGHT_BLAST);
                        events.ScheduleEvent(EVENT_TWILIGHT_BLAST, urand(2000, 3000), GROUP_FLIGHT, PHASE_FLIGHT);
                        break;
                    case EVENT_DAZZLING_DESTRUCTION: // 2 sec timer, summon 6 then cast.
                    {
                        Talk(RAND(SAY_TH_DAZZ, SAY_TH_DAZZ_2));
                        events.CancelEvent(EVENT_TWILIGHT_BLAST);

                        if (uiDazzlingDestructionCount < MAX_DAZZLING_DESTRUCTION_CASTS)
                        {
                            Unit* Target1 = SelectTarget(SELECT_TARGET_RANDOM, 0, ShadowPhaseCheck(false, me));
                            Unit* Target2 = SelectTarget(SELECT_TARGET_RANDOM, 0, ShadowPhaseCheck(false, me));

                            if (Target1 == Target2)
                                Target2 = SelectTarget(SELECT_TARGET_RANDOM, 0, ShadowPhaseCheck(false, me));

                            if (Target1 && Target1->isAlive())
                                Target1->CastSpell(Target1->GetPositionX(), Target1->GetPositionY(), Target1->GetPositionZ(), SPELL_DAZZ_DESTRUCTION_SUMMON, true); // summon spell is casted by unit's target
                            if (Target2 && Target2->isAlive())
                                Target2->CastSpell(Target2->GetPositionX(), Target2->GetPositionY(), Target2->GetPositionZ(), SPELL_DAZZ_DESTRUCTION_SUMMON, true);

                            DoCast(me, SPELL_DAZZ_DESTRUCTION_DUMMY);
                            uiDazzlingDestructionCount++;

                            events.ScheduleEvent(EVENT_DAZZLING_DESTRUCTION, 5000, GROUP_FLIGHT, PHASE_FLIGHT);
                        }
                        else
                        {
                            events.CancelEvent(EVENT_DAZZLING_DESTRUCTION);
                            uiDazzlingDestructionCount = 0; // Revert to initial count for next phase.
                        }
                    }
                    break;
                }
            }
            if (events.IsInPhase(PHASE_GROUND))
                DoMeleeAttackIfReady();
        }

    private:
        uint32 uiDazzlingDestructionCount;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_theralion_botAI(creature);
    }
};

class npc_dazzling_visual : public CreatureScript // 46374
{
public:
    npc_dazzling_visual() : CreatureScript("npc_dazzling_visual") { }

    struct npc_dazzling_visualAI : public ScriptedAI
    {
        npc_dazzling_visualAI(Creature* creature) : ScriptedAI(creature)
        {
            creature->SetReactState(REACT_PASSIVE);
            instance = creature->GetInstanceScript();
            me->SetPhaseMask(PHASEMASK_BOTH_SHADOW_NM, true);
            creature->CastSpell(creature, SPELL_DAZZLING_VISUAL, true);
        }

        void Reset()
        {
            events.ScheduleEvent(EVENT_DESPAWN, 12000);
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_DESPAWN:
                        me->DespawnOrUnsummon();
                        return;
                }
            }
        }
    private:
        InstanceScript* instance;
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_dazzling_visualAI (creature);
    }
};

class npc_fabulous_flames : public CreatureScript // 46448
{
public:
    npc_fabulous_flames() : CreatureScript("npc_fabulous_flames") { }

    struct npc_fabulous_flamesAI : public ScriptedAI
    {
        npc_fabulous_flamesAI(Creature* creature) : ScriptedAI(creature)
        {
            creature->SetReactState(REACT_PASSIVE);
            instance = creature->GetInstanceScript();
            creature->CastSpell(creature, SPELL_FABULOUS_VISUAL, true);
        }

        void Reset()
        {
            events.ScheduleEvent(EVENT_DESPAWN, 45000); // Last 45 sec.
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_DESPAWN:
                        me->DespawnOrUnsummon();
                        return;
                }
            }
        }
    private:
        InstanceScript* instance;
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fabulous_flamesAI (creature);
    }
};

class npc_twilight_sentry : public CreatureScript // 50008 - Heroic. Summons Rift every 15 seconds.
{
public:
    npc_twilight_sentry() : CreatureScript("npc_twilight_sentry") { }

    struct npc_twilight_sentryAI : public ScriptedAI
    {
        npc_twilight_sentryAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
            creature->SetReactState(REACT_PASSIVE);
        }

        void Reset()
        {
            events.ScheduleEvent(EVENT_TWILIGHT_RIFT_SUMMON, 1000);
        }

        void IsSummonedBy(Unit* /*summoner*/)
        {
            me->SetPhaseMask(PHASEMASK_SHADOW, true);
            if (Creature *valiona = Creature::GetCreature(*me, instance->GetData64(NPC_VALIONA_BOT)))
                valiona->AI()->JustSummoned(me);
        }

        void JustDied(Unit* /*killer*/)
        {
            summons.DespawnAll();
        }

        void JustSummoned(Creature* summon)
        {
            summon->SetPhaseMask(PHASEMASK_NM, true);
            if (Creature *valiona = Creature::GetCreature(*me, instance->GetData64(NPC_VALIONA_BOT)))
                valiona->AI()->JustSummoned(summon);
            summons.Summon(summon);
        }

        void UpdateAI(const uint32 diff)
        {
            if (me->HasUnitState(UNIT_STATE_CASTING) || me->HasAura(SPELL_SUM_TW_RIFT_ORBS))
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_TWILIGHT_RIFT_SUMMON:
                        DoCast(SPELL_SUM_TW_RIFT_ORBS);
                        events.ScheduleEvent(EVENT_TWILIGHT_RIFT_SUMMON, 1000);
                        break;
                }
            }
        }
    private:
        InstanceScript* instance;
        EventMap events;
        SummonList summons;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_twilight_sentryAI (creature);
    }
};

class npc_twilight_rift : public CreatureScript // 50014 - Heroic. Spawned in Twilight Realm every 15 seconds.
{
public:
    npc_twilight_rift() : CreatureScript("npc_twilight_rift") { }

    struct npc_twilight_riftAI : public ScriptedAI
    {
        npc_twilight_riftAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            creature->SetReactState(REACT_PASSIVE);
        }

        void Reset()
        {
            events.ScheduleEvent(EVENT_TWILIGHT_RIFT, 3000);
        }

        void UpdateAI(const uint32 diff)
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_TWILIGHT_RIFT:
                        DoCast(me, SPELL_TWILIGHT_RIFT);
                        break;
                }
            }
        }
    private:
        InstanceScript* instance;
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_twilight_riftAI (creature);
    }
};

class npc_twilight_flame : public CreatureScript // 40718. Summ by Valiona on deep breaths.
{
public:
    npc_twilight_flame() : CreatureScript("npc_twilight_flame") { }

    struct npc_twilight_flameAI : public ScriptedAI
    {
        npc_twilight_flameAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            creature->SetReactState(REACT_PASSIVE);
            creature->CastSpell(creature, SPELL_TW_FLAMES_TRIGGER, true);
        }

        void Reset()
        {
            me->SetPhaseMask(PHASEMASK_BOTH_SHADOW_NM, true);
            events.ScheduleEvent(EVENT_DESPAWN, 10000);
        }

        void UpdateAI(const uint32 diff)
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_DESPAWN:
                        me->DespawnOrUnsummon();
                        return;
                }
            }
        }
    private:
        InstanceScript* instance;
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_twilight_flameAI (creature);
    }
};

class npc_unstable_twilight_portal : public CreatureScript
{
public:
    npc_unstable_twilight_portal() : CreatureScript("npc_unstable_twilight_portal") { }

    struct npc_unstable_twilight_portalAI : public ScriptedAI
    {
        npc_unstable_twilight_portalAI(Creature* creature) : ScriptedAI(creature) {}

        void IsSummonedBy(Unit* /*summoner*/)
        {
            me->SetPhaseMask(PHASEMASK_SHADOW, true);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            me->setFaction(35);
            uint32 portalPosId = urand(0, 31);
            me->NearTeleportTo(portalPos[portalPosId].GetPositionX(), portalPos[portalPosId].GetPositionY(), portalPos[portalPosId].GetPositionZ(), portalPos[portalPosId].GetOrientation(), false);
        }

        void OnSpellClick(Unit* clicker, bool& /*result*/)
        {
            clicker->RemoveAurasDueToSpell(RAID_MODE(86202, 92889, 92890, 92891));
            clicker->RemoveAurasDueToSpell(RAID_MODE(88436, 92892, 92893, 92894));
            if (Aura *aura = clicker->GetAura(RAID_MODE(86214, 92885, 92886, 82887)))
                aura->SetDuration(120000);
            //            clicker->RemoveAurasDueToSpell((RAID_MODE(86214, 92885, 92886, 82887)));
        }

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_unstable_twilight_portalAI (creature);
    }
};

class spell_devouring_flames : public SpellScriptLoader // 92051
{
    public:
        spell_devouring_flames() : SpellScriptLoader("spell_devouring_flames") { }

        class spell_devouring_flames_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_devouring_flames_SpellScript);

            void CalculateDamage(SpellEffIndex /*effIndex*/)
            {
                if (!GetHitUnit())
                    return;

                if (Unit *caster = GetCaster())
                    if (InstanceScript *instance = caster->GetInstanceScript())
                        if (Creature *valiona = Creature::GetCreature(*caster, instance->GetData64(NPC_VALIONA_BOT)))
                        {
                            float distance = valiona->GetDistance2d(GetHitUnit());

                            if (!valiona->isInFrontInMap(GetHitUnit(), 100.0f))
                                SetHitDamage(0);

                            if (distance > 1.0f)
                                SetHitDamage(int32(GetHitDamage() - (2000 * distance))); // Drops by 2000 for every yard the player is away.

                            if (GetHitDamage() < 0) // Don't let it go lower then 0.
                                SetHitDamage(0);
                        }
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targets.clear();
                if (Unit *caster = GetCaster())
                    if (InstanceScript *instance = caster->GetInstanceScript())
                        if (Creature *valiona = Creature::GetCreature(*caster, instance->GetData64(NPC_VALIONA_BOT)))
                        {
                            ThreatContainer::StorageType const &threatList = valiona->getThreatManager().getThreatList();
                            for (ThreatContainer::StorageType::const_iterator itr = threatList.begin(); itr != threatList.end(); ++itr)
                                if (Unit* target = (*itr)->getTarget())
                                    if (target->GetPhaseMask() == PHASEMASK_NM)
                                        if (valiona->isInFrontInMap(target, 100.0f))
                                            targets.push_back(target);
                        }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_devouring_flames_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_24);
                OnEffectHitTarget += SpellEffectFn(spell_devouring_flames_SpellScript::CalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_devouring_flames_SpellScript();
        }
};

class DestructionTargetFilter
{
public:
    bool operator()(WorldObject* target) const
    {
        if (Unit* unit = target->ToUnit())
            return unit->GetEntry() != NPC_DAZZ_DESTRUCTION_STALKER;
        return false;
    }
};

class spell_dazzling_destruction: public SpellScriptLoader // 86408
{
public:
    spell_dazzling_destruction () : SpellScriptLoader("spell_dazzling_destruction") { }

    class spell_dazzling_destructionSpellScript: public SpellScript
    {
        PrepareSpellScript(spell_dazzling_destructionSpellScript);

        void HandleDummy (SpellEffIndex effIndex)
        {
            GetCaster()->CastSpell(GetHitUnit(), GetSpellInfo()->Effects[EFFECT_0].BasePoints, true);
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(DestructionTargetFilter());
        }

        void Register ()
        {
            OnEffectHitTarget += SpellEffectFn(spell_dazzling_destructionSpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dazzling_destructionSpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript () const
    {
        return new spell_dazzling_destructionSpellScript();
    }
};

class spell_dazzling_missile: public SpellScriptLoader // 86406
{
public:
    spell_dazzling_missile () : SpellScriptLoader("spell_dazzling_missile") { }

    class spell_dazzling_missileSpellScript: public SpellScript
    {
        PrepareSpellScript(spell_dazzling_missileSpellScript);

        void EffectScriptEffect(SpellEffIndex effIndex)
        {
            if (GetHitUnit()->GetPhaseMask() == PHASEMASK_SHADOW && !GetHitUnit()->HasAura(SPELL_TWIL_PROTECT_BUFF)) // ass to be rewrite with spell 93063
            {
                GetCaster()->Kill(GetHitUnit());
                return;
            }
            uint32 spellId = sSpellMgr->GetSpellIdForDifficulty(GetSpellInfo()->Effects[EFFECT_1].BasePoints, GetCaster());
            GetCaster()->AddAura(spellId, GetHitUnit());
            GetCaster()->AddAura(GetSpellInfo()->Effects[EFFECT_2].BasePoints, GetHitUnit()); // SPELL_TWIL_PROTECT_BUFF add 6 sec immunity from kill
        }

        void Register ()
        {
            OnEffectHitTarget += SpellEffectFn(spell_dazzling_missileSpellScript::EffectScriptEffect, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript * GetSpellScript () const
    {
        return new spell_dazzling_missileSpellScript();
    }
};

class spell_unstable_twilight_searcher : public SpellScriptLoader
{
public:
    spell_unstable_twilight_searcher() : SpellScriptLoader("spell_unstable_twilight_searcher") { }

    class spell_unstable_twilight_searcher_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_unstable_twilight_searcher_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (Unit* caster = GetCaster())
            {
                caster->CastSpell(caster, SPELL_UNSTABLE_TWILIGHT, true);
                caster->RemoveAllAuras();
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_unstable_twilight_searcher_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_unstable_twilight_searcher_SpellScript();
    }
};

class spell_blackout_bot : public SpellScriptLoader
{
public:
    spell_blackout_bot() : SpellScriptLoader("spell_blackout_bot") { }

    class spell_blackout_bot_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_blackout_bot_AuraScript);

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE || GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL || aurEff->GetAmount() == 0)
                GetTarget()->CastSpell(GetTarget(), SPELL_BLACKOUT_DAMAGE, true);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_blackout_bot_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_HEAL_ABSORB, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_blackout_bot_AuraScript();
    }
};

class spell_engulfing_magic : public SpellScriptLoader
{
public:
    spell_engulfing_magic() : SpellScriptLoader("spell_engulfing_magic") { }

    class spell_engulfing_magic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_engulfing_magic_AuraScript);

        void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            int32 damage = eventInfo.GetDamageInfo()->GetDamage();
            int32 heal = eventInfo.GetHealInfo()->GetHeal();

            if (damage > 0)
                GetTarget()->CastCustomSpell(GetTarget(), SPELL_ENGULGING_MAGIC_DAMAGE, &damage, NULL, NULL, true, NULL, aurEff);
            else if (heal > 0)
                GetTarget()->CastCustomSpell(GetTarget(), SPELL_ENGULGING_MAGIC_DAMAGE, &heal, NULL, NULL, true, NULL, aurEff);
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(spell_engulfing_magic_AuraScript::OnProc, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_engulfing_magic_AuraScript();
    }
};

class npc_vt_flame_tracker : public CreatureScript
{
public:
    npc_vt_flame_tracker() : CreatureScript("npc_vt_flame_tracker") { }

    struct npc_vt_flame_trackerAI : public ScriptedAI
    {
        npc_vt_flame_trackerAI(Creature* creature) : ScriptedAI(creature) {}

        void IsSummonedBy(Unit* summoner)
        {
            if (InstanceScript *instance = me->GetInstanceScript())
                if (Creature *valiona = Creature::GetCreature(*me, instance->GetData64(NPC_VALIONA_BOT)))
                    valiona->AI()->JustSummoned(me);
        }

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_vt_flame_trackerAI (creature);
    }
};

class CasterTargetFilter
{
public:
    bool operator()(WorldObject* target) const
    {
        if (target->GetPhaseMask() == PHASEMASK_SHADOW)
            return true;

        if (Player* player = target->ToPlayer())
        {
            switch (player->GetPrimaryTalentTree(player->GetActiveSpec()))
            {
                case TALENT_TREE_PRIEST_DISCIPLINE:
                case TALENT_TREE_PRIEST_SHADOW:
                case TALENT_TREE_PRIEST_HOLY:
                case TALENT_TREE_MAGE_ARCANE:
                case TALENT_TREE_MAGE_FIRE:
                case TALENT_TREE_MAGE_FROST:
                case TALENT_TREE_WARLOCK_AFFLICTION:
                case TALENT_TREE_WARLOCK_DEMONOLOGY:
                case TALENT_TREE_WARLOCK_DESTRUCTION:
                case TALENT_TREE_PALADIN_HOLY:
                case TALENT_TREE_SHAMAN_RESTORATION:
                case TALENT_TREE_SHAMAN_ELEMENTAL:
                case TALENT_TREE_DRUID_RESTORATION:
                case TALENT_TREE_DRUID_BALANCE:
                case TALENT_TREE_HUNTER_BEAST_MASTERY:
                case TALENT_TREE_HUNTER_MARKSMANSHIP:
                case TALENT_TREE_HUNTER_SURVIVAL:
                    return false;
                default:
                    break;
            }
        }

        return true;
    }
};

class spell_engulfing_magic_target_filter : public SpellScriptLoader
{
public:
    spell_engulfing_magic_target_filter () : SpellScriptLoader("spell_engulfing_magic_target_filter") { }

    class spell_engulfing_magic_target_filterSpellScript: public SpellScript
    {
        PrepareSpellScript(spell_engulfing_magic_target_filterSpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(CasterTargetFilter());
            uint32 count = 1;
            if (GetCaster()->GetMap()->Is25ManRaid())
                count += 1;
            if (GetCaster()->GetMap()->IsHeroic())
                count += 1;
            Trinity::Containers::RandomResizeList(targets, count);
        }

        void Register ()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_engulfing_magic_target_filterSpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript () const
    {
        return new spell_engulfing_magic_target_filterSpellScript();
    }
};

class spell_twilight_zone_filter : public SpellScriptLoader
{
public:
    spell_twilight_zone_filter() : SpellScriptLoader("spell_twilight_zone_filter") { }

    class spell_twilight_zone_filter_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_twilight_zone_filter_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end();)
            {
                if ((*itr)->GetTypeId() != TYPEID_PLAYER)
                    targets.erase(itr++);

                if ((*itr)->ToPlayer()->GetPhaseMask() == PHASEMASK_SHADOW)
                    ++itr;
                else
                    targets.erase(itr++);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_twilight_zone_filter_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_twilight_zone_filter_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_twilight_zone_filter_SpellScript();
    }
};

// 93051
class spell_vali_ther_twilight_shift_heroic : public SpellScriptLoader
{
public:
    spell_vali_ther_twilight_shift_heroic() : SpellScriptLoader("spell_vali_ther_twilight_shift_heroic") { }

    class spell_vali_ther_twilight_shift_heroic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_vali_ther_twilight_shift_heroic_AuraScript);

        void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit *caster = GetCaster())
                if (GetStackAmount() == GetSpellInfo()->StackAmount)
                {
                    if (WorldObject *targ = GetOwner())
                        if (Unit *target = targ->ToUnit())
                        {
                            target->RemoveAura(SPELL_TWILIGHT_SHIFT_A);
                            target->RemoveAura(SPELL_TWILIGHT_SHIFT);
                            target->CastSpell(target, SPELL_SHIFTING_REALITY, true);

                            uint32 spellId = sSpellMgr->GetSpellIdForDifficulty(SPELL_TWILIGHT_ZONE_2, GetCaster());

                            if (!target->HasAura(spellId))
                                caster->AddAura(spellId, target);
                        }
                }
        }

        void Register()
        {
            AfterEffectApply += AuraEffectApplyFn(spell_vali_ther_twilight_shift_heroic_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_vali_ther_twilight_shift_heroic_AuraScript();
    }
};

class spell_shifting_reality : public SpellScriptLoader
{
    class spell_shifting_reality_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_shifting_reality_SpellScript);

        void HandleEffect(SpellEffIndex /*effIndex*/)
        {
            uint32 spellId = sSpellMgr->GetSpellIdForDifficulty(SPELL_TWILIGHT_ZONE_2, GetCaster());
            if (Player *target = GetHitPlayer())
                if (!target->HasAura(spellId))
                    target->AddAura(spellId, target);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_shifting_reality_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };
public:
    spell_shifting_reality() : SpellScriptLoader("spell_shifting_reality") { }

    SpellScript* GetSpellScript() const
    {
        return new spell_shifting_reality_SpellScript();
    }
};


class spell_twiligh_sentry_summon_periodic : public SpellScriptLoader
{
public:
    spell_twiligh_sentry_summon_periodic() : SpellScriptLoader("spell_twiligh_sentry_summon_periodic") { }

    class spell_twiligh_sentry_summon_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_twiligh_sentry_summon_periodic_AuraScript);

        void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
        {
            if (WorldObject *owner = GetOwner())
            {
                if (Creature *valiona = owner->ToCreature())
                    valiona->CastSpell(valiona, SPELL_TWILIGHT_SENTRY_SUMMON, true);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_twiligh_sentry_summon_periodic_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_twiligh_sentry_summon_periodic_AuraScript();
    }
};

class spell_tank_twilight_shift_heroic : public SpellScriptLoader
{
public:
    spell_tank_twilight_shift_heroic() : SpellScriptLoader("spell_tank_twilight_shift_heroic") { }

    class spell_tank_twilight_shift_heroic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_tank_twilight_shift_heroic_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            return true;
        }

        bool Load()
        {
            return true;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            if (Unit *target = eventInfo.GetProcTarget())
                if (target->GetTypeId() == TYPEID_PLAYER)
                {
                    if (target->HasSpellCooldown(SPELL_TWILIGHT_SHIFT))
                        return;
                    if (Unit *caster = GetCaster())
                        caster->CastSpell(target, SPELL_TWILIGHT_SHIFT, true);
                    target->ToPlayer()->AddSpellCooldown(SPELL_TWILIGHT_SHIFT, 0, time(NULL) + 20);
                }
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(spell_tank_twilight_shift_heroic_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }

    private:
        Unit* _procTarget;
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_tank_twilight_shift_heroic_AuraScript();
    }
};

class spell_twilight_sentry_summon : public SpellScriptLoader // 93035
{
    public:
        spell_twilight_sentry_summon() : SpellScriptLoader("spell_twilight_sentry_summon") { }

        class spell_twilight_sentry_summon_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_twilight_sentry_summon_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!targets.empty())
                    Trinity::Containers::RandomResizeList(targets, 1);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_twilight_sentry_summon_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_twilight_sentry_summon_SpellScript();
        }
};

class npc_unstable_twilight : public CreatureScript
{
public:
    npc_unstable_twilight() : CreatureScript("npc_unstable_twilight") { }

    enum us_event
    {
        EVENT_RESPAWN = 1,
    };

    struct npc_unstable_twilightAI : public ScriptedAI
    {
        npc_unstable_twilightAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            me->GetMotionMaster()->MoveRandom(5.0f);
        }

        void SpellHitTarget(Unit* target, SpellInfo const* spell)
        {
            events.ScheduleEvent(EVENT_RESPAWN, 30000);
        }

        void EnterEvadeMode() {}

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_RESPAWN:
                        me->AddAura(SPELL_UNSTABLE_TWILIGHT_TRIGGER, me);
                        return;
                }
            }
        }
    private :
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_unstable_twilightAI (creature);
    }
};

class ShadowPhaseFilter
{
public:
    bool operator()(WorldObject* target) const
    {
        if (Unit* unit = target->ToUnit())
            return unit->GetPhaseMask() == PHASEMASK_SHADOW;
        return true;
    }
};

class spell_twilight_flame: public SpellScriptLoader
{
public:
    spell_twilight_flame () : SpellScriptLoader("spell_twilight_flame") { }

    class spell_twilight_flame_SpellScript: public SpellScript
    {
        PrepareSpellScript(spell_twilight_flame_SpellScript);


        void EffectScriptEffect(SpellEffIndex effIndex)
        {
            uint32 spellId = sSpellMgr->GetSpellIdForDifficulty(SPELL_TWILIGHT_ZONE, GetCaster());
            GetCaster()->AddAura(spellId, GetHitUnit());
            GetCaster()->AddAura(GetSpellInfo()->Effects[EFFECT_2].BasePoints, GetHitUnit());
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(ShadowPhaseFilter());
        }

        void Register ()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_twilight_flame_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_twilight_flame_SpellScript::FilterTargets, EFFECT_2, TARGET_UNIT_SRC_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(spell_twilight_flame_SpellScript::EffectScriptEffect, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript * GetSpellScript () const
    {
        return new spell_twilight_flame_SpellScript();
    }
};

class spell_twilight_meteorite : public SpellScriptLoader
{
public:
    spell_twilight_meteorite() : SpellScriptLoader("spell_twilight_meteorite") { }

    class spell_twilight_meteorite_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_twilight_meteorite_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(ShadowPhaseFilter());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_twilight_meteorite_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_twilight_meteorite_SpellScript();
    }
};

// 86631
class spell_engulfing_magic_damage_filter : public SpellScriptLoader
{
public:
    spell_engulfing_magic_damage_filter() : SpellScriptLoader("spell_engulfing_magic_damage_filter") { }

    class spell_engulfing_magic_damage_filter_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_engulfing_magic_damage_filter_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(ShadowPhaseFilter());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_engulfing_magic_damage_filter_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_engulfing_magic_damage_filter_SpellScript();
    }
};

// 86505
class spell_fabulous_flame_damage_filter : public SpellScriptLoader
{
public:
    spell_fabulous_flame_damage_filter() : SpellScriptLoader("spell_fabulous_flame_damage_filter") { }

    class spell_fabulous_flame_damage_filter_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_fabulous_flame_damage_filter_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(ShadowPhaseFilter());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_fabulous_flame_damage_filter_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_fabulous_flame_damage_filter_SpellScript();
    }
};

// 86825
class spell_blackout_damage_filter : public SpellScriptLoader
{
public:
    spell_blackout_damage_filter() : SpellScriptLoader("spell_blackout_damage_filter") { }

    class spell_blackout_damage_filter_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_blackout_damage_filter_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(ShadowPhaseFilter());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_blackout_damage_filter_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_blackout_damage_filter_SpellScript();
    }
};

// 86371
class spell_twilight_blast_damage_filter : public SpellScriptLoader
{
public:
    spell_twilight_blast_damage_filter() : SpellScriptLoader("spell_twilight_blast_damage_filter") { }

    class spell_twilight_blast_damage_filter_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_twilight_blast_damage_filter_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(ShadowPhaseFilter());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_twilight_blast_damage_filter_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_twilight_blast_damage_filter_SpellScript();
    }
};

class NonShadowPhaseFilter
{
public:
    bool operator()(WorldObject* target) const
    {
        if (Unit* unit = target->ToUnit())
            return unit->GetPhaseMask() == PHASEMASK_SHADOW;
        return true;
    }
};

// 86305
class spell_unstable_twilight_damage_filter : public SpellScriptLoader
{
public:
    spell_unstable_twilight_damage_filter() : SpellScriptLoader("spell_unstable_twilight_damage_filter") { }

    class spell_unstable_twilight_damage_filter_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_unstable_twilight_damage_filter_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(NonShadowPhaseFilter());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_unstable_twilight_damage_filter_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_unstable_twilight_damage_filter_SpellScript();
    }
};

void AddSC_boss_valiona_theralion()
{
    new boss_valiona_bot();
    new boss_theralion_bot();
    new npc_fabulous_flames();
    new npc_dazzling_visual();
    new npc_twilight_sentry();
    new npc_twilight_rift();
    new npc_twilight_flame();
    new npc_unstable_twilight_portal(); // Struggling to find correct entry. @TODO
    new spell_devouring_flames();
    new spell_dazzling_destruction();
    new spell_dazzling_missile();
    new spell_unstable_twilight_searcher();
    new spell_blackout_bot();
    new spell_engulfing_magic();
    new npc_vt_flame_tracker();
    new spell_engulfing_magic_target_filter();
    new spell_twilight_zone_filter();
    new spell_vali_ther_twilight_shift_heroic();
    new spell_shifting_reality();
    new spell_twiligh_sentry_summon_periodic(); // Struggling to find correct entry. @TODO
    new spell_tank_twilight_shift_heroic(); // Struggling to find correct entry. @TODO
    new spell_twilight_sentry_summon();
    new npc_unstable_twilight();
    new spell_twilight_flame();
    new spell_twilight_meteorite();
    new spell_engulfing_magic_damage_filter();
    new spell_fabulous_flame_damage_filter();
    new spell_blackout_damage_filter();
    new spell_twilight_blast_damage_filter();
    new spell_unstable_twilight_damage_filter();
}
