/*
 * Copyright (C) 2013-2016 OMFG.GG Network <http://www.omfg.gg/>
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
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "ScriptedEscortAI.h"
#include "hour_of_twilight.h"

enum Talks
{
    TALK_BENEDICTUS_INTRO_1                     = -53254,
    TALK_BENEDICTUS_INTRO_2                     = -53255,
    TALK_BENEDICTUS_INTRO_3                     = -53283,
    TALK_BENEDICTUS_INTRO_4                     = -53278,
    TALK_BENEDICTUS_INTRO_5                     = -54755,
    TALK_BENEDICTUS_AGGRO                       = -56544,
    TALK_BENEDICTUS_WAVE_OF_VIRTUE              = -56542,
    RAID_BENEDICTUS_WARNING_WAVE_OF_VIRTUE      = -53877,
    TALK_BENEDICTUS_WAVE_OF_TWILIGHT            = -56543,
    RAID_BENEDICTUS_WARNING_WAVE_OF_TWILIGHT    = -53896,
    TALK_BENEDICTUS_TWILIGHT_EPIPHANY           = -56541,
    RAID_BENEDICTUS_WARNING_EPIPHANY            = -53904,
    TALK_BENEDICTUS_KILLING_A_PLAYER            = -56539,
    TALK_BENEDICTUS_DEATH                       = -56538,

    TALK_THRALL_3_WARNING_GROUP                 = -53085,

    TALK_THRALL_4_I_WILL_NOT_ARCHBISHOP         = -53275,
    TALK_THRALL_4_YOU_WERE_A_FIGUREHEAD         = -53279,
};

enum Spells
{
    // Intro - Benedictus
    SPELL_HOLY_WALL                         = 102629,
    // phase 1
    SPELL_RIGHTEOUS_SHEAR_DUMMY             = 103149, // 11:31:26.03
    SPELL_RIGHTEOUS_SHEAR                   = 103151, // 11:31:26.03 103161
    SPELL_CHATIMENT                         = 104503, // 11:31:26.031  11:31:40.453
    SPELL_PURIFYING_LIGHT_SUMMON_1          = 103584, // 11:31:32.03
    SPELL_PURIFYING_LIGHT_SUMMON_2          = 103585, // 11:31:32.03
    SPELL_PURIFYING_LIGHT_SUMMON_3          = 103586, // 11:31:32.03
    SPELL_PURIFYING_LIGHT                   = 103565, // trigger SPELL_PURIFYING_LIGHT_DUMMY
    SPELL_PURIFYING_LIGHT_DUMMY             = 103600, // trigger
    SPELL_PURIFYING_ORB_EXPLOSION           = 103648,
    SPELL_PURIFYING_WAVE_SC_EFFECT          = 103676,
    SPELL_PURIFYING_WAVE_SUMMON_1           = 103677,
    SPELL_PURIFYING_WAVE_SUMMON_2           = 103680,
    SPELL_PURIFYING_WAVE_SUMMON_3           = 103681,
    SPELL_TWILIGHT_EPITAPHY_2               = 103755,
    SPELL_TWILIGHT_EPITAPHY                 = 103754, // 11:31:51.281 periodic SPELL_TWILIGHT_EPITAPHY_2
    // phase 2
    SPELL_TRANSFORM                         = 103765, // 11:31:57.29
    SPELL_TWILIGHT_SHEAR_DUMMY              = 103362,
    SPELL_TWILIGHT_SHEAR                    = 103363, // 11:32:03.31 trig 103526
    SPELL_SHADOW_DEFLAG                     = 104504, // 2- 11:32:17.750
    SPELL_CORRUPTING_TWILIGHT_SUMMON_1      = 103770, // 11:32:08.141
    SPELL_CORRUPTING_TWILIGHT_SUMMON_2      = 103771,
    SPELL_CORRUPTING_TWILIGHT_SUMMON_3      = 103772,
    SPELL_CORRUPTING_TWILIGHT               = 103767, // trigger SPELL_CORRUPTING_TWILIGHT_DUMMY
    SPELL_CORRUPTING_TWILIGHT_DUMMY         = 103768, // trigger
    SPELL_SHADOW_ORB_EXPLOSION              = 103776,
    SPELL_TWILIGHT_WAVE_SUMMON_1            = 103782,
    SPELL_TWILIGHT_WAVE_SUMMON_2            = 103783,
    SPELL_TWILIGHT_WAVE_SUMMON_3            = 103784,
    SPELL_TWILIGHT_WAVE_SC_EFF              = 103778,

    // NPC_PURIFYING_LIGHT
    SPELL_AURA_PURIFYING_LIGHT              = 103578,
    SPELL_AURA_PURIFYING_LIGHT_GROW         = 103579,

    // NPC_TWILIGHT_WAVE
    SPELL_TWILIGHT_WAVE                     = 103780,
    // NPC_PURIFYING_WAVE
    SPELL_PURIFYING_WAVE                    = 103678,

    // NPC_CORRUPTING_TWILIGHT
    SPELL_AURA_CORRUPTING_TWILIGHT          = 103769,
    SPELL_AURA_CORRUPTING_TWILIGHT_GROW     = 103773,

    // NPC_FACELESS_SHADOW_WEAVER
    SPELL_SHADOW_SALVE                      = 102992, // 11:27:03.422 11:27:19.047
    SPELL_SHADOW_SEARCH_DUMMY               = 102983, // 11:27:08.219
    SPELL_SHADOW_SEARCH                     = 102984, // 11:27:08.219

    // NPC_FACELESS_BRUTE
    SPELL_TENTACLE_HIT                      = 102848, // 11:27:03.422   11:27:28.672
    SPELL_PESTILENCE_DUMMY                  = 91939,  // 11:27:03.422
    SPELL_DEATH_TOUCH                       = 102861, // 11:27:13.031

    // NPC_SHADOW_BORER
    SPELL_SHADOW_FORAGE_DUMMY               = 102997, // 11:26:59.813
    SPELL_SHADOW_FORAGE                     = 102995, // 11:26:59.813  11:27:12.250

    // NPC_FALLING_TENTACLE
    SPELL_START_TENTACLE                    = 102302, // hit summoner 54663, 54664  11:27:48.328
    SPELL_EJECT_ALL_PASSENGERS              = 50630,  //  11:27:50.734 at death
    SPELL_EARTHEN_RING_RIDE_VEHICLE         = 102322,

    // NPC_CORRUPTED_SLIME
    SPELL_INFECTED_BEAT                     = 102224, // aura at summon

    // NPC_BLOOD
    SPELL_SUMMON_CORRUPTED_SLIME_PERIODIC   = 102213,
    SPELL_SUMMON_CORRUPTED_SLIME_SC_EFF     = 102214,
    SPELL_SUMMON_CORRUPTED_SLIME            = 102215,

    // NPC_LIGHT_ORB_JUMP_SUMMON
    SPELL_AURA_PURIFIED                     = 103654,

    // NPC_SHADOW_ORB_JUMP_SUMMON
    SPELL_AURA_CORRUPTED                    = 103774,
    SPELL_FIRE_TOTEM_AURA                   = 103813,
    SPELL_AURA_GARDETERRE                   = 107455,
    SPELL_LAVA_EXPLOSION                    = 107980,
    SPELL_SUMMON_TOTEM                      = 108374,
    SPELL_SHADOW_FOG                        = 102287,
    SPELL_POOLED_BLOOD_VISUAL               = 102254,
    SPELL_POOLED_BLOOD_VISUAL_GROWTH        = 102255,
    SPELL_DEEP_WOUND                        = 102224,

    // Magic number fixes...
    SPELL_WATER_SHELL_SUMMON                = 103686,
    SPELL_SEAPING_LIGHT                     = 104516,
    SPELL_LAVABURST_TRIGGER                 = 108437,
    SPELL_AGGRO_PERIODIC                    = 102231,
    SPELL_WATER_SHELL                       = 103688,
    SPELL_LAVABURST                         = 108436,
    SPELL_LAVABURST_DAMAGE                  = 108442,
    SPELL_CHAIN_LIGHTNING                   = 103637,
    SPELL_SEAPING_TWILIGHT                  = 104534,
};

enum Events
{
    EVENT_INTRO_1               = 1,
    EVENT_INTRO_2               = 2,
    EVENT_INTRO_3               = 3,
    EVENT_INTRO_4               = 4,
    EVENT_INTRO_5               = 5,
    EVENT_INTRO_6               = 6,
    EVENT_INTRO_7               = 7,
    EVENT_INTRO_8               = 8,
    EVENT_INTRO_9               = 9,
    EVENT_INTRO_10              = 10,
    EVENT_INTRO_11              = 11,
    EVENT_INTRO_12              = 12,
    EVENT_INTRO_13              = 13,

    EVENT_RIGHTEOUS_SHEAR       = 14,
    EVENT_CHATIMENT             = 15,
    EVENT_PURIFYING_LIGHT       = 16,
    EVENT_PURIFYING_WAVE        = 17,

    EVENT_PURIFYING_LIGHT_JUMP  = 18,
    EVENT_LAVABURST_TRIGGER     = 19,
    EVENT_CHAIN_LIGHTNING       = 20,

    EVENT_RESET_ENCOUNTER       = 21,
    EVENT_SUMMON_WAVE           = 22,
    EVENT_STEP_1                = 23,
    EVENT_STEP_2                = 24,
    EVENT_STEP_3                = 25,
    EVENT_ACTIVATE_BLOOD        = 26,
    EVENT_TRANSFORM             = 27,
};

enum Npcs
{
    NPC_ARCH_BISHOP             = 54938,
    NPC_HOLY_WALL               = 54955,
    NPC_PURIFYING_LIGHT         = 55377,
    NPC_PURIFYING_WAVE          = 55441,
    NPC_TWILIGHT_WAVE           = 55469,
    NPC_CORRUPTING_TWILIGHT     = 55467,
    NPC_FACELESS_SHADOW_WEAVER  = 54633,
    NPC_FACELESS_BRUTE          = 54632,
    NPC_SHADOW_BORER            = 54686,
    NPC_FALLING_TENTACLE        = 54696,
    NPC_CORRUPTED_SLIME         = 54646,
    NPC_BLOOD                   = 54644,
    NPC_DARK_HAZE               = 54628,
    NPC_LIGHT_ORB_JUMP_SUMMON   = 55427,
    NPC_SHADOW_ORB_JUMP_SUMMON  = 55468,
    NPC_EARTHEN_SHELL_TARGET    = 55445,
    NPC_WATER_SHELL             = 55447
};

enum actions
{
    ACTION_START_INTRO      = 1,
    ACTION_INTRO_1          = 2,
    ACTION_INTRO_2          = 3,
    ACTION_RESPAWN          = 4,
    ACTION_WATER_SHELL      = 5,
    ACTION_CHAIN_LIGHTNING  = 6,
    ACTION_COMBAT_END       = 7,
};

enum misc
{
    DATA_RESTART_EVENT      = 5,
    QUEST_CREDIT_BENEDICTUS = 57854,
    QUEST_THE_TWILIGHT_PROPHET = 30105
};

enum Phases
{
    PHASE_ALL    = 0,
    PHASE_ONE    = 1,
    PHASE_TWO    = 2
};

struct IntroElements
{
    Position point;
    uint32 timer;
};

IntroElements point_intro[9] =
{
    { {3627.092f, 281.5938f, -120.1445f, 0.0f}, 2500},
    { {3654.418f, 283.1163f, -120.1744f, 0.0f}, 3000},
    { {3673.196f, 284.5243f, -118.3202f, 0.0f}, 2000},
    { {3701.124f, 286.7031f, -104.9859f, 0.0f}, 2000},
    { {3724.686f, 289.0677f, -92.44993f, 0.0f}, 5000},
    { {3655.690f, 282.5538f, -120.1756f, 0.0f}, 8000},
    { {3639.618f, 281.7153f, -120.1446f, 0.0f}, 4000},
    { {3596.269f, 278.0347f, -120.1531f, 0.0f}, 5000},
    { {3547.737f, 272.7934f, -115.9649f, 0.0f}, 7000}
};

Position TentaclesSpawnPositions[]  = 
{
    {3901.132f, 281.1990f, 4.156386f, 0.0f},
    {3897.162f, 276.6766f, 3.435897f, 0.0f},
    {3772.489f, 305.3239f, -61.9611f, 0.0f},
    {3768.179f, 284.6392f, -59.8089f, 0.0f}
};

struct SummonInfo
{
    uint32 entry;
    Position position;
};

SummonInfo const wavesInfos[12] =
{
    { NPC_FACELESS_SHADOW_WEAVER,   { 3901.363f, 251.4531f, 6.676357f, 1.37881f } },
    { NPC_FACELESS_BRUTE,           { 3906.705f, 252.4688f, 8.297945f, 1.37881f } },
    { NPC_SHADOW_BORER,             { 3908.728f, 246.7587f, 10.28710f, 1.37881f } },
    { NPC_DARK_HAZE,                { 3904.888f, 250.9427f, 8.095772f, 1.08210f } },

    { NPC_FACELESS_SHADOW_WEAVER,   { 3808.175f, 323.7656f, -36.16254f, 5.550147f } },
    { NPC_FACELESS_BRUTE,           { 3813.517f, 324.7813f, -32.36459f, 5.462881f } },
    { NPC_SHADOW_BORER,             { 3815.539f, 319.0712f, -33.62153f, 5.515240f } },
    { NPC_DARK_HAZE,                { 3814.132f, 322.7674f, -32.83337f, 0.000000f } },

    { NPC_FACELESS_SHADOW_WEAVER,   { 3721.689f, 265.5486f, -97.21669f, 0.6457718f } },
    { NPC_FACELESS_BRUTE,           { 3727.031f, 266.5642f, -92.36553f, 0.7330383f } },
    { NPC_SHADOW_BORER,             { 3729.054f, 260.8542f, -87.17882f, 0.4712389f } },
    { NPC_DARK_HAZE,                { 3729.343f, 264.7431f, -89.10324f, 0.5410520f } },

};

class boss_archbishop_benedictus : public CreatureScript
{
public:
    boss_archbishop_benedictus() : CreatureScript("boss_archbishop_benedictus") { }

    struct boss_archbishop_benedictusAI : public BossAI
    {
        boss_archbishop_benedictusAI(Creature* creature) : BossAI(creature, DATA_ARCHBISHOP)
        {
            _introDone = false;
        }

        void SetSparksVisible(bool isVisibleNow)
        {
            std::list<Creature*> creatureList;
            GetCreatureListWithEntryInGrid(creatureList, me, NPC_TWILIGHT_SPARK, 300.0f);
            for (Creature* spark : creatureList)
            {
                if (!spark->isAlive())
                    spark->Respawn();
                spark->SetVisible(isVisibleNow);
            }
        }

        void Reset() override
        {
            me->setFaction(me->GetCreatureTemplate()->faction_A);
            me->SetReactState(REACT_PASSIVE);
            me->SetWalk(false);
            _Reset();
            summonCount = 0;
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TWILIGHT_EPITAPHY_2);
        }

        void MoveInLineOfSight(Unit* unit) override
        {
            if (!_introDone)
                return;

            if (instance->GetData(DATA_EVENT_3) != DONE || unit->GetTypeId() != TYPEID_PLAYER || unit->GetDistance(me) < 10.0f || me->GetReactState() == REACT_AGGRESSIVE)
                return;
            me->setFaction(me->GetCreatureTemplate()->faction_H);
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(TALK_BENEDICTUS_DEATH);
            RemoveEncounterFrame();
            SetSparksVisible(false);
            if (Creature* thrall = instance->GetCreature(DATA_THRALL))
                thrall->AI()->DoAction(ACTION_COMBAT_END);

            Map::PlayerList const& playerList = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator i = playerList.begin(); i != playerList.end(); ++i)
                if (Player* player = i->getSource())
                    if (player->GetQuestStatus(QUEST_THE_TWILIGHT_PROPHET) == QUEST_STATUS_INCOMPLETE)
                        player->KilledMonsterCredit(QUEST_CREDIT_BENEDICTUS);

            me->RemoveAllAuras();
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TWILIGHT_EPITAPHY_2);
            _JustDied();
        }

        void KilledUnit(Unit* victim) override
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(TALK_BENEDICTUS_KILLING_A_PLAYER);
        }

        void EnterCombat(Unit* who) override
        {
            Talk(TALK_BENEDICTUS_AGGRO);
            events.SetPhase(PHASE_ONE);
            events.ScheduleEvent(EVENT_RIGHTEOUS_SHEAR, 9000);
            events.ScheduleEvent(EVENT_PURIFYING_WAVE, 30000);
            events.ScheduleEvent(EVENT_PURIFYING_LIGHT, 11000);
            events.ScheduleEvent(EVENT_CHATIMENT, 6000);

            Map::PlayerList const& playerList = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator i = playerList.begin(); i != playerList.end(); ++i)
            {
                if (Player* player = i->getSource())
                {
                    player->ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, me->GetEntry(), 0, true);
                    player->ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, ACHIEVEMENT_CRITERIA_CONDITION_NO_SPELL_HIT, 110260, true);
                }
            }

            if (Creature* shell = me->FindNearestCreature(NPC_EARTHEN_SHELL_TARGET, 200.0f, true))
                shell->CastSpell(shell, SPELL_SEAPING_LIGHT, true);

            if (Creature* thrall = instance->GetCreature(DATA_THRALL))
            {
                thrall->SetInCombatWith(me);
                thrall->SetReactState(REACT_DEFENSIVE);
            }

            AddEncounterFrame();
            _EnterCombat();
        }

        void ExecuteIntroEvent(uint32 eventId)
        {
            me->GetMotionMaster()->MovePoint(eventId, point_intro[eventId].point);
        }

        void SummonedMovementInform(Creature* summon, uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE)
            {
                if (id == 1 && (summon->GetEntry() == NPC_PURIFYING_WAVE || summon->GetEntry() == NPC_TWILIGHT_WAVE))
                    summon->DespawnOrUnsummon();
                else
                    if (id == EVENT_JUMP)
                        summon->DespawnOrUnsummon(500);
            }
        }

        void MovementInform(uint32 type, uint32 point) override
        {
            if (type == POINT_MOTION_TYPE)
            {
                switch (point)
                {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        events.ScheduleEvent(point + 1, 1);
                        break;
                    case 8:
                        events.ScheduleEvent(point + 1, 500);
                        break;
                }
            }
        }

        void SheduleIntroEvents(uint32 const diff)
        {
            if (_introDone)
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_INTRO_5:
                        Talk(TALK_BENEDICTUS_INTRO_1);

                        if (Creature* thrall = instance->GetCreature(DATA_THRALL_EVENT_3))
                            thrall->AI()->SetData(DATA_RESTART_EVENT, 1);

                        events.ScheduleEvent(eventId+1, 6000);
                        break;
                    case EVENT_INTRO_7:
                        if (Creature* holyWall = me->FindNearestCreature(NPC_HOLY_WALL, 50.0f, true))
                            me->SetFacingToObject(holyWall);

                        DoCast(SPELL_HOLY_WALL);
                        events.ScheduleEvent(eventId + 1, 5000);
                        break;
                    case EVENT_INTRO_9:
                        if (Creature* thrall = instance->GetCreature(DATA_THRALL))
                            me->SetFacingToObject(thrall);

                        Talk(TALK_BENEDICTUS_INTRO_2);
                        if (Creature* thrall = instance->GetCreature(DATA_THRALL))
                            thrall->AI()->DoAction(ACTION_INTRO_1);
                        break;
                    case EVENT_INTRO_10:
                        Talk(TALK_BENEDICTUS_INTRO_3);
                        if (Creature* thrall = instance->GetCreature(DATA_THRALL))
                            thrall->AI()->DoAction(ACTION_INTRO_2);
                        break;
                    case EVENT_INTRO_11:
                        Talk(TALK_BENEDICTUS_INTRO_4);
                        events.ScheduleEvent(EVENT_INTRO_12, 10000);
                        break;
                    case EVENT_INTRO_12:
                        Talk(TALK_BENEDICTUS_INTRO_5);
                        events.ScheduleEvent(EVENT_INTRO_13, 9000);
                        break;
                    case EVENT_INTRO_13:
                        me->setFaction(me->GetCreatureTemplate()->faction_H);
                        me->SetReactState(REACT_AGGRESSIVE);
                        _introDone = true;
                        break;
                    default:
                        ExecuteIntroEvent(eventId);
                        break;
                }
            }
        }

        void DoAction(int32 const actionId) override
        {
            switch (actionId)
            {
                case ACTION_START_INTRO:
                    me->SetVisible(true);
                    events.ScheduleEvent(EVENT_INTRO_1, 1000);
                    me->setFaction(me->GetCreatureTemplate()->faction_A);
                    me->SetReactState(REACT_PASSIVE);
                    break;
                case ACTION_INTRO_1:
                    events.ScheduleEvent(EVENT_INTRO_10, 8000);
                    break;
                case ACTION_INTRO_2:
                    events.ScheduleEvent(EVENT_INTRO_11, 8000);
                    break;
                case ACTION_RESPAWN:
                    RemoveEncounterFrame();
                    SetSparksVisible(false);
                    _DespawnAtEvade();
                    break;
            }
        }

        void JustSummoned(Creature* summon) override
        {
            switch (summon->GetEntry())
            {
                case NPC_PURIFYING_LIGHT:
                case NPC_CORRUPTING_TWILIGHT:
                {
                    uint8 seatId = 0;
                    summonCount++;
                    switch (summonCount)
                    {
                        case 1:
                            seatId = 0;
                            break;
                        case 2:
                            seatId = 3;
                            break;
                        case 3:
                            seatId = 4;
                            summonCount = 0;
                            break;
                    }

                    bool IsLight = summon->GetEntry() == NPC_PURIFYING_LIGHT;

                    summon->CastSpell(summon, IsLight ? SPELL_AURA_PURIFYING_LIGHT : SPELL_AURA_CORRUPTING_TWILIGHT, true);
                    summon->CastSpell(summon, IsLight ? SPELL_AURA_PURIFYING_LIGHT_GROW : SPELL_AURA_CORRUPTING_TWILIGHT_GROW, true);

                    summon->EnterVehicle(me, seatId);
                    break;
                }
                case NPC_TWILIGHT_WAVE:
                {
                    summon->SetReactState(REACT_PASSIVE);
                    summon->CastSpell(summon, SPELL_TWILIGHT_WAVE, true);
                    Position pos(*summon);
                    summon->GetNearPosition(pos, 140.00f, 0.00f);
                    summon->GetMotionMaster()->MovePoint(1, pos, false);
                    break;
                }
                case NPC_PURIFYING_WAVE:
                {
                    summon->SetReactState(REACT_PASSIVE);
                    summon->CastSpell(summon, SPELL_PURIFYING_WAVE, true);
                    Position pos(*summon);
                    summon->GetNearPosition(pos, 140.00f, 0.00f);
                    summon->GetMotionMaster()->MovePoint(1, pos, false);
                    break;
                }
                case NPC_LIGHT_ORB_JUMP_SUMMON:
                    summon->CastSpell(summon, SPELL_AURA_PURIFIED, true);
                    break;
                case NPC_SHADOW_ORB_JUMP_SUMMON:
                    summon->CastSpell(summon, SPELL_AURA_CORRUPTED, true);
                    break;
            }
        }

        void DamageTaken(Unit* attacker, uint32& damage) override
        {
            if (me->GetReactState() == REACT_PASSIVE)
                me->SetReactState(REACT_AGGRESSIVE);

            if (me->HasUnitState(UNIT_STATE_CASTING) || me->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT) != 0)
                return;

            if (me->HealthBelowPctDamaged(61, damage) && !events.IsInPhase(PHASE_TWO))
            {
                events.SetPhase(PHASE_TWO);
                Talk(TALK_BENEDICTUS_TWILIGHT_EPIPHANY);
                Talk(RAID_BENEDICTUS_WARNING_EPIPHANY, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                if (Creature* shell = me->FindNearestCreature(NPC_EARTHEN_SHELL_TARGET, 200.0f, true))
                {
                    shell->RemoveAura(SPELL_SEAPING_LIGHT);
                    shell->CastSpell(shell, SPELL_SEAPING_TWILIGHT, true);
                }
                // old but good working workaround... prevent kite exploit to interrupt the next channeled spell...
                me->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                DoCast(me, SPELL_TWILIGHT_EPITAPHY, true);
                events.ScheduleEvent(EVENT_TRANSFORM, 5000);
            }
        }

        void UpdateAI(uint32 const diff) override
        {
            if (!UpdateVictim())
            {
                SheduleIntroEvents(diff);
                return;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_RIGHTEOUS_SHEAR:
                        DoCast(events.IsInPhase(PHASE_TWO) ? SPELL_TWILIGHT_SHEAR_DUMMY : SPELL_RIGHTEOUS_SHEAR_DUMMY);
                        events.ScheduleEvent(EVENT_RIGHTEOUS_SHEAR, 47000);
                        break;
                    case EVENT_CHATIMENT:
                        DoCast(events.IsInPhase(PHASE_TWO) ? SPELL_SHADOW_DEFLAG : SPELL_CHATIMENT);
                        events.ScheduleEvent(EVENT_CHATIMENT, 13000);
                        break;
                    case EVENT_PURIFYING_LIGHT:
                    {
                        if (events.IsInPhase(PHASE_ONE))
                        {
                            if (Creature* thrall = instance->GetCreature(DATA_THRALL))
                            {
                                thrall->AI()->DoAction(ACTION_CHAIN_LIGHTNING);
                            }
                        }

                        Position pos(*me);
                        float x = pos.GetPositionX();
                        float y = pos.GetPositionY();
                        float z = pos.GetPositionZ();

                        me->CastSpell(x, y,       z, events.IsInPhase(PHASE_TWO) ? SPELL_CORRUPTING_TWILIGHT_SUMMON_1 : SPELL_PURIFYING_LIGHT_SUMMON_1, true);
                        me->CastSpell(x, y - 1.f, z, events.IsInPhase(PHASE_TWO) ? SPELL_CORRUPTING_TWILIGHT_SUMMON_2 : SPELL_PURIFYING_LIGHT_SUMMON_2, true);
                        me->CastSpell(x, y + 1.f, z, events.IsInPhase(PHASE_TWO) ? SPELL_CORRUPTING_TWILIGHT_SUMMON_3 : SPELL_PURIFYING_LIGHT_SUMMON_3, true);

                        DoCast(events.IsInPhase(PHASE_TWO) ? SPELL_CORRUPTING_TWILIGHT : SPELL_PURIFYING_LIGHT);
                        events.ScheduleEvent(EVENT_PURIFYING_LIGHT_JUMP, 8000);
                        events.ScheduleEvent(EVENT_PURIFYING_LIGHT, 40000);
                        break;
                    }
                    case EVENT_PURIFYING_LIGHT_JUMP:
                        for (int i = 0; i < 5; i++)
                            if (Unit* passenger = me->GetVehicleKit()->GetPassenger(i))
                            {
                                me->GetVehicleKit()->EjectPassenger(passenger, EJECT_DIR_FRONT, 0.0f);

                                if (Unit* player = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                    passenger->CastSpell(player, events.IsInPhase(PHASE_TWO) ? SPELL_SHADOW_ORB_EXPLOSION : SPELL_PURIFYING_ORB_EXPLOSION, true);
                            }
                        break;
                    case EVENT_PURIFYING_WAVE:
                    {
                        if (events.IsInPhase(PHASE_ONE))
                        {

                            Talk(TALK_BENEDICTUS_WAVE_OF_VIRTUE);
                            Talk(RAID_BENEDICTUS_WARNING_WAVE_OF_VIRTUE, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                            if (Creature* shell = me->FindNearestCreature(NPC_EARTHEN_SHELL_TARGET, 200.0f, true))
                            {
                                shell->CastSpell(shell, SPELL_WATER_SHELL_SUMMON, true);

                                if (Creature* thrall = instance->GetCreature(DATA_THRALL))
                                {
                                    thrall->CastStop();
                                    thrall->InterruptNonMeleeSpells(false, 0, true);
                                    if (AuraEffect* lavaburst = thrall->GetAuraEffect(SPELL_LAVABURST_TRIGGER, EFFECT_0))
                                        lavaburst->SetPeriodic(false);

                                    thrall->AI()->DoAction(ACTION_WATER_SHELL);
                                }
                            }
                        }
                        else
                        {
                            Talk(TALK_BENEDICTUS_WAVE_OF_TWILIGHT);
                            Talk(RAID_BENEDICTUS_WARNING_WAVE_OF_TWILIGHT, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                        }
                        DoCast(me, events.IsInPhase(PHASE_TWO) ? SPELL_TWILIGHT_WAVE_SC_EFF : SPELL_PURIFYING_WAVE_SC_EFFECT, true);
                        DoCast(me, events.IsInPhase(PHASE_TWO) ? RAND(SPELL_TWILIGHT_WAVE_SUMMON_1, SPELL_TWILIGHT_WAVE_SUMMON_2, SPELL_TWILIGHT_WAVE_SUMMON_3)
                                                               : RAND(SPELL_PURIFYING_WAVE_SUMMON_1, SPELL_PURIFYING_WAVE_SUMMON_2, SPELL_PURIFYING_WAVE_SUMMON_3), true);
                        events.ScheduleEvent(EVENT_PURIFYING_WAVE, 40000);
                        break;
                    }
                    case EVENT_TRANSFORM:
                        DoCast(me, SPELL_TRANSFORM, true);
                        SetSparksVisible(true);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

        private:
            bool _introDone;
            uint8 summonCount;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetHourOfTwilightAI<boss_archbishop_benedictusAI>(creature);
    }
};

class npc_hot_archbishop_orb_jump_summon : public CreatureScript
{
public:
    npc_hot_archbishop_orb_jump_summon() : CreatureScript("npc_hot_archbishop_orb_jump_summon") { }

    struct npc_hot_archbishop_orb_jump_summonAI : public ScriptedAI
    {
        npc_hot_archbishop_orb_jump_summonAI(Creature* creature) : ScriptedAI(creature), instance(me->GetInstanceScript()) { }

        void IsSummonedBy(Unit* /*summoner*/) override
        {
            if (Creature* arch = instance->GetCreature(DATA_ARCHBISHOP))
                arch->AI()->JustSummoned(me);
        }

        private:
            InstanceScript *instance;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_hot_archbishop_orb_jump_summonAI(creature);
    }
};

class npc_hot_corrupted_slime : public CreatureScript
{
public:
    npc_hot_corrupted_slime() : CreatureScript("npc_hot_corrupted_slime") { }

    struct npc_hot_corrupted_slimeAI : public ScriptedAI
    {
        npc_hot_corrupted_slimeAI(Creature* creature) : ScriptedAI(creature), instance(me->GetInstanceScript()) { }

        void IsSummonedBy(Unit* /*summoner*/) override
        {
            if (Creature* thrall = instance->GetCreature(DATA_THRALL_EVENT_3))
                thrall->AI()->JustSummoned(me);
        }

        private:
            InstanceScript *instance;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_hot_corrupted_slimeAI(creature);
    }
};

class npc_thrall_hot_3 : public CreatureScript
{
public:
    npc_thrall_hot_3() : CreatureScript("npc_thrall_hot_3") {}

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        if (player && action == GOSSIP_ACTION_INFO_DEF + 1)
            creature->AI()->DoAction(ACTION_START_INTRO);

        player->CLOSE_GOSSIP_MENU();
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        InstanceScript* instance = creature->GetInstanceScript();

        if (!instance || !instance->IsDone(DATA_ASIRA))
            return false;

        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Yes Thrall, lets do this!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    struct npc_thrall_hot_3AI : public npc_escortAI
    {
        friend class npc_thrall_hot_3;

        npc_thrall_hot_3AI(Creature* creature) : npc_escortAI(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
            killCount = 0;
            tentacleCount = 0;
            _isStarted = false;
            if (instance->GetData(DATA_EVENT_2) != DONE && instance->GetData(DATA_EVENT_3) == DONE)
                me->SetVisible(false);
        }

        void ActivateWave()
        {
            if (Creature* darkHaze = me->FindNearestCreature(NPC_DARK_HAZE, 100.0f, true))
            {
                darkHaze->AI()->Talk(TALK_THRALL_3_WARNING_GROUP, NULL, CHAT_MSG_RAID_BOSS_EMOTE);
                darkHaze->RemoveAura(SPELL_SHADOW_FOG);
                darkHaze->RemoveAura(SPELL_POOLED_BLOOD_VISUAL_GROWTH);
                darkHaze->DespawnOrUnsummon(3000);
            }

            if (Creature* faceless = me->FindNearestCreature(NPC_FACELESS_SHADOW_WEAVER, 100.0f, true))
            {
                faceless->CastSpell(faceless, SPELL_SHADOW_SEARCH, true);
                faceless->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                faceless->SetInCombatWithZone();
            }

            if (Creature* borer = me->FindNearestCreature(NPC_SHADOW_BORER, 100.0f, true))
            {
                borer->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                borer->SetInCombatWithZone();
                borer->AI()->DoCastRandom(SPELL_SHADOW_FORAGE, 0.0f);
            }

            if (Creature* brute = me->FindNearestCreature(NPC_FACELESS_BRUTE, 100.0f, true))
            {
                brute->CastSpell(brute, SPELL_AGGRO_PERIODIC, true);
                brute->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                brute->SetInCombatWithZone();
            }
        }

        void ActivateRandomBlood()
        {
            std::list<Creature*> creatureList;
            GetCreatureListWithEntryInGrid(creatureList, me, NPC_BLOOD, 20.0f);
            if (!creatureList.empty())
            {
                std::list<Creature *>::iterator itr = creatureList.begin();
                std::advance(itr, rand() % creatureList.size());
                if (Creature *blood = *itr)
                    blood->CastSpell(blood, SPELL_SUMMON_CORRUPTED_SLIME_SC_EFF, true);
            }
        }

        /// @TODO: correct implement this "visual" event
        //void SummonTentacke(int i)
        //{
        //    if (i > 3)
        //        return;
        //    // toddo maybe rewrite this part seems at each spawn a friendly npc is grab here
        //    me->SummonCreature(NPC_FALLING_TENTACLE, TentaclesSpawnPositions[i], TEMPSUMMON_TIMED_DESPAWN, 15000);
        //}

        void Reset() override
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            events.Reset();
            _targetGUID = 0;
            me->SetWalk(false);
            summons.DespawnAll();
            for (uint8 i = 0; i < 12; i++)
                if (Creature* summon = me->SummonCreature(wavesInfos[i].entry, wavesInfos[i].position))
                {
                    summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_UNK_6 | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);

                    if (summon->GetEntry() == NPC_SHADOW_BORER)
                        summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_UNK_6 | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_DISABLE_MOVE);
                }
        }

        void EnterCombat(Unit* /*who*/) override {}

        void JustDied(Unit* /*killer*/) override
        {
            ResetEncounter();
        }

        void WaypointReached(uint32 point) override
        {
            switch (point)
            {
                case 1:
                    //SummonTentacke(tentacleCount++);
                    break;
                case 2:
                case 3:
                case 5:
                    //SummonTentacke(tentacleCount++);
                    SetEscortPaused(true);
                    events.ScheduleEvent(EVENT_SUMMON_WAVE, 2000);
                    break;
                case 8:
                    instance->SetData(DATA_EVENT_3, DONE);
                    me->DespawnOrUnsummon(2000);
                    break;
                default:
                    break;
            }
        }

        void SetData(uint32 type, uint32 data) override
        {
            if (type == DATA_RESTART_EVENT && data == 1)
                SetEscortPaused(false);
        }

        void JustSummoned(Creature* summon) override
        {
            switch (summon->GetEntry())
            {
                case NPC_DARK_HAZE:
                    summon->CastSpell(summon, SPELL_SHADOW_FOG, true);
                    summon->CastSpell(summon, SPELL_POOLED_BLOOD_VISUAL, true);
                    summon->CastSpell(summon, SPELL_POOLED_BLOOD_VISUAL_GROWTH, true);
                    break;
                case NPC_CORRUPTED_SLIME:
                    summon->SetInCombatWithZone();
                    summon->CastSpell(summon, SPELL_DEEP_WOUND, true);
                    summon->AI()->AttackStart(me);
                    break;
                default:
                    break;
            }
            summons.Summon(summon);
        }

        void DoAction(const int32 action) override
        {
            switch (action)
            {
                case ACTION_START_INTRO:
                    Start(true, true, 0, 0, false, false);
                    SetDespawnAtFar(false);
                    SetDespawnAtEnd(false);
                    events.ScheduleEvent(EVENT_ACTIVATE_BLOOD, 5000);
                    events.ScheduleEvent(EVENT_RESET_ENCOUNTER, 1000);
                    break;
                case ACTION_KILL_CREATURE:
                    killCount++;
                    switch (killCount)
                    {
                        case 3:
                            events.ScheduleEvent(EVENT_STEP_1, 4000);
                            break;
                        case 6:
                            events.ScheduleEvent(EVENT_STEP_2, 4000);
                            break;
                        case 9:
                            if (Creature* arch = instance->GetCreature(DATA_ARCHBISHOP))
                                arch->AI()->DoAction(ACTION_START_INTRO);
                            break;
                    }
                    break;
            }
        }

        void ResetEncounter(bool force = true)
        {
            events.Reset();
            summons.DespawnAll();
            killCount = 0;
            tentacleCount = 0;
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
                case EVENT_ACTIVATE_BLOOD:
                    ActivateRandomBlood();
                    events.ScheduleEvent(EVENT_ACTIVATE_BLOOD, 10000);
                    break;
                case EVENT_SUMMON_WAVE:
                    ActivateWave();
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

                    if (!instance->IsDone(DATA_ARCHBISHOP))
                    {
                        if (isGroupWipe)
                            ResetEncounter();
                        else
                            events.ScheduleEvent(EVENT_RESET_ENCOUNTER, 1000);
                    }
                    break;
                }
                case EVENT_STEP_1:
                    SetEscortPaused(false);
                    break;
                case EVENT_STEP_2:
                    SetEscortPaused(false);
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
        uint32 killCount;
        uint32 tentacleCount;
        uint64 _targetGUID;
        SummonList summons;
        bool _isStarted;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetHourOfTwilightAI<npc_thrall_hot_3AI>(creature);
    }
};

class npc_thrall_hot_4 : public CreatureScript
{
public:
    npc_thrall_hot_4() : CreatureScript("npc_thrall_hot_4")
    {}

    struct npc_thrall_hot_4AI : public BossAI
    {
        friend class npc_thrall_hot_4;

        npc_thrall_hot_4AI(Creature* creature) : BossAI(creature, DATA_THRALL)
        {
            instance = creature->GetInstanceScript();
        }

        void EnterEvadeMode() override {}

        void Reset() override
        {
            if (instance->GetData(DATA_EVENT_3) != DONE && !instance->IsDone(DATA_ARCHBISHOP))
                me->SetVisible(false);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            me->SetReactState(REACT_PASSIVE);

            events.Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            DoCast(me, SPELL_LAVABURST_TRIGGER, true);
            events.Reset();
            events.ScheduleEvent(EVENT_RESET_ENCOUNTER, 1000);
        }

        void JustDied(Unit* /*killer*/) override {}

        void DoAction(const int32 action) override
        {
            switch (action)
            {
                case ACTION_INTRO_1:
                    events.ScheduleEvent(EVENT_INTRO_1, 7000);
                    break;
                case ACTION_INTRO_2:
                    events.ScheduleEvent(EVENT_INTRO_2, 11000);
                    break;
                case ACTION_WATER_SHELL:
                    if (Creature* waterShell = me->FindNearestCreature(NPC_WATER_SHELL, 100.0f, true))
                        me->CastSpell(waterShell, SPELL_WATER_SHELL, true);

                    events.ScheduleEvent(EVENT_LAVABURST_TRIGGER, 15000);
                    break;
                case ACTION_CHAIN_LIGHTNING:
                    me->CastStop();
                    me->InterruptNonMeleeSpells(false, 0, true);
                    if (AuraEffect* lavaburst = me->GetAuraEffect(SPELL_LAVABURST_TRIGGER, EFFECT_0))
                    {
                        lavaburst->SetPeriodic(false);
                        events.ScheduleEvent(EVENT_LAVABURST_TRIGGER, 6000);
                    }
                    events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, 3000);
                    break;
                case ACTION_COMBAT_END:
                    events.Reset();
                    me->RemoveAllAuras();
                    me->CombatStop(true);
                    break;
            }
        }

        void ResetEncounter()
        {
            events.Reset();
            me->RemoveAllAuras();
            me->DespawnCreaturesInArea(NPC_PURIFYING_LIGHT, 200.0f);
            me->DespawnCreaturesInArea(NPC_CORRUPTING_TWILIGHT, 200.0f);
            me->DespawnCreaturesInArea(NPC_SHADOW_ORB_JUMP_SUMMON, 200.0f);
            me->DespawnCreaturesInArea(NPC_LIGHT_ORB_JUMP_SUMMON, 200.0f);
            me->ClearInCombat();

            if (Creature* shell = me->FindNearestCreature(NPC_EARTHEN_SHELL_TARGET, 200.0f, true))
                shell->RemoveAllAuras();

            if (Creature* bishop = instance->GetCreature(DATA_ARCHBISHOP))
                bishop->AI()->DoAction(ACTION_RESPAWN);

            _DespawnAtEvade();
        }

        void SpellHitTarget(Unit* victim, const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_LAVABURST && victim->GetEntry() == NPC_ARCH_BISHOP)
            {
                if (me->HasAura(SPELL_LAVABURST_TRIGGER))
                    me->CastSpell(victim, SPELL_LAVABURST_DAMAGE, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
            }
        }

        void UpdateAI(uint32 const diff) override
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_INTRO_1:
                        Talk(TALK_THRALL_4_I_WILL_NOT_ARCHBISHOP);
                        if (Creature* arch = instance->GetCreature(DATA_ARCHBISHOP))
                            arch->AI()->DoAction(ACTION_INTRO_1);
                        break;
                    case EVENT_INTRO_2:
                        Talk(TALK_THRALL_4_YOU_WERE_A_FIGUREHEAD);
                        if (Creature* arch = instance->GetCreature(DATA_ARCHBISHOP))
                            arch->AI()->DoAction(ACTION_INTRO_2);
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

                        if (!instance->IsDone(DATA_ARCHBISHOP))
                        {
                            if (isGroupWipe)
                                ResetEncounter();
                            else
                                events.ScheduleEvent(EVENT_RESET_ENCOUNTER, 1000);
                        }
                        break;
                    }
                    case EVENT_LAVABURST_TRIGGER:
                        if (AuraEffect* lavaburst = me->GetAuraEffect(SPELL_LAVABURST_TRIGGER, EFFECT_0))
                            lavaburst->SetPeriodic(true);
                        break;
                    case EVENT_CHAIN_LIGHTNING:
                        if (Creature* light = me->FindNearestCreature(NPC_PURIFYING_LIGHT, 200.0f, true))
                            DoCast(light, SPELL_CHAIN_LIGHTNING);
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
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetHourOfTwilightAI<npc_thrall_hot_4AI>(creature);
    }
};

class npc_hot_thrall_corrupted_slime : public CreatureScript
{
public:
    npc_hot_thrall_corrupted_slime() : CreatureScript("npc_hot_thrall_corrupted_slime") { }

    struct npc_hot_thrall_corrupted_slimeAI : public ScriptedAI
    {
        npc_hot_thrall_corrupted_slimeAI(Creature* creature) : ScriptedAI(creature), instance(me->GetInstanceScript()) { }

        void IsSummonedBy(Unit* /*summoner*/) override
        {
            if (Creature* thrall = instance->GetCreature(DATA_THRALL_EVENT_3))
                thrall->AI()->JustSummoned(me);
        }

        private:
            InstanceScript *instance;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_hot_thrall_corrupted_slimeAI(creature);
    }
};

// 103149
class spell_hot_righteous_shear_dummy : public SpellScriptLoader
{
public:
    spell_hot_righteous_shear_dummy() : SpellScriptLoader("spell_hot_righteous_shear_dummy") { }

    class spell_hot_righteous_shear_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hot_righteous_shear_dummy_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            Trinity::Containers::RandomResizeList(targets, 1);
        }

        void HandleDummy(SpellEffIndex effIndex)
        {
            uint32 spellId = GetSpellInfo()->Effects[effIndex].CalcValue();
            if (Unit *caster = GetCaster())
                if (Unit *target = GetHitUnit())
                    caster->CastSpell(target, spellId, true);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hot_righteous_shear_dummy_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(spell_hot_righteous_shear_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_hot_righteous_shear_dummy_SpellScript();
    }
};

// 103151
class spell_hot_righteous_shear_periodic : public SpellScriptLoader
{
public:
    spell_hot_righteous_shear_periodic() : SpellScriptLoader("spell_hot_righteous_shear_periodic") { }

private:
    class spell_hot_righteous_shear_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hot_righteous_shear_periodic_AuraScript)

        void HandlePeriodicTick(AuraEffect const* aurEff)
        {
            PreventDefaultAction();
            GetCaster()->CastCustomSpell(GetSpellInfo()->Effects[EFFECT_0].TriggerSpell, SPELLVALUE_BASE_POINT0, 7500 * aurEff->GetBase()->GetStackAmount(), GetCaster(), true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_hot_righteous_shear_periodic_AuraScript::HandlePeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_hot_righteous_shear_periodic_AuraScript();
    }
};

class spell_hot_purifying_light_dummy : public SpellScriptLoader // same for shadow part
{
public:
    spell_hot_purifying_light_dummy() : SpellScriptLoader("spell_hot_purifying_light_dummy") { }

    class spell_hot_purifying_light_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hot_purifying_light_dummy_SpellScript);
        
        bool Load()
        {
            return GetCaster()->GetTypeId() == TYPEID_UNIT;
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(Trinity::ObjectTypeIdCheck(TYPEID_PLAYER, false));
            _targets = targets;
        }

        void HandleLiberateOrb(SpellEffIndex effIndex)
        {
            std::list<Creature*> creatureList;
            GetCreatureListWithEntryInGrid(creatureList, GetCaster(), GetSpellInfo()->Id == SPELL_PURIFYING_LIGHT_DUMMY ? NPC_PURIFYING_LIGHT : NPC_CORRUPTING_TWILIGHT, 50.0f);
            for (Creature* orb : creatureList)
            {
                if (!orb->isAlive())
                    continue;

                if (_targets.empty())
                {
                    orb->DespawnOrUnsummon();
                    continue;
                }

                if (WorldObject* target = Trinity::Containers::SelectRandomContainerElement(_targets))
                {
                    orb->CastSpell(target->ToUnit(), orb->GetEntry() == NPC_PURIFYING_LIGHT ? SPELL_PURIFYING_ORB_EXPLOSION : SPELL_SHADOW_ORB_EXPLOSION, true);
                    _targets.remove(target);
                }
            }
        }

        std::list<WorldObject* > _targets;

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hot_purifying_light_dummy_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnEffectHit += SpellEffectFn(spell_hot_purifying_light_dummy_SpellScript::HandleLiberateOrb, EFFECT_1, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_hot_purifying_light_dummy_SpellScript();
    }
};

// 102214
class spell_hot_summon_blood : public SpellScriptLoader
{
public:
    spell_hot_summon_blood() : SpellScriptLoader("spell_hot_summon_blood") { }

    class spell_hot_summon_blood_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hot_summon_blood_SpellScript);

        void HandleScriptEffect(SpellEffIndex effIndex)
        {
            if (Unit *caster = GetCaster())
                caster->CastSpell(caster, SPELL_SUMMON_CORRUPTED_SLIME, true);
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_hot_summon_blood_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_hot_summon_blood_SpellScript();
    }
};

class isOnPlatform
{
public:
    isOnPlatform()
    {}

    bool operator()(WorldObject* object)
    {
        if (Unit* unit = object->ToUnit())
        {
            if (unit->GetPositionZ() < -116.5f)
                return false;
        }
        return true;
    }
};

class spell_hot_seaping_light : public SpellScriptLoader
{
public:
    spell_hot_seaping_light() : SpellScriptLoader("spell_hot_seaping_light")
    {}

    class spell_hot_seaping_light_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hot_seaping_light_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(isOnPlatform());
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hot_seaping_light_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hot_seaping_light_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_hot_seaping_light_SpellScript();
    }
};

class isNotThrall
{
public:
    isNotThrall()
    {}

    bool operator()(WorldObject* object)
    {
        if (Unit* unit = object->ToUnit())
        {
            if (unit->GetEntry() == NPC_THRALL)
                return false;
        }
        return true;
    }
};

class spell_hot_engulfing_twilight : public SpellScriptLoader
{
public:
    spell_hot_engulfing_twilight() : SpellScriptLoader("spell_hot_engulfing_twilight")
    {}

    class spell_hot_engulfing_twilight_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hot_engulfing_twilight_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(isNotThrall());
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hot_engulfing_twilight_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_hot_engulfing_twilight_SpellScript();
    }
};

class spell_hot_corrupted_slime_trigger : public SpellScriptLoader
{
public:
    spell_hot_corrupted_slime_trigger() : SpellScriptLoader("spell_hot_corrupted_slime_trigger") { }

    class spell_hot_corrupted_slime_trigger_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hot_corrupted_slime_trigger_AuraScript);

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            if (Unit *caster = GetCaster())
                if (Creature *slime = caster->ToCreature())
                    slime->DespawnOrUnsummon(100);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_hot_corrupted_slime_trigger_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_hot_corrupted_slime_trigger_AuraScript();
    }
};

class spell_hot_corrupted_bite_proc_aura : public SpellScriptLoader
{
public:
    spell_hot_corrupted_bite_proc_aura() : SpellScriptLoader("spell_hot_corrupted_bite_proc_aura") { }

    class spell_hot_corrupted_bite_proc_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hot_corrupted_bite_proc_aura_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            return eventInfo.GetActionTarget()->GetEntry() == NPC_THRALL_EVENT_3;
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_hot_corrupted_bite_proc_aura_AuraScript::CheckProc);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_hot_corrupted_bite_proc_aura_AuraScript();
    }
};

class spell_hot_corrupted_bite : public SpellScriptLoader
{
public:
    spell_hot_corrupted_bite() : SpellScriptLoader("spell_hot_corrupted_bite") { }

    class spell_hot_corrupted_bite_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hot_corrupted_bite_SpellScript);

        bool Load()
        {
            return GetCaster()->GetTypeId() == TYPEID_UNIT;
        }

        void ForceDespawn()
        {
            GetCaster()->ToCreature()->DespawnOrUnsummon(500);
        }

        void Register() override
        {
            AfterHit += SpellHitFn(spell_hot_corrupted_bite_SpellScript::ForceDespawn);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_hot_corrupted_bite_SpellScript();
    }
};

void AddSC_boss_archbishop_benedictus()
{
    new boss_archbishop_benedictus();
    new npc_hot_archbishop_orb_jump_summon();
    new npc_thrall_hot_3();
    new npc_hot_corrupted_slime();
    new npc_hot_thrall_corrupted_slime();
    new spell_hot_righteous_shear_dummy();
    new spell_hot_righteous_shear_periodic();
    new spell_hot_purifying_light_dummy();
    new spell_hot_summon_blood();
    new spell_hot_seaping_light();
    new spell_hot_engulfing_twilight();
    new spell_hot_corrupted_slime_trigger();
    new npc_thrall_hot_4();
    new spell_hot_corrupted_bite_proc_aura();
    new spell_hot_corrupted_bite();
}
