/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "GridNotifiers.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellAuras.h"
#include "MapManager.h"
#include "Transport.h"
#include "icecrown_citadel.h"

enum ScriptTexts
{
    // Deathbringer Saurfang
    SAY_INTRO_ALLIANCE_2            = 0,
    SAY_INTRO_ALLIANCE_3            = 1,
    SAY_INTRO_ALLIANCE_6            = 2,
    SAY_INTRO_ALLIANCE_7            = 3,
    SAY_INTRO_HORDE_2               = 4,
    SAY_INTRO_HORDE_4               = 5,
    SAY_INTRO_HORDE_9               = 6,
    SAY_AGGRO                       = 7,
    SAY_MARK_OF_THE_FALLEN_CHAMPION = 8,
    SAY_BLOOD_BEASTS                = 9,
    SAY_KILL                        = 10,
    SAY_FRENZY                      = 11,
    SAY_BERSERK                     = 12,
    SAY_DEATH                       = 13,
    EMOTE_SCENT_OF_BLOOD            = 14,

    // High Overlord Saurfang
    SAY_INTRO_HORDE_1               = 0,
    SAY_INTRO_HORDE_3               = 1,
    SAY_INTRO_HORDE_5               = 2,
    SAY_INTRO_HORDE_6               = 3,
    SAY_INTRO_HORDE_7               = 4,
    SAY_INTRO_HORDE_8               = 5,
    SAY_OUTRO_ALLIANCE_8            = 6,
    SAY_OUTRO_ALLIANCE_12           = 7, // kneel after WP reached
    SAY_OUTRO_ALLIANCE_13           = 8,
    SAY_OUTRO_ALLIANCE_14           = 9,
    SAY_OUTRO_ALLIANCE_15           = 10,
    SAY_OUTRO_HORDE_1               = 11,
    SAY_OUTRO_HORDE_2               = 12,
    SAY_OUTRO_HORDE_3               = 13,
    SAY_OUTRO_HORDE_4               = 14,

    // Muradin Bronzebeard
    SAY_INTRO_ALLIANCE_1            = 0,
    SAY_INTRO_ALLIANCE_4            = 1,
    SAY_INTRO_ALLIANCE_5            = 2,
    SAY_OUTRO_ALLIANCE_1            = 3, // TODO ALLIANCE OUTRO
    SAY_OUTRO_ALLIANCE_2            = 4,
    SAY_OUTRO_ALLIANCE_3            = 5,
    SAY_OUTRO_ALLIANCE_4            = 6,
    SAY_OUTRO_ALLIANCE_5            = 7,
    SAY_OUTRO_ALLIANCE_6            = 8,
    SAY_OUTRO_ALLIANCE_7            = 9,
    SAY_OUTRO_ALLIANCE_9            = 10,
    SAY_OUTRO_ALLIANCE_10           = 11,
    SAY_OUTRO_ALLIANCE_21           = 12,

    // Lady Jaina Proudmoore
    SAY_OUTRO_ALLIANCE_17           = 0,
    SAY_OUTRO_ALLIANCE_19           = 1,

    // King Varian Wrynn
    SAY_OUTRO_ALLIANCE_11           = 0,
    SAY_OUTRO_ALLIANCE_16           = 1,
    SAY_OUTRO_ALLIANCE_18           = 2,
    SAY_OUTRO_ALLIANCE_20           = 3,
};

enum Spells
{
    // Deathbringer Saurfang
    SPELL_ZERO_POWER                          = 72242,
    SPELL_GRIP_OF_AGONY                       = 70572, // Intro
    SPELL_BLOOD_LINK                          = 72178,
    SPELL_MARK_OF_THE_FALLEN_CHAMPION_S       = 72256,
    SPELL_MARK_OF_THE_FALLEN_CHAMPION_HEAL    = 72260,
    SPELL_MARK_OF_THE_FALLEN_CHAMPION_HEAL_H  = 72280,
    SPELL_RUNE_OF_BLOOD_S                     = 72408,

    SPELL_SUMMON_BLOOD_BEAST                  = 72172,
    SPELL_SUMMON_BLOOD_BEAST_25_MAN           = 72356, // Additional cast, does not replace
    SPELL_FRENZY                              = 72737,
    SPELL_BLOOD_NOVA_TRIGGER                  = 72378,
    SPELL_BLOOD_NOVA                          = 72380,
    SPELL_BLOOD_POWER                         = 72371,
    SPELL_BLOOD_LINK_POWER                    = 72195,
    SPELL_BLOOD_LINK_DUMMY                    = 72202,
    SPELL_MARK_OF_THE_FALLEN_CHAMPION         = 72293,
    SPELL_BOILING_BLOOD                       = 72385,
    SPELL_RUNE_OF_BLOOD                       = 72410,

    // Blood Beast
    SPELL_BLOOD_LINK_BEAST                    = 72176,
    SPELL_RESISTANT_SKIN                      = 72723,
    SPELL_SCENT_OF_BLOOD                      = 72769, // Heroic only
    SPELL_SCENT_OF_BLOOD_TRIGGERED            = 72771,

    SPELL_RIDE_VEHICLE                        = 70640, // Outro
    SPELL_ACHIEVEMENT                         = 72928,
    SPELL_REMOVE_MARKS_OF_THE_FALLEN_CHAMPION = 72257,
    SPELL_PERMANENT_FEIGN_DEATH               = 70628,
};

// Helper to get id of the aura on different modes (HasAura(baseId) wont work)
#define BOILING_BLOOD_HELPER RAID_MODE<int32>(72385, 72441, 72442, 72443)

enum EventTypes
{
    EVENT_INTRO_ALLIANCE_1 = 1,
    EVENT_INTRO_ALLIANCE_2 = 2,
    EVENT_INTRO_ALLIANCE_3 = 3,
    EVENT_INTRO_ALLIANCE_4 = 4,
    EVENT_INTRO_ALLIANCE_5 = 5,
    EVENT_INTRO_ALLIANCE_6 = 6,
    EVENT_INTRO_ALLIANCE_7 = 7,

    EVENT_INTRO_HORDE_1 = 8,
    EVENT_INTRO_HORDE_2 = 9,
    EVENT_INTRO_HORDE_3 = 10,
    EVENT_INTRO_HORDE_4 = 11,
    EVENT_INTRO_HORDE_5 = 12,
    EVENT_INTRO_HORDE_6 = 13,
    EVENT_INTRO_HORDE_7 = 14,
    EVENT_INTRO_HORDE_8 = 15,
    EVENT_INTRO_HORDE_9 = 16,

    EVENT_INTRO_FINISH = 17,

    EVENT_BERSERK = 18,
    EVENT_SUMMON_BLOOD_BEAST = 19,
    EVENT_BOILING_BLOOD = 20,
    EVENT_BLOOD_NOVA = 21,
    EVENT_RUNE_OF_BLOOD = 22,
    EVENT_SCENT_OF_BLOOD = 52,

    EVENT_OUTRO_ALLIANCE_1 = 23,
    EVENT_OUTRO_ALLIANCE_2 = 24,
    EVENT_OUTRO_ALLIANCE_3 = 25,
    EVENT_OUTRO_ALLIANCE_4 = 26,
    EVENT_OUTRO_ALLIANCE_5 = 27,
    EVENT_OUTRO_ALLIANCE_6 = 28,
    EVENT_OUTRO_ALLIANCE_7 = 29,
    EVENT_OUTRO_ALLIANCE_8 = 30,
    EVENT_OUTRO_ALLIANCE_9 = 31,
    EVENT_OUTRO_ALLIANCE_10 = 32,
    EVENT_OUTRO_ALLIANCE_10_1 = 33,
    EVENT_OUTRO_ALLIANCE_11 = 34,
    EVENT_OUTRO_ALLIANCE_11_1 = 35,
    EVENT_OUTRO_ALLIANCE_12 = 36,
    EVENT_OUTRO_ALLIANCE_13 = 37,
    EVENT_OUTRO_ALLIANCE_14 = 38,
    EVENT_OUTRO_ALLIANCE_14_1 = 39,
    EVENT_OUTRO_ALLIANCE_15 = 40,
    EVENT_OUTRO_ALLIANCE_16 = 41,
    EVENT_OUTRO_ALLIANCE_16_1 = 42,
    EVENT_OUTRO_ALLIANCE_17 = 43,
    EVENT_OUTRO_ALLIANCE_18 = 44,
    EVENT_OUTRO_ALLIANCE_19 = 45,
    EVENT_OUTRO_ALLIANCE_20 = 46,
    EVENT_OUTRO_ALLIANCE_21 = 47,
    EVENT_OUTRO_ALLIANCE_21_1       = 48,
    EVENT_OUTRO_ALLIANCE_21_2       = 49,
    EVENT_OUTRO_ALLIANCE_21_3       = 50,
    EVENT_OUTRO_ALLIANCE_21_4       = 51,
    EVENT_OUTRO_ALLIANCE_21_5       = 52,
    EVENT_OUTRO_ALLIANCE_21_6       = 53,
    EVENT_OUTRO_ALLIANCE_21_7       = 54,
    EVENT_OUTRO_ALLIANCE_21_8       = 55,
    EVENT_OUTRO_ALLIANCE_21_9       = 56,

    EVENT_OUTRO_HORDE_1 = 57,
    EVENT_OUTRO_HORDE_2 = 58,
    EVENT_OUTRO_HORDE_3 = 59,
    EVENT_OUTRO_HORDE_4 = 60,
    EVENT_OUTRO_HORDE_5 = 61,
    EVENT_OUTRO_HORDE_6 = 62,
    EVENT_OUTRO_HORDE_6_1       = 63,
    EVENT_OUTRO_HORDE_6_2       = 64,
    EVENT_OUTRO_HORDE_6_3       = 65,
    EVENT_OUTRO_HORDE_6_4       = 66,
    EVENT_OUTRO_HORDE_6_5       = 67,

    EVENT_OUTRO_HORDE_7 = 68,
    EVENT_OUTRO_HORDE_8 = 69,
};

enum Phases
{
    PHASE_INTRO_A = 1,
    PHASE_INTRO_H = 2,
    PHASE_COMBAT = 3,
};

enum Actions
{
    ACTION_START_EVENT = -3781300,
    ACTION_CONTINUE_INTRO = -3781301,
    ACTION_CHARGE = -3781302,
    ACTION_START_OUTRO = -3781303,
    ACTION_DESPAWN = -3781304,
    ACTION_INTERRUPT_INTRO = -3781305,
    ACTION_MOVE                         = -3781306,
    ACTION_STAND                        = -3781307,
    ACTION_KNEEL                        = -3781308,
    ACTION_KING                         = -3781309,
    ACTION_EXIT_1                       = -3791310,
    ACTION_EXIT_2                       = -3791311,
    ACTION_MARK_OF_THE_FALLEN_CHAMPION = -72293,
};

#define DATA_MADE_A_MESS 45374613 // 4537, 4613 are achievement IDs

enum MovePoints
{
    POINT_SAURFANG = 3781300,
    POINT_FIRST_STEP = 3781301,
    POINT_CHARGE = 3781302,
    POINT_CHOKE = 3781303,
    POINT_CORPSE = 3781304,
    POINT_FINAL = 3781305,
    POINT_STAND             = 3781306,
    POINT_CORPSE_1          = 3781307,
    POINT_KING              = 3781308,
    POINT_EXIT_1            = 3781309,
    POINT_EXIT_2            = 3781310,
    POINT_EXIT = 5,        // waypoint id
};

Position const deathbringerPos = { -496.3542f, 2211.33f, 541.1138f, 0.0f };
Position const firstStepPos = { -541.3177f, 2211.365f, 539.2921f, 0.0f };

Position const chargePos[6] =
{
    { -509.6505f, 2211.377f, 539.2872f, 0.0f }, // High Overlord Saurfang/Muradin Bronzebeard
    { -508.7480f, 2211.897f, 539.2870f, 0.0f }, // front left
    { -509.2929f, 2211.411f, 539.2870f, 0.0f }, // front right
    { -506.6607f, 2211.367f, 539.2870f, 0.0f }, // back middle
    { -506.1137f, 2213.306f, 539.2870f, 0.0f }, // back left
    { -509.0040f, 2211.743f, 539.2870f, 0.0f }  // back right
};

Position const chokePos[6] =
{
    { -514.4834f, 2211.334f, 549.2887f, 0.0f }, // High Overlord Saurfang/Muradin Bronzebeard
    { -510.1081f, 2211.592f, 546.3773f, 0.0f }, // front left
    { -513.3210f, 2211.396f, 551.2882f, 0.0f }, // front right
    { -507.3684f, 2210.353f, 545.7497f, 0.0f }, // back middle
    { -507.0486f, 2212.999f, 545.5512f, 0.0f }, // back left
    { -510.7041f, 2211.069f, 546.5298f, 0.0f }  // back right
};

Position const finalPos = { -563.7552f, 2211.328f, 538.7848f, 0.0f };

Position const movePos[5] =
{
    {-516.70f, 2227.09f, 539.28f, 0.0f}, // front left
    {-514.98f, 2226.66f, 539.28f, 0.0f}, // front right
    {-513.25f, 2226.23f, 539.28f, 0.0f}, // back middle
    {-520.84f, 2227.91f, 539.28f, 0.0f}, // back left
    {-523.24f, 2228.28f, 539.29f, 0.0f}  // back right
};

Position const standPos[5] =
{
    {-538.71f, 2228.63f, 539.29f, 0.0f},
    {-539.12f, 2226.92f, 539.29f, 0.0f},
    {-539.80f, 2225.36f, 539.29f, 0.0f},
    {-540.32f, 2223.16f, 539.29f, 0.0f},
    {-540.79f, 2221.18f, 539.29f, 0.0f}
};

Position const exitPos[5] =
{
    {-557.30f, 2208.20f, 539.29f, 0.0f},
    {-559.16f, 2211.43f, 539.29f, 0.0f},
    {-557.26f, 2215.06f, 539.29f, 0.0f},
    {-561.04f, 2207.88f, 539.29f, 0.0f},
    {-560.90f, 2214.58f, 539.29f, 0.0f}
};


class boss_deathbringer_saurfang : public CreatureScript
{
    public:
        boss_deathbringer_saurfang() : CreatureScript("boss_deathbringer_saurfang") { }

        struct boss_deathbringer_saurfangAI : public BossAI
        {
            boss_deathbringer_saurfangAI(Creature* creature) : BossAI(creature, DATA_DEATHBRINGER_SAURFANG)
            {
                ASSERT(creature->GetVehicleKit()); // we dont actually use it, just check if exists
                _introDone = false;
                _fallenChampionCastCount = 0;
            }

            void Reset()
            {
                _Reset();
                me->SetReactState(REACT_DEFENSIVE);
                events.SetPhase(PHASE_COMBAT);
                _frenzied = false;
                _dead = false;
                me->SetPower(POWER_ENERGY, 0);
                DoCast(me, SPELL_ZERO_POWER, true);
                DoCast(me, SPELL_BLOOD_LINK, true);
                //   DoCast(me, SPELL_BLOOD_POWER, true);
                DoCast(me, SPELL_MARK_OF_THE_FALLEN_CHAMPION_S, true);
                DoCast(me, SPELL_RUNE_OF_BLOOD_S, true);
                me->RemoveAurasDueToSpell(SPELL_BERSERK);
                me->RemoveAurasDueToSpell(SPELL_FRENZY);
            }

            void EnterCombat(Unit* who)
            {
                if (_dead)
                    return;

                if (!instance->CheckRequiredBosses(DATA_DEATHBRINGER_SAURFANG, who->ToPlayer()))
                {
                    EnterEvadeMode();
                    instance->DoCastSpellOnPlayers(LIGHT_S_HAMMER_TELEPORT);
                    return;
                }

                // oh just screw intro, enter combat - no exploits please
                me->setActive(true);
                DoZoneInCombat();

                events.Reset();
                events.SetPhase(PHASE_COMBAT);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                DoCast(me, SPELL_BLOOD_POWER, true);
                if (!_introDone)
                {
                    DoCast(me, SPELL_GRIP_OF_AGONY);
                    if (Creature* creature = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_SAURFANG_EVENT_NPC)))
                        creature->AI()->DoAction(ACTION_INTERRUPT_INTRO);
                }

                _introDone = true;

                Talk(SAY_AGGRO);
                events.ScheduleEvent(EVENT_SUMMON_BLOOD_BEAST, 40000, 0, PHASE_COMBAT);
                events.ScheduleEvent(EVENT_BERSERK, IsHeroic() ? 360000 : 480000, 0, PHASE_COMBAT);
                events.ScheduleEvent(EVENT_BOILING_BLOOD, 15500, 0, PHASE_COMBAT);
                events.ScheduleEvent(EVENT_BLOOD_NOVA, 17000, 0, PHASE_COMBAT);
                events.ScheduleEvent(EVENT_RUNE_OF_BLOOD, 20000, 0, PHASE_COMBAT);

                _fallenChampionCastCount = 0;
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MARK_OF_THE_FALLEN_CHAMPION);
                instance->SetBossState(DATA_DEATHBRINGER_SAURFANG, IN_PROGRESS);
            }

            void AttackStart(Unit* victim)
            {
                if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC))
                    return;

                ScriptedAI::AttackStart(victim);
            }

            void EnterEvadeMode()
            {
                ScriptedAI::EnterEvadeMode();
                if (_introDone)
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            }

            void JustReachedHome()
            {
                _JustReachedHome();
                instance->SetBossState(DATA_DEATHBRINGER_SAURFANG, FAIL);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MARK_OF_THE_FALLEN_CHAMPION);
                Reset();
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_KILL);

                if (victim->HasAura(SPELL_MARK_OF_THE_FALLEN_CHAMPION))
                    me->CastSpell(me, (IsHeroic() ? SPELL_MARK_OF_THE_FALLEN_CHAMPION_HEAL_H : SPELL_MARK_OF_THE_FALLEN_CHAMPION_HEAL), true);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage)
            {
                if (damage >= me->GetHealth())
                    damage = me->GetHealth() - 1;

                if (!_frenzied && HealthBelowPct(31)) // AT 30%, not below
                {
                    _frenzied = true;
                    DoCast(me, SPELL_FRENZY);
                    Talk(SAY_FRENZY);
                }

                if (!_dead && me->GetHealth() < FightWonValue)
                {
                    _dead = true;
                    _JustDied();
                    _EnterEvadeMode();
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);

                    DoCastAOE(SPELL_REMOVE_MARKS_OF_THE_FALLEN_CHAMPION);
                    DoCast(me, SPELL_ACHIEVEMENT, true);
                    Talk(SAY_DEATH);

                    //instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MARK_OF_THE_FALLEN_CHAMPION);
                    DoCast(me, SPELL_PERMANENT_FEIGN_DEATH);
                    if (Creature* creature = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_SAURFANG_EVENT_NPC)))
                        creature->AI()->DoAction(ACTION_START_OUTRO);
                    instance->SetBossState(DATA_DEATHBRINGER_SAURFANG, IsHeroic() ? DONE_HM : DONE);
                }
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
            }

            void SummonedCreatureDies(Creature* summon, Unit* /*killer*/)
            {
                summons.Despawn(summon);
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type != POINT_MOTION_TYPE && id != POINT_SAURFANG)
                    return;

                instance->HandleGameObject(instance->GetData64(GO_SAURFANG_S_DOOR), false);
            }

            void SpellHitTarget(Unit* target, SpellInfo const* spell) override
            {
                switch (spell->Id)
                {
                    case SPELL_MARK_OF_THE_FALLEN_CHAMPION:
                        Talk(SAY_MARK_OF_THE_FALLEN_CHAMPION);
                        break;
                    case 72255: // Mark of the Fallen Champion, triggered id
                    case 72444:
                    case 72445:
                    case 72446:
                        if (me->GetPower(POWER_ENERGY) != me->GetMaxPower(POWER_ENERGY))
                            target->CastCustomSpell(SPELL_BLOOD_LINK_DUMMY, SPELLVALUE_BASE_POINT0, 1, me, true);
                        break;
                default:
                    break;
                }
            }

            void UpdateAI(uint32 const diff) override
            {
                if (!UpdateVictim() && !(events.IsInPhase(PHASE_INTRO_A) || events.IsInPhase(PHASE_INTRO_H)))
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_INTRO_ALLIANCE_2:
                        Talk(SAY_INTRO_ALLIANCE_2);
                        break;
                    case EVENT_INTRO_ALLIANCE_3:
                        Talk(SAY_INTRO_ALLIANCE_3);
                        break;
                    case EVENT_INTRO_ALLIANCE_6:
                        Talk(SAY_INTRO_ALLIANCE_6);
                        Talk(SAY_INTRO_ALLIANCE_7);
                        DoCast(me, SPELL_GRIP_OF_AGONY);
                        break;
                    case EVENT_INTRO_HORDE_2:
                        Talk(SAY_INTRO_HORDE_2);
                        break;
                    case EVENT_INTRO_HORDE_4:
                        Talk(SAY_INTRO_HORDE_4);
                        break;
                    case EVENT_INTRO_HORDE_9:
                        DoCast(me, SPELL_GRIP_OF_AGONY);
                        Talk(SAY_INTRO_HORDE_9);
                        break;
                    case EVENT_INTRO_FINISH:
                        events.SetPhase(PHASE_COMBAT);
                        _introDone = true;
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                        break;
                    case EVENT_SUMMON_BLOOD_BEAST:
                        for (uint32 i10 = 0; i10 < 2; ++i10)
                            DoCast(me, SPELL_SUMMON_BLOOD_BEAST + i10);
                        if (Is25ManRaid())
                        for (uint32 i25 = 0; i25 < 3; ++i25)
                            DoCast(me, SPELL_SUMMON_BLOOD_BEAST_25_MAN + i25);
                        Talk(SAY_BLOOD_BEASTS);
                        events.ScheduleEvent(EVENT_SUMMON_BLOOD_BEAST, 40000, 0, PHASE_COMBAT);
                        if (IsHeroic())
                            events.ScheduleEvent(EVENT_SCENT_OF_BLOOD, 10000, 0, PHASE_COMBAT);
                        break;
                    case EVENT_BLOOD_NOVA:
                        DoCastAOE(SPELL_BLOOD_NOVA_TRIGGER);
                        events.ScheduleEvent(EVENT_BLOOD_NOVA, urand(20000, 25000), 0, PHASE_COMBAT);
                        break;
                    case EVENT_RUNE_OF_BLOOD:
                        DoCastVictim(SPELL_RUNE_OF_BLOOD);
                        events.ScheduleEvent(EVENT_RUNE_OF_BLOOD, urand(20000, 25000), 0, PHASE_COMBAT);
                        break;
                    case EVENT_BOILING_BLOOD:
                        DoCast(me, SPELL_BOILING_BLOOD);
                        events.ScheduleEvent(EVENT_BOILING_BLOOD, urand(15000, 20000), 0, PHASE_COMBAT);
                        break;
                    case EVENT_BERSERK:
                        DoCast(me, SPELL_BERSERK);
                        Talk(SAY_BERSERK);
                        break;
                    case EVENT_SCENT_OF_BLOOD:
                        if (!summons.empty())
                        {
                            Talk(EMOTE_SCENT_OF_BLOOD);
                            DoCastAOE(SPELL_SCENT_OF_BLOOD);
                        }
                        break;
                    default:
                        break;
                    }
                }

                DoMeleeAttackIfReady();
            }

            uint32 GetData(uint32 type)  const
            {
                if (type == DATA_MADE_A_MESS)
                if (_fallenChampionCastCount < RAID_MODE<uint32>(3, 5, 3, 5))
                    return 1;

                return 0;
            }

            // intro setup
            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case PHASE_INTRO_A:
                    case PHASE_INTRO_H:
                    {
                        if (GameObject* teleporter = GameObject::GetGameObject(*me, instance->GetData64(GO_SCOURGE_TRANSPORTER_SAURFANG)))
                        {
                            instance->HandleGameObject(0, false, teleporter);
                            teleporter->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
                        }

                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        // controls what events will execute
                        events.SetPhase(uint32(action));
                        me->SetHomePosition(deathbringerPos.GetPositionX(), deathbringerPos.GetPositionY(), deathbringerPos.GetPositionZ(), me->GetOrientation());
                        me->GetMotionMaster()->MovePoint(POINT_SAURFANG, deathbringerPos.GetPositionX(), deathbringerPos.GetPositionY(), deathbringerPos.GetPositionZ());
                        events.ScheduleEvent(EVENT_INTRO_ALLIANCE_2, 2500, 0, PHASE_INTRO_A);
                        events.ScheduleEvent(EVENT_INTRO_ALLIANCE_3, 20000, 0, PHASE_INTRO_A);
                        events.ScheduleEvent(EVENT_INTRO_HORDE_2, 5000, 0, PHASE_INTRO_H);
                        break;
                    }
                    case ACTION_CONTINUE_INTRO:
                    {
                        if (_introDone)
                            return;

                        events.ScheduleEvent(EVENT_INTRO_ALLIANCE_6, 6500 + 500, 0, PHASE_INTRO_A);
                        events.ScheduleEvent(EVENT_INTRO_FINISH, 8000, 0, PHASE_INTRO_A);

                        events.ScheduleEvent(EVENT_INTRO_HORDE_4, 6500, 0, PHASE_INTRO_H);
                        events.ScheduleEvent(EVENT_INTRO_HORDE_9, 46700 + 1000 + 500, 0, PHASE_INTRO_H);
                        events.ScheduleEvent(EVENT_INTRO_FINISH, 46700 + 1000 + 8000, 0, PHASE_INTRO_H);
                        break;
                    }
                    case ACTION_MARK_OF_THE_FALLEN_CHAMPION:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true, -SPELL_MARK_OF_THE_FALLEN_CHAMPION))
                        {
                            ++_fallenChampionCastCount;
                            DoCast(target, SPELL_MARK_OF_THE_FALLEN_CHAMPION);
                            me->SetPower(POWER_ENERGY, 0);
                            if (Aura* bloodPower = me->GetAura(SPELL_BLOOD_POWER))
                                bloodPower->RecalculateAmountOfEffects();
                        }
                        break;
                    }
                    default:
                        break;
                }
            }

            static uint32 const FightWonValue;

        private:
            uint32 _fallenChampionCastCount;
            bool _introDone;
            bool _frenzied;   // faster than iterating all auras to find Frenzy
            bool _dead;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetIcecrownCitadelAI<boss_deathbringer_saurfangAI>(creature);
        }
};

uint32 const boss_deathbringer_saurfang::boss_deathbringer_saurfangAI::FightWonValue = 100000;

class npc_high_overlord_saurfang_icc : public CreatureScript
{
    public:
        npc_high_overlord_saurfang_icc() : CreatureScript("npc_high_overlord_saurfang_icc") { }

        struct npc_high_overlord_saurfangAI : public ScriptedAI
        {
            npc_high_overlord_saurfangAI(Creature* creature) : ScriptedAI(creature)
            {
                ASSERT(creature->GetVehicleKit());
                _instance = me->GetInstanceScript();
            }

            void Reset()
            {
                _hordeportal2GUID = 0;
                _hordeportal1GUID = 0;
                _peon2GUID = 0;
                _peon1GUID = 0;
                _events.Reset();
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case ACTION_START_EVENT:
                    {
                        // Prevent crashes
                        if (_events.IsInPhase(PHASE_INTRO_A) || _events.IsInPhase(PHASE_INTRO_H))
                            return;

                        GetCreatureListWithEntryInGrid(_guardList, me, NPC_SE_KOR_KRON_REAVER, 20.0f);
                        _guardList.sort(Trinity::ObjectDistanceOrderPred(me));
                        uint32 x = 1;
                        for (std::list<Creature*>::iterator itr = _guardList.begin(); itr != _guardList.end(); ++x, ++itr)
                            (*itr)->AI()->SetData(0, x);

                        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        Talk(SAY_INTRO_HORDE_1);
                        _events.SetPhase(PHASE_INTRO_H);
                        _events.ScheduleEvent(EVENT_INTRO_HORDE_3, 18500, 0, PHASE_INTRO_H);
                        _instance->HandleGameObject(_instance->GetData64(GO_SAURFANG_S_DOOR), true);
                        if (Creature* deathbringer = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_DEATHBRINGER_SAURFANG)))
                            deathbringer->AI()->DoAction(PHASE_INTRO_H);
                        break;
                    }
                    case ACTION_START_OUTRO:
                    {
                        me->RemoveAurasDueToSpell(SPELL_GRIP_OF_AGONY);
                        Talk(SAY_OUTRO_HORDE_1);
                        _events.ScheduleEvent(EVENT_OUTRO_HORDE_2, 10000);   // say
                        _events.ScheduleEvent(EVENT_OUTRO_HORDE_3, 18000);   // say
                        _events.ScheduleEvent(EVENT_OUTRO_HORDE_4, 24000);   // cast
                        _events.ScheduleEvent(EVENT_OUTRO_HORDE_5, 30000);   // move
                        me->SetDisableGravity(false);
                        me->GetMotionMaster()->MoveFall();
                        for (std::list<Creature*>::iterator itr = _guardList.begin(); itr != _guardList.end(); ++itr)
                            (*itr)->AI()->DoAction(ACTION_DESPAWN);
                        break;
                    }
                    case ACTION_INTERRUPT_INTRO:
                    {
                        _events.Reset();
                        for (std::list<Creature*>::iterator itr = _guardList.begin(); itr != _guardList.end(); ++itr)
                            (*itr)->AI()->DoAction(ACTION_DESPAWN);
                        break;
                    }
                    default:
                        break;
                }
            }

            void SpellHit(Unit* /*caster*/, SpellInfo const* spell)
            {
                if (spell->Id == SPELL_GRIP_OF_AGONY)
                {
                    me->SetDisableGravity(true);
                    me->GetMotionMaster()->MovePoint(POINT_CHOKE, chokePos[0]);
                }
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type == POINT_MOTION_TYPE)
                {
                    switch (id)
                    {
                    case POINT_FIRST_STEP:
                        me->SetWalk(false);
                        Talk(SAY_INTRO_HORDE_3);
                        _events.ScheduleEvent(EVENT_INTRO_HORDE_5, 15500, 0, PHASE_INTRO_H);
                        _events.ScheduleEvent(EVENT_INTRO_HORDE_6, 29500, 0, PHASE_INTRO_H);
                        _events.ScheduleEvent(EVENT_INTRO_HORDE_7, 43800, 0, PHASE_INTRO_H);
                        _events.ScheduleEvent(EVENT_INTRO_HORDE_8, 47000, 0, PHASE_INTRO_H);
                        if (Creature* deathbringer = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_DEATHBRINGER_SAURFANG)))
                            deathbringer->AI()->DoAction(ACTION_CONTINUE_INTRO);
                        break;
                    case POINT_CORPSE:
                        if (Creature* deathbringer = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_DEATHBRINGER_SAURFANG)))
                        {
                            deathbringer->CastSpell(me, SPELL_RIDE_VEHICLE, true);  // for the packet logs.
                            deathbringer->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                            deathbringer->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_DROWNED);
                        }
                        _events.ScheduleEvent(EVENT_OUTRO_HORDE_5, 1000);    // move
                        _events.ScheduleEvent(EVENT_OUTRO_HORDE_6, 4000);    // say
                        break;
                    case POINT_FINAL:
                        if (Creature* deathbringer = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_DEATHBRINGER_SAURFANG)))
                            deathbringer->DespawnOrUnsummon();
                        me->SetVisible(false);
                        _events.ScheduleEvent(EVENT_OUTRO_HORDE_6_1, 1000);
                        break;
                    default:
                        break;
                    }
                }
                else if (type == WAYPOINT_MOTION_TYPE && id == POINT_EXIT)
                {
                    std::list<Creature*> guards;
                    GetCreatureListWithEntryInGrid(guards, me, NPC_KOR_KRON_GENERAL, 50.0f);
                    for (std::list<Creature*>::iterator itr = guards.begin(); itr != guards.end(); ++itr)
                        (*itr)->DespawnOrUnsummon();
                    me->DespawnOrUnsummon();
                }
            }

            void UpdateAI(uint32 const diff)
            {
                _events.Update(diff);
                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_INTRO_HORDE_3:
                            me->SetWalk(true);
                            me->GetMotionMaster()->MovePoint(POINT_FIRST_STEP, firstStepPos.GetPositionX(), firstStepPos.GetPositionY(), firstStepPos.GetPositionZ());
                            break;
                        case EVENT_INTRO_HORDE_5:
                            Talk(SAY_INTRO_HORDE_5);
                            break;
                        case EVENT_INTRO_HORDE_6:
                            Talk(SAY_INTRO_HORDE_6);
                            break;
                        case EVENT_INTRO_HORDE_7:
                            Talk(SAY_INTRO_HORDE_7);
                            break;
                        case EVENT_INTRO_HORDE_8:
                            Talk(SAY_INTRO_HORDE_8);
                            for (std::list<Creature*>::iterator itr = _guardList.begin(); itr != _guardList.end(); ++itr)
                                (*itr)->AI()->DoAction(ACTION_CHARGE);
                            me->GetMotionMaster()->MoveCharge(chargePos[0].GetPositionX(), chargePos[0].GetPositionY(), chargePos[0].GetPositionZ(), 8.5f, POINT_CHARGE);
                            break;
                        case EVENT_OUTRO_HORDE_2:   // say
                            if (Creature* deathbringer = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_DEATHBRINGER_SAURFANG)))
                                me->SetFacingToObject(deathbringer);
                            Talk(SAY_OUTRO_HORDE_2);
                            break;
                        case EVENT_OUTRO_HORDE_3:   // say
                            Talk(SAY_OUTRO_HORDE_3);
                            break;
                        case EVENT_OUTRO_HORDE_4:   // move
                            if (Creature* deathbringer = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_DEATHBRINGER_SAURFANG)))
                            {
                                float x, y, z;
                                deathbringer->GetClosePoint(x, y, z, deathbringer->GetObjectSize());
                                me->SetWalk(true);
                                me->GetMotionMaster()->MovePoint(POINT_CORPSE, x, y, z);
                            }
                            break;
                        case EVENT_OUTRO_HORDE_5:   // move
                            me->GetMotionMaster()->MovePoint(POINT_FINAL, finalPos);
                            break;
                        case EVENT_OUTRO_HORDE_6:   // say
                            Talk(SAY_OUTRO_HORDE_4);
                            break;
                        case EVENT_OUTRO_HORDE_6_1:
                            if (GameObject *hordeportal1 = me->SummonGameObject(195702, -561.16f, 2219.30f, 539.28f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, DAY))
                            {
                                hordeportal1->SetGoState(GO_STATE_READY);
                                _hordeportal1GUID = hordeportal1->GetGUID();
                            }
                            if (GameObject *hordeportal2 = me->SummonGameObject(195702, -560.84f, 2202.57f, 539.28f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, DAY))
                            {
                                hordeportal2->SetGoState(GO_STATE_READY);
                                _hordeportal2GUID = hordeportal2->GetGUID();
                            }
                            _events.ScheduleEvent(EVENT_OUTRO_HORDE_6_2, 5000);
                            break;
                        case EVENT_OUTRO_HORDE_6_2:
                            if (GameObject *hordeportal1 = GameObject::GetGameObject(*me, _hordeportal1GUID))
                                hordeportal1->SetGoState(GO_STATE_ACTIVE);
                            if (GameObject *hordeportal2 = GameObject::GetGameObject(*me, _hordeportal2GUID))
                                hordeportal2->SetGoState(GO_STATE_ACTIVE);
                            _events.ScheduleEvent(EVENT_OUTRO_HORDE_6_3, 5000);
                            break;
                        case EVENT_OUTRO_HORDE_6_3:
                            if (Creature *peon1 = me->SummonCreature(14901, -561.16f, 2219.30f, 539.28f, 0.0f))
                            {
                                peon1->GetMotionMaster()->MovePoint(0, -538.82f, 2225.95f, 539.29f);
                                _peon1GUID = peon1->GetGUID();
                            }
                            if (Creature *peon2 = me->SummonCreature(14901, -560.84f, 2202.57f, 539.28f, 0.0f))
                            {
                                peon2->GetMotionMaster()->MovePoint(0, -534.05f, 2222.42f, 539.29f);
                                _peon2GUID = peon2->GetGUID();
                            }
                            _events.ScheduleEvent(EVENT_OUTRO_HORDE_6_4, 5000);
                            break;
                        case EVENT_OUTRO_HORDE_6_4:
                            if (Creature *peon1 = Unit::GetCreature(*me, _peon1GUID))
                                peon1->SetStandState(EMOTE_STATE_WORK);
                            if (Creature *peon2 = Unit::GetCreature(*me, _peon2GUID))
                                peon2->SetStandState(EMOTE_STATE_WORK);
                            _events.ScheduleEvent(EVENT_OUTRO_HORDE_6_5, 5000);
                            break;
                        case EVENT_OUTRO_HORDE_6_5:
                            if (GameObject* pGo = me->SummonGameObject(GO_SAURFANG_FORGE, -514.589f, 2244.63f, 539.291f, 5.04638f, 0, 0, 0, 0, 7*DAY))
                                pGo->SetRespawnTime(7*DAY);
                            if (GameObject* pGo = me->SummonGameObject(GO_SAURFANG_ANVIL, -518.931f, 2242.55f, 539.29f, 5.282f, 0, 0, 0, 0, 7*DAY))
                                pGo->SetRespawnTime(7*DAY);
                            if (GameObject* pGo = me->SummonGameObject(GO_SAURFANG_BONFIRE, -525.496f, 2227.39f, 539.291f, 2.18124f, 0, 0, 0, 0, 7*DAY))
                                pGo->SetRespawnTime(7*DAY);
                            if (GameObject* pGo = me->SummonGameObject(GO_SAURFANG_HORDE_TENT1, -524.565f, 2236.97f, 539.29f, 2.23466f, 0, 0, 0, 0, 7*DAY))
                                pGo->SetRespawnTime(7*DAY);
                            if (GameObject* pGo = me->SummonGameObject(GO_SAURFANG_HORDE_TENT2, -532.78f, 2228.43f, 539.29f, 2.48599f, 0, 0, 0, 0, 7*DAY))
                                pGo->SetRespawnTime(7*DAY);
                            if (Creature* pCreature = me->SummonCreature(37935 ,-529.348f , 2226.35f ,539.291f, 5.87753f, TEMPSUMMON_TIMED_DESPAWN, 7*DAY))
                                pCreature->SetVisible(true);
                            if (Creature* pCreature = me->SummonCreature(37936 ,-521.219f , 2233.37f ,539.292f, 5.41553f, TEMPSUMMON_TIMED_DESPAWN, 7*DAY))
                                pCreature->SetVisible(true);
                            if (Creature *peon1 = Unit::GetCreature(*me, _peon1GUID))
                                peon1->DespawnOrUnsummon();
                            if (Creature *peon2 = Unit::GetCreature(*me, _peon2GUID))
                                peon2->DespawnOrUnsummon();
                            Map* map = me->GetMap();
                            for (Map::PlayerList::const_iterator itr = map->GetPlayers().begin(); itr != map->GetPlayers().end(); ++itr)
                            {
                                if (Player* pPlayer = itr->getSource())
                                    pPlayer->CombatStop();
                            }
                            break;
                    }
                }
            }

        private:
            uint64 _hordeportal2GUID, _hordeportal1GUID;
            uint64 _peon2GUID, _peon1GUID;
            EventMap _events;
            InstanceScript* _instance;
            std::list<Creature*> _guardList;
        };

        bool OnGossipHello(Player* player, Creature* creature)
        {
            InstanceScript* instance = creature->GetInstanceScript();
            if (instance && !instance->IsDone(DATA_DEATHBRINGER_SAURFANG))
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "We are ready to go, High Overlord. The Lich King must fall!", 631, -ACTION_START_EVENT);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            }

            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
        {
            player->PlayerTalkClass->ClearMenus();
            player->CLOSE_GOSSIP_MENU();
            if (action == -ACTION_START_EVENT)
                creature->AI()->DoAction(ACTION_START_EVENT);

            return true;
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetIcecrownCitadelAI<npc_high_overlord_saurfangAI>(creature);
        }
};

class npc_muradin_bronzebeard_icc : public CreatureScript
{
    public:
        npc_muradin_bronzebeard_icc() : CreatureScript("npc_muradin_bronzebeard_icc") { }

        struct npc_muradin_bronzebeard_iccAI : public ScriptedAI
        {
            npc_muradin_bronzebeard_iccAI(Creature* creature) : ScriptedAI(creature)
            {
                _instance = me->GetInstanceScript();
            }

            void Reset()
            {
                _HighLordSaurfangGUID = 0;
                _KingVarianWrynnGUID = 0;
                _LadyJainaProudmooreGUID = 0;
                _peon1GUID = 0;
                _peon2GUID = 0;
                _stormwindportalGUID = 0;
                _allianceportal1GUID = 0;
                _allianceportal2GUID = 0;
                _zeppGUID = 0;
                _events.Reset();
            }


            void UpdateTransportMotionInMap(Transport* t)
            {
                Map* map = t->GetMap();

                for (Map::PlayerList::const_iterator itr = map->GetPlayers().begin(); itr != map->GetPlayers().end(); ++itr)
                {
                    if (Player* pPlayer = itr->getSource())
                    {
                        UpdateData transData(t->GetMapId());
                        t->BuildCreateUpdateBlockForPlayer(&transData, pPlayer);
                        WorldPacket packet;
                        transData.BuildPacket(&packet);
                        pPlayer->SendDirectMessage(&packet);
                    }
                }
            }

            Transport* CreateZeppelin()
            {
                if (Transport *t = sMapMgr->LoadTransportInMap(me->GetMap(), GO_SAURFRANG_ZEPLIN, 0))
                {
                    UpdateTransportMotionInMap(t);
                    t->LoadStaticPassengers();
                    return t;
                }
                return NULL;
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case ACTION_START_EVENT:
                    {
                        // Prevent crashes
                        if (_events.IsInPhase(PHASE_INTRO_A) || _events.IsInPhase(PHASE_INTRO_H))
                            return;

                        _events.SetPhase(PHASE_INTRO_A);
                        GetCreatureListWithEntryInGrid(_guardList, me, NPC_SE_SKYBREAKER_MARINE, 20.0f);
                        _guardList.sort(Trinity::ObjectDistanceOrderPred(me));
                        uint32 x = 1;
                        for (std::list<Creature*>::iterator itr = _guardList.begin(); itr != _guardList.end(); ++x, ++itr)
                            (*itr)->AI()->SetData(0, x);

                        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        Talk(SAY_INTRO_ALLIANCE_1);
                        _events.ScheduleEvent(EVENT_INTRO_ALLIANCE_4, 2500 + 17500 + 9500, 0, PHASE_INTRO_A);
                        _instance->HandleGameObject(_instance->GetData64(GO_SAURFANG_S_DOOR), true);
                        if (Creature* deathbringer = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_DEATHBRINGER_SAURFANG)))
                            deathbringer->AI()->DoAction(PHASE_INTRO_A);
                        break;
                    }
                    case ACTION_START_OUTRO:
                    {
                        me->RemoveAurasDueToSpell(SPELL_GRIP_OF_AGONY);
                        Talk(SAY_OUTRO_ALLIANCE_1);
                        if (Transport * zepp = CreateZeppelin())
                            _zeppGUID = zepp->GetGUID();
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_2, 4000);
                        me->SetDisableGravity(false);
                        me->Relocate(me->GetPositionX(), me->GetPositionY(), 539.2917f);
                        for (std::list<Creature*>::iterator itr = _guardList.begin(); itr != _guardList.end(); ++itr)
                        {
                            (*itr)->RemoveAurasDueToSpell(SPELL_GRIP_OF_AGONY);
                            (*itr)->SetDisableGravity(false);
                            (*itr)->Relocate((*itr)->GetPositionX(), (*itr)->GetPositionY(), 539.2917f);
                            (*itr)->MonsterMoveWithSpeed(me->GetPositionX(), me->GetPositionY(), 539.2917f, 0.0f);
                        }
                        break;
                    }
                    case ACTION_INTERRUPT_INTRO:
                        _events.Reset();
                        for (std::list<Creature*>::iterator itr = _guardList.begin(); itr != _guardList.end(); ++itr)
                            (*itr)->AI()->DoAction(ACTION_DESPAWN);
                        break;
                    case ACTION_KNEEL:
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_13, 1000);
                        break;
                    case ACTION_KING:
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_15, 1000);
                        break;
                    case ACTION_EXIT_1:
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_17, 1000);
                        break;
                }
            }

            void SpellHit(Unit* /*caster*/, SpellInfo const* spell)
            {
                if (spell->Id == SPELL_GRIP_OF_AGONY)
                {
                    me->SetDisableGravity(true);
                    me->GetMotionMaster()->MovePoint(POINT_CHOKE, chokePos[0]);
                }
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type == POINT_MOTION_TYPE && id == POINT_FIRST_STEP)
                {
                    me->SetWalk(false);
                    Talk(SAY_INTRO_ALLIANCE_4);
                    _events.ScheduleEvent(EVENT_INTRO_ALLIANCE_5, 5000, 0, PHASE_INTRO_A);
                    if (Creature* deathbringer = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_DEATHBRINGER_SAURFANG)))
                        deathbringer->AI()->DoAction(ACTION_CONTINUE_INTRO);
                }
                else if (type == WAYPOINT_MOTION_TYPE && id == POINT_EXIT)
                {
                    std::list<Creature*> guards;
                    GetCreatureListWithEntryInGrid(guards, me, NPC_ALLIANCE_COMMANDER, 50.0f);
                    for (std::list<Creature*>::iterator itr = guards.begin(); itr != guards.end(); ++itr)
                        (*itr)->DespawnOrUnsummon();
                    me->DespawnOrUnsummon();
                }

                switch (id)
                {
                    case POINT_STAND:
                        me->SetFacingTo(6.05f);
                        break;
                    case POINT_CORPSE_1:
                    {
                        Creature* muradin = ObjectAccessor::GetCreature(*me, me->GetCreatorGUID());
                        muradin->AI()->DoAction(ACTION_KNEEL);
                        break;
                    }
                    case POINT_KING:
                    {
                        Creature* muradin = ObjectAccessor::GetCreature(*me, me->GetCreatorGUID());
                        muradin->AI()->DoAction(ACTION_KING);
                        break;
                    }
                    case POINT_EXIT_1:
                    {
                        Creature* muradin = ObjectAccessor::GetCreature(*me, me->GetCreatorGUID());
                        if (Creature* deathbringer = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_DEATHBRINGER_SAURFANG)))
                            deathbringer->DespawnOrUnsummon();
                        muradin->AI()->DoAction(ACTION_EXIT_1);
                        me->DespawnOrUnsummon();
                        break;
                    }
                    case POINT_EXIT_2:
                    {
                        me->SetFacingTo(0.0f);
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_21_2, 1000);
                        break;
                    }
                }
            }

            void UpdateAI(uint32 const diff)
            {
                _events.Update(diff);
                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_INTRO_ALLIANCE_4:
                        me->SetWalk(true);
                        me->GetMotionMaster()->MovePoint(POINT_FIRST_STEP, firstStepPos.GetPositionX(), firstStepPos.GetPositionY(), firstStepPos.GetPositionZ());
                        break;
                    case EVENT_INTRO_ALLIANCE_5:
                        Talk(SAY_INTRO_ALLIANCE_5);
                        for (std::list<Creature*>::iterator itr = _guardList.begin(); itr != _guardList.end(); ++itr)
                            (*itr)->AI()->DoAction(ACTION_CHARGE);
                        me->GetMotionMaster()->MoveCharge(chargePos[0].GetPositionX(), chargePos[0].GetPositionY(), chargePos[0].GetPositionZ(), 8.5f, POINT_CHARGE);
                        break;
                    case EVENT_OUTRO_ALLIANCE_2:
                        Talk(SAY_OUTRO_ALLIANCE_2);
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_3, 9000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_3:
                        me->SetFacingTo(2.69f);
                        Talk(SAY_OUTRO_ALLIANCE_3);
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_4, 6000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_4:
                        me->GetMotionMaster()->MovePoint(0, -518.49f, 2227.77f, 539.29f);
                        Talk(SAY_OUTRO_ALLIANCE_4);
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_5, 12000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_5:
                        me->SetFacingTo(1.31f);
                        Talk(SAY_OUTRO_ALLIANCE_5);
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_6, 15000);
                        for (std::list<Creature*>::iterator itr = _guardList.begin(); itr != _guardList.end(); ++itr)
                            (*itr)->AI()->DoAction(ACTION_MOVE);
                        break;
                    case EVENT_OUTRO_ALLIANCE_6:
                        Talk(SAY_OUTRO_ALLIANCE_6);
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_7, 4000);
                        me->SummonCreature(37187, -519.17f, 2250.46f, 539.29f, 4.89f);
                        for (std::list<Creature*>::iterator itr = _guardList.begin(); itr != _guardList.end(); ++itr)
                            (*itr)->SetFacingTo(1.31f);
                        break;
                    case EVENT_OUTRO_ALLIANCE_7:
                        Talk(SAY_OUTRO_ALLIANCE_7);
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_8, 8000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_8:
                        if (Creature *HighLordSaurfang = Unit::GetCreature(*me, _HighLordSaurfangGUID))
                            HighLordSaurfang->AI()->Talk(SAY_OUTRO_ALLIANCE_8);
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_9, 10000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_9:
                        Talk(SAY_OUTRO_ALLIANCE_9);
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_10, 10000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_10:
                        if (GameObject *stormwindportal = me->SummonGameObject(195139, -532.01f, 2235.35f, 539.29f, 0.0f,0.0f,0.0f,0.0f,0.0f, 30000))
                        {
                            _stormwindportalGUID = stormwindportal->GetGUID();
                            stormwindportal->CastSpell(stormwindportal->ToUnit(), 52096); // wtf ??
                            stormwindportal->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                        }
                        Talk(SAY_OUTRO_ALLIANCE_10);
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_10_1, 2000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_10_1:
                        me->SummonCreature(NPC_SE_JAINA_PROUDMOORE, -529.64f, 2232.39f, 539.29f, 5.88f);
                        me->SummonCreature(NPC_SE_KING_VARIAN_WRYNN, -528.45f, 2235.44f, 539.29f, 5.91f);
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_11, 2000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_11:
                        if (Creature *KingVarianWrynn = Unit::GetCreature(*me, _KingVarianWrynnGUID))
                            KingVarianWrynn->AI()->Talk(SAY_OUTRO_ALLIANCE_11);
                        if (GameObject *stormwindportal = GameObject::GetGameObject(*me, _stormwindportalGUID))
                            stormwindportal->SetRespawnTime(30);
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_11_1, 5000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_11_1:
                        me->GetMotionMaster()->MovePoint(POINT_STAND, -531.14f, 2229.90f, 539.29f);
                        for (std::list<Creature*>::iterator itr = _guardList.begin(); itr != _guardList.end(); ++itr)
                            (*itr)->AI()->DoAction(ACTION_STAND);
                        if (Creature* deathbringer = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_DEATHBRINGER_SAURFANG)))
                            if (Creature *HighLordSaurfang = Unit::GetCreature(*me, _HighLordSaurfangGUID))
                            {
                                float x, y, z;
                                deathbringer->GetClosePoint(x, y, z, deathbringer->GetObjectSize());
                                HighLordSaurfang->GetMotionMaster()->MovePoint(POINT_CORPSE_1, x, y, z);
                                HighLordSaurfang->AI()->Talk(SAY_OUTRO_ALLIANCE_12);
                            }
                        break;
                    case EVENT_OUTRO_ALLIANCE_13:
                        if (Creature *HighLordSaurfang = Unit::GetCreature(*me, _HighLordSaurfangGUID))
                            HighLordSaurfang->AI()->Talk(SAY_OUTRO_ALLIANCE_13);
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_14, 8000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_14:
                        if (Creature* deathbringer = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_DEATHBRINGER_SAURFANG)))
                            if (Creature *HighLordSaurfang = Unit::GetCreature(*me, _HighLordSaurfangGUID))
                            {
                                Vehicle* vehicle(HighLordSaurfang->GetVehicleKit());
                                deathbringer->CastSpell(HighLordSaurfang, SPELL_RIDE_VEHICLE, true);
                                deathbringer->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                                deathbringer->setDeathState(ALIVE);
                            }
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_14_1, 2000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_14_1:
                        if (Creature *HighLordSaurfang = Unit::GetCreature(*me, _HighLordSaurfangGUID))
                        {
                            HighLordSaurfang->AI()->Talk(SAY_OUTRO_ALLIANCE_14);
                            HighLordSaurfang->GetMotionMaster()->MovePoint(POINT_KING, -525.41f, 2233.26f, 539.29f);
                        }
                        break;
                    case EVENT_OUTRO_ALLIANCE_15:
                        if (Creature *HighLordSaurfang = Unit::GetCreature(*me, _HighLordSaurfangGUID))
                        {
                            HighLordSaurfang->SetFacingTo(2.42f);
                            HighLordSaurfang->AI()->Talk(SAY_OUTRO_ALLIANCE_15);
                        }
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_16, 8000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_16:
                        if (Creature *KingVarianWrynn = Unit::GetCreature(*me, _KingVarianWrynnGUID))
                            KingVarianWrynn->AI()->Talk(SAY_OUTRO_ALLIANCE_16);
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_16_1, 18000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_16_1:
                        if (Creature *HighLordSaurfang = Unit::GetCreature(*me, _HighLordSaurfangGUID))
                            HighLordSaurfang->GetMotionMaster()->MovePoint(POINT_EXIT_1, -520.34f, 2249.83f, 539.29f);
                        break;
                    case EVENT_OUTRO_ALLIANCE_17:
                        if (Creature *LadyJainaProudmoore = Unit::GetCreature(*me, _LadyJainaProudmooreGUID))
                            LadyJainaProudmoore->AI()->Talk(SAY_OUTRO_ALLIANCE_17);
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_18, 4000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_18:
                        if (Creature *KingVarianWrynn = Unit::GetCreature(*me, _KingVarianWrynnGUID))
                            if (Creature *LadyJainaProudmoore = Unit::GetCreature(*me, _LadyJainaProudmooreGUID))
                            {
                                KingVarianWrynn->SetFacingToObject(LadyJainaProudmoore);
                                KingVarianWrynn->AI()->Talk(SAY_OUTRO_ALLIANCE_18);
                            }
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_19, 4000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_19:
                        if (Creature *KingVarianWrynn = Unit::GetCreature(*me, _KingVarianWrynnGUID))
                            if (Creature *LadyJainaProudmoore = Unit::GetCreature(*me, _LadyJainaProudmooreGUID))
                            {
                                LadyJainaProudmoore->SetFacingToObject(KingVarianWrynn);
                                LadyJainaProudmoore->AI()->Talk(SAY_OUTRO_ALLIANCE_19);
                            }
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_20, 9000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_20:
                        if (Creature *KingVarianWrynn = Unit::GetCreature(*me, _KingVarianWrynnGUID))
                        {
                            me->SetFacingToObject(KingVarianWrynn);
                            KingVarianWrynn->AI()->Talk(SAY_OUTRO_ALLIANCE_20);
                        }
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_21, 11000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_21:
                        Talk(SAY_OUTRO_ALLIANCE_21);
                        if (Creature *KingVarianWrynn = Unit::GetCreature(*me, _KingVarianWrynnGUID))
                            if (Creature *LadyJainaProudmoore = Unit::GetCreature(*me, _LadyJainaProudmooreGUID))
                            {
                                KingVarianWrynn->CastSpell(me, 52096, false);
                                LadyJainaProudmoore->CastSpell(me, 52096, false);
                            }
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_21_1, 1000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_21_1:
                        if (Creature *KingVarianWrynn = Unit::GetCreature(*me, _KingVarianWrynnGUID))
                            KingVarianWrynn->DespawnOrUnsummon();
                        if (Creature *LadyJainaProudmoore = Unit::GetCreature(*me, _LadyJainaProudmooreGUID))
                            LadyJainaProudmoore->DespawnOrUnsummon();
                        me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                        me->GetMotionMaster()->MovePoint(POINT_EXIT_2, -556.0f, 2211.42f, 539.37f);
                        for (std::list<Creature*>::iterator itr = _guardList.begin(); itr != _guardList.end(); ++itr)
                            (*itr)->AI()->DoAction(ACTION_EXIT_2);
                        break;
                    case EVENT_OUTRO_ALLIANCE_21_2:
                        if (GameObject * allianceportal1 = me->SummonGameObject(195701, -561.16f, 2219.30f, 539.28f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, DAY))
                        {
                            allianceportal1->SetGoState(GO_STATE_READY);
                            _allianceportal1GUID = allianceportal1->GetGUID();
                        }
                        if (GameObject * allianceportal2 = me->SummonGameObject(195701, -560.84f, 2202.57f, 539.28f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, DAY))
                        {
                            allianceportal2->SetGoState(GO_STATE_READY);
                            _allianceportal2GUID = allianceportal2->GetGUID();
                        }
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_21_3, 5000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_21_3:
                        if (GameObject *allianceportal1 = GameObject::GetGameObject(*me, _allianceportal1GUID))
                            allianceportal1->SetGoState(GO_STATE_ACTIVE);
                        if (GameObject *allianceportal2 = GameObject::GetGameObject(*me, _allianceportal2GUID))
                            allianceportal2->SetGoState(GO_STATE_ACTIVE);
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_21_4, 5000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_21_4:
                        if (Creature *peon1 = me->SummonCreature(20424, -561.16f, 2219.30f, 539.28f, 0.0f))
                        {
                            peon1->GetMotionMaster()->MovePoint(0, -538.82f, 2225.95f, 539.29f);
                            _peon1GUID = peon1->GetGUID();
                        }
                        if (Creature *peon2 = me->SummonCreature(20424, -560.84f, 2202.57f, 539.28f, 0.0f))
                        {
                            peon2->GetMotionMaster()->MovePoint(0, -534.05f, 2222.42f, 539.29f);
                            _peon2GUID = peon2->GetGUID();
                        }
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_21_5, 7000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_21_5:
                        if (Creature* peon1 = Unit::GetCreature(*me, _peon1GUID))
                            peon1->SetStandState(EMOTE_STATE_WORK);
                        if (Creature* peon2 = Unit::GetCreature(*me, _peon2GUID))
                            peon2->SetStandState(EMOTE_STATE_WORK);
                        _events.ScheduleEvent(EVENT_OUTRO_ALLIANCE_21_6, 5000);
                        break;
                    case EVENT_OUTRO_ALLIANCE_21_6:

                        if (GameObject* pGo = me->SummonGameObject(GO_SAURFANG_FORGE, -514.589f, 2244.63f, 539.291f, 5.04638f, 0, 0, 0, 0, 7*DAY))
                            pGo->SetRespawnTime(7*DAY);
                        if (GameObject* pGo = me->SummonGameObject(GO_SAURFANG_ANVIL, -518.931f, 2242.55f, 539.29f, 5.282f, 0, 0, 0, 0, 7*DAY))
                            pGo->SetRespawnTime(7*DAY);
                        if (GameObject* pGo = me->SummonGameObject(GO_SAURFANG_BONFIRE, -525.496f, 2227.39f, 539.291f, 2.18124f, 0, 0, 0, 0, 7*DAY))
                            pGo->SetRespawnTime(7*DAY);
                        if (GameObject* pGo = me->SummonGameObject(GO_SAURFANG_ALLIANCE_TENT1, -524.565f, 2236.97f, 539.29f, 2.23466f, 0, 0, 0, 0, 7*DAY))
                            pGo->SetRespawnTime(7*DAY);
                        if (GameObject* pGo = me->SummonGameObject(GO_SAURFANG_ALLIANCE_TENT2, -532.78f, 2228.43f, 539.29f, 2.48599f, 0, 0, 0, 0, 7*DAY))
                            pGo->SetRespawnTime(7*DAY);
                        if (Creature* pCreature = me->SummonCreature(37904 ,-529.532f , 2226.46f ,539.292f, 5.63936f, TEMPSUMMON_TIMED_DESPAWN, 7*DAY))
                            pCreature->SetVisible(true);
                        if (Creature* pCreature = me->SummonCreature(37903 ,-521.219f , 2233.37f ,539.292f, 5.41553f, TEMPSUMMON_TIMED_DESPAWN, 7*DAY))
                            pCreature->SetVisible(true);
                        if (Creature* peon1 = Unit::GetCreature(*me, _peon1GUID))
                            peon1->DespawnOrUnsummon();
                        if (Creature* peon2 = Unit::GetCreature(*me, _peon2GUID))
                            peon2->DespawnOrUnsummon();
                        Map* map = me->GetMap();
                        for (Map::PlayerList::const_iterator itr = map->GetPlayers().begin(); itr != map->GetPlayers().end(); ++itr)
                        {
                            if (Player* pPlayer = itr->getSource())
                                pPlayer->CombatStop();
                        }
                        break;
                    }
                }
            }

            void JustSummoned(Creature* summon)
            {
                if (summon->GetEntry() == NPC_SE_MURADIN_BRONZEBEARD)
                {
                    summon->UpdateEntry(37187);
                    summon->SetCreatorGUID(me->GetGUID());
                    summon->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
                    _HighLordSaurfangGUID = summon->GetGUID();
                    summon->GetMotionMaster()->MovePoint(0, -517.80f, 2236.91f, 539.29f);
                }
                if (summon->GetEntry() == NPC_SE_KING_VARIAN_WRYNN)
                {
                    summon->CastSpell(me, 52096,false);
                    _KingVarianWrynnGUID = summon->GetGUID();
                }
                if (summon->GetEntry() == NPC_SE_JAINA_PROUDMOORE)
                {
                    summon->CastSpell(me, 52096,false);
                    _LadyJainaProudmooreGUID = summon->GetGUID();
                }
            }

        private:
            uint64 _zeppGUID;
            uint64 _HighLordSaurfangGUID, _KingVarianWrynnGUID, _LadyJainaProudmooreGUID, _peon1GUID, _peon2GUID;
            uint64 _stormwindportalGUID, _allianceportal1GUID, _allianceportal2GUID;

            EventMap _events;
            InstanceScript* _instance;
            std::list<Creature*> _guardList;
        };

        bool OnGossipHello(Player* player, Creature* creature)
        {
            InstanceScript* instance = creature->GetInstanceScript();
            if (instance && !instance->IsDone(DATA_DEATHBRINGER_SAURFANG))
            {
                player->ADD_GOSSIP_ITEM(0, "Let it begin...", 631, -ACTION_START_EVENT + 1);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            }

            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
        {
            player->PlayerTalkClass->ClearMenus();
            player->CLOSE_GOSSIP_MENU();
            if (action == -ACTION_START_EVENT + 1)
                creature->AI()->DoAction(ACTION_START_EVENT);

            return true;
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetIcecrownCitadelAI<npc_muradin_bronzebeard_iccAI>(creature);
        }
};

class npc_saurfang_event : public CreatureScript
{
    public:
        npc_saurfang_event() : CreatureScript("npc_saurfang_event") { }

        struct npc_saurfang_eventAI : public ScriptedAI
        {
            npc_saurfang_eventAI(Creature* creature) : ScriptedAI(creature)
            {
                _index = 0;
            }

            void SetData(uint32 type, uint32 data)
            {
                ASSERT(!type && data && data < 6);
                _index = data;
            }

            void SpellHit(Unit* /*caster*/, SpellInfo const* spell)
            {
                if (spell->Id == SPELL_GRIP_OF_AGONY)
                {
                    me->SetDisableGravity(true);
                    me->GetMotionMaster()->MovePoint(POINT_CHOKE, chokePos[_index]);
                }
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_CHARGE && _index)
                    me->GetMotionMaster()->MoveCharge(chargePos[_index].GetPositionX(), chargePos[_index].GetPositionY(), chargePos[_index].GetPositionZ(), 13.0f, POINT_CHARGE);
                else if (action == ACTION_DESPAWN)
                    me->DespawnOrUnsummon();
            }

        private:
            uint32 _index;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetIcecrownCitadelAI<npc_saurfang_eventAI>(creature);
        }
};

class npc_blood_beast : public CreatureScript
{
    public:
        npc_blood_beast() : CreatureScript("npc_blood_beast") { }

        struct npc_blood_beastAI : public ScriptedAI
        {
            npc_blood_beastAI(Creature* creature) : ScriptedAI(creature), _instance(creature->GetInstanceScript())
            {
            }

            void Reset()
            {
                if (IsHeroic())
                    DoCast(me, SPELL_SCENT_OF_BLOOD);

                DoCast(me, SPELL_BLOOD_LINK_BEAST, true);
                DoCast(me, SPELL_RESISTANT_SKIN, true);

                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);

                me->SetReactState(REACT_PASSIVE);
                AttackTimer = 3000;
                isReady = true;
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->HasAura(SPELL_MARK_OF_THE_FALLEN_CHAMPION))
                    if (Creature* saurfang = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_DEATHBRINGER_SAURFANG)))
                        saurfang->CastSpell(saurfang, (IsHeroic() ? SPELL_MARK_OF_THE_FALLEN_CHAMPION_HEAL_H : SPELL_MARK_OF_THE_FALLEN_CHAMPION_HEAL), true);
            }

            Unit* SelectEnemyCaster()
            {
                Map::PlayerList const &tList = me->GetMap()->GetPlayers();
                Map::PlayerList::const_iterator iter;

                Unit* target;
                std::list<Unit *> newListTarget;
                for (iter = tList.begin(); iter != tList.end(); ++iter)
                {
                    target = iter->getSource();
                    if (target && target->getPowerType() == POWER_MANA && target->isAlive() && me->GetDistance2d(target->GetPositionX(), target->GetPositionY()) > 12.0f)
                        newListTarget.push_back(target);
                }
                if (newListTarget.empty() || newListTarget.size() <= 1)
                    for (iter = tList.begin(); iter!=tList.end(); ++iter)
                    {
                        target = iter->getSource();
                        if (target && target->getPowerType() == POWER_MANA && target->isAlive())
                            newListTarget.push_back(target);
                    }
                if (newListTarget.size() > 0)
                {
                    std::list<Unit*>::iterator itr = newListTarget.begin();
                    if (!newListTarget.empty())
                    {
                        std::advance(itr, urand(0, newListTarget.size() - 1));
                        return *itr;
                    }
                }
                return SelectTarget(SELECT_TARGET_RANDOM);
            }


            void UpdateAI(const uint32 diff)
            {
                if (isReady)
                {
                    if (AttackTimer <= diff)
                    {
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->SetInCombatWithZone();
                        if (Unit *target = SelectEnemyCaster())
                        {
                            me->AddThreat(target, 100000.0f);
                            me->GetMotionMaster()->MoveChase(target);
                            me->AI()->AttackStart(target);
                        }
                        isReady = false;
                    }
                    else AttackTimer -= diff;
                    return;
                }

                if (me->isAttackReady())
                {
                    //If we are within range melee the target
                    if (me->IsWithinMeleeRange(me->getVictim()))
                    {
                        if (Creature* saurfang = me->FindNearestCreature(NPC_DEATHBRINGER_SAURFANG, 100.0f, true))
                        {
                            if (saurfang->isAlive())
                                if (saurfang->GetPower(POWER_ENERGY) != saurfang->GetMaxPower(POWER_ENERGY))
                                {
                                    ActPower = 0;
                                    ExtraPower = 0;
                                    NewPower = 0;
                                    ActPower = saurfang->GetPower(POWER_ENERGY);
                                    ExtraPower = saurfang->GetMaxPower(POWER_ENERGY) / 100 * 3;
                                    NewPower = ActPower + ExtraPower;
                                    if (NewPower >= 100)
                                        NewPower = 100;
                                    saurfang->SetPower(POWER_ENERGY, NewPower);
                                }
                        }
                        me->AttackerStateUpdate(me->getVictim());
                        me->resetAttackTimer();
                    }
                }
            }
        private:
            uint32 ActPower;
            uint32 ExtraPower;
            uint32 NewPower;
            uint32 AttackTimer;
            InstanceScript* _instance;
            bool isReady;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetIcecrownCitadelAI<npc_blood_beastAI>(creature);
        }
};

class spell_deathbringer_blood_link : public SpellScriptLoader
{
    public:
        spell_deathbringer_blood_link() : SpellScriptLoader("spell_deathbringer_blood_link") { }

        class spell_deathbringer_blood_link_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_deathbringer_blood_link_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_BLOOD_LINK_POWER))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_BLOOD_POWER))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                GetHitUnit()->CastCustomSpell(SPELL_BLOOD_LINK_POWER, SPELLVALUE_BASE_POINT0, GetEffectValue(), GetHitUnit(), true);
                if (Aura* bloodPower = GetHitUnit()->GetAura(SPELL_BLOOD_POWER))
                    bloodPower->RecalculateAmountOfEffects();
                PreventHitDefaultEffect(EFFECT_0);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_deathbringer_blood_link_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_deathbringer_blood_link_SpellScript();
        }
};

class spell_deathbringer_blood_link_aura : public SpellScriptLoader
{
    public:
        spell_deathbringer_blood_link_aura() : SpellScriptLoader("spell_deathbringer_blood_link_aura") { }

        class spell_deathbringer_blood_link_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_deathbringer_blood_link_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MARK_OF_THE_FALLEN_CHAMPION))
                    return false;
                return true;
            }

            void HandlePeriodicTick(AuraEffect const* /*aurEff*/)
            {
                PreventDefaultAction();
                if (GetUnitOwner()->getPowerType() == POWER_ENERGY && GetUnitOwner()->GetPower(POWER_ENERGY) == GetUnitOwner()->GetMaxPower(POWER_ENERGY))
                if (Creature* saurfang = GetUnitOwner()->ToCreature())
                    saurfang->AI()->DoAction(ACTION_MARK_OF_THE_FALLEN_CHAMPION);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_deathbringer_blood_link_AuraScript::HandlePeriodicTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_deathbringer_blood_link_AuraScript();
        }
};

class spell_deathbringer_blood_power : public SpellScriptLoader
{
    public:
        spell_deathbringer_blood_power() : SpellScriptLoader("spell_deathbringer_blood_power") { }

        class spell_deathbringer_blood_power_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_deathbringer_blood_power_SpellScript);

            void ModAuraValue()
            {
                if (Aura* aura = GetHitAura())
                    aura->RecalculateAmountOfEffects();
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_deathbringer_blood_power_SpellScript::ModAuraValue);
            }
        };

        class spell_deathbringer_blood_power_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_deathbringer_blood_power_AuraScript);

            void RecalculateHook(AuraEffect const* /*aurEffect*/, int32& amount, bool& canBeRecalculated)
            {
                amount = int32(GetUnitOwner()->GetPower(POWER_ENERGY));
                canBeRecalculated = true;
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_deathbringer_blood_power_AuraScript::RecalculateHook, EFFECT_0, SPELL_AURA_MOD_SCALE);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_deathbringer_blood_power_AuraScript::RecalculateHook, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
            }

            bool Load()
            {
                if (GetUnitOwner()->getPowerType() != POWER_ENERGY)
                    return false;
                return true;
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_deathbringer_blood_power_SpellScript();
        }

        AuraScript* GetAuraScript() const
        {
            return new spell_deathbringer_blood_power_AuraScript();
        }
};

class spell_deathbringer_rune_of_blood : public SpellScriptLoader
{
    public:
        spell_deathbringer_rune_of_blood() : SpellScriptLoader("spell_deathbringer_rune_of_blood") { }

        class spell_deathbringer_rune_of_blood_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_deathbringer_rune_of_blood_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_BLOOD_LINK_DUMMY))
                    return false;
                return true;
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);  // make this the default handler
                if (GetCaster()->GetPower(POWER_ENERGY) != GetCaster()->GetMaxPower(POWER_ENERGY))
                    GetHitUnit()->CastCustomSpell(SPELL_BLOOD_LINK_DUMMY, SPELLVALUE_BASE_POINT0, 1, GetCaster(), true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_deathbringer_rune_of_blood_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_deathbringer_rune_of_blood_SpellScript();
        }
};

class spell_deathbringer_blood_nova : public SpellScriptLoader
{
    public:
        spell_deathbringer_blood_nova() : SpellScriptLoader("spell_deathbringer_blood_nova") { }

        class spell_deathbringer_blood_nova_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_deathbringer_blood_nova_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_BLOOD_LINK_DUMMY))
                    return false;
                return true;
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);  // make this the default handler
                if (GetCaster()->GetPower(POWER_ENERGY) != GetCaster()->GetMaxPower(POWER_ENERGY))
                    GetHitUnit()->CastCustomSpell(SPELL_BLOOD_LINK_DUMMY, SPELLVALUE_BASE_POINT0, 2, GetCaster(), true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_deathbringer_blood_nova_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_deathbringer_blood_nova_SpellScript();
        }
};

class spell_deathbringer_blood_nova_targeting : public SpellScriptLoader
{
    public:
        spell_deathbringer_blood_nova_targeting() : SpellScriptLoader("spell_deathbringer_blood_nova_targeting") { }

        class spell_deathbringer_blood_nova_targeting_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_deathbringer_blood_nova_targeting_SpellScript);

            bool Load()
            {
                // initialize variable
                target = NULL;
                return true;
            }

            void FilterTargetsInitial(std::list<WorldObject*>& targets)
            {
                if (targets.empty())
                    return;
                // select one random target, with preference of ranged targets
                uint32 targetsAtRange = 0;
                uint32 const minTargets = uint32(GetCaster()->GetMap()->GetSpawnMode() & 1 ? 10 : 4);
                targets.sort(Trinity::ObjectDistanceOrderPred(GetCaster(), false));

                // get target count at range
                for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end(); ++itr, ++targetsAtRange)
                if ((*itr)->GetDistance(GetCaster()) < 12.0f)
                    break;

                // set the upper cap
                if (targetsAtRange < minTargets)
                    targetsAtRange = std::min<uint32>(targets.size() - 1, minTargets);

                std::list<WorldObject*>::const_iterator itr = targets.begin();
                std::advance(itr, urand(0, targetsAtRange));
                target = *itr;
                targets.clear();
                targets.push_back(target);
            }

            // use the same target for first and second effect
            void FilterTargetsSubsequent(std::list<WorldObject*>& unitList)
            {
                if (!target)
                    return;

                unitList.clear();
                unitList.push_back(target);
            }

            void HandleForceCast(SpellEffIndex /*effIndex*/)
            {
                GetCaster()->CastSpell(GetHitUnit(), uint32(GetEffectValue()), TRIGGERED_FULL_MASK);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_deathbringer_blood_nova_targeting_SpellScript::FilterTargetsInitial, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_deathbringer_blood_nova_targeting_SpellScript::FilterTargetsSubsequent, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_deathbringer_blood_nova_targeting_SpellScript::HandleForceCast, EFFECT_0, SPELL_EFFECT_FORCE_CAST);
            }

            WorldObject* target;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_deathbringer_blood_nova_targeting_SpellScript();
        }
};

class spell_deathbringer_boiling_blood : public SpellScriptLoader
{
    public:
        spell_deathbringer_boiling_blood() : SpellScriptLoader("spell_deathbringer_boiling_blood") { }

        class spell_deathbringer_boiling_blood_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_deathbringer_boiling_blood_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_UNIT;
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targets.remove(GetCaster()->getVictim());
                if (targets.empty())
                    return;

                WorldObject* target = Trinity::Containers::SelectRandomContainerElement(targets);
                targets.clear();
                targets.push_back(target);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_deathbringer_boiling_blood_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_deathbringer_boiling_blood_SpellScript();
        }
};

class spell_deathbringer_remove_marks : public SpellScriptLoader
{
    public:
        spell_deathbringer_remove_marks() : SpellScriptLoader("spell_deathbringer_remove_marks") { }

        class spell_deathbringer_remove_marks_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_deathbringer_remove_marks_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                GetHitUnit()->RemoveAurasDueToSpell(uint32(GetEffectValue()));
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_deathbringer_remove_marks_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_deathbringer_remove_marks_SpellScript();
        }
};

class spell_deathbringer_mark_of_the_darkfallen : public SpellScriptLoader
{
    public:
        spell_deathbringer_mark_of_the_darkfallen() : SpellScriptLoader("spell_deathbringer_mark_of_the_darkfallen") { }

        class spell_deathbringer_mark_of_the_darkfallen_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_deathbringer_mark_of_the_darkfallen_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targets.remove_if(Trinity::UnitAuraCheck(true, 72293));
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_deathbringer_mark_of_the_darkfallen_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_deathbringer_mark_of_the_darkfallen_SpellScript();
        }
};

class achievement_ive_gone_and_made_a_mess : public AchievementCriteriaScript
{
    public:
        achievement_ive_gone_and_made_a_mess() : AchievementCriteriaScript("achievement_ive_gone_and_made_a_mess") { }

        bool OnCheck(Player* /*source*/, Unit* target)
        {
            if (target)
                if (Creature* saurfang = target->ToCreature())
                    if (saurfang->AI()->GetData(DATA_MADE_A_MESS))
                        return true;

            return false;
        }
};

void AddSC_boss_deathbringer_saurfang()
{
    new boss_deathbringer_saurfang();
    new npc_high_overlord_saurfang_icc();
    new npc_muradin_bronzebeard_icc();
    new npc_saurfang_event();
    new npc_blood_beast();
    new spell_deathbringer_blood_link();
    new spell_deathbringer_blood_link_aura();
    new spell_deathbringer_blood_power();
    new spell_deathbringer_rune_of_blood();
    new spell_deathbringer_blood_nova();
    new spell_deathbringer_blood_nova_targeting();
    new spell_deathbringer_boiling_blood();
    new spell_deathbringer_remove_marks();
    new achievement_ive_gone_and_made_a_mess();
}
