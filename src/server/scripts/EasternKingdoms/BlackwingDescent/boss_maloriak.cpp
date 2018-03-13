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

#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "blackwing_descent.h"

enum Spells
{
    SPELL_BERSERK                   = 26662,
    SPELL_ARCANE_STORM              = 77896,
    SPELL_RELEASE_ABERRATIONS       = 77569,
    // Red Vial
    SPELL_THROW_RED                 = 77925,
    SPELL_DRINK_RED_VIAL            = 88699,
    SPELL_AURA_FIRE_IMBUED          = 78896,
    SPELL_CONSUMING_FLAMES          = 77786,
    SPELL_SCORCHING_BLAST           = 77679,
    // Blue Vial
    SPELL_THROW_BLUE                = 77932,
    SPELL_DRINK_BLUE_VIAL           = 88700,
    SPELL_AURA_FROST_IMBUED         = 78895,
    SPELL_BITING_CHILL              = 77760,
    SPELL_FLASH_FREEZE              = 77699,
    SPELL_FLASH_FREEZE_TARGETING    = 97693,
    SPELL_FLASH_FREEZE_SUMMON       = 77711,
    SPELL_FLASH_FREEZE_SHATTER      = 77715,
    SPELL_FLASH_FREEZE_VIS          = 77712,
    // Green Vial
    SPELL_THROW_GREEN               = 77937,
    SPELL_REMEDY                    = 77912,
    SPELL_DEBILITATING_SLIME_VIS    = 77602,
    SPELL_DEBILITATING_SLIME_DMG    = 77615,
    SPELL_DEBILITATING_SLIME_DMG_2  = 92910,
    SPELL_DEBILITATING_SLIME_KNK    = 77948,
    // 25%
    SPELL_DRINK_ALL_VIAL            = 95662,
    SPELL_RELEASE_ALL               = 77991,
    SPELL_MAGMA_JETS                = 78194,
    SPELL_MAGMA_JETS_EFF            = 78095,
    SPELL_MAGMA_JETS_PERIODIC       = 93018,
    SPELL_ABSOLUTE_ZERO             = 78223,
    SPELL_ACID_NOVA                 = 78225,

    SPELL_ABSOLUTE_ZERO_CAST        = 78201,
    SPELL_ABSOLUTE_ZERO_PERIODIC    = 78206,
    SPELL_ABSOLUTE_ZERO_DMG         = 78208,
    SPELL_FIXATE                    = 78618,
    SPELL_GROWTH_CATALYST           = 77987,
    SPELL_REND                      = 78034,

    SPELL_BOSS_HITTN_YA             = 91334, // idk for what the spell is ...
    SPELL_DROWNED_STATE             = 77564,

    //heroic
    SPELL_TELEPORTATION_VISUAL = 41232,
    SPELL_NEFARIUS_AURA = 78494,
    SPELL_THROW_BLACK = 92837,
    SPELL_THROW_BLACK_1 = 92831,

    SPELL_ENGULFIND_DARKESS = 92754,
    SPELL_ENGULFIND_DARKESS_TRIGGER = 92787,
    // Black Vial
    SPELL_DRINK_BLACK_VIAL          = 92828,
    SPELL_AURA_SHADOW_IMBUED        = 92716,

    SPELL_VIL_RATA_MALORIAK = 92720, // casted on mloriak by maloriak 5 secondes

    SPELL_VIL_RATA_MISSILE = 92724, // trigger SPELL_VIL_RATA_SUMMON dest x y z
    SPELL_VIL_SWILL_PRE_EFFECT_SUMMON = 92741, // summon 49812

    SPELL_VIL_RATA_AURA = 92737, // npc 49812 on himself // trigger SPELL_VIL_SWILL_PRE_EFFECT_SUMMON
    SPELL_VIL_RATA_SUMMON = 92732, // summon 49811
    //vil rata
    SPELL_SOMBRE_VASE = 92929, // trigger SPELL_SOMBRE_VASE_TRIGGER 11:18:13.406 11:18:19.422 11:18:25.437
    SPELL_SOMBRE_VASE_TRIGGER = 92930,

    SPELL_PLAYERS_TITLE = 89798, // killing x adds in x time
};

enum Entities
{
    NPC_ABERRATION                  = 41440,
    NPC_FLASH_FREEZE                = 41576,
    NPC_CAULDRON_STALKER            = 41505,
    NPC_MAGMA_JETS_CONTROLLER       = 50030,
    NPC_PRIME_SUBJECT               = 41841,
    NPC_MAGMA_JET                   = 41901,
    NPC_RELEASE_TRIGGER             = 41377,
    NPC_SWILL_PRE_EFFECT            = 49812,
    NPC_VIL_RATA                    = 49811,
    GO_CAULDRON                     = 203306,
    GO_LARGE_CHAMBER                = 206705,
    GO_SMALL_CHAMBER                = 206704
};

enum Events
{
    EVENT_ARCANE_STORM              = 1,
    EVENT_RELEASE_ABERRATIONS,
    EVENT_CONSUMING_FLAMES,
    EVENT_SCORCHING_BLAST,
    EVENT_BITING_CHILL,
    EVENT_FLASH_FREEZE,
    EVENT_REMEDY,
    EVENT_MAGMA_JETS,
    EVENT_ABSOLUTE_ZERO,
    EVENT_ACID_NOVA,
    EVENT_NEXT_VIAL,
    EVENT_RESUME_COMBAT,
    EVENT_ENGULFIND_DARKESS,
    EVENT_VIL_RATA,
};

enum EventGroups
{
    EVENT_GROUP_RED             = 1,
    EVENT_GROUP_BLUE,
    EVENT_GROUP_BLACK
};

enum Misc
{
    MAX_ABERRATIONS             = 18,
    GOBJ_ANIM_RED               = 0,
    GOBJ_ANIM_BLUE,
    GOBJ_ANIM_GREEN,
    GOBJ_ANIM_BLACK,
    POINT_CAULDRON              = 1,
    POINT_RELEASE               = 2,
    ACTION_RELEASE_CASTED       = 3
};

enum Phases
{
    PHASE_NORMAL,
    PHASE_RED,
    PHASE_BLUE,
    PHASE_GREEN,
    PHASE_BLACK,
    PHASE_25
};

enum Texts
{
    SAY_AGGRO          = 0,
    SAY_SLAY           = 1,
    SAY_SLAY_2         = 2,
    SAY_RED_PHASE      = 3,
    SAY_BLUE_PHASE     = 4,
    SAY_GREEN_PHASE    = 5,
    SAY_25             = 6,
    SAY_DEATH          = 7,
    EMOTE_BLUE_BOTTLE  = 8,
    EMOTE_RED_BOTTLE   = 9,
    EMOTE_GREEN_BOTTLE = 10
};

static const Position cauldronPos = {-106.330498f, -470.160492f, 73.454094f, 0.0f};

static const Position aberrationSpawn [18] =
{
    {-61.2656f, -431.288f, 83.4554f, 2.70526f},
    {-63.2465f, -437.377f, 103.945f, 2.68781f},
    {-69.1615f, -417.741f, 94.9475f, 3.12414f},
    {-149.154f, -438.099f, 94.9475f, 5.67232f},
    {-69.0035f, -417.688f, 85.2381f, 3.10669f},
    {-150.337f, -432.331f, 82.4068f, 2.01323f},
    {-148.575f, -438.087f, 104.458f, 5.68977f},
    {-143.809f, -418.507f, 103.945f, 5.11381f},
    {-151.096f, -426.809f, 83.1099f, 0.174533f},
    {-143.931f, -418.472f, 94.9965f, 5.20108f},
    {-144.153f, -418.741f, 85.5198f, 5.58505f},
    {-149.401f, -421.208f, 82.8674f, 4.7822f},
    {-148.835f, -438.078f, 85.2381f, 5.65487f},
    {-61.3299f, -425.25f, 83.6979f, 3.80482f},
    {-69.3472f, -417.59f, 104.458f, 3.14159f},
    {-63.1927f, -437.582f, 94.9965f, 2.77507f},
    {-63.2135f, -437.502f, 85.5198f, 3.15905f},
    {-64.0729f, -420.663f, 83.6404f, 3.42085f}
};

static const Position primeSubjectSpawn [2] =
{
    {-145.46f, -427.517f, 97.1776f, 0.610865f},
    {-66.408f, -426.899f, 97.1776f, 4.18879f}
};

class boss_maloriak : public CreatureScript
{
    public:
        boss_maloriak() : CreatureScript("boss_maloriak") {}

        struct boss_maloriakAI : public BossAI
        {
            boss_maloriakAI(Creature* creature) : BossAI(creature, DATA_MALORIAK)
            {
                nefGUID = 0;
            }

            void Reset()
            {
                _Reset();
                resetChambers();
                bluePhase = urand(0, 1);
                phase = PHASE_NORMAL;
                moving = false;
                releaseCastCounter = 0;
                enrageTimer = (IsHeroic() ? 720000 : 420000);
                me->SetReactState(REACT_AGGRESSIVE);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FLASH_FREEZE);
                if (Creature* nefarian = me->GetCreature(*me, nefGUID))
                {
                    nefarian->CastSpell(nefarian, SPELL_TELEPORTATION_VISUAL, true);
                    nefarian->DespawnOrUnsummon(1000);
                    nefGUID = 0;
                }
                _engDar = 0;
                _orientation = 0.0f;
                _vilRata = 0;
            }

            void DamageTaken(Unit* /*done_by*/, uint32 &damage)
            {
                if (me->HealthBelowPctDamaged(25, damage) && phase != PHASE_25)
                {
                    phase = PHASE_25;
                    Talk(SAY_25);
                    events.Reset();
                    moving = true;
                    me->SetReactState(REACT_PASSIVE);
                    me->SetUInt64Value(UNIT_FIELD_TARGET, 0);
                    me->GetMotionMaster()->MovePoint(POINT_CAULDRON, cauldronPos);
                }
            }

            void JustDied(Unit* /*killer*/)
            {
                if (Creature* nefarian = me->GetCreature(*me, nefGUID))
                {
                    nefarian->CastSpell(nefarian, SPELL_TELEPORTATION_VISUAL, true);
                    nefarian->DespawnOrUnsummon(1000);
                }
                _JustDied();
                Talk(SAY_DEATH);
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(urand(SAY_SLAY, SAY_SLAY_2));
            }

            void JustSummoned(Creature* summon)
            {
                switch (summon->GetEntry())
                {
                    case NPC_NEFARIUS_MALORIAK:
                        summon->CastSpell(summon, SPELL_TELEPORTATION_VISUAL, true);
                        summon->CastSpell(summon, SPELL_NEFARIUS_AURA, true);
                        summon->SetHover(true);
                        nefGUID = summon->GetGUID();
                        break;
                    case NPC_ABERRATION:
                    case NPC_PRIME_SUBJECT:
                         summon->CastSpell(summon, SPELL_DROWNED_STATE);
                         break;
                    case NPC_MAGMA_JETS_CONTROLLER:
                        summon->SetReactState(REACT_PASSIVE);
                        summon->SetOrientation(_orientation);
                        summon->CastSpell(summon, SPELL_MAGMA_JETS_PERIODIC, true);
                        summon->DespawnOrUnsummon(5000);
                        break;
                    case NPC_SWILL_PRE_EFFECT:
                        summon->CastSpell(summon, SPELL_VIL_RATA_AURA, true);
                        summon->CastWithDelay(2000, summon, SPELL_VIL_RATA_SUMMON, false);
                        summon->DespawnOrUnsummon(3000);
                        _vilRata++;
                        break;
                    default:
                        break;
                }
                BossAI::JustSummoned(summon);
            }

            void SummonedCreatureDies(Creature* summon, Unit* killer)
            {
                switch (summon->GetEntry())
                {
                    case NPC_ABERRATION:
                    {
                        instance->DoStartTimedAchievement(ACHIEVEMENT_TIMED_TYPE_CREATURE, 41440);
                        instance->DoUpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, NPC_ABERRATION, 1, summon);
                        break;
                    }
                    case NPC_VIL_RATA:
                    {
                        if (_vilRata > 0)
                            _vilRata--;
                        if (_vilRata == 0 && phase == PHASE_BLACK)
                        {
                            me->RemoveAurasDueToSpell(SPELL_AURA_SHADOW_IMBUED);
                            events.CancelEventGroup(EVENT_GROUP_BLACK);
                            events.CancelEvent(EVENT_NEXT_VIAL);
                            events.ScheduleEvent(EVENT_NEXT_VIAL, 1000);
                        }
                        break;
                    }
                }

            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(SAY_AGGRO);
                events.ScheduleEvent(EVENT_ARCANE_STORM, 15000);
                events.ScheduleEvent(EVENT_NEXT_VIAL, 20000);
                events.ScheduleEvent(EVENT_REMEDY, 18000);
                if (IsHeroic())
                    me->SummonCreature(NPC_NEFARIUS_MALORIAK, -105.9514f, -494.0278f, 89.33157f, 1.605703f);
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type == POINT_MOTION_TYPE && id == POINT_CAULDRON)
                {
                    switch (phase)
                    {
                        case PHASE_BLUE:
                            DoCast(me, SPELL_DRINK_BLUE_VIAL);
                            me->AddAura(SPELL_AURA_FROST_IMBUED, me);
                            if (GameObject* go = me->FindNearestGameObject(GO_CAULDRON, 20.0f))
                                go->SendCustomAnim(GOBJ_ANIM_BLUE);
                            break;
                        case PHASE_RED:
                            DoCast(me, SPELL_DRINK_RED_VIAL);
                            me->AddAura(SPELL_AURA_FIRE_IMBUED, me);
                            if (GameObject* go = me->FindNearestGameObject(GO_CAULDRON, 20.0f))
                                go->SendCustomAnim(GOBJ_ANIM_RED);
                            break;
                        case PHASE_GREEN:
                            if (Creature* stalker = me->FindNearestCreature(NPC_CAULDRON_STALKER, 30.0f))
                            {
                                stalker->CastSpell((Unit*)NULL, SPELL_DEBILITATING_SLIME_DMG, true);
                                stalker->CastSpell(stalker, SPELL_DEBILITATING_SLIME_VIS, true);
                                stalker->CastSpell((Unit*)NULL, SPELL_DEBILITATING_SLIME_KNK, true);
                            }
                            if (GameObject* go = me->FindNearestGameObject(GO_CAULDRON, 20.0f))
                                go->SendCustomAnim(GOBJ_ANIM_GREEN);
                            break;
                        case PHASE_BLACK:
                            DoCast(me, SPELL_DRINK_BLACK_VIAL);
                            me->AddAura(SPELL_AURA_SHADOW_IMBUED, me);
                            if (GameObject* go = me->FindNearestGameObject(GO_CAULDRON, 20.0f))
                                go->SendCustomAnim(GOBJ_ANIM_BLACK);
                            break;
                        case PHASE_25:
                            DoCast(me, SPELL_DRINK_ALL_VIAL);
                            events.ScheduleEvent(EVENT_ACID_NOVA, 8000);
                            events.ScheduleEvent(EVENT_ABSOLUTE_ZERO, 10000);
                            events.ScheduleEvent(EVENT_MAGMA_JETS, 15000);
                            break;
                    }
                    moving = false;
                    events.DelayEvents(4000);
                    events.ScheduleEvent(EVENT_RESUME_COMBAT, 1000);
                }
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_RELEASE_CASTED)
                    releaseCastCounter++;
            }

            void resetChambers()
            {
                std::list<GameObject*> chamberList;
                me->GetGameObjectListWithEntryInGrid(chamberList, GO_LARGE_CHAMBER, 200.0f);
                me->GetGameObjectListWithEntryInGrid(chamberList, GO_SMALL_CHAMBER, 200.0f);
                if (!chamberList.empty())
                {
                    for (std::list<GameObject*>::const_iterator itr = chamberList.begin(); itr != chamberList.end(); ++itr)
                        (*itr)->SetGoState(GO_STATE_READY);
                }

                for (uint8 i = 0; i < MAX_ABERRATIONS; ++i)
                    if (Creature* aberration = me->SummonCreature(NPC_ABERRATION, aberrationSpawn[i]))
                        aberration->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

                if (Creature* prime = me->SummonCreature(NPC_PRIME_SUBJECT, primeSubjectSpawn[0]))
                    prime->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

                if (Creature* prime = me->SummonCreature(NPC_PRIME_SUBJECT, primeSubjectSpawn[1]))
                    prime->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || moving)
                    return;

                if (enrageTimer <= diff)
                {
                    DoCast(me, SPELL_BERSERK, true);
                    enrageTimer = 420000;
                }
                else
                    enrageTimer -= diff;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_ARCANE_STORM:
                        if (phase != PHASE_BLACK)
                            DoCast(SPELL_ARCANE_STORM);
                        if (releaseCastCounter < 6)
                            events.ScheduleEvent(EVENT_RELEASE_ABERRATIONS, 5000);
                        events.ScheduleEvent(EVENT_ARCANE_STORM, 15000);
                        break;
                    case EVENT_RELEASE_ABERRATIONS:
                        if (phase != PHASE_GREEN && phase != PHASE_BLACK)
                            DoCast(SPELL_RELEASE_ABERRATIONS);
                        break;
                    case EVENT_CONSUMING_FLAMES:
                        DoCastRandom(SPELL_CONSUMING_FLAMES, 60.0f, true);
                        events.ScheduleEvent(EVENT_CONSUMING_FLAMES, 15000, EVENT_GROUP_RED);
                        break;
                    case EVENT_SCORCHING_BLAST:
                        DoCastVictim(SPELL_SCORCHING_BLAST);
                        events.ScheduleEvent(EVENT_SCORCHING_BLAST, 10000, EVENT_GROUP_RED);
                        break;
                    case EVENT_BITING_CHILL:
                        DoCastRandom(SPELL_BITING_CHILL, 3.0f, true);
                        events.ScheduleEvent(EVENT_BITING_CHILL, 15000, EVENT_GROUP_BLUE);
                        break;
                    case EVENT_FLASH_FREEZE:
                        DoCast(SPELL_FLASH_FREEZE_TARGETING);
                        events.ScheduleEvent(EVENT_FLASH_FREEZE, 15000, EVENT_GROUP_BLUE);
                        break;
                    case EVENT_REMEDY:
                        if (phase != PHASE_BLACK)
                            DoCast(me, SPELL_REMEDY, true);
                        events.ScheduleEvent(EVENT_REMEDY, 15000);
                        break;
                    case EVENT_NEXT_VIAL:
                        switch (phase)
                        {
                            case PHASE_NORMAL:
                            {
                                if (!IsHeroic())
                                    phase = bluePhase ? PHASE_BLUE : PHASE_RED;
                                else
                                    phase = PHASE_BLACK;
                                break;
                            }
                            case PHASE_RED:
                                events.CancelEventGroup(EVENT_GROUP_RED);
                                phase = !bluePhase ? PHASE_BLUE : PHASE_GREEN;
                                break;
                            case PHASE_BLUE:
                                events.CancelEventGroup(EVENT_GROUP_BLUE);
                                phase = bluePhase ? PHASE_RED : PHASE_GREEN;
                                break;
                            case PHASE_GREEN:
                            {
                                if (!IsHeroic())
                                {
                                    bluePhase = urand(0,1);
                                    phase = bluePhase ? PHASE_BLUE : PHASE_RED;
                                }
                                else
                                    phase = PHASE_BLACK;
                                break;
                            }
                            case PHASE_BLACK:
                                events.CancelEventGroup(EVENT_GROUP_BLACK);
                                phase = bluePhase ? PHASE_BLUE : PHASE_RED;
                                break;
                        }

                        // schedule events
                        switch (phase)
                        {
                            case PHASE_RED:
                                Talk(EMOTE_RED_BOTTLE);
                                Talk(SAY_RED_PHASE);
                                events.ScheduleEvent(EVENT_CONSUMING_FLAMES, 10000, EVENT_GROUP_RED);
                                events.ScheduleEvent(EVENT_SCORCHING_BLAST, 20000, EVENT_GROUP_RED);
                                if (Creature* stalker = me->FindNearestCreature(NPC_CAULDRON_STALKER, 30.0f))
                                    DoCast(stalker, SPELL_THROW_RED, true);
                                break;
                            case PHASE_BLUE:
                                Talk(EMOTE_BLUE_BOTTLE);
                                Talk(SAY_BLUE_PHASE);
                                events.ScheduleEvent(EVENT_BITING_CHILL, 20000, EVENT_GROUP_BLUE);
                                events.ScheduleEvent(EVENT_FLASH_FREEZE, 15000, EVENT_GROUP_BLUE);
                                if (Creature* stalker = me->FindNearestCreature(NPC_CAULDRON_STALKER, 30.0f))
                                    DoCast(stalker, SPELL_THROW_BLUE, true);
                                break;
                            case PHASE_GREEN:
                                Talk(EMOTE_GREEN_BOTTLE);
                                Talk(SAY_GREEN_PHASE);
                                if (Creature* stalker = me->FindNearestCreature(NPC_CAULDRON_STALKER, 30.0f))
                                    DoCast(stalker, SPELL_THROW_GREEN, true);
                                break;
                            case PHASE_BLACK:
                            {
                                events.ScheduleEvent(EVENT_ENGULFIND_DARKESS, 4500, EVENT_GROUP_BLACK);
                                events.ScheduleEvent(EVENT_VIL_RATA, 0, EVENT_GROUP_BLACK);
                                if (Creature* nefarian = me->GetCreature(*me, nefGUID))
                                    if (Creature* stalker = me->FindNearestCreature(NPC_CAULDRON_STALKER, 50.0f))
                                    {
                                        nefarian->CastSpell(stalker, SPELL_THROW_BLACK, true);
                                        nefarian->CastSpell(stalker, SPELL_THROW_BLACK_1, true);
                                    }
                                _engDar = 0;
                                break;
                            }
                        }
                        moving = true;
                        me->SetReactState(REACT_PASSIVE);
                        me->SetUInt64Value(UNIT_FIELD_TARGET, 0);
                        me->GetMotionMaster()->MovePoint(POINT_CAULDRON, cauldronPos);
                        break;
                    case EVENT_RESUME_COMBAT:
                        if (phase == PHASE_25)
                            DoCastAOE(SPELL_RELEASE_ALL);
                        else
                            events.ScheduleEvent(EVENT_NEXT_VIAL, phase == PHASE_BLACK ? 90000 : 50000);
                        me->SetReactState(REACT_AGGRESSIVE);
                        if (Unit* victim = me->getVictim())
                        {
                            me->SetUInt64Value(UNIT_FIELD_TARGET, victim->GetGUID());
                            DoStartMovement(victim);
                        }
                        break;
                    case EVENT_MAGMA_JETS:
                    {
                        DoCastVictim(SPELL_MAGMA_JETS);
                        if (me->getVictim())
                            _orientation = me->GetAngle(me->getVictim());
                        events.ScheduleEvent(EVENT_MAGMA_JETS, urand(10000, 15000));
                        break;
                    }
                    case EVENT_ABSOLUTE_ZERO:
                        DoCastRandom(SPELL_ABSOLUTE_ZERO, 0.0f, true);
                        events.ScheduleEvent(EVENT_ABSOLUTE_ZERO, urand(7000, 10000));
                        break;
                    case EVENT_ACID_NOVA:
                        DoCast(SPELL_ACID_NOVA);
                        events.ScheduleEvent(EVENT_ACID_NOVA, 20000);
                        break;
                    case EVENT_ENGULFIND_DARKESS:
                    {
                        _engDar++;
                        if (_engDar <= 7)
                            DoCast(SPELL_ENGULFIND_DARKESS);
                        events.ScheduleEvent(EVENT_ENGULFIND_DARKESS, 11000, EVENT_GROUP_BLACK);
                        break;
                    }
                    case EVENT_VIL_RATA:
                        DoCast(SPELL_VIL_RATA_MALORIAK);
                        break;
                    default:
                        break;
                    }
                }

                DoMeleeAttackIfReady();
                EnterEvadeIfOutOfCombatArea(diff);
            }
        private:
            uint32 combatTimer;
            uint32 enrageTimer;
            uint8 phase;
            uint8 releaseCastCounter;
            bool bluePhase;
            bool moving;
            uint64 nefGUID;
            float _orientation;
            uint32 _engDar;
            uint32 _vilRata;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_maloriakAI(creature);
        }
};

class npc_maloriak_absolute_zero : public CreatureScript
{
public:
    npc_maloriak_absolute_zero() : CreatureScript("npc_maloriak_absolute_zero") {}

    struct npc_maloriak_absolute_zeroAI : public ScriptedAI
    {
        npc_maloriak_absolute_zeroAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetDisplayId(11686);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }

        void IsSummonedBy(Unit* /*summoner*/)
        {
            me->SetInCombatWithZone();
            DoCast(SPELL_ABSOLUTE_ZERO_CAST);
            me->GetMotionMaster()->MoveRandom(50.0f);
        }

        void AttackStart(Unit* /*who*/) {} // disable attackstart

        void SpellHitTarget(Unit* /*victim*/, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_ABSOLUTE_ZERO_PERIODIC)
            {
                me->RemoveAllAuras();
                DoCast(SPELL_ABSOLUTE_ZERO_DMG);
                me->DespawnOrUnsummon(2000);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_maloriak_absolute_zeroAI(creature);
    }
};

class npc_maloriak_prime_subject : public CreatureScript
{
public:
    npc_maloriak_prime_subject() : CreatureScript("npc_maloriak_prime_subject") {}

    struct npc_maloriak_prime_subjectAI : public ScriptedAI
    {
        npc_maloriak_prime_subjectAI(Creature * creature) : ScriptedAI(creature)
        {
            me->DisableMovementFlagUpdate(true);
            me->SetDisableGravity(true);
        }

        void Reset()
        {
            tauntTimer = 10000;
            rendTimer = 1000;
            // nerf 4.2
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
        }

        void SpellHit(Unit* target, SpellInfo const* spellInfo)
        {
            if (spellInfo->Id == SPELL_RELEASE_ALL)
            {
                me->RemoveAurasDueToSpell(SPELL_DROWNED_STATE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                DoCast(me, SPELL_GROWTH_CATALYST);
                if (GameObject* chamber = me->FindNearestGameObject(GO_LARGE_CHAMBER, 10.0f))
                    chamber->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);
                if (Creature* releaseTrigger = me->FindNearestCreature(NPC_RELEASE_TRIGGER, 100.0f, true))
                    me->GetMotionMaster()->MoveJump(releaseTrigger->GetPositionX(), releaseTrigger->GetPositionY(), releaseTrigger->GetPositionZ(), 20.0f, 20.0f, POINT_RELEASE);
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == EFFECT_MOTION_TYPE && id == POINT_RELEASE)
            {
                me->SetDisableGravity(false);
                me->SetInCombatWithZone();
            }
        }

        void EnterCombat(Unit* who)
        {
            lastTarget = who;
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim() || me->HasAura(SPELL_DROWNED_STATE))
                return;

            if (tauntTimer <= diff)
            {
                if (!me->HasAura(SPELL_FIXATE))
                    DoCast(me, SPELL_FIXATE);

                if (lastTarget != me->getVictim())
                {
                    lastTarget = me->getVictim();
                    me->getThreatManager().resetAllAggro();
                    me->AddThreat(me->getVictim(), 1000000.0f);
                }

                tauntTimer = 5000;
            }else tauntTimer -= diff;

            if (rendTimer <= diff)
            {
                if (me->getVictim() && me->IsWithinMeleeRange(me->getVictim(), 2.0f))
                    DoCastVictim(SPELL_REND);
                rendTimer = 3000;
            }else rendTimer -= diff;

            DoMeleeAttackIfReady();
        }
    private:
        Unit* lastTarget;
        uint32 tauntTimer;
        uint32 rendTimer;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_maloriak_prime_subjectAI(creature);
    }
};

class npc_maloriak_aberration : public CreatureScript
{
public:
    npc_maloriak_aberration() : CreatureScript("npc_maloriak_aberration") {}

    struct npc_maloriak_aberrationAI : public ScriptedAI
    {
        npc_maloriak_aberrationAI(Creature * creature) : ScriptedAI(creature)
        {
            me->DisableMovementFlagUpdate(true);
            me->SetDisableGravity(true);
        }


        void SpellHit(Unit* target, SpellInfo const* spellInfo)
        {
            if (spellInfo->Id == SPELL_RELEASE_ABERRATIONS || spellInfo->Id == SPELL_RELEASE_ALL)
            {
                me->RemoveAurasDueToSpell(SPELL_DROWNED_STATE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                DoCast(me, SPELL_GROWTH_CATALYST);
                if (GameObject* chamber = me->FindNearestGameObject(GO_SMALL_CHAMBER, 10.0f))
                    chamber->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);
                if (Creature* releaseTrigger = me->FindNearestCreature(NPC_RELEASE_TRIGGER, 100.0f, true))
                    me->GetMotionMaster()->MoveJump(releaseTrigger->GetPositionX(), releaseTrigger->GetPositionY(), releaseTrigger->GetPositionZ(), 10.0f, 10.0f, POINT_RELEASE);
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == EFFECT_MOTION_TYPE && id == POINT_RELEASE)
            {
                me->SetDisableGravity(false);
                me->SetInCombatWithZone();
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim() || me->HasAura(SPELL_DROWNED_STATE))
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_maloriak_aberrationAI(creature);
    }
};

class npc_maloriak_flash_freeze : public CreatureScript
{
public:
    npc_maloriak_flash_freeze() : CreatureScript("npc_maloriak_flash_freeze") { }

    struct npc_maloriak_flash_freezeAI : public PassiveAI
    {
        npc_maloriak_flash_freezeAI(Creature* creature) : PassiveAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void IsSummonedBy(Unit* summoner)
        {
            if (Creature* maloriak = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_MALORIAK)))
                maloriak->AI()->JustSummoned(me);

            DoCast(me, SPELL_FLASH_FREEZE_VIS, true);
        }

        void JustDied(Unit* killer)
        {
            DoCastAOE(SPELL_FLASH_FREEZE_SHATTER);

            std::list<Creature*> freezeTriggers;
            GetCreatureListWithEntryInGrid(freezeTriggers, me, NPC_FLASH_FREEZE, 5.5f);
            if (!freezeTriggers.empty())
                for (std::list<Creature*>::iterator iter = freezeTriggers.begin(); iter != freezeTriggers.end(); ++iter)
                    if ((*iter)->isAlive())
                        me->Kill((*iter));
        }
    private:
        InstanceScript* instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_maloriak_flash_freezeAI(creature);
    }
};

class npc_maloriak_vil_rata : public CreatureScript
{
public:
    npc_maloriak_vil_rata() : CreatureScript("npc_maloriak_vil_rata") { }

    class VilRataEvent : public BasicEvent
    {
    public:
        VilRataEvent(Creature* owner) : _owner(owner) {}

        bool Execute(uint64 execTime, uint32 /*diff*/)
        {
            _owner->AI()->DoCastRandom(SPELL_SOMBRE_VASE, 0.0f);
            _owner->m_Events.AddEvent(this, execTime + 6000);
            return false;
        }

    private:
        Creature* _owner;
    };

    struct npc_maloriak_vil_rataAI : public ScriptedAI
    {
        npc_maloriak_vil_rataAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
            me->m_Events.AddEvent(new VilRataEvent(me), me->m_Events.CalculateTime(6000));
            me->SetInCombatWithZone();
        }

        void IsSummonedBy(Unit* /*summoner*/)
        {
            if (Creature *maloriak = Unit::GetCreature(*me, instance->GetData64(DATA_MALORIAK_GUID)))
                maloriak->AI()->JustSummoned(me);
        }

        void JustDied(Unit* killer)
        {
        }
    private:
        InstanceScript* instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_maloriak_vil_rataAI(creature);
    }
};

class spell_maloriak_remedy : public SpellScriptLoader
{
    public:
        spell_maloriak_remedy() : SpellScriptLoader("spell_maloriak_remedy") { }

        class spell_maloriak_remedy_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_maloriak_remedy_AuraScript)

            void HandleEffectPeriodicUpdate(AuraEffect* aurEff)
            {
                uint32 baseAmt = aurEff->GetBaseAmount();
                aurEff->SetAmount(aurEff->GetAmount() + baseAmt);
            }

            void Register()
            {
                OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_maloriak_remedy_AuraScript::HandleEffectPeriodicUpdate, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_maloriak_remedy_AuraScript();
        }
};

class spell_maloriak_magma_jets_periodic : public SpellScriptLoader
{
    public:
        spell_maloriak_magma_jets_periodic() : SpellScriptLoader("spell_maloriak_magma_jets_periodic") { }

        class spell_maloriak_magma_jets_periodic_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_maloriak_magma_jets_periodic_AuraScript)

            void HandleEffectPeriodic(AuraEffect const* aurEff)
            {
                float dist = aurEff->GetTickNumber() * 5.0f;
                Position pos;

                if (Unit* caster = GetCaster())
                {
                    caster->GetNearPosition(pos, dist, 0.0f);
                    if (Creature* summon = caster->SummonCreature(NPC_MAGMA_JET, pos, TEMPSUMMON_TIMED_DESPAWN, 20000))
                        summon->CastSpell(summon, SPELL_MAGMA_JETS_EFF, true);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_maloriak_magma_jets_periodic_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_maloriak_magma_jets_periodic_AuraScript();
        }
};

class spell_maloriak_throw_bottle_effect : public SpellScriptLoader
{
public:
    spell_maloriak_throw_bottle_effect() : SpellScriptLoader("spell_maloriak_throw_bottle_effect") { }

    class spell_maloriak_throw_bottle_effect_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_maloriak_throw_bottle_effect_SpellScript);

        void HandleDummy(SpellEffIndex effIndex)
        {
            GetCaster()->CastSpell(GetHitUnit(), GetSpellValue()->EffectBasePoints[EFFECT_0], true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_maloriak_throw_bottle_effect_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_maloriak_throw_bottle_effect_SpellScript();
    }
};

class spell_maloriak_flash_freeze_target_filter : public SpellScriptLoader
{
public:
    spell_maloriak_flash_freeze_target_filter() : SpellScriptLoader("spell_maloriak_flash_freeze_target_filter") { }

    class FlashFreezeTargetSelector
    {

    public:
        explicit FlashFreezeTargetSelector(Unit* unit) : _me(unit) {};

        bool operator()(WorldObject* target) const
        {
            return (target->GetDistance2d(_me->GetPositionX(), _me->GetPositionY()) <= 10.0f || target->ToUnit()->HasAura(SPELL_FLASH_FREEZE));
        }

    private:
        Unit const* _me;
    };


    class spell_maloriak_flash_freeze_target_filter_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_maloriak_flash_freeze_target_filter_SpellScript);

        void HandleDummy(SpellEffIndex effIndex)
        {
            if (Unit* caster = GetCaster())
                if (Unit *target = GetHitUnit())
                {
                    Map::PlayerList const& players = caster->GetMap()->GetPlayers();
                    if (!players.isEmpty())
                        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                            if (Player* player = itr->getSource())
                                if (target->GetDistance(player) < 5.0f || target->GetGUID() == player->GetGUID())
                                {
                                    player->CastSpell(player, SPELL_FLASH_FREEZE_SUMMON, true);
                                    GetCaster()->CastSpell(player, GetSpellValue()->EffectBasePoints[EFFECT_0], true);
                                }
                }
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            if (!unitList.empty())
            {
                WorldObject *target = Trinity::Containers::SelectRandomContainerElement(unitList);
                if (Unit *caster = GetCaster())
                    unitList.remove_if(FlashFreezeTargetSelector(caster));
                if (!unitList.empty())
                    Trinity::Containers::RandomResizeList(unitList, 1);
                else if (target)
                    unitList.push_back(target);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_maloriak_flash_freeze_target_filter_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(spell_maloriak_flash_freeze_target_filter_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript * GetSpellScript() const
    {
        return new spell_maloriak_flash_freeze_target_filter_SpellScript();
    }
};

class spell_maloriak_shatter : public SpellScriptLoader
{
public:
    spell_maloriak_shatter() : SpellScriptLoader("spell_maloriak_shatter") { }

    class spell_maloriak_shatter_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_maloriak_shatter_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            if (GetHitUnit()->isAlive())
            {
                GetHitUnit()->RemoveAurasDueToSpell(77699);
                GetHitUnit()->RemoveAurasDueToSpell(92978);
                GetHitUnit()->RemoveAurasDueToSpell(92979);
                GetHitUnit()->RemoveAurasDueToSpell(92980);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_maloriak_shatter_SpellScript::HandleScript, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_maloriak_shatter_SpellScript();
    }
};

class spell_maloriak_release_aberrations_filter : public SpellScriptLoader
{
public:
    spell_maloriak_release_aberrations_filter() : SpellScriptLoader("spell_maloriak_release_aberrations_filter") { }

    class spell_maloriak_release_aberrations_filter_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_maloriak_release_aberrations_filter_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        bool load()
        {
            hit = false;
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            aberrationsCount = 0;
            for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end();)
            {
                if ((*itr)->GetTypeId() == TYPEID_UNIT && (*itr)->ToCreature()->GetEntry() == NPC_ABERRATION && (*itr)->ToCreature()->HasAura(SPELL_DROWNED_STATE) && aberrationsCount < 3)
                {
                    ++itr;
                    ++aberrationsCount;
                }
                else
                    targets.erase(itr++);
            }
        }

        void HandleDummyLaunch(SpellEffIndex effIndex)
        {
            if (hit)
                return;
            hit = true;
            GetCaster()->ToCreature()->AI()->DoAction(ACTION_RELEASE_CASTED);
        }


        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_maloriak_release_aberrations_filter_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnEffectHitTarget += SpellEffectFn(spell_maloriak_release_aberrations_filter_SpellScript::HandleDummyLaunch, EFFECT_0, SPELL_EFFECT_DUMMY);

        }
    private:
        uint8 aberrationsCount;
        bool hit;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_maloriak_release_aberrations_filter_SpellScript();
    }
};

class spell_maloriak_release_all_aberrations_filter : public SpellScriptLoader
{
public:
    spell_maloriak_release_all_aberrations_filter() : SpellScriptLoader("spell_maloriak_release_all_aberrations_filter") { }

    class spell_maloriak_release_all_aberrations_filter_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_maloriak_release_all_aberrations_filter_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end();)
            {
                if ((*itr)->GetTypeId() == TYPEID_UNIT && (*itr)->ToCreature()->HasAura(SPELL_DROWNED_STATE))
                    ++itr;
                else
                    targets.erase(itr++);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_maloriak_release_all_aberrations_filter_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_maloriak_release_all_aberrations_filter_SpellScript();
    }
};

class spell_maloriak_debilitating_slime : public SpellScriptLoader
{
public:
    spell_maloriak_debilitating_slime() : SpellScriptLoader("spell_maloriak_debilitating_slime") { }

    class spell_maloriak_debilitating_slime_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_maloriak_debilitating_slime_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            if (!target)
                return;

            target->RemoveAurasDueToSpell(SPELL_GROWTH_CATALYST);
            target->RemoveAurasDueToSpell(101440);
            target->RemoveAurasDueToSpell(101441);
            target->RemoveAurasDueToSpell(101442);
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            if (!target)
                return;

            if (target->isAlive())
                target->CastSpell(target, SPELL_GROWTH_CATALYST, true);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_maloriak_debilitating_slime_AuraScript::OnApply, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_maloriak_debilitating_slime_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_maloriak_debilitating_slime_AuraScript();
    }
};

class spell_maloriak_consuming_flames : public SpellScriptLoader
{
public:
    spell_maloriak_consuming_flames() : SpellScriptLoader("spell_maloriak_consuming_flames") { }

    class spell_maloriak_consuming_flames_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_maloriak_consuming_flames_AuraScript);

        bool load()
        {
            damage = 0;
            return true;
        }

        void HandleEffectPeriodicUpdate(AuraEffect* aurEff)
        {
            aurEff->SetAmount(aurEff->GetAmount() + damage);
            damage = 0;
        }

        void DoProc(ProcEventInfo& eventInfo)
        {
            if (eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id != GetSpellInfo()->Id)
                if (eventInfo.GetSchoolMask() == SPELL_SCHOOL_MASK_MAGIC)
                    damage = eventInfo.GetDamageInfo()->GetDamage() / 2;
        }

    private:
        int32 damage;

        void Register()
        {
            OnProc += AuraProcFn(spell_maloriak_consuming_flames_AuraScript::DoProc);
            OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_maloriak_consuming_flames_AuraScript::HandleEffectPeriodicUpdate, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_maloriak_consuming_flames_AuraScript();
    }
};

class spell_maloriak_engulfing_madness : public SpellScriptLoader
{
    public:
        spell_maloriak_engulfing_madness() : SpellScriptLoader("spell_maloriak_engulfing_madness") { }

        class spell_maloriak_engulfing_madness_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_maloriak_engulfing_madness_AuraScript)

            void HandleEffectPeriodicUpdate(AuraEffect* aurEff)
            {
                if (Unit *caster = GetCaster())
                    if (Unit *victim = caster->getVictim())
                        caster->SetFacingToObject(victim);
            }

            void Register()
            {
                OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_maloriak_engulfing_madness_AuraScript::HandleEffectPeriodicUpdate, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_maloriak_engulfing_madness_AuraScript();
        }
};

class spell_maloriak_vil_rata_periodic : public SpellScriptLoader
{
    public:
        spell_maloriak_vil_rata_periodic() : SpellScriptLoader("spell_maloriak_vil_rata_periodic") { }

        class spell_maloriak_vil_rata_periodic_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_maloriak_vil_rata_periodic_AuraScript)

            bool load()
            {
                return true;
            }

            void HandleEffectPeriodic(AuraEffect const* aurEff)
            {
                if (Unit* caster = GetCaster())
                {
                    float x, y;
                    caster->GetNearPoint2D(x, y, 3.0f, (float)(M_PI * (float)aurEff->GetTickNumber() / (float)aurEff->GetTotalTicks()));
                    caster->CastSpell(x, y, caster->GetPositionZ(), SPELL_VIL_RATA_MISSILE, true);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_maloriak_vil_rata_periodic_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_maloriak_vil_rata_periodic_AuraScript();
        }
};

void AddSC_boss_maloriak()
{
    new boss_maloriak();
    new npc_maloriak_absolute_zero();
    new npc_maloriak_prime_subject();
    new npc_maloriak_aberration();
    new npc_maloriak_flash_freeze();
    new npc_maloriak_vil_rata();
    new spell_maloriak_remedy();
    new spell_maloriak_magma_jets_periodic();
    new spell_maloriak_throw_bottle_effect();
    new spell_maloriak_flash_freeze_target_filter();
    new spell_maloriak_shatter();
    new spell_maloriak_release_aberrations_filter();
    new spell_maloriak_release_all_aberrations_filter();
    new spell_maloriak_debilitating_slime();
    new spell_maloriak_consuming_flames();
    new spell_maloriak_engulfing_madness();
    new spell_maloriak_vil_rata_periodic();
};
