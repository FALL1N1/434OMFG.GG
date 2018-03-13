/*
 * Copyright (C) 2013-2014 OMFG.GG Network <http://www.omfg.gg/>
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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "ScriptedEscortAI.h"
#include "hour_of_twilight.h"

enum Yells
{
    TALK_THRALL_BEWARE_ENEMIES  = -53553,
    TALK_THRALL_LET_NONE_STAND  = -53821,
    TALK_THRALL_LET_THEM_COME   = -53823,
    TALK_THRALL_ALEXSTRASZA     = -53857,
    TALK_THRALL_UP_THERE        = -53875,
    TALK_THRALL_AN_ASSASSIN     = -53913,
    TALK_THRALL_I_HAVENT_TO_BE  = -53968,
    TALK_THRALL_WELL_DONE       = -53986,
    TALK_THRALL_THE_REST_OF_THE = -53987,

    TALK_ASIRA_WHERE_DO_YOU     = -53825,
    TALK_ASIRA_AND_WITH_THAT    = -53751,
    TALK_ASIRA_AGGRO            = -53268,
    TALK_ASIRA_SMOKE_BOMB       = -54826,
    TALK_ASIRA_SMOKE_BOMB_2     = -54828,
    TALK_ASIRA_SMOKE_BOMB_3     = -54827,
    TALK_ASIRA_KILL_PLAYER      = -54822,
    TALK_ASIRA_KILL_PLAYER_2    = -54823,
    TALK_ASIRA_KILL_PLAYER_3    = -54824,
    TALK_ASIRA_KILL_PLAYER_4    = -54825,
    TALK_ASIRA_GROUP_WIPE       = -54820,
    TALK_ASIRA_DEATH            = -54821,
};

enum Spells
{
    SPELL_MOUNT                      = 103646,
    SPELL_ESTROPIER                  = 103655,
    SPELL_DISMOUNT                   = 103720,
    SPELL_ATTACK_ME                  = 110289,
    SPELL_MARK_OF_SILENCE            = 102726,
    SPELL_SILENCED                   = 103587,
    SPELL_LAUNCH_KNEEL               = 103597,
    SPELL_CHOKING_SMOKE_BOMB         = 103558,
    SPELL_CHOKING_SMOKE_BOMB_2       = 103790,
    SPELL_BLADE_BARRIER              = 103419,
    SPELL_LESSER_BLADE_BARRIER       = 103562,
    SPELL_ASIRA_DRAKE_FORCE_DISMOUNT = 103719,
    SPELL_RESURECT_ASIRA_DRAKE       = 103947,
    // 54972
    SPELL_AURA_GARDETERRE            = 107455,
    SPELL_LAVA_EXPLOSION             = 107980,
    SPELL_SUMMON_TOTEM               = 108374, //103819,
    // NPC_TWILIGHT_ASSASINS
    SPELL_DISAPEAR                   = 102921,
    SPELL_FORCE_ATTACK               = 110289,
    SPELL_GARROT                     = 102925,

    SPELL_FIRE_TOTEM_AURA            = 103813,
    SPELL_FEIGN_DEATH                = 29266,
    SPELL_RIDE_DRAKE_VEHICLE         = 103989
};

enum eNpcs
{
    NPC_ASIRA_LIFE_WARDEN           = 55415,
    NPC_ASIRA_PLAYER_LIFE_WARDEN    = 55549,
    NPC_TWILIGHT_ASSASINS           = 55106,
    NPC_TWILIGHT_THUG               = 55111,
    NPC_TWILIGHT_BRUISER            = 55112,
    NPC_TWILIGHT_RANGER             = 55107,
    NPC_TWILIGHT_SHADOW_WALKER      = 55109,
    NPC_THRALL_FIRE_TOTEM           = 55474,
};

enum Events
{
    EVENT_LAVA_BURST = 1,
    EVENT_RESET_ENCOUNTER,
    EVENT_SUMMON_TOTEM,
    EVENT_SUMMON_WAVE,
    EVENT_MARK_OF_SILENCE,
    EVENT_BARRIER,
    EVENT_CHOKING_SMOKE_BOMB,
    EVENT_START_INTRO,
    EVENT_FIRST_STEP,
    EVENT_KILL_DRAKE,
    EVENT_OUTRO,
    EVENT_OUTRO_2,
    EVENT_OUTRO_3,
    EVENT_START_DRAKE_PATH,
    EVENT_COMBAT_START
};

struct SummonInfo
{
    uint32 entry;
    Position position;
};

SummonInfo const wavesInfos[21] =
{
    { NPC_TWILIGHT_ASSASINS,      { 4382.02f, 418.05f, 10.75f, 0.86f } },
    { NPC_TWILIGHT_THUG,          { 4294.0f,  387.0f,  -8.0f,  0.42f } },
    { NPC_TWILIGHT_RANGER,        { 4292.26f, 380.74f, -8.0f,  0.42f } },
    { NPC_TWILIGHT_SHADOW_WALKER, { 4288.0f,  386.0f,  -8.0f,  0.42f } },
    { NPC_TWILIGHT_BRUISER,       { 4288.0f,  392.0f,  -8.0f,  0.42f } },
    { NPC_TWILIGHT_RANGER,        { 4294.0f,  464.0f,  -6.0f,  0.65f } },
    { NPC_TWILIGHT_RANGER,        { 4293.0f,  465.0f,  -6.0f,  0.65f } },
    { NPC_TWILIGHT_BRUISER,       { 4292.0f,  469.0f,  -6.0f,  0.65f } },
    { NPC_TWILIGHT_SHADOW_WALKER, { 4292.0f,  465.0f,  -6.0f,  0.65f } },
    { NPC_TWILIGHT_THUG,          { 4293.0f,  467.0f,  -6.0f,  0.65f } },
    { NPC_TWILIGHT_BRUISER,       { 4349.0f,  580.0f,   7.9f,  4.29f } },
    { NPC_TWILIGHT_BRUISER,       { 4346.0f,  582.9f,   7.8f,  4.29f } },
    { NPC_TWILIGHT_RANGER,        { 4346.78f, 585.41f,  9.36f, 4.29f } },
    { NPC_TWILIGHT_SHADOW_WALKER, { 4349.0f,  585.94f, 10.22f, 4.29f } },
    { NPC_TWILIGHT_THUG,          { 4343.1f,  580.17f,  5.45f, 4.29f } },
    { NPC_TWILIGHT_ASSASINS,      { 4349.0f,  585.94f, 10.22f, 4.29f } },
    { NPC_TWILIGHT_BRUISER,       { 4270.9f,  569.0f,  -6.0f,  6.15f } },
    { NPC_TWILIGHT_BRUISER,       { 4271.24f, 560.31f, -6.0f,  6.15f } },
    { NPC_TWILIGHT_RANGER,        { 4267.13f, 563.98f, -6.0f,  6.15f } },
    { NPC_TWILIGHT_SHADOW_WALKER, { 4267.76f, 567.72f, -6.0f,  6.15f } },
    { NPC_TWILIGHT_THUG,          { 4271.90f, 566.06f, -6.0f,  6.15f } }
};

uint32 const waveSummonsCount[5] = { 1, 4, 5, 6, 5 };

Position const drakesPos[5][2] =
{
    { {4215.0f, 591.12f, 73.88f, 0.0f}, { 4253.95f, 586.52f, -6.21f } },
    { {4215.0f, 579.0f,  73.88f, 0.0f }, { 4253.95f, 576.52f, -6.21f } },
    { {4215.0f, 569.0f,  73.88f, 0.0f }, { 4253.95f, 566.52f, -6.21f } },
    { {4215.0f, 559.0f,  73.88f, 0.0f }, { 4253.95f, 556.52f, -6.21f } },
    { {4215.0f, 549.0f,  73.88f, 0.0f }, { 4253.95f, 546.52f, -6.21f } },
};

uint32 const lifeWardenOutroPathSize = 12;
const G3D::Vector3 lifeWardenOutroPath[lifeWardenOutroPathSize] =
{
    { 4237.99f, 587.386f, 67.0783f },
    { 4188.34f, 587.700f, 88.7852f },
    { 4142.70f, 576.924f, 90.8184f },
    { 4086.67f, 556.100f, 93.0304f },
    { 4062.53f, 533.963f, 94.4381f },
    { 4043.45f, 502.779f, 105.907f },
    { 4031.09f, 480.383f, 103.570f },
    { 4004.91f, 453.611f, 98.6840f },
    { 3968.75f, 402.037f, 91.2890f },
    { 3956.42f, 379.809f, 79.5980f },
    { 3936.81f, 329.893f, 48.0870f },
    { 3938.30f, 300.145f, 22.1750f }
};

uint32 const lifeWardenPathSize = 10;
const G3D::Vector3 lifeWardenPath[lifeWardenPathSize] =
{
    { 4138.709f, 590.6111f, 62.38307f },
    { 4139.312f, 589.8125f, 62.38307f },
    { 4143.344f, 584.3108f, 66.63746f },
    { 4172.907f, 545.8472f, 77.70271f },
    { 4269.030f, 494.8038f, 17.48905f },
    { 4329.573f, 498.6510f, 1.070258f },
    { 4331.688f, 561.2899f, 3.610033f },
    { 4319.734f, 582.5972f, 2.033872f },
    { 4309.477f, 595.6875f, -0.78888f },
    { 4286.967f, 590.8906f, -6.55511f }
};

const G3D::Vector3 lifeWardenPath2[lifeWardenPathSize] =
{
    { 4131.524f, 583.6799f, 62.80218f },
    { 4132.154f, 582.9028f, 62.80218f },
    { 4137.388f, 579.0955f, 68.66771f },
    { 4168.403f, 546.2274f, 70.96072f },
    { 4249.799f, 493.8733f, 23.46905f },
    { 4318.497f, 507.4931f, 2.754524f },
    { 4315.221f, 567.8333f, 2.375407f },
    { 4307.860f, 580.9514f, 0.311499f },
    { 4295.418f, 576.717f, -3.834398f },
    { 4295.418f, 576.717f, -6.555110f }
};

const G3D::Vector3 lifeWardenPath3[lifeWardenPathSize] =
{
    { 4108.733f, 548.5226f, 60.69159f },
    { 4109.733f, 548.5226f, 60.69159f },
    { 4119.816f, 547.2448f, 64.63828f },
    { 4154.833f, 541.0938f, 67.62702f },
    { 4223.546f, 527.3906f, 25.25583f },
    { 4280.589f, 506.5278f, 14.83808f },
    { 4321.616f, 524.4323f, 8.365682f },
    { 4322.548f, 560.8351f, -2.62199f },
    { 4300.106f, 560.9479f, -4.13131f },
    { 4300.106f, 560.9479f, -6.55511f }
};

const G3D::Vector3 lifeWardenPath4[lifeWardenPathSize] =
{
    { 4122.522f, 571.9618f, 62.19633f },
    { 4123.522f, 571.9618f, 62.19633f },
    { 4131.061f, 568.5851f, 65.63412f },
    { 4161.332f, 548.3629f, 68.59656f },
    { 4239.093f, 510.1094f, 25.84231f },
    { 4317.169f, 502.8871f, -1.58852f },
    { 4321.915f, 554.0174f, -2.28311f },
    { 4313.952f, 572.0799f, -2.16401f },
    { 4298.124f, 568.6597f, -3.73903f },
    { 4298.124f, 568.6597f, -6.55511f }
};

const G3D::Vector3 lifeWardenPath5[lifeWardenPathSize] =
{
    { 4105.705f, 535.6927f, 60.03286f },
    { 4106.705f, 535.6927f, 60.03286f },
    { 4115.329f, 533.1406f, 66.64790f },
    { 4165.736f, 520.2500f, 77.71184f },
    { 4260.010f, 500.8750f, 19.59984f },
    { 4323.022f, 500.7986f, 1.156603f },
    { 4327.795f, 552.5313f, 0.560256f },
    { 4314.211f, 587.5903f, -0.36022f },
    { 4291.940f, 583.3055f, -3.63677f },
    { 4291.940f, 583.3055f, -6.55511f }
};

uint32 const lifeWardenBossPathSize = 9;
const G3D::Vector3 lifeWardenBossPath[lifeWardenBossPathSize] =
{
    { 4138.424f, 554.4774f, 66.60613f },
    { 4164.514f, 549.0555f, 68.71677f },
    { 4240.319f, 500.9618f, 24.71219f },
    { 4268.984f, 530.2552f, 19.39990f },
    { 4248.996f, 558.0364f, 19.99987f },
    { 4241.496f, 568.2986f, 19.96937f },
    { 4252.358f, 583.1268f, 16.01521f },
    { 4273.138f, 565.5035f, 8.949485f },
    { 4286.611f, 604.8941f, 11.78889f },
};

uint32 const lifeWardenBossOutroPathSize = 3;
const G3D::Vector3 lifeWardenBossOutroPath[lifeWardenBossOutroPathSize] =
{ 
    { 4206.213f, 472.6389f, 73.64722f },
    { 4138.598f, 523.4948f, 73.64722f },
    { 4054.506f, 549.8229f, 73.64722f }
};

struct DrakePathInfo
{
    uint32 entry;
    Position position;
    G3D::Vector3 const* pathName;
};

DrakePathInfo const drakeInfo[5] =
{
    { NPC_ASIRA_PLAYER_LIFE_WARDEN, { 4139.312f, 589.8125f, 62.38307f, 5.344859f }, lifeWardenPath  },
    { NPC_ASIRA_PLAYER_LIFE_WARDEN, { 4132.154f, 582.9028f, 62.80218f, 5.654335f }, lifeWardenPath2 },
    { NPC_ASIRA_PLAYER_LIFE_WARDEN, { 4109.733f, 548.5226f, 60.69159f, 6.157132f }, lifeWardenPath3 },
    { NPC_ASIRA_PLAYER_LIFE_WARDEN, { 4123.522f, 571.9618f, 62.19633f, 5.862059f }, lifeWardenPath4 },
    { NPC_ASIRA_PLAYER_LIFE_WARDEN, { 4106.705f, 535.6927f, 60.03286f, 5.995471f }, lifeWardenPath5 }
};

enum eMiscs
{
    ACTION_COMBAT_END   = 0,
    ACTION_START_OUTRO  = 1,
    POINT_COMBAT        = 2,
    POINT_AIR           = 3,
    POINT_OUTRO         = 4
};

class boss_asira_dawnslayer : public CreatureScript
{
public:
    boss_asira_dawnslayer() : CreatureScript("boss_asira_dawnslayer") { }

    struct boss_asira_dawnslayerAI : public BossAI
    {
        boss_asira_dawnslayerAI(Creature* creature) : BossAI(creature, DATA_ASIRA) 
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
            me->SetReactState(REACT_PASSIVE);
        }

        void Reset() override
        {
            _eventBarrier = false;
            RemoveEncounterFrame();
            _Reset();
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(TALK_ASIRA_DEATH);
            if (Creature* thrall = instance->GetCreature(DATA_THRALL_EVENT_2))
                thrall->AI()->DoAction(ACTION_STOP_COMBAT);
            RemoveEncounterFrame();
            _JustDied();
        }

        void KilledUnit(Unit* victim) override
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(RAND(TALK_ASIRA_KILL_PLAYER, TALK_ASIRA_KILL_PLAYER_2, TALK_ASIRA_KILL_PLAYER_3, TALK_ASIRA_KILL_PLAYER_4));
        }

        void DamageTaken(Unit* doneBy, uint32& damage) override
        {
            if (!_eventBarrier && me->HealthBelowPctDamaged(30, damage))
            {
                _eventBarrier = true;
                events.ScheduleEvent(EVENT_BARRIER, 0);
            }
        }

        void EnterCombat(Unit* /*who*/) override
        {
            Talk(TALK_ASIRA_AGGRO);
            DoCast(SPELL_ATTACK_ME);
            events.ScheduleEvent(EVENT_MARK_OF_SILENCE, 100);
            events.ScheduleEvent(EVENT_CHOKING_SMOKE_BOMB, 10000);
            if (Creature* thrall = instance->GetCreature(DATA_THRALL_EVENT_2))
                thrall->AI()->DoAction(ACTION_START_COMBAT);
            AddEncounterFrame();
            _EnterCombat();
        }

        void UpdateAI(uint32 const diff) override
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
                    case EVENT_BARRIER:
                        DoCast(me, SPELL_BLADE_BARRIER, true);
                        break;
                    case EVENT_MARK_OF_SILENCE:
                        DoCast(SPELL_MARK_OF_SILENCE);
                        events.ScheduleEvent(EVENT_MARK_OF_SILENCE, 20000);
                        break;
                    case EVENT_CHOKING_SMOKE_BOMB:
                        Talk(RAND(TALK_ASIRA_SMOKE_BOMB, TALK_ASIRA_SMOKE_BOMB_2, TALK_ASIRA_SMOKE_BOMB_3));
                        DoCast(me, SPELL_CHOKING_SMOKE_BOMB);
                        events.ScheduleEvent(EVENT_CHOKING_SMOKE_BOMB, 24000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private:
        bool _eventBarrier;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetHourOfTwilightAI<boss_asira_dawnslayerAI>(creature);
    }
};

class npc_asira_drake : public CreatureScript
{
public:
    npc_asira_drake() : CreatureScript("npc_asira_drake") { }

    struct npc_asira_drakeAI : public ScriptedAI
    {
        npc_asira_drakeAI(Creature* creature) : ScriptedAI(creature), instance(me->GetInstanceScript()) 
        {
            isOutroPath = false;
        }

        void Reset() override {}

        void IsSummonedBy(Unit* owner) override
        {
            events.ScheduleEvent(EVENT_START_INTRO, 1000);
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type == SPLINE_MOTION_TYPE)
            {
                switch (pointId)
                {
                    case lifeWardenBossOutroPathSize - 1:
                        if (isOutroPath)
                        {
                            if (Creature* thrall = instance->GetCreature(DATA_THRALL_EVENT_2))
                                thrall->SetVisible(false);
                            me->SetVisible(false);
                        }
                        break;
                    case 4:
                        if (Creature* asira = instance->GetCreature(DATA_ASIRA))
                            asira->AI()->Talk(TALK_ASIRA_WHERE_DO_YOU);
                        break;
                    case 8:
                        events.ScheduleEvent(EVENT_KILL_DRAKE, 500);
                        break;
                }
            }
        }

        void DoAction(const int32 action) override
        {
            switch (action)
            {
                case ACTION_START_OUTRO:
                    events.ScheduleEvent(EVENT_OUTRO, 2000);
                    break;
            }
        }

        void SpellHit(Unit* target, SpellInfo const* spellInfo) override
        {
            if (spellInfo->Id == SPELL_RESURECT_ASIRA_DRAKE)
                me->RemoveAurasDueToSpell(SPELL_FEIGN_DEATH);
        }

        void UpdateAI(uint32 const diff) override
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_START_INTRO:
                        me->GetMotionMaster()->MoveSmoothPath(lifeWardenBossPath, lifeWardenBossPathSize);
                        break;
                    case EVENT_KILL_DRAKE:
                        if (Creature* thrall = instance->GetCreature(DATA_THRALL_EVENT_2))
                        {
                            me->SetFacingToObject(thrall);
                            thrall->SetFacingToObject(me);
                            thrall->AI()->Talk(TALK_THRALL_AN_ASSASSIN);
                            thrall->AI()->TalkWithDelay(17000, TALK_THRALL_I_HAVENT_TO_BE);
                        }

                        if (Creature* asira = instance->GetCreature(DATA_ASIRA))
                        {
                            asira->AI()->TalkWithDelay(4000, TALK_ASIRA_AND_WITH_THAT);
                            asira->ExitVehicle();
                            asira->RemoveAurasDueToSpell(SPELL_RIDE_DRAKE_VEHICLE);
                        }
                        me->CastSpell(me, SPELL_FEIGN_DEATH, true);
                        me->GetMotionMaster()->MoveFall();
                        events.ScheduleEvent(EVENT_COMBAT_START, 22000);
                        break;
                    case EVENT_COMBAT_START:
                        if (Creature* asira = instance->GetCreature(DATA_ASIRA))
                        {
                            asira->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                            asira->SetReactState(REACT_AGGRESSIVE);
                        }
                        break;
                    case EVENT_OUTRO:
                        isOutroPath = true;
                        me->GetMotionMaster()->MoveSmoothPath(lifeWardenBossOutroPath, lifeWardenBossOutroPathSize);
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        EventMap events;
        InstanceScript *instance;
        bool isOutroPath;
    };
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_asira_drakeAI(creature);
    }
};

class npc_asira_player_drake : public CreatureScript
{
public:
    npc_asira_player_drake() : CreatureScript("npc_asira_player_drake") { }

    struct npc_asira_player_drakeAI : public ScriptedAI
    {
        npc_asira_player_drakeAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            isIntroPath = true;
        }

        void AttackStart(Unit* /*who*/) override {}
        void EnterCombat(Unit* /*who*/) override {}
        void EnterEvadeMode() override {}
        void Reset() override {}
        void OnCharmed(bool /*apply*/) {}

        void OnSpellClick(Unit* player, bool& result) override
        {
            int32 seat = 1;
            player->CastCustomSpell(me, SPELL_RIDE_DRAKE_VEHICLE, &seat, NULL, NULL, TRIGGERED_FULL_MASK);
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
                events.ScheduleEvent(EVENT_START_DRAKE_PATH, 1000);
        }

        void MovementInform(uint32 type, uint32 point)
        {
            if (type == SPLINE_MOTION_TYPE)
            {
                switch (point)
                {
                    case lifeWardenPathSize - 1:
                        if (isIntroPath)
                        {
                            isIntroPath = false;
                            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                            me->SetFacingTo(3.35150f);
                        }
                        break;
                    case 11:
                        if (!isIntroPath)
                        {
                            me->Kill(me);
                            me->GetMotionMaster()->MoveFall();
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_START_DRAKE_PATH:
                        me->GetMotionMaster()->MoveSmoothPath(lifeWardenOutroPath, lifeWardenOutroPathSize);
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        InstanceScript *instance;
        EventMap events;
        bool isIntroPath;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_asira_player_drakeAI(creature);
    }
};

class npc_thrall_hot_2 : public CreatureScript
{
public:
    npc_thrall_hot_2() : CreatureScript("npc_thrall_hot_2") {}

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();
        npc_thrall_hot_2AI* ai = CAST_AI(npc_thrall_hot_2::npc_thrall_hot_2AI, creature->AI());
        if (!ai)
            return false;
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                if (!ai->_isStarted)
                    ai->Start(true, true, player->GetGUID(), 0, false, false);
                else
                    ai->SetHoldState(false);
                break;
        }
        player->CLOSE_GOSSIP_MENU();
        ai->SetDespawnAtFar(false);
        ai->SetDespawnAtEnd(false);
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        ai->_isStarted = true;
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        npc_thrall_hot_2AI* ai = CAST_AI(npc_thrall_hot_2::npc_thrall_hot_2AI, creature->AI());
        if (!ai)
            return false;
        InstanceScript* instance = creature->GetInstanceScript();
        if (!instance || !instance->IsDone(DATA_ARCURION))
            return false;
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Yes Thrall, lets do this!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    struct npc_thrall_hot_2AI : public npc_escortAI
    {
        friend class npc_thrall_hot_2;

        npc_thrall_hot_2AI(Creature* creature) : npc_escortAI(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
            _sumCount = 0;
            _step = 0;
            _isStarted = false;
            _waveCount = 0;
            _wavSum = 0;
            if (instance->GetData(DATA_EVENT_2) == DONE || (instance->GetData(DATA_EVENT_1) != DONE && !instance->IsDone(DATA_ASIRA)))
                me->SetVisible(false);
        }

        void EnterEvadeMode() override
        {
            events.CancelEvent(EVENT_LAVA_BURST);
            events.CancelEvent(EVENT_SUMMON_TOTEM);
        }

        void Reset() override
        {
            events.Reset();
            _targetGUID = 0;
            _isBossStarted = false;
        }

        void EnterCombat(Unit* /*who*/) override
        {
            if (_isBossStarted)
                return;
            events.Reset();
            events.ScheduleEvent(EVENT_LAVA_BURST, 0);
            events.ScheduleEvent(EVENT_SUMMON_TOTEM, 5000);
            events.ScheduleEvent(EVENT_RESET_ENCOUNTER, 1000);
        }

        void JustDied(Unit* /*killer*/) override
        {
            ResetEncounter(false);
        }

        void SetHoldState(bool bOnHold)
        {
            SetEscortPaused(bOnHold);
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            if (summon->GetEntry() == NPC_THRALL_FIRE_TOTEM)
                return;

            if (!--_sumCount)
                SetHoldState(false);
        }

        void JustSummoned(Creature* summon) override
        {
            switch (summon->GetEntry())
            {
                case NPC_THRALL_FIRE_TOTEM:
                    summon->CastSpell(summon, SPELL_FIRE_TOTEM_AURA, true);
                    summon->SetInCombatWithZone();
                    break;
                case NPC_TWILIGHT_ASSASINS:
                    summon->CastSpell(summon, SPELL_DISAPEAR, true);
                    _sumCount++;
                    summon->SetInCombatWithZone();
                    break;
                case NPC_ASIRA_LIFE_WARDEN:
                    if (Creature* asira = me->SummonCreature(NPC_ASIRA, 4114.684f, 563.2726f, 66.26726f, 5.707227f))
                    {
                        int32 seat = 1;
                        asira->CastCustomSpell(summon, SPELL_RIDE_DRAKE_VEHICLE, &seat, NULL, NULL, TRIGGERED_FULL_MASK);
                        asira->SetReactState(REACT_DEFENSIVE);
                    }
                    break;
                case NPC_ASIRA:
                    // dont trigger default case....
                    break;
                default:
                    summon->SetInCombatWithZone();
                    _sumCount++;
                    break;
            }
            summons.Summon(summon);
        }

        void WaypointReached(uint32 waypointId) override
        {
            switch (waypointId)
            {
                case 3:
                case 24:
                case 44:
                case 62:
                case 70:
                    _step++;
                    SetHoldState(true);
                    events.ScheduleEvent(EVENT_SUMMON_WAVE, 0);
                    break;
                case 71:
                    Talk(TALK_THRALL_ALEXSTRASZA);
                    break;
                case 74:
                    _step++;
                    SetHoldState(true);
                    TalkWithDelay(3000, TALK_THRALL_UP_THERE);
                    me->SummonCreature(NPC_ASIRA_LIFE_WARDEN, 4114.684f, 563.2726f, 66.26726f, 5.707227f);
                    me->SetReactState(REACT_PASSIVE);
                    break;
                default:
                    break;
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE && id == POINT_OUTRO)
            {
                if (Creature* drake = GetArisaDrake())
                {
                    DoCast(drake, SPELL_RESURECT_ASIRA_DRAKE, false);
                    events.ScheduleEvent(EVENT_OUTRO_2, 4500);
                }
            }
            npc_escortAI::MovementInform(type, id);
        }

        void DoAction(const int32 action) override
        {
            switch (action)
            {
                case ACTION_START_COMBAT:
                {
                    me->SetReactState(REACT_AGGRESSIVE);
                    events.Reset();
                    events.ScheduleEvent(EVENT_RESET_ENCOUNTER, 1000);
                    _isBossStarted = true;
                    break;
                }
                case ACTION_STOP_COMBAT:
                    events.Reset();
                    me->CombatStop(true);
                    me->SetReactState(REACT_PASSIVE);
                    instance->SetData(DATA_EVENT_2, DONE);
                    events.ScheduleEvent(EVENT_OUTRO, 8000);
                    break;
            }
        }

        void ResetEncounter(bool force = true)
        {
            _isStarted = false;
            summons.DespawnAll();
            _sumCount = 0;
            _step = 0;
            _waveCount = 0;
            _wavSum = 0;
            if (force)
                me->Kill(me);
            me->Respawn(true);
        }

        void UpdateAI(uint32 const diff) override
        {
            npc_escortAI::UpdateAI(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_LAVA_BURST:
                        if (me->isInCombat())
                            DoCastVictim(SPELL_LAVA_EXPLOSION);
                        events.ScheduleEvent(EVENT_LAVA_BURST, 6000);
                        break;
                    case EVENT_SUMMON_TOTEM:
                        DoCast(SPELL_SUMMON_TOTEM);
                        events.ScheduleEvent(EVENT_SUMMON_TOTEM, 22000);
                        break;
                    case EVENT_RESET_ENCOUNTER:
                    {
                        bool isGroupWipe = true;
                        Map::PlayerList const& playerList = me->GetMap()->GetPlayers();
                        for (Map::PlayerList::const_iterator i = playerList.begin(); i != playerList.end(); ++i)
                            if (i->getSource()->isAlive() && !i->getSource()->isGameMaster())
                            {
                                isGroupWipe = false;
                                break;
                            }
                        if (!instance->IsDone(DATA_ASIRA))
                        {
                            if (isGroupWipe)
                                ResetEncounter();
                            else
                                events.ScheduleEvent(EVENT_RESET_ENCOUNTER, 1000);
                        }
                        break;
                    }
                    case EVENT_SUMMON_WAVE:
                    {
                        for (uint8 i = 0; i < waveSummonsCount[_waveCount]; i++)
                            me->SummonCreature(wavesInfos[_wavSum + i].entry, wavesInfos[_wavSum + i].position);
                        _wavSum += waveSummonsCount[_waveCount];

                        switch (++_waveCount)
                        {
                            case 2:
                                Talk(TALK_THRALL_BEWARE_ENEMIES);
                                break;
                            case 3:
                                Talk(TALK_THRALL_LET_NONE_STAND);
                                break;
                            case 5:
                                Talk(TALK_THRALL_LET_THEM_COME);
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    case EVENT_OUTRO:
                    {
                        Talk(TALK_THRALL_WELL_DONE);
                        me->GetMotionMaster()->MovePoint(POINT_OUTRO, 4283.675f, 598.955f, -6.745f);
                        break;
                    }
                    case EVENT_OUTRO_2:
                        if (Creature* drake = GetArisaDrake())
                        {
                            int32 seat = 1;
                            me->CastCustomSpell(drake, SPELL_RIDE_DRAKE_VEHICLE, &seat, NULL, NULL, TRIGGERED_FULL_MASK);
                            TalkWithDelay(1000, TALK_THRALL_THE_REST_OF_THE);
                            drake->AI()->DoAction(ACTION_START_OUTRO);

                            for (uint8 i = 0; i < 5; i++)
                                if (Creature* lifeWarden = me->SummonCreature(drakeInfo[i].entry, drakeInfo[i].position, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000))
                                    lifeWarden->GetMotionMaster()->MoveSmoothPath(drakeInfo[i].pathName, lifeWardenPathSize);
                        }
                        break;
                    default:
                        break;
                }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

    private:
        InstanceScript* instance;
        EventMap events;
        uint32 _sumCount, _waveCount, _wavSum, _step;
        uint64 _targetGUID;
        bool _isBossStarted, _isStarted;
        SummonList summons;
        Creature* GetArisaDrake()
        {
            for (uint64 summonGuid : summons)
                if (Creature* summon = ObjectAccessor::GetCreature(*me, summonGuid))
                    if (summon->GetEntry() == NPC_ASIRA_LIFE_WARDEN)
                        return summon;
            return NULL;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetHourOfTwilightAI<npc_thrall_hot_2AI>(creature);
    }
};

class spell_asira_blade_barrier : public SpellScriptLoader
{
public:
    spell_asira_blade_barrier() : SpellScriptLoader("spell_asira_blade_barrier") { }

    class spell_asira_blade_barrier_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_asira_blade_barrier_AuraScript);

        void Absorb(AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& /*absorbAmount*/)
        {
            uint32 damage = dmgInfo.GetDamage();
            if (int32(damage) < aurEff->GetAmount())
            {
                PreventDefaultAction();
                dmgInfo.AbsorbDamage(damage - 1);
            }
        }

        void AfterAbsorb(AuraEffect* /*aurEff*/, DamageInfo& /*dmgInfo*/, uint32& /*absorbAmount*/)
        {
            GetTarget()->CastSpell(GetTarget(), SPELL_LESSER_BLADE_BARRIER, true);
        }

        void Register() override
        {
            OnEffectAbsorb += AuraEffectAbsorbFn(spell_asira_blade_barrier_AuraScript::Absorb, EFFECT_0);

            if (m_scriptSpellId == SPELL_BLADE_BARRIER)
                AfterEffectAbsorb += AuraEffectAbsorbFn(spell_asira_blade_barrier_AuraScript::AfterAbsorb, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_asira_blade_barrier_AuraScript();
    }
};

class spell_asira_mark_of_silence : public SpellScriptLoader
{
public:
    spell_asira_mark_of_silence() : SpellScriptLoader("spell_asira_mark_of_silence") { }

    class spell_asira_mark_of_silence_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_asira_mark_of_silence_AuraScript);

        void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
        {
            PreventDefaultAction();
            if (Unit* caster = GetCaster())
            {
                caster->SetFacingToObject(GetTarget());

                std::list<Player *> players = caster->GetPlayersInRange(80.0f, true);
                players.sort(Trinity::ObjectDistanceOrderPred(caster));
                for (Player* player : players)
                {
                    if (player == GetTarget() || player->IsInBetween(caster, GetTarget(), 5.00f))
                    {
                        caster->CastSpell(player, SPELL_LAUNCH_KNEEL, true);
                        break;
                    }
                }
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_asira_mark_of_silence_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_asira_mark_of_silence_AuraScript();
    }
};

class spell_asira_launch_kneel : public SpellScriptLoader
{
public:
    spell_asira_launch_kneel() : SpellScriptLoader("spell_asira_launch_kneel") { }

    class spell_asira_launch_kneel_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_asira_launch_kneel_SpellScript);

        void HandleEffect(SpellEffIndex effIndex)
        {
            if (GetHitUnit()->HasAura(SPELL_MARK_OF_SILENCE))
                GetCaster()->CastSpell(GetHitUnit(), GetEffectValue(), true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_asira_launch_kneel_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_asira_launch_kneel_SpellScript();
    }
};

class spell_asira_choking_bomb_periodic_dummy : public SpellScriptLoader
{
public:
    spell_asira_choking_bomb_periodic_dummy() : SpellScriptLoader("spell_choking_bomb_periodic_dummy") { }

    class spell_asira_choking_bomb_periodic_dummy_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_asira_choking_bomb_periodic_dummy_AuraScript);

        bool Load() override
        {
            _pos.Relocate(GetOwner());
            return true;
        }

        void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
        {
            GetTarget()->CastSpell(_pos.GetPositionX(), _pos.GetPositionY(), _pos.GetPositionZ(), SPELL_CHOKING_SMOKE_BOMB_2, true);
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_asira_choking_bomb_periodic_dummy_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }

    private:
        Position _pos;
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_asira_choking_bomb_periodic_dummy_AuraScript();
    }
};

void AddSC_boss_asira_dawnslayer()
{
    new boss_asira_dawnslayer();
    new npc_thrall_hot_2();
    new npc_asira_drake();
    new npc_asira_player_drake();
    new spell_asira_blade_barrier();
    new spell_asira_mark_of_silence();
    new spell_asira_choking_bomb_periodic_dummy();
    new spell_asira_launch_kneel();
}
