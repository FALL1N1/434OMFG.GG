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
#include "blackrock_caverns.h"

enum texts
{
    TALK_AGGRO                  = 0,
    TALK_SKULLCRACKER           = 1,
    TALK_SLAY                   = 2,
    TALK_DEATH                  = 3,
};
enum Spells
{
    SPELL_CALL_FOR_HELP         = 82137,
    SPELL_QUAKE                 = 75272,
    SPELL_CHAINS_OF_WOE_SUMMON  = 75539,
    SPELL_CHAINS_OF_WOE_CHANNEL = 75441,
    SPELL_CHAINS_OF_WOE_TELEPORT = 75464,
    SPELL_CHAINS_OF_WOE_AURA    = 82192,
    SPELL_THE_SKULLCRACKER      = 75543,
    SPELL_WOUNDING_STRIKE       = 69651,
};

enum Events
{
    EVENT_QUAKE                 = 1,
    EVENT_WOUNDING_STRIKE       = 2,
    EVENT_SKULLCRACKER          = 3,
};

enum Groups
{
    GROUP_PHASE_NORMAL          = 1,
};

#define ACHIEVEMENT_CRUSHING_BONES_AND_CRACKING_SKULLS  5281

class boss_romogg_bonecrusher : public CreatureScript
{
    public:
        boss_romogg_bonecrusher() : CreatureScript("boss_romogg_bonecrusher") { }

        struct boss_romogg_bonecrusherAI : public BossAI
        {
            boss_romogg_bonecrusherAI(Creature* creature) : BossAI(creature, BOSS_ROMOGG_BONECRUSHER){}

            void Reset()
            {
                isFirst = true;
                isSec = true;
                earthKillCount = 0;
                DespawnCreatures(NPC_ANGERED_EARTH);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                _Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();

                if (IsHeroic())
                    DoCastAOE(SPELL_CALL_FOR_HELP);

                events.ScheduleEvent(EVENT_WOUNDING_STRIKE, 6000, GROUP_PHASE_NORMAL);
                events.ScheduleEvent(EVENT_QUAKE, 19000, GROUP_PHASE_NORMAL);
                Talk(TALK_AGGRO);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }

            void JustDied(Unit* /*killer*/)
            {
                Talk(TALK_DEATH);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                _JustDied();

                if (IsHeroic() && earthKillCount >= 10)
                    instance->DoCompleteAchievement(ACHIEVEMENT_CRUSHING_BONES_AND_CRACKING_SKULLS);
            }

            void KilledUnit(Unit* who)
            {
                if (who->GetTypeId() == TYPEID_PLAYER)
                    Talk(TALK_SLAY);

                if (who->GetTypeId() == TYPEID_UNIT)
                    if (who->ToCreature()->GetEntry() == NPC_ANGERED_EARTH)
                        earthKillCount++;
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
                summon->setActive(true);

                if (summon->GetEntry() == NPC_CHAINS_OF_WOE)
                {
                    Map::PlayerList const& players = me->GetMap()->GetPlayers();
                    if (!players.isEmpty())
                        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                            if (Player* player = itr->getSource())
                                if (player->isAlive())
                                    player->CastSpell(summon, SPELL_CHAINS_OF_WOE_TELEPORT, true);

                    summon->CastSpell((Unit*)NULL, SPELL_CHAINS_OF_WOE_CHANNEL, true);
                }
            }

            void DespawnCreatures(uint32 entry)
            {
                std::list<Creature*> creatures;
                GetCreatureListWithEntryInGrid(creatures, me, entry, 1000.0f);

                if (creatures.empty())
                    return;

                for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                    (*iter)->DespawnOrUnsummon();
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage)
            {
                if (me->HealthBelowPct(66) && isFirst || me->HealthBelowPct(33) && isSec)
                {
                    isFirst ? isFirst = false : isSec = false;
                    events.DelayEvents(14500, GROUP_PHASE_NORMAL); // woe cast 2 sec, skullcracker 12sec + delay
                    events.ScheduleEvent(EVENT_SKULLCRACKER, 2100);
                    me->CastStop();
                    DoCastAOE(SPELL_CHAINS_OF_WOE_SUMMON);
                }
            }

            void SummonedCreatureDies(Creature* summon, Unit* /*killer*/)
            {
                if (summon->GetEntry() == NPC_CHAINS_OF_WOE)
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CHAINS_OF_WOE_AURA);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_QUAKE:
                            DoCastVictim(SPELL_QUAKE);
                            events.ScheduleEvent(EVENT_QUAKE, 19000, GROUP_PHASE_NORMAL);
                            break;
                        case EVENT_WOUNDING_STRIKE:
                            DoCastVictim(SPELL_WOUNDING_STRIKE);
                            events.ScheduleEvent(EVENT_WOUNDING_STRIKE, 6000, GROUP_PHASE_NORMAL);
                            break;
                        case EVENT_SKULLCRACKER:
                            Talk(TALK_SKULLCRACKER);
                            DoCastAOE(SPELL_THE_SKULLCRACKER);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        private:
            uint8 isFirst;
            uint8 isSec;
            uint8 earthKillCount;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_romogg_bonecrusherAI(creature);
        }
};

class spell_chains_of_woe_triggered : public SpellScriptLoader
{
public:
    spell_chains_of_woe_triggered() : SpellScriptLoader("spell_chains_of_woe_triggered") { }

    class spell_chains_of_woe_triggered_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_chains_of_woe_triggered_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            GetHitUnit()->CastSpell(GetHitUnit(), uint32(GetEffectValue()), true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_chains_of_woe_triggered_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_chains_of_woe_triggered_SpellScript();
    }
};

class spell_romogg_quake : public SpellScriptLoader
{
public:
    spell_romogg_quake() : SpellScriptLoader("spell_romogg_quake") { }

    class spell_romogg_quake_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_romogg_quake_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            GetHitUnit()->CastSpell(GetHitUnit(), uint32(GetEffectValue()), true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_romogg_quake_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    class spell_romogg_quake_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_romogg_quake_AuraScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_QUAKE))
                return false;
            return true;
        }

        void OnPeriodic(AuraEffect const* /*aurEff*/)
        {
            Unit* target = GetTarget();
            std::list<Creature*> quakeTriggers;
            GetCreatureListWithEntryInGrid(quakeTriggers, target, NPC_QUAKE_TRIGGER, 200.0f);
            if (!quakeTriggers.empty())
            {
                quakeTriggers.sort(Trinity::ObjectDistanceOrderPred(target, true));
                Creature* quake = quakeTriggers.front();

                if (target->GetExactDist2d(quake) >= 2.0f)
                    Remove(AURA_REMOVE_BY_DEFAULT);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_romogg_quake_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_romogg_quake_SpellScript();
    }

    AuraScript* GetAuraScript() const
    {
        return new spell_romogg_quake_AuraScript();
    }
};

enum raz_spells
{
    SPELL_AGGRO_NEARBY_TARGETS = 80189, //80196,
    SPELL_AGGRO_NEARBY_TARGET_TRIGGER = 80196,
    SPELL_JUMP_FROM_THE_CAGE = 79720, // dest  X: 227.6 Y: 949.8 Z: 192.6
    SPELL_FURIOUS_BALAYAGE = 80206, // Time: 09/27/1970 18:13:08.000
    SPELL_FURIOUS_RAGE = 80218, //09/27/1970 18:33:10.000
    SPELL_FURIOUS_BALAYAGE_DUMMY = 80340, // Time: 09/27/1970 18:33:10.000 target entry 39978
};

static const Position razWaypoints[5] =
{
    { 252.13f, 908.57f, 191.07f, 0.0f },
    { 286.43f, 948.1f, 191.0f, 0.0f },
    { 354.95f, 947.9f, 191.11f, 0.0f },
    { 382.526f, 948.79f, 192.38f, 0.0f },
    { 408.99f, 948.82f, 194.0f, 0.0f }
};

enum razEnums
{
    EVENT_FURIOUS_RAGE = 1,
    EVENT_FURIOUS_BALAYAGE,
};

#define ACTION_CHANGE_PHASE 42

class npc_raz_the_crazed : public CreatureScript
{
    public:
        npc_raz_the_crazed() : CreatureScript("npc_raz_the_crazed") { }

        struct npc_raz_the_crazedAI : public ScriptedAI
        {
            npc_raz_the_crazedAI(Creature* creature) : ScriptedAI(creature), instance(creature->GetInstanceScript())
            {
                phaseId = 0;
                start = false;
            }

            void Reset()
            {
                timerEvent = 1000;
                events.ScheduleEvent(EVENT_FURIOUS_RAGE, 20000);
                events.ScheduleEvent(EVENT_FURIOUS_BALAYAGE, 2000);
                me->CastSpell(me, SPELL_AGGRO_NEARBY_TARGETS, true);
            }

            void DoAction(const int32 act)
            {
                switch (act)
                {
                    case ACTION_RAZ_START_EVENT:
                        start = true;
                        me->CastSpell(me, SPELL_JUMP_FROM_THE_CAGE, true);
                        break;
                    case ACTION_CHANGE_PHASE:
                        if (phaseId == 0)
                        {
                            start = false;
                            me->RemoveAurasDueToSpell(SPELL_AGGRO_NEARBY_TARGETS);
                            me->GetMotionMaster()->MoveJump(412.86f, 817.87f, 103.50f, 50.0f, 50.0f);
                            phaseId = 1;
                        }
                        else if (phaseId == 1)
                        {
                            start = true;
                            phaseId = 2;
                        }
                        else
                            me->DespawnOrUnsummon();
                        break;
                    default:
                        break;
                }
            }

            void MoveInLineOfSight(Unit *who)
            {
                if (phaseId != 1 || me->isMoving() || me->GetDistance(who) > 3.0f)
                    return;
                DoAction(ACTION_CHANGE_PHASE);
            }

            void EnterCombat(Unit* /*who*/)
            {
            }

            void JustDied(Unit* /*killer*/)
            {
            }

            void KilledUnit(Unit* who)
            {
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage)
            {
            }

            Unit *GetNextTarget(float checkdist)
            {
                Unit *target = NULL;
                float dist = 0;
                if (target = me->FindNearestCreature(39985, checkdist))
                    dist = me->GetDistance(target);
                if (Creature *c = me->FindNearestCreature(39980, checkdist))
                {
                    float tempdist = me->GetDistance(c);
                    if (tempdist < dist || dist == 0)
                        target = c;
                }
                if (Creature *c = me->FindNearestCreature(39978, checkdist))
                {
                    float tempdist = me->GetDistance(c);
                    if (tempdist < dist || dist == 0)
                        target = c;
                }
                if (Creature *c = me->FindNearestCreature(39982, checkdist))
                {
                    float tempdist = me->GetDistance(c);
                    if (tempdist < dist || dist == 0)
                        target = c;
                }
                if (Creature *c = me->FindNearestCreature(39987, checkdist))
                {
                    float tempdist = me->GetDistance(c);
                    if (tempdist < dist || dist == 0)
                        target = c;
                }
                return target;
            }

            void UpdateAI(uint32 const diff)
            {
                if (!me->HasAura(SPELL_AGGRO_NEARBY_TARGETS))
                    me->CastSpell(me, SPELL_AGGRO_NEARBY_TARGETS, true);

                if (!start)
                    return;

                if (!UpdateVictim() && !me->isMoving()) // this is a fucking hack ! i don't really manage to make it working like on video without this, not as blizz as it will be without this
                {
                    if (timerEvent <= diff)
                    {
                        if (Unit *target = GetNextTarget(200))
                        {
                            if (target->GetPositionZ() < 180.0f && phaseId == 0)
                            {
                                DoAction(ACTION_CHANGE_PHASE);
                            }
                            else
                            {
                                Position pos;
                                target->GetPosition(&pos);
                                me->SetHomePosition(pos);
                                target->setFaction(2111);
                                me->AddThreat(target, 100000);
                                me->AI()->AttackStart(target);
                            }
                        }
                        else
                            DoAction(ACTION_CHANGE_PHASE);
                        timerEvent = 500;
                    }
                    else
                        timerEvent -= diff;
                    return;
                }

                if (!UpdateVictim())
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FURIOUS_RAGE:
                            me->CastSpell(me, SPELL_FURIOUS_RAGE);
                            events.ScheduleEvent(EVENT_FURIOUS_RAGE, 20000);
                            break;
                        case EVENT_FURIOUS_BALAYAGE:
                            if (me->getVictim() && me->GetDistance(me->getVictim()) < 1.0f)
                                DoCastAOE(SPELL_FURIOUS_BALAYAGE);
                            events.ScheduleEvent(EVENT_FURIOUS_BALAYAGE, 2000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private :
            uint8 phaseId;
            uint32 timerEvent;
            EventMap events;
            InstanceScript *instance;
            bool start;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_raz_the_crazedAI(creature);
        }
};

class spell_balayage_dummy : public SpellScriptLoader
{
public:
    spell_balayage_dummy() : SpellScriptLoader("spell_balayage_dummy") { }

    class spell_balayage_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_balayage_dummy_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            GetCaster()->CastSpell(GetHitUnit(), SPELL_FURIOUS_BALAYAGE);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_balayage_dummy_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_balayage_dummy_SpellScript();
    }
};

class spell_raz_agro_nearby_entry : public SpellScriptLoader
{
public:
    spell_raz_agro_nearby_entry() : SpellScriptLoader("spell_raz_agro_nearby_entry") { }

    class spell_raz_agro_nearby_entry_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_raz_agro_nearby_entry_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        bool isGoodTarget(uint32 entry)
        {
            switch (entry)
            {
               case 39985:
               case 39980:
               case 39978:
               case 39982:
               case 39987:
                   return true;
            }
            return false;
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end();)
            {
                if ((*itr)->GetTypeId() != TYPEID_PLAYER && isGoodTarget((*itr)->ToCreature()->GetEntry()))
                    ++itr;
                else
                    targets.erase(itr++);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_raz_agro_nearby_entry_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_raz_agro_nearby_entry_SpellScript();
    }
};

class spell_brc_fire_strike : public SpellScriptLoader
{
public:
    spell_brc_fire_strike() : SpellScriptLoader("spell_brc_fire_strike") { }

    class spell_brc_fire_strike_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_brc_fire_strike_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (!targets.empty())
                Trinity::Containers::RandomResizeList(targets, 1);
        }

        void HandleDummy(SpellEffIndex effIndex)
        {
            GetCaster()->CastSpell(GetHitUnit(), GetSpellInfo()->Effects[effIndex].BasePoints, true);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_brc_fire_strike_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(spell_brc_fire_strike_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_brc_fire_strike_SpellScript();
    }
};

void AddSC_boss_romogg_bonecrusher()
{
    new boss_romogg_bonecrusher();
    new spell_chains_of_woe_triggered();
    new spell_romogg_quake();
    new npc_raz_the_crazed();
    new spell_balayage_dummy();
    new spell_raz_agro_nearby_entry();
    new spell_brc_fire_strike();
}
