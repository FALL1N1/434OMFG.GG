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
 * Scripts for spells with SPELLFAMILY_ROGUE and SPELLFAMILY_GENERIC spells used by rogue players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_rog_".
 */

#include "Player.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

enum RogueSpells
{
    SPELL_ROGUE_BLADE_FLURRY_EXTRA_ATTACK        = 22482,
    SPELL_ROGUE_CHEAT_DEATH_COOLDOWN             = 31231,
    SPELL_ROGUE_GLYPH_OF_PREPARATION             = 56819,
    SPELL_ROGUE_PREY_ON_THE_WEAK                 = 58670,
    SPELL_ROGUE_SHIV_TRIGGERED                   = 5940,
    SPELL_ROGUE_TRICKS_OF_THE_TRADE_DMG_BOOST    = 57933,
    SPELL_ROGUE_TRICKS_OF_THE_TRADE_PROC         = 59628,
    SPELL_ROGUE_REVEALING_STRIKE                 = 84617,
    SPELL_ROGUE_T12_2P_TRIGGERED                 = 99173,
    SPELL_ROGUE_SHALLOW_INSIGHT                  = 84745,
    SPELL_ROGUE_MODERATE_INSIGHT                 = 84746,
    SPELL_ROGUE_DEEP_INSIGHT                     = 84747,
    SPELL_ROGUE_BANDITS_GUILE_BONUS              = 84748,
    SPELL_ROGUE_TRICKS_OF_TIME                   = 105864,
    SPELL_ROGUE_LEGENDARY_FURY_OF_THE_DESTROYER  = 109949,
    SPELL_ROGUE_LEGENDARY_DAGGER_PROC_AURA       = 109939,
    SPELL_ROGUE_LEGENDARY_SHADOW_OF_THE_DESTORYER = 109941,
};

enum RogueSpellIcons
{
    ICON_ROGUE_IMPROVED_RECUPERATE               = 4819
};

// 13877, 33735, (check 51211, 65956) - Blade Flurry
class spell_rog_blade_flurry : public SpellScriptLoader
{
    public:
        spell_rog_blade_flurry() : SpellScriptLoader("spell_rog_blade_flurry") { }

        class spell_rog_blade_flurry_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_blade_flurry_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_ROGUE_BLADE_FLURRY_EXTRA_ATTACK))
                    return false;
                return true;
            }

            bool Load()
            {
                _procTarget = NULL;
                return true;
            }

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                _procTarget = GetTarget()->SelectNearbyTarget(eventInfo.GetProcTarget());
                return _procTarget && eventInfo.GetDamageInfo();
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                GetTarget()->CastCustomSpell(SPELL_ROGUE_BLADE_FLURRY_EXTRA_ATTACK, SPELLVALUE_BASE_POINT0, eventInfo.GetDamageInfo()->GetDamage(), _procTarget, true, NULL, aurEff);
            }

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_rog_blade_flurry_AuraScript::CheckProc);
                if (m_scriptSpellId == 13877)
                    OnEffectProc += AuraEffectProcFn(spell_rog_blade_flurry_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_MOD_POWER_REGEN_PERCENT);
                else
                    OnEffectProc += AuraEffectProcFn(spell_rog_blade_flurry_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_MOD_MELEE_HASTE);
            }

        private:
            Unit* _procTarget;
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_blade_flurry_AuraScript();
        }
};

// 31228 - Cheat Death
class spell_rog_cheat_death : public SpellScriptLoader
{
    public:
        spell_rog_cheat_death() : SpellScriptLoader("spell_rog_cheat_death") { }

        class spell_rog_cheat_death_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_cheat_death_AuraScript);

            uint32 absorbChance;

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_ROGUE_CHEAT_DEATH_COOLDOWN))
                    return false;
                return true;
            }

            bool Load()
            {
                absorbChance = GetSpellInfo()->Effects[EFFECT_0].CalcValue();
                return GetUnitOwner()->ToPlayer();
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                Player* target = GetTarget()->ToPlayer();
                if (dmgInfo.GetDamage() < target->GetHealth() || target->HasSpellCooldown(SPELL_ROGUE_CHEAT_DEATH_COOLDOWN) ||  !roll_chance_i(absorbChance))
                    return;

                target->CastSpell(target, SPELL_ROGUE_CHEAT_DEATH_COOLDOWN, true);
                target->AddSpellCooldown(SPELL_ROGUE_CHEAT_DEATH_COOLDOWN, 0, time(NULL) + 60);

                uint32 health10 = target->CountPctFromMaxHealth(10);

                // hp > 10% - absorb hp till 10%
                if (target->GetHealth() > health10)
                    absorbAmount = dmgInfo.GetDamage() - target->GetHealth() + health10;
                // hp lower than 10% - absorb everything
                else
                    absorbAmount = dmgInfo.GetDamage();
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_rog_cheat_death_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_rog_cheat_death_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_cheat_death_AuraScript();
        }
};

// 2818 - Deadly Poison
class spell_rog_deadly_poison : public SpellScriptLoader
{
    public:
        spell_rog_deadly_poison() : SpellScriptLoader("spell_rog_deadly_poison") { }

        class spell_rog_deadly_poison_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_deadly_poison_SpellScript);

            bool Load()
            {
                _stackAmount = 0;
                // at this point CastItem must already be initialized
                return GetCaster()->GetTypeId() == TYPEID_PLAYER && GetCastItem();
            }

            void HandleBeforeHit()
            {
                if (Unit* target = GetHitUnit())
                    // Deadly Poison
                    if (AuraEffect const* aurEff = target->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_ROGUE, 0x10000, 0x80000, 0, GetCaster()->GetGUID()))
                        _stackAmount = aurEff->GetBase()->GetStackAmount();
            }

            void HandleAfterHit()
            {
                if (_stackAmount < 5)
                    return;

                Player* player = GetCaster()->ToPlayer();

                if (Unit* target = GetHitUnit())
                {

                    Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);

                    if (item == GetCastItem())
                        item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

                    if (!item)
                        return;

                    // item combat enchantments
                    for (uint8 slot = 0; slot < MAX_ENCHANTMENT_SLOT; ++slot)
                    {
                        if (slot != TEMP_ENCHANTMENT_SLOT)    // not holding enchantment id
                            continue;

                        SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(item->GetEnchantmentId(EnchantmentSlot(slot)));
                        if (!enchant)
                            continue;

                        for (uint8 s = 0; s < 3; ++s)
                        {
                            if (enchant->type[s] != ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL)
                                continue;

                            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(enchant->spellid[s]);
                            if (!spellInfo)
                            {
                                TC_LOG_ERROR("spells", "Player::CastItemCombatSpell Enchant %i, player (Name: %s, GUID: %u) cast unknown spell %i", enchant->ID, player->GetName().c_str(), player->GetGUIDLow(), enchant->spellid[s]);
                                continue;
                            }

                            // Proc only rogue poisons
                            if (spellInfo->SpellFamilyName != SPELLFAMILY_ROGUE || spellInfo->Dispel != DISPEL_POISON)
                                continue;

                            // Do not reproc deadly
                            if (spellInfo->SpellFamilyFlags.IsEqual(0x10000, 0x80000, 0))
                                continue;

                            if (spellInfo->IsPositive())
                                player->CastSpell(player, enchant->spellid[s], true, item);
                            else
                                player->CastSpell(target, enchant->spellid[s], true, item);
                        }
                    }
                }
            }

            void Register()
            {
                BeforeHit += SpellHitFn(spell_rog_deadly_poison_SpellScript::HandleBeforeHit);
                AfterHit += SpellHitFn(spell_rog_deadly_poison_SpellScript::HandleAfterHit);
            }

            uint32 _stackAmount;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_deadly_poison_SpellScript();
        }
};

// 31130 - Nerves of Steel
class spell_rog_nerves_of_steel : public SpellScriptLoader
{
    public:
        spell_rog_nerves_of_steel() : SpellScriptLoader("spell_rog_nerves_of_steel") { }

        class spell_rog_nerves_of_steel_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_nerves_of_steel_AuraScript);

            uint32 absorbPct;

            bool Load()
            {
                absorbPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue(GetCaster());
                return true;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                // reduces all damage taken while stun or fear
                if (GetTarget()->GetUInt32Value(UNIT_FIELD_FLAGS) & (UNIT_FLAG_FLEEING) || (GetTarget()->GetUInt32Value(UNIT_FIELD_FLAGS) & (UNIT_FLAG_STUNNED) && GetTarget()->HasAuraWithMechanic(1<<MECHANIC_STUN)))
                    absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
            }

            void Register()
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_rog_nerves_of_steel_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_rog_nerves_of_steel_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_nerves_of_steel_AuraScript();
        }
};

// 14185 - Preparation
class spell_rog_preparation : public SpellScriptLoader
{
    public:
        spell_rog_preparation() : SpellScriptLoader("spell_rog_preparation") { }

        class spell_rog_preparation_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_preparation_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_ROGUE_GLYPH_OF_PREPARATION))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Player* caster = GetCaster()->ToPlayer();

                // immediately finishes the cooldown on certain Rogue abilities
                const SpellCooldowns& cm = caster->GetSpellCooldownMap();
                for (SpellCooldowns::const_iterator itr = cm.begin(); itr != cm.end();)
                {
                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);

                    if (spellInfo && spellInfo->SpellFamilyName == SPELLFAMILY_ROGUE)
                    {
                        if (spellInfo->Id == 36554 || spellInfo->Id == 1856 || spellInfo->Id == 2983)     // Vanish, Sprint, Shadowstep
                            caster->RemoveSpellCooldown((itr++)->first, true);
                        else if (caster->HasAura(SPELL_ROGUE_GLYPH_OF_PREPARATION))
                        {
                            if (spellInfo->SpellFamilyFlags[1] & SPELLFAMILYFLAG1_ROGUE_DISMANTLE ||        // Dismantle
                                spellInfo->SpellFamilyFlags[0] & SPELLFAMILYFLAG_ROGUE_KICK ||              // Kick
                                spellInfo->Id == 76577)                                                     // Smoke bomb
                                caster->RemoveSpellCooldown((itr++)->first, true);
                            else
                                ++itr;
                        }
                        else
                            ++itr;
                    }
                    else
                        ++itr;
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_rog_preparation_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_preparation_SpellScript();
        }
};

// 51685 - Prey on the Weak
class spell_rog_prey_on_the_weak : public SpellScriptLoader
{
    public:
        spell_rog_prey_on_the_weak() : SpellScriptLoader("spell_rog_prey_on_the_weak") { }

        class spell_rog_prey_on_the_weak_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_prey_on_the_weak_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_ROGUE_PREY_ON_THE_WEAK))
                    return false;
                return true;
            }

            void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
            {
                Unit* target = GetTarget();
                Unit* victim = target->getVictim();
                if (victim && (target->GetHealthPct() > victim->GetHealthPct()))
                {
                    if (!target->HasAura(SPELL_ROGUE_PREY_ON_THE_WEAK))
                    {
                        int32 bp = GetSpellInfo()->Effects[EFFECT_0].CalcValue();
                        target->CastCustomSpell(target, SPELL_ROGUE_PREY_ON_THE_WEAK, &bp, 0, 0, true);
                    }
                }
                else
                    target->RemoveAurasDueToSpell(SPELL_ROGUE_PREY_ON_THE_WEAK);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_rog_prey_on_the_weak_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_prey_on_the_weak_AuraScript();
        }
};

// 73651 - Recuperate
class spell_rog_recuperate : public SpellScriptLoader
{
    public:
        spell_rog_recuperate() : SpellScriptLoader("spell_rog_recuperate") { }

        class spell_rog_recuperate_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_recuperate_AuraScript);

            void HandleUpdatePeriodic(AuraEffect* aurEff)
            {
                aurEff->RecalculateAmount();
            }

            void CalculateBonus(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
            {
                canBeRecalculated = false;
                if (Unit* caster = GetCaster())
                {
                    amount *= 1000;
                    // Improved Recuperate
                    if (AuraEffect const* auraEffect = caster->GetDummyAuraEffect(SPELLFAMILY_ROGUE, ICON_ROGUE_IMPROVED_RECUPERATE, EFFECT_0))
                        amount += auraEffect->GetAmount();

                    amount = CalculatePct(caster->GetMaxHealth(), float(amount) / 1000.0f);
                }
            }

            void Register() override
            {
                OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_rog_recuperate_AuraScript::HandleUpdatePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_rog_recuperate_AuraScript::CalculateBonus, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_rog_recuperate_AuraScript();
        }
};

// Stealth
class spell_rog_stealth : public SpellScriptLoader
{
    public:
        spell_rog_stealth() : SpellScriptLoader("spell_rog_stealth") { }

        class spell_rog_stealth_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_stealth_AuraScript);

            void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    // Overkill
                    if (caster->HasAura(58426))
                        caster->CastSpell(caster, 58427, true);
            }

            void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* target = GetTarget())
                    // Overkill
                    if (Aura* overkill = target->GetAura(58427, target->GetGUID()))
                    {
                        overkill->SetMaxDuration(20 * IN_MILLISECONDS);
                        overkill->SetDuration(20 * IN_MILLISECONDS);
                    }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_rog_stealth_AuraScript::HandleApply, EFFECT_1, SPELL_AURA_MOD_STEALTH, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                OnEffectRemove += AuraEffectApplyFn(spell_rog_stealth_AuraScript::HandleRemove, EFFECT_1, SPELL_AURA_MOD_STEALTH, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_stealth_AuraScript();
        }
};

// -1943 - Rupture
class spell_rog_rupture : public SpellScriptLoader
{
    public:
        spell_rog_rupture() : SpellScriptLoader("spell_rog_rupture") { }

        class spell_rog_rupture_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_rupture_AuraScript);

            bool Load()
            {
                Unit* caster = GetCaster();
                return caster && caster->GetTypeId() == TYPEID_PLAYER;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
            {
                if (Unit* caster = GetCaster())
                {
                    canBeRecalculated = false;

                    float const attackpowerPerCombo[6] =
                    {
                        0.0f,
                        0.015f,         // 1 point:  ${($m1 + $b1*1 + 0.015 * $AP) * 4} damage over 8 secs
                        0.024f,         // 2 points: ${($m1 + $b1*2 + 0.024 * $AP) * 5} damage over 10 secs
                        0.03f,          // 3 points: ${($m1 + $b1*3 + 0.03 * $AP) * 6} damage over 12 secs
                        0.03428571f,    // 4 points: ${($m1 + $b1*4 + 0.03428571 * $AP) * 7} damage over 14 secs
                        0.0375f         // 5 points: ${($m1 + $b1*5 + 0.0375 * $AP) * 8} damage over 16 secs
                    };

                    uint8 cp = caster->ToPlayer()->GetComboPoints();
                    if (cp > 5)
                        cp = 5;

                    amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * attackpowerPerCombo[cp]);

                    if (AuraEffect const* strike = GetUnitOwner()->GetAuraEffect(SPELL_ROGUE_REVEALING_STRIKE, EFFECT_2, caster->GetGUID()))
                    {
                        AddPct(amount, strike->GetAmount());

                        strike->GetBase()->Remove();
                    }
                }
            }

            void HandleRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
                {
                    if (aurEff->GetBase()->GetDuration() >= 1000)
                    {
                        int32 bp0 = 5 * (aurEff->GetBase()->GetDuration() / 1000);
                        if (Unit* caster = GetCaster())
                            if (caster->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_ROGUE, 4888, EFFECT_0))
                                caster->CastCustomSpell(caster, 51637, &bp0, 0, 0, true);
                    }
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_rog_rupture_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
                OnEffectRemove += AuraEffectApplyFn(spell_rog_rupture_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_rupture_AuraScript();
        }
};

// 5938 - Shiv
class spell_rog_shiv : public SpellScriptLoader
{
    public:
        spell_rog_shiv() : SpellScriptLoader("spell_rog_shiv") { }

        class spell_rog_shiv_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_shiv_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_ROGUE_SHIV_TRIGGERED))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                    caster->CastSpell(unitTarget, SPELL_ROGUE_SHIV_TRIGGERED, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_rog_shiv_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_shiv_SpellScript();
        }
};

// 53 - Backstab
class spell_rog_backstab : public SpellScriptLoader
{
    public:
        spell_rog_backstab() : SpellScriptLoader("spell_rog_backstab") { }

        class spell_rog_backstab_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_backstab_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spellInfo*/){ return true; }

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                    if (unitTarget->HasAuraState(AURA_STATE_HEALTHLESS_35_PERCENT, GetSpellInfo(), caster))
                        if (AuraEffect* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_ROGUE, 134, 0))
                        {
                            int32 bp0 = aurEff->GetAmount();
                            caster->CastCustomSpell(caster, 79132, &bp0, 0, 0, true);
                        }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_rog_backstab_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_backstab_SpellScript();
        }
};

// 57934 - Tricks of the Trade
class spell_rog_tricks_of_the_trade : public SpellScriptLoader
{
    public:
        spell_rog_tricks_of_the_trade() : SpellScriptLoader("spell_rog_tricks_of_the_trade") { }

        class spell_rog_tricks_of_the_trade_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_tricks_of_the_trade_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_ROGUE_TRICKS_OF_THE_TRADE_DMG_BOOST))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_ROGUE_TRICKS_OF_THE_TRADE_PROC))
                    return false;
                return true;
            }

            bool Load()
            {
                _redirectTarget = NULL;
                return true;
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEFAULT)
                    GetTarget()->ResetRedirectThreat();

                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                    if (Unit* caster = GetCaster())
                        caster->ToPlayer()->RemoveSpellCooldown(GetId(), true);
            }

            bool CheckProc(ProcEventInfo& /*eventInfo*/)
            {
                _redirectTarget = GetTarget()->GetRedirectThreatTarget();
                return _redirectTarget;
            }

            void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
            {
                PreventDefaultAction();

                Unit* target = GetTarget();
                target->CastSpell(_redirectTarget, SPELL_ROGUE_TRICKS_OF_THE_TRADE_DMG_BOOST, true);
                target->CastSpell(target, SPELL_ROGUE_TRICKS_OF_THE_TRADE_PROC, true);
                Remove(AURA_REMOVE_BY_DEFAULT); // maybe handle by proc charges

                if (Unit* caster = GetCaster())
                {
                    // Rogue T12 4P Bonus
                    if (caster->HasAura(99175, caster->GetGUID()))
                        caster->CastSpell(caster, RAND(99186, 99187, 99188), true);

                    // Rogue T13 2P Bonus (Tricks of the Trade)
                    if (caster->HasAura(105849, caster->GetGUID()))
                        caster->CastSpell(caster, SPELL_ROGUE_TRICKS_OF_TIME, true);
                }
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_rog_tricks_of_the_trade_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                DoCheckProc += AuraCheckProcFn(spell_rog_tricks_of_the_trade_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_rog_tricks_of_the_trade_AuraScript::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
            }

        private:
            Unit* _redirectTarget;
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_tricks_of_the_trade_AuraScript();
        }
};

// 99186, 99187, 99188
class spell_rog_t12_set_proc : public SpellScriptLoader
{
public:
    spell_rog_t12_set_proc() : SpellScriptLoader("spell_rog_t12_set_proc") { }

    class spell_rog_t12_set_proc_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_rog_t12_set_proc_AuraScript);

        void DoCalcAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
        {
            canBeRecalculated = false;
            uint32 maxrating = 0;
            for (uint32 rating = 0; rating < MAX_COMBAT_RATING; ++rating)
                if (aurEff->GetMiscValue() & (1 << rating))
                {
                    uint32 temp = GetCaster()->ToPlayer()->GetCombatRating(CombatRating(rating));
                    if (temp > maxrating)
                        maxrating = temp;
                }
            amount = CalculatePct(maxrating, aurEff->GetBaseAmount());
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_rog_t12_set_proc_AuraScript::DoCalcAmount, EFFECT_0, SPELL_AURA_MOD_RATING);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_rog_t12_set_proc_AuraScript();
    }
};

// 59628 - Tricks of the Trade (Proc)
class spell_rog_tricks_of_the_trade_proc : public SpellScriptLoader
{
    public:
        spell_rog_tricks_of_the_trade_proc() : SpellScriptLoader("spell_rog_tricks_of_the_trade_proc") { }

        class spell_rog_tricks_of_the_trade_proc_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_tricks_of_the_trade_proc_AuraScript);

            void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->ResetRedirectThreat();
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_rog_tricks_of_the_trade_proc_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_tricks_of_the_trade_proc_AuraScript();
        }
};

// 6770 - Sap
class spell_rog_sap : public SpellScriptLoader
{
    public:
        spell_rog_sap() : SpellScriptLoader("spell_rog_sap") { }

        class spell_rog_sap_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_sap_AuraScript);

            void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;

                if (AuraEffect * dummy = GetCaster()->GetDummyAuraEffect(SPELLFAMILY_GENERIC, 4900, 0))
                {
                    uint32 spellId = 0;
                    switch (dummy->GetId())
                    {
                        case 79123:
                            spellId = 79124;
                            break;
                        case 79125:
                            spellId = 79126;
                            break;
                    }
                    GetTarget()->CastSpell(GetTarget(), spellId, true);
                }
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_rog_sap_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_sap_AuraScript();
        }
};

// -1776 - Gouge
class spell_rog_gouge : public SpellScriptLoader
{
    public:
        spell_rog_gouge() : SpellScriptLoader("spell_rog_gouge") { }

        class spell_rog_gouge_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_gouge_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) { return true; }

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                const SpellInfo* spellInfo = eventInfo.GetDamageInfo()->GetSpellInfo();
                if (!spellInfo)
                    return true;

                if (spellInfo->GetAllEffectsMechanicMask() & (1 << MECHANIC_BLEED))
                    if (AuraEffect* aurEff = eventInfo.GetActor()->GetDummyAuraEffect(SPELLFAMILY_GENERIC, 4821, 1))
                        if (roll_chance_i(aurEff->GetAmount()))
                            return false;
                return true;
            }

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_rog_gouge_AuraScript::CheckProc);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_gouge_AuraScript();
        }
};

// 2094 - Blind
class spell_rog_blind : public SpellScriptLoader
{
    public:
        spell_rog_blind() : SpellScriptLoader("spell_rog_blind") { }

        class spell_rog_blind_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_blind_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spellInfo*/){ return true; }

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                    if (caster->GetDummyAuraEffect(SPELLFAMILY_ROGUE, 48, 0))
                    {
                        unitTarget->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE, 0, unitTarget->GetAura(32409)); // SW:D shall not be removed.
                        unitTarget->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE_PERCENT, 0, NULL, true, false);
                        unitTarget->RemoveAurasByType(SPELL_AURA_PERIODIC_LEECH);
                    }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_rog_blind_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rog_blind_SpellScript();
        }
};

// -76577 - Smoke bomb
class spell_rog_smoke : public SpellScriptLoader
{
    public:
        spell_rog_smoke() : SpellScriptLoader("spell_rog_smoke") { }

        class spell_rog_smoke_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_smoke_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) { return true; }

            void HandleEffectPeriodic(AuraEffect const* aurEff)
            {
                if (DynamicObject* dyn = GetTarget()->GetDynObject(aurEff->GetId()))
                    GetTarget()->CastSpell(dyn->GetPositionX(), dyn->GetPositionY(), dyn->GetPositionZ(), 88611, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_rog_smoke_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rog_smoke_AuraScript();
        }
};


class spell_rog_revealing_strike : public SpellScriptLoader
{
public:
    spell_rog_revealing_strike() : SpellScriptLoader("spell_rog_revealing_strike") { }

    class spell_rog_revealing_strike_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_rog_revealing_strike_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_ROGUE_REVEALING_STRIKE))
                return false;
            return true;
        }

        void HandleEffect(SpellEffIndex /*effIndex*/)
        {
            /// @HACK: this should be handle by proc of EFFECT_2 of the strike aura
            if (Unit* target = GetHitUnit())
            {
                if (AuraEffect const* strike = target->GetAuraEffect(SPELL_ROGUE_REVEALING_STRIKE, EFFECT_2, GetCaster()->GetGUID()))
                {
                    int32 damage = GetHitDamage();
                    AddPct(damage, strike->GetAmount());
                    SetHitDamage(damage);

                    strike->GetBase()->Remove();
                }
            }
        }

        void Register() override
        {
            if (m_scriptSpellId == 2098)
                OnEffectHitTarget += SpellEffectFn(spell_rog_revealing_strike_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            else
                OnEffectHitTarget += SpellEffectFn(spell_rog_revealing_strike_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_WEAPON_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_rog_revealing_strike_SpellScript();
    }
};

class spell_rog_fan_of_knives : public SpellScriptLoader
{
public:
    spell_rog_fan_of_knives() : SpellScriptLoader("spell_rog_fan_of_knives") { }

    class spell_rog_fan_of_knives_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_rog_fan_of_knives_SpellScript);

        bool Load()
        {
            return GetCaster()->GetTypeId() == TYPEID_PLAYER;
        }

        void HandleAfterHit()
        {
            Player* player = GetCaster()->ToPlayer();

            if (!player->GetDummyAuraEffect(SPELLFAMILY_ROGUE, 857, EFFECT_2))
                return;

            uint8 talentProcChance = 0;
            if (AuraEffect* aurEff = player->GetDummyAuraEffect(SPELLFAMILY_ROGUE, 857, EFFECT_2))
                talentProcChance = aurEff->GetAmount();

            if (Unit* target = GetHitUnit())
            {
                Item* mainItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                Item* offitem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

                if (mainItem)
                {
                    for (uint8 slot = 0; slot < MAX_ENCHANTMENT_SLOT; ++slot)
                    {
                        if (slot != TEMP_ENCHANTMENT_SLOT)
                            continue;

                        SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(mainItem->GetEnchantmentId(EnchantmentSlot(slot)));
                        if (!enchant)
                            continue;

                        for (uint8 s = 0; s < 3; ++s)
                        {
                            if (enchant->type[s] != ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL)
                                continue;

                            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(enchant->spellid[s]);
                            if (!spellInfo)
                                continue;

                            if (spellInfo->SpellFamilyName != SPELLFAMILY_ROGUE || spellInfo->Dispel != DISPEL_POISON)
                                continue;

                            uint8 mainproc = 0;
                            switch (enchant->spellid[s])
                            {
                                case 3409:  // Crippling Poison
                                case 13218: // Wound Poison
                                    mainproc = 100;
                                    break;
                                case 2818:  // Deadly Poison
                                    mainproc = 30;
                                    break;
                                case 5760:  // Mind - numbing Poison
                                case 50328: // Leeching Poison
                                    mainproc = 50;
                                    break;
                                case 8680:  // instant poison
                                    mainproc = 25;
                                    break;
                                case 35201: // Paralytic Poison
                                    mainproc = 20;
                                    break;
                                default:
                                    break;
                            }

                            mainproc = CalculatePct(mainproc, talentProcChance);
                            if (roll_chance_i(mainproc))
                            {
                                if (spellInfo->IsPositive())
                                    player->CastSpell(player, enchant->spellid[s], true, mainItem);
                                else
                                    player->CastSpell(target, enchant->spellid[s], true, mainItem);
                            }
                        }
                    }
                }

                if (offitem)
                {
                    for (uint8 slot = 0; slot < MAX_ENCHANTMENT_SLOT; ++slot)
                    {
                        if (slot != TEMP_ENCHANTMENT_SLOT)
                            continue;

                        SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(offitem->GetEnchantmentId(EnchantmentSlot(slot)));
                        if (!enchant)
                            continue;

                        for (uint8 s = 0; s < 3; ++s)
                        {
                            if (enchant->type[s] != ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL)
                                continue;

                            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(enchant->spellid[s]);
                            if (!spellInfo)
                                continue;

                            if (spellInfo->SpellFamilyName != SPELLFAMILY_ROGUE || spellInfo->Dispel != DISPEL_POISON)
                                continue;

                            uint8 mainproc = 0;
                            switch (enchant->spellid[s])
                            {
                                case 13218: // Wound Poison
                                case 3409:  // Crippling Poison
                                    mainproc = 100;
                                    break;
                                case 2818:  // Deadly Poison
                                    mainproc = 30;
                                    break;
                                case 50328: // Leeching Poison
                                case 5760:  // Mind - numbing Poison
                                    mainproc = 50;
                                    break;
                                case 8680:  // instant poison
                                    mainproc = 25;
                                    break;
                                case 35201: // Paralytic Poison
                                    mainproc = 20;
                                    break;
                                default:
                                    break;
                            }

                            mainproc = CalculatePct(mainproc, talentProcChance);
                            if (roll_chance_i(mainproc))
                            {
                                if (spellInfo->IsPositive())
                                    player->CastSpell(player, enchant->spellid[s], true, mainItem);
                                else
                                    player->CastSpell(target, enchant->spellid[s], true, mainItem);
                            }
                        }
                    }
                }
            }
        }

        void Register()
        {
            AfterHit += SpellHitFn(spell_rog_fan_of_knives_SpellScript::HandleAfterHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_rog_fan_of_knives_SpellScript();
    }
};

class spell_rog_kidney_shot : public SpellScriptLoader
{
public:
    spell_rog_kidney_shot() : SpellScriptLoader("spell_rog_kidney_shot") { }

    class spell_rog_kidney_shot_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_rog_kidney_shot_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_ROGUE_REVEALING_STRIKE))
                return false;
            return true;
        }

        void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            /// @HACK: this should be handle by proc of EFFECT_2 of the strike aura
            if (AuraEffect const* strike = GetTarget()->GetAuraEffect(SPELL_ROGUE_REVEALING_STRIKE, EFFECT_2, GetCasterGUID()))
            {
                int32 duration = GetMaxDuration();
                AddPct(duration, strike->GetAmount());
                SetDuration(duration);

                strike->GetBase()->Remove();
            }
        }

        void Register() override
        {
            AfterEffectApply += AuraEffectApplyFn(spell_rog_kidney_shot_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_rog_kidney_shot_AuraScript();
    }
};

class spell_rog_redirect : public SpellScriptLoader
{
public:
    spell_rog_redirect() : SpellScriptLoader("spell_rog_redirect") { }

    class spell_rog_redirect_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_rog_redirect_SpellScript);

        void HandleEffect(SpellEffIndex /*effIndex*/)
        {
            if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                return;

            if (Player* caster = GetCaster()->ToPlayer())
            {
                if (Unit* target = GetHitUnit())
                {
                    Unit::AuraList& scAuras = caster->GetSingleCastAuras();
                    for (Unit::AuraList::iterator itr = scAuras.begin(); itr != scAuras.end(); ++itr)
                    {
                        Aura* aura = (*itr);
                        if (aura->GetId() == 84748)
                        {
                            if (!target->HasAura(aura->GetId(), caster->GetGUID()))
                            {
                                if (Aura* newAura = caster->AddAura(aura->GetId(), target))
                                {
                                    AuraEffect* oldAuraEff0 = aura->GetEffect(EFFECT_0);
                                    AuraEffect* oldAuraEff1 = aura->GetEffect(EFFECT_1);
                                    AuraEffect* oldAuraEff2 = aura->GetEffect(EFFECT_2);
                                    if (AuraEffect* newAuraEff0 = target->GetAuraEffect(aura->GetId(), EFFECT_0, caster->GetGUID()))
                                        newAuraEff0->SetAmount(oldAuraEff0->GetAmount()); // transfer damage done effect
                                    if (AuraEffect* newAuraEff1 = target->GetAuraEffect(aura->GetId(), EFFECT_1, caster->GetGUID()))
                                        newAuraEff1->SetAmount(oldAuraEff1->GetAmount()); // transfer damage dont melee part
                                    if (AuraEffect* newAuraEff2 = target->GetAuraEffect(aura->GetId(), EFFECT_2, caster->GetGUID()))
                                        newAuraEff2->SetAmount(oldAuraEff2->GetAmount()); // transfer stack part
                                    newAura->SetDuration(aura->GetDuration());
                                    aura->Remove();
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_rog_redirect_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_ADD_COMBO_POINTS);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_rog_redirect_SpellScript();
    }
};

class spell_rog_wound_poison : public SpellScriptLoader
{
public:
    spell_rog_wound_poison() : SpellScriptLoader("spell_rog_wound_poison") { }

    class spell_rog_wound_poison_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_rog_wound_poison_AuraScript);

        void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* target = GetTarget())
                if (target->GetTypeId() == TYPEID_PLAYER)
                    SetDuration(10 * IN_MILLISECONDS);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_rog_wound_poison_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_MOD_HEALING_PCT, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_rog_wound_poison_AuraScript();
    }
};

// rog t12 dps 2p bonus 99174
class spell_rog_t12_helper : public SpellScriptLoader
{
public:
    spell_rog_t12_helper() : SpellScriptLoader("spell_rog_t12_helper") { }

    class spell_rog_t12_helper_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_rog_t12_helper_AuraScript);

        void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
        {
            if (Unit* caster = GetCaster())
            {
                if (Unit* target = eventInfo.GetActionTarget())
                {
                    SpellInfo const* burningWounds = sSpellMgr->GetSpellInfo(SPELL_ROGUE_T12_2P_TRIGGERED);
                    int32 damage = CalculatePct(eventInfo.GetDamageInfo()->GetDamage(), burningWounds->Effects[EFFECT_0].CalcValue(caster));
                    damage /= burningWounds->GetMaxTicks();
                    damage += target->GetRemainingPeriodicAmount(caster->GetGUID(), SPELL_ROGUE_T12_2P_TRIGGERED, SPELL_AURA_PERIODIC_DAMAGE);
                    caster->CastCustomSpell(target, SPELL_ROGUE_T12_2P_TRIGGERED, &damage, NULL, NULL, true);
                }
            }
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(spell_rog_t12_helper_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_rog_t12_helper_AuraScript();
    }
};

class spell_rog_combat_readiness : public SpellScriptLoader
{
public:
    spell_rog_combat_readiness() : SpellScriptLoader("spell_rog_combat_readiness") { }

    class spell_rog_combat_readiness_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_rog_combat_readiness_AuraScript);

        bool Load()
        {
            tickCount = 0;
            return true;
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            return !(eventInfo.GetHitMask() & PROC_EX_ABSORB);
        }

        void CalcPeriodic(AuraEffect const* /*effect*/, bool& isPeriodic, int32& amplitude)
        {
            isPeriodic = true;
            amplitude = 1000;
        }

        void OnPeriodicTick(AuraEffect const* /*aurEff*/)
        {
            if (++tickCount >= 10)
                Remove(AURA_REMOVE_BY_EXPIRE);
        }

        void ResetTimerCheck(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            tickCount = 0;
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(spell_rog_combat_readiness_AuraScript::CheckProc);
            DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_rog_combat_readiness_AuraScript::CalcPeriodic, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_rog_combat_readiness_AuraScript::OnPeriodicTick, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
            OnEffectProc += AuraEffectProcFn(spell_rog_combat_readiness_AuraScript::ResetTimerCheck, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }

        uint8 tickCount;
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_rog_combat_readiness_AuraScript();
    }
};

class spell_rog_bandits_guile : public SpellScriptLoader
{
public:
    spell_rog_bandits_guile() : SpellScriptLoader("spell_rog_bandits_guile") { }

    class spell_rog_bandits_guile_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_rog_bandits_guile_AuraScript);

        void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
        {
            if (Unit* target = eventInfo.GetActionTarget())
            {
                if (Unit* caster = eventInfo.GetActor())
                {
                    // Don't proc anymore when we have the maximum benefit
                    if (caster->HasAura(SPELL_ROGUE_DEEP_INSIGHT))
                        return;

                    if (!target->HasAura(SPELL_ROGUE_BANDITS_GUILE_BONUS, caster->GetGUID()))
                        caster->CastSpell(target, SPELL_ROGUE_BANDITS_GUILE_BONUS, true);

                    if (Aura* guile = target->GetAura(SPELL_ROGUE_BANDITS_GUILE_BONUS, caster->GetGUID()))
                    {
                        AuraEffect* aurEff = guile->GetEffect(EFFECT_2);
                        aurEff->SetAmount(aurEff->GetAmount() + 1);
                        guile->RefreshDuration();

                        switch (aurEff->GetAmount())
                        {
                            case 4:
                                caster->CastSpell(caster, SPELL_ROGUE_SHALLOW_INSIGHT, true);
                                guile->GetEffect(EFFECT_0)->SetAmount(10);
                                guile->GetEffect(EFFECT_1)->SetAmount(10);
                                break;
                            case 8:
                                caster->RemoveAurasDueToSpell(SPELL_ROGUE_SHALLOW_INSIGHT);
                                caster->CastSpell(caster, SPELL_ROGUE_MODERATE_INSIGHT, true);
                                guile->GetEffect(EFFECT_0)->SetAmount(20);
                                guile->GetEffect(EFFECT_1)->SetAmount(20);
                                break;
                            case 12:
                                caster->RemoveAurasDueToSpell(SPELL_ROGUE_MODERATE_INSIGHT);
                                caster->CastSpell(caster, SPELL_ROGUE_DEEP_INSIGHT, true);
                                guile->GetEffect(EFFECT_0)->SetAmount(30);
                                guile->GetEffect(EFFECT_1)->SetAmount(30);
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_rog_bandits_guile_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_rog_bandits_guile_AuraScript();
    }
};

class spell_rog_combat_potency : public SpellScriptLoader
{
public:
    spell_rog_combat_potency() : SpellScriptLoader("spell_rog_combat_potency") { }

    class spell_rog_combat_potency_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_rog_combat_potency_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            if ((eventInfo.GetTypeMask() & PROC_FLAG_DONE_OFFHAND_ATTACK) || eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == 86392)
                return true;
            return false;
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(spell_rog_combat_potency_AuraScript::CheckProc);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_rog_combat_potency_AuraScript();
    }
};

class spell_rog_legendary_dagger_proc : public SpellScriptLoader
{
public:
    spell_rog_legendary_dagger_proc() : SpellScriptLoader("spell_rog_legendary_dagger_proc") { }

    class spell_rog_legendary_dagger_proc_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_rog_legendary_dagger_proc_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            if (Player* player = GetTarget()->ToPlayer())
            {
                // This values are taken from a VALID BLUEPOST DONT CHANGE IT!
                float procChance = 9.438f;
                switch (player->GetPrimaryTalentTree(player->GetActiveSpec()))
                {
                    case TALENT_TREE_ROGUE_ASSASSINATION:
                        procChance = 23.139f;
                        break;
                    case TALENT_TREE_ROGUE_SUBTLETY:
                        procChance = 28.223f;
                        break;
                    case TALENT_TREE_ROGUE_COMBAT:
                        procChance = 9.438f;
                        break;
                    default:
                        break;
                }
                return roll_chance_f(procChance);
            }
            return false;
        }

        void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
        {
            // This part is legendary only
            if (GetId() != SPELL_ROGUE_LEGENDARY_DAGGER_PROC_AURA || GetTarget()->HasAura(SPELL_ROGUE_LEGENDARY_FURY_OF_THE_DESTROYER) || !eventInfo.GetActionTarget())
                return;

            if (Aura* procStacks = GetTarget()->GetAura(SPELL_ROGUE_LEGENDARY_SHADOW_OF_THE_DESTORYER, GetTarget()->GetGUID()))
            {
                uint32 procChance = 1 + procStacks->GetStackAmount();
                if (procChance > 30)
                    if (roll_chance_i(procChance))
                        GetTarget()->CastSpell(eventInfo.GetActionTarget(), SPELL_ROGUE_LEGENDARY_FURY_OF_THE_DESTROYER, true);
            }
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(spell_rog_legendary_dagger_proc_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_rog_legendary_dagger_proc_AuraScript::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_rog_legendary_dagger_proc_AuraScript();
    }
};

void AddSC_rogue_spell_scripts()
{
    new spell_rog_blade_flurry(); // Verified
    new spell_rog_cheat_death(); // Verified
    new spell_rog_deadly_poison(); // Verified
    new spell_rog_nerves_of_steel(); // Verified
    new spell_rog_preparation(); // Verified
    new spell_rog_prey_on_the_weak(); // Verified
    new spell_rog_recuperate(); // Verified 
    new spell_rog_rupture(); // Verified
    new spell_rog_shiv(); // Verified
    new spell_rog_tricks_of_the_trade(); // Verified 
    new spell_rog_tricks_of_the_trade_proc(); // Verified
    new spell_rog_gouge(); // Verified
    new spell_rog_backstab(); // Verified
    new spell_rog_sap(); // Verified
    new spell_rog_smoke(); // Verified
    new spell_rog_blind(); // Verified
    new spell_rog_stealth(); // Verified 
    new spell_rog_revealing_strike(); // Verified
    new spell_rog_fan_of_knives(); // Recently Verified
    new spell_rog_kidney_shot(); // Recently Verified
    new spell_rog_redirect(); // Verified
    new spell_rog_wound_poison(); // Recently Verified, a bit unsure if it's the correct spell id attached in  spell_script_names
    new spell_rog_t12_set_proc(); // Recently Verified, Item - Rogue T12 4P Bonus
    new spell_rog_t12_helper(); // Recently Verified, but really unsure which spell_id goes in here + not working if I attach meele attack + fan of knives
    new spell_rog_combat_readiness(); // Verified
    new spell_rog_bandits_guile(); // Recently Verified, (should) be the right spell
    new spell_rog_combat_potency(); // Recently Verified
    new spell_rog_legendary_dagger_proc(); // Recently Verified, (should be the right spell(s))
}
