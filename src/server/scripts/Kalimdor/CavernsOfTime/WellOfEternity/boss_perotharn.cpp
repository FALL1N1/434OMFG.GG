/*
 * Copyright (C) 2013-2015 OMFG.GG Network <http://www.omfg.gg/>
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

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"
#include "well_of_eternity.h"

enum Talks
{
    TALK_AGGRO                          = -54778,
    TALK_ILLIDAN_AGGRO_BOSS             = -54829,
    TALK_DRAIN_ESSENCE                  = -54546,
    TALK_ILLIDAN_ABSORB                 = -54775,
    TALK_INTO_SHADOWS                   = -54816,
    TALK_INTO_SHADOWS_EMOTE             = -55983,
    TALK_ILLIDAN_RETURN                 = -54831,
    TALK_EYE_EVENT                      = -54817,
    TALK_EYE_EVENT_WARNING              = -55984,
    TALK_YOU_HIDE_WELL                  = -54766,
    TALK_ILLIDAN_STRENGTH_RETURN        = -54832,
    TALK_FRENZY                         = -54812,
    TALK_DEATH                          = -56991,
    TALK_COWER_IN_HIDING                = -54819,
    TALK_PORTAL_SHUTDOWN                = -54833,
    TALK_ILLIDAN_MOVE_IN_LINE_OF_SIGHT  = -54274,
    TALK_ILLIDAN_WAIT_FOR_EVENT_START   = -54277,
    TALK_ILLIDAN_HIDE_IN_SHADOWS        = -54705,
    TALK_ILLIDAN_START_MOVEMENT         = -54284,
    TALK_ILLIDAN_MANY_DEMONS            = -54288,
    TALK_ILLIDAN_SINGLE_GUARDIAN_TALK   = -54296,
    TALK_ILLIDAN_HOLD_THEM_BACK         = -54297,
    TALK_ILLIDAN_MAGIC_IS_FADING        = -54299,
    TALK_ILLIDAN_ATTACK_CRYSTALL        = -54304,
    TALK_ILLIDAN_DESTROY_THE_CRYSTALL   = -54709,
    TALK_ILLIDAN_LEAVING                = -54511,
    TALK_ILLIDAN_THEY_COME_ENDLESSLY    = -54510,
    TALK_ILLIDAN_ATTACKING              = -54444,
    TALK_ILLIDAN_THE_STENCH_OF_SULFUR   = -54512,
    TALK_ILLIDAN_CUT_THIS_ONE_DOWN      = -54514,
    TALK_ILLIDAN_SHUT_DOWN_LAST_PORTAL  = -54736,
    TALK_ILLIDAN_DEMONS_LEAVING         = -54750,
    TALK_ILLIDAN_DEMONS_NOT_ON_PLACE    = -54572,
    TALK_ILLIDAN_TOO_EASY               = -54573,
    TALK_ILLIDAN_ANOTHER_DEMON          = -54574,
    TALK_ILLIDAN_PATIENCE               = -54309,
    TALK_ILLIDAN_AGGRO                  = -54444,
    TALK_ILLIDAN_PEROTHARN_DEATH        = -54838,
    TALK_ILLIDAN_GOOD_BYE               = -54839,
    TALK_NOZDORMU_INTRO                 = -55271,
    TALK_NOZDORMU_INTRO_TWO             = -55304,
    TALK_NOZDORMU_INTRO_THREE           = -55305,
    TALK_PEROTHARN_INTRO                = -54683,
    TALK_PEROTHARN_INTRO_TWO            = -54685,
    TALK_PEROTHARN_INTRO_THREE          = -54687,
    TALK_PEROTHARN_PORTAL_SHUTDOWN      = -54833,
};

enum Spells
{
    // Perotharn
    SPELL_CORRUPTING_TOUCH                      = 104939,
    SPELL_50_PCT_INCREASED_SCALE                = 105014,
    SPELL_CAMOUFLAGE                            = 105341,
    SPELL_FEL_FLAMES                            = 108141,
    SPELL_FEL_DECAY                             = 105544,
    SPELL_FEL_SURGE                             = 108128,
    SPELL_DRAIN_ESSENCE                         = 104905,
    SPELL_FEL_ADDLED                            = 105545,
    SPELL_ENDLESS_FRENZY                        = 105521,
    SPELL_CHARGE_PLAYER                         = 105496,
    SPELL_ASPHYXIATE                            = 93422,
    SPELL_ENFEEBLED                             = 105442,

    // Fel Flames
    SPELL_FEL_FLAMES_PERIODIC                   = 108214,

    // Hunting Circle
    SPELL_SUMMON_CIRCLE_SPAWN                   = 105462,

    // Hunting Stalker
    SPELL_PUNISHING_FLAMES                      = 107532,
    SPELL_FEL_FIREBALL_AGGRO                    = 105491,
    SPELL_CANCEL_FEL_FIREBALL_AGGRO             = 105492,
    SPELL_HUNTING_CHANNEL                       = 105353, // The Eyes of Peroth'arn are looking for you.
    SPELL_SUMMON_CIRCLE_TRIGGER                 = 105463, // unused
    SPELL_END_EXHAUSTED                         = 105548, // target illidan
    
    // Eye of Perotharn
    SPELL_EASY_PREY                             = 105493,
    SPELL_EASY_PREY_SUMMON                      = 105761,
    SPELL_INVISIBILITY_DETECTION                = 93105,

    // Illidan event
    SPELL_ABSORB_FEL_ENERGY                     = 105543,
    SPELL_ILLIDAN_WALL_OF_SHADOWS               = 104400,
    SPELL_ILLIDAN_SHADOWCLOAK                   = 105915,
    SPELL_ILLIDAN_STALKER_SHADOWCLOAK_COSMETIC  = 105924,
    SPELL_RIDE_VEHICLE_HARDCODED                = 46598,
    SPELL_PLAYER_SHADOWCLOAK_COSMETIC           = 102951,
    SPELL_PLAYER_SHADOW_AMBUSHER                = 103018,
    SPELL_PLAYER_SHADOW_AMBUSHER_STEALTH        = 103420,
    SPELL_DISTRACT_DEMONS_STATIONARY            = 110082,
    SPELL_DEMON_RUSH                            = 104205,
    SPELL_DEMONIC_SIGHT                         = 104746,
    SPELL_REGENERATION                          = 105547,
    // Legion demon
    SPELL_SUMMON_FIREWALL                       = 105243,
    SPELL_SUMMON_FIREWALL_PERIODIC              = 105247,
    SPELL_SUMMON_FIREWALL_PULSE                 = 105250,
    SPELL_STRIKE_FEAR                           = 103913,
    // Dreadlord Defender
    SPELL_DEMONIC_WARDING                       = 107900,
    SPELL_CARRION_SWARM                         = 107840,
    // Corrupted Arcanist
    SPELL_CORRUPTED_NIGHT_ELF_EYES              = 110048,
    SPELL_DISCIPLINED_CASTER                    = 107877,
    SPELL_ARCANE_ANNIHILATION                   = 107865,
    SPELL_INFINITE_MANA                         = 107880,
    // Fel Crystal
    SPELL_FEL_CRYSTAL_MELTDOWN                  = 105074,
    SPELL_FEL_CRYSTAL_DESTRUCTION               = 105079,
    SPELL_FEL_CRYSTAL_DESTROYED                 = 105119,
    SPELL_FEL_CRYSTAL_SWITCH_GLOW               = 105046,
    SPELL_SHATTER_FEL_CRYSTAL                   = 105004,
    SPELL_ARCANE_EXPLOSION_VISUAL               = 98122,
    // Guardian Demon
    SPELL_DEMONIC_GRIP                          = 102561,
    SPELL_DEMONIC_GRIP_PULL                     = 103649,
    SPELL_DEMONIC_GRIP_ROOT                     = 102937,
    // Teleporter
    SPELL_TELEPORT_AZSHARAS_PALACE              = 107690,
    SPELL_TELEPORT_WELL_OF_ETERNITY             = 107691,
    SPELL_TELEPORT_AZSHARAS_PALACE_OVERLOOK     = 107979,
    SPELL_TELEPORT_COURTYARD_ENTRANCE           = 107934,
};

enum Events
{
    EVENT_SUMMON_DEMON_WAVES_PORTAL_THREE   = 1,
    EVENT_SUMMON_DEMON_WAVES_PORTAL_ONE_TWO = 2,
    EVENT_FEL_FLAMES                        = 3,
    EVENT_FEL_DECAY                         = 4,
    EVENT_DRAIN_ESSENCE                     = 5,
    EVENT_DRAIN_ESSENCE_ILLIDAN             = 6,
    EVENT_MOVE_TO_COMBAT_AREA               = 7,
    EVENT_TELEPORT_TO_HOME_POS              = 8,
    EVENT_CAMOUFLAGE                        = 9,
    EVENT_START_EYE_EVENT                   = 10,
    EVENT_END_EYE_EVENT                     = 11,
    // Legion Demon
    EVENT_MOVE_TO_PORTAL                    = 12,
    // Illidan
    EVENT_RESTART_ESCORT                    = 1,
    EVENT_SHADOWCLOAK_CHECK                 = 2,
    EVENT_MOVE_TO_NEXT_PORTAL               = 3,
    EVENT_START_BOSS_FIGHT                  = 4,
    EVENT_PLAYER_IN_COMBAT_CHECK            = 5,
    EVENT_PLAYER_ALIVE_CHECK                = 6,
    // Instance pre event
    EVENT_START_PEROTHARN_INTRO             = 1,
    EVENT_START_DEMON_MOVEMENT              = 2,
    EVENT_REMOVE_FLAGS                      = 3,
    // Legion Demon
    EVENT_STRIKE_FEAR                       = 1,
    // Dreadlord Defender
    EVENT_DEMONIC_WARDING                   = 1,
    EVENT_CARRION_SWARM                     = 2,
    EVENT_START_NEXT_WAYPOINT               = 3,
    EVENT_LAST_WAYPOINT                     = 4,
    // Corrupted Arcanist
    EVENT_ARCANE_ANNIHILATION               = 1,
    EVENT_CHECK_MANA_POOL                   = 2
};

enum Points
{
    POINT_COMBAT_AREA       = 1,
    POINT_EVENT_WALK        = 2,
    POINT_PORTAL            = 3,
    POINT_EYE_PHASE         = 4,
    POINT_SUMMON_FIREWALL   = 5,
    POINT_EVENT_WALK_TWO    = 7
};

enum Phases
{
    EVENT_PHASE_COMBAT                       = 1,
    EVENT_PHASE_INTRO                        = 2,
};

enum summonGroups
{
    SUMMON_GROUP_DEMON_WAVE_PORTAL_THREE    = 1,
    SUMMON_GROUP_DEMON_WAVE_PORTAL_ONE      = 2,
    SUMMON_GROUP_DEMON_WAVE_PORTAL_TWO      = 3,
    SUMMON_GROUP_EVENT_INTRO                = 1,
};

enum AnimKits
{
    ANIM_ONE_SHOT_CIRCLE    = 1688,
};

enum actions
{
    ACTION_START_EVENT              = 1,
    ACTION_SET_FIRST_FIREWALL_DEMON = 2,
    ACTION_SALUTE_EMOTE             = 3,
    ACTION_DEMON_KILLED             = 4,
    ACTION_MOVE_NEXT_PORTAL         = 5,
    ACTION_PLAYER_FOUND             = 6,
    ACTION_DEMON_MOVE_SPEED         = 7,
    // Illidan ACtions
    ACTION_ILLIDAN_START_EVENT      = 1,
    ACTION_ILLIDAN_FINISH_EVENT     = 2,
    ACTION_ILLIDAN_PORTAL_CLOSED    = 3,
    ACTION_ILLIDAN_COMBAT_FINISHED  = 4
};

enum Factions
{
    ILLIDAN_FACTION_ATTACKER    = 2385,
    ILLIDAN_FACTION_ESCORT      = 2386
};

enum QuestData
{
    QUEST_IN_UNENDING_NUMBERS   = 30099,
    QUEST_CREDIT_PORTAL_ONE     = 58239,
    QUEST_CREDIT_PORTAL_TWO     = 58240,
    QUEST_CREDIT_PORTAL_THREE   = 58241,
};

enum npcs
{
    NPC_ILLIDAN_SHADOWCLOAK_STALKER = 56389,
    NPC_PLAYER_SHADOWCLOAK_STALKER  = 55154
};

const Position portalPositions[3] =
{
    { 3256.285f, -4942.128f, 181.679f, 0.000f },
    { 3287.178f, -4816.500f, 181.471f, 0.000f },
    { 3414.551f, -4842.202f, 181.704f, 0.000f}
};

uint32 const demonPathSize = 10;
const G3D::Vector3 demonLeftPath[demonPathSize] =
{
    { 3450.713f, -5065.567f, 213.8622f },
    { 3448.213f, -5061.567f, 213.8622f },
    { 3430.963f, -5034.567f, 197.3622f },
    { 3421.463f, -5019.567f, 197.1122f },
    { 3410.463f, -5003.067f, 197.1122f },
    { 3401.213f, -4989.067f, 197.1122f },
    { 3392.963f, -4975.567f, 197.3622f },
    { 3380.213f, -4954.817f, 181.8622f },
    { 3342.788f, -4895.653f, 181.3267f },
    { 3292.490f, -4820.826f, 181.4715f },
};

const G3D::Vector3 demonRightPath[demonPathSize] =
{
    { 3444.493f, -5069.685f, 213.8785f },
    { 3441.993f, -5065.685f, 213.8785f },
    { 3425.493f, -5037.935f, 197.3785f },
    { 3405.743f, -5006.435f, 197.1285f },
    { 3396.493f, -4992.185f, 197.1285f },
    { 3388.243f, -4978.435f, 197.3785f },
    { 3375.493f, -4957.685f, 181.6285f },
    { 3360.243f, -4934.185f, 181.3785f },
    { 3340.493f, -4904.435f, 181.3785f },
    { 3288.470f, -4823.823f, 181.4715f },
};

const Position PortalOneWavePortalPositions[4] =
{
    { 3254.163f, -4938.855f, 181.680f, 0.000f }, // right demon portal one
    { 3257.081f, -4945.927f, 181.680f, 0.000f }, // left demon portal one
    { 3415.980f, -4845.407f, 181.704f, 0.000f }, // right demon portal two
    { 3412.173f, -4839.316f, 181.704f, 0.000f }  // left demon portal two
};

const Position huntingCirclePos[8] =
{
    { 3338.792f, -4898.623f, 181.2851f, 0.000f },
    { 3331.352f, -4884.458f, 181.2851f, 0.000f },
    { 3332.695f, -4899.179f, 181.2851f, 0.000f },
    { 3337.449f, -4883.901f, 181.2851f, 0.000f },
    { 3327.990f, -4895.260f, 181.2851f, 0.000f },
    { 3342.154f, -4887.820f, 181.2851f, 0.000f },
    { 3327.433f, -4889.163f, 181.2851f, 0.000f },
    { 3342.711f, -4893.917f, 181.2851f, 0.000f }
};

const Position firewallPeriodicPositions[7] =
{
    { 3204.615f, -4935.365f, 194.4112f, 0.000f },
    { 3182.099f, -4933.120f, 194.4112f, 0.000f },
    { 3201.065f, -4937.566f, 194.4112f, 0.000f },
    { 3185.144f, -4935.979f, 194.4112f, 0.000f },
    { 3197.066f, -4938.774f, 194.4112f, 0.000f },
    { 3188.825f, -4937.952f, 194.4112f, 0.000f },
    { 3192.892f, -4938.905f, 194.4112f, 0.000f }
};

const Position DemonStalkerPos[4] =
{
    { 3324.963f, -4898.221f, 181.243f, 0.000f },
    { 3321.393f, -4900.539f, 181.285f, 0.000f },
    { 3317.850f, -4902.840f, 181.284f, 0.000f },
    { 3314.297f, -4905.146f, 181.284f, 0.000f }
};

const Position demonEndPos = { 3193.616f, -4940.210f, 189.525f, 5.119f };
const Position huntingStalkerPos = { 3335.072f, -4891.54f, 181.1601f, 0.000f };
const Position eyeSummonPos = { 3335.223f, -4891.458f, 181.076f, 0.000f };

class DelayedEyeSummonEvent : public BasicEvent
{
public:
    DelayedEyeSummonEvent(Unit* trigger) : _trigger(trigger)
    {
    }

    bool Execute(uint64 /*time*/, uint32 /*diff*/)
    {
        _trigger->CastSpell(_trigger, SPELL_SUMMON_CIRCLE_SPAWN, true);
        return true;
    }

private:
    Unit* _trigger;
};

class boss_woe_perotharn : public CreatureScript
{
public:
    boss_woe_perotharn() : CreatureScript("boss_woe_perotharn") {}

    struct boss_woe_perotharnAI : public BossAI
    {
        boss_woe_perotharnAI(Creature* creature) : BossAI(creature, DATA_PEROTHARN) {}

        void InitializeAI() override
        {
            demonWaves = 0;
            demonWavesPortalOneAndTwo = 0;
            eyeCount = 0;
            eyeTimer = 1000;
            skipUpdateVictim = true;
            drainEssenceTriggered = false;
            lazyEyeAchievementDone = true;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC);
            BossAI::InitializeAI();
        }

        void Reset() override
        {
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->RemoveAllAuras();
            DoCast(me, SPELL_CORRUPTING_TOUCH, true);
            drainEssenceTriggered = false;
            lazyEyeAchievementDone = true;
            eyeTimer = 1000;
            eyeCount = 0;
            _Reset();
            if (demonWaves == 0)
            {
                events.ScheduleEvent(EVENT_SUMMON_DEMON_WAVES_PORTAL_THREE, 1000);
                events.ScheduleEvent(EVENT_SUMMON_DEMON_WAVES_PORTAL_ONE_TWO, 50000);
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage) override
        {
            if (!drainEssenceTriggered && me->HealthBelowPctDamaged(60, damage))
            {
                drainEssenceTriggered = true;
                skipUpdateVictim = true;
                Talk(TALK_DRAIN_ESSENCE);
                me->SetReactState(REACT_PASSIVE);
                me->AttackStop();
                events.Reset();
                DoCast(SPELL_DRAIN_ESSENCE);
                events.ScheduleEvent(EVENT_DRAIN_ESSENCE_ILLIDAN, 4000);
            }
            else if (me->HealthBelowPctDamaged(20, damage) && !me->HasAura(SPELL_ENDLESS_FRENZY))
            {
                Talk(TALK_FRENZY);
                DoCast(me, SPELL_ENDLESS_FRENZY, true);
            }
        }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
                case ACTION_DESPAWN_PORTAL_ONE:
                case ACTION_DESPAWN_PORTAL_TWO:
                case ACTION_DESPAWN_PORTAL_THREE:
                    // Here we use doaction to doaction because with a grid searcher we need ~500m search radius... here we have a summonlist...
                    for (uint64 summonGUID : summons)
                        if (Creature* summon = ObjectAccessor::GetCreature(*me, summonGUID))
                            summon->AI()->DoAction(action);
                    if (action == ACTION_DESPAWN_PORTAL_THREE)
                        TalkWithDelay(8000, TALK_PORTAL_SHUTDOWN);
                    break;
                case ACTION_START_COMBAT:
                    me->SetVisible(true);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC);
                    break;
                case ACTION_PLAYER_FOUND:
                    lazyEyeAchievementDone = false;
                    events.CancelEvent(EVENT_END_EYE_EVENT);
                    DoCastAOE(SPELL_CANCEL_FEL_FIREBALL_AGGRO);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PLAYER_SHADOW_WALK_VISUAL);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PLAYER_SHADOW_WALK);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    me->RemoveAurasDueToSpell(SPELL_CAMOUFLAGE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->SetInCombatWithZone();
                    summons.DespawnEntry(NPC_HUNTING_CIRCLE);
                    summons.DespawnEntry(NPC_HUNTING_STALKER);
                    summons.DespawnEntry(NPC_EYE_OF_THE_PEROTHARN);
                    if (Creature* illidan = instance->GetCreature(DATA_ILLIDAN_PART_ONE))
                        DoCast(illidan, SPELL_END_EXHAUSTED, true);
                    DoCast(me, SPELL_CHARGE_PLAYER);
                    skipUpdateVictim = false;
                    events.ScheduleEvent(EVENT_FEL_FLAMES, 15000);
                    events.ScheduleEvent(EVENT_FEL_DECAY, 18000);
                    break;
                default:
                    break;
            }
        }

        void EnterCombat(Unit* victim) override
        {
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            Talk(TALK_AGGRO);
            if (Creature* illidan = instance->GetCreature(DATA_ILLIDAN_PART_ONE))
            {
                illidan->AI()->TalkWithDelay(4000, TALK_ILLIDAN_AGGRO_BOSS);
                illidan->AI()->AttackStart(me);
            }
            skipUpdateVictim = false;
            events.ScheduleEvent(EVENT_FEL_FLAMES, 5000);
            events.ScheduleEvent(EVENT_FEL_DECAY, 8000);
            _EnterCombat();
        }

        uint32 GetData(uint32 data) const override
        {
            if (data == DATA_LAZY_EYE_ACHIEVEMENT)
                return lazyEyeAchievementDone ? 1 : 0;

            return 0;
        }

        void JustDied(Unit* killer) override
        {
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            Talk(TALK_DEATH);
            _JustDied();
            if (Creature* illidan = instance->GetCreature(DATA_ILLIDAN_PART_ONE))
                illidan->AI()->DoAction(ACTION_COMBAT_FINISHED);
            if (Creature* nozdormu = instance->GetCreature(DATA_NOZDORMU))
                nozdormu->NearTeleportTo(NozdormuPositions[0]);
        }

        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            switch (summon->GetEntry())
            {
                case NPC_FEL_FLAMES_TRIGGER:
                    summon->CastSpell(summon, SPELL_FEL_FLAMES_PERIODIC, true);
                    break;
                case NPC_HUNTING_CIRCLE:
                    if (++eyeCount > 2)
                    {
                        eyeTimer += 1000;
                        eyeCount = 1;
                    }
                    summon->m_Events.AddEvent(new DelayedEyeSummonEvent(summon), summon->m_Events.CalculateTime(eyeTimer));
                    break;
                case NPC_HUNTING_STALKER:
                    summon->CastSpell(summon, SPELL_PUNISHING_FLAMES, true);
                    summon->CastSpell((Unit*)NULL, SPELL_FEL_FIREBALL_AGGRO, true);
                    summon->CastSpell(summon, SPELL_HUNTING_CHANNEL, true);
                    summon->AI()->Talk(TALK_EYE_EVENT_WARNING, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                    break;
                case NPC_EYE_OF_THE_PEROTHARN:
                    summon->CastSpell(summon, SPELL_INVISIBILITY_DETECTION, true);
                    summon->GetMotionMaster()->MoveRandom(65.00f);
                    break;
                default:
                    break;
            }
        }

        void KilledUnit(Unit* victim) override
        {

        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE)
            {
                switch (id)
                {
                    case POINT_EVENT_WALK:
                        events.ScheduleEvent(EVENT_MOVE_TO_COMBAT_AREA, 100);
                        break;
                    case POINT_COMBAT_AREA:
                    {
                        Position newHomePos(*me);
                        me->SetHomePosition(newHomePos);
                        me->SetVisible(false);
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        void UpdateAI(uint32 const diff) override
        {
            if (!skipUpdateVictim && !UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_MOVE_TO_COMBAT_AREA:
                        me->SetHomePosition(eventWaypoints[2]);
                        me->GetMotionMaster()->MovePoint(POINT_COMBAT_AREA, eventWaypoints[2]);
                        break;
                    case EVENT_SUMMON_DEMON_WAVES_PORTAL_THREE:
                        me->SummonCreatureGroup(SUMMON_GROUP_DEMON_WAVE_PORTAL_THREE);
                        if (++demonWaves <= 22)
                            events.ScheduleEvent(EVENT_SUMMON_DEMON_WAVES_PORTAL_THREE, 4000);
                        break;
                    case EVENT_SUMMON_DEMON_WAVES_PORTAL_ONE_TWO:
                        me->SummonCreatureGroup(SUMMON_GROUP_DEMON_WAVE_PORTAL_ONE);
                        me->SummonCreatureGroup(SUMMON_GROUP_DEMON_WAVE_PORTAL_TWO);
                        if (++demonWavesPortalOneAndTwo <= 6)
                            events.ScheduleEvent(EVENT_SUMMON_DEMON_WAVES_PORTAL_ONE_TWO, 4000);
                        break;
                    case EVENT_FEL_FLAMES:
                        DoCastRandom(SPELL_FEL_FLAMES, 150.00f);
                        events.ScheduleEvent(EVENT_FEL_FLAMES, 8400);
                        break;
                    case EVENT_FEL_DECAY:
                        DoCastRandom(SPELL_FEL_DECAY, 150.00f, false, -SPELL_FEL_DECAY);
                        events.ScheduleEvent(EVENT_FEL_DECAY, 17000);
                        break;
                    case EVENT_DRAIN_ESSENCE_ILLIDAN:
                        DoCastAOE(SPELL_FEL_ADDLED);
                        if (Creature* illidan = instance->GetCreature(DATA_ILLIDAN_PART_ONE))
                        {
                            illidan->AI()->Talk(TALK_ILLIDAN_ABSORB);
                            illidan->CastSpell(me, SPELL_ABSORB_FEL_ENERGY, false);
                        }
                        events.ScheduleEvent(EVENT_CAMOUFLAGE, 5000);
                        break;
                    case EVENT_CAMOUFLAGE:
                        Talk(TALK_INTO_SHADOWS);
                        Talk(TALK_INTO_SHADOWS_EMOTE, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        DoCast(me, SPELL_CAMOUFLAGE, true);
                        if (Creature* illidan = instance->GetCreature(DATA_ILLIDAN_PART_ONE))
                        {
                            illidan->CastSpell(illidan, SPELL_REGENERATION, true);
                            illidan->AI()->Talk(TALK_ILLIDAN_RETURN);
                            Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();
                            for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                                if (Player* player = itr->getSource())
                                {
                                    if (!player->HasAura(SPELL_PLAYER_SHADOW_WALK))
                                    {
                                        player->CastSpell(player, SPELL_PLAYER_SHADOW_WALK, true);
                                        player->CastSpell(player, SPELL_PLAYER_SHADOW_WALK_VISUAL, true);
                                    }
                                }
                        }
                        events.ScheduleEvent(EVENT_START_EYE_EVENT, 3000);
                        break;
                    case EVENT_START_EYE_EVENT:
                        Talk(TALK_EYE_EVENT);
                        me->NearTeleportTo(eyeSummonPos);
                        for (int8 i = 0; i < 9; i++)
                            me->SummonCreature(NPC_HUNTING_CIRCLE, huntingCirclePos[i], TEMPSUMMON_TIMED_DESPAWN, 40000);
                        me->SummonCreature(NPC_HUNTING_STALKER, huntingStalkerPos, TEMPSUMMON_TIMED_DESPAWN, 40000);
                        events.ScheduleEvent(EVENT_END_EYE_EVENT, 40000);
                        break;
                    case EVENT_END_EYE_EVENT:
                        if (Creature* illidan = instance->GetCreature(DATA_ILLIDAN_PART_ONE))
                            illidan->AI()->Talk(TALK_ILLIDAN_STRENGTH_RETURN);
                        TalkWithDelay(3000, TALK_YOU_HIDE_WELL);
                        DoCastAOE(SPELL_CANCEL_FEL_FIREBALL_AGGRO);
                        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PLAYER_SHADOW_WALK_VISUAL);
                        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PLAYER_SHADOW_WALK);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        me->RemoveAurasDueToSpell(SPELL_CAMOUFLAGE);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->SetInCombatWithZone();
                        summons.DespawnEntry(NPC_EYE_OF_THE_PEROTHARN);
                        if (Creature* illidan = instance->GetCreature(DATA_ILLIDAN_PART_ONE))
                            DoCast(illidan, SPELL_END_EXHAUSTED, true);
                        DoCast(me, SPELL_ENFEEBLED, true);
                        skipUpdateVictim = false;
                        events.ScheduleEvent(EVENT_FEL_FLAMES, 5000);
                        events.ScheduleEvent(EVENT_FEL_DECAY, 8000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        bool drainEssenceTriggered;
        bool skipUpdateVictim;
        bool lazyEyeAchievementDone;
        uint8 demonWaves;
        uint8 demonWavesPortalOneAndTwo;
        uint32 eyeTimer;
        uint32 eyeCount;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_woe_perotharnAI(creature);
    }
};

class npc_woe_legion_demon : public CreatureScript
{
public:
    npc_woe_legion_demon() : CreatureScript("npc_woe_legion_demon") {}

    struct npc_woe_legion_demonAI : public ScriptedAI
    {
        npc_woe_legion_demonAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
            isLeftDemon = true;
            isPortalOneOrTwoDemon = false;
            isPortalOneWave = false;
            despawnBeforeNextTeleport = false;
        }

        void AttackStart(Unit* attacker) override {}
        void EnterEvadeMode() override {}

        bool CanAIAttack(Unit const* target) const override
        {
            return false;
        }

        void IsSummonedBy(Unit* summoner) override
        {
            if (me->GetPositionX() > 3449.00f && me->GetPositionX() < 3451.00f)
                isLeftDemon = false;
            
            if (me->GetPositionZ() <= 190.00f)
                isPortalOneOrTwoDemon = true;

            if (isPortalOneOrTwoDemon)
            {
                if (me->GetDistance2d(3324.450f, -4899.000f) <= 12.00f)
                    isPortalOneWave = true;

                if (isPortalOneWave)
                {
                    if (me->GetPositionX() > 3330.000f && me->GetPositionX() < 3332.000f)
                    {
                        isLeftDemon = false;
                        me->GetMotionMaster()->MovePoint(POINT_PORTAL, PortalOneWavePortalPositions[0], false);
                    }
                    else
                        me->GetMotionMaster()->MovePoint(POINT_PORTAL, PortalOneWavePortalPositions[1], false);
                }
                else
                {
                    if (me->GetPositionX() > 3341.000f && me->GetPositionX() < 3343.000f)
                    {
                        isLeftDemon = false;
                        me->GetMotionMaster()->MovePoint(POINT_PORTAL, PortalOneWavePortalPositions[2], false);
                    }
                    else
                        me->GetMotionMaster()->MovePoint(POINT_PORTAL, PortalOneWavePortalPositions[3], false);
                }
            }
            else
                me->GetMotionMaster()->MoveSmoothPath(isLeftDemon ? demonLeftPath : demonRightPath, demonPathSize);
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (!isPortalOneOrTwoDemon && type == SPLINE_MOTION_TYPE)
            {
                switch (id)
                {
                    case demonPathSize - 1:
                        events.ScheduleEvent(EVENT_TELEPORT_TO_HOME_POS, 100);
                        break;
                }
                return;
            }

            if (isPortalOneOrTwoDemon && type == POINT_MOTION_TYPE)
            {
                if (id == POINT_PORTAL)
                {
                    if (isPortalOneWave && me->FindNearestCreature(NPC_DISTRACT_STALKER, 100.00f))
                        me->SetVisible(false);
                    else
                        me->SetVisible(true);
                    events.ScheduleEvent(EVENT_TELEPORT_TO_HOME_POS, 100);
                }
            }
        }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
                case ACTION_DESPAWN_PORTAL_ONE:
                    if (isPortalOneWave)
                        despawnBeforeNextTeleport = true;
                    break;
                case ACTION_DESPAWN_PORTAL_TWO:
                    if (!isPortalOneWave && isPortalOneOrTwoDemon)
                        despawnBeforeNextTeleport = true;
                    break;
                case ACTION_DESPAWN_PORTAL_THREE:
                    if (!isPortalOneOrTwoDemon)
                        despawnBeforeNextTeleport = true;
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 const diff) override
        {
            events.Update(diff);

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_TELEPORT_TO_HOME_POS:
                        if (!despawnBeforeNextTeleport)
                        {
                            me->NearTeleportTo(me->GetHomePosition());
                            events.ScheduleEvent(isPortalOneOrTwoDemon ? EVENT_MOVE_TO_PORTAL : EVENT_MOVE_TO_COMBAT_AREA, 200);
                        }
                        else
                            me->DespawnOrUnsummon();
                        break;
                    case EVENT_MOVE_TO_COMBAT_AREA:
                        me->GetMotionMaster()->MoveSmoothPath(isLeftDemon ? demonLeftPath : demonRightPath, demonPathSize);
                        break;
                    case EVENT_MOVE_TO_PORTAL:
                        if (isPortalOneWave)
                        {
                            if (isLeftDemon)
                                me->GetMotionMaster()->MovePoint(POINT_PORTAL, PortalOneWavePortalPositions[1], false);
                            else
                                me->GetMotionMaster()->MovePoint(POINT_PORTAL, PortalOneWavePortalPositions[0], false);
                        }
                        else
                        {
                            if (isLeftDemon)
                                me->GetMotionMaster()->MovePoint(POINT_PORTAL, PortalOneWavePortalPositions[3], false);
                            else
                                me->GetMotionMaster()->MovePoint(POINT_PORTAL, PortalOneWavePortalPositions[2], false);
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        private:
            EventMap events;
            InstanceScript* instance;
            Position portalDest;
            bool isLeftDemon;
            bool isPortalOneOrTwoDemon;
            bool isPortalOneWave;
            bool despawnBeforeNextTeleport;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_woe_legion_demonAI(creature);
    }
};


class npc_woe_illidan_part_one : public CreatureScript
{
public:
    npc_woe_illidan_part_one() : CreatureScript("npc_woe_illidan_part_one") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();
        InstanceScript* instance = creature->GetInstanceScript();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
            {
                if (creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP))
                {
                    creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    creature->AI()->DoAction(ACTION_START_EVENT);
                }
                player->CLOSE_GOSSIP_MENU();
                break;
            }
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->isQuestGiver())
        {
            player->PrepareQuestMenu(creature->GetGUID());
            player->SendPreparedQuest(creature->GetGUID());
        }
        if (InstanceScript* instance = creature->GetInstanceScript())
            if (!instance->IsDone(DATA_PEROTHARN))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "start event", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    struct npc_woe_illidan_part_oneAI : public npc_escortAI
    {
        npc_woe_illidan_part_oneAI(Creature* creature) : npc_escortAI(creature)
        {
            me->GetPosition(&spawnPosition);
            instance = me->GetInstanceScript();
        }

        void InitializeAI() override
        {
            isInitialStart = true;
            EscortStarted = false;
            npc_escortAI::InitializeAI();
        }

        void Reset()
        {
            DoCast(me, SPELL_DEMONIC_SIGHT, true);
            DoCast(me, SPELL_ILLIDAN_SHADOWCLOAK, true);
            Position pos(*me);
            me->SummonCreature(NPC_ILLIDAN_SHADOWCLOAK_STALKER, pos, TEMPSUMMON_MANUAL_DESPAWN);
        }

        void EnterCombat(Unit* target) override
        {
            Talk(TALK_ILLIDAN_AGGRO);
        }

        void EnterEvadeMode() override
        {
            if (!me->HasAura(SPELL_REGENERATION))
            {
                me->DeleteThreatList();
                me->CombatStop(true);
                DoCast(me, SPELL_DEMONIC_SIGHT, true);
            }
        }

        void ResetCombat(bool fullReset)
        {
            me->RemoveAurasDueToSpell(SPELL_REGENERATION);
            me->DeleteThreatList();
            me->CombatStop(true);
            me->setFaction(ILLIDAN_FACTION_ESCORT);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
            SetCanAttack(false);
            me->GetMotionMaster()->MoveTargetedHome();
            events.Reset();
            if (fullReset)
            {
                me->NearTeleportTo(spawnPosition);
                me->Kill(me);
                me->Respawn();
                EscortStarted = false;
            }
            else
            {
                events.ScheduleEvent(EVENT_PLAYER_IN_COMBAT_CHECK, 1000);
                events.ScheduleEvent(EVENT_SHADOWCLOAK_CHECK, 1000);
            }
        }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
                case ACTION_START_EVENT:
                {
                    if (isInitialStart)
                    {
                        isInitialStart = false;
                        demonKillCounter = 0;
                        currentPortalAction = ACTION_DESPAWN_PORTAL_ONE;
                    }
                    Talk(TALK_ILLIDAN_HIDE_IN_SHADOWS);
                    TalkWithDelay(4000, TALK_ILLIDAN_START_MOVEMENT);
                    events.ScheduleEvent(EVENT_SHADOWCLOAK_CHECK, 100);
                    DoCast(me, SPELL_ILLIDAN_SHADOWCLOAK, true);
                    Position pos(*me);
                    me->SummonCreature(NPC_ILLIDAN_SHADOWCLOAK_STALKER, pos, TEMPSUMMON_MANUAL_DESPAWN);
                    events.ScheduleEvent(EVENT_RESTART_ESCORT, 8000);
                    break;
                }
                case ACTION_DEMON_KILLED:
                {
                    if (++demonKillCounter == 3 || demonKillCounter == 6 || demonKillCounter == 7 || demonKillCounter == 10)
                    {
                        switch (demonKillCounter)
                        {
                            case 3:
                                if (GameObject* energyFocus = instance->GetGameObject(DATA_ENERGY_FOCUS_ONE))
                                {
                                    Talk(TALK_ILLIDAN_DESTROY_THE_CRYSTALL);
                                    energyFocus->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED);
                                    energyFocus->SetFlag(GAMEOBJECT_DYNAMIC, 4294901760);
                                }
                                break;
                            case 6:
                                if (GameObject* energyFocus = instance->GetGameObject(DATA_ENERGY_FOCUS_TWO))
                                {
                                    Talk(TALK_ILLIDAN_DESTROY_THE_CRYSTALL);
                                    energyFocus->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED);
                                    energyFocus->SetFlag(GAMEOBJECT_DYNAMIC, 4294901760);
                                }
                                break;
                            case 7:
                                Talk(TALK_ILLIDAN_SHUT_DOWN_LAST_PORTAL);
                                SetEscortPaused(false);
                                break;
                            case 10:
                                if (GameObject* energyFocus = instance->GetGameObject(DATA_ENERGY_FOCUS_THREE))
                                {
                                    Talk(TALK_ILLIDAN_DESTROY_THE_CRYSTALL);
                                    energyFocus->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED);
                                    energyFocus->SetFlag(GAMEOBJECT_DYNAMIC, 4294901760);
                                }
                                break;
                            default:
                                break;
                        }
                        if (Creature* visual = me->FindNearestCreature(NPC_FEL_CRYSTAL_STALKER, 50.00f))
                            visual->CastSpell(visual, SPELL_ARCANE_EXPLOSION_VISUAL, true);
                        events.CancelEvent(EVENT_PLAYER_IN_COMBAT_CHECK); 
                        events.CancelEvent(EVENT_PLAYER_ALIVE_CHECK);
                        SetCanAttack(false);
                        me->setFaction(ILLIDAN_FACTION_ESCORT);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
                    }
                    break;
                }
                case ACTION_MOVE_NEXT_PORTAL:
                {              
                    if (!me->HasAura(SPELL_ILLIDAN_SHADOWCLOAK))
                    {
                        DoCast(me, SPELL_ILLIDAN_SHADOWCLOAK, true);
                        Position pos(*me);
                        me->SummonCreature(NPC_ILLIDAN_SHADOWCLOAK_STALKER, pos, TEMPSUMMON_MANUAL_DESPAWN);
                    }
                    TalkWithDelay(5000, TALK_ILLIDAN_LEAVING);
                    if (Creature* perotharn = instance->GetCreature(DATA_PEROTHARN))
                        perotharn->AI()->DoAction(currentPortalAction);

                    Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();
                    for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                        if (Player* player = itr->getSource())
                            if (player->GetQuestStatus(QUEST_IN_UNENDING_NUMBERS) == QUEST_STATUS_INCOMPLETE)
                                player->KilledMonsterCredit(currentPortalAction == 10 ? QUEST_CREDIT_PORTAL_ONE : (currentPortalAction == 11 ? QUEST_CREDIT_PORTAL_TWO : QUEST_CREDIT_PORTAL_THREE));

                    events.ScheduleEvent(EVENT_RESTART_ESCORT, 6000);
                    currentPortalAction++;
                    break;
                }
                case ACTION_COMBAT_FINISHED:
                    events.Reset();
                    TalkWithDelay(3000, TALK_ILLIDAN_PEROTHARN_DEATH);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PLAYER_SHADOWCLOAK);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PLAYER_SHADOW_WALK);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PLAYER_SHADOW_WALK_VISUAL);
                    SetEscortPaused(false);
                    break;
                default:
                    break;
            }
        }

        void DamageTaken(Unit* attacker, uint32& damage)
        {
            if (me->HealthBelowPctDamaged(10, damage))
                me->SetHealth(me->GetMaxHealth());
        }

        void JustSummoned(Creature* summon) override
        {
            if (summon->GetEntry() == NPC_ILLIDAN_SHADOWCLOAK_STALKER)
            {
                summon->CastSpell(summon, SPELL_ILLIDAN_STALKER_SHADOWCLOAK_COSMETIC, true);
                summon->CastSpell(me, SPELL_RIDE_VEHICLE_HARDCODED, true);
                summon->AddUnitTypeMask(UNIT_MASK_ACCESSORY);
            }
            npc_escortAI::JustSummoned(summon);
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (!EscortStarted && who->GetTypeId() == TYPEID_PLAYER)
            {
                if (!instance->IsDone(DATA_PEROTHARN))
                {
                    if (me->GetDistance2d(who) <= 50.00f)
                    {
                        EscortStarted = true;
                        Start(false, false, who->GetGUID());
                        SetMaxPlayerDistance(500.0f);
                        SetDespawnAtEnd(false);
                        SetDespawnAtFar(false);
                    }
                }
            }
            npc_escortAI::MoveInLineOfSight(who);
        }

        void WaypointReached(uint32 waypointId) override
        {
            currentWaypoint = waypointId;
            switch (waypointId)
            {
                case 1:
                    Talk(TALK_ILLIDAN_MOVE_IN_LINE_OF_SIGHT);
                    break;
                case 5:
                    Talk(TALK_ILLIDAN_WAIT_FOR_EVENT_START);
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    me->RemoveAurasDueToSpell(SPELL_ILLIDAN_SHADOWCLOAK);
                    SetEscortPaused(true);
                    break;
                case 9:
                    Talk(TALK_ILLIDAN_MANY_DEMONS);
                    break;
                case 10:
                    Talk(TALK_ILLIDAN_SINGLE_GUARDIAN_TALK);
                    break;
                case 15:
                    me->CastWithDelay(5000, me, SPELL_ILLIDAN_WALL_OF_SHADOWS);
                    Talk(TALK_ILLIDAN_HOLD_THEM_BACK);
                    TalkWithDelay(18000, TALK_ILLIDAN_MAGIC_IS_FADING);
                    break;
                case 26:
                {
                    if (currentPortalAction == 10)
                    {
                        Talk(TALK_ILLIDAN_ATTACK_CRYSTALL);
                        SetEscortPaused(true);
                        Position newHomePos(*me);
                        me->SetHomePosition(newHomePos);
                        events.ScheduleEvent(EVENT_PLAYER_IN_COMBAT_CHECK, 100);
                    }
                    break;
                }
                case 32:
                    Talk(TALK_ILLIDAN_THEY_COME_ENDLESSLY);
                    break;
                case 46:
                {
                    Talk(TALK_ILLIDAN_ATTACK_CRYSTALL);
                    SetEscortPaused(true);
                    Position newHomePos(*me);
                    me->SetHomePosition(newHomePos);
                    events.ScheduleEvent(EVENT_PLAYER_IN_COMBAT_CHECK, 100);
                    break;
                }
                case 51:
                    Talk(TALK_ILLIDAN_THE_STENCH_OF_SULFUR);
                    break;
                case 72:
                {
                    Talk(TALK_ILLIDAN_CUT_THIS_ONE_DOWN);
                    if (Creature* demon = me->FindNearestCreature(NPC_LEGION_DEMON, 20.00f))
                        demon->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_6 | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                    SetEscortPaused(true);
                    Position newHomePos(*me);
                    me->SetHomePosition(newHomePos);
                    events.ScheduleEvent(EVENT_PLAYER_IN_COMBAT_CHECK, 100);
                    break;
                }
                case 81:
                {
                    Talk(TALK_ILLIDAN_ATTACK_CRYSTALL);
                    SetEscortPaused(true);
                    Position newHomePos(*me);
                    me->SetHomePosition(newHomePos);
                    events.ScheduleEvent(EVENT_PLAYER_IN_COMBAT_CHECK, 100);
                    break;
                }
                case 89:
                    Talk(TALK_ILLIDAN_DEMONS_LEAVING);
                    me->DespawnCreaturesInArea(NPC_GUARDIAN_DEMON, 300.00f);
                    break;
                case 99:
                    Talk(TALK_ILLIDAN_DEMONS_NOT_ON_PLACE);
                    break;
                case 103:
                    Talk(TALK_ILLIDAN_TOO_EASY);
                    break;
                case 109:
                {
                    Position newHomePos(*me);
                    me->SetHomePosition(newHomePos);
                    Talk(TALK_ILLIDAN_ANOTHER_DEMON);
                    TalkWithDelay(8000, TALK_ILLIDAN_PATIENCE);
                    SetEscortPaused(true);
                    events.ScheduleEvent(EVENT_PLAYER_IN_COMBAT_CHECK, 10000);
                    events.ScheduleEvent(EVENT_START_BOSS_FIGHT, 10000);
                    break;
                }
                case 110:
                    Talk(TALK_ILLIDAN_GOOD_BYE);
                    break;
                case 123:
                    me->DespawnOrUnsummon();
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 const diff) override
        {
            npc_escortAI::UpdateAI(diff);
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_RESTART_ESCORT:
                        SetEscortPaused(false);
                        break;
                    case EVENT_SHADOWCLOAK_CHECK:
                    {
                        Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();
                        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                            if (Player* player = itr->getSource())
                                if (!player->HasAura(SPELL_PLAYER_SHADOWCLOAK) && !player->HasAura(SPELL_DEMONIC_GRIP_ROOT) && player->isAlive())
                                    player->CastSpell(player, SPELL_PLAYER_SHADOWCLOAK, true);
                        events.ScheduleEvent(EVENT_SHADOWCLOAK_CHECK, 1000);
                        break;
                    }
                    case EVENT_MOVE_TO_NEXT_PORTAL:
                        Talk(TALK_ILLIDAN_LEAVING);
                        SetEscortPaused(false);
                        break;
                    case EVENT_START_BOSS_FIGHT:
                        if (Creature* perotharn = instance->GetCreature(DATA_PEROTHARN))
                            perotharn->AI()->DoAction(ACTION_START_COMBAT);
                        break;
                    case EVENT_PLAYER_IN_COMBAT_CHECK:
                    {
                        bool needsNewCheck = true;
                        Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();
                        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                            if (Player* player = itr->getSource())
                                if (player->GetDistance2d(me) <= 40.00f && player->isInCombat())
                                {
                                    SetCanAttack(true);
                                    me->setFaction(ILLIDAN_FACTION_ATTACKER);
                                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
                                    me->RemoveAurasDueToSpell(SPELL_ILLIDAN_SHADOWCLOAK);
                                    needsNewCheck = false;
                                    events.ScheduleEvent(EVENT_PLAYER_ALIVE_CHECK, 1000);
                                    break;
                                }

                        if (needsNewCheck)
                            events.ScheduleEvent(EVENT_PLAYER_IN_COMBAT_CHECK, 1000);
                        break;
                    }
                    case EVENT_PLAYER_ALIVE_CHECK:
                    {
                        bool alivePlayerFound = false;
                        Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();
                        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                            if (Player* player = itr->getSource())
                                if (!player->isGameMaster() && player->isAlive())
                                {
                                    alivePlayerFound = true;
                                    break;
                                }

                        if (!alivePlayerFound)
                            ResetCombat(currentWaypoint < 27);
                        else
                            events.ScheduleEvent(EVENT_PLAYER_ALIVE_CHECK, 1000);
                        break;
                    }
                    default:
                        break;
                }
            }
            if (!me->HasAura(SPELL_REGENERATION) && !UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
        InstanceScript* instance;
        bool EscortStarted;
        bool isInitialStart;
        uint8 demonKillCounter;
        uint8 currentPortalAction;
        uint8 currentWaypoint;
        Position spawnPosition;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_woe_illidan_part_oneAI(creature);
    }
};

class npc_woe_nozdormu_part_one : public CreatureScript
{
public:
    npc_woe_nozdormu_part_one() : CreatureScript("npc_woe_nozdormu_part_one") {}

    struct npc_woe_nozdormu_part_oneAI : public ScriptedAI
    {
        npc_woe_nozdormu_part_oneAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            instance = me->GetInstanceScript();
            introStarted = false;
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (!introStarted)
            {
                if (!instance->IsDone(DATA_PEROTHARN))
                {
                    if (who->GetTypeId() == TYPEID_PLAYER)
                    {
                        introStarted = true;
                        me->SummonCreatureGroup(SUMMON_GROUP_EVENT_INTRO);
                        Talk(TALK_NOZDORMU_INTRO);
                        TalkWithDelay(10000, TALK_NOZDORMU_INTRO_TWO);
                        TalkWithDelay(16000, TALK_NOZDORMU_INTRO_THREE);
                        events.ScheduleEvent(EVENT_START_PEROTHARN_INTRO, 28000);
                    }
                }
            }
        }

        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_6 | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
            if (summon->GetEntry() == NPC_LEGION_DEMON)
                summon->AI()->DoAction(ACTION_SET_FIRST_FIREWALL_DEMON);
        }

        void UpdateAI(uint32 const diff) override
        {
            events.Update(diff);

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_START_PEROTHARN_INTRO:
                        if (Creature* perotharn = instance->GetCreature(DATA_PEROTHARN))
                        {
                            perotharn->AI()->Talk(TALK_PEROTHARN_INTRO);
                            perotharn->AI()->TalkWithDelay(6000, TALK_PEROTHARN_INTRO_TWO);
                            perotharn->AI()->TalkWithDelay(9000, TALK_PEROTHARN_INTRO_THREE);
                        }
                        for (uint64 summonGuid : summons)
                            if (Creature* summon = ObjectAccessor::GetCreature(*me, summonGuid))
                                if (summon->GetEntry() == NPC_LEGION_DEMON)
                                    summon->EmoteWithDelay(6000, EMOTE_ONESHOT_SALUTE_NO_SHEATH);
                        events.ScheduleEvent(EVENT_START_DEMON_MOVEMENT, 12000);
                        break;
                    case EVENT_START_DEMON_MOVEMENT:
                        for (uint64 summonGuid : summons)
                        {
                            if (Creature* summon = ObjectAccessor::GetCreature(*me, summonGuid))
                            {
                                switch (summon->GetEntry())
                                {
                                    case NPC_LEGION_DEMON:
                                        summon->GetMotionMaster()->MovePoint(POINT_SUMMON_FIREWALL, demonEndPos);
                                        break;
                                    default:
                                        summon->GetMotionMaster()->MovePoint(POINT_EVENT_WALK, eventWaypoints[0]);
                                        break;
                                }
                            }
                        }

                        if (Creature* perotharn = instance->GetCreature(DATA_PEROTHARN))
                            perotharn->GetMotionMaster()->MovePoint(POINT_EVENT_WALK, eventWaypoints[1]);
                        break;
                    default:
                        break;
                }
            }
        }
    private:
        EventMap events;
        InstanceScript* instance;
        SummonList summons;
        bool introStarted;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_woe_nozdormu_part_oneAI(creature);
    }
};

class npc_woe_legion_demon_firewall : public CreatureScript
{
public:
    npc_woe_legion_demon_firewall() : CreatureScript("npc_woe_legion_demon_firewall") {}

    struct npc_woe_legion_demon_firewallAI : public ScriptedAI
    {
        npc_woe_legion_demon_firewallAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
            isSecFirewallDemon = true;
        }

        void InitializeAI() override
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_6 | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
            ScriptedAI::InitializeAI();
        }

        void Reset()
        {
            events.Reset();
        }

        void DoAction(int32 const action) override
        {
            if (action == ACTION_SET_FIRST_FIREWALL_DEMON)
            {
                events.SetPhase(EVENT_PHASE_INTRO);
                isSecFirewallDemon = false;
            }
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != POINT_MOTION_TYPE || id != POINT_SUMMON_FIREWALL)
                return;

            Position pos(*me);
            me->SetHomePosition(pos);

            me->SetFacingTo(5.150090f);
            me->AddAura(SPELL_SUMMON_FIREWALL_PERIODIC, me);
            DoCast(me, SPELL_SUMMON_FIREWALL);
            events.ScheduleEvent(EVENT_REMOVE_FLAGS, 2000);
        }

        void EnterCombat(Unit* attacker) override
        {
            events.RemovePhase(EVENT_PHASE_INTRO);
            events.ScheduleEvent(EVENT_STRIKE_FEAR, 5000);
        }

        void JustDied(Unit* killer) override
        {
            if (!isSecFirewallDemon)
            {
                if (GameObject* go = instance->GetGameObject(DATA_COURTYYARD_DOOR_ONE))
                    go->SetGoState(GO_STATE_ACTIVE);

                if (GameObject* go = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_FIREWALL_ONE)))
                    go->SetGoState(GO_STATE_ACTIVE);
            }
            else
            {
                if (GameObject* go = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_FIREWALL_TWO)))
                    go->SetGoState(GO_STATE_ACTIVE);
                if (GameObject* go = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_FIREWALL_THREE)))
                    go->SetGoState(GO_STATE_ACTIVE);
                if (Creature* illidan = instance->GetCreature(DATA_ILLIDAN_PART_ONE))
                    illidan->AI()->DoAction(ACTION_DEMON_KILLED);
            }

        }

        void UpdateAI(uint32 const diff) override
        {
            if (!events.IsInPhase(EVENT_PHASE_INTRO) && !UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_REMOVE_FLAGS:
                        if (GameObject* go = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_FIREWALL_ONE)))
                            go->SetGoState(GO_STATE_READY);

                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_6 | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                        break;
                    case EVENT_STRIKE_FEAR:
                        DoCastAOE(SPELL_STRIKE_FEAR);
                        events.ScheduleEvent(EVENT_STRIKE_FEAR, 7000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
        InstanceScript* instance;
        bool isSecFirewallDemon;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_woe_legion_demon_firewallAI(creature);
    }
};

class npc_woe_dreadlord_defender : public CreatureScript
{
public:
    npc_woe_dreadlord_defender() : CreatureScript("npc_woe_dreadlord_defender") {}

    struct npc_woe_dreadlord_defenderAI : public ScriptedAI
    {
        npc_woe_dreadlord_defenderAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
            events.Reset();
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE)
            {
                switch (id)
                {
                    case POINT_EVENT_WALK:
                        events.SetPhase(EVENT_PHASE_INTRO);
                        events.ScheduleEvent(EVENT_START_NEXT_WAYPOINT, 100);
                        break;
                    case POINT_EVENT_WALK_TWO:
                        events.ScheduleEvent(EVENT_LAST_WAYPOINT, 100);
                        break;
                    default:
                        break;
                }
            }
        }

        void EnterCombat(Unit* attacker) override
        {
            events.RemovePhase(EVENT_PHASE_INTRO);
            events.ScheduleEvent(EVENT_DEMONIC_WARDING, urand(1000,10000));
            events.ScheduleEvent(EVENT_CARRION_SWARM, 5000);
        }

        void JustDied(Unit* killer) override
        {
            if (Creature* illidan = instance->GetCreature(DATA_ILLIDAN_PART_ONE))
                illidan->AI()->DoAction(ACTION_DEMON_KILLED);
        }

        void UpdateAI(uint32 const diff) override
        {
            if (!events.IsInPhase(EVENT_PHASE_INTRO) && !UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_DEMONIC_WARDING:
                        if (Creature* arcanist = me->FindNearestCreature(NPC_CORRUPTED_ARCANIST, 40.00f))
                            if (!arcanist->HasAura(SPELL_DEMONIC_WARDING))
                                DoCast(arcanist, SPELL_DEMONIC_WARDING);
                        events.ScheduleEvent(EVENT_DEMONIC_WARDING, urand(8000, 16000));
                        break;
                    case EVENT_CARRION_SWARM:
                        DoCastVictim(SPELL_CARRION_SWARM);
                        events.ScheduleEvent(EVENT_CARRION_SWARM, 5500);
                        break;
                    case EVENT_START_NEXT_WAYPOINT:
                        me->GetMotionMaster()->MovePoint(POINT_EVENT_WALK_TWO, eventWaypoints[1]);
                        break;
                    case EVENT_LAST_WAYPOINT:
                        me->GetMotionMaster()->MovePoint(POINT_EVENT_WALK_TWO, eventWaypoints[2]);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
        private:
            EventMap events;
            InstanceScript* instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_woe_dreadlord_defenderAI(creature);
    }
};

class npc_woe_corrupted_arcanist : public CreatureScript
{
public:
    npc_woe_corrupted_arcanist() : CreatureScript("npc_woe_corrupted_arcanist") {}

    struct npc_woe_corrupted_arcanistAI : public ScriptedAI
    {
        npc_woe_corrupted_arcanistAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
            if (!me->HasAura(SPELL_CORRUPTED_NIGHT_ELF_EYES))
                DoCast(me, SPELL_CORRUPTED_NIGHT_ELF_EYES, true);
            if (!me->HasAura(SPELL_DISCIPLINED_CASTER))
                DoCast(me, SPELL_DISCIPLINED_CASTER, true);
            events.Reset();
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE)
            {
                switch (id)
                {
                    case POINT_EVENT_WALK:
                        events.SetPhase(EVENT_PHASE_INTRO);
                        events.ScheduleEvent(EVENT_START_NEXT_WAYPOINT, 100);
                        break;
                    case POINT_EVENT_WALK_TWO:
                        events.ScheduleEvent(EVENT_LAST_WAYPOINT, 100);
                        break;
                    default:
                        break;
                }
            }
        }

        void EnterCombat(Unit* attacker) override
        {
            events.RemovePhase(EVENT_PHASE_INTRO);
            events.ScheduleEvent(EVENT_ARCANE_ANNIHILATION, 1000);
            events.ScheduleEvent(EVENT_CHECK_MANA_POOL, 1000);
        }

        void JustDied(Unit* killer) override
        {
            if (Creature* illidan = instance->GetCreature(DATA_ILLIDAN_PART_ONE))
                illidan->AI()->DoAction(ACTION_DEMON_KILLED);
        }

        void UpdateAI(uint32 const diff) override
        {
            if (!events.IsInPhase(EVENT_PHASE_INTRO) && !UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_START_NEXT_WAYPOINT:
                        me->GetMotionMaster()->MovePoint(POINT_EVENT_WALK_TWO, eventWaypoints[1]);
                        break;
                    case EVENT_LAST_WAYPOINT:
                        me->GetMotionMaster()->MovePoint(POINT_EVENT_WALK_TWO, eventWaypoints[2]);
                        break;
                    case EVENT_CHECK_MANA_POOL:
                        if (me->GetPowerPct(POWER_MANA) <= 11)
                        {
                            DoCast(me, SPELL_INFINITE_MANA);
                            events.DelayEvents(6000);
                            events.ScheduleEvent(EVENT_CHECK_MANA_POOL, 6000);
                        }
                        events.ScheduleEvent(EVENT_CHECK_MANA_POOL, 1000);
                        break;
                    case EVENT_ARCANE_ANNIHILATION:
                        DoCastVictim(SPELL_ARCANE_ANNIHILATION);
                        events.ScheduleEvent(EVENT_ARCANE_ANNIHILATION, 3100);
                        break;
                default:
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
        InstanceScript* instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_woe_corrupted_arcanistAI(creature);
    }
};

class npc_woe_guardian_demon : public CreatureScript
{
public:
    npc_woe_guardian_demon() : CreatureScript("npc_woe_guardian_demon") {}

    struct npc_woe_guardian_demonAI : public ScriptedAI
    {
        npc_woe_guardian_demonAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void EnterEvadeMode()
        {
            if (me->HasUnitState(UNIT_STATE_CASTING) || me->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT) != 0)
                return;

            ScriptedAI::EnterEvadeMode();
        }

        void EnterCombat(Unit* attacker) override
        {
            attacker->RemoveAurasDueToSpell(SPELL_PLAYER_SHADOW_WALK);
            attacker->RemoveAurasDueToSpell(SPELL_PLAYER_SHADOW_WALK_VISUAL);
            attacker->RemoveAurasDueToSpell(SPELL_PLAYER_SHADOW_AMBUSHER_STEALTH);
            attacker->CastSpell(attacker, SPELL_DEMONIC_GRIP_ROOT, true);
            DoCast(attacker, SPELL_DEMONIC_GRIP, false);
        }

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_woe_guardian_demonAI(creature);
    }
};

class go_woe_portal_energy_focus : public GameObjectScript
{
public:
    go_woe_portal_energy_focus() : GameObjectScript("go_woe_portal_energy_focus") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (!go->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE))
        {
            go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
            if (Creature* stalker = go->FindNearestCreature(NPC_FEL_CRYSTAL_STALKER, 5.0f))
            {
                stalker->RemoveAurasDueToSpell(SPELL_FEL_CRYSTAL_SWITCH_GLOW);
                stalker->CastSpell(stalker, SPELL_FEL_CRYSTAL_MELTDOWN, false);
            }

            if (InstanceScript* instance = go->GetInstanceScript())
                if (Creature* illidan = instance->GetCreature(DATA_ILLIDAN_PART_ONE))
                    illidan->AI()->DoAction(ACTION_MOVE_NEXT_PORTAL);
        }

        return true;
    }
};

class go_woe_time_transit_device : public GameObjectScript
{
public:
    go_woe_time_transit_device() : GameObjectScript("go_woe_time_transit_device") {}

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (player->isInCombat())
            return true;
        if (InstanceScript *instance = go->GetInstanceScript())
        {
            if (go->GetEntry() != 209998)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to the garden entrance", GOSSIP_SENDER_MAIN, SPELL_TELEPORT_COURTYARD_ENTRANCE);
            if (go->GetEntry() != 209997 && instance->IsDone(DATA_PEROTHARN))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to Azshara's Palace.", GOSSIP_SENDER_MAIN, SPELL_TELEPORT_AZSHARAS_PALACE);
            if (instance->IsDone(DATA_PEROTHARN) && instance->IsDone(DATA_QUEEN_AZSHARA))
            {
                if (go->GetEntry() != 210000)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to Azshara's Palace Overlook.", GOSSIP_SENDER_MAIN, SPELL_TELEPORT_AZSHARAS_PALACE_OVERLOOK);
                if (go->GetEntry() != 209999)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to The Well of Eternity.", GOSSIP_SENDER_MAIN, SPELL_TELEPORT_WELL_OF_ETERNITY);
            }
        }
        player->SEND_GOSSIP_MENU(player->GetGossipTextId(go), go->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, GameObject* go, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (player->isInCombat())
            return true;
        player->CastSpell(player, action, true);
        return true;
    }
};

class spell_woe_fel_decay_heal : public SpellScriptLoader
{
public:
    spell_woe_fel_decay_heal() : SpellScriptLoader("spell_woe_fel_decay_heal") { }

    class spell_woe_fel_decay_heal_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_woe_fel_decay_heal_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            if (eventInfo.GetHealInfo() && eventInfo.GetHealInfo()->GetSpellInfo())
                return true;
            return false;
        }

        void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            if (InstanceScript* instance = GetTarget()->GetInstanceScript())
            {
                if (Creature* perotharn = instance->GetCreature(DATA_PEROTHARN))
                {
                    int32 damage = eventInfo.GetHealInfo()->GetHeal();
                    perotharn->CastCustomSpell(eventInfo.GetActor(), SPELL_FEL_SURGE, &damage, NULL, NULL, true);
                }
            }
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_woe_fel_decay_heal_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_woe_fel_decay_heal_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_woe_fel_decay_heal_AuraScript();
    }
};

class spell_woe_drain_essence : public SpellScriptLoader
{
public:
    spell_woe_drain_essence() : SpellScriptLoader("spell_woe_drain_essence") { }

private:
    class spell_woe_drain_essence_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_woe_drain_essence_SpellScript)

        void TriggerSpell(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            GetCaster()->CastSpell(GetCaster(), GetSpellInfo()->Effects[effIndex].TriggerSpell, true);
        }

        void Register()
        {
            OnEffectLaunch += SpellEffectFn(spell_woe_drain_essence_SpellScript::TriggerSpell, EFFECT_2, SPELL_EFFECT_TRIGGER_SPELL);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_woe_drain_essence_SpellScript();
    }
};

class spell_woe_summon_circle_spawn : public SpellScriptLoader
{
public:
    spell_woe_summon_circle_spawn() : SpellScriptLoader("spell_woe_summon_circle_spawn") { }

    class spell_woe_summon_circle_spawn_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_woe_summon_circle_spawn_AuraScript);

        void CalcPeriodic(AuraEffect const* /*effect*/, bool& isPeriodic, int32& amplitude)
        {
            isPeriodic = true;
            amplitude = 4000;
        }

        void OnPeriodicTick(AuraEffect const* /*aurEff*/)
        {
            if (InstanceScript* instance = GetTarget()->GetInstanceScript())
                if (Creature* perotharn = instance->GetCreature(DATA_PEROTHARN))
                {
                    Position summonPos(*GetTarget());
                    GetTarget()->PlayOneShotAnimKit(ANIM_ONE_SHOT_CIRCLE);
                    perotharn->SummonCreature(NPC_EYE_OF_THE_PEROTHARN, summonPos);
                }
        }

        void Register()
        {
            DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_woe_summon_circle_spawn_AuraScript::CalcPeriodic, EFFECT_0, SPELL_AURA_DUMMY);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_woe_summon_circle_spawn_AuraScript::OnPeriodicTick, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_woe_summon_circle_spawn_AuraScript();
    }
};

class spell_woe_punishing_flames : public SpellScriptLoader
{
public:
    spell_woe_punishing_flames() : SpellScriptLoader("spell_woe_punishing_flames") { }

    class spell_woe_punishing_flames_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_woe_punishing_flames_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            Unit* caster = GetCaster();
            targets.remove_if([caster](WorldObject* target)
            {
                return caster->GetDistance2d(target) < 70.00f;
            });
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_woe_punishing_flames_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_woe_punishing_flames_SpellScript();
    }
};

class spell_woe_fel_fireball_aggro : public SpellScriptLoader
{
public:
    spell_woe_fel_fireball_aggro() : SpellScriptLoader("spell_woe_fel_fireball_aggro") { }

    class spell_woe_fel_fireball_aggro_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_woe_fel_fireball_aggro_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (InstanceScript* instance = GetCaster()->GetInstanceScript())
            {
                if (Creature* perotharn = instance->GetCreature(DATA_PEROTHARN))
                {
                    GetCaster()->CastSpell(GetCaster(), SPELL_EASY_PREY, true, NULL, NULL, perotharn->GetGUID());
                    GetCaster()->CastSpell(GetCaster(), SPELL_EASY_PREY_SUMMON, true);
                    perotharn->AI()->DoAction(ACTION_PLAYER_FOUND);
                }
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_woe_fel_fireball_aggro_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript *GetSpellScript() const override
    {
        return new spell_woe_fel_fireball_aggro_SpellScript();
    }
};

class spell_woe_tracked_lock_on_player : public SpellScriptLoader
{
public:
    spell_woe_tracked_lock_on_player() : SpellScriptLoader("spell_woe_tracked_lock_on_player") { }

    class spell_woe_tracked_lock_on_player_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_woe_tracked_lock_on_player_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(Trinity::UnitAuraCheck(false, SPELL_EASY_PREY));
            // should never happen
            if (targets.size() > 1)
                Trinity::Containers::RandomResizeList(targets, 1);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_woe_tracked_lock_on_player_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_woe_tracked_lock_on_player_SpellScript();
    }
};

class spell_woe_drain_essence_damage : public SpellScriptLoader
{
public:
    spell_woe_drain_essence_damage() : SpellScriptLoader("spell_woe_drain_essence_damage") { }

    class spell_woe_drain_essence_damage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_woe_drain_essence_damage_SpellScript);

        void HandleDamage(SpellEffIndex /*effIndex*/)
        {
            if (Unit* target = GetHitUnit())
                SetHitDamage(target->CountPctFromMaxHealth(5));
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_woe_drain_essence_damage_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_woe_drain_essence_damage_SpellScript();
    }
};

class spell_woe_drain_essence_knockback : public SpellScriptLoader
{
public:
    spell_woe_drain_essence_knockback() : SpellScriptLoader("spell_woe_drain_essence_knockback") { }

    class spell_woe_drain_essence_knockback_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_woe_drain_essence_knockback_SpellScript);

        void ApplyStun(SpellEffIndex /*effIndex*/)
        {
            if (Unit* target = GetHitUnit())
                target->CastSpell(target, SPELL_ASPHYXIATE, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_woe_drain_essence_knockback_SpellScript::ApplyStun, EFFECT_0, SPELL_EFFECT_KNOCK_BACK);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_woe_drain_essence_knockback_SpellScript();
    }
};

class spell_woe_shadowcloak_periodic : public SpellScriptLoader
{
public:
    spell_woe_shadowcloak_periodic() : SpellScriptLoader("spell_woe_shadowcloak_periodic") { }

    class spell_woe_shadowcloak_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_woe_shadowcloak_periodic_AuraScript);

        void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
        {
            if (GetTarget()->HasAura(SPELL_PLAYER_SHADOW_WALK))
            {
                GetTarget()->RemoveAurasDueToSpell(SPELL_PLAYER_SHADOW_WALK);
                GetTarget()->RemoveAurasDueToSpell(SPELL_PLAYER_SHADOW_WALK_VISUAL);
                GetTarget()->RemoveAurasDueToSpell(SPELL_PLAYER_SHADOW_AMBUSHER_STEALTH);
                GetTarget()->CastSpell(GetTarget(), SPELL_PLAYER_SHADOW_AMBUSHER, true);
            }
        }

        void onPeriodicTick(AuraEffect const* aurEff)
        {
            if (!GetTarget()->isInCombat())
            {
                if (!GetTarget()->HasAura(SPELL_PLAYER_SHADOW_WALK))
                    GetTarget()->CastSpell(GetTarget(), SPELL_PLAYER_SHADOW_WALK, true);

                if (!GetTarget()->HasAura(SPELL_PLAYER_SHADOW_AMBUSHER_STEALTH))
                    GetTarget()->CastSpell(GetTarget(), SPELL_PLAYER_SHADOW_AMBUSHER_STEALTH, true);

                GetTarget()->CastSpell(GetTarget(), SPELL_PLAYER_SHADOW_WALK_VISUAL, true);
            }
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(spell_woe_shadowcloak_periodic_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_woe_shadowcloak_periodic_AuraScript::onPeriodicTick, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_woe_shadowcloak_periodic_AuraScript();
    }
};

class spell_woe_shadow_walk : public SpellScriptLoader
{
public:
    spell_woe_shadow_walk() : SpellScriptLoader("spell_woe_shadow_walk") { }

    class spell_woe_shadow_walk_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_woe_shadow_walk_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Position pos(*GetTarget());
            if (Creature* creature = GetTarget()->SummonCreature(NPC_PLAYER_SHADOWCLOAK_STALKER, pos, TEMPSUMMON_MANUAL_DESPAWN))
            {
                creature->CastSpell(creature, SPELL_PLAYER_SHADOWCLOAK_COSMETIC, true);
                creature->CastSpell(GetTarget(), SPELL_RIDE_VEHICLE_HARDCODED, true);
                creature->AddUnitTypeMask(UNIT_MASK_ACCESSORY);
            }
        }

        void Register()
        {
            AfterEffectApply += AuraEffectApplyFn(spell_woe_shadow_walk_AuraScript::OnApply, EFFECT_0, SPELL_AURA_SET_VEHICLE_ID, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_woe_shadow_walk_AuraScript();
    }
};

class spell_woe_firewall_periodic : public SpellScriptLoader
{
public:
    spell_woe_firewall_periodic() : SpellScriptLoader("spell_woe_firewall_periodic") { }

    class spell_woe_firewall_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_woe_firewall_periodic_AuraScript);

        void onPeriodicTick(AuraEffect const* aurEff)
        {
            uint8 position = aurEff->GetTickNumber();
            if (Creature* target = GetTarget()->SummonCreature(56096, firewallPeriodicPositions[position]))
                target->CastSpell(target, SPELL_SUMMON_FIREWALL_PULSE, true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_woe_firewall_periodic_AuraScript::onPeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_woe_firewall_periodic_AuraScript();
    }
};

class spell_woe_wall_of_shadow_periodic : public SpellScriptLoader
{
public:
    spell_woe_wall_of_shadow_periodic() : SpellScriptLoader("spell_woe_wall_of_shadow_periodic") { }

    class spell_woe_wall_of_shadow_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_woe_wall_of_shadow_periodic_AuraScript);

        void onPeriodicTick(AuraEffect const* aurEff)
        {
            if (aurEff->GetTickNumber() > 3)
                return;

            if (Creature* visual = GetTarget()->SummonCreature(NPC_DISTRACT_STALKER, DemonStalkerPos[aurEff->GetTickNumber()], TEMPSUMMON_TIMED_DESPAWN, 15000))
                visual->CastSpell(visual, SPELL_DISTRACT_DEMONS_STATIONARY, true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_woe_wall_of_shadow_periodic_AuraScript::onPeriodicTick, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_woe_wall_of_shadow_periodic_AuraScript();
    }
};

class DelayedCrystalDeathEvent : public BasicEvent
{
public:
    DelayedCrystalDeathEvent(Unit* trigger, uint64 targetGUID) : _trigger(trigger), _targetGUID(targetGUID)
    {
    }

    bool Execute(uint64 /*time*/, uint32 /*diff*/)
    {
        if (Creature* crystal = ObjectAccessor::GetCreature(*_trigger, _targetGUID))
            _trigger->Kill(crystal);
        return true;
    }

private:
    Unit* _trigger;
    uint64 _targetGUID;
};

class spell_woe_fel_crystal_meltdown : public SpellScriptLoader
{
public:
    spell_woe_fel_crystal_meltdown() : SpellScriptLoader("spell_woe_fel_crystal_meltdown") { }

    class spell_woe_fel_crystal_meltdown_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_woe_fel_crystal_meltdown_AuraScript);

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
        {
            if (Unit* caster = GetCaster())
            {
                caster->CastSpell(GetUnitOwner(), SPELL_SHATTER_FEL_CRYSTAL, true);
                GetUnitOwner()->CastSpell(GetUnitOwner(), SPELL_FEL_CRYSTAL_DESTRUCTION, true);

                if (!caster->HasAura(SPELL_FEL_CRYSTAL_DESTROYED))
                    caster->CastSpell(caster, SPELL_FEL_CRYSTAL_DESTROYED, true);

                if (GameObject* focus = caster->FindNearestGameObject(GO_PORTAL_ENERGY_FOCUS_ONE, 5.00f))
                    focus->RemoveFromWorld();
                else if (GameObject* focus = caster->FindNearestGameObject(GO_PORTAL_ENERGY_FOCUS_TWO, 5.00f))
                    focus->RemoveFromWorld();
                else if (GameObject* focus = caster->FindNearestGameObject(GO_PORTAL_ENERGY_FOCUS_THREE, 5.00f))
                    focus->RemoveFromWorld();

                if (GetTarget() != caster)
                    caster->m_Events.AddEvent(new DelayedCrystalDeathEvent(caster, GetTarget()->GetGUID()), caster->m_Events.CalculateTime(500));
            }
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_woe_fel_crystal_meltdown_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_woe_fel_crystal_meltdown_AuraScript();
    }
};

class spell_woe_sheen_of_elune : public SpellScriptLoader
{
public:
    spell_woe_sheen_of_elune() : SpellScriptLoader("spell_woe_sheen_of_elune") { }

    class spell_woe_sheen_of_elune_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_woe_sheen_of_elune_AuraScript);

        void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            if (eventInfo.GetDamageInfo() && eventInfo.GetDamageInfo()->GetDamage() > 0)
            {
                int32 damage = CalculatePct(eventInfo.GetDamageInfo()->GetDamage(), 60);
                eventInfo.GetActionTarget()->CastCustomSpell(eventInfo.GetActor(), 102258, &damage, NULL, NULL, TRIGGERED_FULL_MASK);
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_woe_sheen_of_elune_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_woe_sheen_of_elune_AuraScript();
    }
};

class spell_woe_shadowcloak_aggro_check_stealth : public SpellScriptLoader
{
public:
    spell_woe_shadowcloak_aggro_check_stealth() : SpellScriptLoader("spell_woe_shadowcloak_aggro_check_stealth") { }

    class spell_woe_shadowcloak_aggro_check_stealth_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_woe_shadowcloak_aggro_check_stealth_SpellScript);

        void CastDemonicPull(SpellEffIndex effIndex)
        {
            GetCaster()->RemoveAurasDueToSpell(SPELL_PLAYER_SHADOW_WALK);
            GetCaster()->RemoveAurasDueToSpell(SPELL_PLAYER_SHADOW_WALK_VISUAL);
            GetCaster()->RemoveAurasDueToSpell(SPELL_PLAYER_SHADOW_AMBUSHER_STEALTH);
            GetCaster()->CastSpell(GetCaster(), SPELL_DEMONIC_GRIP_ROOT, true);
            if (Creature* guardian = GetCaster()->FindNearestCreature(NPC_GUARDIAN_DEMON, 100.00f))
            {
                if (guardian->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT) == 0)
                {
                    guardian->SetInCombatWith(GetCaster());
                    guardian->CastSpell(GetCaster(), SPELL_DEMONIC_GRIP, false);
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_woe_shadowcloak_aggro_check_stealth_SpellScript::CastDemonicPull, EFFECT_0, SPELL_EFFECT_FORCE_CAST);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_woe_shadowcloak_aggro_check_stealth_SpellScript();
    }
};

class spell_woe_demon_grip_pull : public SpellScriptLoader
{
public:
    spell_woe_demon_grip_pull() : SpellScriptLoader("spell_woe_demon_grip_pull") { }

    class spell_woe_demon_grip_pull_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_woe_demon_grip_pull_SpellScript);

        void ModDestHeight(SpellEffIndex /*effIndex*/)
        {
            if (Creature* guardian = GetCaster()->FindNearestCreature(NPC_GUARDIAN_DEMON, 100.00f))
            {
                Position pos;
                guardian->GetNearPosition(pos, 5.0f, 0.0f);
                GetHitDest()->Relocate(pos);
            }
        }

        void Register() override
        {
            OnEffectLaunch += SpellEffectFn(spell_woe_demon_grip_pull_SpellScript::ModDestHeight, EFFECT_0, SPELL_EFFECT_JUMP_DEST);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_woe_demon_grip_pull_SpellScript();
    }
};

class spell_woe_demon_grip_periodic : public SpellScriptLoader
{
public:
    spell_woe_demon_grip_periodic() : SpellScriptLoader("spell_woe_demon_grip_periodic") { }

    class spell_woe_demon_grip_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_woe_demon_grip_periodic_AuraScript);

        void onPeriodicTick(AuraEffect const* aurEff)
        {
            PreventDefaultAction();
            if (Unit* caster = GetCaster())
                if (Unit* target = ObjectAccessor::FindUnit(caster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT)))
                    caster->CastSpell(target, GetAura()->GetSpellInfo()->Effects[EFFECT_2].TriggerSpell, true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_woe_demon_grip_periodic_AuraScript::onPeriodicTick, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_woe_demon_grip_periodic_AuraScript();
    }
};

class achievement_woe_lazy_eye : public AchievementCriteriaScript
{
public:
    achievement_woe_lazy_eye() : AchievementCriteriaScript("achievement_woe_lazy_eye") { }

    bool OnCheck(Player* /*player*/, Unit* target)
    {
        return target && target->GetAI()->GetData(DATA_LAZY_EYE_ACHIEVEMENT) == 1;
    }
};

void AddSC_boss_perotharn()
{
    new boss_woe_perotharn();
    new npc_woe_legion_demon();
    new npc_woe_illidan_part_one();
    new npc_woe_nozdormu_part_one();
    new npc_woe_legion_demon_firewall();
    new npc_woe_dreadlord_defender();
    new npc_woe_corrupted_arcanist();
    new npc_woe_guardian_demon();
    new go_woe_portal_energy_focus();
    new go_woe_time_transit_device();
    new spell_woe_fel_decay_heal();
    new spell_woe_drain_essence();
    new spell_woe_summon_circle_spawn();
    new spell_woe_punishing_flames();
    new spell_woe_fel_fireball_aggro();
    new spell_woe_tracked_lock_on_player();
    new spell_woe_drain_essence_damage();
    new spell_woe_drain_essence_knockback();
    new spell_woe_shadowcloak_periodic();
    new spell_woe_shadow_walk();
    new spell_woe_firewall_periodic();
    new spell_woe_wall_of_shadow_periodic();
    new spell_woe_fel_crystal_meltdown();
    new spell_woe_sheen_of_elune();
    new spell_woe_shadowcloak_aggro_check_stealth();
    new spell_woe_demon_grip_pull();
    new spell_woe_demon_grip_periodic();
    new achievement_woe_lazy_eye();
}