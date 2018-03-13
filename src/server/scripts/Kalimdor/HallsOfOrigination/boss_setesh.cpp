/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "halls_of_origination.h"

enum Texts
{
    SAY_AGGRO       = 0,
    SAY_EVENT       = 1,
    SAY_SLAY        = 2,
    SAY_DEATH       = 3,
};

enum Spells
{
    //Setesh
    SPELL_CHAOS_BOLT                        = 77069,
    SPELL_SUMMON_REIGN_OF_CHAOS             = 77023,
    SPELL_REIGN_OF_CHAOS                    = 77026, // visual + damage trigger
    SPELL_REIGN_OF_CHAOS_DAMAGE             = 77030, // 89872 hc --> dummy effect need condition to setesh  ( effect #1 )
    SPELL_CHAOS_BLAST                       = 76674, // summon spell
    SPELL_CHAOS_BLAST_VISUAL                = 76676, // spliting visual need to condition to entry 41041
    SPELL_CHAOS_BLAST_AURA                  = 76681,
    SPELL_SEED_OF_CHAOS                     = 76870,
    SPELL_SUMMON_CHAOS_SEED                 = 76888, // summons entry 41126
    SPELL_SEED_OF_CHAOS_DAMAGE              = 76870,
    SPELL_CHAOS_SEED_CRYSTAL_VISUAL         = 76865, // this could trigger the damage ?
    SPELL_PORTAL_CHANNEL                    = 76784, // need condition to trigger (entry 41055)
    SPELL_NIGHTMARE_PORTAL                  = 76714, // on heroic the portal stays until encounter end, on nhc the portal can be killed
    SPELL_NIGHTMARE_PORTAL_SUMMON_VISUAL    = 77607, // casted on summon
    SPEL_CHAOS_BURN                         = 77128, // casted by chaos blast trigger (entry 41041);
    SPELL_CHAOS_BURN                        = 76684, // area damage. need more research about the spell (fuu fail hordeguides and my brain for forgeting such things)
    //Sentinel
    SPELL_VOID_BARRIER                      = 63710,
    SPELL_CHARGED_FISTS                     = 77238,
    //Seeker
    SPELL_ANTIMAGIC_PRISON                  = 76903,
};

enum Events
{
    EVENT_CHAOS_BOLT            = 1,
    EVENT_REIGN_OF_CHAOS        = 2,
    EVENT_CHAOS_BLAST           = 3,
    EVENT_SUMMON_SEED_OF_CHAOS  = 4,
    EVENT_CHAOS_PORTAL          = 5,
    EVENT_SUMMON_PORTAL         = 6,
    EVENT_CHAOS_BLAST_AURA      = 7,
};

enum Points
{
    POINT_PORTAL = 1,
};

class boss_setesh : public CreatureScript
{
    public:
        boss_setesh() : CreatureScript("boss_setesh") { }

        struct boss_seteshAI : public BossAI
        {
            boss_seteshAI(Creature* creature) : BossAI(creature, DATA_SETESH) { }

            void Reset()
            {
                _Reset();
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(SAY_AGGRO);

                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                events.ScheduleEvent(EVENT_CHAOS_BOLT, 6000);       // every 2 seconds
                events.ScheduleEvent(EVENT_REIGN_OF_CHAOS, 20000);
                events.ScheduleEvent(EVENT_CHAOS_BLAST, 15000);
                events.ScheduleEvent(EVENT_SUMMON_SEED_OF_CHAOS, 25000);
                events.ScheduleEvent(EVENT_CHAOS_PORTAL, 8000);
            }

            void KilledUnit(Unit* /*Killed*/)
            {
                Talk(SAY_SLAY);
            }

            void JustSummoned(Creature* summon)
            {
                BossAI::JustSummoned(summon);

                switch (summon->GetEntry())
                {
                    case NPC_CHAOS_PORTAL:
                        summon->AddAura(SPELL_NIGHTMARE_PORTAL, summon);
                        summon->CastSpell((Unit* ) NULL, SPELL_NIGHTMARE_PORTAL_SUMMON_VISUAL);
                        summon->SetReactState(REACT_PASSIVE);
                        if (!IsHeroic())
                            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
                        break;
                }
            }

            void MovementInform(uint32 type, uint32 pointId)
            {
                if (type != POINT_MOTION_TYPE)
                    return;
                
                switch (pointId)
                {
                    case POINT_PORTAL:
                        events.ScheduleEvent(EVENT_SUMMON_PORTAL, 1);
                        break;
                }
            }

            void UpdateAI(uint32 const diff)
            {
                events.Update(diff);

                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CHAOS_BOLT:
                            DoCastRandom(SPELL_CHAOS_BOLT, 0.0f);
                            events.ScheduleEvent(EVENT_CHAOS_BOLT, 2000);
                            break;
                        case EVENT_REIGN_OF_CHAOS:
                            DoCastAOE(SPELL_SUMMON_REIGN_OF_CHAOS);
                            events.ScheduleEvent(EVENT_REIGN_OF_CHAOS, urand(30000, 40000));
                            Talk(SAY_EVENT);
                            break;
                        case EVENT_CHAOS_BLAST:
                            DoCast(SPELL_CHAOS_BLAST);
                            DoCast(SPELL_CHAOS_BLAST_VISUAL);
                            events.ScheduleEvent(EVENT_CHAOS_BLAST, 15000);
                            break;
                        case EVENT_SUMMON_SEED_OF_CHAOS:
                            DoCast(SPELL_SUMMON_CHAOS_SEED);
                            events.ScheduleEvent(EVENT_SUMMON_SEED_OF_CHAOS, 25000);
                            break;
                        case EVENT_CHAOS_PORTAL:
                            me->SetOrientation(urand(0.0f, 3.0f));
                            //me->SetOrientation(MapManager::NormalizeOrientation(me->GetOrientation()));
                            me->GetMotionMaster()->MovePoint(POINT_PORTAL, me->GetPositionX()+cos(me->GetOrientation())*20, me->GetPositionY()+sin(me->GetOrientation())*20, me->GetPositionZ());
                            events.ScheduleEvent(EVENT_CHAOS_PORTAL, 30000);
                            events.RescheduleEvent(EVENT_CHAOS_BOLT, 7000);
                            break;
                        case EVENT_SUMMON_PORTAL:
                        {
                            float x = me->GetPositionX()+7.0f;
                            float y = me->GetPositionY()+7.0f;
                            if (Creature* trigger = me->SummonCreature(NPC_CHAOS_PORTAL, x, y, me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
                            {
                                me->SetFacingToObject(trigger);
                                DoCastAOE(SPELL_PORTAL_CHANNEL);
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
            }

            void JustDied(Unit* /*who*/)
            {
                _JustDied();
                Talk(SAY_DEATH);

                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_seteshAI(creature);
        }
};

class npc_chaos_portal : public CreatureScript
{
public:
    npc_chaos_portal() : CreatureScript("npc_chaos_portal") { }

    struct npc_chaos_portalAI : public Scripted_NoMovementAI
    {
        npc_chaos_portalAI(Creature* creature) : Scripted_NoMovementAI(creature) { }

        void Reset()
        {
            SummonTimer = 3000;
            instance = me->GetInstanceScript();
        }

        void JustSummoned(Creature* summon)
        {
            summon->SetInCombatWithZone();
            summon->SetCorpseDelay(2000);
            if (Creature* setesh = Creature::GetCreature(*me, instance->GetData64(DATA_SETESH_GUID)))
                setesh->AI()->JustSummoned(summon);
        }

        void Summon()
        {
            if (urand(0, 1))
            {
                me->SummonCreature(NPC_VOID_SEEKER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                me->SummonCreature(NPC_VOID_WURM, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
            }
            else
                me->SummonCreature(NPC_VOID_SENTINEL, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
        }

        void UpdateAI(uint32 const diff)
        {
            if (SummonTimer <= diff)
            {
                Summon();
                SummonTimer = 15000;
            } else SummonTimer -= diff;
        }

        void JustDied(Unit* /*killer*/)
        {
            me->DespawnOrUnsummon();
        }

    private:
        uint32 SummonTimer;
        InstanceScript* instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_chaos_portalAI(creature);
    }
};

class npc_setesh_void_sentinel : public CreatureScript
{
public:
    npc_setesh_void_sentinel() : CreatureScript("npc_setesh_void_sentinel") { }

    struct npc_setesh_void_sentinelAI : public ScriptedAI
    {
        npc_setesh_void_sentinelAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            BarrierTimer = 10000;
            FistTimer = 3000;
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (BarrierTimer <= diff)
            {
                DoCast(SPELL_VOID_BARRIER);
                BarrierTimer = 25000;
            } else BarrierTimer -= diff;

            if (FistTimer <= diff)
            {
                DoCast(SPELL_CHARGED_FISTS);
                FistTimer = 25000;
            } else FistTimer -= diff;
        }

        void JustDied(Unit* /*killer*/)
        {
            // used to despawn corpse immediately
            me->DespawnOrUnsummon();
        }
    private:
        uint32 BarrierTimer;
        uint32 FistTimer;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_setesh_void_sentinelAI(creature);
    }
};

class npc_setesh_void_seeker : public CreatureScript
{
public:
    npc_setesh_void_seeker() : CreatureScript("npc_setesh_void_seeker") { }

    struct npc_setesh_void_seekerAI : public ScriptedAI
    {
        npc_setesh_void_seekerAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            PrisonTimer = 5000;
        }

         void UpdateAI(uint32 const diff)
         {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (PrisonTimer <= diff)
            {
                DoCastRandom(SPELL_ANTIMAGIC_PRISON, 0.0f);
                PrisonTimer = 25000;
            } else PrisonTimer -= diff;
        }

        void JustDied(Unit* /*killer*/)
        {
            // used to despawn corpse immediately
            me->DespawnOrUnsummon();
        }
    private:
        uint32 PrisonTimer;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_setesh_void_seekerAI(creature);
    }
};

class npc_chaos_blast : public CreatureScript
{
public:
    npc_chaos_blast() : CreatureScript("npc_chaos_blast") { }

    struct npc_chaos_blastAI : public ScriptedAI
    {
        npc_chaos_blastAI(Creature* creature) : ScriptedAI(creature) { }

        void IsSummonedBy(Unit* summoner)
        {
            me->SetInCombatWithZone();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_chaos_blastAI(creature);
    }
};

class npc_seed_of_chaos : public CreatureScript
{
public:
    npc_seed_of_chaos() : CreatureScript("npc_seed_of_chaos") { }

    struct npc_seed_of_chaosAI : public Scripted_NoMovementAI
    {
        npc_seed_of_chaosAI(Creature* creature) : Scripted_NoMovementAI(creature) { }

        void IsSummonedBy(Unit* summoner)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetInCombatWithZone();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
            me->SetDisplayId(me->GetCreatureTemplate()->Modelid1);
            me->setFaction(16);
            me->AddAura(SPELL_CHAOS_SEED_CRYSTAL_VISUAL, me);
            spellHit = false;
            checkTimer = 1000;
        }

        void SpellHitTarget(Unit* target, SpellInfo const* spellInfo)
        {
            uint32 chaoesAoe = sSpellMgr->GetSpellIdForDifficulty(SPELL_SEED_OF_CHAOS_DAMAGE, me);
            if (!spellHit && spellInfo->Id == chaoesAoe)
            {
                spellHit = true; // avoid double buff
                me->AddAura(SPELL_SEED_OF_CHAOS_DAMAGE, target);
                me->DespawnOrUnsummon(500);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (checkTimer <= diff)
            {
                if (Unit* target = me->FindNearestPlayer(3.0f))
                    DoCastAOE(SPELL_SEED_OF_CHAOS_DAMAGE);
                checkTimer = 1000;
            } else checkTimer -= diff;
        }

    private:
        uint32 checkTimer;
        bool spellHit;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_seed_of_chaosAI(creature);
    }
};

class spell_hoo_chaos_blast : public SpellScriptLoader
{
public:
    spell_hoo_chaos_blast() : SpellScriptLoader("spell_hoo_chaos_blast") { }

    class spell_hoo_chaos_blast_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hoo_chaos_blast_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if([](WorldObject* target) 
            { 
                if (target->GetTypeId() != TYPEID_UNIT || target->ToUnit()->HasAura(SPELL_CHAOS_BLAST_AURA) || target->GetEntry() != 41041)
                    return true;
                return false;
            });
        }

        void HandleDummy(SpellEffIndex effIndex)
        {
            GetHitUnit()->CastSpell(GetHitUnit(), SPELL_CHAOS_BLAST_AURA, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_hoo_chaos_blast_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hoo_chaos_blast_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hoo_chaos_blast_SpellScript();
    }
};

class npc_reign_of_chaos : public CreatureScript
{
public:
    npc_reign_of_chaos() : CreatureScript("npc_reign_of_chaos") { }

    struct npc_reign_of_chaosAI : public Scripted_NoMovementAI
    {
        npc_reign_of_chaosAI(Creature* creature) : Scripted_NoMovementAI(creature) { }

        void IsSummonedBy(Unit* summoner)
        {
            me->CastSpell(me, SPELL_REIGN_OF_CHAOS, true);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            checkTimer = 1000;
        }

        void UpdateAI(uint32 const diff)
        {
            if (checkTimer <= diff)
            {
                me->SetObjectScale(me->GetFloatValue(OBJECT_FIELD_SCALE_X) + 0.1f);
                checkTimer = 1000;
            }
            else checkTimer -= diff;
        }

    private:
        uint32 checkTimer;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_reign_of_chaosAI(creature);
    }
};

void AddSC_boss_setesh()
{
    new boss_setesh();
    new npc_chaos_portal();
    new npc_setesh_void_sentinel();
    new npc_setesh_void_seeker();
    new npc_chaos_blast();
    new npc_seed_of_chaos();
    new spell_hoo_chaos_blast();
    new npc_reign_of_chaos();
}
