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
Author: Kazlim - Nagash
Progress: 90%
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

enum Spells
{
    SPELL_CONCENTRATION = 98256,
    SPELL_CONCENTRATION_APPLY = 98229,
    SPELL_UNCOMMON_CONCENTRATION = 98254,
    SPELL_RARE_CONCENTRATION = 98253,
    SPELL_EPIC_CONCENTRATION = 98252,
    SPELL_LEGENDARY_CONCENTRATION = 98245,

    SPELL_CAT_FORM = 98374,
    SPELL_SCORPION_FORM = 98379,

    SPELL_ADRENALINE = 97238,
    SPELL_BERSERK = 26662,
    SPELL_BURNING_ORBS_CAST = 98451,
    SPELL_BURNING_ORBS_SUMMON = 98565,
    SPELL_BURNING_ORBS_AURA = 98583,
    SPELL_FIERY_CYCLONE = 98443,
    SPELL_FLAME_SCYTHE = 98474, // 10N
    SPELL_FURY = 97235,
    SPELL_LEAPING_FLAMES = 101165,
    SPELL_LEAPING_FLAMES_DMG = 98535,
    SPELL_LEAPING_FLAMES_SUMMON = 101222,
    SPELL_SEARING_SEEDS = 98450,
    SPELL_SEARING_SEEDS_DMG = 98620,

    // Druid fo the Flame
    SPELL_DRUID_CAT_FORM = 99574,
    SPELL_KNEEL_TO_THE_FLAME = 99705,
    SPELL_REACTIVE_FLAMES = 99649,
    SPELL_RECKLESS_LEAP = 99629,
    SPELL_RECKLESS_LEAP_STUN = 99646,
    SPELL_SUNFIRE = 99626
};

enum Events
{
    EVENT_SCORPION_FORM = 1,
    EVENT_CAT_FORM,
    EVENT_SCORPION_NIGHT_ELF_FORM,
    EVENT_CAT_NIGHT_ELF_FORM,
    EVENT_BURNING_ORBS,
    EVENT_FLAME_SCYTHE,
    EVENT_LEAPING_FLAMES,
    EVENT_BERSERK,
    EVENT_CHECK_CLUSTERED,
    EVENT_REGENERATE_POWER,
    EVENT_KNEEL_TO_THE_FLAME,
    EVENT_RECKLESS_LEAP,
    EVENT_RECKLESS_LEAP_STUN,
    EVENT_SUNFIRE
};

enum MajordomoPhase
{
    PHASE_DRUID,
    PHASE_CAT,
    PHASE_SCORPION
};

enum Achievements
{
    ACHIEVEMENT_MAJOR_DOMO = 5804
};


class DeadCheck
{
public:
    bool operator() (WorldObject* unit) const
    {
        if (unit->ToUnit()->isDead())
            return true;
        return false;
    }
};

class boss_majordomostaghelm : public CreatureScript
{
public:
    boss_majordomostaghelm() : CreatureScript("boss_majordomostaghelm") {}

    struct boss_majordomostaghelmAI : public BossAI
    {
        boss_majordomostaghelmAI(Creature * creature) : BossAI(creature, DATA_MAJORDOMO_STAGHELM), summons(me)
        {
            instance = creature->GetInstanceScript();
        }

        uint8 countPhase;
        uint8 rateEnergy;
        InstanceScript* instance;
        SummonList summons;
        EventMap events;
        uint32 killtimer;
        bool introDone;
        bool clustered;

        void InitializeAI()
        {
            std::list<Creature*> unitList;
            me->GetCreatureListWithEntryInGrid(unitList, NPC_DRUID_OF_THE_FLAME, 100.0f);

            unitList.remove_if(DeadCheck());

            if (!unitList.empty())
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->SetVisible(false);
                introDone = false;
            }
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }

        void Reset()
        {
            _Reset();
            events.Reset();
            summons.DespawnAll();
            me->LowerPlayerDamageReq(me->GetMaxHealth());
            killtimer = 0;
            countPhase = 1;

            if (instance)
            {
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SEARING_SEEDS);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CONCENTRATION);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CONCENTRATION_APPLY);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_UNCOMMON_CONCENTRATION);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_RARE_CONCENTRATION);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_EPIC_CONCENTRATION);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_LEGENDARY_CONCENTRATION);


            }
            instance->SetData(DATA_MAJORDOMO_STAGHELM_EVENT, NOT_STARTED);
        }

        void DamageTaken(Unit* who, uint32 &damage)
        {
            if (!me || !me->isAlive())
                return;
        }

        void EnterCombat(Unit* victim)
        {
            _EnterCombat();
            DoZoneInCombat(me);
            countPhase = 1;

            events.ScheduleEvent(EVENT_CHECK_CLUSTERED, 4000);
            events.ScheduleEvent(EVENT_BERSERK, 600 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_REGENERATE_POWER, 2000);
            instance->SetData(DATA_MAJORDOMO_STAGHELM_EVENT, IN_PROGRESS);

            if (me->GetMap()->IsHeroic())
                me->CastSpell(me, SPELL_CONCENTRATION, false);

        }

        void JustDied(Unit* killer)
        {
            _JustDied();
            me->LowerPlayerDamageReq(me->GetMaxHealth());
            summons.DespawnAll();
            if (me->GetMap()->IsHeroic())
            {// this will give the achievement to players in heroic difficulty
                Map::PlayerList const& PlayerList = instance->instance->GetPlayers();
                AchievementEntry const* major_domo = sAchievementStore.LookupEntry(ACHIEVEMENT_MAJOR_DOMO);
                for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                {
                    if (Player* player = itr->getSource())
                    {
                        player->CompletedAchievement(major_domo);
                    }
                }
            }

            if (GameObject* fire = me->FindNearestGameObject(208906, 100.0f))
                instance->HandleGameObject(fire->GetGUID(), true, fire);

            if (instance)
            {
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SEARING_SEEDS);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CONCENTRATION);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CONCENTRATION_APPLY);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_UNCOMMON_CONCENTRATION);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_RARE_CONCENTRATION);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_EPIC_CONCENTRATION);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_LEGENDARY_CONCENTRATION);


            }

            instance->SetData(DATA_MAJORDOMO_STAGHELM_EVENT, DONE);
        }

        void KilledUnit(Unit* victim)
        {
            if (!victim || victim->GetTypeId() != TYPEID_PLAYER || killtimer > 0)
                return;
            //me->MonsterYell(SAY_KILL, 0, 0);
            //DoPlaySoundToSet(me, SOU_KILL);
            killtimer = 8000;
        }

        bool Clustered()
        {
            bool clustered = false;
            std::list<Player*> players;
            Trinity::AnyPlayerInObjectRangeCheck u_check(me, 500.0f);
            Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, u_check);
            (me->getVictim())->VisitNearbyWorldObject(10.0f, searcher);

            uint32 treshhold = players.size() - uint32(players.size() * 0.33);
            for (std::list<Player*>::const_iterator i = players.begin(); i != players.end(); ++i)
            {
                if ((*i)->ToPlayer())
                {
                    std::list<Player*> targets;
                    Trinity::AnyPlayerInObjectRangeCheck u_check((*i), 10.0f);
                    Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher((*i), targets, u_check);
                    (*i)->VisitNearbyWorldObject(10.0f, searcher);

                    uint32 size = targets.size();
                    if (size >= treshhold)
                    {
                        clustered = true;
                        break;
                    }
                }
            }
            return clustered;
        }

        void RegenerateEnergy()
        {
            rateEnergy = 6;
            if (Aura* aura = me->GetAura(SPELL_ADRENALINE))
                AddPct(rateEnergy, 20 * aura->GetStackAmount());

            me->SetPower(POWER_ENERGY, me->GetPower(POWER_ENERGY) + rateEnergy);
        }

        void UpdateAI(uint32 diff)
        {
            if (me->HasAura(SPELL_SCORPION_FORM) && me->GetPower(POWER_ENERGY) == 100)
            {
                me->CastSpell(me, SPELL_FLAME_SCYTHE, false);
                me->SetPower(POWER_ENERGY, 0);
            }

            if (me->HasAura(SPELL_CAT_FORM) && me->GetPower(POWER_ENERGY) == 100)
            {
                me->CastSpell(me, SPELL_LEAPING_FLAMES, false);
                me->SetPower(POWER_ENERGY, 0);
                me->SummonCreature(NPC_SPIRIT_OF_THE_FLAME, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), TEMPSUMMON_CORPSE_DESPAWN);
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_REGENERATE_POWER:
                    RegenerateEnergy();
                    events.ScheduleEvent(EVENT_REGENERATE_POWER, 1000);
                    break;
                case EVENT_CHECK_CLUSTERED:
                    if (Clustered() == true)
                    {
                        if (!me->HasAura(SPELL_SCORPION_FORM))
                            events.ScheduleEvent(EVENT_SCORPION_FORM, 1);
                    }
                    else
                    {
                        if (!me->HasAura(SPELL_CAT_FORM))
                            events.ScheduleEvent(EVENT_CAT_FORM, 1);
                    }
                    events.ScheduleEvent(EVENT_CHECK_CLUSTERED, 3000);
                    break;
                case EVENT_BERSERK:
                    me->CastSpell(me, SPELL_BERSERK, false);
                    break;
                case EVENT_SCORPION_FORM:
                    me->RemoveAura(SPELL_ADRENALINE);
                    me->RemoveAura(SPELL_CAT_FORM);

                    if (countPhase == 3)
                        events.ScheduleEvent(EVENT_SCORPION_NIGHT_ELF_FORM, 1);
                    else
                    {
                        me->setPowerType(POWER_ENERGY);
                        me->SetMaxPower(POWER_ENERGY, 100);
                        me->SetPower(POWER_ENERGY, 0);
                        me->SetCanRegen(false);

                        me->CastSpell(me, SPELL_SCORPION_FORM, false);
                        me->CastSpell(me, SPELL_FURY, false);
                    }
                    countPhase++;
                    events.RescheduleEvent(EVENT_CHECK_CLUSTERED, 2000);
                    break;
                case EVENT_CAT_FORM:
                    me->RemoveAura(SPELL_ADRENALINE);
                    me->RemoveAura(SPELL_SCORPION_FORM);

                    if (countPhase == 3)
                        events.ScheduleEvent(EVENT_CAT_NIGHT_ELF_FORM, 1);
                    else
                    {
                        me->setPowerType(POWER_ENERGY);
                        me->SetMaxPower(POWER_ENERGY, 100);
                        me->SetPower(POWER_ENERGY, 0);
                        me->SetCanRegen(false);

                        me->CastSpell(me, SPELL_CAT_FORM, false);
                        me->CastSpell(me, SPELL_FURY, false);
                    }
                    countPhase++;
                    events.RescheduleEvent(EVENT_CHECK_CLUSTERED, 2000);
                    break;
                case EVENT_SCORPION_NIGHT_ELF_FORM:
                    countPhase = 0;
                    me->RemoveAura(SPELL_ADRENALINE);
                    me->CastSpell(me, SPELL_FIERY_CYCLONE, false);
                    me->CastSpell(me, SPELL_SEARING_SEEDS, false);
                    events.CancelEvent(EVENT_CHECK_CLUSTERED);
                    events.ScheduleEvent(EVENT_SCORPION_FORM, 2000);
                    break;
                case EVENT_CAT_NIGHT_ELF_FORM:
                    countPhase = 0;
                    me->RemoveAura(SPELL_ADRENALINE);
                    me->CastSpell(me, SPELL_FIERY_CYCLONE, false);
                    me->CastSpell(me, SPELL_BURNING_ORBS_CAST, false);
                    for (int i = 0; i < RAID_MODE(2, 5, 2, 5); ++i)
                    {
                        me->CastSpell(me, SPELL_BURNING_ORBS_SUMMON, false);
                    }
                    events.CancelEvent(EVENT_CHECK_CLUSTERED);
                    events.ScheduleEvent(EVENT_CAT_FORM, 2000);
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_majordomostaghelmAI(creature);
    }
};

class DistanceCheck
{
public:
    explicit DistanceCheck(Unit* _caster) : caster(_caster) { }

    bool operator() (WorldObject* unit) const
    {
        if (caster->GetExactDist2d(unit) <= 10.0f)
            return true;
        return false;
    }

    Unit* caster;
};

class spell_check_distance : public SpellScriptLoader
{
public:
    spell_check_distance() : SpellScriptLoader("spell_check_distance") { }

    class spell_check_distance_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_check_distance_SpellScript);

        bool Load()
        {
            return GetCaster()->GetTypeId() == TYPEID_UNIT;
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(DistanceCheck(GetCaster()));

            Trinity::Containers::RandomResizeList(targets, 1);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_check_distance_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_check_distance_SpellScript();
    }
};

class npc_druid_of_the_flame : public CreatureScript
{
public:
    npc_druid_of_the_flame() : CreatureScript("npc_druid_of_the_flame"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_druid_of_the_flameAI(creature);
    }

    struct npc_druid_of_the_flameAI : public ScriptedAI
    {
        npc_druid_of_the_flameAI(Creature *creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }
        InstanceScript* instance;
        EventMap events;
        bool targeted;

        void InitializeAI()
        {
            if (me->GetPositionX() < 515.0f)
                me->CastSpell(me, SPELL_DRUID_CAT_FORM, false);
        }

        void Reset()
        {
            events.Reset();
        }

        void JustReachedHome()
        {
            if (me->GetPositionX() < 515.0f)
                me->CastSpell(me, SPELL_DRUID_CAT_FORM, false);
        }

        void JustDied(Unit* killer)
        {
            std::list<Creature*> unitList;
            me->GetCreatureListWithEntryInGrid(unitList, NPC_DRUID_OF_THE_FLAME, 100.0f);

            unitList.remove_if(DeadCheck());

            if (unitList.empty())
            {
                if (Unit *boss = me->FindNearestCreature(BOSS_MAJORDOMO_STAGHELM, 100.0f, true))
                {
                    boss->ToCreature()->SetReactState(REACT_AGGRESSIVE);
                    boss->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    boss->SetVisible(true);
                    instance->SetData(DATA_MAJORDOMO_STAGHELM_EVENT, NOT_STARTED);
                }
            }
        }

        void EnterCombat(Unit* /*who*/)
        {
            DoZoneInCombat();
            targeted = false;
            if (me->HasAura(SPELL_DRUID_CAT_FORM))
            {

                me->CastSpell(me, SPELL_DRUID_CAT_FORM, false); // Aura in creature_addon doesn't transform
                events.ScheduleEvent(EVENT_RECKLESS_LEAP, 3000);
            }
            else
                events.ScheduleEvent(EVENT_SUNFIRE, 1000);
        }

        void UpdateAI(uint32 diff)
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SUNFIRE:
                    me->CastSpell(me, SPELL_SUNFIRE, false);
                    events.ScheduleEvent(EVENT_SUNFIRE, urand(3000, 5000));
                    break;
                case EVENT_RECKLESS_LEAP:
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM_AT_THIS_FLOOR, 0, 90.0f, true, 0))
                        me->CastSpell(target, SPELL_RECKLESS_LEAP, false);
                    events.ScheduleEvent(EVENT_RECKLESS_LEAP, 3000);
                    targeted = true;
                    break;
                default:
                    break;
                }
            }
        }

    };
};

class NotVictimFilter
{
public:
    NotVictimFilter(Unit* caster) : _victim(caster->getVictim())
    {
    }

    bool operator()(WorldObject* target)
    {
        return target == _victim;
    }

private:
    Unit* _victim;
};

class spell_searing_seeds : public SpellScriptLoader
{
public:
    spell_searing_seeds() : SpellScriptLoader("spell_searing_seeds") { }

    class spell_searing_seeds_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_searing_seeds_SpellScript);

        bool Load()
        {
            return GetCaster()->GetTypeId() == TYPEID_UNIT;
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(NotVictimFilter(GetCaster()));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_searing_seeds_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    class spell_searing_seeds_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_searing_seeds_AuraScript);

        void HandleApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            aurEff->GetBase()->SetDuration(urand(3000, 60000));
        }

        void HandleRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* owner = GetUnitOwner())
                owner->CastSpell(owner, SPELL_SEARING_SEEDS_DMG, false);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_searing_seeds_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_searing_seeds_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }

    };

    SpellScript* GetSpellScript() const
    {
        return new spell_searing_seeds_SpellScript();
    }

    AuraScript* GetAuraScript() const
    {
        return new spell_searing_seeds_AuraScript();
    }
};

class spell_concentration : public SpellScriptLoader
{
public:
    spell_concentration() : SpellScriptLoader("spell_concentration") { }

    class spell_concentration_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_concentration_AuraScript);


        void HandleProc(ProcEventInfo& procInfo)
        {
            if (Unit* target = GetTarget()->ToPlayer())
            {
                target->ToPlayer()->SetPower(POWER_ALTERNATE_POWER, -1);
                target->RemoveAura(98254);
                target->RemoveAura(98253);
                target->RemoveAura(98252);
                target->RemoveAura(98245);
            }
        }

        void HandleProc(AuraEffect const* aurEff)
        {

            if (Unit* target = GetTarget())
            {
                if (Player* player = target->ToPlayer())
                {
                    if (Unit* caster = GetCaster())
                    {
                        target->ToPlayer()->SetPower(POWER_ALTERNATE_POWER, target->GetPower(POWER_ALTERNATE_POWER) + 5);

                        if (target->GetPower(POWER_ALTERNATE_POWER) > 25 && target->GetPower(POWER_ALTERNATE_POWER) < 50)
                        {
                            if (!target->HasAura(SPELL_UNCOMMON_CONCENTRATION))
                                caster->CastSpell(target, SPELL_UNCOMMON_CONCENTRATION, false);
                        }
                        else if (target->GetPower(POWER_ALTERNATE_POWER) > 50 && target->GetPower(POWER_ALTERNATE_POWER) < 75)
                        {
                            target->RemoveAura(SPELL_UNCOMMON_CONCENTRATION);
                            if (!target->HasAura(SPELL_RARE_CONCENTRATION))
                                caster->CastSpell(target, SPELL_RARE_CONCENTRATION, false);

                        }
                        else if (target->GetPower(POWER_ALTERNATE_POWER) > 75 && target->GetPower(POWER_ALTERNATE_POWER) < 100)
                        {
                            target->RemoveAura(SPELL_RARE_CONCENTRATION);
                            if (!target->HasAura(SPELL_EPIC_CONCENTRATION))
                                caster->CastSpell(target, SPELL_EPIC_CONCENTRATION, false);
                        }
                        else if (target->GetPower(POWER_ALTERNATE_POWER) == 100)
                        {
                            target->RemoveAura(SPELL_EPIC_CONCENTRATION);
                            if (!target->HasAura(SPELL_LEGENDARY_CONCENTRATION))
                                caster->CastSpell(target, SPELL_LEGENDARY_CONCENTRATION, false);
                        }
                    }
                }
            }
        }

        void Register()
        {
            OnProc += AuraProcFn(spell_concentration_AuraScript::HandleProc);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_concentration_AuraScript::HandleProc, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_concentration_AuraScript();
    }
};

void AddSC_boss_majordomostaghelm()
{
    new boss_majordomostaghelm();
    new spell_check_distance();
    new spell_searing_seeds();
    new npc_druid_of_the_flame();
    new spell_concentration();
}