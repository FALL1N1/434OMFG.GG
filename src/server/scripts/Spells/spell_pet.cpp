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
 * Scripts for spells with SPELLFAMILY_DEATHKNIGHT and SPELLFAMILY_GENERIC spells used by deathknight players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_dk_".
 */

#include "GridNotifiers.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Unit.h"
#include "Player.h"
#include "Pet.h"

enum MiscPetSpells
{
    SPELL_HUNTER_PET_LAST_STAND_TRIGGERED           = 53479,
    SPELL_HUNTER_PET_HEART_OF_THE_PHOENIX           = 55709,
    SPELL_HUNTER_PET_HEART_OF_THE_PHOENIX_TRIGGERED = 54114,
    SPELL_HUNTER_PET_HEART_OF_THE_PHOENIX_DEBUFF    = 55711,
    SPELL_HUNTER_PET_CARRION_FEEDER_TRIGGERED       = 54045,
    SPELL_PRIEST_MANA_LEECH_PROC                    = 34650,
    SPELL_PRIEST_SHADOWFLAME                        = 99156,
    SPELL_WARLOCK_GLYPH_OF_VOIDWALKER               = 56247
};

enum PetCalculate
{
    SPELL_TAMED_PET_PASSIVE_06      = 19591,
    SPELL_TAMED_PET_PASSIVE_07      = 20784,
    SPELL_TAMED_PET_PASSIVE_08      = 34666,
    SPELL_TAMED_PET_PASSIVE_09      = 34667,
    SPELL_TAMED_PET_PASSIVE_10      = 34675,
    SPELL_HUNTER_PET_SCALING_01     = 34902,
    SPELL_HUNTER_PET_SCALING_02     = 34903,
    SPELL_HUNTER_PET_SCALING_03     = 34904,
    SPELL_HUNTER_PET_SCALING_04     = 61017,
    SPELL_HUNTER_PET_SCALING_05     = 89446,
    SPELL_PET_PASSIVE_CRIT          = 35695,
    SPELL_PET_PASSIVE_DAMAGE_TAKEN  = 35697,
    SPELL_WARLOCK_PET_SCALING_01    = 34947,
    SPELL_WARLOCK_PET_SCALING_02    = 34956,
    SPELL_WARLOCK_PET_SCALING_03    = 34957,
    SPELL_WARLOCK_PET_SCALING_04    = 34958,
    SPELL_WARLOCK_PET_SCALING_05    = 61013,
};

enum HunterPetTreeSpecializationMarker
{
    SPELL_HUNTER_PET_FEROCITY_MARKER = 87887,
    SPELL_HUNTER_PET_TENACITY_MARKER = 87891,
    SPELL_HUNTER_PET_CUNNING_MARKER  = 87884
};

enum WarlockPetCalculate
{
    ENTRY_FELGUARD      = 17252,
    ENTRY_VOIDWALKER    = 1860,
    ENTRY_FELHUNTER     = 417,
    ENTRY_SUCCUBUS      = 1863,
    ENTRY_IMP           = 416
};

// 54044 - Pet Carrion Feeder
class spell_hun_pet_carrion_feeder : public SpellScriptLoader
{
public:
    spell_hun_pet_carrion_feeder() : SpellScriptLoader("spell_hun_pet_carrion_feeder") { }

    class spell_hun_pet_carrion_feeder_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_pet_carrion_feeder_SpellScript);

        bool Load()
        {
            if (!GetCaster()->isPet())
                return false;
            return true;
        }

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_HUNTER_PET_CARRION_FEEDER_TRIGGERED))
                return false;
            return true;
        }

        SpellCastResult CheckIfCorpseNear()
        {
            Unit* caster = GetCaster();
            float max_range = GetSpellInfo()->GetMaxRange(false);
            WorldObject* result = NULL;
            // search for nearby enemy corpse in range
            Trinity::AnyDeadUnitSpellTargetInRangeCheck check(caster, max_range, GetSpellInfo(), TARGET_CHECK_ENEMY);
            Trinity::WorldObjectSearcher<Trinity::AnyDeadUnitSpellTargetInRangeCheck> searcher(caster, result, check);
            caster->GetMap()->VisitFirstFound(caster->m_positionX, caster->m_positionY, max_range, searcher);
            if (!result)
                return SPELL_FAILED_NO_EDIBLE_CORPSES;
            return SPELL_CAST_OK;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            caster->CastSpell(caster, SPELL_HUNTER_PET_CARRION_FEEDER_TRIGGERED, false);
        }

        void Register()
        {
            OnEffectHit += SpellEffectFn(spell_hun_pet_carrion_feeder_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            OnCheckCast += SpellCheckCastFn(spell_hun_pet_carrion_feeder_SpellScript::CheckIfCorpseNear);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hun_pet_carrion_feeder_SpellScript();
    }
};

// 55709 - Pet Heart of the Phoenix
class spell_hun_pet_heart_of_the_phoenix : public SpellScriptLoader
{
public:
    spell_hun_pet_heart_of_the_phoenix() : SpellScriptLoader("spell_hun_pet_heart_of_the_phoenix") { }

    class spell_hun_pet_heart_of_the_phoenix_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_pet_heart_of_the_phoenix_SpellScript);

        bool Load()
        {
            if (!GetCaster()->isPet())
                return false;
            return true;
        }

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_HUNTER_PET_HEART_OF_THE_PHOENIX_TRIGGERED) || !sSpellMgr->GetSpellInfo(SPELL_HUNTER_PET_HEART_OF_THE_PHOENIX_DEBUFF))
                return false;
            return true;
        }

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            if (Unit* owner = caster->GetOwner())
                if (!caster->HasAura(SPELL_HUNTER_PET_HEART_OF_THE_PHOENIX_DEBUFF))
                {
                    owner->CastCustomSpell(SPELL_HUNTER_PET_HEART_OF_THE_PHOENIX_TRIGGERED, SPELLVALUE_BASE_POINT0, 100, caster, true);
                    caster->CastSpell(caster, SPELL_HUNTER_PET_HEART_OF_THE_PHOENIX_DEBUFF, true);
                }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_hun_pet_heart_of_the_phoenix_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hun_pet_heart_of_the_phoenix_SpellScript();
    }
};

class spell_hun_pet_scaling_01 : public SpellScriptLoader
{
public:
    spell_hun_pet_scaling_01() : SpellScriptLoader("spell_hun_pet_scaling_01") { }

    class spell_hun_pet_scaling_01_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_pet_scaling_01_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            _tempHealth = 0;
            return true;
        }

        void CalculateMaxHealthAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                if (pet->isPet())
                {
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        float mod = 0.0f;
                        if (pet->HasAura(SPELL_HUNTER_PET_FEROCITY_MARKER))
                            mod = 0.67f;
                        else if (pet->HasAura(SPELL_HUNTER_PET_TENACITY_MARKER))
                            mod = 0.78f;
                        else if (pet->HasAura(SPELL_HUNTER_PET_CUNNING_MARKER))
                            mod = 0.725f;

                        float ownerBonus = owner->GetStat(STAT_STAMINA) * mod * 14.0f;
                        amount += ownerBonus;
                    }
                }
            }
        }

        void ApplyEffect(AuraEffect const* /* aurEff */, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                if (_tempHealth)
                    pet->SetHealth(_tempHealth);
                else
                    pet->SetHealth(pet->GetMaxHealth());
            }
        }

        void RemoveEffect(AuraEffect const* /* aurEff */, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* pet = GetUnitOwner())
                _tempHealth = pet->GetHealth();
        }

        void CalculateAttackPowerAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                if (!pet->isPet())
                    return;

                Unit* owner = pet->ToPet()->GetOwner();
                if (!owner)
                    return;

                float mod = 0.425f;                                                 //Hunter contribution modifier
                float bonusAP = owner->GetTotalAttackPowerValue(RANGED_ATTACK) * mod;
                amount += bonusAP;
            }
        }

        void CalculateSpellDamageDoneAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                if (!pet->isPet())
                    return;

                Unit* owner = pet->ToPet()->GetOwner();
                if (!owner)
                    return;

                float mod = 1.0f;                                                 //Hunter contribution modifier
                float bonusDamage = owner->GetTotalAttackPowerValue(RANGED_ATTACK) * 0.1287f * mod;
                amount += bonusDamage;
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_hun_pet_scaling_01_AuraScript::RemoveEffect, EFFECT_0, SPELL_AURA_MOD_MAX_HEALTH, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            AfterEffectApply += AuraEffectApplyFn(spell_hun_pet_scaling_01_AuraScript::ApplyEffect, EFFECT_0, SPELL_AURA_MOD_MAX_HEALTH, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_scaling_01_AuraScript::CalculateMaxHealthAmount, EFFECT_0, SPELL_AURA_MOD_MAX_HEALTH);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_scaling_01_AuraScript::CalculateAttackPowerAmount, EFFECT_1, SPELL_AURA_MOD_ATTACK_POWER);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_scaling_01_AuraScript::CalculateSpellDamageDoneAmount, EFFECT_2, SPELL_AURA_MOD_DAMAGE_DONE);
        }

    private:
        uint32 _tempHealth;
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hun_pet_scaling_01_AuraScript();
    }
};

class spell_hun_pet_scaling_02 : public SpellScriptLoader
{
public:
    spell_hun_pet_scaling_02() : SpellScriptLoader("spell_hun_pet_scaling_02") { }

    class spell_hun_pet_scaling_02_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_pet_scaling_02_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateFrostResistanceAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                if (!pet->isPet())
                    return;

                Unit* owner = pet->ToPet()->GetOwner();
                if (!owner)
                    return;

                float ownerBonus = 0.0f;

                ownerBonus = CalculatePct(owner->GetResistance(SPELL_SCHOOL_FROST), 40);

                amount += ownerBonus;
            }
        }

        void CalculateFireResistanceAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                if (!pet->isPet())
                    return;

                Unit* owner = pet->ToPet()->GetOwner();
                if (!owner)
                    return;

                float ownerBonus = 0.0f;

                ownerBonus = CalculatePct(owner->GetResistance(SPELL_SCHOOL_FIRE), 40);

                amount += ownerBonus;
            }
        }

        void CalculateNatureResistanceAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                if (!pet->isPet())
                    return;

                Unit* owner = pet->ToPet()->GetOwner();
                if (!owner)
                    return;

                float ownerBonus = 0.0f;

                ownerBonus = CalculatePct(owner->GetResistance(SPELL_SCHOOL_NATURE), 40);

                amount += ownerBonus;
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_scaling_02_AuraScript::CalculateFireResistanceAmount, EFFECT_0, SPELL_AURA_MOD_RESISTANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_scaling_02_AuraScript::CalculateFrostResistanceAmount, EFFECT_1, SPELL_AURA_MOD_RESISTANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_scaling_02_AuraScript::CalculateNatureResistanceAmount, EFFECT_2, SPELL_AURA_MOD_RESISTANCE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hun_pet_scaling_02_AuraScript();
    }
};

class spell_hun_pet_scaling_03 : public SpellScriptLoader
{
public:
    spell_hun_pet_scaling_03() : SpellScriptLoader("spell_hun_pet_scaling_03") { }

    class spell_hun_pet_scaling_03_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_pet_scaling_03_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateShadowResistanceAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                if (!pet->isPet())
                    return;

                Unit* owner = pet->ToPet()->GetOwner();
                if (!owner)
                    return;

                float ownerBonus = 0.0f;

                ownerBonus = CalculatePct(owner->GetResistance(SPELL_SCHOOL_SHADOW), 40);

                amount += ownerBonus;
            }
        }

        void CalculateArcaneResistanceAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                if (!pet->isPet())
                    return;

                Unit* owner = pet->ToPet()->GetOwner();
                if (!owner)
                    return;

                float ownerBonus = 0.0f;

                ownerBonus = CalculatePct(owner->GetResistance(SPELL_SCHOOL_ARCANE), 40);

                amount += ownerBonus;
            }
        }

        void CalculateArmorAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                if (!pet->isPet())
                    return;

                Unit* owner = pet->ToPet()->GetOwner();
                if (!owner)
                    return;

                float ownerBonus = 0.0f;
                float pctMod = 0.0f;

                if (pet->HasAura(SPELL_HUNTER_PET_FEROCITY_MARKER))
                    pctMod = 50.0f;
                else if (pet->HasAura(SPELL_HUNTER_PET_TENACITY_MARKER))
                    pctMod = 60.0f;
                else if (pet->HasAura(SPELL_HUNTER_PET_CUNNING_MARKER))
                    pctMod = 70.0f;

                ownerBonus = CalculatePct(owner->GetArmor(), pctMod);

                amount += ownerBonus;
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_scaling_03_AuraScript::CalculateShadowResistanceAmount, EFFECT_0, SPELL_AURA_MOD_RESISTANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_scaling_03_AuraScript::CalculateArcaneResistanceAmount, EFFECT_1, SPELL_AURA_MOD_RESISTANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_scaling_03_AuraScript::CalculateArmorAmount, EFFECT_2, SPELL_AURA_MOD_RESISTANCE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hun_pet_scaling_03_AuraScript();
    }
};

class spell_hun_pet_scaling_04 : public SpellScriptLoader
{
public:
    spell_hun_pet_scaling_04() : SpellScriptLoader("spell_hun_pet_scaling_04") { }

    class spell_hun_pet_scaling_04_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_pet_scaling_04_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountMeleeHit(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float HitMelee = 0.0f;
                // Increase hit from SPELL_AURA_MOD_HIT_CHANCE
                HitMelee += owner->GetTotalAuraModifier(SPELL_AURA_MOD_HIT_CHANCE);
                // Increase hit melee from meele hit ratings
                HitMelee += std::max(owner->GetRatingBonusValue(CR_HIT_MELEE), owner->GetRatingBonusValue(CR_HIT_RANGED));

                amount += int32(HitMelee);
            }
        }

        void CalculateAmountSpellHit(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float HitSpell = 0.0f;
                // Increase hit from SPELL_AURA_MOD_SPELL_HIT_CHANCE
                HitSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
                // Increase hit spell from spell hit ratings
                HitSpell += owner->GetRatingBonusValue(CR_HIT_SPELL);

                amount += int32(HitSpell);
            }
        }

        void CalculateAmountExpertise(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float Expertise = 0.0f;
                // Increase hit from SPELL_AURA_MOD_EXPERTISE
                Expertise += owner->GetTotalAuraModifier(SPELL_AURA_MOD_EXPERTISE);
                // Increase Expertise from Expertise ratings
                Expertise += owner->GetRatingBonusValue(CR_EXPERTISE);

                amount += int32(Expertise);
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_scaling_04_AuraScript::CalculateAmountMeleeHit, EFFECT_0, SPELL_AURA_MOD_HIT_CHANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_scaling_04_AuraScript::CalculateAmountSpellHit, EFFECT_1, SPELL_AURA_MOD_SPELL_HIT_CHANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_scaling_04_AuraScript::CalculateAmountExpertise, EFFECT_2, SPELL_AURA_MOD_EXPERTISE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hun_pet_scaling_04_AuraScript();
    }
};

class spell_hun_pet_scaling_05 : public SpellScriptLoader
{
public:
    spell_hun_pet_scaling_05() : SpellScriptLoader("spell_hun_pet_scaling_05") { }

    class spell_hun_pet_scaling_05_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_pet_scaling_05_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountCritPct(AuraEffect const* /*aurEff*/, int32& /*amount*/, bool& /*canBeRecalculated*/)
        {
        }

        void CalculateAmountHaste(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                float meleeHaste = (1.0f - owner->m_modAttackSpeedPct[BASE_ATTACK]) * 100.0f;
                amount += int32(meleeHaste);
            }
        }

        void CalculateAmountResistance(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
                amount += owner->GetInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE);
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_scaling_05_AuraScript::CalculateAmountCritPct, EFFECT_0, SPELL_AURA_MOD_CRIT_PCT);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_scaling_05_AuraScript::CalculateAmountHaste, EFFECT_1, SPELL_AURA_MELEE_SLOW);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_scaling_05_AuraScript::CalculateAmountResistance, EFFECT_2, SPELL_AURA_MOD_TARGET_RESISTANCE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hun_pet_scaling_05_AuraScript();
    }
};

class spell_hun_pet_passive_crit : public SpellScriptLoader
{
public:
    spell_hun_pet_passive_crit() : SpellScriptLoader("spell_hun_pet_passive_crit") { }

    class spell_hun_pet_passive_crit_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_pet_passive_crit_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountCritSpell(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player *player = GetCaster()->GetOwner()->ToPlayer())
                amount += player->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1);
        }

        void CalculateAmountCritMelee(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player *player = GetCaster()->GetOwner()->ToPlayer())
                amount += player->GetFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE);
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_passive_crit_AuraScript::CalculateAmountCritMelee, EFFECT_0, SPELL_AURA_MOD_WEAPON_CRIT_PERCENT);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_passive_crit_AuraScript::CalculateAmountCritSpell, EFFECT_1, SPELL_AURA_MOD_SPELL_CRIT_CHANCE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hun_pet_passive_crit_AuraScript();
    }
};

class spell_hun_pet_passive_damage_done : public SpellScriptLoader
{
public:
    spell_hun_pet_passive_damage_done() : SpellScriptLoader("spell_hun_pet_passive_damage_done") { }

    class spell_hun_pet_passive_damage_done_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_pet_passive_damage_done_AuraScript);

        bool Load() override
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        //void CalculateAmountDamageDone(AuraEffect const* /*aurEff*/, int32& /*amount*/, bool& /*canBeRecalculated*/)
        //{
        //    if (!GetCaster() || !GetCaster()->GetOwner())
        //        return;
        //}

        void Register() override
        {
            //DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_passive_damage_done_AuraScript::CalculateAmountDamageDone, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_hun_pet_passive_damage_done_AuraScript();
    }
};

class spell_hun_pet_passive_tree_marker : public SpellScriptLoader
{
public:
    spell_hun_pet_passive_tree_marker() : SpellScriptLoader("spell_hun_pet_passive_tree_marker") { }

    class spell_hun_pet_passive_tree_marker_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_pet_passive_tree_marker_AuraScript);

        void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (caster->HasUnitTypeMask(UNIT_MASK_GUARDIAN))
                {
                    ((Guardian*)caster)->RecalculatePetScalingStats(STAT_STAMINA);
                    ((Guardian*)caster)->RecalculatePetScalingResistance(SPELL_SCHOOL_NORMAL);
                }
            }
        }

        void Register()
        {
            AfterEffectApply += AuraEffectApplyFn(spell_hun_pet_passive_tree_marker_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hun_pet_passive_tree_marker_AuraScript();
    }
};


// Mend pet
class spell_hun_mend_pet : public SpellScriptLoader
{
public:
    spell_hun_mend_pet() : SpellScriptLoader("spell_hun_mend_pet") { }

    class spell_hun_mend_pet_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_mend_pet_AuraScript);

        void HandlePeriodic(AuraEffect const* /*aurEff*/)
        {
            if (Unit* caster = GetCaster())
                if (AuraEffect* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_HUNTER, 267, EFFECT_0))
                    if (roll_chance_i(aurEff->GetAmount()))
                        caster->CastSpell(GetTarget(), 24406, true);
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_hun_mend_pet_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_OBS_MOD_HEALTH);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_hun_mend_pet_AuraScript();
    }
};

enum damageSpellsTalents
{
    SPELL_WILD_HUNT_RANK_1  = 62758,
    SPELL_WILD_HUNT_RANK_2  = 62762
};

class spell_hun_pet_damage_spells : public SpellScriptLoader
{
public:
    spell_hun_pet_damage_spells() : SpellScriptLoader("spell_hun_pet_damage_spells") { }

    class spell_hun_pet_damage_spells_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_pet_damage_spells_SpellScript);

        void HandleHit(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            if (Unit* owner = caster->GetOwner())
            {
                int32 baseDamage = 0;
                switch (GetSpellInfo()->Id)
                {
                    case 16827: // Claw
                    case 49966: // Smack
                    case 17253: // Bite
                        baseDamage = GetEffectValue() + (0.08f * caster->GetTotalAttackPowerValue(BASE_ATTACK));
                        if (caster->GetPower(POWER_FOCUS) >= 50)
                        {
                            // Wild Hunt
                            if (caster->HasSpell(SPELL_WILD_HUNT_RANK_1))
                            {
                                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_WILD_HUNT_RANK_1);
                                AddPct(baseDamage, spellInfo->Effects[EFFECT_0].BasePoints);
                            }
                            else if (caster->HasSpell(SPELL_WILD_HUNT_RANK_2))
                            {
                                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_WILD_HUNT_RANK_2);
                                AddPct(baseDamage, spellInfo->Effects[EFFECT_0].BasePoints);
                            }
                        }
                        break;
                    case 83381: // Kill command
                        baseDamage = GetEffectValue() + (0.516f * owner->GetTotalAttackPowerValue(RANGED_ATTACK));
                        break;
                    case 53508: // wolverine bite
                        baseDamage = GetEffectValue() + (0.04f * caster->GetTotalAttackPowerValue(BASE_ATTACK));
                        break;
                }

                SetEffectDamage(baseDamage);
            }
        }

        void HandleKillCommand(SpellEffIndex /*effIndex*/)
        {
            if (Unit* owner = GetCaster()->GetOwner())
            {
                owner->RemoveAura(94006);
                owner->RemoveAura(94007);
            }
        }

        void Register()
        {
            OnEffectLaunchTarget += SpellEffectFn(spell_hun_pet_damage_spells_SpellScript::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            
            if (m_scriptSpellId == 83381)
                OnEffectHitTarget += SpellEffectFn(spell_hun_pet_damage_spells_SpellScript::HandleKillCommand, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hun_pet_damage_spells_SpellScript();
    }
};

class spell_hun_pet_spirit_mend : public SpellScriptLoader
{
public:
    spell_hun_pet_spirit_mend() : SpellScriptLoader("spell_hun_pet_spirit_mend") { }

    class spell_hun_pet_spirit_mend_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_pet_spirit_mend_SpellScript);

        void HandleHeal(SpellEffIndex /*effIndex*/)
        {
            if (!GetCaster())
                return;

            if (Unit* owner = GetCaster()->GetOwner())
            {
                float coeff = 0.35f * 0.5f;
                SetEffectDamage(GetEffectValue() + (coeff * owner->GetTotalAttackPowerValue(RANGED_ATTACK)));
            }
        }

        void Register()
        {
            OnEffectLaunchTarget += SpellEffectFn(spell_hun_pet_spirit_mend_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hun_pet_spirit_mend_SpellScript();
    }

    class spell_hun_pet_spirit_mend_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_pet_spirit_mend_AuraScript);

        void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
        {
            Unit* caster = GetCaster();
            if (!caster)
                return;

            if (Unit* owner = caster->GetOwner())
            {
                float coeff = 0.35f * 0.335f;
                amount += (coeff * owner->GetTotalAttackPowerValue(RANGED_ATTACK));
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_pet_spirit_mend_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_PERIODIC_HEAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hun_pet_spirit_mend_AuraScript();
    }
};

// 53478 - Last Stand Pet
class spell_hun_last_stand_pet : public SpellScriptLoader
{
public:
    spell_hun_last_stand_pet() : SpellScriptLoader("spell_hun_last_stand_pet") { }

    class spell_hun_last_stand_pet_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_last_stand_pet_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_HUNTER_PET_LAST_STAND_TRIGGERED))
                return false;
            return true;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            int32 healthModSpellBasePoints0 = int32(caster->CountPctFromMaxHealth(30));
            caster->CastCustomSpell(caster, SPELL_HUNTER_PET_LAST_STAND_TRIGGERED, &healthModSpellBasePoints0, NULL, NULL, true, NULL);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_hun_last_stand_pet_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hun_last_stand_pet_SpellScript();
    }
};

enum cowerSpells
{
    SPELL_HUNTER_TALENT_IMPROVED_COWER_RANK_1 = 53180,
    SPELL_HUNTER_TALENT_IMPROVED_COWER_RANK_2 = 53181
};

class spell_hun_cower : public SpellScriptLoader
{
public:
    spell_hun_cower() : SpellScriptLoader("spell_hun_cower") {}

    class spell_hun_cower_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_cower_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (AuraEffect* aurEff = GetEffect(EFFECT_1))
            {
                if (GetTarget()->HasAura(SPELL_HUNTER_TALENT_IMPROVED_COWER_RANK_1))
                    aurEff->SetAmount(-25);
                else if (GetTarget()->HasAura(SPELL_HUNTER_TALENT_IMPROVED_COWER_RANK_2))
                    aurEff->SetAmount(0);
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_hun_cower_AuraScript::OnApply, EFFECT_1, SPELL_AURA_MOD_DECREASE_SPEED, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hun_cower_AuraScript();
    }
};

class spell_hun_tamed_pet_passive_07 : public SpellScriptLoader
{
public:
    spell_hun_tamed_pet_passive_07() : SpellScriptLoader("spell_hun_tamed_pet_passive_07") { }

    class spell_hun_tamed_pet_passive_07_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_tamed_pet_passive_07_AuraScript);

        bool CheckProc(ProcEventInfo& /*eventInfo*/)
        {
            return GetEffect(EFFECT_0)->GetAmount() != 0;
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(spell_hun_tamed_pet_passive_07_AuraScript::CheckProc);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hun_tamed_pet_passive_07_AuraScript();
    }
};

// Claw
class spell_dk_pet_spells : public SpellScriptLoader
{
public:
    spell_dk_pet_spells() : SpellScriptLoader("spell_dk_pet_spells") { }

    class spell_dk_pet_spells_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_pet_spells_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            bool darkTransformation = GetCaster()->HasAura(63560);
            int32 triggered_spell_id = 0;
            switch (GetSpellInfo()->Id)
            {
            case 47481: // Gnaw
                triggered_spell_id = darkTransformation ? 91797 : 91800;
                break;
            case 47468: // Claw
                triggered_spell_id = darkTransformation ? 91778 : 91776;
                break;
            case 47482: // Leap
                triggered_spell_id = darkTransformation ? 91802 : 91809;
                break;
            case 47484: // Huddle
                triggered_spell_id = darkTransformation ? 91837 : 91838;
                break;
            }
            if (triggered_spell_id)
                GetCaster()->CastSpell(GetHitUnit(), triggered_spell_id, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_dk_pet_spells_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_pet_spells_SpellScript();
    }
};


class spell_dk_pet_scaling_01 : public SpellScriptLoader
{
public:
    spell_dk_pet_scaling_01() : SpellScriptLoader("spell_dk_pet_scaling_01") { }

    class spell_dk_pet_scaling_01_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_pet_scaling_01_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            _tempHealth = 0;
            return true;
        }

        void CalculateStaminaAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                if (Unit* owner = pet->GetOwner())
                {
                    float mod = 0.8f;
                    // Glyph of Raise dead
                    if (AuraEffect const* aurEff = owner->GetAuraEffect(58686, 0))
                        AddPct(mod, aurEff->GetAmount());

                    float ownerBonus = float(owner->GetStat(STAT_STAMINA)) * mod;
                    amount += ownerBonus;
                }
            }
        }

        void ApplyEffect(AuraEffect const* /* aurEff */, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (_tempHealth)
                    pet->SetHealth(_tempHealth);
        }

        void RemoveEffect(AuraEffect const* /* aurEff */, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* pet = GetUnitOwner())
                _tempHealth = pet->GetHealth();
        }

        void CalculateStrengthAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                Unit* owner = pet->GetOwner();
                if (!owner)
                    return;

                float mod = 1.0f;
                // Glyph of raise dead
                if (AuraEffect const* aurEff = owner->GetAuraEffect(58686, 0))
                    AddPct(mod, aurEff->GetAmount());

                float ownerBonus = float(owner->GetStat(STAT_STRENGTH)) * mod;
                amount += ownerBonus;
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_dk_pet_scaling_01_AuraScript::RemoveEffect, EFFECT_0, SPELL_AURA_MOD_STAT, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            AfterEffectApply += AuraEffectApplyFn(spell_dk_pet_scaling_01_AuraScript::ApplyEffect, EFFECT_0, SPELL_AURA_MOD_STAT, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_pet_scaling_01_AuraScript::CalculateStaminaAmount, EFFECT_0, SPELL_AURA_MOD_STAT);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_pet_scaling_01_AuraScript::CalculateStrengthAmount, EFFECT_1, SPELL_AURA_MOD_STAT);
        }

    private:
        uint32 _tempHealth;
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_pet_scaling_01_AuraScript();
    }
};

class spell_dk_pet_scaling_02 : public SpellScriptLoader
{
public:
    spell_dk_pet_scaling_02() : SpellScriptLoader("spell_dk_pet_scaling_02") { }

    class spell_dk_pet_scaling_02_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_pet_scaling_02_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountMeleeHaste(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float HasteMelee = 0.0f;
                HasteMelee += (1 - owner->m_modAttackSpeedPct[BASE_ATTACK]) * 100;
                amount += int32(HasteMelee);
            }
        }

        void CalculateBonusDamagePct(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;

            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                Unit::AuraEffectList const& auraDamagePctList = owner->GetAuraEffectsByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
                for (Unit::AuraEffectList::const_iterator itr = auraDamagePctList.begin(); itr != auraDamagePctList.end(); ++itr)
                    if ((*itr)->GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL)
                        amount += (*itr)->GetAmount();
            }
        }


        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_pet_scaling_02_AuraScript::CalculateBonusDamagePct, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_pet_scaling_02_AuraScript::CalculateAmountMeleeHaste, EFFECT_1, SPELL_AURA_MELEE_SLOW);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_pet_scaling_02_AuraScript();
    }
};

class spell_dk_pet_scaling_03 : public SpellScriptLoader
{
public:
    spell_dk_pet_scaling_03() : SpellScriptLoader("spell_dk_pet_scaling_03") { }

    class spell_dk_pet_scaling_03_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_pet_scaling_03_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountMeleeHit(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float HitMelee = 0.0f;
                // Increase hit from SPELL_AURA_MOD_HIT_CHANCE
                HitMelee += owner->GetTotalAuraModifier(SPELL_AURA_MOD_HIT_CHANCE);
                // Increase hit melee from meele hit ratings
                HitMelee += owner->GetRatingBonusValue(CR_HIT_MELEE);

                amount += int32(HitMelee);
            }
        }

        void CalculateAmountSpellHit(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float HitSpell = 0.0f;
                // Increase hit from SPELL_AURA_MOD_SPELL_HIT_CHANCE
                HitSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
                // Increase hit spell from spell hit ratings
                HitSpell += owner->GetRatingBonusValue(CR_HIT_SPELL);

                amount += int32(HitSpell);
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_pet_scaling_03_AuraScript::CalculateAmountMeleeHit, EFFECT_0, SPELL_AURA_MOD_HIT_CHANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_pet_scaling_03_AuraScript::CalculateAmountSpellHit, EFFECT_1, SPELL_AURA_MOD_SPELL_HIT_CHANCE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_pet_scaling_03_AuraScript();
    }
};

class spell_dk_pet_scaling_05 : public SpellScriptLoader
{
public:
    spell_dk_pet_scaling_05() : SpellScriptLoader("spell_dk_pet_scaling_05") { }

    class spell_dk_pet_scaling_05_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_pet_scaling_05_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountCritPct(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                float CritSpell = owner->GetMeleeCritFromAgility();
                CritSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_CHANCE);
                CritSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_PCT);
                CritSpell += owner->GetRatingBonusValue(CR_CRIT_MELEE);
                amount += int32(CritSpell);
            }
        }

        void CalculateAmountResistance(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
                amount += owner->GetInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE);
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_pet_scaling_05_AuraScript::CalculateAmountCritPct, EFFECT_0, SPELL_AURA_MOD_CRIT_PCT);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_pet_scaling_05_AuraScript::CalculateAmountResistance, EFFECT_2, SPELL_AURA_MOD_TARGET_RESISTANCE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_pet_scaling_05_AuraScript();
    }
};

class spell_dk_rune_weapon_scaling_02 : public SpellScriptLoader
{
public:
    spell_dk_rune_weapon_scaling_02() : SpellScriptLoader("spell_dk_rune_weapon_scaling_02") { }

    class spell_dk_rune_weapon_scaling_02_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_rune_weapon_scaling_02_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateDamageDoneAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                Unit* owner = pet->GetOwner();
                if (!owner)
                    return;

                if (pet->isGuardian())
                    ((Guardian*)pet)->SetSpellBonusDamage(owner->GetTotalAttackPowerValue(BASE_ATTACK));

                amount += owner->CalculateDamage(BASE_ATTACK, true, true);
            }
        }

        void CalculateAmountMeleeHaste(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float HasteMelee = 0.0f;
                // Increase hit from SPELL_AURA_MOD_HIT_CHANCE
                HasteMelee += (1 - owner->m_modAttackSpeedPct[BASE_ATTACK]) * 100;

                amount += int32(HasteMelee);
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_rune_weapon_scaling_02_AuraScript::CalculateDamageDoneAmount, EFFECT_0, SPELL_AURA_MOD_DAMAGE_DONE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_rune_weapon_scaling_02_AuraScript::CalculateAmountMeleeHaste, EFFECT_1, SPELL_AURA_MELEE_SLOW);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_rune_weapon_scaling_02_AuraScript();
    }
};

class spell_dk_ghoul_leap : public SpellScriptLoader
{
public:
    spell_dk_ghoul_leap() : SpellScriptLoader("spell_dk_ghoul_leap") { }

    class spell_dk_ghoul_leap_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_ghoul_leap_SpellScript)

            SpellCastResult CheckState()
        {
                if (Unit* caster = GetCaster())
                    if (caster->HasUnitState(UNIT_STATE_ROOT)) // Cant jump while rooted
                        return SPELL_FAILED_ROOTED;

                return SPELL_CAST_OK;
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_dk_ghoul_leap_SpellScript::CheckState);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_dk_ghoul_leap_SpellScript();
    }
};

class spell_mage_pet_scaling_05 : public SpellScriptLoader
{
public:
    spell_mage_pet_scaling_05() : SpellScriptLoader("spell_mage_pet_scaling_05") { }

    class spell_mage_pet_scaling_05_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mage_pet_scaling_05_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->ToTempSummon() || !GetCaster()->ToTempSummon()->GetSummoner() || GetCaster()->ToTempSummon()->GetSummoner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountCritPct(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (GetCaster() && GetCaster()->ToTempSummon() && GetCaster()->ToTempSummon()->GetSummoner())
                if (Player* owner = GetCaster()->ToTempSummon()->GetSummoner()->ToPlayer())
                {
                    float CritSpell = owner->GetSpellCritFromIntellect();
                    CritSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_CRIT_CHANCE);
                    CritSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_PCT);
                    CritSpell += owner->GetRatingBonusValue(CR_CRIT_SPELL);
                    amount += int32(CritSpell);
                }
        }

        void CalculateAmountHaste(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || !GetCaster()->ToTempSummon() || !GetCaster()->ToTempSummon()->GetSummoner())
                return;
            if (Player* owner = GetCaster()->ToTempSummon()->GetSummoner()->ToPlayer())
            {
                float meleeHaste = (1.0f - owner->m_modAttackSpeedPct[BASE_ATTACK]) * 100.0f;
                amount += int32(meleeHaste);
            }
        }

        void CalculateAmountResistance(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
                amount += owner->GetInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE);
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_pet_scaling_05_AuraScript::CalculateAmountCritPct, EFFECT_0, SPELL_AURA_MOD_CRIT_PCT);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_pet_scaling_05_AuraScript::CalculateAmountHaste, EFFECT_1, SPELL_AURA_MELEE_SLOW);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_pet_scaling_05_AuraScript::CalculateAmountResistance, EFFECT_2, SPELL_AURA_MOD_TARGET_RESISTANCE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_mage_pet_scaling_05_AuraScript();
    }
};

// Flame orb target filter
class spell_mage_orb_filter : public SpellScriptLoader
{
public:
    spell_mage_orb_filter() : SpellScriptLoader("spell_mage_orb_filter") { }

    class spell_mage_orb_filter_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mage_orb_filter_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            Position casterPos;
            GetCaster()->GetPosition(&casterPos);
            WorldObject* closestTarget = NULL;
            float closestDistance = 100.0f;
            for (std::list<WorldObject*>::const_iterator itr = targets.begin(); itr != targets.end(); itr++)
            {
                Position comparePos;
                (*itr)->GetPosition(&comparePos);
                float exactDist = casterPos.GetExactDist(&comparePos);
                if (exactDist < closestDistance 
                    && !((*itr)->ToUnit() && (*itr)->ToUnit()->HasBreakableByDamageCrowdControlAura()) 
                    && !((*itr)->ToUnit() && (*itr)->ToUnit()->GetCreatureType() == CREATURE_TYPE_CRITTER))
                {
                    closestDistance = exactDist;
                    closestTarget = (*itr);
                }
            }
            targets.clear();
            if (closestTarget)
                targets.push_back(closestTarget);
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            if (Unit* target = GetHitUnit())
            {
                // Flame Orb
                if (caster->GetEntry() == 44214)
                    caster->CastSpell(target, 82739, true, NULL, NULL, caster->GetOwnerGUID());
                else
                {
                    // Cast 40% slow if second rank of talent is learned
                    if (caster->GetOwner()->HasAura(84727))
                        caster->CastSpell(target, 84721, true, NULL, NULL, caster->GetOwnerGUID());
                    else
                        caster->CastSpell(target, 95969, true, NULL, NULL, caster->GetOwnerGUID());
                }
                caster->AddSpellCooldown(GetSpellInfo()->Id, 0, time(NULL) + GetSpellInfo()->Effects[EFFECT_0].BasePoints / 1000);
                caster->CastSpell(target, 86719, true);
                if (!caster->HasAura(82736))
                    caster->CastSpell(caster, 82736);
                target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TAKE_DAMAGE2);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_orb_filter_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(spell_mage_orb_filter_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_mage_orb_filter_SpellScript();
    }
};

class spell_mage_pet_freeze : public SpellScriptLoader
{
public:
    spell_mage_pet_freeze() : SpellScriptLoader("spell_mage_pet_freeze") { }

    class spell_mage_pet_freeze_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mage_pet_freeze_SpellScript);

        SpellCastResult CheckIfCorpseNear()
        {
            if (GetCaster()->HasUnitState(UNIT_STATE_CONTROLLED))
                return SPELL_FAILED_DONT_REPORT;

            return SPELL_CAST_OK;
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_mage_pet_freeze_SpellScript::CheckIfCorpseNear);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_mage_pet_freeze_SpellScript();
    }
};

class spell_pri_pet_scaling_05 : public SpellScriptLoader
{
public:
    spell_pri_pet_scaling_05() : SpellScriptLoader("spell_priest_pet_scaling_05") { }

    class spell_pri_pet_scaling_05_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_pet_scaling_05_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->ToTempSummon() || !GetCaster()->ToTempSummon()->GetSummoner() || GetCaster()->ToTempSummon()->GetSummoner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountCritPct(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (GetCaster() && GetCaster()->ToTempSummon() && GetCaster()->ToTempSummon()->GetSummoner())
                if (Player* owner = GetCaster()->ToTempSummon()->GetSummoner()->ToPlayer())
                {
                    float CritSpell = owner->GetSpellCritFromIntellect();
                    CritSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_CRIT_CHANCE);
                    CritSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_PCT);
                    CritSpell += owner->GetRatingBonusValue(CR_CRIT_SPELL);
                    amount += int32(CritSpell);
                }
        }

        void CalculateAmountResistance(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
                amount += owner->GetInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE);
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_pet_scaling_05_AuraScript::CalculateAmountCritPct, EFFECT_0, SPELL_AURA_MOD_CRIT_PCT);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_pet_scaling_05_AuraScript::CalculateAmountResistance, EFFECT_1, SPELL_AURA_MOD_TARGET_RESISTANCE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_pri_pet_scaling_05_AuraScript();
    }
};

// 28305 - Mana Leech (Passive) (Priest Pet Aura)
class spell_pri_mana_leech : public SpellScriptLoader
{
public:
    spell_pri_mana_leech() : SpellScriptLoader("spell_pri_mana_leech") { }

    class spell_pri_mana_leech_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_mana_leech_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_MANA_LEECH_PROC))
                return false;
            return true;
        }

        bool Load()
        {
            _procTarget = NULL;
            return true;
        }

        bool CheckProc(ProcEventInfo& /*eventInfo*/)
        {
            _procTarget = GetTarget()->GetOwner();
            return _procTarget;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
        {
            PreventDefaultAction();
            GetTarget()->CastSpell(_procTarget, SPELL_PRIEST_MANA_LEECH_PROC, true, NULL, aurEff);

            // Priest T13 Shadow 4P Bonus (Shadowfiend and Shadowy Apparition)
            if (Unit* owner = GetTarget()->GetOwner())
                if (owner->HasAura(105844, owner->GetGUID()))
                    owner->CastCustomSpell(77487, SPELLVALUE_AURA_STACK, 3, owner, true);
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(spell_pri_mana_leech_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_pri_mana_leech_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }

    private:
        Unit* _procTarget;
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_pri_mana_leech_AuraScript();
    }
};

class spell_pri_shadowflame : public SpellScriptLoader
{
public:
    spell_pri_shadowflame() : SpellScriptLoader("spell_pri_shadowflame") { }

    class spell_pri_shadowflame_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_shadowflame_AuraScript);

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            if (Unit* target = eventInfo.GetActionTarget())
            {
                int32 damage = CalculatePct(eventInfo.GetDamageInfo()->GetDamage(), aurEff->GetAmount());
                GetTarget()->CastCustomSpell(target, SPELL_PRIEST_SHADOWFLAME, &damage, NULL, NULL, TRIGGERED_FULL_MASK);
            }
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(spell_pri_shadowflame_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_pri_shadowflame_AuraScript();
    }
};

class spell_sha_pet_scaling_04 : public SpellScriptLoader
{
public:
    spell_sha_pet_scaling_04() : SpellScriptLoader("spell_sha_pet_scaling_04") { }

    class spell_sha_pet_scaling_04_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sha_pet_scaling_04_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountMeleeHit(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float HitMelee = 0.0f;
                // Increase hit from SPELL_AURA_MOD_HIT_CHANCE
                HitMelee += owner->GetTotalAuraModifier(SPELL_AURA_MOD_HIT_CHANCE);
                // Increase hit melee from meele hit ratings
                HitMelee += owner->GetRatingBonusValue(CR_HIT_MELEE);

                amount += int32(HitMelee);
            }
        }

        void CalculateAmountSpellHit(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float HitSpell = 0.0f;
                // Increase hit from SPELL_AURA_MOD_SPELL_HIT_CHANCE
                HitSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
                // Increase hit spell from spell hit ratings
                HitSpell += owner->GetRatingBonusValue(CR_HIT_SPELL);

                amount += int32(HitSpell);
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_sha_pet_scaling_04_AuraScript::CalculateAmountMeleeHit, EFFECT_0, SPELL_AURA_MOD_HIT_CHANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_sha_pet_scaling_04_AuraScript::CalculateAmountSpellHit, EFFECT_1, SPELL_AURA_MOD_SPELL_HIT_CHANCE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_sha_pet_scaling_04_AuraScript();
    }
};

class spell_warl_pet_scaling_01 : public SpellScriptLoader
{
public:
    spell_warl_pet_scaling_01() : SpellScriptLoader("spell_warl_pet_scaling_01") { }

    class spell_warl_pet_scaling_01_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_pet_scaling_01_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void ApplyEffect(AuraEffect const* /* aurEff */, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* pet = GetUnitOwner())
                pet->SetHealth(pet->GetMaxHealth());
        }

        void RemoveEffect(AuraEffect const* /* aurEff */, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* pet = GetUnitOwner())
                _tempHealth = pet->GetHealth();
        }

        void CalculateMaxHealthAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                        amount += owner->ToPlayer()->GetHealthBonusFromStamina() * 0.75f;
        }

        void CalculateAttackPowerAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        int32 fire = int32(owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE))
                            - owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + SPELL_SCHOOL_FIRE);
                        int32 shadow = int32(owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW))
                            - owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + SPELL_SCHOOL_SHADOW);
                        int32 maximum = (fire > shadow) ? fire : shadow;
                        if (maximum < 0)
                            maximum = 0;
                        float bonusAP = maximum * 0.57f;
                        amount += bonusAP;
                        // Glyph of felguard
                        if (pet->GetEntry() == ENTRY_FELGUARD)
                        {
                            if (AuraEffect* ect = owner->GetAuraEffect(56246, EFFECT_0))
                            {
                                float base_attPower = pet->GetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE) * pet->GetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_PCT);
                                amount += CalculatePct(amount + base_attPower, ect->GetAmount());
                            }
                        }
                    }
        }

        void CalculateDamageDoneAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        //the damage bonus used for pets is either fire or shadow damage, whatever is higher
                        int32 fire = int32(owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE))
                            - owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + SPELL_SCHOOL_FIRE);
                        int32 shadow = int32(owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW))
                            - owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + SPELL_SCHOOL_SHADOW);
                        int32 maximum = (fire > shadow) ? fire : shadow;
                        float bonusDamage = 0.0f;
                        if (maximum > 0)
                            bonusDamage = maximum * 0.15f;
                        amount += bonusDamage;
                    }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_warl_pet_scaling_01_AuraScript::RemoveEffect, EFFECT_0, SPELL_AURA_MOD_MAX_HEALTH, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            AfterEffectApply += AuraEffectApplyFn(spell_warl_pet_scaling_01_AuraScript::ApplyEffect, EFFECT_0, SPELL_AURA_MOD_MAX_HEALTH, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_01_AuraScript::CalculateMaxHealthAmount, EFFECT_0, SPELL_AURA_MOD_MAX_HEALTH);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_01_AuraScript::CalculateAttackPowerAmount, EFFECT_1, SPELL_AURA_MOD_ATTACK_POWER);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_01_AuraScript::CalculateDamageDoneAmount, EFFECT_2, SPELL_AURA_MOD_DAMAGE_DONE);
        }

    private:
        uint32 _tempHealth;
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_pet_scaling_01_AuraScript();
    }
};

class spell_warl_pet_scaling_02 : public SpellScriptLoader
{
public:
    spell_warl_pet_scaling_02() : SpellScriptLoader("spell_warl_pet_scaling_02") { }

    class spell_warl_pet_scaling_02_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_pet_scaling_02_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            _tempMana = 0;
            return true;
        }

        void ApplyEffect(AuraEffect const* /* aurEff */, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (_tempMana)
                    pet->SetPower(POWER_MANA, _tempMana);
        }

        void RemoveEffect(AuraEffect const* /* aurEff */, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* pet = GetUnitOwner())
                _tempMana = pet->GetPower(POWER_MANA);
        }

        void CalculateEnergyAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        float ownerBonus = 0.0f;
                        float multiplicator = 15.0f;

                        ownerBonus = CalculatePct(owner->GetStat(STAT_INTELLECT), 30);
                        switch (pet->ToCreature()->GetEntry())
                        {
                        case ENTRY_IMP:
                            multiplicator = 4.95f;
                            break;
                        case ENTRY_VOIDWALKER:
                        case ENTRY_SUCCUBUS:
                        case ENTRY_FELHUNTER:
                        case ENTRY_FELGUARD:
                            multiplicator = 11.5f;
                            break;
                        default:
                            multiplicator = 15.0f;
                            break;
                        }
                        amount += int32(ownerBonus * multiplicator);
                    }
        }

        void CalculateArmorAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        float ownerBonus = 0.0f;
                        ownerBonus = CalculatePct(owner->GetArmor(), 35);
                        amount += ownerBonus;
                    }
        }

        void CalculateFireResistanceAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        float ownerBonus = 0.0f;
                        ownerBonus = CalculatePct(owner->GetResistance(SPELL_SCHOOL_FIRE), 40);
                        amount += ownerBonus;
                    }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_warl_pet_scaling_02_AuraScript::RemoveEffect, EFFECT_0, SPELL_AURA_MOD_INCREASE_ENERGY, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            AfterEffectApply += AuraEffectApplyFn(spell_warl_pet_scaling_02_AuraScript::ApplyEffect, EFFECT_0, SPELL_AURA_MOD_INCREASE_ENERGY, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_02_AuraScript::CalculateEnergyAmount, EFFECT_0, SPELL_AURA_MOD_INCREASE_ENERGY);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_02_AuraScript::CalculateArmorAmount, EFFECT_1, SPELL_AURA_MOD_RESISTANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_02_AuraScript::CalculateFireResistanceAmount, EFFECT_2, SPELL_AURA_MOD_RESISTANCE);
        }

    private:
        uint32 _tempMana;
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_pet_scaling_02_AuraScript();
    }
};


class spell_warl_pet_scaling_03 : public SpellScriptLoader
{
public:
    spell_warl_pet_scaling_03() : SpellScriptLoader("spell_warl_pet_scaling_03") { }

    class spell_warl_pet_scaling_03_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_pet_scaling_03_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateFrostResistanceAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        float ownerBonus = 0.0f;
                        ownerBonus = CalculatePct(owner->GetResistance(SPELL_SCHOOL_FROST), 40);
                        amount += ownerBonus;
                    }
        }

        void CalculateArcaneResistanceAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        float ownerBonus = 0.0f;
                        ownerBonus = CalculatePct(owner->GetResistance(SPELL_SCHOOL_ARCANE), 40);
                        amount += ownerBonus;
                    }
        }

        void CalculateNatureResistanceAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        float ownerBonus = 0.0f;
                        ownerBonus = CalculatePct(owner->GetResistance(SPELL_SCHOOL_NATURE), 40);
                        amount += ownerBonus;
                    }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_03_AuraScript::CalculateFrostResistanceAmount, EFFECT_0, SPELL_AURA_MOD_RESISTANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_03_AuraScript::CalculateArcaneResistanceAmount, EFFECT_1, SPELL_AURA_MOD_RESISTANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_03_AuraScript::CalculateNatureResistanceAmount, EFFECT_2, SPELL_AURA_MOD_RESISTANCE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_pet_scaling_03_AuraScript();
    }
};

class spell_warl_pet_scaling_04 : public SpellScriptLoader
{
public:
    spell_warl_pet_scaling_04() : SpellScriptLoader("spell_warl_pet_scaling_04") { }

    class spell_warl_pet_scaling_04_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_pet_scaling_04_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateShadowResistanceAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                    {
                        float ownerBonus = 0.0f;
                        ownerBonus = CalculatePct(owner->GetResistance(SPELL_SCHOOL_SHADOW), 40);
                        amount += ownerBonus;
                    }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_04_AuraScript::CalculateShadowResistanceAmount, EFFECT_0, SPELL_AURA_MOD_RESISTANCE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_pet_scaling_04_AuraScript();
    }
};

class spell_warl_pet_scaling_05 : public SpellScriptLoader
{
public:
    spell_warl_pet_scaling_05() : SpellScriptLoader("spell_warl_pet_scaling_05") { }

    class spell_warl_pet_scaling_05_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_pet_scaling_05_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountMeleeHit(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                float HitMelee = 0.0f;
                HitMelee += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
                HitMelee += owner->GetRatingBonusValue(CR_HIT_SPELL);
                amount += int32(HitMelee);
            }
        }

        void CalculateAmountSpellHit(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                float HitSpell = 0.0f;
                HitSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
                HitSpell += owner->GetRatingBonusValue(CR_HIT_SPELL);
                amount += int32(HitSpell);
            }
        }

        void CalculateAmountExpertise(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float Expertise = 0.0f;
                // Increase hit from SPELL_AURA_MOD_EXPERTISE
                Expertise += owner->GetTotalAuraModifier(SPELL_AURA_MOD_EXPERTISE);
                // Increase Expertise from Expertise ratings
                Expertise += owner->GetRatingBonusValue(CR_EXPERTISE);

                amount += int32(Expertise);
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_05_AuraScript::CalculateAmountMeleeHit, EFFECT_0, SPELL_AURA_MOD_HIT_CHANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_05_AuraScript::CalculateAmountSpellHit, EFFECT_1, SPELL_AURA_MOD_SPELL_HIT_CHANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_05_AuraScript::CalculateAmountExpertise, EFFECT_2, SPELL_AURA_MOD_EXPERTISE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_pet_scaling_05_AuraScript();
    }
};

class spell_warl_pet_scaling_06 : public SpellScriptLoader
{
public:
    spell_warl_pet_scaling_06() : SpellScriptLoader("spell_warl_pet_scaling_06") { }

    class spell_warl_pet_scaling_06_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_pet_scaling_06_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountCritPct(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                float CritSpell = owner->GetSpellCritFromIntellect();
                CritSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_CRIT_CHANCE);
                CritSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_PCT);
                CritSpell += owner->GetRatingBonusValue(CR_CRIT_SPELL);
                amount += int32(CritSpell);
            }
        }

        void CalculateAmountMeleeHaste(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                float HasteMelee = 0.0f;
                HasteMelee += (1 - owner->m_modAttackSpeedPct[BASE_ATTACK]) * 100;
                amount += int32(HasteMelee);
            }
        }

        void CalculateAmountResistance(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
                amount += owner->GetInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE);
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_06_AuraScript::CalculateAmountCritPct, EFFECT_0, SPELL_AURA_MOD_CRIT_PCT);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_06_AuraScript::CalculateAmountMeleeHaste, EFFECT_1, SPELL_AURA_MELEE_SLOW);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_scaling_06_AuraScript::CalculateAmountResistance, EFFECT_2, SPELL_AURA_MOD_TARGET_RESISTANCE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_pet_scaling_06_AuraScript();
    }
};

class spell_warl_pet_passive : public SpellScriptLoader
{
public:
    spell_warl_pet_passive() : SpellScriptLoader("spell_warl_pet_passive") { }

    class spell_warl_pet_passive_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_pet_passive_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountCritSpell(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                float CritSpell = 0.0f;
                CritSpell += owner->GetSpellCritFromIntellect();
                CritSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_CRIT_CHANCE);
                CritSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_PCT);
                CritSpell += owner->GetRatingBonusValue(CR_CRIT_SPELL);
                if (AuraApplication* improvedDemonicTacticsApp = owner->GetAuraApplicationOfRankedSpell(54347))
                    if (Aura* improvedDemonicTactics = improvedDemonicTacticsApp->GetBase())
                        if (AuraEffect* improvedDemonicTacticsEffect = improvedDemonicTactics->GetEffect(EFFECT_0))
                            amount += CalculatePct(CritSpell, improvedDemonicTacticsEffect->GetAmount());
            }
        }

        void CalculateAmountCritMelee(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                float CritMelee = 0.0f;
                CritMelee += owner->GetMeleeCritFromAgility();
                CritMelee += owner->GetTotalAuraModifier(SPELL_AURA_MOD_WEAPON_CRIT_PERCENT);
                CritMelee += owner->GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_PCT);
                CritMelee += owner->GetRatingBonusValue(CR_CRIT_MELEE);
                if (AuraApplication* improvedDemonicTacticsApp = owner->GetAuraApplicationOfRankedSpell(54347))
                    if (Aura* improvedDemonicTactics = improvedDemonicTacticsApp->GetBase())
                        if (AuraEffect* improvedDemonicTacticsEffect = improvedDemonicTactics->GetEffect(EFFECT_0))
                            amount += CalculatePct(CritMelee, improvedDemonicTacticsEffect->GetAmount());
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_passive_AuraScript::CalculateAmountCritSpell, EFFECT_0, SPELL_AURA_MOD_SPELL_CRIT_CHANCE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_passive_AuraScript::CalculateAmountCritMelee, EFFECT_1, SPELL_AURA_MOD_WEAPON_CRIT_PERCENT);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_pet_passive_AuraScript();
    }
};

class spell_warl_pet_passive_damage_done : public SpellScriptLoader
{
public:
    spell_warl_pet_passive_damage_done() : SpellScriptLoader("spell_warl_pet_passive_damage_done") { }

    class spell_warl_pet_passive_damage_done_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_pet_passive_damage_done_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountDamageDone(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!GetCaster() || !GetCaster()->GetOwner())
                return;
            if (GetCaster()->GetOwner()->ToPlayer())
            {
                switch (GetCaster()->GetEntry())
                {
                case ENTRY_VOIDWALKER:
                    amount += -16;
                    break;
                case ENTRY_SUCCUBUS:
                case ENTRY_FELGUARD:
                    amount += 5;
                    break;
                }
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_passive_damage_done_AuraScript::CalculateAmountDamageDone, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_passive_damage_done_AuraScript::CalculateAmountDamageDone, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_pet_passive_damage_done_AuraScript();
    }
};

class spell_warl_pet_passive_voidwalker : public SpellScriptLoader
{
public:
    spell_warl_pet_passive_voidwalker() : SpellScriptLoader("spell_warl_pet_passive_voidwalker") { }

    class spell_warl_pet_passive_voidwalker_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_pet_passive_voidwalker_AuraScript);

        bool Load()
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
                if (pet->isPet())
                    if (Unit* owner = pet->ToPet()->GetOwner())
                        if (AuraEffect* ect = owner->GetAuraEffect(SPELL_WARLOCK_GLYPH_OF_VOIDWALKER, EFFECT_0))
                            amount += ect->GetAmount();
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_pet_passive_voidwalker_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_pet_passive_voidwalker_AuraScript();
    }
};

class spell_warl_pet_damage_spells : public SpellScriptLoader
{
public:
    spell_warl_pet_damage_spells() : SpellScriptLoader("spell_warl_pet_damage_spells") { }

    class spell_warl_pet_damage_spells_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_warl_pet_damage_spells_SpellScript);

        void HandleHit(SpellEffIndex /*effIndex*/)
        {
            if (Unit* owner = GetCaster()->GetOwner())
            {
                float coeff = 1.0f;
                switch (GetSpellInfo()->Id)
                {
                    // Firebolt
                case 3110:
                    coeff = 0.657f;
                    break;
                    // Shadow bite
                case 54049:
                    coeff = 1.228f;
                    break;
                    // Whiplash
                case 6360:
                    coeff = 0.85f;
                    break;
                    // Lash of pain
                case 7814:
                    coeff = 0.612f;
                    break;
                }
                int32 baseDamage = GetEffectValue() + (coeff * owner->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask()) * 0.5f);
                SetEffectDamage(baseDamage);
            }
        }

        void Register()
        {
            OnEffectLaunchTarget += SpellEffectFn(spell_warl_pet_damage_spells_SpellScript::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_warl_pet_damage_spells_SpellScript();
    }
};

void AddSC_pet_spell_scripts()
{
    new spell_hun_pet_carrion_feeder(); // Verified
	new spell_hun_pet_heart_of_the_phoenix(); // Verified
	new spell_hun_pet_scaling_01(); // Verified
	new spell_hun_pet_scaling_02(); // Verified
	new spell_hun_pet_scaling_03(); // Verified
	new spell_hun_pet_scaling_04(); // Verified
	new spell_hun_pet_scaling_05(); // Verified
	new spell_hun_pet_passive_crit(); // Verified
	new spell_hun_pet_passive_damage_done(); // Verified
	new spell_hun_pet_passive_tree_marker(); // Verified
	new spell_hun_pet_damage_spells(); // Verified
	new spell_hun_pet_spirit_mend(); // Verified
	new spell_hun_mend_pet(); // Verified
    new spell_hun_last_stand_pet(); // Verified
    new spell_hun_cower(); // Recently Verified
    new spell_hun_tamed_pet_passive_07(); // Recently Verified
	new spell_dk_pet_spells(); // Verified
	new spell_dk_pet_scaling_01(); // Verified
	new spell_dk_pet_scaling_02(); // Verified
	new spell_dk_pet_scaling_03(); // Verified
    new spell_dk_pet_scaling_05(); // Recently Verified
    new spell_dk_rune_weapon_scaling_02(); // Verified
    new spell_dk_ghoul_leap(); // Recently Verified
    new spell_mage_pet_scaling_05(); // Verified
    new spell_mage_orb_filter(); // Verified
    new spell_mage_pet_freeze(); // Recently Verified
    new spell_pri_pet_scaling_05(); // Recently Verified
    new spell_pri_mana_leech(); // Verified
    new spell_pri_shadowflame(); // Recently Verified
    new spell_sha_pet_scaling_04(); // Verified
    new spell_warl_pet_scaling_01(); // Verified
    new spell_warl_pet_scaling_02(); // Verified
    new spell_warl_pet_scaling_03(); // Verified
    new spell_warl_pet_scaling_04(); // Verified
    new spell_warl_pet_scaling_05(); // Verified
    new spell_warl_pet_scaling_06(); // Recently Verified
	new spell_warl_pet_passive(); // Verified
	new spell_warl_pet_passive_damage_done(); // Verified
	new spell_warl_pet_passive_voidwalker(); // Verified
	new spell_warl_pet_damage_spells(); // Verified
}
