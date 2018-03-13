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

/*
 * Spells used in holidays/game events that do not fit any other category.
 * Ordered alphabetically using scriptname.
 * Scriptnames in this file should be prefixed with "spell_#holidayname_".
 */

#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "GridNotifiers.h"
#include "CellImpl.h"

// 45102 Romantic Picnic
enum SpellsPicnic
{
    SPELL_BASKET_CHECK              = 45119,    // Holiday - Valentine - Romantic Picnic Near Basket Check
    SPELL_ROMANTIC_PICNIC           = 45102,    // Holiday - Valentine - Romantic Picnic - Basket spell
    SPELL_MEAL_PERIODIC             = 45103,    // Holiday - Valentine - Romantic Picnic Meal Periodic - effect dummy
    SPELL_MEAL_EAT_VISUAL           = 45120,    // Holiday - Valentine - Romantic Picnic Meal Eat Visual
    //SPELL_MEAL_PARTICLE             = 45114,    // Holiday - Valentine - Romantic Picnic Meal Particle - unused
    SPELL_DRINK_VISUAL              = 45121,    // Holiday - Valentine - Romantic Picnic Drink Visual
    SPELL_ROMANTIC_PICNIC_ACHIEV    = 45123,    // Romantic Picnic periodic = 5000
};

class spell_love_is_in_the_air_romantic_picnic : public SpellScriptLoader
{
    public:
        spell_love_is_in_the_air_romantic_picnic() : SpellScriptLoader("spell_love_is_in_the_air_romantic_picnic") { }

        class spell_love_is_in_the_air_romantic_picnic_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_love_is_in_the_air_romantic_picnic_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                target->SetStandState(UNIT_STAND_STATE_SIT);
                target->CastSpell(target, SPELL_MEAL_PERIODIC, false);
            }

            void OnPeriodic(AuraEffect const* /*aurEff*/)
            {
                // Every 5 seconds
                Unit* target = GetTarget();
                Unit* caster = GetCaster();

                // If our player is no longer sit, remove all auras
                if (target->getStandState() != UNIT_STAND_STATE_SIT)
                {
                    target->RemoveAura(SPELL_ROMANTIC_PICNIC_ACHIEV);
                    target->RemoveAura(GetAura());
                    return;
                }

                target->CastSpell(target, SPELL_BASKET_CHECK, false); // unknown use, it targets Romantic Basket
                target->CastSpell(target, RAND(SPELL_MEAL_EAT_VISUAL, SPELL_DRINK_VISUAL), false);

                bool foundSomeone = false;
                // For nearby players, check if they have the same aura. If so, cast Romantic Picnic (45123)
                // required by achievement and "hearts" visual
                std::list<Player*> playerList;
                Trinity::AnyPlayerInObjectRangeCheck checker(target, INTERACTION_DISTANCE*2);
                Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(target, playerList, checker);
                target->VisitNearbyWorldObject(INTERACTION_DISTANCE*2, searcher);
                for (std::list<Player*>::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                {
                    if ((*itr) != target && (*itr)->HasAura(GetId())) // && (*itr)->getStandState() == UNIT_STAND_STATE_SIT)
                    {
                        if (caster)
                        {
                            caster->CastSpell(*itr, SPELL_ROMANTIC_PICNIC_ACHIEV, true);
                            caster->CastSpell(target, SPELL_ROMANTIC_PICNIC_ACHIEV, true);
                        }
                        foundSomeone = true;
                        // break;
                    }
                }

                if (!foundSomeone && target->HasAura(SPELL_ROMANTIC_PICNIC_ACHIEV))
                    target->RemoveAura(SPELL_ROMANTIC_PICNIC_ACHIEV);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_love_is_in_the_air_romantic_picnic_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_love_is_in_the_air_romantic_picnic_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_love_is_in_the_air_romantic_picnic_AuraScript();
        }
};

class go_romantic_basket : public GameObjectScript
{
public:
    go_romantic_basket() : GameObjectScript("go_romantic_basket") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (player && go)
            go->CastSpell(player, SPELL_ROMANTIC_PICNIC);

        return true;
    }
};

enum snagglebolts_khorium_bomb {
    THROW_BOMB = 71024,
    WAGON = 201716,
    WAGON_CREDIT = 38035,
};

class spell_snagglebolts_khorium_bomb : public SpellScriptLoader
{
public:
    spell_snagglebolts_khorium_bomb() : SpellScriptLoader("spell_snagglebolts_khorium_bomb") { }

    class spell_snagglebolts_khorium_bomb_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_snagglebolts_khorium_bomb_SpellScript);

        bool Validate(SpellInfo const* /*spell*/) override
        {
            if (!sSpellMgr->GetSpellInfo(THROW_BOMB))
                return false;
            return true;
        }

        bool Load() override
        {
            return GetCaster()->GetTypeId() == TYPEID_PLAYER;
        }

        void HandleOnCast()
        {
            GetCaster()->ToPlayer()->KilledMonsterCredit(WAGON_CREDIT);
        }

        SpellCastResult CheckCast()
        {
            Player* player = GetCaster()->ToPlayer();

            uint8 const count = 18;
            uint32 quests[count] = { 24658, 24659, 24660, 24662, 24663, 24664, 24665, 24666, 28934, 24638, 24645, 24647, 24648, 24649, 24650, 24651, 24652, 28935 };

            for (uint8 i = 0; i < count; ++i)
            {
                if (player->GetQuestStatus(quests[i]) == QUEST_STATUS_INCOMPLETE)
                {
                    if (player->FindNearestGameObject(WAGON, 15.0f))
                        return SPELL_CAST_OK;
                    else
                        return SPELL_FAILED_OUT_OF_RANGE;
                }
            }

            return SPELL_FAILED_NOT_HERE;
        }

        void Register() override
        {
            OnCast += SpellCastFn(spell_snagglebolts_khorium_bomb_SpellScript::HandleOnCast);
            OnCheckCast += SpellCheckCastFn(spell_snagglebolts_khorium_bomb_SpellScript::CheckCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_snagglebolts_khorium_bomb_SpellScript();
    }
};


// 24751 Trick or Treat
enum TrickOrTreatSpells
{
    SPELL_TRICK                 = 24714,
    SPELL_TREAT                 = 24715,
    SPELL_TRICKED_OR_TREATED    = 24755,
    SPELL_TRICKY_TREAT_SPEED    = 42919,
    SPELL_TRICKY_TREAT_TRIGGER  = 42965,
    SPELL_UPSET_TUMMY           = 42966
};

class spell_hallow_end_trick_or_treat : public SpellScriptLoader
{
    public:
        spell_hallow_end_trick_or_treat() : SpellScriptLoader("spell_hallow_end_trick_or_treat") {}

        class spell_hallow_end_trick_or_treat_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hallow_end_trick_or_treat_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_TRICK) || !sSpellMgr->GetSpellInfo(SPELL_TREAT) || !sSpellMgr->GetSpellInfo(SPELL_TRICKED_OR_TREATED))
                    return false;
                return true;
            }

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Player* target = GetHitPlayer())
                {
                    caster->CastSpell(target, roll_chance_i(50) ? SPELL_TRICK : SPELL_TREAT, true);
                    caster->CastSpell(target, SPELL_TRICKED_OR_TREATED, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_hallow_end_trick_or_treat_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hallow_end_trick_or_treat_SpellScript();
        }
};

class spell_hallow_end_tricky_treat : public SpellScriptLoader
{
    public:
        spell_hallow_end_tricky_treat() : SpellScriptLoader("spell_hallow_end_tricky_treat") { }

        class spell_hallow_end_tricky_treat_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hallow_end_tricky_treat_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_TRICKY_TREAT_SPEED))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_TRICKY_TREAT_TRIGGER))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_UPSET_TUMMY))
                    return false;
                return true;
            }

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (caster->HasAura(SPELL_TRICKY_TREAT_TRIGGER) && caster->GetAuraCount(SPELL_TRICKY_TREAT_SPEED) > 3 && roll_chance_i(33))
                    caster->CastSpell(caster, SPELL_UPSET_TUMMY, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_hallow_end_tricky_treat_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hallow_end_tricky_treat_SpellScript();
        }
};

enum Mistletoe
{
    SPELL_CREATE_MISTLETOE          = 26206,
    SPELL_CREATE_HOLLY              = 26207,
    SPELL_CREATE_SNOWFLAKES         = 45036
};

class spell_winter_veil_mistletoe : public SpellScriptLoader
{
    public:
        spell_winter_veil_mistletoe() : SpellScriptLoader("spell_winter_veil_mistletoe") { }

        class spell_winter_veil_mistletoe_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_winter_veil_mistletoe_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_CREATE_MISTLETOE) ||
                    !sSpellMgr->GetSpellInfo(SPELL_CREATE_HOLLY) ||
                    !sSpellMgr->GetSpellInfo(SPELL_CREATE_SNOWFLAKES))
                    return false;
                return true;
            }

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                if (Player* target = GetHitPlayer())
                {
                    uint32 spellId = RAND(SPELL_CREATE_HOLLY, SPELL_CREATE_MISTLETOE, SPELL_CREATE_SNOWFLAKES);
                    GetCaster()->CastSpell(target, spellId, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_winter_veil_mistletoe_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_winter_veil_mistletoe_SpellScript();
        }
};

// 26275 - PX-238 Winter Wondervolt TRAP
enum PX238WinterWondervolt
{
    SPELL_PX_238_WINTER_WONDERVOLT_TRANSFORM_1  = 26157,
    SPELL_PX_238_WINTER_WONDERVOLT_TRANSFORM_2  = 26272,
    SPELL_PX_238_WINTER_WONDERVOLT_TRANSFORM_3  = 26273,
    SPELL_PX_238_WINTER_WONDERVOLT_TRANSFORM_4  = 26274
};

class spell_winter_veil_px_238_winter_wondervolt : public SpellScriptLoader
{
    public:
        spell_winter_veil_px_238_winter_wondervolt() : SpellScriptLoader("spell_winter_veil_px_238_winter_wondervolt") { }

        class spell_winter_veil_px_238_winter_wondervolt_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_winter_veil_px_238_winter_wondervolt_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PX_238_WINTER_WONDERVOLT_TRANSFORM_1) ||
                    !sSpellMgr->GetSpellInfo(SPELL_PX_238_WINTER_WONDERVOLT_TRANSFORM_2) ||
                    !sSpellMgr->GetSpellInfo(SPELL_PX_238_WINTER_WONDERVOLT_TRANSFORM_3) ||
                    !sSpellMgr->GetSpellInfo(SPELL_PX_238_WINTER_WONDERVOLT_TRANSFORM_4))
                    return false;
                return true;
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);

                uint32 const spells[4] =
                {
                    SPELL_PX_238_WINTER_WONDERVOLT_TRANSFORM_1,
                    SPELL_PX_238_WINTER_WONDERVOLT_TRANSFORM_2,
                    SPELL_PX_238_WINTER_WONDERVOLT_TRANSFORM_3,
                    SPELL_PX_238_WINTER_WONDERVOLT_TRANSFORM_4
                };

                if (Unit* target = GetHitUnit())
                {
                    for (uint8 i = 0; i < 4; ++i)
                        if (target->HasAura(spells[i]))
                            return;

                    target->CastSpell(target, spells[urand(0, 3)], true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_winter_veil_px_238_winter_wondervolt_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }

        private:

        };

        SpellScript* GetSpellScript() const
        {
            return new spell_winter_veil_px_238_winter_wondervolt_SpellScript();
        }
};

enum spells
{
    SPELL_CHARGE_FIND_TARGET = 49299, // charge target finder... wenn gefunden -> charge dest
    SPELL_CHARGE_DEST = 49302, // triggern durch main spell
    SPELL_CHARGE_CHECK = 49324, // casten bei ankunft von dest durch charge
    SPELL_CHARGE_PARRENT_MISSLE = 49337, // wenn einer getroffen durch ID - 49325 Racer Slam, resolve
    SPELL_AURA_KILL_COUNT = 49444,
    SPELL_RACER_DEATH_ROOT = 49439,
    SPELL_SUMMON_DEATH_RACER = 49368,
};

class spell_rocket_slam_slamming : public SpellScriptLoader
{
public:
    spell_rocket_slam_slamming() : SpellScriptLoader("spell_rocket_slam_slamming") { }

    class spell_rocket_slam_slamming_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_rocket_slam_slamming_SpellScript);

        void HandleAfterCast()
        {
            if (GetCaster()->GetTypeId() != TYPEID_UNIT)
                return;

            if (Creature* caster = GetCaster()->ToCreature())
            {
                Position pos;
                GetCaster()->GetNearPosition(pos, 20.0f, 0.0f);
                if (Creature* target = caster->SummonCreature(27674, pos, TEMPSUMMON_TIMED_DESPAWN, 5000))
                {
                    GetCaster()->CastSpell(target, SPELL_CHARGE_DEST, true);
                    GetCaster()->CastSpell(target, SPELL_CHARGE_CHECK, true);
                }
            }
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_rocket_slam_slamming_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_rocket_slam_slamming_SpellScript();
    }
};

class spell_racer_slam_resolve : public SpellScriptLoader
{
public:
    spell_racer_slam_resolve() : SpellScriptLoader("spell_racer_slam_resolve") { }

    class spell_racer_slam_resolve_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_racer_slam_resolve_SpellScript);

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            if (Unit* target = GetHitUnit())
            {
                if (target->GetTypeId() != TYPEID_UNIT)
                    return;

                if (target->GetEntry() == 40281 || target->GetEntry() == 27664)
                {
                    Position pos;
                    target->GetRandomNearPosition(pos, 10.0f);
                    target->CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), SPELL_CHARGE_PARRENT_MISSLE, true);
                    target->CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), SPELL_SUMMON_DEATH_RACER, true);
                    target->CastSpell(target, SPELL_RACER_DEATH_ROOT, true);
                    target->ToCreature()->DespawnOrUnsummon(2000);

                    if (Unit* targetOwner = target->GetOwner())
                        targetOwner->RemoveAurasDueToSpell(SPELL_AURA_KILL_COUNT);

                    if (Unit* owner = GetCaster()->GetOwner())
                    {
                        GetCaster()->AddAura(SPELL_AURA_KILL_COUNT, owner);
                        owner->ToPlayer()->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_AURA_KILL_COUNT);
                    }
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_racer_slam_resolve_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_racer_slam_resolve_SpellScript();
    }
};

// mid summer

enum TorchCatchingData
{
    SPELL_FLING_TORCH_MISSILE     = 45669,
    SPELL_TOSS_TORCH_SHADOW       = 46105,
    SPELL_TORCH_TARGET_PICKER     = 45907,
    SPELL_TORCHES_COUGHT          = 45693,
    SPELL_JUGGLE_TORCH_MISSED     = 45676,
    SPELL_TORCH_CATCHING_SUCCESS  = 46081,
    SPELL_TORCH_DAILY_SUCCESS     = 46654,
    NPC_JUGGLE_TARGET             = 25515,
    QUEST_TORCH_CATCHING_A        = 11657,
    QUEST_TORCH_CATCHING_H        = 11923,
    QUEST_MORE_TORCH_CATCHING_A   = 11924,
    QUEST_MORE_TORCH_CATCHING_H   = 11925
};

class spell_torch_target_picker : public SpellScriptLoader
{
public:
    spell_torch_target_picker() : SpellScriptLoader("spell_torch_target_picker") {}

    class spell_torch_target_picker_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_torch_target_picker_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_FLING_TORCH_MISSILE))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_TOSS_TORCH_SHADOW))
                return false;
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            Unit* caster = GetCaster();

            if (!caster)
                return;

            std::list<Creature*> juggleList;
            caster->GetCreatureListWithEntryInGrid(juggleList, NPC_JUGGLE_TARGET, 10.0f);

            if (!juggleList.empty())
                for (std::list<Creature*>::iterator iter = juggleList.begin(); iter != juggleList.end(); ++iter)
                    unitList.remove(*iter);

            if (unitList.empty())
                return;

            std::list<WorldObject*>::iterator itr = unitList.begin();
            std::advance(itr, urand(0, unitList.size() - 1));

            WorldObject* target = *itr;
            caster->MonsterSay(target->GetName().c_str(), 0, 0);
            unitList.clear();
            unitList.push_back(target);
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            Unit* target = GetHitUnit();

            if (!caster || !target)
                return;

            caster->CastSpell(target, SPELL_FLING_TORCH_MISSILE, true);
            caster->CastSpell(target, SPELL_TOSS_TORCH_SHADOW, true);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_torch_target_picker_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnEffectHitTarget += SpellEffectFn(spell_torch_target_picker_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_torch_target_picker_SpellScript();
    }
};

class spell_juggle_torch_catch : public SpellScriptLoader
{
public:
    spell_juggle_torch_catch() : SpellScriptLoader("spell_juggle_torch_catch") {}

    class spell_juggle_torch_catch_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_juggle_torch_catch_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_TORCH_TARGET_PICKER))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_TORCHES_COUGHT))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_JUGGLE_TORCH_MISSED))
                return false;
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            Unit* caster = GetCaster();
            Unit* juggleTarget = NULL;
            bool missed = true;

            if (unitList.empty() || !caster || !caster->ToPlayer())
                return;

            for (std::list<WorldObject*>::iterator iter = unitList.begin(); iter != unitList.end(); ++iter)
                if (Unit *tar = (*iter)->ToUnit())
                {
                    if (tar == caster)
                        missed = false;

                    if ((*iter)->ToCreature())
                        juggleTarget = tar;
                }

            if (missed)
            {
                if (juggleTarget)
                    juggleTarget->CastSpell(juggleTarget, SPELL_JUGGLE_TORCH_MISSED, true);
                caster->RemoveAurasDueToSpell(SPELL_TORCHES_COUGHT);
            }
            else
            {
                uint8 neededCatches;

                if (caster->ToPlayer()->GetQuestStatus(QUEST_TORCH_CATCHING_A) == QUEST_STATUS_INCOMPLETE
                    || caster->ToPlayer()->GetQuestStatus(QUEST_TORCH_CATCHING_H) == QUEST_STATUS_INCOMPLETE)
                {
                    neededCatches = 4;
                }
                else if (caster->ToPlayer()->GetQuestStatus(QUEST_MORE_TORCH_CATCHING_A) == QUEST_STATUS_INCOMPLETE
                         || caster->ToPlayer()->GetQuestStatus(QUEST_MORE_TORCH_CATCHING_H) == QUEST_STATUS_INCOMPLETE)
                {
                    neededCatches = 10;
                }
                else
                {
                    caster->RemoveAurasDueToSpell(SPELL_TORCHES_COUGHT);
                    return;
                }

                caster->CastSpell(caster, SPELL_TORCH_TARGET_PICKER, true);
                caster->CastSpell(caster, SPELL_TORCHES_COUGHT, true);

                // reward quest
                if (caster->GetAuraCount(SPELL_TORCHES_COUGHT) >= neededCatches)
                {
                    caster->CastSpell(caster, SPELL_TORCH_CATCHING_SUCCESS, true);
                    caster->CastSpell(caster, SPELL_TORCH_DAILY_SUCCESS, true);
                    caster->RemoveAurasDueToSpell(SPELL_TORCHES_COUGHT);
                }
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_juggle_torch_catch_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_juggle_torch_catch_SpellScript();
    }
};

enum RibbonPoleData
{
    SPELL_HAS_FULL_MIDSUMMER_SET    = 58933,
    SPELL_BURNING_HOT_POLE_DANCE    = 58934,
    SPELL_RIBBON_DANCE              = 29175,
    GO_RIBBON_POLE                  = 181605,
};

class spell_gen_ribbon_pole_dancer_check : public SpellScriptLoader
{
public:
    spell_gen_ribbon_pole_dancer_check() : SpellScriptLoader("spell_gen_ribbon_pole_dancer_check") { }

    class spell_gen_ribbon_pole_dancer_check_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_gen_ribbon_pole_dancer_check_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_HAS_FULL_MIDSUMMER_SET)
                || !sSpellMgr->GetSpellInfo(SPELL_RIBBON_DANCE)
                || !sSpellMgr->GetSpellInfo(SPELL_BURNING_HOT_POLE_DANCE))
                return false;
            return true;
        }

        void PeriodicTick(AuraEffect const* /*aurEff*/)
        {
            Unit* target = GetTarget();

            // check if aura needs to be removed
            if (!target->FindNearestGameObject(GO_RIBBON_POLE, 20.0f) || !target->HasUnitState(UNIT_STATE_CASTING))
            {
                target->InterruptNonMeleeSpells(false);
                target->RemoveAurasDueToSpell(GetId());
                return;
            }

            // set xp buff duration
            if (Aura* aur = target->GetAura(SPELL_RIBBON_DANCE))
            {
                aur->SetMaxDuration(std::min(3600000, aur->GetMaxDuration() + 180000));
                aur->RefreshDuration();

                // reward achievement criteria
                if (aur->GetMaxDuration() == 3600000 && target->HasAura(SPELL_HAS_FULL_MIDSUMMER_SET))
                    target->CastSpell(target, SPELL_BURNING_HOT_POLE_DANCE, true);
            }
            else
                target->AddAura(SPELL_RIBBON_DANCE, target);
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_gen_ribbon_pole_dancer_check_AuraScript::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_gen_ribbon_pole_dancer_check_AuraScript();
    }
};

void AddSC_holiday_spell_scripts()
{
    // Love is in the Air
    new spell_love_is_in_the_air_romantic_picnic(); // Verified
    new go_romantic_basket(); // Recently Verified
    new spell_snagglebolts_khorium_bomb(); // Recently Verified
    // Hallow's End
    new spell_hallow_end_trick_or_treat(); // Verified
    new spell_hallow_end_tricky_treat(); // Verified
    // Winter Veil
    new spell_winter_veil_mistletoe(); // Verified
    new spell_winter_veil_px_238_winter_wondervolt(); // Verified
    new spell_rocket_slam_slamming(); // Recently Verified, not 100% sure about whether it's correct spell id.
    new spell_racer_slam_resolve(); // Recently Verified
    // Mid summer festival
    new spell_torch_target_picker(); // Recently Verified
    new spell_juggle_torch_catch(); // Recently Verified
    new spell_gen_ribbon_pole_dancer_check(); // Recently Verified
}
