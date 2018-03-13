/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "zulgurub.h"

enum Spells
{
    SPELL_TIKI_BURN = 96861,

    SPELL_ARTEFACT_AURA = 97040,
    SPELL_PORTAL_AURA = 97081,
    SPELL_BREAK_ARTIFACT = 97065,

    // renataki
    SPELL_FRENSIE = 8269,
    SPELL_VANISH = 96639,
    SPELL_EMBUSH = 96640,
    SPELL_THOUSAND_BLADES = 96646,
    SPELL_CHARGE = 72700,
    SPELL_DEADLY_POISON = 96648,

    // GRILEK
    SPELL_AVATAR = 96618,
    SPELL_RUPTURE_LINE = 96619,
    SPELL_PURSUIT = 96306,
    SPELL_PURSUIT_ZOMBI = 96631,
    SPELL_ENTANGLING_ROOTS = 96633,
    SPELL_RUPTURE_EFF = 96620,

    // boss_wushoolay
    SPELL_FORKED_LIGHTING = 96712,
    SPELL_LIGHTING_CLOUD = 96710,
    SPELL_LIGHTING_ROD = 96698, // trigger SPELL_AIR_TO_GROUND_LIGHTING_VISUAL
    SPELL_AIR_TO_GROUND_LIGHTING_VISUAL = 96698,
    SPELL_LIGHTING_RUSH = 96697,
};

enum Events
{
    // npc_ritual_tiki_mask
    EVENT_MOVE_AROUND_PLAYER            = 1,
    EVENT_MOVE_CIRCLE                   = 2,
    EVENT_MOVE_CIRCLE_GO                = 3,
    EVENT_EVADE                         = 4,
    EVENT_TIKI_BURN                     = 5,
    // npc_ancient_artifact
    ACTION_START_FLY                    = 1,
    EVENT_MOVE_AROUND                   = 2,
    EVENT_MOVE_AROUND_CIRCLE            = 3,
    EVENT_SUMMON_BOSS_VISUAL            = 4,
    EVENT_SHATTER_ARTIFACT              = 5,
    // boss_renataki
    EVENT_DEADLY_POISON                 = 1,
    EVENT_VANISH                        = 2,
    EVENT_AMBUSH                        = 3,
    EVENT_RETURN_TO_FIGHT               = 4,
    EVENT_THOUSAND_BLADES               = 5,
    EVENT_CHARGE_TARGET_DEST            = 6,
    EVENT_REMOVE_THOUSAND_BLADES        = 7,
    // boss_grilek
    EVENT_AVATAR                        = 1,
    EVENT_RUPTURE                       = 2,
    EVENT_ENTANGLING_ROOTS              = 3,
    // boss_wushoolay
    EVENT_LIGHTING_CLOUD                = 1,
    EVENT_FORKED_LIGHTING               = 2,
    EVENT_LIGHTING_RUSH                 = 3,
    EVENT_CHARGE_PLAYER                 = 4,
    EVENT_LIGHTING_ROD                  = 5,
};

enum Yells
{
    // boss_renataki
    YELL_AMBUSH                         = 0,
    YELL_THOUSAND_BLADES                = 1,
    YELL_AGGRO                          = 2,
    YELL_PLAYER_KILL                    = 3,
    YELL_DIED                           = 4,
    EMOTE_FRENZY                        = 5,
};

enum eNpcs
{
   NPC_ARTIFACT_1 = 52452,
   NPC_ARTIFACT_2 = 52454,
   NPC_ARTIFACT_3 = 52450,
   NPC_ARTIFACT_4 = 52446,
   NPC_GENERAL_BUNNY = 45979,
   NPC_NIGHTMARE_SUMMON = 52284,
};

enum Misc
{
    POINT_MOVE_CIRCLE                   = 1,
    POINT_CHARGE            = 1003,
    POINT_TARGET = 2,
};

const uint32 ArtifactEntry[4]=
{
    NPC_ARTIFACT_1,
    NPC_ARTIFACT_2,
    NPC_ARTIFACT_3,
    NPC_ARTIFACT_4,
};

const float TikiCenter[3] = {-11877.9f, -1874.97f, 63.5755f};

class npc_ritual_tiki_mask : public CreatureScript
{
    public:
        npc_ritual_tiki_mask() : CreatureScript("npc_ritual_tiki_mask") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_ritual_tiki_maskAI(creature);
        }

        struct npc_ritual_tiki_maskAI : public ScriptedAI
        {
            npc_ritual_tiki_maskAI(Creature* creature) : ScriptedAI(creature), instance(me->GetInstanceScript()) {}

            InstanceScript* instance;
            EventMap events;
            bool MoveCircle;

            void InitializeAI()
            {
                ASSERT(instance);

                if (me->GetPositionZ() >= 65.0f)
                    MoveCircle = true;
                else
                    MoveCircle = false;

                Reset();
            }

            void Reset()
            {
                events.Reset();

                if (MoveCircle)
                    events.ScheduleEvent(EVENT_MOVE_CIRCLE, 100);
            }

            void EnterEvadeMode()
            {
                instance->SetData(DATA_TIKI_MASK_ID, NOT_STARTED);
                me->SetControlled(false, UNIT_STATE_ROOT);
                ScriptedAI::EnterEvadeMode();
                me->ClearUnitState(UNIT_STATE_EVADE);
            }

            void EnterCombat(Unit* /*who*/)
            {
                me->SetControlled(true, UNIT_STATE_ROOT);
                me->StopMoving();
                events.Reset();
                events.ScheduleEvent(EVENT_MOVE_AROUND_PLAYER, 200);
                events.ScheduleEvent(EVENT_TIKI_BURN, 2000 * instance->GetData(DATA_TIKI_MASK_ID));
                instance->SetData(DATA_TIKI_MASK_ID, IN_PROGRESS);
            }

            void FillCirclePath(Position const& centerPos, float radius, float z, Movement::PointsArray& path, bool clockwise, bool randDist)
            {
                float step = clockwise ? -M_PI / 8.0f : M_PI / 8.0f;
                float angle = centerPos.GetAngle(me->GetPositionX(), me->GetPositionY());

                for (uint8 i = 0; i < 16; angle += step, ++i)
                {
                    G3D::Vector3 point;
                    float _dist = randDist ? frand(-3.0f, 3.0f) : 0.0f;
                    point.x = centerPos.GetPositionX() + (radius + _dist) * cosf(angle);
                    point.y = centerPos.GetPositionY() + (radius + _dist) * sinf(angle);
                    point.z = z;
                    path.push_back(point);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                events.Update(diff);

                if (!UpdateVictim())
                {
                    if (uint32 eventId = events.ExecuteEvent())
                    {
                        switch (eventId)
                        {
                            case EVENT_MOVE_CIRCLE:
                            {
                                if (me->isMoving())
                                    me->StopMoving();

                                float x, y, z, angle = frand(-M_PI, M_PI), dist = frand(5.0f, 15.0f);
                                x = TikiCenter[0] + dist * cos(angle);
                                y = TikiCenter[1] + dist * sin(angle);
                                z = TikiCenter[2] + frand(5.0f, 10.0f);
                                me->GetMotionMaster()->MovePoint(POINT_MOVE_CIRCLE, x, y, z);
                                events.ScheduleEvent(EVENT_MOVE_CIRCLE_GO, 6000);
                                events.ScheduleEvent(EVENT_MOVE_CIRCLE, urand(60000, 360000));
                                break;
                            }
                            case EVENT_MOVE_CIRCLE_GO:
                            {
                                Position center;
                                me->GetPosition(&center);
                                me->GetMotionMaster()->MoveAroundPoint(center, 7.0f, rand() % 1 ? true : false, 100);
                                break;
                            }
                        }
                    }

                    return;
                }

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_MOVE_AROUND_PLAYER:
                        {
                            if (Unit* target = me->getVictim())
                            {
                                Position center;
                                target->GetPosition(&center);
                                me->GetMotionMaster()->MoveAroundPoint(center, 7.0f, rand() % 1 ? true : false, 100);
                            }
                            events.ScheduleEvent(EVENT_MOVE_AROUND_PLAYER, urand(2000, 5000));
                            break;
                        }
                        case EVENT_TIKI_BURN:
                            me->CastSpell(me, SPELL_TIKI_BURN, true);
                            break;
                    }
                }
            }
        };
};

const float ArtifactWP[25][3]=
{
    {-11883.58f, -1884.578f, 65.00301f},
    {-11885.81f, -1880.564f, 65.00301f},
    {-11884.55f, -1876.148f, 65.00301f},
    {-11880.54f, -1873.918f, 65.00301f},
    {-11876.12f, -1875.179f, 65.00301f},
    {-11873.89f, -1879.193f, 65.00301f},
    {-11875.15f, -1883.609f, 65.00301f},
    {-11879.17f, -1885.839f, 65.00301f},

    {-11885.75f, -1874.650f, 65.00301f},
    {-11879.23f, -1872.431f, 65.00301f},
    {-11873.05f, -1875.474f, 65.00301f},
    {-11870.83f, -1881.995f, 65.00301f},
    {-11873.88f, -1888.175f, 65.00301f},
    {-11880.40f, -1890.394f, 65.00301f},
    {-11886.58f, -1887.351f, 65.00301f},
    {-11888.80f, -1880.830f, 65.00301f},

    {-11870.02f, -1877.223f, 65.00301f},
    {-11870.71f, -1886.381f, 65.00301f},
    {-11877.67f, -1892.373f, 65.00301f},
    {-11886.83f, -1891.687f, 65.00301f},
    {-11892.82f, -1884.725f, 65.00301f},
    {-11892.14f, -1875.567f, 65.00301f},
    {-11885.17f, -1869.575f, 65.00301f},
    {-11876.02f, -1870.261f, 65.00301f},

    {-11880.74f, -1881.625f, 65.00301f},
};

const Position PortalSP = {-11880.47f, -1880.661f, 64.04917f, 1.553343f};

class npc_ancient_artifact : public CreatureScript
{
    public:
        npc_ancient_artifact() : CreatureScript("npc_ancient_artifact") { }

    private:
        bool OnGossipHello(Player* player, Creature* creature)
        {
            if (InstanceScript *instance = player->GetInstanceScript())
                if (!instance->IsDone(DATA_CACHE_OF_MADNESS))
                    if (player->GetMaxSkillValue(SKILL_ARCHAEOLOGY) >= 225 || player->isGameMaster())
                    {
                        creature->AI()->DoAction(ACTION_START_FLY);
                        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    }

            return true;
        }

        struct npc_ancient_artifactAI : public ScriptedAI
        {
            npc_ancient_artifactAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset()
            {
                pathId = 0;
                active = false;
                me->SetSpeed(MOVE_FLIGHT, 0.25f);
                me->RemoveAllAuras();
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_START_FLY && !active)
                {
                    active = true;
                    me->GetMotionMaster()->MovePoint(0, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 2.0f);

                    if (InstanceScript* instance = me->GetInstanceScript())
                    {
                        events.ScheduleEvent(EVENT_MOVE_AROUND, 2000);
                        pathId = instance->GetData(DATA_CACHE_OF_MADNESS_ARTIFACT);
                        instance->SetData(DATA_CACHE_OF_MADNESS_ARTIFACT, IN_PROGRESS);
                    }
                }
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type == POINT_MOTION_TYPE && id == POINT_MOVE_CIRCLE)
                    events.ScheduleEvent(pathId < 3 ? EVENT_MOVE_AROUND_CIRCLE : EVENT_SUMMON_BOSS_VISUAL, 250);
            }

            void UpdateAI(uint32 const diff)
            {
                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_MOVE_AROUND:
                        {
                            me->CastSpell(me, SPELL_ARTEFACT_AURA, true);
                            me->GetMotionMaster()->MovePoint(POINT_MOVE_CIRCLE, ArtifactWP[pathId * 8][0], ArtifactWP[pathId * 8][1], ArtifactWP[pathId * 8][2]);
                            break;
                        }
                        case EVENT_MOVE_AROUND_CIRCLE:
                        {
                            me->GetMotionMaster()->MoveAroundPoint(EdgeofMadnessSP, 5.0f, true, 150);
                            break;
                        }
                        case EVENT_SUMMON_BOSS_VISUAL:
                        {
                            if (Creature* bunny = me->SummonCreature(NPC_GENERAL_BUNNY, PortalSP, TEMPSUMMON_TIMED_DESPAWN, 30000))
                            {
                                bunny->CastSpell(bunny, SPELL_PORTAL_AURA, false);
                                events.ScheduleEvent(EVENT_SHATTER_ARTIFACT, 12000);
                            }
                            break;
                        }
                        case EVENT_SHATTER_ARTIFACT:
                        {
                            for (int i = 0; i < 4; ++i)
                                if (me->GetEntry() != ArtifactEntry[i])
                                    if (Creature* artifact = me->FindNearestCreature(ArtifactEntry[i], 30.0f))
                                    {
                                        artifact->CastSpell(artifact, SPELL_BREAK_ARTIFACT, true);
                                        artifact->StopMoving();
                                        artifact->DespawnOrUnsummon(1000);
                                    }

                            me->CastSpell(me, SPELL_BREAK_ARTIFACT, true);
                            me->StopMoving();
                            me->DespawnOrUnsummon(1000);
                            break;
                        }
                    }
                }
            }

        private:
            EventMap events;
            uint8 pathId;
            bool active;
        };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ancient_artifactAI (creature);
    }
};

class spell_cursed_portal : public SpellScriptLoader
{
    public:
        spell_cursed_portal() : SpellScriptLoader("spell_cursed_portal") { }

    private:
        class spell_cursed_portalAuraScript : public AuraScript
        {
            PrepareAuraScript(spell_cursed_portalAuraScript)

            void EffectApply(AuraEffect const * aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (InstanceScript* instance = GetTarget()->GetInstanceScript())
                    instance->SetData(DATA_CACHE_OF_MADNESS_ARTIFACT, DONE);

                aurEff->GetBase()->SetDuration(7000);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_cursed_portalAuraScript::EffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript *GetAuraScript() const
        {
            return new spell_cursed_portalAuraScript();
        }
};

class boss_renataki : public CreatureScript
{
    public:
        boss_renataki() : CreatureScript("boss_renataki") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_renatakiAI(creature);
        }

        struct boss_renatakiAI : public BossAI
        {
            boss_renatakiAI(Creature* creature) : BossAI(creature, DATA_CACHE_OF_MADNESS) { }

            void Reset()
            {
                _Reset();
                me->SetReactState(REACT_AGGRESSIVE);
                Frenzy = false;
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }

            void DamageTaken(Unit* /*done_by*/, uint32 & /*damage*/)
            {
                if (!Frenzy && me->GetHealthPct() <= 30.0f)
                {
                    Frenzy = true;
                    Talk(EMOTE_FRENZY);
                    me->CastSpell(me, SPELL_FRENSIE, true);
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(YELL_AGGRO);
                events.ScheduleEvent(EVENT_VANISH, urand(7000, 21000));
                events.ScheduleEvent(EVENT_DEADLY_POISON, urand(3000, 6000));
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(YELL_PLAYER_KILL);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(YELL_DIED);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_VANISH:
                            me->CastSpell(me, SPELL_VANISH, false);
                            me->SetReactState(REACT_PASSIVE);
                            me->AttackStop();
                            events.Reset();
                            events.ScheduleEvent(EVENT_AMBUSH, 2000);
                            break;
                        case EVENT_AMBUSH:
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 50.0f, true))
                                me->CastSpell(target, SPELL_EMBUSH, true);

                            me->RemoveAura(SPELL_VANISH);
                            Talk(YELL_AMBUSH);
                            events.ScheduleEvent(EVENT_DEADLY_POISON, urand(3000, 6000));
                            events.ScheduleEvent(EVENT_RETURN_TO_FIGHT, 2000);
                            events.ScheduleEvent(EVENT_THOUSAND_BLADES, urand(10000, 20000));
                            break;
                        }
                        case EVENT_RETURN_TO_FIGHT:
                            me->SetReactState(REACT_AGGRESSIVE);
                            break;
                        case EVENT_THOUSAND_BLADES:
                            Talk(YELL_THOUSAND_BLADES);
                            me->CastSpell(me, SPELL_THOUSAND_BLADES, false);
                            events.ScheduleEvent(EVENT_CHARGE_TARGET_DEST, urand(1000, 2000));
                            events.ScheduleEvent(EVENT_REMOVE_THOUSAND_BLADES, urand(10000, 15000));
                            break;
                        case EVENT_CHARGE_TARGET_DEST:
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 50.0f, true))
                                me->CastSpell(target, SPELL_CHARGE, true);

                            if (me->HasAura(SPELL_THOUSAND_BLADES))
                                events.ScheduleEvent(EVENT_CHARGE_TARGET_DEST, urand(1000, 2000));
                            break;
                        }
                        case EVENT_DEADLY_POISON:
                            DoCastVictim(SPELL_DEADLY_POISON);
                            events.ScheduleEvent(EVENT_DEADLY_POISON, urand(3000, 6000));
                            break;
                        case EVENT_REMOVE_THOUSAND_BLADES:
                            me->RemoveAura(SPELL_THOUSAND_BLADES);
                            events.ScheduleEvent(EVENT_VANISH, urand(10000, 20000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
                EnterEvadeIfOutOfCombatArea(diff);
            }

        private:
            bool Frenzy;
        };
};

class spell_thousand_blades_damage : public SpellScriptLoader
{
    public:
        spell_thousand_blades_damage() : SpellScriptLoader("spell_thousand_blades_damage") { }

    private:
        class spell_thousand_blades_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_thousand_blades_damage_SpellScript);

            void CalculateDamage(SpellEffIndex /*effIndex*/)
            {
                float dist = 1.0f - GetCaster()->GetDistance(GetHitUnit()) / 100;
                SetHitDamage(int32(GetHitDamage() * dist));
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_thousand_blades_damage_SpellScript::CalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_thousand_blades_damage_SpellScript();
        }
};

// 52258
class boss_grilek : public CreatureScript
{
    public:
        boss_grilek() : CreatureScript("boss_grilek") { }

    private:
        struct boss_grilekAI : public BossAI
        {
            boss_grilekAI(Creature* creature) : BossAI(creature, DATA_CACHE_OF_MADNESS) {}

            void Reset()
            {
                _Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_AVATAR, 20000);
                events.ScheduleEvent(EVENT_RUPTURE, 5000);
                _EnterCombat();
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
            }

            void SpawnRupture(std::list<Creature *> &ruptures)
            {
                Position centerPos;
                me->GetPosition(&centerPos);
                float step = M_PI / 16.0f;
                float angle = 0.0f;

                float basedist = 2.0f;
                for (uint8 i = 0; i < 4; ++i)
                {
                    Position pos;
                    me->GetFirstCollisionPosition(pos, basedist, angle + i * step + M_PI / 4.0f);
                    basedist += 1.0f;
                    if (Creature *c = me->SummonCreature(NPC_GENERAL_BUNNY, pos, TEMPSUMMON_TIMED_DESPAWN, 10000))
                        ruptures.push_back(c);
                }


                basedist += 6.0f;
                for (uint8 i = 0; i < 6; ++i)
                {
                    Position pos;
                    me->GetFirstCollisionPosition(pos, basedist, angle + i * step);
                    basedist -= 1.0f;
                    if (Creature *c = me->SummonCreature(NPC_GENERAL_BUNNY, pos, TEMPSUMMON_TIMED_DESPAWN, 10000))
                        ruptures.push_back(c);
                }

                basedist += 13.0f;
                for (uint8 i = 0; i < 7; ++i)
                {
                    Position pos;
                    me->GetFirstCollisionPosition(pos, basedist, angle + i * step);
                    basedist -= 1.0f;
                    if (Creature *c = me->SummonCreature(NPC_GENERAL_BUNNY, pos, TEMPSUMMON_TIMED_DESPAWN, 10000))
                        ruptures.push_back(c);
                }

                basedist += 15.0f;
                for (uint8 i = 0; i < 8; ++i)
                {
                    Position pos;
                    me->GetFirstCollisionPosition(pos, basedist, angle + i * step);
                    basedist -= 1.0f;
                    if (Creature *c = me->SummonCreature(NPC_GENERAL_BUNNY, pos, TEMPSUMMON_TIMED_DESPAWN, 10000))
                        ruptures.push_back(c);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_AVATAR:
                            me->CastSpell(me, SPELL_AVATAR, true);
                            me->SetReactState(REACT_PASSIVE);
                            me->AttackStop();
                            DoCastRandom(SPELL_PURSUIT_ZOMBI, 0.0f);
                            events.ScheduleEvent(EVENT_ENTANGLING_ROOTS, 8000);
                            events.ScheduleEvent(EVENT_AVATAR, 50000);
                            break;
                        case EVENT_RUPTURE:
                        {
                            if (!me->HasAura(SPELL_AVATAR))
                            {
                                std::list<Creature *> ruptures;
                                SpawnRupture(ruptures);
                                for (std::list<Creature *>::iterator itr = ruptures.begin(); itr != ruptures.end(); itr++)
                                    if (Creature *c = *itr)
                                        me->CastSpell(c, SPELL_RUPTURE_EFF, false);
                            }
                            events.ScheduleEvent(EVENT_RUPTURE, 10000);
                            break;
                        }
                        case EVENT_ENTANGLING_ROOTS:
                            DoCastVictim(SPELL_ENTANGLING_ROOTS);
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
            return new boss_grilekAI(creature);
        }
};

// 96631
class spell_grilek_pursuit : public SpellScriptLoader
{
public:
    spell_grilek_pursuit() : SpellScriptLoader("spell_grilek_pursuit") { }

    class spell_grilek_pursuit_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_grilek_pursuit_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            return true;
        }

        void HandleScript(SpellEffIndex effIndex)
        {
            if (Unit* target = GetHitUnit())
                target->CastWithDelay(3000, GetCaster(), GetEffectValue(), true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_grilek_pursuit_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_grilek_pursuit_SpellScript();
    }
};

// 99284
class spell_grilek_fixate : public SpellScriptLoader
{
public:
    spell_grilek_fixate() : SpellScriptLoader("spell_grilek_fixate") { }

    class spell_grilek_fixate_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_grilek_fixate_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit *grilek = GetTarget())
                if (Unit *player = GetCaster())
                {
                    if (grilek->ToCreature())
                        grilek->ToCreature()->SetReactState(REACT_AGGRESSIVE);
                    grilek->AddThreat(player, 50000000.0f);
                    grilek->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                    grilek->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
                }
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit *grilek = GetTarget())
                if (Unit *player = GetCaster())
                {
                    grilek->getThreatManager().addThreat(player, -50000000.0f);
                    grilek->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
                    grilek->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
                }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_grilek_fixate_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_TAUNT, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_grilek_fixate_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_TAUNT, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_grilek_fixate_AuraScript();
    }
};

// 52286
class boss_wushoolay : public CreatureScript
{
    public:
        boss_wushoolay() : CreatureScript("boss_wushoolay") { }

    private:
        struct boss_wushoolayAI : public BossAI
        {
            boss_wushoolayAI(Creature* creature) : BossAI(creature, DATA_CACHE_OF_MADNESS) {}

            void Reset()
            {
                _Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                // 0.13
                // 0.23
                events.ScheduleEvent(EVENT_LIGHTING_RUSH, 30000); // 0.32
                events.ScheduleEvent(EVENT_LIGHTING_CLOUD, 60000);
                events.ScheduleEvent(EVENT_FORKED_LIGHTING, 10000);
                _EnterCombat();
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type == POINT_MOTION_TYPE && id == POINT_CHARGE)
                    events.ScheduleEvent(EVENT_LIGHTING_ROD, 1000);
            }


            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_LIGHTING_CLOUD:
                            if (!me->HasAura(SPELL_LIGHTING_RUSH))
                                DoCastRandom(SPELL_LIGHTING_CLOUD, 0.0f);
                            events.ScheduleEvent(EVENT_LIGHTING_CLOUD, 10000);
                            break;
                        case EVENT_FORKED_LIGHTING:
                            if (!me->HasAura(SPELL_LIGHTING_RUSH))
                                DoCast(SPELL_FORKED_LIGHTING);
                            events.ScheduleEvent(EVENT_FORKED_LIGHTING, urand(5000, 10000));
                            break;
                        case EVENT_LIGHTING_RUSH:
                            DoCast(SPELL_LIGHTING_RUSH);
                            events.ScheduleEvent(EVENT_CHARGE_PLAYER, 3000);
                            events.ScheduleEvent(EVENT_LIGHTING_RUSH, 30000);
                            break;
                        case EVENT_CHARGE_PLAYER:
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 50.0f, true))
                                me->CastSpell(target, SPELL_CHARGE, true);
                            break;
                        }
                        case EVENT_LIGHTING_ROD:
                            me->RemoveAura(SPELL_LIGHTING_RUSH);
                            DoCast(SPELL_LIGHTING_ROD);
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
            return new boss_wushoolayAI(creature);
        }
};

enum eSpells
{
    SPELL_EARTH_SHOCK = 96650,
    SPELL_WRATH = 96651,
    SPELL_AWAKEN_MADNESS = 96658,
    SPELL_SLEEP = 96654,
    SPELL_NIGHTMARE_ILLUSION_SUMMON = 96670,

    // NIGHTMARE SUMMON
    SPELL_WALKING_NIGHTMARE = 96757,
    SPELL_CONSUME_SOUL = 96758,
};

enum eEvents
{
    EVENT_AWAKEN_MADNESS     = 1,
    EVENT_EARTH_SHOCK        = 2,
    EVENT_WRATH              = 3,
};

// 52271
class boss_hazzarah : public CreatureScript
{
    public:
        boss_hazzarah() : CreatureScript("boss_hazzarah") { }

    private:
        struct boss_hazzarahAI : public BossAI
        {
            boss_hazzarahAI(Creature* creature) : BossAI(creature, DATA_CACHE_OF_MADNESS) {}

            void Reset()
            {
                firstSleep = false;
                secondSleep = false;
                _Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_EARTH_SHOCK, 10000);
                events.ScheduleEvent(EVENT_WRATH, 4000);
                _EnterCombat();
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
            }

            void DamageTaken(Unit* /*done_by*/, uint32 & /*damage*/)
            {
                if (!firstSleep && me->GetHealthPct() <= 66.0f)
                {
                    firstSleep = true;
                    events.ScheduleEvent(EVENT_AWAKEN_MADNESS, 100);
                }
                if (!secondSleep && me->GetHealthPct() <= 33.0f)
                {
                    secondSleep = true;
                    events.ScheduleEvent(EVENT_AWAKEN_MADNESS, 100);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_AWAKEN_MADNESS:
                            DoCast(SPELL_AWAKEN_MADNESS);
                            break;
                        case EVENT_EARTH_SHOCK:
                            events.CancelEvent(EVENT_WRATH);
                            DoCastVictim(SPELL_EARTH_SHOCK);
                            events.ScheduleEvent(EVENT_EARTH_SHOCK, 10000);
                            events.ScheduleEvent(EVENT_WRATH, 4000);
                            break;
                        case EVENT_WRATH:
                            DoCastVictim(SPELL_WRATH);
                            events.ScheduleEvent(EVENT_WRATH, 2000);
                            break;
                        default:
                            break;
                    }
                }


                DoMeleeAttackIfReady();
            }

        private:
            bool firstSleep, secondSleep;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_hazzarahAI(creature);
        }
};

// 52284
class npc_haz_nightmare : public CreatureScript
{
public:
    npc_haz_nightmare() : CreatureScript("npc_haz_nightmare"){}

    struct npc_haz_nightmareAI : public ScriptedAI
    {
        npc_haz_nightmareAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            m_uiRun = 0;
        }

        void IsSummonedBy(Unit* owner)
        {
            if (owner->GetTypeId() != TYPEID_PLAYER)
            {
                me->DespawnOrUnsummon();
                return;
            }
            me->SetReactState(REACT_PASSIVE);
            owner->GetPosition(&destpos);
            owner->GetFirstCollisionPosition(pos, 30.0f, 0.0f);
            me->NearTeleportTo(pos.m_positionX, pos.m_positionY, pos.m_positionZ, owner->GetAngle(pos.m_positionX, pos.m_positionY) - M_PI);
            me->SetSpeed(MOVE_WALK, 0.5f);
            me->SetSpeed(MOVE_RUN, 0.5f);
            m_uiRun = 3000;
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE && id == POINT_TARGET)
                if (me->ToTempSummon())
                    if (Unit *owner = me->ToTempSummon()->GetSummoner())
                    {
                        me->CastSpell(owner, SPELL_CONSUME_SOUL, true);
                        me->DespawnOrUnsummon(1000);
                    }
        }

        void UpdateAI(uint32 const diff)
        {
            if (m_uiRun)
            {
                if (m_uiRun <= diff)
                {
                    me->GetMotionMaster()->MovePoint(POINT_TARGET, destpos);
                    if (me->ToTempSummon())
                        if (Unit *owner = me->ToTempSummon()->GetSummoner())
                            me->CastSpell(owner, SPELL_WALKING_NIGHTMARE, true);
                    me->ClearUnitState(UNIT_STATE_CASTING);
                    m_uiRun = 0;
                }
                else
                    m_uiRun -= diff;
            }
        }

    private:
        Position pos, destpos;
        uint32 m_uiRun;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_haz_nightmareAI(creature);
    }
};

// 96658
class spell_haz_nightmare : public SpellScriptLoader
{
public:
    spell_haz_nightmare() : SpellScriptLoader("spell_haz_nightmare") { }

private:
    class NonPlayerTarget
    {
    public:
        explicit NonPlayerTarget(Unit *caster) : _caster(caster) {};

        bool operator()(WorldObject* target) const
        {
            return target->GetTypeId() != TYPEID_PLAYER;
        }

    private:
        Unit *_caster;
    };

    class spell_haz_nightmare_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_haz_nightmare_AuraScript);

        void HandleApplyEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit *target = GetTarget())
                target->CastSpell(target, SPELL_NIGHTMARE_ILLUSION_SUMMON, true);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_haz_nightmare_AuraScript::HandleApplyEffect, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }
    };

    class spell_haz_nightmare_SpellScript: public SpellScript
    {
        PrepareSpellScript(spell_haz_nightmare_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (!targets.empty())
                targets.remove_if(NonPlayerTarget(GetCaster()));
            if (targets.size() > 1)
                Trinity::Containers::RandomResizeList(targets, targets.size() - 1);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_haz_nightmare_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript () const
    {
        return new spell_haz_nightmare_SpellScript();
    }

    AuraScript* GetAuraScript() const
    {
        return new spell_haz_nightmare_AuraScript();
    }
};

void AddSC_boss_cache_of_madness()
{
    new boss_grilek();
    new boss_wushoolay();
    new boss_hazzarah();
    new boss_renataki();
    new npc_ritual_tiki_mask();
    new npc_ancient_artifact();
    new npc_haz_nightmare();
    new spell_cursed_portal();
    new spell_thousand_blades_damage();
    new spell_grilek_pursuit();
    new spell_grilek_fixate();
    new spell_haz_nightmare();
}
