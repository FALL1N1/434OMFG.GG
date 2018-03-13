/*
* Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
*

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

/*
##### Script Info #####
Author: Nagash and Kazlim
Progress: 80%
*/

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


#define SAY_AGGRO    "You are forbidden from my master's domain, mortals." 
#define SOU_AGGRO    24441
#define SAY_DEATH    "Mortal filth... the master's keep is forbidden..." 
#define SOU_DEATH    24444
#define SAY_DECIM    "By the Firelord's command, you, too, shall perish!"
#define SOU_DECIM    24447
#define SAY_INFER    "Burn beneath my molten fury!" 
#define SOU_INFER    24459
#define SAY_KILL1    "You have been judged."  
#define SOU_KILL1    24449
#define SAY_KILL2    "Behold your weakness." 
#define SOU_KILL2    24451
#define SAY_KILL3    "None shall pass!" 
#define SOU_KILL3    24452
#define SAY_ENRAG    "Your flesh is forfeit to the fires of this realm." 
#define SOU_ENRAG    24450
#define SAY_SHARD    "Fool mortals. Hurl yourselves into your own demise!" 
#define SOU_SHARD    24446

enum Spells
{
    SPELL_BLAZE_OF_GLORY = 99252,
    SPELL_INCENDIARY_SOUL = 99369,
    SPELL_SHARDS_OF_TORMENT = 99259,
    SPELL_SUMMON_SHARDS_OF_TORMENT = 99260,
    SPELL_TORMENT = 99255,
    SPELL_SHARD_VISUAL_1 = 99258,
    SPELL_SHARD_VISUAL_2 = 99254,
    SPELL_TORMENTED = 99257,
    SPELL_WAVE_OF_TORMENT = 99261,
    SPELL_VITAL_SPARK = 99262,
    SPELL_VITAL_FLAME = 99263,
    SPELL_DECIMATION_BLADE = 99352,
    SPELL_DECIMATING_STRIKE = 99353,
    SPELL_INFERNO_BLADE = 99350,
    SPELL_INFERNO_STRIKE = 99351,
    SPELL_COUNTDOWN = 99516,
    SPELL_COUNTDOWN_LINK = 99519,
    SPELL_COUNTDOWN_DMG = 99518,
    SPELL_BERSERK = 26662
};

enum Events
{
    EVENT_BLAZE_OF_GLORY = 1,
    EVENT_SHARDS_OF_TORMENT,
    EVENT_SHARD_VISUAL,
    EVENT_TORMENT,
    EVENT_INFERNO_BLADE,
    EVENT_INFERNO_STRIKE,
    EVENT_DECIMATION_BLADE,
    EVENT_DECIMATING_STRIKE,
    EVENT_COUNTDOWN,
    EVENT_BERSERK
};

class boss_baleroc : public CreatureScript
{
public:
    boss_baleroc() : CreatureScript("boss_baleroc") {}

    struct boss_balerocAI : public BossAI
    {
        boss_balerocAI(Creature * creature) : BossAI(creature, DATA_BALEROC), summons(me)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        SummonList summons;
        uint32 killtimer;

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }

        void EnterCombat(Unit* /*who*/)
        {
            if (instance)
            {
                instance->SetData(DATA_BALEROC_EVENT, IN_PROGRESS);
            }
            _EnterCombat();
            DoZoneInCombat(me);
            me->MonsterYell(SAY_AGGRO, 0, 0);
            DoPlaySoundToSet(me, SOU_AGGRO);

            events.ScheduleEvent(EVENT_BERSERK, 350 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_BLAZE_OF_GLORY, 7000);
            events.ScheduleEvent(EVENT_SHARDS_OF_TORMENT, 10000);
            events.ScheduleEvent(EVENT_INFERNO_BLADE, 30000);

            if (me->GetMap()->IsHeroic())
                events.ScheduleEvent(EVENT_COUNTDOWN, 30000);
        }

        void Reset()
        {
            if (instance)
            {
                instance->SetData(DATA_BALEROC_EVENT, NOT_STARTED);
            }
            killtimer = 0;
            events.Reset();
            _Reset();
            summons.DespawnAll();

            if (const CreatureTemplate* cinfo = me->GetCreatureTemplate())
            {
                me->SetCanDualWield(true);
                me->SetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, cinfo->mindmg);
                me->SetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, cinfo->maxdmg);
            }

            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLAZE_OF_GLORY);
        }

        void JustDied(Unit* killer)
        {
            if (instance)
            {
                instance->SetData(DATA_BALEROC_EVENT, DONE);
            }
            if (me->GetMap()->IsHeroic())
            {// this will give the achievement to players in heroic difficulty
                Map::PlayerList const& PlayerList = instance->instance->GetPlayers();
                AchievementEntry const* baleroc = sAchievementStore.LookupEntry(5805);
                for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                {
                    if (Player* player = itr->getSource())
                    {
                        player->CompletedAchievement(baleroc);
                    }
                }
            }
            _JustDied();
            summons.DespawnAll();
            me->MonsterYell(SAY_DEATH, 0, 0);
            DoPlaySoundToSet(me, SOU_DEATH);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLAZE_OF_GLORY);

            std::list<Player*> targets;
            Trinity::AnyPlayerInObjectRangeCheck u_check(me, 300.0f);
            Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, targets, u_check);
            me->VisitNearbyWorldObject(500.0f, searcher);

            if (GameObject* bridge = me->FindNearestGameObject(209251, 500))
            {
                bridge->DestroyForNearbyPlayers();
                bridge->SetDestructibleState(GO_DESTRUCTIBLE_DESTROYED);
                //bridge->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_DESTROYED); // SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_DESTROYED);
                //bridge->setActive(true);
            }
        }

        void KilledUnit(Unit* victim)
        {
            if (!victim || victim->GetTypeId() != TYPEID_PLAYER || killtimer > 0)
                return;
            int caso = urand(1, 34);
            switch (caso)
            {
            case 1:
                me->MonsterYell(SAY_KILL1, 0, 0);
                DoPlaySoundToSet(me, SOU_KILL1);
                break;
            case 2:
                me->MonsterYell(SAY_KILL2, 0, 0);
                DoPlaySoundToSet(me, SOU_KILL2);
                break;
            case 3:
                me->MonsterYell(SAY_KILL3, 0, 0);
                DoPlaySoundToSet(me, SOU_KILL3);
                break;
            default:
                break;
            }
            killtimer = 4000;
        }

        void DoMeleeAttacksIfReady()
        {
            if (me->HasAura(SPELL_DECIMATION_BLADE) || me->HasAura(SPELL_INFERNO_BLADE))
                return;

            if (me->IsWithinMeleeRange(me->getVictim()) && !me->IsNonMeleeSpellCasted(false))
            {
                //Check for base attack
                if (me->isAttackReady() && me->getVictim())
                {
                    me->AttackerStateUpdate(me->getVictim());
                    me->resetAttackTimer();
                }
                //Check for offhand attack
                if (me->isAttackReady(OFF_ATTACK) && me->getVictim())
                {
                    me->AttackerStateUpdate(me->getVictim(), OFF_ATTACK);
                    me->resetAttackTimer(OFF_ATTACK);
                }
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (killtimer >= diff)
                killtimer -= diff;

            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                case EVENT_BLAZE_OF_GLORY:
                    me->CastSpell(me->getVictim(), SPELL_BLAZE_OF_GLORY, false);
                    me->CastSpell(me, SPELL_INCENDIARY_SOUL, false);
                    events.ScheduleEvent(EVENT_BLAZE_OF_GLORY, urand(8000, 14000));
                    break;
                case EVENT_SHARDS_OF_TORMENT:
                    if (roll_chance_i(33))
                    {
                        me->MonsterYell(SAY_SHARD, 0, 0);
                        DoPlaySoundToSet(me, SOU_SHARD);
                    }

                    if (GetDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL || GetDifficulty() == RAID_DIFFICULTY_25MAN_HEROIC)
                    {
                        me->SummonCreature(NPC_SHARD_OF_TORMENT, me->GetPositionX() + 5 * cos(me->GetOrientation() + 3.14), me->GetPositionY() + 5 * sin(me->GetOrientation() + 3.14), me->GetPositionZ() + 5.0f, me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 30000);
                        me->SummonCreature(NPC_SHARD_OF_TORMENT, me->GetPositionX() + 25 * cos(me->GetOrientation() + 3.14), me->GetPositionY() + 25 * sin(me->GetOrientation() + 3.14), me->GetPositionZ() + 5.0f, me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 30000);
                    }
                    else
                        me->SummonCreature(NPC_SHARD_OF_TORMENT, me->GetPositionX() + 15 * cos(me->GetOrientation() + 3.14), me->GetPositionY() + 15 * sin(me->GetOrientation() + 3.14), me->GetPositionZ() + 5.0f, me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 30000);

                    me->CastSpell(me, SPELL_SHARDS_OF_TORMENT, false);
                    events.ScheduleEvent(EVENT_SHARDS_OF_TORMENT, 35000);
                    break;
                case EVENT_INFERNO_BLADE:
                    me->MonsterYell(SAY_INFER, 0, 0);
                    DoPlaySoundToSet(me, SOU_INFER);
                    me->SetCanDualWield(false);
                    me->CastSpell(me, SPELL_INFERNO_BLADE, false);
                    events.ScheduleEvent(EVENT_INFERNO_STRIKE, 3000);
                    events.ScheduleEvent(EVENT_DECIMATION_BLADE, 45000);
                    break;
                case EVENT_INFERNO_STRIKE:
                    if (me->HasAura(SPELL_INFERNO_BLADE))
                    {
                        me->CastSpell(me->getVictim(), SPELL_INFERNO_STRIKE, false);
                        events.ScheduleEvent(EVENT_INFERNO_STRIKE, 1500);
                    }
                    else
                    {
                        me->SetCanDualWield(true);
                        events.CancelEvent(EVENT_INFERNO_STRIKE);
                    }
                    break;
                case EVENT_DECIMATION_BLADE:
                    me->MonsterYell(SAY_DECIM, 0, 0);
                    DoPlaySoundToSet(me, SOU_DECIM);
                    me->SetCanDualWield(false);
                    me->CastSpell(me, SPELL_DECIMATION_BLADE, false);
                    events.ScheduleEvent(EVENT_DECIMATING_STRIKE, 4000);
                    events.ScheduleEvent(EVENT_INFERNO_BLADE, 45000);
                    break;
                case EVENT_DECIMATING_STRIKE:
                    if (me->HasAura(SPELL_DECIMATION_BLADE))
                    {
                        me->CastSpell(me->getVictim(), SPELL_DECIMATING_STRIKE, false);
                        events.ScheduleEvent(EVENT_DECIMATING_STRIKE, 5000);
                    }
                    else
                    {
                        me->SetCanDualWield(true);
                        events.CancelEvent(EVENT_DECIMATING_STRIKE);
                    }
                    break;
                case EVENT_COUNTDOWN:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true, -SPELL_BLAZE_OF_GLORY))
                    {

                        target->AddAura(SPELL_COUNTDOWN, target);

                        if (Unit* targetLink = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true, -SPELL_COUNTDOWN || -SPELL_BLAZE_OF_GLORY))
                        {
                            targetLink->AddAura(SPELL_COUNTDOWN, targetLink);
                            target->CastSpell(targetLink, SPELL_COUNTDOWN_LINK, false);
                        }
                    }
                    events.ScheduleEvent(EVENT_COUNTDOWN, 45000);
                    break;
                case EVENT_BERSERK:
                    me->CastSpell(me, SPELL_BERSERK, false);
                    me->MonsterYell(SAY_ENRAG, 0, 0);
                    DoPlaySoundToSet(me, SOU_ENRAG);
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_balerocAI(creature);
    }
};

class npc_shard_of_torment : public CreatureScript
{
public:
    npc_shard_of_torment() : CreatureScript("npc_shard_of_torment") { }

    struct npc_shard_of_tormentAI : public ScriptedAI
    {
        npc_shard_of_tormentAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        uint32 targetGUID;

        void Reset()
        {
            events.Reset();
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_PASSIVE);
            me->Attack(summoner->getVictim(), true);
            me->AttackStop();
            me->StopMoving();
            me->CastSpell(me, SPELL_SHARD_VISUAL_1, false);

        }

        void EnterCombat(Unit* who)
        {
            DoZoneInCombat(me);
            events.ScheduleEvent(EVENT_SHARD_VISUAL, 3000);
            events.ScheduleEvent(EVENT_TORMENT, 6000);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                case EVENT_SHARD_VISUAL:
                    me->RemoveAura(SPELL_SHARD_VISUAL_1);
                    me->CastSpell(me, SPELL_SHARD_VISUAL_2, false);
                    break;
                case EVENT_TORMENT:
                    if (Unit* target = SelectTarget(SELECT_TARGET_NEAREST, 0, 15.0f, true))
                    {
                        me->SetFacingToObject(target);
                        me->CastSpell(target, SPELL_TORMENT, false);
                    }
                    else
                        me->CastSpell(target, SPELL_WAVE_OF_TORMENT, false);

                    events.ScheduleEvent(EVENT_TORMENT, 1000);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_shard_of_tormentAI(creature);
    }
};

class spell_countdown : public SpellScriptLoader
{
public:
    spell_countdown() : SpellScriptLoader("spell_countdown") { }

    class spell_countdown_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_countdown_AuraScript);

        void HandleRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
        {
            if (Unit* owner = GetUnitOwner())
                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                    owner->CastSpell(owner, SPELL_COUNTDOWN_DMG, false);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_countdown_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_countdown_AuraScript();
    }
};

class spell_countdown_tick : public SpellScriptLoader
{
public:
    spell_countdown_tick() : SpellScriptLoader("spell_countdown_tick") { }

    class spell_countdown_tick_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_countdown_tick_SpellScript)

            void HandleScript(SpellEffIndex effIndex)
        {
            if (Unit* caster = GetCaster())
            {
                if (Unit* target = GetHitUnit())
                {
                    if (target->HasAura(99516))
                    {
                        if (target != caster)
                        {
                            caster->RemoveAura(99519);
                            target->RemoveAura(99519);
                            target->RemoveAura(99516);
                            caster->RemoveAura(99516);
                        }
                    }
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_countdown_tick_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_countdown_tick_SpellScript();
    }
};

class spell_vital_flame : public SpellScriptLoader
{
public:
    spell_vital_flame() : SpellScriptLoader("spell_vital_flame") { }

    class spell_vital_flame_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_vital_flame_AuraScript);

        void HandleApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* owner = GetUnitOwner())
            {
                if (Aura* aura = owner->GetAura(SPELL_VITAL_SPARK))
                    stack = aura->GetStackAmount();

                owner->RemoveAura(SPELL_VITAL_SPARK);
            }
        }

        void HandleRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* owner = GetUnitOwner())
            {
                if (Aura* aura = owner->GetAura(SPELL_VITAL_SPARK))
                    stack += aura->GetStackAmount();

                owner->SetAuraStack(SPELL_VITAL_SPARK, owner, stack);
            }
        }

    private:
        uint32 stack;

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_vital_flame_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_MOD_HEALING_DONE_PERCENT_WITH_SPECIFIC_AURA, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_vital_flame_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_MOD_HEALING_DONE_PERCENT_WITH_SPECIFIC_AURA, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_vital_flame_AuraScript();
    }
};

class spell_torment : public SpellScriptLoader
{
public:
    spell_torment() : SpellScriptLoader("spell_torment") { }

    class spell_torment_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_torment_SpellScript);

        void CalculateDamage(SpellEffIndex /*effIndex*/)
        {
            if (Unit* target = GetHitUnit())
            {
                if (AuraEffect* aurEff = target->GetDummyAuraEffect(SPELLFAMILY_GENERIC, 2901, EFFECT_1))
                {
                    //Recalculating the damage by using effectamount will wipe every damage reduction effect
                    int32 damage = this->GetHitDamage();
                    damage *= aurEff->GetBase()->GetStackAmount();
                    if (AuraEffect* aurEff = target->GetAuraEffect(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, SPELLFAMILY_GENERIC, 173, EFFECT_0))
                        AddPct(damage, aurEff->GetAmount());
                    SetHitDamage(damage);
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_torment_SpellScript::CalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    class spell_torment_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_torment_AuraScript);

        void HandleRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* owner = GetUnitOwner())
                owner->CastSpell(owner, SPELL_TORMENTED, false);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_torment_AuraScript::HandleRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_torment_SpellScript();
    }

    AuraScript* GetAuraScript() const
    {
        return new spell_torment_AuraScript();
    }
};

class spell_decimating_strike : public SpellScriptLoader
{
public: spell_decimating_strike() : SpellScriptLoader("spell_decimating_strike") { }

        class spell_decimating_strike_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_decimating_strike_SpellScript);

            void Damage()
            {
                Unit* target = GetHitUnit();
                if (!target)
                    return;
                uint32 damage = target->GetMaxHealth()* 0.9f;
                if (damage < 250000)
                    damage = 250000;
                if (AuraEffect* aurEff = target->GetAuraEffect(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, SPELLFAMILY_GENERIC, 173, EFFECT_0))
                    AddPct(damage, aurEff->GetAmount());
                SetHitDamage(damage);
            }

            void Register()
            {
                BeforeHit += SpellHitFn(spell_decimating_strike_SpellScript::Damage);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_decimating_strike_SpellScript();
        }
};

class npc_magma_orb : public CreatureScript
{
public:
    npc_magma_orb() : CreatureScript("npc_magma_orb") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (InstanceScript* instance = player->GetInstanceScript())
        {
            if (instance->GetData(DATA_BALEROC_EVENT) == DONE)
            {
                if (GameObject* bridge = creature->FindNearestGameObject(209251, 500))
                {
                    bridge->DestroyForNearbyPlayers();
                    bridge->SetDestructibleState(GO_DESTRUCTIBLE_DESTROYED);
                    // creature->DealDamage(creature, creature->GetMaxHealth() + 1);
                }
            }
        }
        return true;
    }
};

void AddSC_boss_baleroc()
{
    new boss_baleroc();
    new npc_shard_of_torment();
    new npc_magma_orb();
    new spell_countdown();
    new spell_countdown_tick();
    new spell_vital_flame();
    new spell_torment();
    new spell_decimating_strike();
}