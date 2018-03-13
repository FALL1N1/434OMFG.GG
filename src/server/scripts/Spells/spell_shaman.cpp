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
 * Scripts for spells with SPELLFAMILY_SHAMAN and SPELLFAMILY_GENERIC spells used by shaman players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_sha_".
 */

#include "Player.h"
#include "ScriptMgr.h"
#include "GridNotifiers.h"
#include "Unit.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

enum ShamanSpells
{
    SPELL_HUNTER_INSANITY                       = 95809,
    SPELL_MAGE_TEMPORAL_DISPLACEMENT            = 80354,
    SPELL_SHAMAN_ANCESTRAL_AWAKENING_PROC       = 52752,
    SPELL_SHAMAN_BIND_SIGHT                     = 6277,
    SPELL_SHAMAN_EARTH_SHIELD_HEAL              = 379,
    SPELL_SHAMAN_EXHAUSTION                     = 57723,
    SPELL_SHAMAN_FIRE_NOVA_TRIGGERED_R1         = 8349,
    SPELL_SHAMAN_FLAME_SHOCK                    = 8050,
    SPELL_SHAMAN_GLYPH_OF_EARTH_SHIELD          = 63279,
    SPELL_SHAMAN_GLYPH_OF_HEALING_STREAM_TOTEM  = 55456,
    SPELL_SHAMAN_GLYPH_OF_MANA_TIDE             = 55441,
    SPELL_SHAMAN_GLYPH_OF_THUNDERSTORM          = 62132,
    SPELL_SHAMAN_LAVA_FLOWS_R1                  = 51480,
    SPELL_SHAMAN_LAVA_FLOWS_TRIGGERED_R1        = 65264,
    SPELL_SHAMAN_SATED                          = 57724,
    SPELL_SHAMAN_STORM_EARTH_AND_FIRE           = 51483,
    SPELL_SHAMAN_TOTEM_EARTHBIND_EARTHGRAB      = 64695,
    SPELL_SHAMAN_TOTEM_EARTHBIND_TOTEM          = 6474,
    SPELL_SHAMAN_TOTEM_EARTHEN_POWER            = 59566,
    SPELL_SHAMAN_TOTEM_HEALING_STREAM_HEAL      = 52042,
    SPELL_SHAMAN_FEEDBACK_R1                    = 86183,
    SPELL_SHAMAN_ELEMENTAL_MASTERY              = 16166,
    SPELL_SHAMAN_DEEP_HEALING                   = 77226,
    SPELL_SHAMAN_PURIFICATION                   = 16213,
    SPELL_SHAMAN_SOOTHE_R1                      = 16187,
    SPELL_SHAMAN_SOOTHE_R2                      = 16205,
    SPELL_SHAMAN_MEALSTROM_ALERT                = 60349,
    SPELL_SHAMAN_TIMEWALKER                     = 105877,
    SPELL_SHAMAN_MEALSTORM_STACKS               = 53817,
    SPELL_SHAMAN_ENHANCEMENT_T13_2P             = 105866
};

enum ShamanSpellIcons
{
    SHAMAN_ICON_ID_SOOTHING_RAIN                = 2011,
    SHAMAN_ICON_ID_SHAMAN_LAVA_FLOW             = 3087
};

enum ShamanPetCalculate
{
    SPELL_FERAL_SPIRIT_PET_UNK_01      = 35674,
    SPELL_FERAL_SPIRIT_PET_UNK_02      = 35675,
    SPELL_FERAL_SPIRIT_PET_UNK_03      = 35676,
    SPELL_FERAL_SPIRIT_PET_SCALING_04  = 61783,
};

// 52759 - Ancestral Awakening
/// Updated 4.3.4
class spell_sha_ancestral_awakening_proc : public SpellScriptLoader
{
    public:
        spell_sha_ancestral_awakening_proc() : SpellScriptLoader("spell_sha_ancestral_awakening_proc") { }

        class spell_sha_ancestral_awakening_proc_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_ancestral_awakening_proc_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SHAMAN_ANCESTRAL_AWAKENING_PROC))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                int32 damage = GetEffectValue();
                if (GetCaster() && GetHitUnit())
                    GetCaster()->CastCustomWithDelay(200, GetHitUnit(), SPELL_SHAMAN_ANCESTRAL_AWAKENING_PROC, damage, 0, 0, true);
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (targets.size() < 2)
                    return;

                std::list<Unit*> temp;
                for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end(); itr++)
                    if (Unit* unit = (*itr)->ToUnit())
                        temp.push_back(unit);

                targets.clear();
                temp.sort(Trinity::HealthPctOrderPred());
                if (temp.size() > 1)
                    temp.resize(1);
                for (std::list<Unit*>::iterator itr = temp.begin(); itr != temp.end(); itr++)
                    targets.push_back((WorldObject*) (*itr));
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_ancestral_awakening_proc_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_RAID);
                OnEffectHitTarget += SpellEffectFn(spell_sha_ancestral_awakening_proc_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_ancestral_awakening_proc_SpellScript();
        }
};

// Earth shock - Fulmination
class spell_sha_fulmination : public SpellScriptLoader
{
    public:
        spell_sha_fulmination() : SpellScriptLoader("spell_sha_fulmination") { }

        class spell_sha_fulmination_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_fulmination_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                if (AuraEffect* fulmination = caster->GetDummyAuraEffect(SPELLFAMILY_SHAMAN, 2010, EFFECT_0))
                    if (Aura* lShield = caster->GetAura(324, caster->GetGUID()))
                        if (lShield->GetCharges() > fulmination->GetAmount())
                        {
                            uint8 charges = lShield->GetCharges() - fulmination->GetAmount();
                            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(lShield->GetSpellInfo()->Effects[EFFECT_0].TriggerSpell);
                            // Done fixed damage bonus auras
                            int32 bonus  = caster->SpellBaseDamageBonusDone(spellInfo->GetSchoolMask()) * 0.267f;
                            // Unsure about the calculation
                            int32 basepoints0 = spellInfo->Effects[EFFECT_0].CalcValue(caster) + bonus;
                            if (Player* modOwner = caster->GetSpellModOwner())
                                modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_DAMAGE, basepoints0);

                            basepoints0 *= charges;
                            caster->CastCustomSpell(GetHitUnit(), 88767, &basepoints0, NULL, NULL, true);
                            // Remove Glow
                            caster->RemoveAurasDueToSpell(95774);
                            lShield->SetCharges(fulmination->GetAmount());
                        }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_sha_fulmination_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_fulmination_SpellScript();
        }
};

// Ancestral Resolve
class spell_sha_ancestral_resolve : public SpellScriptLoader
{
    public:
        spell_sha_ancestral_resolve() : SpellScriptLoader("spell_sha_ancestral_resolve") { }

        class spell_sha_ancestral_resolve_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_ancestral_resolve_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                Player* target = GetTarget()->ToPlayer();
                if (!target->HasUnitState(UNIT_STATE_CASTING))
                {
                    PreventDefaultAction();
                    return;
                }
                absorbAmount = dmgInfo.GetDamage() * (aurEff->GetBaseAmount() / 100.0f);
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_sha_ancestral_resolve_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_sha_ancestral_resolve_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_ancestral_resolve_AuraScript();
        }
};

// 1064 - Chain Heal
/// Updated 4.3.4
class spell_sha_chain_heal : public SpellScriptLoader
{
    public:
        spell_sha_chain_heal() : SpellScriptLoader("spell_sha_chain_heal") { }

        class spell_sha_chain_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_chain_heal_SpellScript);

            bool Load()
            {
                firstHeal = true;
                riptide = false;
                amount = 0;
                return true;
            }

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                if (firstHeal)
                {
                    // Check if the target has Riptide
                    if (AuraEffect* aurEff = GetHitUnit()->GetAuraEffect(SPELL_AURA_PERIODIC_HEAL, SPELLFAMILY_SHAMAN, 0, 0, 0x10, GetCaster()->GetGUID()))
                    {
                        riptide = true;
                        amount = aurEff->GetSpellInfo()->Effects[EFFECT_2].CalcValue();
                        // Consume it only if the caster don't have Shaman T12 Restoration 4P Bonus
                        if (!GetCaster()->HasAura(99195))
                            GetHitUnit()->RemoveAura(aurEff->GetBase());
                    }
                    firstHeal = false;
                }
                // Riptide increases the Chain Heal effect by 25%
                if (riptide)
                {
                    uint32 bonus = CalculatePct(GetHitHeal(), amount);
                    SetHitHeal(GetHitHeal() + bonus);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_sha_chain_heal_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
            }

            bool firstHeal;
            bool riptide;
            uint32 amount;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_chain_heal_SpellScript();
        }
};

class spell_sha_earth_shield : public SpellScriptLoader
{
    public:
        spell_sha_earth_shield() : SpellScriptLoader("spell_sha_earth_shield") { }

        class spell_sha_earth_shield_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_earth_shield_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SHAMAN_EARTH_SHIELD_HEAL))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_SHAMAN_GLYPH_OF_EARTH_SHIELD))
                    return false;
                return true;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool & /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    amount = caster->SpellHealingBonusDone(GetUnitOwner(), GetSpellInfo(), amount, HEAL);

                    // Glyph of Earth Shield
                    if (AuraEffect* glyph = caster->GetAuraEffect(SPELL_SHAMAN_GLYPH_OF_EARTH_SHIELD, EFFECT_0))
                        AddPct(amount, glyph->GetAmount());
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_sha_earth_shield_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_earth_shield_AuraScript();
        }
};

// 6474 - Earthbind Totem - Fix Talent:Earthen Power, Earth's Grasp
/// Updated 4.3.4
class spell_sha_earthbind_totem : public SpellScriptLoader
{
    public:
        spell_sha_earthbind_totem() : SpellScriptLoader("spell_sha_earthbind_totem") { }

        class spell_sha_earthbind_totem_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_earthbind_totem_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SHAMAN_TOTEM_EARTHBIND_TOTEM) || !sSpellMgr->GetSpellInfo(SPELL_SHAMAN_TOTEM_EARTHEN_POWER))
                    return false;
                return true;
            }

            void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
            {
                if (!GetCaster())
                    return;
                if (Player* owner = GetCaster()->GetCharmerOrOwnerPlayerOrPlayerItself())
                    if (AuraEffect* aur = owner->GetDummyAuraEffect(SPELLFAMILY_SHAMAN, 2289, 0))
                        if (roll_chance_i(aur->GetBaseAmount()))
                            GetTarget()->CastSpell((Unit*)NULL, SPELL_SHAMAN_TOTEM_EARTHEN_POWER, true);
            }

            void Apply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;
                Player* owner = GetCaster()->GetCharmerOrOwnerPlayerOrPlayerItself();
                if (!owner)
                    return;
                // Earth's Grasp
                if (AuraEffect* aurEff = owner->GetDummyAuraEffect(SPELLFAMILY_SHAMAN, 20, EFFECT_1))
                    if (roll_chance_i(aurEff->GetAmount()))
                        GetCaster()->CastSpell(GetCaster(), SPELL_SHAMAN_TOTEM_EARTHBIND_EARTHGRAB, false);
            }

            void Register()
            {
                 OnEffectPeriodic += AuraEffectPeriodicFn(spell_sha_earthbind_totem_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
                 OnEffectApply += AuraEffectApplyFn(spell_sha_earthbind_totem_AuraScript::Apply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_earthbind_totem_AuraScript();
        }
};

class EarthenPowerTargetSelector
{
    public:
        EarthenPowerTargetSelector() { }

        bool operator() (WorldObject* target)
        {
            if (!target->ToUnit())
                return true;

            if (!target->ToUnit()->HasAuraWithMechanic(1 << MECHANIC_SNARE))
                return true;

            return false;
        }
};

// 59566 - Earthen Power
class spell_sha_earthen_power : public SpellScriptLoader
{
    public:
        spell_sha_earthen_power() : SpellScriptLoader("spell_sha_earthen_power") { }

        class spell_sha_earthen_power_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_earthen_power_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                unitList.remove_if(EarthenPowerTargetSelector());
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_earthen_power_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_earthen_power_SpellScript();
        }
};

// 1535 Fire Nova
/// Updated 4.3.4
class spell_sha_fire_nova : public SpellScriptLoader
{
    public:
        spell_sha_fire_nova() : SpellScriptLoader("spell_sha_fire_nova") { }

        class spell_sha_fire_nova_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_fire_nova_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                for (std::list<WorldObject*>::const_iterator itr = unitList.begin(); itr != unitList.end();)
                {
                    WorldObject* temp = (*itr);
                    itr++;
                    if (!temp->ToUnit() || !temp->ToUnit()->HasAura(SPELL_SHAMAN_FLAME_SHOCK, GetCaster()->GetGUID()))
                        unitList.remove(temp);
                }
            }


            SpellCastResult CheckCast()
            {
                std::list<Unit*> targets;
                Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(GetCaster(), GetCaster(), 100.0f);
                Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(GetCaster(), targets, u_check);
                GetCaster()->VisitNearbyObject(100.0f, searcher);
                for (std::list<Unit*>::const_iterator itr = targets.begin(); itr != targets.end();)
                {
                    Unit* temp = (*itr);
                    itr++;
                    if (!temp->HasAura(SPELL_SHAMAN_FLAME_SHOCK, GetCaster()->GetGUID()))
                        targets.remove(temp);
                }
                if (!targets.size())
                {
                    SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_FLAME_SHOCK_NOT_ACTIVE);
                    return SPELL_FAILED_CUSTOM_ERROR;
                }
                return SPELL_CAST_OK;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetHitUnit())
                    caster->CastSpell(target, SPELL_SHAMAN_FIRE_NOVA_TRIGGERED_R1, true);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_fire_nova_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnCheckCast += SpellCheckCastFn(spell_sha_fire_nova_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_sha_fire_nova_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_fire_nova_SpellScript();
        }
};

// 8050 -Flame Shock
/// Updated 4.3.4
class spell_sha_flame_shock : public SpellScriptLoader
{
    public:
        spell_sha_flame_shock() : SpellScriptLoader("spell_sha_flame_shock") { }

        class spell_sha_flame_shock_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_flame_shock_AuraScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SHAMAN_LAVA_FLOWS_R1))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_SHAMAN_LAVA_FLOWS_TRIGGERED_R1))
                    return false;
                return true;
            }

            void HandleDispel(DispelInfo* /*dispelInfo*/)
            {
                if (Unit* caster = GetCaster())
                {
                    // Lava Flows
                    if (AuraEffect const* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_SHAMAN, SHAMAN_ICON_ID_SHAMAN_LAVA_FLOW, EFFECT_0))
                    {
                        int32 basepoints = aurEff->GetAmount();
                        caster->CastCustomSpell(caster, SPELL_SHAMAN_LAVA_FLOWS_TRIGGERED_R1, &basepoints, NULL, NULL, true);
                    }
                }
            }

            void Register()
            {
                AfterDispel += AuraDispelFn(spell_sha_flame_shock_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_flame_shock_AuraScript();
        }
};

// Lava Surge!
class spell_sha_lava_surge : public SpellScriptLoader
{
    public:
        spell_sha_lava_surge() : SpellScriptLoader("spell_sha_lava_surge") { }

        class spell_sha_lava_surge_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_lava_surge_SpellScript);

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                if (Player* caster = GetCaster()->ToPlayer())
                {
                    if (caster->HasSpellCooldown(51505))
                        caster->RemoveSpellCooldown(51505, true);

                    if (caster->HasAura(99206)) // Shaman T12 Elemental 4P Bonus
                        caster->CastSpell(caster, 99207, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_sha_lava_surge_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_lava_surge_SpellScript();
        }
};

// 52041 - Healing Stream Totem
/// Updated 4.3.4
class spell_sha_healing_stream_totem : public SpellScriptLoader
{
    public:
        spell_sha_healing_stream_totem() : SpellScriptLoader("spell_sha_healing_stream_totem") { }

        class spell_sha_healing_stream_totem_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_healing_stream_totem_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                return sSpellMgr->GetSpellInfo(SPELL_SHAMAN_TOTEM_HEALING_STREAM_HEAL);
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                int32 damage = GetEffectValue();

                SpellInfo const* triggeringSpell = GetTriggeringSpell();

                if (Unit* target = GetHitUnit())
                {
                    if (Unit* caster = GetCaster())
                    {
                        if (Unit* owner = caster->GetOwner())
                        {
                            if (triggeringSpell)
                                damage = triggeringSpell->Effects[EFFECT_0].CalcValue(caster);

                            float bonus = 1.0f;

                            if (AuraEffect* soother1 = owner->GetAuraEffect(SPELL_SHAMAN_SOOTHE_R1, EFFECT_0))
                                AddPct(bonus, soother1->GetAmount());

                            if (AuraEffect* soother2 = owner->GetAuraEffect(SPELL_SHAMAN_SOOTHE_R2, EFFECT_0))
                                AddPct(bonus, soother2->GetAmount());

                            if (owner->GetAura(SPELL_SHAMAN_PURIFICATION))
                                AddPct(bonus, 10);

                            damage += owner->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL) * 0.0827;
                            damage *= bonus;

                            if (owner->GetAura(SPELL_SHAMAN_DEEP_HEALING))
                                AddPct(damage, (owner->ToPlayer() ? owner->ToPlayer()->GetMasteryAmount(SPELL_SHAMAN_DEEP_HEALING, EFFECT_0) : 0) *
                                (1.0f - (target->GetHealthPct() / 100.0f)));

                            caster->CastCustomSpell(target, SPELL_SHAMAN_TOTEM_HEALING_STREAM_HEAL, &damage, 0, 0, true, 0, 0, GetOriginalCaster()->GetGUID());
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_sha_healing_stream_totem_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_healing_stream_totem_SpellScript();
        }
};

class spell_sha_healing_stream_totem_triggered : public SpellScriptLoader
{
    public:
        spell_sha_healing_stream_totem_triggered () : SpellScriptLoader("spell_sha_healing_stream_totem_triggered") { }

        class spell_sha_healing_stream_totem_triggered_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_healing_stream_totem_triggered_AuraScript);

            void Apply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (Unit* owner = caster->GetOwner())
                        if (owner->GetDummyAuraEffect(SPELLFAMILY_SHAMAN, 1647, EFFECT_0))
                            caster->CastSpell(caster, 8185, true);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_sha_healing_stream_totem_triggered_AuraScript::Apply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_healing_stream_totem_triggered_AuraScript();
        }
};

// Healing rain
class spell_sha_healing_rain : public SpellScriptLoader
{
    public:
        spell_sha_healing_rain() : SpellScriptLoader("spell_sha_healing_rain") { }

        class spell_sha_healing_rain_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_healing_rain_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) { return true; }

            void HandleEffectPeriodic(AuraEffect const* aurEff)
            {
                if (DynamicObject* dyn = GetTarget()->GetDynObject(aurEff->GetId()))
                    GetTarget()->CastSpell(dyn->GetPositionX(), dyn->GetPositionY(), dyn->GetPositionZ(), 73921, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_sha_healing_rain_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_healing_rain_AuraScript();
        }
};

// Earthquake
class spell_sha_earthquake : public SpellScriptLoader
{
    public:
        spell_sha_earthquake() : SpellScriptLoader("spell_sha_earthquake") { }

        class spell_sha_earthquake_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_earthquake_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) { return true; }

            void HandleEffectPeriodic(AuraEffect const* aurEff)
            {
                if (DynamicObject* dyn = GetTarget()->GetDynObject(aurEff->GetId()))
                    GetTarget()->CastSpell(dyn->GetPositionX(), dyn->GetPositionY(), dyn->GetPositionZ(), 77478, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_sha_earthquake_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_earthquake_AuraScript();
        }
};

// earthquake triggered
class spell_sha_earthquake_trigger : public SpellScriptLoader
{
    public:
        spell_sha_earthquake_trigger() : SpellScriptLoader("spell_sha_earthquake_trigger") { }

        class spell_sha_earthquake_trigger_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_earthquake_trigger_SpellScript);

            void HandleScriptEffect(SpellEffIndex effIndex)
            {
                if (roll_chance_i(GetSpellInfo()->Effects[effIndex].BasePoints))
                    if (!GetHitUnit()->HasAura(77505, GetCaster()->GetGUID()))
                        GetCaster()->CastSpell(GetHitUnit(), 77505, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_sha_earthquake_trigger_SpellScript::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_earthquake_trigger_SpellScript();
        }
};

// Healing rain triggered
class spell_sha_healing_rain_trigger : public SpellScriptLoader
{
    public:
        spell_sha_healing_rain_trigger() : SpellScriptLoader("spell_sha_healing_rain_trigger") { }

        class spell_sha_healing_rain_trigger_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_healing_rain_trigger_SpellScript);

            bool Load()
            {
                _targets = 0;
                return true;
            }

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                if (_targets > 6)
                    SetHitHeal((GetHitHeal() * 6) / _targets);
            }

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                _targets = unitList.size();
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_healing_rain_trigger_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
                OnEffectHitTarget += SpellEffectFn(spell_sha_healing_rain_trigger_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
            }

            uint32 _targets;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_healing_rain_trigger_SpellScript();
        }
};

// Improved Lava Lash trigger
class spell_sha_lava_lash_trigger : public SpellScriptLoader
{
    public:
        spell_sha_lava_lash_trigger() : SpellScriptLoader("spell_sha_lava_lash_trigger") { }

        class spell_sha_lava_lash_trigger_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_lava_lash_trigger_SpellScript)

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                targets = unitList;

                if (GetExplTargetUnit())
                    targets.remove(GetExplTargetUnit());

                targets.remove_if(Trinity::UnitAuraCheck(true, 8050, GetCaster()->GetGUID()));

                if (!targets.empty())
                    Trinity::Containers::RandomResizeList(targets, 4);
            }

            void HandleOnHit()
            {
                if(!GetCaster()->GetDummyAuraEffect(SPELLFAMILY_SHAMAN, 4780, EFFECT_0))
                    return;

                if (Unit* target = GetHitUnit())
                    if (Aura* flameShock = target->GetAura(8050, GetCaster()->GetGUID()))
                        for (std::list<WorldObject*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                            if (Unit* triggerTarget = (*itr)->ToUnit())
                                GetCaster()->AddAuraForTarget(flameShock, triggerTarget);
            }


            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_lava_lash_trigger_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
                OnHit += SpellHitFn(spell_sha_lava_lash_trigger_SpellScript::HandleOnHit);
            }

        private:
            std::list<WorldObject*> targets;

        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_lava_lash_trigger_SpellScript();
        }
};

// 16191 - Mana Tide
/// Updated 4.3.4
class spell_sha_mana_tide_totem : public SpellScriptLoader
{
    public:
        spell_sha_mana_tide_totem() : SpellScriptLoader("spell_sha_mana_tide_totem") { }

        class spell_sha_mana_tide_totem_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_mana_tide_totem_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* owner = caster->GetOwner())
                    {
                        uint32 baseSpirit = owner->GetStat(STAT_SPIRIT);
                        Unit::AuraEffectList const& auras = owner->GetAuraEffectsByType(SPELL_AURA_MOD_STAT);
                        for (Unit::AuraEffectList::const_iterator i = auras.begin(); i != auras.end(); ++i)
                        {
                            if ((*i)->GetMiscValue() != UNIT_MOD_STAT_SPIRIT || ((*i)->GetBase() && ((*i)->GetBase()->GetMaxDuration() >= 30000 || (*i)->GetBase()->GetMaxDuration() < 1 && (*i)->GetId() != GetId())))
                                continue;

                            if ((*i)->GetAmount() > 0)
                                baseSpirit -= (*i)->GetAmount();
                        }
                        amount = CalculatePct(baseSpirit, amount);
                    }
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_sha_mana_tide_totem_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_STAT);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_mana_tide_totem_AuraScript();
        }
};

// -51490 - Thunderstorm
class spell_sha_thunderstorm : public SpellScriptLoader
{
    public:
        spell_sha_thunderstorm() : SpellScriptLoader("spell_sha_thunderstorm") { }

        class spell_sha_thunderstorm_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_thunderstorm_SpellScript);

            void HandleKnockBack(SpellEffIndex effIndex)
            {
                if (Unit* caster = GetCaster())
                {
                    // Glyph of Thunderstorm
                    if (caster->HasAura(SPELL_SHAMAN_GLYPH_OF_THUNDERSTORM))
                        PreventHitDefaultEffect(effIndex);
                    else
                        caster->CastSpell(GetHitUnit(), 100955, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_sha_thunderstorm_SpellScript::HandleKnockBack, EFFECT_2, SPELL_EFFECT_KNOCK_BACK);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_thunderstorm_SpellScript();
        }
};

// Frozen power
class spell_sha_frozen_power : public SpellScriptLoader
{
    public:
        spell_sha_frozen_power() : SpellScriptLoader("spell_sha_frozen_power") { }

        class spell_sha_frozen_power_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_frozen_power_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                // Frozen power
                if (AuraEffect* aur = caster->GetDummyAuraEffect(SPELLFAMILY_SHAMAN, 3780, EFFECT_1))
                    if (roll_chance_i(aur->GetAmount()))
                        if (caster->GetDistance(GetHitUnit()) > 15.0f)
                            caster->CastSpell(GetHitUnit(), 63685, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_sha_frozen_power_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_frozen_power_SpellScript();
        }
};

// Searing Flames
class spell_sha_searing_flames : public SpellScriptLoader
{
    public:
        spell_sha_searing_flames() : SpellScriptLoader("spell_sha_searing_flames") { }

        class spell_sha_searing_flames_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_searing_flames_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                // Searing Flames
                if (Unit* owner = caster->GetOwner())
                    if (AuraEffect* aur = owner->GetDummyAuraEffect(SPELLFAMILY_SHAMAN, 680, EFFECT_0))
                        if (roll_chance_i(aur->GetAmount()))
                        {
                            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(77661);
                            uint32 ticks = spellInfo->GetDuration() / spellInfo->Effects[EFFECT_0].Amplitude;
                            int32 basepoints0 = GetHitDamage() / ticks;
                            caster->CastCustomSpell(GetHitUnit(), 77661, &basepoints0, NULL, NULL, true, NULL, NULL, owner->GetGUID());
                        }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_sha_searing_flames_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_searing_flames_SpellScript();
        }
};

// Unleash Elements
class spell_sha_unleash_elements : public SpellScriptLoader
{
    public:
        spell_sha_unleash_elements() : SpellScriptLoader("spell_sha_unleash_elements") { }

        class spell_sha_unleash_elements_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_unleash_elements_SpellScript);

            SpellCastResult CheckCast()
            {
                Player* caster = GetCaster()->ToPlayer();
                if (!caster)
                    return SPELL_CAST_OK;

                if (Unit* target = GetExplTargetUnit())
                {
                    bool isFriendly = caster->IsFriendlyTo(target);
                    bool anyEnchant = false;
                    Item *weapons[2];
                    weapons[0] = caster->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                    weapons[1] = caster->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    for (int i = 0; i < 2; i++)
                    {
                        if (!weapons[i])
                            continue;

                        switch (weapons[i]->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                        {
                            case 3345:
                                anyEnchant = true;
                                break;
                            case 5: // Flametongue Weapon
                            case 2: // Frostbrand Weapon
                            case 3021: // Rockbiter Weapon
                            case 283: // Windfury Weapon
                                if (isFriendly)
                                    return SPELL_FAILED_BAD_TARGETS;
                                anyEnchant = true;
                                break;
                        }
                    }
                    if (!anyEnchant)
                    {
                        SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_NO_ACTIVE_ENCHANTMENT);
                        return SPELL_FAILED_CUSTOM_ERROR;
                    }
                }
                return SPELL_CAST_OK;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Player* player = GetCaster()->ToPlayer();
                if (!player)
                    return;

                Unit* target = GetHitUnit();
                Item *weapons[2];
                weapons[0] = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                weapons[1] = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

                for (int i = 0; i < 2; i++)
                {
                    if (!weapons[i])
                        continue;

                    switch (weapons[i]->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                    {
                        case 3345: // Earthliving Weapon
                            // Unleash Life
                            if (!target->IsFriendlyTo(player))
                                target = player;
                            player->CastSpell(target, 73685, true);
                            break;
                        case 5: // Flametongue Weapon
                            // Unleash Flame
                            player->CastSpell(target, 73683, true);
                            break;
                        case 2: // Frostbrand Weapon
                            // Unleash Frost
                            player->CastSpell(target, 73682, true);
                            break;
                        case 3021: // Rockbiter Weapon
                            // Unleash Earth
                            player->CastSpell(target, 73684, true);
                            break;
                        case 283: // Windfury Weapon
                            // Unleash Wind
                            player->CastSpell(target, 73681, true);
                            break;
                    }
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_sha_unleash_elements_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_sha_unleash_elements_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_unleash_elements_SpellScript();
        }
};

// Spirit link
class spell_sha_spirit_link : public SpellScriptLoader
{
    public:
        spell_sha_spirit_link() : SpellScriptLoader("spell_sha_spirit_link") { }

        class spell_sha_spirit_link_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_spirit_link_SpellScript);

            bool Load()
            {
                averagePercentage = 0.0f;
                return true;
            }

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                uint32 targetCount = 0;
                for (std::list<WorldObject*>::const_iterator itr = unitList.begin(); itr != unitList.end();)
                {
                    if (Unit* target = (*itr)->ToUnit())
                    {
                        if (target->HasAura(76577)) // smokebomb should block spirit link heal
                            unitList.remove(*itr++);
                        else
                        {
                            targets[target->GetGUID()] = target->GetHealthPct();
                            averagePercentage += target->GetHealthPct();
                            ++targetCount;
                            ++itr;
                        }
                    }
                }

                averagePercentage /= targetCount;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetHitUnit())
                {
                    if (targets.find(target->GetGUID()) == targets.end())
                        return;

                    int32 bp0 = 0;
                    int32 bp1 = 0;
                    float percentage = targets[target->GetGUID()];
                    uint32 currentHp = target->CountPctFromMaxHealth(percentage);
                    uint32 desiredHp = target->CountPctFromMaxHealth(averagePercentage);
                    if (desiredHp > currentHp)
                        bp1 = desiredHp - currentHp;
                    else
                        bp0 = currentHp - desiredHp;
                    GetCaster()->CastCustomSpell(target, 98021, &bp0, &bp1, NULL, true);
                }

            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_spirit_link_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_RAID);
                OnEffectHitTarget += SpellEffectFn(spell_sha_spirit_link_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }

        private:
            std::map<uint64, float> targets;
            float averagePercentage;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_spirit_link_SpellScript();
        }
};

// Devour Magic
class spell_sha_cleanse: public SpellScriptLoader
{
    public:
        spell_sha_cleanse() : SpellScriptLoader("spell_sha_cleanse") { }

        class spell_sha_cleanse_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_cleanse_SpellScript);

            void HandleOnDispel()
            {
                // Cleansing Waters
                if (AuraEffect* aura = GetCaster()->GetDummyAuraEffect(SPELLFAMILY_SHAMAN, 2020, EFFECT_0))
                {
                    switch (aura->GetId())
                    {
                        case 86959:
                            GetCaster()->CastSpell(GetHitUnit(), 86961, true);
                            break;
                        case 86962:
                            GetCaster()->CastSpell(GetHitUnit(), 86958, true);
                            break;
                    }
                }
            }

            void Register()
            {
                OnSuccessfulDispel += SpellDispelFn(spell_sha_cleanse_SpellScript::HandleOnDispel);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_cleanse_SpellScript();
        }
};

class spell_sha_shamanistic_rage : public SpellScriptLoader
{
public:
    spell_sha_shamanistic_rage() : SpellScriptLoader("spell_sha_shamanistic_rage") { }

    class spell_sha_shamanistic_rage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_sha_shamanistic_rage_SpellScript);

        SpellCastResult CheckCast()
        {
            // "Wrath-Logo-Small Patch 3.2.2 (2009-09-22): Thunderstorm and Shamanistic Rage can no longer be used while Frozen, Cycloned, Sapped, or Incapacitated."
            if (Unit* caster = GetCaster())
                if (caster->HasAuraWithMechanic((1 << MECHANIC_BANISH) | (1 << MECHANIC_SAPPED) | (1 << MECHANIC_FREEZE)))
                    return SPELL_FAILED_DONT_REPORT;

            return SPELL_CAST_OK;
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_sha_shamanistic_rage_SpellScript::CheckCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_sha_shamanistic_rage_SpellScript();
    }
};

class spell_sha_t12_taming_the_flames : public SpellScriptLoader
{
public:
    spell_sha_t12_taming_the_flames() : SpellScriptLoader("spell_sha_t12_taming_the_flames") { }

    class spell_sha_t12_taming_the_flames_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_sha_t12_taming_the_flames_SpellScript);

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            if (Player* caster = GetCaster()->ToPlayer())
            {
                int32 cooldown = 2894;
                int32 reducedCooldown = 4000;
                if (caster->HasSpellCooldown(cooldown))
                {
                    uint32 newCooldownDelay = caster->GetSpellCooldownDelay(cooldown);
                    if (newCooldownDelay < uint32(reducedCooldown / 1000))
                        newCooldownDelay = 0;
                    else
                        newCooldownDelay -= uint32(reducedCooldown / 1000);
                    caster->AddSpellCooldown(cooldown, 0, uint32(time(NULL) + newCooldownDelay));

                    WorldPacket data(SMSG_MODIFY_COOLDOWN, 4 + 8 + 4);
                    data << uint32(cooldown);
                    data << uint64(caster->GetGUID());
                    data << int32(-reducedCooldown);
                    caster->GetSession()->SendPacket(&data);
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_sha_t12_taming_the_flames_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_sha_t12_taming_the_flames_SpellScript();
    }
};

class spell_sha_feedback : public SpellScriptLoader
{
public:
    spell_sha_feedback() : SpellScriptLoader("spell_sha_feedback") {}

    class spell_sha_feedback_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sha_feedback_AuraScript);

        bool CheckProc(ProcEventInfo& /*eventInfo*/)
        {
            return !GetTarget()->HasSpellCooldown(GetId());
        }

        void HandleCooldown(ProcEventInfo& /*eventInfo*/)
        {
            if (Player* caster = GetTarget()->ToPlayer())
            {
                if (AuraEffect* feedback = caster->GetAuraEffectOfRankedSpell(SPELL_SHAMAN_FEEDBACK_R1, EFFECT_0))
                {
                    int32 reducedCooldown = -feedback->GetAmount();
                    if (caster->HasSpellCooldown(SPELL_SHAMAN_ELEMENTAL_MASTERY))
                    {
                        uint32 newCooldownDelay = caster->GetSpellCooldownDelay(SPELL_SHAMAN_ELEMENTAL_MASTERY);
                        if (newCooldownDelay < uint32(reducedCooldown / 1000))
                            newCooldownDelay = 0;
                        else
                            newCooldownDelay -= uint32(reducedCooldown / 1000);
                        caster->AddSpellCooldown(SPELL_SHAMAN_ELEMENTAL_MASTERY, 0, uint32(time(NULL) + newCooldownDelay));
                        caster->AddSpellCooldown(GetId(), 0, uint32(time(NULL) + 1));
                        WorldPacket data(SMSG_MODIFY_COOLDOWN, 4 + 8 + 4);
                        data << uint32(SPELL_SHAMAN_ELEMENTAL_MASTERY); // Spell ID
                        data << uint64(caster->GetGUID());              // Player GUID
                        data << int32(-reducedCooldown);                // Cooldown mod in milliseconds
                        caster->GetSession()->SendPacket(&data);
                    }
                }
            }
        }

        void Register()
        {
            OnProc += AuraProcFn(spell_sha_feedback_AuraScript::HandleCooldown);
            DoCheckProc += AuraCheckProcFn(spell_sha_feedback_AuraScript::CheckProc);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_sha_feedback_AuraScript();
    }
};

class spell_mealstrom_weapon_visual : public SpellScriptLoader
{
public:
    spell_mealstrom_weapon_visual() : SpellScriptLoader("spell_mealstrom_weapon_visual") { }

    class spell_mealstrom_weapon_visual_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mealstrom_weapon_visual_AuraScript);

        void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (GetAura()->GetStackAmount() >= 5)
                GetCaster()->CastSpell(GetCaster(), SPELL_SHAMAN_MEALSTROM_ALERT, true);
        }

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            GetCaster()->RemoveAurasDueToSpell(SPELL_SHAMAN_MEALSTROM_ALERT);
        }

        void Register() override
        {
            AfterEffectApply += AuraEffectApplyFn(spell_mealstrom_weapon_visual_AuraScript::OnApply, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            AfterEffectRemove += AuraEffectRemoveFn(spell_mealstrom_weapon_visual_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_mealstrom_weapon_visual_AuraScript();
    }
};

class spell_sha_spiritwalkers_grace : public SpellScriptLoader
{
public:
    spell_sha_spiritwalkers_grace() : SpellScriptLoader("spell_sha_spiritwalkers_grace") { }

    class spell_sha_spiritwalkers_grace_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sha_spiritwalkers_grace_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_SHAMAN_TIMEWALKER))
                return false;
            return true;
        }

        void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            // Shaman T13 Restoration 4P Bonus (Spiritwalker's Grace)
            if (GetTarget()->HasAura(105876, GetTarget()->GetGUID()))
                GetTarget()->CastSpell(GetTarget(), SPELL_SHAMAN_TIMEWALKER, true);
        }

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            GetTarget()->RemoveAurasDueToSpell(SPELL_SHAMAN_TIMEWALKER);
        }

        void Register() override
        {
            AfterEffectApply += AuraEffectApplyFn(spell_sha_spiritwalkers_grace_AuraScript::OnApply, EFFECT_0, SPELL_AURA_CAST_WHILE_WALKING, AURA_EFFECT_HANDLE_REAL);
            AfterEffectRemove += AuraEffectRemoveFn(spell_sha_spiritwalkers_grace_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_CAST_WHILE_WALKING, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_sha_spiritwalkers_grace_AuraScript();
    }
};

class spell_sha_t13_2p_enhancement : public SpellScriptLoader
{
public:
    spell_sha_t13_2p_enhancement() : SpellScriptLoader("spell_sha_t13_2p_enhancement") { }

    class spell_sha_t13_2p_enhancement_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_sha_t13_2p_enhancement_SpellScript);

        bool Load()
        {
            applyBonus = false;
            return true;
        }

        void CheckStacks()
        {
            applyBonus = GetCaster()->HasAura(SPELL_SHAMAN_MEALSTORM_STACKS, GetCaster()->GetGUID()) && GetCaster()->HasAura(SPELL_SHAMAN_ENHANCEMENT_T13_2P, GetCaster()->GetGUID());
        }

        void HandleHit(SpellEffIndex /*effIndex*/)
        {
            if (applyBonus)
            {
                int32 bonus = CalculatePct(GetHitDamage(), 20);
                SetHitDamage(GetHitDamage() + bonus);
            }
        }

        private:
            bool applyBonus;

        void Register()
        {
            OnCast += SpellCastFn(spell_sha_t13_2p_enhancement_SpellScript::CheckStacks);
            OnEffectHitTarget += SpellEffectFn(spell_sha_t13_2p_enhancement_SpellScript::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }

    };

    SpellScript* GetSpellScript() const
    {
        return new spell_sha_t13_2p_enhancement_SpellScript();
    }
};

class spell_sha_t13_2p_resto : public SpellScriptLoader
{
public:
    spell_sha_t13_2p_resto() : SpellScriptLoader("spell_sha_t13_2p_resto") {}

    class spell_sha_t13_2p_resto_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sha_t13_2p_resto_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == 16190;
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(spell_sha_t13_2p_resto_AuraScript::CheckProc);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_sha_t13_2p_resto_AuraScript();
    }
};

class spell_sha_focused_insight : public SpellScriptLoader
{
public:
    spell_sha_focused_insight() : SpellScriptLoader("spell_sha_focused_insight") {}

    class spell_sha_focused_insight_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sha_focused_insight_AuraScript);

        void HandleProc(ProcEventInfo& eventInfo)
        {
            if (eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == 73921)
            {
                int32 amount = GetEffect(EFFECT_1)->GetAmount();
                GetCaster()->CastCustomSpell(GetCaster(), 96300, &amount, NULL, NULL, true);
            }
        }

        void Register()
        {
            OnProc += AuraProcFn(spell_sha_focused_insight_AuraScript::HandleProc);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_sha_focused_insight_AuraScript();
    }
};

void AddSC_shaman_spell_scripts()
{
    new spell_sha_ancestral_awakening_proc(); // Verified
    new spell_sha_chain_heal(); // Verified
    new spell_sha_earthbind_totem(); // Verified
    new spell_sha_earthen_power(); // Verified
    new spell_sha_fire_nova(); // Verified
    new spell_sha_flame_shock(); // Verified
    new spell_sha_healing_stream_totem(); // Verified
    new spell_sha_mana_tide_totem(); // Verified
    new spell_sha_thunderstorm(); // Verified
    new spell_sha_ancestral_resolve(); // Verified
    new spell_sha_healing_rain_trigger(); // Verified
    new spell_sha_healing_rain(); // Verified
    new spell_sha_spirit_link(); // Verified
    new spell_sha_unleash_elements(); // Verified 
    new spell_sha_frozen_power(); // Verified
    new spell_sha_searing_flames(); // Verified 
    new spell_sha_lava_lash_trigger(); // Verified
    new spell_sha_fulmination(); // Verified 
    new spell_sha_lava_surge(); // Verified 
    new spell_sha_earthquake_trigger(); // Verified 
    new spell_sha_earthquake(); // Verified
    new spell_sha_healing_stream_totem_triggered(); // Verified
    new spell_sha_earth_shield(); // Verified
    new spell_sha_cleanse(); // Verified
    new spell_sha_shamanistic_rage(); // Recently Verified 
    new spell_sha_t12_taming_the_flames(); // Recently Verified 
    new spell_sha_feedback(); // Recently Verified
    new spell_mealstrom_weapon_visual(); // Recently Verified
    new spell_sha_spiritwalkers_grace(); // Recently Verified
    new spell_sha_t13_2p_enhancement(); // Recently Verified
    new spell_sha_t13_2p_resto(); // Recently Verified BUT the T13 2p resto still doesn't seem to work. Perhaps spell_proc_event? I still don't understand this particular spell script, the effect tho.
    new spell_sha_focused_insight(); // Recently Verified, TC.
}
