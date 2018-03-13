
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
#include "firelands.h"

enum Texts
{
    // Cenarius
    SAY_CENARIUS_1 = 0,
    SAY_CENARIUS_OUTRO_1 = 1,
    SAY_CENARIUS_OUTRO_2 = 2,
    SAY_CENARIUS_OUTRO_3 = 3,

    // Malfurion
    SAY_MALFURION_1 = 0,
    SAY_MALFURION_OUTRO_1 = 1,
    SAY_MALFURION_OUTRO_2 = 2,

    // Hamuul
    SAY_HAMUUL_OUTRO_1 = 0,

    // Ragnaros
    SAY_ARRIVE = 0,
    SAY_DEATH_NORMAL = 1,
    SAY_DEATH_HEROIC = 2,
    SAY_SLAY = 3,
    SAY_ANNOUNCE_SPLIT = 4,
    SAY_SUBMERGE = 5,
    SAY_EMERGE = 6,
    SAY_SONS_OF_FLAME_1 = 7,
    SAY_SONS_OF_FLAME_2 = 8,
    SAY_ANNOUNCE_EMERGE = 9,
    SAY_MAGMA_TRAP = 10,
    SAY_ANNOUNCE_SULFURAS_SMASH = 11,
    SAY_SUBMERGE_HEROIC = 12,
    SAY_INTRO_HEROIC_1 = 13,
    SAY_INTRO_HEROIC_2 = 14,
    SAY_ENGULFING_FLAMES = 15,
    SAY_WORLD_IN_FLAMES = 16,
    SAY_EMPOWER_SULFURAS = 17,
    SAY_DREADFLAME = 18,
};

enum Spells
{
    // Ragnaros Base Auras
    SPELL_BASE_VISUAL = 98860,
    SPELL_BURNING_WOUNDS_AURA = 99401,

    // Magma
    SPELL_MAGMA_PERIODIC = 99908,

    // Spell Sulfuras Smash
    SPELL_SULFURAS_SMASH_DUMMY = 98703,
    SPELL_SULFURAS_SMASH = 98710,
    SPELL_SULFURAS_SMASH_MISSILE = 98708,
    SPELL_SULFURAS_SMASH_TARGET = 98706,

    // Lava Wave Summons
    SPELL_LAVA_WAVE_DUMMY_NORTH = 98874,
    SPELL_LAVA_WAVE_DUMMY_EAST = 98875,
    SPELL_LAVA_WAVE_DUMMY_WEST = 98876,
    SPELL_LAVA_POOL = 98712,
    SPELL_SCORCHED_GROUND = 98871,
    SPELL_LAVA_WAVE_DAMAGE = 98928,

    // Magma Strike
    SPELL_MAGMA_STRIKE = 98313,

    // Magma Trap
    SPELL_MAGMA_TRAP = 98164,
    SPELL_MAGMA_TRAP_UNK = 98159,
    SPELL_MAGMA_TRAP_VISUAL = 98179,
    SPELL_MAGMA_TRAP_PERIODIC = 98172,
    SPELL_MAGMA_TRAP_PERIODIC_TICK = 98171,
    SPELL_MAGMA_TRAP_ERUPTION = 98175,

    // Hand of Ragnaros
    SPELL_HAND_OF_RAGNAROS = 98237,

    // Wrath of Ragnaros
    SPELL_WRATH_OF_RAGNAROS = 98263,

    // Lava Wave
    SPELL_LAVA_WAVE = 98873,

    // Splitting Blow
    SPELL_SUBMERGE = 100051,
    SPELL_SUBMERGE_AURA = 98982,
    SPELL_SPLITTING_BLOW_WEST = 98953,
    SPELL_SPLITTING_BLOW_NORTH = 98952,
    SPELL_SPLITTING_BLOW_EAST = 98951,
    SPELL_SUMMON_SULFURAS_AURA = 99056,

    // Sulfuras, Hand of Ragnaros
    SPELL_INVOKE_SONS = 99054,
    SPELL_INVOKE_SONS_MISSILE = 99050,
    SPELL_INVOKE_SONS_TRIGGERED_SPELL = 99051,
    SPELL_SULFURAS_AURA = 100456,

    // Son of Flame
    SPELL_HIT_ME = 100446,
    SPELL_PRE_VISUAL = 98983,
    SPELL_BURNING_SPEED = 98473,
    SPELL_BURNING_SPEED_STACKS = 99414,
    SPELL_SUPERNOVA = 99112,

    // Engulfing Flames
    SPELL_ENGULFING_FLAMES_BOTTOM = 99236,
    SPELL_ENGULFING_FLAMES_CENTER = 99235,
    SPELL_ENGULFING_FLAMES_MELEE = 99172,
    SPELL_ENGULFING_FLAMES_VISUAL_MELEE = 99216,
    SPELL_ENGULFING_FLAMES_VISUAL_CENTER = 99217,
    SPELL_ENGULFING_FLAMES_VISUAL_BOTTOM = 99218,
    SPELL_WOLRD_IN_FLAMES = 100171,

    // Molten Seed
    SPELL_MOLTEN_SEED = 98333,
    SPELL_MOLTEN_SEED_SUMMON = 100141,

    SPELL_MOLTEN_SEED_MISSILE = 98495,
    SPELL_MOLTEN_SEED_DAMAGE = 98498,
    SPELL_UNK_1 = 98497,

    // Molten Elemental
    SPELL_INVISIBLE_PRE_VISUAL = 100153,
    SPELL_MOLTEN_SEED_VISUAL = 98520,
    SPELL_MOLTEN_INFERNO = 98518,
    SPELL_MOLTEN_POWER_AURA = 100157,
    SPELL_MOLTEN_POWER_PERIODIC = 100158,

    // Lava Scion
    SPELL_BLAZING_HEAT = 100460,
    SPELL_BLAZING_HEAT_SONAR = 100459,

    // Blazing Heat
    SPELL_BLAZING_HEAT_AURA = 99126,
    SPELL_BLAZING_HEAT_SUMMON = 99129,
    SPELL_BLAZING_HEAT_DUMMY = 99125,
    SPELL_BLAZING_HEAT_DAMAGE_AURA = 99128,

    // Living Meteor
    SPELL_LIVING_METEOR_MISSILE = 99268,
    SPELL_LIVING_METEOR_DAMAGE_REDUCTION = 100904,
    SPELL_LIVING_METEOR_COMBUSTIBLE = 99296,
    SPELL_LIVING_METEOR_COMBUSTITION = 99303, // Knockback 
    SPELL_LIVING_METEOR_EXPLOSION_TRIGGER = 99269,
    SPELL_LIVING_METEOR_INCREASE_SPEED = 100278,
    SPELL_LIVING_METEOR_FIXATE = 99849,
    SPELL_LIVING_METEOR_UNK = 99267,
    SPELL_LIVING_METEOR_EXPLOSION = 99287,

    // Heroic Encounter

    // Ragnaros
    SPELL_LEGS_HEAL = 100346,
    SPELL_TRANSFORM = 100420,
    SPELL_SUPERHEATED = 100593,
    SPELL_SUPERHEATED_TRIGGERED = 100594,

    SPELL_EMPOWER_SULFURAS = 100604,
    SPELL_EMPOWER_SULFURAS_TRIGGER = 100605,
    SPELL_EMPOWER_SULFURAS_MISSILE = 100606,

    // Dreadflame
    SPELL_SUMMON_DREADFLAME = 100675,
    SPELL_DREADFLAME_CONTROL_AURA_1 = 100695,
    SPELL_DREADFLAME_AURA_1_TRIGERED = 100966,
    SPELL_DREADFLAME_CONTROL_AURA_2 = 100696,
    SPELL_DREADFLAME_AURA_2_TRIGERED = 100823,
    SPELL_DREADFLAME_DAMAGE_AURA = 100692,
    SPELL_DREADFLAME_CONTROL_AURA_3 = 100905,
    SPELL_DREADFLAME_AURA_3_TRIGERED = 100906,

    SPELL_DREADFLAME_DUMMY = 100691,
    SPELL_DREADFLAME_SUMMON_MISSILE = 100675,

    // Protection Traps
    SPELL_BREADTH_OF_FROST_SCRIPT = 100472,
    SPELL_BREADTH_OF_FROST_SUMMON = 100476,
    SPELL_BREADTH_OF_FROST_AURA = 100479,
    SPELL_BREADTH_OF_FROST_STUN = 100567,
    SPELL_BREADTH_OF_FROST_PROTECTION = 100503,

    SPELL_ENTRAPPING_ROOTS_AURA_MISSILE = 100646,
    SPELL_ENTRAPPING_ROOTS_SUMMON = 100644,
    SPELL_ENTRAPPING_ROOTS_ROOT = 100653,

    SPELL_CLOUDBURST_DUMMY = 100751,
    SPELL_CLOUDBURST_SUMMON = 100714,
    SPELL_CLOUDBURST_DUMMY_AURA = 100758,
    SPELL_CLOUDBURST_VISUAL_WATER = 100757,
    SPELL_CLOUDBURST_PLAYER_AURA = 100713,

    // Hamuul Runetotem
    SPELL_TRANSFORM_HAMUUL = 100311,
    SPELL_HAMUL_DRAW_FIRELORD = 100344,

    // Malfurion Stormrage
    SPELL_TRANSFORM_MALFURION = 100310,
    SPELL_MALFURION_DRAW_FIRELORD = 100342,

    // Cenarius
    SPELL_CENARIUS_DRAW_FIRELORD = 100345,

    // Heart of Ragnaros
    SPELL_RAGE_OF_RAGNAROS = 101110,
    SPELL_HEART_OF_RAGNAROS_SUMMON = 101254,
    SPELL_HEART_OF_RAGNAROS_DUMMY_AURA = 101127,
    SPELL_HEART_OF_RAGNAROS_CREATE_HEART = 101125,
};

enum Phases
{
    PHASE_INTRO = 1,
    PHASE_1 = 2,
    PHASE_2 = 3,
    PHASE_3 = 4,
    PHASE_SUBMERGED = 5,
    PHASE_HEROIC = 6,
};

enum ragnarosNpcs
{
    NPC_SULFURAS_SMASH_TRIGGER = 53266,
    NPC_SULFURAS_SMASH_TARGET = 53268,
    NPC_LAVA_WAVE = 53363,
    NPC_ENGULFING_FLAMES_TRIGGER = 53485,
    NPC_SPLITTING_BLOW_TRIGGER = 53393,
    NPC_MAGMA_POOL_TRIGGER = 53729,
    NPC_PLATFORM_TRIGGER = 53952,
    NPC_MAGMA_TRAP = 53086,
    NPC_SULFURAS_HAND_OF_RAGNAROS = 53420,
    NPC_SON_OF_FLAME = 53140,
    NPC_MOLTEN_ERUPTER = 53617,
    NPC_MOLTEN_SPEWER = 53545,
    NPC_MOLTEN_ELEMENTAL = 53189,
    NPC_MOLTEN_SEED_CASTER = 53186,
    NPC_LAVA_SCION = 53231,
    NPC_BLAZING_HEAT = 53473,
    NPC_LIVING_METEOR = 53500,
    NPC_DREADFLAME = 54127,
    NPC_DREADFLAME_SPAWN = 54203,
    NPC_CLOUDBURST = 54147,
    NPC_ENTRAPPING_ROOTS = 54074,
    NPC_BREADTH_OF_FROST = 53953,
    NPC_HEART_OF_RAGNAROS = 54293,
    NPC_CENARIUS = 53872,
    NPC_HAMUUL = 53876,
    NPC_MALFURION = 53875,
};

enum Events
{
    // Ragnaros
    EVENT_INTRO = 1,
    EVENT_ATTACK,
    EVENT_SULFURAS_SMASH_TRIGGER,
    EVENT_SULFURAS_SMASH,
    EVENT_MAGMA_TRAP,
    EVENT_WRATH_OF_RAGNAROS,
    EVENT_HAND_OF_RAGNAROS,
    EVENT_SPLITTING_BLOW,
    EVENT_CALL_SONS,
    EVENT_ANNOUNCE_EMERGE,
    EVENT_EMERGE,
    EVENT_ENGULFING_FLAMES,
    EVENT_MOLTEN_SEED,
    EVENT_LIVING_METEOR,
    EVENT_ATTACK_HEROIC,
    EVENT_CHECK_DIST,

    // Sulfuras Smash
    EVENT_SCORCH,
    EVENT_MOVE_LAVA_WAVE,
    EVENT_RESTART_MOVE_LAVA_WAVE,
    EVENT_SUMMON_WAVE_1,
    EVENT_SUMMON_WAVE_2,
    EVENT_SUMMON_WAVE_3,

    // Magma Trap
    EVENT_PREPARE_TRAP,

    // Son of Flame
    EVENT_MOVE_HAMMER,

    // Sulfuras, Hand of Ragnaros
    EVENT_CHECK_SONS,

    // Molten Elemental
    EVENT_PREPARE_ELEMENTAL,
    EVENT_ACTIVATE,

    // Lava Scion
    EVENT_BLAZING_HEAT,

    // Living Meteor
    EVENT_STALK_PLAYER,
    EVENT_KILL_PLAYER,
    EVENT_ENABLE_KNOCKBACK,

    // Archdruids
    EVENT_SAY_PULL,
    EVENT_SAY_CAUGHT,
    EVENT_DRAW_RAGNAROS,
    EVENT_TRANSFORM,

    // Ragnaros Heroic Events
    EVENT_EMERGE_HEROIC,
    EVENT_TALK,
    EVENT_TAUNT_EMOTE,
    EVENT_STANDUP,
    EVENT_FREEZE_PLATFORM,
    EVENT_BREAK_PLATFORM,
    EVENT_IDLE,
    EVENT_TRANSFORM_RAGNAROS,
    EVENT_SUMMON_DREADFLAME,
    EVENT_SCHEDULE_EMPOWER,
    EVENT_EMPOWER_SULFURAS,

    // Cenarius
    EVENT_BREADTH_OF_FROST,
    EVENT_TALK_CENARIUS_1,
    EVENT_TALK_CENARIUS_2,
    EVENT_TALK_CENARIUS_3,

    // Hamuul
    EVENT_ENTRAPPING_ROOTS,
    EVENT_TALK_HAMUUL_1,

    // Malfurion
    EVENT_CLOUDBURST,
    EVENT_TALK_MALFURION_1,
    EVENT_TALK_MALFURION_2,

    // Dreadflame
    EVENT_CHECK_PLAYER,
    EVENT_SPREAD_FLAME,
};

enum Actions
{
    // Ragnaros
    ACTION_SON_KILLED = 1,
    ACTION_ACTIVATE_SON = 2,
    ACTION_INSTANT_EMERGE = 3,
    ACTION_ACTIVATE_HEROIC = 4,
    ACTION_SUBMERGE = 5,

    // Archdruids
    ACTION_SCHEDULE_CLOUDBURST = 6,
    ACTION_SCHEDULE_ROOTS = 7,
    ACTION_SCHEDULE_BREADTH = 8,
    ACTION_SCHEDULE_OUTRO = 9,
};

enum AnimKits
{
    // Ragnaros
    ANIM_KIT_RESURFACE = 1465,
    ANIM_KIT_EMERGE = 1467,
    ANIM_KIT_TAUNT = 1468,
    ANIM_KIT_STAND_UP = 1486,
    ANIM_KIT_SUBMERGE = 1522,
    // Malfurion
    ANIM_KIT_TALK = 1523,

    // Son of Flame
    ANIM_KIT_UNK_1 = 1370,
};

enum RagnarosQuest
{
    QUEST_HEART_OF_FIRE = 29307,
};

Position const RagnarosSummonPosition = { 1075.201f, -57.84896f, 55.42427f, 3.159046f };
Position const SplittingTriggerNorth = { 1023.55f, -57.158f, 55.4215f, 3.12414f };
Position const SplittingTriggerEast = { 1035.45f, -25.3646f, 55.4924f, 2.49582f };
Position const SplittingTriggerWest = { 1036.27f, -89.2396f, 55.5098f, 3.83972f };

Position const CenariusSummonPosition = { 926.705f, -57.833f, 55.933f, 0.02050f };
Position const HamuulSummonPosition = { 926.705f, -57.833f, 55.933f, 0.02050f };
Position const MalfurionSummonPosition = { 926.705f, -57.833f, 55.933f, 0.02050f };

Position const MalfurionPoint = { 984.996f, -73.638f, 55.348f };
Position const CenariusPoint = { 984.1371f, -57.65625f, 55.36652f };
Position const HamuulPoint = { 982.9132f, -43.22049f, 55.35419f };

Position const CachePosition = { 1012.48999f, -57.2882004f, 55.3302002f, 4.41094017f };

/*
Positions for Sons of Flame
*/

const Position SonsOfFlameWest[] =
{
    { 999.533f, -45.74826f, 55.56229f, 5.009095f },
    { 1065.372f, -108.8698f, 55.56829f, 2.373648f },
    { 1034.168f, -15.93056f, 55.60827f, 4.049164f },
    { 1051.76f, 0.1284722f, 55.44715f, 4.031711f },
    { 1014.134f, -43.94445f, 55.42412f, 3.979351f },
    { 1068.151f, -101.3924f, 55.56828f, 2.565634f },
    { 1051.497f, -113.7292f, 55.44935f, 2.391101f },
    { 1074.866f, -100.7882f, 55.42414f, 2.600541f },
};

const Position SonsOfFlameNorth[] =
{
    { 1065.372f, -108.8698f, 55.56829f, 2.373648f },
    { 1030.372f, -23.63715f, 55.44154f, 4.049164f },
    { 1014.325f, -88.80209f, 55.52722f, 1.919862f },
    { 1051.760f, 0.1284722f, 55.44715f, 4.031711f },
    { 1012.120f, -26.89063f, 55.564f, 4.398230f },
    { 1055.556f, -8.875f, 55.43348f, 3.874631f },
    { 1040.394f, -91.81944f, 55.42585f, 2.234021f },
    { 1065.257f, -6.946181f, 55.56818f, 3.839724f },
};

const Position SonsOfFlameEast[] =
{
    { 1013.458f, -68.08507f, 55.42097f, 2.513274f },
    { 1065.372f, -108.8698f, 55.56829f, 2.373648f },
    { 1042.033f, -114.9132f, 55.44709f, 1.919862f },
    { 1051.76f, 0.1284722f, 55.44715f, 4.031711f },
    { 1061.34f, -16.74132f, 55.56819f, 3.769911f },
    { 1051.497f, -113.7292f, 55.44935f, 2.391101f },
    { 1024.845f, -97.67882f, 55.52884f, 2.234021f },
    { 1074.866f, -100.7882f, 55.42414f, 2.600541f },
};

/*
Positions for engulfing flames
*/

const Position EngulfingFlamesMelee[] =
{
    { 1086.55f, -18.0885f, 55.4228f },
    { 1091.83f, -21.9254f, 55.4241f },
    { 1092.52f, -92.3924f, 55.4241f },
    { 1079.15f, -15.5312f, 55.4230f },
    { 1078.01f, -97.7760f, 55.4227f },
    { 1065.44f, -17.7049f, 55.4250f },
    { 1063.59f, -97.0573f, 55.4934f },
    { 1051.80f, -24.0903f, 55.4258f },
    { 1049.27f, -90.6892f, 55.4259f },
    { 1042.34f, -32.1059f, 55.4254f },
    { 1041.26f, -81.4340f, 55.4240f },
    { 1036.82f, -44.3385f, 55.4425f },
    { 1036.34f, -69.8281f, 55.4425f },
    { 1034.76f, -63.9583f, 55.4397f },
    { 1033.93f, -57.0920f, 55.4225f },
    { 1086.42f, -96.7812f, 55.4226f },
};

const Position EngulfingFlamesRange[] =
{
    { 1052.58f, -120.561f, 55.4561f },
    { 1049.73f, -118.396f, 55.5661f },
    { 1035.56f, -114.155f, 55.4471f },
    { 1024.91f, -106.851f, 55.4471f },
    { 1012.09f, -97.5121f, 55.4570f },
    { 1005.48f, -86.4569f, 55.4275f },
    { 1003.44f, -74.0243f, 55.4063f },
    { 1003.07f, -66.4913f, 55.4067f },
    { 1002.00f, -58.2396f, 55.4331f },
    { 1002.21f, -49.7048f, 55.4075f },
    { 1002.71f, -40.7430f, 55.4063f },
    { 1006.09f, -27.3680f, 55.4277f },
    { 1014.15f, -17.3281f, 55.4628f },
    { 1024.44f, -8.1388f, 55.4469f },
    { 1035.91f, 0.9097f, 55.4469f },
    { 1049.32f, 5.0434f, 55.4632f },
    { 1055.33f, 5.0677f, 55.4471f },
    { 1032.47f, 13.2708f, 55.4468f },
    { 1023.83f, 12.9774f, 55.4469f },
    { 1019.59f, 7.7691f, 55.4469f },
    { 1012.70f, -4.8333f, 55.6050f },
    { 1005.79f, -8.8177f, 55.4672f },
    { 1000.80f, -14.5069f, 55.4566f },
    { 991.79f, -25.0955f, 55.4440f },
    { 986.60f, -37.7655f, 55.4411f },
    { 988.20f, -50.3646f, 55.4291f },
    { 980.92f, -58.2655f, 55.4542f },
    { 989.86f, -66.0868f, 55.4331f },
    { 985.17f, -77.3785f, 55.4408f },
    { 991.73f, -87.1632f, 55.4445f },
    { 999.75f, -98.4792f, 55.4426f },
    { 1009.55f, -108.161f, 55.4697f },
    { 1018.28f, -117.833f, 55.4471f },
    { 1023.04f, -128.257f, 55.4471f },
    { 1035.17f, -125.646f, 55.4471f },
};

const Position EngulfingFlamesCenter[] =
{
    { 1069.66f, -4.5399f, 55.4308f },
    { 1062.93f, -4.3420f, 55.5681f },
    { 1057.03f, -4.1041f, 55.4258f },
    { 1049.97f, -7.2239f, 55.4537f },
    { 1036.90f, -14.6181f, 55.5714f },
    { 1025.33f, -25.8472f, 55.4068f },
    { 1021.84f, -33.7482f, 55.4239f },
    { 1018.46f, -43.7673f, 55.4217f },
    { 1016.98f, -57.5642f, 55.4133f },
    { 1018.28f, -70.1875f, 55.4231f },
    { 1021.48f, -79.6075f, 55.4261f },
    { 1025.29f, -86.2325f, 55.4071f },
    { 1030.21f, -92.8403f, 55.4343f },
    { 1038.53f, -100.253f, 55.6012f },
    { 1049.66f, -104.905f, 55.4556f },
    { 1062.13f, -109.004f, 55.4259f },
    { 1069.91f, -109.651f, 55.4277f },
};

const Position LavaScionPos[] =
{
    { 1027.306f, -121.7465f, 55.4471f, 1.361f },
    { 1026.861f, 5.895833f, 55.44697f, 4.904f },
};

class at_sulfuron_keep : public AreaTriggerScript
{
public:
    at_sulfuron_keep() : AreaTriggerScript("at_sulfuron_keep") { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* /*areaTrigger*/)
    {
        if (InstanceScript* instance = player->GetInstanceScript())
        {
            if (Creature* ragna = ObjectAccessor::GetCreature(*player, instance->GetData64(DATA_RAGNAROS)))
            {
                // Do nothing
            }
            else
            {
                if (instance->GetData(DATA_RAGNAROS_EVENT) != DONE &&
                    instance->GetData(DATA_RHYOLITH_EVENT) == DONE &&
                    instance->GetData(DATA_BETHTILAC_EVENT) == DONE &&
                    instance->GetData(DATA_BALEROC_EVENT) == DONE &&
                    instance->GetData(DATA_MAJORDOMO_STAGHELM_EVENT) == DONE)
                    player->SummonCreature(BOSS_RAGNAROS, RagnarosSummonPosition, TEMPSUMMON_MANUAL_DESPAWN, 0);
            }
        }
        return true;
    }
};

class boss_ragnaros_cata : public CreatureScript
{
public:
    boss_ragnaros_cata() : CreatureScript("boss_ragnaros_cata") { }

    struct boss_ragnaros_cataAI : public BossAI
    {
        boss_ragnaros_cataAI(Creature* creature) : BossAI(creature, DATA_RAGNAROS)
        {
            _submergeCounter = 0;
            _sonCounter = 0;
            _heartQuest = false;
        }

        uint8 _submergeCounter;
        uint8 _sonCounter;
        Unit* magma;
        bool _heartQuest;

        void Reset()
        {
            _Reset();
            events.Reset();
            events.SetPhase(PHASE_1);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            me->SetByteValue(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetStandState(0);
            me->SetReactState(REACT_DEFENSIVE);

            if (instance)
                instance->SetData(DATA_RAGNAROS_EVENT, NOT_STARTED);

            if (GameObject* platform = me->FindNearestGameObject(GO_RAGNAROS_PLATFORM, 200.0f))
                platform->SetDestructibleState(GO_DESTRUCTIBLE_INTACT);

            _submergeCounter = 0;
            _sonCounter = 0;
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            HandleDoor();

            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            me->SetReactState(REACT_AGGRESSIVE);
            events.SetPhase(PHASE_1);
            events.ScheduleEvent(EVENT_SULFURAS_SMASH_TRIGGER, 30000, 0, PHASE_1);
            events.ScheduleEvent(EVENT_MAGMA_TRAP, 15500, 0, PHASE_1);
            events.ScheduleEvent(EVENT_HAND_OF_RAGNAROS, 25000, 0, PHASE_1);
            events.ScheduleEvent(EVENT_CHECK_DIST, 6000);

            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);

            Map::PlayerList const& player = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator itr = player.begin(); itr != player.end(); ++itr)
                if (Player* player = itr->getSource())
                    if (player->hasQuest(QUEST_HEART_OF_FIRE))
                        _heartQuest = true;

            if (instance)
                instance->SetData(DATA_RAGNAROS_EVENT, IN_PROGRESS);
        }

        void JustDied(Unit* killer)
        {
            HandleDoor();
            _JustDied();
            Cleanup();

            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SUPERHEATED_TRIGGERED);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CLOUDBURST_PLAYER_AURA);
            me->RemoveDynObjectInDistance(SPELL_DREADFLAME_DAMAGE_AURA, 200.0f);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            if (!IsHeroic())
            {
                Talk(SAY_DEATH_NORMAL);
                me->SetHealth(1);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
                me->SetStandState(UNIT_STAND_STATE_SUBMERGED);
                me->HandleEmoteCommand(EMOTE_ONESHOT_SUBMERGE);
                me->DespawnOrUnsummon(6000);
                instance->DoUpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, 102237);

                if (_heartQuest)
                    DoCast(SPELL_HEART_OF_RAGNAROS_SUMMON);
            }
            else
                Talk(SAY_DEATH_HEROIC);

            if (Creature* malfurion = me->FindNearestCreature(NPC_MALFURION, 200.0f, true))
            {
                malfurion->AI()->DoAction(ACTION_SCHEDULE_OUTRO);
                malfurion->DespawnOrUnsummon(60000);
            }

            if (Creature* cenarius = me->FindNearestCreature(NPC_CENARIUS, 200.0f, true))
            {
                cenarius->AI()->DoAction(ACTION_SCHEDULE_OUTRO);
                cenarius->DespawnOrUnsummon(60000);
            }

            if (Creature* hamuul = me->FindNearestCreature(NPC_HAMUUL, 200.0f, true))
            {
                hamuul->AI()->DoAction(ACTION_SCHEDULE_OUTRO);
                hamuul->DespawnOrUnsummon(60000);
            }

            if (killer)
                killer->SummonGameObject(RAID_MODE(GO_CACHE_OF_THE_FIRELORD_10N, GO_CACHE_OF_THE_FIRELORD_25N, GO_CACHE_OF_THE_FIRELORD_10H, GO_CACHE_OF_THE_FIRELORD_25H), 1012.48999f, -57.2882004f, 55.3302002f, 4.41094017f, 0, 0, 1, 1, 0);
            else
            {
                if (Player* plr = me->FindNearestPlayer(100.0f))
                    plr->SummonGameObject(RAID_MODE(GO_CACHE_OF_THE_FIRELORD_10N, GO_CACHE_OF_THE_FIRELORD_25N, GO_CACHE_OF_THE_FIRELORD_10H, GO_CACHE_OF_THE_FIRELORD_25H), 1012.48999f, -57.2882004f, 55.3302002f, 4.41094017f, 0, 0, 1, 1, 0);
            }


            if (instance)
                instance->SetData(DATA_RAGNAROS_EVENT, DONE);
        }

        void EnterEvadeMode()
        {
            _EnterEvadeMode();
            HandleDoor();
            Cleanup();
            summons.DespawnAll();
            if (instance)
                instance->SetData(DATA_RAGNAROS_EVENT, NOT_STARTED);

            if (Creature* malfurion = me->FindNearestCreature(NPC_MALFURION, 200.0f, true))
                malfurion->DespawnOrUnsummon(0);
            if (Creature* cenarius = me->FindNearestCreature(NPC_CENARIUS, 200.0f, true))
                cenarius->DespawnOrUnsummon(0);
            if (Creature* hamuul = me->FindNearestCreature(NPC_HAMUUL, 200.0f, true))
                hamuul->DespawnOrUnsummon(0);


            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SUPERHEATED_TRIGGERED);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CLOUDBURST_PLAYER_AURA);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

            instance->SetBossState(DATA_RAGNAROS, NOT_STARTED);

            me->RemoveDynObjectInDistance(SPELL_DREADFLAME_DAMAGE_AURA, 200.0f);
            me->SetReactState(REACT_DEFENSIVE);
            me->DespawnOrUnsummon(1);
        }

        void IsSummonedBy(Unit* /*summoner*/)
        {
            events.SetPhase(PHASE_INTRO);
            events.ScheduleEvent(EVENT_INTRO, 5500, 0, PHASE_INTRO);
            me->setActive(true);
            me->PlayOneShotAnimKit(ANIM_KIT_EMERGE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            Position pos;
            pos.Relocate(me);
            me->SetHomePosition(pos);
            me->SetHover(true);
            me->SetDisableGravity(true);
            if (magma = me->FindNearestCreature(NPC_MAGMA_POOL_TRIGGER, 20.0f, true))
            {
                magma->AddAura(SPELL_MAGMA_PERIODIC, magma);
                magma->setFaction(me->getFaction());
            }
        }

        void KilledUnit(Unit* killed)
        {
            if (killed->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_SLAY);
        }

        void JustSummoned(Creature* summon)
        {
            switch (summon->GetEntry())
            {
            case NPC_SULFURAS_SMASH_TARGET:
                summons.Summon(summon);
                summon->AddAura(SPELL_LAVA_POOL, summon);
                break;
            case NPC_SON_OF_FLAME:
                summons.Summon(summon);
                DoZoneInCombat(summon);
                summon->AttackStop();
                summon->StopMoving();
                summon->SetReactState(REACT_PASSIVE);
                summon->AddAura(SPELL_HIT_ME, summon);
                summon->AddAura(SPELL_PRE_VISUAL, summon);
                summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
                break;
            case NPC_LIVING_METEOR:
            case NPC_LAVA_SCION:
            case NPC_DREADFLAME_SPAWN:
            case NPC_DREADFLAME:
                summons.Summon(summon);
                break;
            default:
                break;
            }
        }

        void HandleDoor() // Needed because the instancescript needs db guids for door datas
        {
            if (GameObject* door = me->FindNearestGameObject(GO_RAGNAROS_DOOR, 200.0f))
            {
                if (door->GetGoState() == GO_STATE_ACTIVE)
                    door->SetGoState(GO_STATE_READY);
                else
                    door->SetGoState(GO_STATE_ACTIVE);
            }
        }

        void Cleanup()
        {
            std::list<Creature*> units;

            GetCreatureListWithEntryInGrid(units, me, NPC_SULFURAS_HAND_OF_RAGNAROS, 200.0f);
            for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                (*itr)->DespawnOrUnsummon();

            GetCreatureListWithEntryInGrid(units, me, NPC_MAGMA_TRAP, 200.0f);
            for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                (*itr)->DespawnOrUnsummon();

            GetCreatureListWithEntryInGrid(units, me, NPC_DREADFLAME_SPAWN, 200.0f);
            for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                (*itr)->DespawnOrUnsummon();

            GetCreatureListWithEntryInGrid(units, me, NPC_MOLTEN_ELEMENTAL, 200.0f);
            for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                (*itr)->DespawnOrUnsummon();

            GetCreatureListWithEntryInGrid(units, me, NPC_CLOUDBURST, 200.0f);
            for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                (*itr)->DespawnOrUnsummon();

            GetCreatureListWithEntryInGrid(units, me, NPC_ENTRAPPING_ROOTS, 200.0f);
            for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                (*itr)->DespawnOrUnsummon();

            GetCreatureListWithEntryInGrid(units, me, NPC_BREADTH_OF_FROST, 200.0f);
            for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                (*itr)->DespawnOrUnsummon();

            GetCreatureListWithEntryInGrid(units, me, NPC_SPLITTING_BLOW_TRIGGER, 200.0f);
            for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                (*itr)->DespawnOrUnsummon();
        }

        void CleanSubmerge()
        {
            std::list<Creature*> units;

            GetCreatureListWithEntryInGrid(units, me, NPC_SULFURAS_HAND_OF_RAGNAROS, 200.0f);
            for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                (*itr)->DespawnOrUnsummon();

            GetCreatureListWithEntryInGrid(units, me, NPC_SPLITTING_BLOW_TRIGGER, 200.0f);
            for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                (*itr)->DespawnOrUnsummon();
        }

        void DamageTaken(Unit* attacker, uint32& damage)
        {
            if (me->HealthBelowPct(70) && _submergeCounter == 0)
            {
                _submergeCounter++;

                std::list<Creature*> units;
                GetCreatureListWithEntryInGrid(units, me, NPC_SULFURAS_SMASH_TARGET, 200.0f);
                for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                    (*itr)->DespawnOrUnsummon();

                events.SetPhase(PHASE_SUBMERGED);
                events.ScheduleEvent(EVENT_SPLITTING_BLOW, 1, 0, PHASE_SUBMERGED);
            }
            else if (me->HealthBelowPct(40) && _submergeCounter == 1)
            {
                _submergeCounter++;

                std::list<Creature*> units;
                GetCreatureListWithEntryInGrid(units, me, NPC_SULFURAS_SMASH_TARGET, 200.0f);
                for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                    (*itr)->DespawnOrUnsummon();

                events.SetPhase(PHASE_SUBMERGED);
                events.ScheduleEvent(EVENT_SPLITTING_BLOW, 1, 0, PHASE_SUBMERGED);
            }
            else if (me->HealthBelowPct(10) && _submergeCounter == 2)
            {
                _submergeCounter++;

                if (!IsHeroic())
                {
                    summons.DespawnAll();
                    me->Kill(me);
                }
                else
                {
                    DoAction(ACTION_ACTIVATE_HEROIC);
                }

            }
        }

        void DoAction(int32 action)
        {
            switch (action)
            {
            case ACTION_SON_KILLED:
            {
                _sonCounter++;
                if (_sonCounter == 8)
                {
                    DoAction(ACTION_INSTANT_EMERGE);
                    _sonCounter = 0;
                }
                break;
            }
            case ACTION_INSTANT_EMERGE:
                events.CancelEvent(EVENT_ANNOUNCE_EMERGE);
                events.CancelEvent(EVENT_EMERGE);
                events.ScheduleEvent(EVENT_EMERGE, 1000);
                break;
            case ACTION_ACTIVATE_HEROIC:
                if (Creature* cenarius = me->SummonCreature(NPC_CENARIUS, CenariusSummonPosition, TEMPSUMMON_MANUAL_DESPAWN, 0))
                    me->SetFacingToObject(cenarius);
                events.Reset();

                me->SummonCreature(NPC_MALFURION, MalfurionSummonPosition, TEMPSUMMON_MANUAL_DESPAWN, 0);
                me->SummonCreature(NPC_HAMUUL, HamuulSummonPosition, TEMPSUMMON_MANUAL_DESPAWN, 0);
                break;
            case ACTION_SUBMERGE:
                Talk(SAY_SUBMERGE_HEROIC);
                me->CastStop();
                me->AttackStop();
                me->SetReactState(REACT_PASSIVE);
                events.Reset();
                me->RemoveAllAuras();
                me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
                me->SetStandState(UNIT_STAND_STATE_SUBMERGED);
                me->HandleEmoteCommand(EMOTE_ONESHOT_SUBMERGE);
                events.ScheduleEvent(EVENT_TRANSFORM_RAGNAROS, 3000);
                events.ScheduleEvent(EVENT_EMERGE_HEROIC, 14500);
                break;
            default:
                break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!(events.IsInPhase(PHASE_INTRO)))
                if (!UpdateVictim())
                    return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_INTRO:
                    Talk(SAY_ARRIVE);
                    me->AddAura(SPELL_BASE_VISUAL, me);
                    me->AddAura(SPELL_BURNING_WOUNDS_AURA, me);
                    me->PlayOneShotAnimKit(ANIM_KIT_TAUNT);
                    break;
                case EVENT_ATTACK:
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->Attack(me->getVictim(), false);
                    me->AddAura(SPELL_BASE_VISUAL, me);
                    me->AddAura(SPELL_BURNING_WOUNDS_AURA, me);
                    break;
                case EVENT_SULFURAS_SMASH_TRIGGER:
                    if (me->HasAura(SPELL_WOLRD_IN_FLAMES) || me->HasUnitState(UNIT_STATE_CASTING))
                        events.ScheduleEvent(EVENT_SULFURAS_SMASH_TRIGGER, 1000);
                    else if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true, 0))
                    {
                        me->SetFacingToObject(target);
                        DoCast(SPELL_SULFURAS_SMASH_DUMMY);
                        events.ScheduleEvent(EVENT_SULFURAS_SMASH, 500);
                        if (events.IsInPhase(PHASE_1))
                            events.ScheduleEvent(EVENT_SULFURAS_SMASH_TRIGGER, 29500, 0, PHASE_1);
                        else if (events.IsInPhase(PHASE_2))
                            events.ScheduleEvent(EVENT_SULFURAS_SMASH_TRIGGER, 39500, 0, PHASE_2);
                        else if (events.IsInPhase(PHASE_3))
                            events.ScheduleEvent(EVENT_SULFURAS_SMASH_TRIGGER, 29500, 0, PHASE_3);
                    }
                    break;
                case EVENT_SULFURAS_SMASH:
                    if (Unit* trigger = me->FindNearestCreature(NPC_SULFURAS_SMASH_TRIGGER, 60.0f, true))
                    {
                        Talk(SAY_ANNOUNCE_SULFURAS_SMASH);
                        me->AttackStop();
                        me->SetReactState(REACT_PASSIVE);
                        me->SetFacingToObject(trigger);
                        DoCast(SPELL_SULFURAS_SMASH);
                        DoCast(SPELL_SULFURAS_SMASH_TARGET);
                        events.ScheduleEvent(EVENT_ATTACK, 6000);
                        events.ScheduleEvent(EVENT_WRATH_OF_RAGNAROS, 12000, 0, PHASE_1);
                    }
                    break;
                case EVENT_MAGMA_TRAP:
                    Talk(SAY_MAGMA_TRAP);
                    if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 0, true, 0))
                        DoCast(target, SPELL_MAGMA_TRAP);
                    events.ScheduleEvent(EVENT_MAGMA_TRAP, urand(25000, 30000), 0, PHASE_1);
                    break;
                case EVENT_WRATH_OF_RAGNAROS:
                    DoCastAOE(SPELL_WRATH_OF_RAGNAROS);
                    break;
                case EVENT_HAND_OF_RAGNAROS:
                    DoCastAOE(SPELL_HAND_OF_RAGNAROS);
                    events.ScheduleEvent(EVENT_HAND_OF_RAGNAROS, 25000, 0, PHASE_1);
                    break;
                case EVENT_CHECK_DIST:
                    if (!me->IsWithinMeleeRange(me->getVictim()))
                        me->CastSpell(me->getVictim(), SPELL_MAGMA_STRIKE, true);

                    events.ScheduleEvent(EVENT_CHECK_DIST, 1000);
                    break;
                case EVENT_SPLITTING_BLOW:
                {
                    if (me->HasUnitState(UNIT_STATE_CASTING))
                        events.ScheduleEvent(EVENT_SPLITTING_BLOW, 1000);
                    else
                    {
                        events.CancelEvent(EVENT_ATTACK);
                        events.ScheduleEvent(EVENT_CALL_SONS, 10000, 0, PHASE_SUBMERGED);

                        std::list<Creature*> units;
                        GetCreatureListWithEntryInGrid(units, me, NPC_SULFURAS_SMASH_TARGET, 200.0f);
                        for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                            (*itr)->DespawnOrUnsummon(0);

                        me->CastStop();
                        me->AttackStop();
                        me->SetReactState(REACT_PASSIVE);
                        Talk(SAY_ANNOUNCE_SPLIT);
                        Talk(SAY_SUBMERGE);
                        DoCast(SPELL_SUBMERGE);

                        switch (urand(0, 2))
                        {
                        case 0: // Splitting Blow East
                        {
                            if (Creature* trigger = me->SummonCreature(NPC_SPLITTING_BLOW_TRIGGER, SplittingTriggerEast, TEMPSUMMON_MANUAL_DESPAWN))
                            {
                                me->SetFacingToObject(trigger);
                                DoCastAOE(SPELL_SPLITTING_BLOW_EAST);

                                for (uint32 x = 0; x < 8; ++x)
                                    me->SummonCreature(NPC_SON_OF_FLAME, SonsOfFlameEast[x], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                            }
                            break;
                        }
                        case 1: // Splitting Blow West
                        {
                            if (Creature* trigger = me->SummonCreature(NPC_SPLITTING_BLOW_TRIGGER, SplittingTriggerWest, TEMPSUMMON_MANUAL_DESPAWN))
                            {
                                me->SetFacingToObject(trigger);
                                DoCastAOE(SPELL_SPLITTING_BLOW_WEST);

                                for (uint32 x = 0; x < 8; ++x)
                                    me->SummonCreature(NPC_SON_OF_FLAME, SonsOfFlameWest[x], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                            }
                            break;
                        }
                        case 2: // Splitting Blow North
                        {
                            if (Creature* trigger = me->SummonCreature(NPC_SPLITTING_BLOW_TRIGGER, SplittingTriggerNorth, TEMPSUMMON_MANUAL_DESPAWN))
                            {
                                me->SetFacingToObject(trigger);
                                DoCastAOE(SPELL_SPLITTING_BLOW_NORTH);

                                for (uint32 x = 0; x < 8; ++x)
                                    me->SummonCreature(NPC_SON_OF_FLAME, SonsOfFlameNorth[x], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                            }
                            break;
                        }
                        }
                    }
                    break;
                }
                case EVENT_CALL_SONS:
                    if (_submergeCounter == 0)
                        Talk(SAY_SONS_OF_FLAME_1);
                    else
                        Talk(SAY_SONS_OF_FLAME_2);

                    me->RemoveAurasDueToSpell(SPELL_BASE_VISUAL);
                    events.ScheduleEvent(EVENT_ANNOUNCE_EMERGE, 40000, 0, PHASE_SUBMERGED);
                    events.ScheduleEvent(EVENT_EMERGE, 45000, 0, PHASE_SUBMERGED);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    if (_submergeCounter == 2)
                        for (uint32 x = 0; x < 2; ++x)
                            me->SummonCreature(NPC_LAVA_SCION, LavaScionPos[x], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                    break;
                case EVENT_ANNOUNCE_EMERGE:
                    Talk(SAY_ANNOUNCE_EMERGE);
                    break;
                case EVENT_EMERGE:
                {
                    Talk(SAY_EMERGE);
                    CleanSubmerge();
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->RemoveAllAuras();
                    me->AddAura(SPELL_BASE_VISUAL, me);
                    me->PlayOneShotAnimKit(ANIM_KIT_RESURFACE);
                    events.ScheduleEvent(EVENT_ATTACK, 4500);

                    std::list<Creature*> units;
                    GetCreatureListWithEntryInGrid(units, me, NPC_SON_OF_FLAME, 200.0f);
                    for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                    {
                        if (!(*itr)->isDead())
                        {
                            (*itr)->CastSpell(*itr, SPELL_SUPERNOVA);
                            (*itr)->Kill(*itr);
                        }
                    }

                    if (_submergeCounter == 1)
                    {
                        events.SetPhase(PHASE_2);
                        if (IsHeroic())
                        {
                            events.ScheduleEvent(EVENT_MOLTEN_SEED, urand(14800, 16000), 0, PHASE_2);
                            events.ScheduleEvent(EVENT_SULFURAS_SMASH_TRIGGER, 6000, 0, PHASE_2);
                        }
                        else
                        {
                            events.ScheduleEvent(EVENT_MOLTEN_SEED, 21500, 0, PHASE_2);
                            events.ScheduleEvent(EVENT_SULFURAS_SMASH_TRIGGER, 15000, 0, PHASE_2);
                        }
                        events.ScheduleEvent(EVENT_ENGULFING_FLAMES, 40000, 0, PHASE_2);
                    }
                    if (_submergeCounter == 2)
                    {
                        events.SetPhase(PHASE_3);
                        events.ScheduleEvent(EVENT_SULFURAS_SMASH_TRIGGER, 15500, 0, PHASE_3);
                        events.ScheduleEvent(EVENT_ENGULFING_FLAMES, 30000, 0, PHASE_3);
                        events.ScheduleEvent(EVENT_LIVING_METEOR, 45000, 0, PHASE_3);
                    }
                    break;
                }
                case EVENT_ENGULFING_FLAMES:
                    if (!IsHeroic())
                    {
                        Talk(SAY_ENGULFING_FLAMES);
                        switch (urand(0, 2))
                        {
                        case 0: // Melee
                        {
                            for (uint32 x = 0; x < 16; ++x)
                                me->SummonCreature(NPC_ENGULFING_FLAMES_TRIGGER, EngulfingFlamesMelee[x], TEMPSUMMON_TIMED_DESPAWN, 18000);

                            DoCastAOE(SPELL_ENGULFING_FLAMES_VISUAL_MELEE);
                            DoCastAOE(SPELL_ENGULFING_FLAMES_MELEE);
                            break;
                        }
                        case 1: // Range
                        {
                            for (uint32 x = 0; x < 35; ++x)
                                me->SummonCreature(NPC_ENGULFING_FLAMES_TRIGGER, EngulfingFlamesRange[x], TEMPSUMMON_TIMED_DESPAWN, 18000);

                            DoCastAOE(SPELL_ENGULFING_FLAMES_VISUAL_BOTTOM);
                            DoCastAOE(SPELL_ENGULFING_FLAMES_BOTTOM);
                            break;
                        }
                        case 2: // Center
                        {
                            for (uint32 x = 0; x < 17; ++x)
                                me->SummonCreature(NPC_ENGULFING_FLAMES_TRIGGER, EngulfingFlamesCenter[x], TEMPSUMMON_TIMED_DESPAWN, 18000);

                            DoCastAOE(SPELL_ENGULFING_FLAMES_VISUAL_CENTER);
                            DoCastAOE(SPELL_ENGULFING_FLAMES_CENTER);
                            break;
                        }
                        }
                    }
                    else
                    {
                        Talk(SAY_WORLD_IN_FLAMES);
                        me->AddAura(SPELL_WOLRD_IN_FLAMES, me);
                    }

                    if (_submergeCounter == 1)
                        events.ScheduleEvent(EVENT_ENGULFING_FLAMES, 60000, 0, PHASE_2);
                    else if (_submergeCounter == 2)
                        events.ScheduleEvent(EVENT_ENGULFING_FLAMES, 30000, 0, PHASE_3);
                    break;
                case EVENT_MOLTEN_SEED:
                    DoCastAOE(SPELL_MOLTEN_SEED);
                    events.ScheduleEvent(EVENT_MOLTEN_SEED, 60000, 0, PHASE_2);
                    break;
                case EVENT_LIVING_METEOR:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true, 0))
                        DoCast(target, SPELL_LIVING_METEOR_MISSILE);
                    events.ScheduleEvent(EVENT_LIVING_METEOR, 55000, 0, PHASE_3);
                    break;
                case EVENT_EMERGE_HEROIC:
                    me->SetStandState(0);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->PlayOneShotAnimKit(ANIM_KIT_EMERGE);
                    events.ScheduleEvent(EVENT_TALK, 3250);
                    events.ScheduleEvent(EVENT_FREEZE_PLATFORM, 200);
                    events.ScheduleEvent(EVENT_TAUNT_EMOTE, 5500);
                    break;
                case EVENT_TALK:
                    Talk(SAY_INTRO_HEROIC_1);
                    me->SendSetPlayHoverAnim(true);
                    me->SetHover(true);
                    if (Creature* cenarius = me->FindNearestCreature(NPC_CENARIUS, 200.0f))
                        cenarius->CastStop();
                    if (Creature* hamuul = me->FindNearestCreature(NPC_HAMUUL, 200.0f))
                        hamuul->CastStop();
                    if (Creature* malfurion = me->FindNearestCreature(NPC_MALFURION, 200.0f))
                        malfurion->CastStop();
                    events.ScheduleEvent(EVENT_STANDUP, 9400);
                    events.ScheduleEvent(EVENT_BREAK_PLATFORM, 9400);
                    break;
                case EVENT_TAUNT_EMOTE:
                    me->PlayOneShotAnimKit(ANIM_KIT_TAUNT);
                    break;
                case EVENT_FREEZE_PLATFORM:
                    if (GameObject* platform = me->FindNearestGameObject(GO_RAGNAROS_PLATFORM, 200.0f))
                        platform->SetDestructibleState(GO_DESTRUCTIBLE_DAMAGED);
                    if (magma)
                        magma->RemoveAllAuras();
                    break;
                case EVENT_STANDUP:
                    Talk(SAY_INTRO_HEROIC_2);
                    me->RemoveAurasDueToSpell(SPELL_BASE_VISUAL);
                    me->SendSetPlayHoverAnim(false);
                    me->SetDisableGravity(false);
                    me->PlayOneShotAnimKit(ANIM_KIT_STAND_UP);
                    events.ScheduleEvent(EVENT_ATTACK_HEROIC, 7500);
                    break;
                case EVENT_BREAK_PLATFORM:
                    if (GameObject* platform = me->FindNearestGameObject(GO_RAGNAROS_PLATFORM, 200.0f))
                        platform->SetDestructibleState(GO_DESTRUCTIBLE_DESTROYED);
                    break;
                case EVENT_ATTACK_HEROIC:
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                    me->SetSpeed(MOVE_RUN, 2.0f, true);
                    me->SetSpeed(MOVE_WALK, 2.0f, true);
                    me->SetReactState(REACT_AGGRESSIVE);
                    DoCast(SPELL_SUPERHEATED);
                    events.ScheduleEvent(EVENT_SCHEDULE_EMPOWER, urand(56000, 64000));
                    events.ScheduleEvent(EVENT_SUMMON_DREADFLAME, 9000);
                    if (Creature* malfurion = me->FindNearestCreature(NPC_MALFURION, 200.0f, true))
                        malfurion->AI()->DoAction(ACTION_SCHEDULE_CLOUDBURST);
                    if (Creature* cenarius = me->FindNearestCreature(NPC_CENARIUS, 200.0f, true))
                        cenarius->AI()->DoAction(ACTION_SCHEDULE_BREADTH);
                    break;
                case EVENT_TRANSFORM_RAGNAROS:
                    DoCast(me, SPELL_LEGS_HEAL);
                    DoCast(me, SPELL_TRANSFORM);
                    me->SetFloatValue(UNIT_FIELD_COMBATREACH, 21);

                    break;
                case EVENT_SCHEDULE_EMPOWER:
                    if (Creature* hamuul = me->FindNearestCreature(NPC_HAMUUL, 200.0f, true))
                        hamuul->AI()->DoAction(ACTION_SCHEDULE_ROOTS);
                    events.ScheduleEvent(EVENT_EMPOWER_SULFURAS, urand(3000, 11000));
                    events.ScheduleEvent(EVENT_SCHEDULE_EMPOWER, urand(56000, 64000));
                    break;
                case EVENT_EMPOWER_SULFURAS:
                {
                    Talk(SAY_EMPOWER_SULFURAS);
                    std::list<Creature*> units; // Temphack
                    GetCreatureListWithEntryInGrid(units, me, NPC_MOLTEN_SEED_CASTER, 500.0f);
                    for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                        (*itr)->AddAura(SPELL_EMPOWER_SULFURAS_TRIGGER, (*itr));
                    DoCast(SPELL_EMPOWER_SULFURAS);
                    break;
                }
                case EVENT_SUMMON_DREADFLAME:
                    Talk(SAY_DREADFLAME);
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true, 0))
                    {
                        if (Unit* target2 = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true, 0))
                        {
                            DoCast(target, SPELL_DREADFLAME_SUMMON_MISSILE);
                            DoCast(target2, SPELL_DREADFLAME_SUMMON_MISSILE);
                        }
                    }
                    events.ScheduleEvent(EVENT_SUMMON_DREADFLAME, 40000);
                    break;
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_ragnaros_cataAI(creature);
    }
};

class npc_fl_magma_trap : public CreatureScript
{
public:
    npc_fl_magma_trap() : CreatureScript("npc_fl_magma_trap") { }

    struct npc_fl_magma_trapAI : public ScriptedAI
    {
        npc_fl_magma_trapAI(Creature* creature) : ScriptedAI(creature)
        {
            _exploded = false;
        }

        bool _exploded;

        void IsSummonedBy(Unit* /*summoner*/)
        {
            events.ScheduleEvent(EVENT_PREPARE_TRAP, 5000);
            me->AddAura(SPELL_MAGMA_TRAP_VISUAL, me);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
            SetCombatMovement(false);
            _exploded = false;
        }

        void SpellHitTarget(Unit* /*target*/, SpellInfo const* spell)
        {
            if (spell->Id == SPELL_MAGMA_TRAP_PERIODIC_TICK && !_exploded)
            {
                DoCastAOE(SPELL_MAGMA_TRAP_ERUPTION);
                me->RemoveAurasDueToSpell(SPELL_MAGMA_TRAP_VISUAL);
                _exploded = true;
                me->DespawnOrUnsummon(3000);
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_PREPARE_TRAP:
                    DoCast(SPELL_MAGMA_TRAP_PERIODIC);
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
        return new npc_fl_magma_trapAI(creature);
    }
};

class npc_fl_sulfuras_smash : public CreatureScript
{
public:
    npc_fl_sulfuras_smash() : CreatureScript("npc_fl_sulfuras_smash") { }

    struct npc_fl_sulfuras_smashAI : public ScriptedAI
    {
        npc_fl_sulfuras_smashAI(Creature* creature) : ScriptedAI(creature)
        {
            _summonCounter = 0;
        }

        uint8 _summonCounter;

        void IsSummonedBy(Unit* summoner)
        {
            events.ScheduleEvent(EVENT_SCORCH, 3400);
            events.ScheduleEvent(EVENT_SUMMON_WAVE_1, 3400);
            me->SetOrientation(summoner->GetOrientation());
            me->setFaction(summoner->getFaction());
        }

        void JustSummoned(Creature* summon)
        {
            summon->setFaction(me->getFaction());
            _summonCounter++;
            if (_summonCounter == 2)
                summon->SetOrientation(me->GetOrientation() + M_PI / 2);
            else if (_summonCounter == 3)
            {
                summon->SetOrientation(me->GetOrientation() - M_PI / 2);
                _summonCounter = 0;
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SCORCH:
                    me->RemoveAurasDueToSpell(SPELL_LAVA_POOL);
                    me->AddAura(SPELL_SCORCHED_GROUND, me);
                    break;
                case EVENT_SUMMON_WAVE_1:
                    DoCastAOE(SPELL_LAVA_WAVE_DUMMY_NORTH);
                    events.ScheduleEvent(EVENT_SUMMON_WAVE_2, 50);
                    break;
                case EVENT_SUMMON_WAVE_2:
                    DoCastAOE(SPELL_LAVA_WAVE_DUMMY_WEST);
                    events.ScheduleEvent(EVENT_SUMMON_WAVE_3, 50);
                    break;
                case EVENT_SUMMON_WAVE_3:
                    DoCastAOE(SPELL_LAVA_WAVE_DUMMY_EAST);
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
        return new npc_fl_sulfuras_smashAI(creature);
    }
};

class npc_fl_lava_wave : public CreatureScript
{
public:
    npc_fl_lava_wave() : CreatureScript("npc_fl_lava_wave") { }

    struct npc_fl_lava_waveAI : public ScriptedAI
    {
        npc_fl_lava_waveAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void IsSummonedBy(Unit* summoner)
        {
            events.ScheduleEvent(EVENT_MOVE_LAVA_WAVE, 100);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_LEAP_BACK, true); // Prevent the knockback if the wave hits an player
            me->SetSpeed(MOVE_RUN, 1.6f);
            me->SetReactState(REACT_PASSIVE);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_MOVE_LAVA_WAVE:
                    DoCast(SPELL_LAVA_WAVE);
                    me->GetMotionMaster()->MovePoint(0, me->GetPositionX() + cos(me->GetOrientation()) * 100, me->GetPositionY() + sin(me->GetOrientation()) * 100, me->GetPositionZ(), false);
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
        return new npc_fl_lava_waveAI(creature);
    }
};

class npc_fl_sulfuras_hammer : public CreatureScript // Temphack until dest area effects can affect npc's
{
public:
    npc_fl_sulfuras_hammer() : CreatureScript("npc_fl_sulfuras_hammer") { }

    struct npc_fl_sulfuras_hammerAI : public ScriptedAI
    {
        npc_fl_sulfuras_hammerAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        EventMap events;

        void IsSummonedBy(Unit* /*summoner*/)
        {
            me->AddAura(SPELL_SULFURAS_AURA, me);
            DoCastAOE(SPELL_INVOKE_SONS);
            events.ScheduleEvent(EVENT_CHECK_SONS, 500);
        }

        void SpellHitTarget(Unit* target, SpellInfo const* spell)
        {
            if (spell->Id == SPELL_INVOKE_SONS_TRIGGERED_SPELL && target->GetEntry() == NPC_SON_OF_FLAME)
            {
                if (Creature* son = target->ToCreature())
                    son->AI()->DoAction(ACTION_ACTIVATE_SON);
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_CHECK_SONS:
                {
                    std::list<Creature*> units;
                    GetCreatureListWithEntryInGrid(units, me, NPC_SON_OF_FLAME, 5.0f);
                    for (std::list<Creature*>::iterator itr = units.begin(); itr != units.end(); ++itr)
                    {
                        if ((*itr)->HasAura(SPELL_HIT_ME))
                        {
                            (*itr)->RemoveAurasDueToSpell(SPELL_HIT_ME);
                            (*itr)->StopMoving();
                            (*itr)->CastSpell((*itr), SPELL_SUPERNOVA);
                            me->Kill((*itr), false);
                        }
                    }
                    events.ScheduleEvent(EVENT_CHECK_SONS, 500);
                    break;
                }
                default:
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fl_sulfuras_hammerAI(creature);
    }
};

class npc_fl_son_of_flame : public CreatureScript
{
public:
    npc_fl_son_of_flame() : CreatureScript("npc_fl_son_of_flame") { }

    struct npc_fl_son_of_flameAI : public ScriptedAI
    {
        npc_fl_son_of_flameAI(Creature* creature) : ScriptedAI(creature), instance(creature->GetInstanceScript())
        {
        }

        InstanceScript* instance;
        EventMap events;

        void JustDied(Unit* /*killer*/)
        {
            if (Creature* ragnaros = me->FindNearestCreature(BOSS_RAGNAROS, 200.0f, true))
                ragnaros->AI()->DoAction(ACTION_SON_KILLED);
        }

        void DamageTaken(Unit* /*damager*/, uint32& damage) // Temphack
        {
            uint32 curStacks = 0;
            if (Aura* speed = me->GetAura(SPELL_BURNING_SPEED_STACKS))
            {
                curStacks = speed->GetStackAmount();

                uint32 pctCheck = 100 - ((11 - curStacks) * 5);

                if (me->HealthBelowPct(pctCheck))
                    me->SetAuraStack(SPELL_BURNING_SPEED_STACKS, me, curStacks - 1);
            }
            if (me->HealthBelowPct(50))
            {
                me->RemoveAurasDueToSpell(SPELL_BURNING_SPEED_STACKS);
                me->RemoveAurasDueToSpell(SPELL_BURNING_SPEED);
            }
        }

        void DoAction(int32 action)
        {
            switch (action)
            {
            case ACTION_ACTIVATE_SON:
                me->RemoveAurasDueToSpell(SPELL_PRE_VISUAL);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
                me->AddAura(SPELL_BURNING_SPEED, me);
                me->AddAura(SPELL_BURNING_SPEED_STACKS, me);
                me->SetAuraStack(SPELL_BURNING_SPEED_STACKS, me, 10);
                me->PlayOneShotAnimKit(ANIM_KIT_UNK_1);
                events.ScheduleEvent(EVENT_MOVE_HAMMER, 3000);
                break;
            default:
                break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_MOVE_HAMMER:
                    if (Creature* sulfuras = me->FindNearestCreature(NPC_SULFURAS_HAND_OF_RAGNAROS, 200.0f))
                        me->GetMotionMaster()->MoveFollow(sulfuras, 1.0f, 0);
                    break;
                default:
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fl_son_of_flameAI(creature);
    }
};

class npc_fl_engulfing_flame : public CreatureScript
{
public:
    npc_fl_engulfing_flame() : CreatureScript("npc_fl_engulfing_flame") { }

    struct npc_fl_engulfing_flameAI : public ScriptedAI
    {
        npc_fl_engulfing_flameAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void UpdateAI(uint32 diff)
        {
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fl_engulfing_flameAI(creature);
    }
};

class npc_fl_molten_elemental : public CreatureScript
{
public:
    npc_fl_molten_elemental() : CreatureScript("npc_fl_molten_elemental") { }

    struct npc_fl_molten_elementalAI : public ScriptedAI
    {
        npc_fl_molten_elementalAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        EventMap events;

        void IsSummonedBy(Unit* /*summoner*/)
        {
            DoCastAOE(SPELL_INVISIBLE_PRE_VISUAL);
            events.ScheduleEvent(EVENT_PREPARE_ELEMENTAL, 2000);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
            me->SetReactState(REACT_PASSIVE);

            if (Creature* seedCaster = me->FindNearestCreature(NPC_MOLTEN_SEED_CASTER, 100.0f, true))
                seedCaster->CastSpell(me, SPELL_MOLTEN_SEED_MISSILE);
        }

        void JustDied(Unit* /*killer*/)
        {
            me->DespawnOrUnsummon(5000);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_PREPARE_ELEMENTAL:
                    DoCastAOE(SPELL_MOLTEN_SEED_VISUAL);
                    events.ScheduleEvent(EVENT_ACTIVATE, 10000);
                    break;
                case EVENT_ACTIVATE:
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->RemoveAura(SPELL_INVISIBLE_PRE_VISUAL);
                    if (Unit* player = me->FindNearestPlayer(200.0f))
                        me->AI()->AttackStart(player);
                    break;
                default:
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fl_molten_elementalAI(creature);
    }
};

class npc_fl_lava_scion : public CreatureScript
{
public:
    npc_fl_lava_scion() : CreatureScript("npc_fl_lava_scion") { }

    struct npc_fl_lava_scionAI : public ScriptedAI
    {
        npc_fl_lava_scionAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void IsSummonedBy(Unit* summoner)
        {
            if (Unit* player = me->FindNearestPlayer(200.0f))
                me->AI()->AttackStart(player);
        }

        void EnterCombat(Unit* /*victim*/)
        {
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            events.ScheduleEvent(EVENT_BLAZING_HEAT, 12000);
        }

        void JustDied(Unit* /*killer*/)
        {
            me->DespawnOrUnsummon(5000);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_BLAZING_HEAT:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true, 0))
                        DoCast(target, SPELL_BLAZING_HEAT);
                    events.ScheduleEvent(EVENT_BLAZING_HEAT, 21000);
                    break;
                default:
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fl_lava_scionAI(creature);
    }
};

class npc_fl_blazing_heat : public CreatureScript
{
public:
    npc_fl_blazing_heat() : CreatureScript("npc_fl_blazing_heat") { }

    struct npc_fl_blazing_heatAI : public ScriptedAI
    {
        npc_fl_blazing_heatAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
            DoCastAOE(SPELL_BLAZING_HEAT_DAMAGE_AURA);
        }

        void UpdateAI(uint32 diff)
        {
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fl_blazing_heatAI(creature);
    }
};

class npc_fl_living_meteor : public CreatureScript
{
public:
    npc_fl_living_meteor() : CreatureScript("npc_fl_living_meteor") { }

    struct npc_fl_living_meteorAI : public ScriptedAI
    {
        npc_fl_living_meteorAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        Unit* target;
        EventMap events;
        bool canBack;

        void IsSummonedBy(Unit* summoner)
        {
            canBack = false;
            me->SetReactState(REACT_PASSIVE);
            me->SetInCombatWithZone();

            if (target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true, 0))
                me->CastSpell(target, SPELL_LIVING_METEOR_FIXATE, false);

            me->AddAura(SPELL_LIVING_METEOR_DAMAGE_REDUCTION, me);
            me->AddAura(SPELL_LIVING_METEOR_COMBUSTIBLE, me);

            events.ScheduleEvent(EVENT_STALK_PLAYER, 3000);

        }

        void DamageTaken(Unit* attacker, uint32 &damage)
        {
            if (!target)
                return;

            if (attacker->GetGUID() != target->GetGUID())
                return;

            if (canBack)
            {

                events.Reset();
                me->RemoveAllAuras();
                me->GetMotionMaster()->Clear();
                me->StopMoving();

                me->CastSpell(attacker, SPELL_LIVING_METEOR_COMBUSTITION, true);

                DoCastAOE(SPELL_LIVING_METEOR_COMBUSTITION);

                if (!me->HasAura(100567))
                    me->AddAura(SPELL_LIVING_METEOR_DAMAGE_REDUCTION, me);

                if (target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true, 0))
                {
                    events.ScheduleEvent(EVENT_STALK_PLAYER, 3000);
                    me->CastSpell(target, SPELL_LIVING_METEOR_FIXATE, false);
                }
                canBack = false;
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_STALK_PLAYER:
                    // DoCastAOE(SPELL_LIVING_METEOR_INCREASE_SPEED);
                    me->ClearUnitState(UNIT_STATE_CASTING);
                    if (target)
                        me->GetMotionMaster()->MoveFollow(target, 0.0f, 0.0f);

                    events.ScheduleEvent(EVENT_KILL_PLAYER, 500);
                    events.ScheduleEvent(EVENT_ENABLE_KNOCKBACK, 2000);
                    break;
                case EVENT_KILL_PLAYER:

                    events.ScheduleEvent(EVENT_KILL_PLAYER, 500);

                    if (target)
                    {
                        if (me->GetDistance(target) > 5.0f)
                            return;

                        events.Reset();
                        me->RemoveAllAuras();
                        canBack = false;
                        me->GetMotionMaster()->Clear();

                        if (!me->HasAura(100567))
                            me->AddAura(SPELL_LIVING_METEOR_DAMAGE_REDUCTION, me);

                        DoCastAOE(SPELL_LIVING_METEOR_EXPLOSION);
                        if (target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true, 0))
                        {
                            events.ScheduleEvent(EVENT_STALK_PLAYER, 3000);
                            me->CastSpell(target, SPELL_LIVING_METEOR_FIXATE, false);
                        }
                    }
                    else
                    {
                        if (target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true, 0))
                        {
                            events.ScheduleEvent(EVENT_STALK_PLAYER, 3000);
                            me->CastSpell(target, SPELL_LIVING_METEOR_FIXATE, false);
                        }
                    }
                    break;
                case EVENT_ENABLE_KNOCKBACK:
                    me->AddAura(SPELL_LIVING_METEOR_COMBUSTIBLE, me);
                    canBack = true;
                    break;
                default:
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fl_living_meteorAI(creature);
    }
};

class npc_fl_archdruids : public CreatureScript
{
public:
    npc_fl_archdruids() : CreatureScript("npc_fl_archdruids") { }

    struct npc_fl_archdruidsAI : public ScriptedAI
    {
        npc_fl_archdruidsAI(Creature* creature) : ScriptedAI(creature)
        {
            casted = false;
        }

        bool casted;

        void SpellHitTarget(Unit* target, SpellInfo const* spell)
        {
            if (spell->Id == SPELL_CLOUDBURST_DUMMY)
            {
                if (!casted)
                {
                    if (target->GetEntry() == NPC_PLATFORM_TRIGGER)
                    {
                        std::list<Creature*> list;
                        GetCreatureListWithEntryInGrid(list, me, NPC_PLATFORM_TRIGGER, 200.0f);
                        if (!list.empty())
                        {
                            std::list<Creature*>::iterator itr = list.begin();
                            std::advance(itr, urand(0, list.size() - 1));
                            DoCast((*itr), SPELL_CLOUDBURST_SUMMON);
                        }
                        casted = true;
                    }
                }
            }
        }

        void DoAction(int32 action)
        {
            switch (action)
            {
            case ACTION_SCHEDULE_CLOUDBURST:
                events.ScheduleEvent(EVENT_CLOUDBURST, 15000);
                break;
            case ACTION_SCHEDULE_ROOTS:
                events.ScheduleEvent(EVENT_ENTRAPPING_ROOTS, 1);
                break;
            case ACTION_SCHEDULE_BREADTH:
                events.ScheduleEvent(EVENT_BREADTH_OF_FROST, 1000);
                break;
            case ACTION_SCHEDULE_OUTRO:
                me->CastStop();
                events.CancelEvent(EVENT_CLOUDBURST);
                events.CancelEvent(EVENT_ENTRAPPING_ROOTS);
                events.CancelEvent(EVENT_BREADTH_OF_FROST);
                if (me->GetEntry() == NPC_MALFURION)
                    events.ScheduleEvent(EVENT_TALK_MALFURION_1, 7000);
                if (me->GetEntry() == NPC_CENARIUS)
                    events.ScheduleEvent(EVENT_TALK_CENARIUS_1, 10000);
                if (me->GetEntry() == NPC_HAMUUL)
                    events.ScheduleEvent(EVENT_TALK_HAMUUL_1, 26000);

                break;
            default:
                break;
            }
        }

        void IsSummonedBy(Unit* /*summoner*/)
        {
            if (me->GetEntry() == NPC_CENARIUS)
            {
                me->SetHover(true);
                me->SetCanFly(true);
                me->SetDisableGravity(true);
                me->SetSpeed(MOVE_RUN, 10.0f, true);
                me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                me->GetMotionMaster()->MovePoint(1, CenariusPoint.GetPositionX(), CenariusPoint.GetPositionY(), CenariusPoint.GetPositionZ(), false);
            }
            else if (me->GetEntry() == NPC_MALFURION)
            {
                me->SetHover(true);
                me->SetCanFly(true);
                me->SetDisableGravity(true);
                me->SetSpeed(MOVE_RUN, 10.0f, true);
                me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                me->GetMotionMaster()->MovePoint(1, MalfurionPoint.GetPositionX(), MalfurionPoint.GetPositionY(), MalfurionPoint.GetPositionZ(), false);
            }
            else if (me->GetEntry() == NPC_HAMUUL)
            {
                me->SetHover(true);
                me->SetCanFly(true);
                me->SetDisableGravity(true);
                me->SetSpeed(MOVE_RUN, 10.0f, true);
                me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                me->GetMotionMaster()->MovePoint(1, HamuulPoint.GetPositionX(), HamuulPoint.GetPositionY(), HamuulPoint.GetPositionZ(), false);
            }
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            switch (pointId)
            {
            case 1:
                me->SetHover(false);
                me->SetCanFly(false);
                me->SetDisableGravity(false);
                if (me->GetEntry() == NPC_CENARIUS)
                {
                    if (Creature* ragnaros = me->FindNearestCreature(BOSS_RAGNAROS, 200.0f, true))
                        ragnaros->AI()->DoAction(ACTION_SUBMERGE);
                    events.ScheduleEvent(EVENT_SAY_PULL, 3700);
                    events.ScheduleEvent(EVENT_DRAW_RAGNAROS, 9600);
                }
                else if (me->GetEntry() == NPC_MALFURION)
                {
                    DoCastAOE(SPELL_TRANSFORM_MALFURION);
                    events.ScheduleEvent(EVENT_TRANSFORM, 1000);
                    events.ScheduleEvent(EVENT_DRAW_RAGNAROS, 8300);
                }
                else if (me->GetEntry() == NPC_HAMUUL)
                {
                    DoCastAOE(SPELL_TRANSFORM_HAMUUL);
                    events.ScheduleEvent(EVENT_TRANSFORM, 1000);
                    events.ScheduleEvent(EVENT_DRAW_RAGNAROS, 7100);
                }
                break;
            default:
                break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SAY_PULL:
                    Talk(SAY_CENARIUS_1);
                    break;
                case EVENT_DRAW_RAGNAROS:
                    if (me->GetEntry() == NPC_CENARIUS)
                        DoCastAOE(SPELL_CENARIUS_DRAW_FIRELORD);
                    if (me->GetEntry() == NPC_HAMUUL)
                        DoCastAOE(SPELL_HAMUL_DRAW_FIRELORD);
                    if (me->GetEntry() == NPC_MALFURION)
                    {
                        DoCastAOE(SPELL_MALFURION_DRAW_FIRELORD);
                        events.ScheduleEvent(EVENT_SAY_CAUGHT, 14500);
                    }
                    break;
                case EVENT_TRANSFORM:
                    if (me->GetEntry() == NPC_MALFURION)
                        DoCastAOE(SPELL_TRANSFORM_MALFURION);
                    else if (me->GetEntry() == NPC_HAMUUL)
                        DoCastAOE(SPELL_TRANSFORM_HAMUUL);
                    break;
                case EVENT_SAY_CAUGHT:
                    Talk(SAY_MALFURION_1);
                    break;
                case EVENT_CLOUDBURST:
                    casted = false;
                    DoCastAOE(SPELL_CLOUDBURST_DUMMY);
                    break;
                case EVENT_ENTRAPPING_ROOTS:
                    if (Creature* ragnaros = me->FindNearestCreature(BOSS_RAGNAROS, 200.0f, true))
                        if (Unit* player = ragnaros->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true, 0))
                            if (Unit* root = player->FindNearestCreature(NPC_PLATFORM_TRIGGER, 200.0f, true))
                                if (Creature* trap = me->SummonCreature(NPC_ENTRAPPING_ROOTS, root->GetPositionX(), root->GetPositionY(), root->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 120000))
                                    DoCast(trap, SPELL_ENTRAPPING_ROOTS_AURA_MISSILE);
                    break;
                case EVENT_BREADTH_OF_FROST:
                    if (Creature* ragnaros = me->FindNearestCreature(BOSS_RAGNAROS, 200.0f, true))
                        if (Unit* player = ragnaros->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true, 0))
                            if (Unit* root = player->FindNearestCreature(NPC_PLATFORM_TRIGGER, 200.0f, true))
                                if (Creature* trap = me->SummonCreature(NPC_BREADTH_OF_FROST, root->GetPositionX(), root->GetPositionY(), root->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 120000))
                                    DoCast(trap, SPELL_BREADTH_OF_FROST_AURA);

                    events.ScheduleEvent(EVENT_BREADTH_OF_FROST, urand(90000, 100000));
                    break;
                case EVENT_TALK_MALFURION_1:
                    Talk(SAY_MALFURION_OUTRO_1);
                    events.ScheduleEvent(EVENT_TALK_MALFURION_2, 5000);
                    break;
                case EVENT_TALK_MALFURION_2:
                    Talk(SAY_MALFURION_OUTRO_2);
                    break;
                case EVENT_TALK_CENARIUS_1:
                    Talk(SAY_CENARIUS_OUTRO_1);
                    events.ScheduleEvent(EVENT_TALK_CENARIUS_2, 6000);
                    events.ScheduleEvent(EVENT_TALK_CENARIUS_3, 28000);
                    break;
                case EVENT_TALK_CENARIUS_2:
                    Talk(SAY_CENARIUS_OUTRO_2);
                    break;
                case EVENT_TALK_CENARIUS_3:
                    Talk(SAY_CENARIUS_OUTRO_3);
                    break;
                case EVENT_TALK_HAMUUL_1:
                    Talk(SAY_HAMUUL_OUTRO_1);
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
        return new npc_fl_archdruidsAI(creature);
    }
};

class npc_fl_dreadflame : public CreatureScript
{
public:
    npc_fl_dreadflame() : CreatureScript("npc_fl_dreadflame") { }

    struct npc_fl_dreadflameAI : public ScriptedAI
    {
        npc_fl_dreadflameAI(Creature* creature) : ScriptedAI(creature)
        {
            casted = false;
        }

        EventMap events;
        bool casted;
        uint8 flamecount;

        void IsSummonedBy(Unit* summoner)
        {
            flamecount = 0;
            if (Creature* ragnaros = me->FindNearestCreature(BOSS_RAGNAROS, 200.0, false))
            {
                if (ragnaros->isDead())
                    me->DespawnOrUnsummon(0);
            }
            else
            {
                me->SetReactState(REACT_PASSIVE);
                if (summoner->GetEntry() == BOSS_RAGNAROS)
                {
                    me->setFaction(summoner->getFaction());
                    me->SummonCreature(NPC_DREADFLAME_SPAWN, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_CORPSE_DESPAWN, 10000);
                }
                else
                {
                    events.ScheduleEvent(EVENT_CHECK_PLAYER, 500);
                    events.ScheduleEvent(EVENT_SPREAD_FLAME, 5000);
                    me->setFaction(summoner->getFaction());
                    DoCastAOE(SPELL_DREADFLAME_DAMAGE_AURA);
                }
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_CHECK_PLAYER:
                {
                    if (Player* player = me->FindNearestPlayer(0.1f))
                    {
                        if (player->HasAura(SPELL_CLOUDBURST_PLAYER_AURA) && !casted)
                        {
                            DoCastAOE(SPELL_CLOUDBURST_VISUAL_WATER);
                            me->DespawnOrUnsummon(3000);
                            casted = true;
                        }
                    }
                    events.ScheduleEvent(EVENT_CHECK_PLAYER, 500);
                    break;
                }
                case EVENT_SPREAD_FLAME:
                {
                    // Prevent wowerror
                    if (flamecount >= 3)
                        break;

                    switch (urand(0, 5))
                    {
                    case 0:
                        me->SummonCreature(NPC_DREADFLAME_SPAWN, me->GetPositionX() + 4, me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_CORPSE_DESPAWN, 10000);
                        flamecount++;
                        break;
                    case 1:
                        me->SummonCreature(NPC_DREADFLAME_SPAWN, me->GetPositionX() - 4, me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_CORPSE_DESPAWN, 10000);
                        flamecount++;
                        break;
                    case 2:
                        me->SummonCreature(NPC_DREADFLAME_SPAWN, me->GetPositionX(), me->GetPositionY() + 4, me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_CORPSE_DESPAWN, 10000);
                        flamecount++;
                        break;
                    case 3:
                        me->SummonCreature(NPC_DREADFLAME_SPAWN, me->GetPositionX(), me->GetPositionY() - 4, me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_CORPSE_DESPAWN, 10000);
                        flamecount++;
                        break;
                    case 4:
                    case 5:
                        break;
                    default:
                        break;
                    }
                    events.ScheduleEvent(EVENT_SPREAD_FLAME, 5000);
                    break;
                }
                default:
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fl_dreadflameAI(creature);
    }
};

class npc_fl_cloudburst : public CreatureScript
{
public:
    npc_fl_cloudburst() : CreatureScript("npc_fl_cloudburst") { }

    struct npc_fl_cloudburstAI : public ScriptedAI
    {
        npc_fl_cloudburstAI(Creature* creature) : ScriptedAI(creature)
        {
            _playerCount = 0;
        }

        EventMap events;
        uint8 _playerCount;

        void IsSummonedBy(Unit* summoner)
        {
            me->AddAura(SPELL_CLOUDBURST_DUMMY_AURA, me);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            events.ScheduleEvent(EVENT_CHECK_PLAYER, 500);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_CHECK_PLAYER:
                    if (Player* player = me->FindNearestPlayer(1.0f))
                    {
                        if (me->HasAura(SPELL_CLOUDBURST_DUMMY_AURA))
                        {
                            if (Is25ManRaid()) // 25 Player Mode: 3 Players can use a single cloudburst
                            {
                                if (_playerCount < 3)
                                {
                                    player->AddAura(SPELL_CLOUDBURST_PLAYER_AURA, player);
                                    _playerCount++;
                                }
                                else
                                {
                                    player->AddAura(SPELL_CLOUDBURST_PLAYER_AURA, player);
                                    me->DespawnOrUnsummon(100);
                                    events.Reset();
                                }

                            }
                            else
                            {
                                player->AddAura(SPELL_CLOUDBURST_PLAYER_AURA, player);
                                me->DespawnOrUnsummon(100);
                                events.Reset();
                            }
                        }
                    }
                    else
                        events.ScheduleEvent(EVENT_CHECK_PLAYER, 500);
                    break;
                default:
                    break;
                }
            }
        }

    };
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fl_cloudburstAI(creature);
    }
};

class spell_fl_splitting_blow : public SpellScriptLoader
{
public:
    spell_fl_splitting_blow() : SpellScriptLoader("spell_fl_splitting_blow") { }

    class spell_fl_splitting_blow_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_fl_splitting_blow_SpellScript);

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            GetHitUnit()->CastSpell(GetHitUnit(), SPELL_SUMMON_SULFURAS_AURA);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_fl_splitting_blow_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_fl_splitting_blow_SpellScript();
    }
};

class spell_fl_invoke_sons : public SpellScriptLoader
{
public:
    spell_fl_invoke_sons() : SpellScriptLoader("spell_fl_invoke_sons") { }

    class spell_fl_invoke_sons_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_fl_invoke_sons_SpellScript);

        bool Validate(SpellInfo const* spellEntry)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_INVOKE_SONS))
                return false;
            return true;
        }

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            GetCaster()->CastSpell(GetHitUnit(), SPELL_INVOKE_SONS_MISSILE, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_fl_invoke_sons_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_fl_invoke_sons_SpellScript();
    }
};

class spell_fl_blazing_heat : public SpellScriptLoader
{
public:
    spell_fl_blazing_heat() : SpellScriptLoader("spell_fl_blazing_heat") { }

    class spell_fl_blazing_heat_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_fl_blazing_heat_AuraScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_BLAZING_HEAT_AURA))
                return false;
            return true;
        }

        void OnPeriodic(AuraEffect const* /*aurEff*/)
        {
            GetTarget()->CastSpell(GetTarget(), SPELL_BLAZING_HEAT_SUMMON, false);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_fl_blazing_heat_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_fl_blazing_heat_AuraScript();
    }
};

class spell_fl_world_in_flames : public SpellScriptLoader
{
public:
    spell_fl_world_in_flames() : SpellScriptLoader("spell_fl_world_in_flames") { }

    class spell_fl_world_in_flames_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_fl_world_in_flames_SpellScript);

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            switch (urand(0, 2))
            {
            case 0: // Melee
            {
                if (Unit* caster = GetCaster())
                {
                    for (uint32 x = 0; x < 16; ++x)
                        caster->SummonCreature(NPC_ENGULFING_FLAMES_TRIGGER, EngulfingFlamesMelee[x], TEMPSUMMON_TIMED_DESPAWN, 4000);

                    caster->CastSpell(caster, SPELL_ENGULFING_FLAMES_VISUAL_MELEE);
                    caster->CastSpell(caster, SPELL_ENGULFING_FLAMES_MELEE);
                }
                break;
            }
            case 1: // Range
            {
                if (Unit* caster = GetCaster())
                {
                    for (uint32 x = 0; x < 35; ++x)
                        caster->SummonCreature(NPC_ENGULFING_FLAMES_TRIGGER, EngulfingFlamesRange[x], TEMPSUMMON_TIMED_DESPAWN, 4000);

                    caster->CastSpell(caster, SPELL_ENGULFING_FLAMES_VISUAL_BOTTOM);
                    caster->CastSpell(caster, SPELL_ENGULFING_FLAMES_BOTTOM);
                }
                break;
            }
            case 2: // Center
            {
                if (Unit* caster = GetCaster())
                {
                    for (uint32 x = 0; x < 17; ++x)
                        caster->SummonCreature(NPC_ENGULFING_FLAMES_TRIGGER, EngulfingFlamesCenter[x], TEMPSUMMON_TIMED_DESPAWN, 4000);

                    caster->CastSpell(GetCaster(), SPELL_ENGULFING_FLAMES_VISUAL_CENTER);
                    caster->CastSpell(GetCaster(), SPELL_ENGULFING_FLAMES_CENTER);
                }
                break;
            }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_fl_world_in_flames_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_fl_world_in_flames_SpellScript();
    }
};

class spell_fl_empower_sulfuras : public SpellScriptLoader
{
public:
    spell_fl_empower_sulfuras() : SpellScriptLoader("spell_fl_empower_sulfuras") { }

    class spell_fl_empower_sulfuras_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_fl_empower_sulfuras_AuraScript);

        void OnPeriodic(AuraEffect const* /*aurEff*/)
        {
            switch (urand(0, 1))
            {
            case 0:
                break;
            case 1:
                GetCaster()->CastSpell(GetCaster(), SPELL_EMPOWER_SULFURAS_MISSILE);
                break;
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_fl_empower_sulfuras_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_fl_empower_sulfuras_AuraScript();
    }
};

class spell_fl_breadth_of_frost : public SpellScriptLoader
{
public:
    spell_fl_breadth_of_frost() : SpellScriptLoader("spell_fl_breadth_of_frost") { }

    class spell_fl_breadth_of_frost_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_fl_breadth_of_frost_AuraScript);

        void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                caster->ApplySpellImmune(0, IMMUNITY_ID, SPELL_SUPERHEATED_TRIGGERED, true);
                caster->ApplySpellImmune(0, IMMUNITY_ID, 100915, true);
            }
        }

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                caster->ApplySpellImmune(0, IMMUNITY_ID, SPELL_SUPERHEATED_TRIGGERED, false);
                caster->ApplySpellImmune(0, IMMUNITY_ID, 100915, false);
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_fl_breadth_of_frost_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectApply += AuraEffectApplyFn(spell_fl_breadth_of_frost_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    class spell_fl_breadth_of_frost_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_fl_breadth_of_frost_SpellScript);

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            GetHitUnit()->RemoveAurasDueToSpell(SPELL_SUPERHEATED_TRIGGERED);
            GetHitUnit()->RemoveAurasDueToSpell(100915);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_fl_breadth_of_frost_SpellScript::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_fl_breadth_of_frost_SpellScript();
    }

    AuraScript* GetAuraScript() const
    {
        return new spell_fl_breadth_of_frost_AuraScript();
    }
};

class spell_fl_breadth_of_frost_freeze : public SpellScriptLoader
{
public:
    spell_fl_breadth_of_frost_freeze() : SpellScriptLoader("spell_fl_breadth_of_frost_freeze") { }

    class spell_fl_breadth_of_frost_freeze_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_fl_breadth_of_frost_freeze_AuraScript);

        void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* meteor = GetOwner()->ToUnit())
            {
                if (Creature* trap = meteor->FindNearestCreature(NPC_BREADTH_OF_FROST, 1.0f, true))
                    trap->DespawnOrUnsummon(1);

                meteor->RemoveAura(100904);
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_fl_breadth_of_frost_freeze_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_fl_breadth_of_frost_freeze_AuraScript();
    }
};

class spell_fl_entrapping_roots : public SpellScriptLoader
{
public:
    spell_fl_entrapping_roots() : SpellScriptLoader("spell_fl_entrapping_roots") { }

    class spell_fl_entrapping_roots_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_fl_entrapping_roots_AuraScript);

        void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* ragnaros = GetOwner()->ToUnit())
                if (Creature* trap = ragnaros->FindNearestCreature(NPC_ENTRAPPING_ROOTS, 1.0f, true))
                    trap->DespawnOrUnsummon(1);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_fl_entrapping_roots_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_fl_entrapping_roots_AuraScript();
    }
};

class spell_fl_molten_inferno : public SpellScriptLoader
{
public:
    spell_fl_molten_inferno() : SpellScriptLoader("spell_fl_molten_inferno") { }

    class spell_fl_molten_inferno_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_fl_molten_inferno_SpellScript);

        void HandleDamage(SpellEffIndex /*effIndex*/)
        {
            int32 damage = GetEffectValue();

            if (Unit* target = GetHitUnit())
            {
                float distance = GetCaster()->GetDistance2d(target->GetPositionX(), target->GetPositionY());
                SetHitDamage(damage / distance);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_fl_molten_inferno_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_fl_molten_inferno_SpellScript();
    }
};

class spell_fl_deluge : public SpellScriptLoader
{
public:
    spell_fl_deluge() : SpellScriptLoader("spell_fl_deluge") { }

    class spell_fl_deluge_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_fl_deluge_AuraScript);

        void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
                caster->ApplySpellImmune(0, IMMUNITY_ID, 100941, true);
        }

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
                caster->ApplySpellImmune(0, IMMUNITY_ID, 100941, false);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_fl_deluge_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_INCREASE_SPEED, AURA_EFFECT_HANDLE_REAL);
            OnEffectApply += AuraEffectApplyFn(spell_fl_deluge_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_INCREASE_SPEED, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_fl_deluge_AuraScript();
    }
};

class spell_fl_lavalogged : public SpellScriptLoader
{
public:
    spell_fl_lavalogged() : SpellScriptLoader("spell_fl_lavalogged") { }

    class spell_fl_lavalogged_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_fl_lavalogged_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            GetCaster()->AddAura(SPELL_LIVING_METEOR_INCREASE_SPEED, GetCaster());
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_fl_lavalogged_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_fl_lavalogged_AuraScript();
    }
};

void AddSC_boss_ragnaros_cata()
{
    new at_sulfuron_keep();
    new boss_ragnaros_cata();
    new npc_fl_magma_trap();
    new npc_fl_sulfuras_smash();
    new npc_fl_lava_wave();
    new npc_fl_sulfuras_hammer();
    new npc_fl_son_of_flame();
    new npc_fl_engulfing_flame();
    new npc_fl_molten_elemental();
    new npc_fl_lava_scion();
    new npc_fl_blazing_heat();
    new npc_fl_living_meteor();
    new npc_fl_archdruids();
    new npc_fl_dreadflame();
    new npc_fl_cloudburst();
    new spell_fl_splitting_blow();
    new spell_fl_invoke_sons();
    new spell_fl_blazing_heat();
    new spell_fl_world_in_flames();
    new spell_fl_empower_sulfuras();
    new spell_fl_breadth_of_frost();
    new spell_fl_breadth_of_frost_freeze();
    new spell_fl_entrapping_roots();
    new spell_fl_molten_inferno();
    new spell_fl_deluge();
    new spell_fl_lavalogged();
}