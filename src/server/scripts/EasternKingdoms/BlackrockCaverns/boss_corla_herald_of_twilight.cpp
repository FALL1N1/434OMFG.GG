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

/*
 * TODO:
 * Script: 90% Complete
 * - Implement Netherbeam
 */

#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "blackrock_caverns.h"

enum Texts
{
    TALK_AGGRO  = 0,
    TALK_ZEALOT = 1,
    TALK_SLAY   = 2,
    TALK_DEATH  = 3
};

enum Spells
{
    SPELL_EVOLUTION_AURA            = 75697,
    SPELL_EVOLUTION_AURA_HC         = 87378, // seems spell difficulty is not working for aura checks
    SPELL_TWILIGHT_EVOLUTION        = 75732,
    SPELL_AURA_OF_ACCELERATION      = 75817,
    SPELL_DARK_COMMAND              = 75823,
    SPELL_KNEELING_IN_SUPPLICATION  = 75608,
    SPELL_DRAIN_ESSENCE_CHANNELING  = 75645,
    SPELL_NETHERESSENCE_AURA        = 75649,
    SPELL_NETHERESSENCE_VISUAL      = 75650,
    SPELL_FORCE_BLAST               = 76522,
    SPELL_GRAVITY_STRIKE            = 76561,
    SPELL_GRIEVOUS_WHIRL            = 93658,
    SPELL_SHADOW_STRIKE             = 66134,
    SPELL_NETHER_BREAM              = 75706,
    SPELL_EVOLUTION                 = 75610
};

enum Events
{
    EVENT_DARK_COMMAND      = 1,
    EVENT_FORCE_BLAST       = 2,
    EVENT_GRAVITY_STRIKE    = 3,
    EVENT_GRIEVOUS_WHIRL    = 4,
    EVENT_SHADOW_STRIKE     = 5,
    EVENT_EVOLUTION_CAST    = 6,
    EVENT_NETHER_ESSENCE    = 7
};

enum Actions
{
    ACTION_TRIGGER_START_CHANNELING = 1,
    ACTION_COUNT_EVOLVE             = 2,
    ACTION_DESPAWN_ESSENCE          = 3
};

#define ACHIEVEMENT_ARRESTED_DEVELOPMENT 5282

Position const summonPositions[4] =
{
    {580.919f, 982.981f, 155.354f, 2.05572f}, // Twilight Zealot spawn positions
    {565.629f, 983.0f, 155.354f, 0.689123f},
    {573.676f, 980.619f, 155.354f, 1.58448f},
    {573.482f, 944.277f, 173.305f, 1.5513f},   // Nether Essence trigger spawning position
};

struct evolutionHelper
{
    uint8 data;
    uint8 data64;
    Position const position;
};

static const evolutionHelper positionData[3] =
{
    {DATA_SPAWN_LEFT, DATA_ZEALOT_LEFT, summonPositions[0]},
    {DATA_SPAWN_RIGHT, DATA_ZEALOT_RIGHT, summonPositions[1]},
    {DATA_SPAWN_CENTER, DATA_ZEALOT_CENTER, summonPositions[2]},
};

class boss_corla_herald_of_twilight : public CreatureScript
{
public:
    boss_corla_herald_of_twilight() : CreatureScript("boss_corla_herald_of_twilight") { }

    struct boss_corla_herald_of_twilightAI : public BossAI
    {
        boss_corla_herald_of_twilightAI(Creature* creature) : BossAI(creature, BOSS_CORLA) {}

        void Reset()
        {
            instance->SetBossState(BOSS_CORLA, NOT_STARTED);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TWILIGHT_EVOLUTION);
            events.Reset();
            spawnZealots();
            DoCast(me, SPELL_DRAIN_ESSENCE_CHANNELING, true);
            evolvedCounter = 0;
            left = IN_PROGRESS;
            right = IN_PROGRESS;
            center = IN_PROGRESS;
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            Talk(TALK_AGGRO);
            me->CastStop();
            events.ScheduleEvent(EVENT_DARK_COMMAND, 10000);
            DoCastAOE(SPELL_AURA_OF_ACCELERATION);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

            for (SummonList::iterator itr = summons.begin(); itr != summons.end(); ++itr)
                if (Creature* zealot = ObjectAccessor::GetCreature(*me, *itr))
                    if (zealot->isAlive() && (zealot->GetEntry() == NPC_TWILIGHT_ZEALOT_CORLA))
                        zealot->SetInCombatWithZone();
        }

        void JustDied(Unit* /*killer*/)
        {
            if (IsHeroic() && evolvedCounter >= 3)
                instance->DoCompleteAchievement(ACHIEVEMENT_ARRESTED_DEVELOPMENT);

            Talk(TALK_DEATH);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            _JustDied();
        }

        void KilledUnit(Unit* who)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
                Talk(TALK_SLAY);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);

            switch (summon->GetEntry())
            {
                case NPC_TWILIGHT_ZEALOT_CORLA:
                    summon->SummonCreature(NPC_NETHER_ESSENCE_TRIGGER, summonPositions[3], TEMPSUMMON_MANUAL_DESPAWN);
                    summon->AddAura(SPELL_KNEELING_IN_SUPPLICATION, summon);
                    break;
                default:
                    break;
            }
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_COUNT_EVOLVE)
                evolvedCounter++;
        }

        void SpellHit(Unit* /*caster*/, const SpellInfo* spell)
        {
            if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL))
                for (uint8 i = 0; i < 3; ++i)
                    if (spell->Effects[i].Effect == SPELL_EFFECT_INTERRUPT_CAST || spell->Effects[i].Mechanic == MECHANIC_INTERRUPT)
                        if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL)->m_spellInfo->Id == SPELL_DARK_COMMAND)
                            me->InterruptSpell(CURRENT_GENERIC_SPELL, false);
        }

        void SetData(uint32 data, uint32 state)
        {
            switch (data)
            {
                case DATA_SPAWN_LEFT:
                    left = data;
                    break;
                case DATA_SPAWN_RIGHT:
                    right = data;
                    break;
                case DATA_SPAWN_CENTER:
                    center = data;
                    break;
                default:
                    break;
            }
        }

        uint32 GetData(uint32 data) const
        {
            switch(data)
            {
                case DATA_SPAWN_LEFT:
                    return left;
                case DATA_SPAWN_RIGHT:
                    return right;
                case DATA_SPAWN_CENTER:
                    return center;
            }
            return 0;
        }

        uint64 GetGUID(int32 type) const
        {
            switch(type)
            {
                case DATA_ZEALOT_LEFT:
                    return ZealotGUID[0];
                case DATA_ZEALOT_RIGHT:
                    return ZealotGUID[1];
                case DATA_ZEALOT_CENTER:
                    return ZealotGUID[2];
            }
            return 0;
        }

        void spawnZealots()
        {
            for (SummonList::iterator itr = summons.begin(); itr != summons.end(); ++itr)
                if (Creature* zealot = ObjectAccessor::GetCreature(*me, *itr))
                    if (zealot->isAlive() && (zealot->GetEntry() == NPC_TWILIGHT_ZEALOT_CORLA))
                        zealot->AI()->DoAction(ACTION_DESPAWN_ESSENCE);

            summons.DespawnAll();

            for(uint8 i = 0; i <= (IsHeroic() ? 2 : 1); i++)
            {
                ZealotGUID[i] = 0;

                if (Creature* zealot = me->SummonCreature(NPC_TWILIGHT_ZEALOT_CORLA, positionData[i].position, TEMPSUMMON_MANUAL_DESPAWN))
                    ZealotGUID[i] = zealot->GetGUID();
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_DARK_COMMAND:
                        if(Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 200, true))
                            DoCast(target, SPELL_DARK_COMMAND);
                        events.ScheduleEvent(EVENT_DARK_COMMAND, 10000);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        uint8 evolvedCounter;
        uint32 left;
        uint32 right;
        uint32 center;
        uint64 ZealotGUID[3];
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_corla_herald_of_twilightAI (creature);
    }
};

class mob_twilight_zealot : public CreatureScript
{
public:
    mob_twilight_zealot() : CreatureScript("mob_twilight_zealot") { }

    struct mob_twilight_zealotAI : public ScriptedAI
    {
        mob_twilight_zealotAI(Creature* creature) : ScriptedAI(creature), summons(creature), Intialized(false)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
            events.Reset();
            Intialized = false;
            me->SetReactState(REACT_PASSIVE);
        }

        void EnterCombat(Unit* /*who*/)
        {
            for (SummonList::iterator itr = summons.begin(); itr != summons.end(); ++itr)
                if (Creature* essence = ObjectAccessor::GetCreature(*me, *itr))
                    if (essence->isAlive() && (essence->GetEntry() == NPC_NETHER_ESSENCE_TRIGGER))
                        essence->AI()->DoAction(ACTION_TRIGGER_START_CHANNELING);

            if (Creature* corla = Creature::GetCreature(*me, instance->GetData64(DATA_CORLA)))
                corla->SetInCombatWithZone();
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_DESPAWN_ESSENCE)
                summons.DespawnAll();
        }

        void UpdateAI(const uint32 Diff)
        {
            if (!me->isInCombat() || me->HasAura(SPELL_KNEELING_IN_SUPPLICATION))
                return;

            if(!Intialized)
            {
                events.ScheduleEvent(EVENT_FORCE_BLAST, 10000);
                events.ScheduleEvent(EVENT_GRAVITY_STRIKE, 22000);
                events.ScheduleEvent(EVENT_GRIEVOUS_WHIRL, 7000);
                events.ScheduleEvent(EVENT_SHADOW_STRIKE, 14000);

                Intialized = true;

                me->SetReactState(REACT_AGGRESSIVE);

                me->GetMotionMaster()->MoveChase(GetPlayerAtMinimumRange(1.0f));
            }

            events.Update(Diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_FORCE_BLAST:
                        if(Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 200, true))
                            DoCast(target,SPELL_FORCE_BLAST);
                        events.ScheduleEvent(EVENT_FORCE_BLAST, 10000);
                        break;
                    case EVENT_GRAVITY_STRIKE:
                        DoCastVictim(SPELL_GRAVITY_STRIKE);
                        events.ScheduleEvent(EVENT_GRAVITY_STRIKE, 22000);
                        break;
                    case EVENT_GRIEVOUS_WHIRL:
                        DoCastAOE(SPELL_GRIEVOUS_WHIRL);
                        events.ScheduleEvent(EVENT_GRIEVOUS_WHIRL, 7000);
                        break;
                    case EVENT_SHADOW_STRIKE:
                        DoCastVictim(SPELL_SHADOW_STRIKE);
                        events.ScheduleEvent(EVENT_SHADOW_STRIKE, 14000);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        InstanceScript* instance;
        EventMap events;
        SummonList summons;
        bool Intialized;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_twilight_zealotAI (creature);
    }
};

class mob_corla_netheressence_trigger : public CreatureScript
{
public:
    mob_corla_netheressence_trigger() : CreatureScript("mob_corla_netheressence_trigger") { }

    struct mob_corla_netheressence_triggerAI : public ScriptedAI
    {
        mob_corla_netheressence_triggerAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NOT_SELECTABLE);
            instance = me->GetInstanceScript();
        }

        void DoAction(const int32 action)
        {
            switch(action)
            {
                case ACTION_TRIGGER_START_CHANNELING:
                    me->AddAura(SPELL_NETHERESSENCE_AURA, me);
                    events.ScheduleEvent(EVENT_EVOLUTION_CAST, 100);
                    events.ScheduleEvent(EVENT_NETHER_ESSENCE, 500);
                    break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            if (instance && instance->IsDone(BOSS_CORLA))
                me->DespawnOrUnsummon();

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_EVOLUTION_CAST:
                        DoCast(me, SPELL_EVOLUTION, true);
                        events.ScheduleEvent(EVENT_EVOLUTION_CAST, 200);
                        break;
                    case EVENT_NETHER_ESSENCE:
                        DoCast(me, SPELL_NETHERESSENCE_VISUAL, true);
                        events.ScheduleEvent(EVENT_NETHER_ESSENCE, 3500);
                        break;
                }
            }
        }

    private:
        EventMap events;
        InstanceScript* instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_corla_netheressence_triggerAI (creature);
    }
};

class EvolutionTargetFilter
{
public:
    bool operator()(WorldObject* target) const
    {
        InstanceScript* instance = target->GetInstanceScript();

        if (instance->IsDone(BOSS_CORLA))
            return true;

        if (Unit* unit = target->ToUnit())
        {
            if (unit->GetTypeId() == TYPEID_UNIT && unit->GetEntry() == NPC_TWILIGHT_ZEALOT_CORLA)
                if (Player* player = unit->FindNearestPlayer(1.5f))
                    if (!player->HasAura(SPELL_TWILIGHT_EVOLUTION))
                        return true;

            if (Creature* corla = Creature::GetCreature(*unit, instance->GetData64(DATA_CORLA)))
            {
                if (unit->GetDistance(positionData[0].position) < 2.0f && corla->AI()->GetData(positionData[0].data) == IN_PROGRESS
                    || unit->GetDistance(positionData[1].position) < 2.0f && corla->AI()->GetData(positionData[1].data) == IN_PROGRESS
                    || unit->GetMap()->IsHeroic() && unit->GetDistance(positionData[2].position) < 2.0f && corla->AI()->GetData(positionData[2].data) == IN_PROGRESS)
                    return false;
            }
        }
        return true;
    }
};

class spell_corla_evolution : public SpellScriptLoader
{
public:
    spell_corla_evolution() : SpellScriptLoader("spell_corla_evolution") { }

    class spell_corla_evolution_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_corla_evolution_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(EvolutionTargetFilter());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_corla_evolution_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_corla_evolution_SpellScript();
    }
};

class spell_corla_evolution_aura : public SpellScriptLoader
{
public:
    spell_corla_evolution_aura() : SpellScriptLoader("spell_corla_evolution_aura") { }

    class spell_corla_evolution_aura_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_corla_evolution_aura_SpellScript);

        void HandleEvolution(SpellEffIndex effIndex)
        {
            InstanceScript* instance = GetHitUnit()->GetInstanceScript();
            Unit* target = GetHitUnit();

            if (!instance || !target)
                return;

            if (Creature* corla = Creature::GetCreature(*GetCaster(), instance->GetData64(DATA_CORLA)))
            {
                if(target->GetAuraCount(SPELL_EVOLUTION_AURA) == 100 || target->GetAuraCount(SPELL_EVOLUTION_AURA_HC) == 100)
                {
                    switch (target->GetTypeId())
                    {
                        case TYPEID_PLAYER:
                            corla->CastSpell(target, SPELL_TWILIGHT_EVOLUTION, true); // a hostile creature needs to cast it...
                            target->RemoveAurasDueToSpell(SPELL_EVOLUTION_AURA);
                            target->RemoveAurasDueToSpell(SPELL_EVOLUTION_AURA_HC);
                            break;
                        case TYPEID_UNIT:
                            target->ToCreature()->AI()->DoAction(ACTION_DESPAWN_ESSENCE);
                            target->RemoveAurasDueToSpell(SPELL_KNEELING_IN_SUPPLICATION);
                            target->RemoveAurasDueToSpell(SPELL_EVOLUTION_AURA);
                            target->RemoveAurasDueToSpell(SPELL_EVOLUTION_AURA_HC);
                            target->SetDisplayId(31705);
                            target->SetMaxHealth(target->GetMaxHealth() / 100 * 300);
                            target->SetHealth(target->GetMaxHealth());
                            if (Unit* attackTarget = target->ToCreature()->SelectNearestPlayer(200.0f))
                                target->ToCreature()->AI()->AttackStart(attackTarget);
                            if (corla->AI()->GetGUID(DATA_ZEALOT_LEFT) == target->GetGUID())
                                corla->AI()->SetData(DATA_SPAWN_LEFT, DONE);
                            if (corla->AI()->GetGUID(DATA_ZEALOT_RIGHT) == target->GetGUID())
                                corla->AI()->SetData(DATA_SPAWN_RIGHT, DONE);
                            if (target->GetMap()->IsHeroic() && corla->AI()->GetGUID(DATA_ZEALOT_CENTER) == target->GetGUID())
                                corla->AI()->SetData(DATA_SPAWN_CENTER, DONE);
                            break;
                        default:
                            break;
                    }

                    if (target->GetMap()->IsHeroic())
                        corla->AI()->DoAction(ACTION_COUNT_EVOLVE);

                    corla->AI()->Talk(TALK_ZEALOT);
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_corla_evolution_aura_SpellScript::HandleEvolution, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_corla_evolution_aura_SpellScript();
    }
};

void AddSC_boss_corla_herald_of_twilight()
{
    new boss_corla_herald_of_twilight();
    new mob_twilight_zealot();
    new mob_corla_netheressence_trigger();
    new spell_corla_evolution();
    new spell_corla_evolution_aura();
}
