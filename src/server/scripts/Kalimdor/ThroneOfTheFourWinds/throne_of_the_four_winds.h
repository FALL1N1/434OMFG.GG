#ifndef DEF_THRONE_OF_THE_FOUR_WINDS_H
#define DEF_THRONE_OF_THE_FOUR_WINDS_H

#define DataHeader "TW"

enum Data
{
    DATA_CONCLAVE_OF_WIND_EVENT,
    DATA_ALAKIR_EVENT,
    DATA_KILLED_BOSSES,
    DATA_DEACTIVATE_SLIPSTREAM
};

enum Data64
{
    DATA_ANSHAL,
    DATA_NEZIR,
    DATA_ROHASH,
    DATA_ALAKIR,
};

enum CreatureIds
{
    BOSS_ANSHAL                     = 45870,
    BOSS_NEZIR                      = 45871,
    BOSS_ROHASH                     = 45872,

    BOSS_ALAKIR                     = 46753,

    // Conclave of Wind
    NPC_SOOTHING_BREEZE             = 46246,
    NPC_RAVENOUS_CREEPER            = 45812,
    NPC_RAVENOUS_CREEPER_TRIGGER    = 45813,
    NPC_ICE_PATCH                   = 46186,
    NPC_TORNADO                     = 46207,
    NPC_WORLD_TRIGGER_2             = 19871,
    NPC_HURRICANE_TRIGGER           = 46419,
    NPC_SLIPSTREAM_ALAKIR           = 47066
};

enum InstanceSpells
{
    SPELL_PRE_COMBAT_EFFECT_ANSHAL  = 85537,
    SPELL_PRE_COMBAT_EFFECT_NEZIR   = 85532,
    SPELL_PRE_COMBAT_EFFECT_ROHASH  = 85538,
    SPELL_WIND_BOSSES_ALL_DEAD      = 88835
};

enum GenericSpells
{
    SPELL_BERSERK                   = 26662
};

enum GameObjectEntrys
{
    GOB_CENTER_PLATFORM = 207737,
};

enum alak_actions
{
    ACTION_DISABLE_SLIPSTREAM_TRIGGER,
};

enum AchievementCriterias
{
    CRITERIA_GUILD_RUN_ALAKIR   = 15699,
    CRITERIA_GUILD_RUN_ALAKIR_2 = 15700,
    CRITERIA_GUILD_RUN_CONCLAVE = 14146
};

#endif
