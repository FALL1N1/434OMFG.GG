/*
 * Copyright (C) 2013-2014 OMFG.GG Network <http://www.omfg.gg/>
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
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
#include "SpellScript.h"
#include "the_stonecore.h"

/*
 * TODO: fix jump animation (jumps every time to the tank... damage part works fine)
 */

enum Spells
{
    SPELL_CLEARALLDEBUFFS           = 34098,
    SPELL_CRYSTAL_BARRAGE           = 86881,
    SPELL_CRYSTAL_CHARGE_HC         = 92012,
    SPELL_DAMPENING_WAVE            = 82415,
    SPELL_SUBMERGE                  = 81629,
    SPELL_THRASHING_CHARGE_TELEPORT = 81839,
    SPELL_THRASHING_CHARGE          = 81801, // dummy / visual
    SPELL_THRASHING_CHARGE_SUMMON   = 81816,
    SPELL_THRASHING_CHARGE_DAMAGE   = 81828, // casted by Trigger
    SPELL_EMERGE                    = 82185, // Rock Borer
    SPELL_EMERGE_CORBORUS           = 81948,
    SPELL_CRYSTAL_SHARDS_AURA       = 80895,
    SPELL_CRYSTAL_SHARDS_TARGET     = 80912,
    SPELL_CRYSTAL_SHARDS_DAMAGE     = 92122,
    SPELL_CORBORUS_CHARGE           = 81237, // 07:43:25.00
    SPELL_CORBORUS_KNOCK_BACK       = 81235, // 07:56:56.00

    SPELL_RANDOM_AGGRO              = 92111
};

enum Phases
{
    PHASE_NORMAL                    = 1,
    PHASE_SUBMERGED
};

enum Events
{
    EVENT_CRYSTAL_BARRAGE           = 1,
    EVENT_DAMPENING_WAVE,
    EVENT_SUBMERGE,
    EVENT_EMERGE,
    EVENT_EMERGE_END,
    EVENT_THRASHING_CHARGE,
    EVENT_THRASHING_CHARGE_CAST,
    EVENT_SET_ACTIVE
};

enum misc
{
    ACTION_START_AT    = 1,
    ACTION_SET_VISIBLE = 2,
    POINT_HOME         = 3
};

class boss_corborus : public CreatureScript
{
public:
    boss_corborus() : CreatureScript("boss_corborus") {}

    struct boss_corborusAI : public BossAI
    {
        boss_corborusAI(Creature* creature) : BossAI(creature, DATA_CORBORUS)
        {
            started = false;
        }

        void Reset()
        {
            thrashingCharges = 0;
            sumGUID = 0;
            events.SetPhase(PHASE_NORMAL);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetFloatValue(UNIT_FIELD_COMBATREACH, 12.0f);
            _Reset();
            me->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_START_AT:
                    if (!started)
                    {
                        DoCast(SPELL_CORBORUS_CHARGE);
                        me->SetHomePosition(1159.375f, 880.072f, 284.991f, 3.251901f);
                        started = true;
                    }
                    me->SetReactState(REACT_AGGRESSIVE);
                    break;
                default:
                    break;
            }
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            me->SetSpeed(MOVE_WALK, 1.5f);
            events.SetPhase(PHASE_NORMAL);
            events.ScheduleEvent(EVENT_CRYSTAL_BARRAGE, urand(8000, 10000), 0, PHASE_NORMAL);
            events.ScheduleEvent(EVENT_DAMPENING_WAVE, urand(5000, 8000), 0, PHASE_NORMAL);
            events.ScheduleEvent(EVENT_SUBMERGE, 30000, 0, PHASE_NORMAL);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            switch (summon->GetEntry())
            {
                case NPC_THRASHING_CHARGE:
                    me->SetReactState(REACT_PASSIVE);
                    me->SetTarget(0);
                    me->SetFacingToObject(summon);
                    me->SetTarget(summon->GetGUID());
                    DoCast(me, SPELL_THRASHING_CHARGE, false);
                    sumGUID = summon->GetGUID();
                    events.ScheduleEvent(EVENT_THRASHING_CHARGE_CAST, PHASE_SUBMERGED);
                    break;
                case NPC_ROCK_BORER:
                    summon->CastSpell(summon, SPELL_EMERGE, false);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if(!UpdateVictim())
                return;

            events.Update(diff);

            if(me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if(uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_CRYSTAL_BARRAGE:
                        DoCastRandom(SPELL_CRYSTAL_BARRAGE, 50.0f);
                        events.ScheduleEvent(EVENT_CRYSTAL_BARRAGE, urand(10000, 12000), 0, PHASE_NORMAL);
                        break;
                    case EVENT_DAMPENING_WAVE:
                        DoCast(SPELL_DAMPENING_WAVE);
                        events.ScheduleEvent(EVENT_DAMPENING_WAVE, urand(10000, 12000), 0, PHASE_NORMAL);
                        break;
                    case EVENT_SUBMERGE:
                        DoCast(SPELL_CLEARALLDEBUFFS);
                        thrashingCharges = 0;
                        me->SetFloatValue(UNIT_FIELD_COMBATREACH, 3.0f);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        me->SetReactState(REACT_PASSIVE);
                        me->SetUInt64Value(UNIT_FIELD_TARGET, 0);
                        me->RemoveAllAuras();
                        me->StopMoving();
                        me->SetWalk(false);
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MoveIdle();
                        DoCast(SPELL_SUBMERGE);
                        events.SetPhase(PHASE_SUBMERGED);
                        events.RescheduleEvent(EVENT_THRASHING_CHARGE, 4000, 0, PHASE_SUBMERGED);
                        break;
                    case EVENT_EMERGE:
                        events.SetPhase(PHASE_NORMAL);
                        events.RescheduleEvent(EVENT_DAMPENING_WAVE, 3000, 0, PHASE_NORMAL);
                        events.RescheduleEvent(EVENT_CRYSTAL_BARRAGE, 4500, 0, PHASE_NORMAL);
                        events.ScheduleEvent(EVENT_EMERGE_END, 2500, 0, PHASE_NORMAL);
                        DoCast(SPELL_EMERGE_CORBORUS);
                        break;
                    case EVENT_EMERGE_END:
                        me->RemoveAllAuras();
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->SetFloatValue(UNIT_FIELD_COMBATREACH, 12.0f);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                        if(Unit* victim = me->getVictim())
                            DoStartMovement(victim);
                        events.ScheduleEvent(EVENT_SUBMERGE, 60000, 0, PHASE_NORMAL);
                        break;
                    case EVENT_THRASHING_CHARGE:
                        summons.DespawnEntry(NPC_THRASHING_CHARGE);
                        DoCastRandom(SPELL_THRASHING_CHARGE_TELEPORT, 100.0f);
                        DoCast(me, SPELL_THRASHING_CHARGE_SUMMON, true);
                        break;
                    case EVENT_THRASHING_CHARGE_CAST:
                        me->SetReactState(REACT_AGGRESSIVE);
                        ++thrashingCharges;
                        if (Creature *summon = Unit::GetCreature(*me, sumGUID))
                            summon->CastSpell(summon, SPELL_THRASHING_CHARGE_DAMAGE, true);
                        if(thrashingCharges >= 3)
                            events.ScheduleEvent(EVENT_EMERGE, 1000, 0, PHASE_SUBMERGED);
                        else
                            events.ScheduleEvent(EVENT_THRASHING_CHARGE, 1000, 0, PHASE_SUBMERGED);
                        break;
                    default:
                        break;
                }
            }

            if(events.IsInPhase(PHASE_NORMAL))
                DoMeleeAttackIfReady();

            EnterEvadeIfOutOfCombatArea(diff);
        }

    private:
        uint8 thrashingCharges;
        bool started;
        uint64 sumGUID;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new boss_corborusAI(creature);
    }
};

class spell_crystal_barrage : public SpellScriptLoader
{
public:
        spell_crystal_barrage() : SpellScriptLoader("spell_crystal_barrage") { }

        class spell_crystal_barrage_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_crystal_barrage_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if(Unit * caster = GetCaster())
                    for (int i = 0; i < 10; i++)
                        if (caster->GetMap()->IsHeroic())
                            caster->CastSpell(caster, SPELL_CRYSTAL_CHARGE_HC, true);
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_crystal_barrage_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_crystal_barrage_AuraScript();
        }
};

class npc_crystal_shard : public CreatureScript
{
public:
    npc_crystal_shard() : CreatureScript("npc_crystal_shard") {}

    struct npc_crystal_shardAI : public BossAI
    {
        npc_crystal_shardAI(Creature * creature) : BossAI(creature, DATA_CORBORUS)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
        }

        void Reset()
        {
            events.Reset();
            events.ScheduleEvent(EVENT_SET_ACTIVE, 3000);
        }

        void SpellHitTarget(Unit * /*target*/, const SpellInfo * spell)
        {
            if(spell->Id == SPELL_CRYSTAL_SHARDS_TARGET)
            {
                DoCast(SPELL_CRYSTAL_SHARDS_DAMAGE);
                me->DespawnOrUnsummon(250);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if(uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_SET_ACTIVE:
                        DoCast(SPELL_CRYSTAL_SHARDS_AURA);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                        me->SetInCombatWithZone();
                        DoCast(SPELL_RANDOM_AGGRO);
                        break;
                }
            }
        }

    private:
        EventMap events;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_crystal_shardAI(creature);
    }
};

class AreaTrigger_at_rockdoor_break : public AreaTriggerScript
{
    public:
        AreaTrigger_at_rockdoor_break() : AreaTriggerScript("at_rockdoor_break") {}

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/)
        {
            if (InstanceScript* instance = player->GetInstanceScript())
            {
                if (GameObject* go = ObjectAccessor::GetGameObject(*player, instance->GetData64(DATA_ROCKDOOR)))
                {
                    if (go->getLootState() != GO_READY)
                        return false;

                    go->UseDoorOrButton();
                    go->EnableCollision(false);
                }

                    if (Creature* corborus = ObjectAccessor::GetCreature(*player, instance->GetData64(DATA_CORBORUS)))
                        corborus->AI()->DoAction(ACTION_START_AT);
            }

            return false;
        }
};

class spell_corborus_sum_triggered : public SpellScriptLoader
{
public:
    spell_corborus_sum_triggered() : SpellScriptLoader("spell_corborus_sum_triggered")
    {
    }

    class script_impl : public SpellScript
    {
        PrepareSpellScript(script_impl);

        bool Load()
        {
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            if (unitList.empty())
                return;
            std::list<WorldObject*>::const_iterator itr = unitList.begin();
            advance(itr, rand() % unitList.size());
            if (WorldObject * tar = *itr)
            {
                unitList.clear();
                unitList.push_back(tar);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new script_impl();
    }

};

void AddSC_boss_corborus()
{
    new boss_corborus;
    new spell_crystal_barrage();
    new npc_crystal_shard();
    new AreaTrigger_at_rockdoor_break();
    new spell_corborus_sum_triggered();
};
