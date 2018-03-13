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
 * Scripts for spells with SPELLFAMILY_PALADIN and SPELLFAMILY_GENERIC spells used by paladin players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_pal_".
 */

#include "GridNotifiers.h"
#include "Group.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

enum PaladinSpells
{
    SPELL_PALADIN_DIVINE_PLEA                    = 54428,
    SPELL_PALADIN_BLESSING_OF_SANCTUARY_BUFF     = 67480,
    SPELL_PALADIN_BLESSING_OF_SANCTUARY_ENERGIZE = 57319,

    SPELL_PALADIN_HOLY_SHOCK_R1                  = 20473,
    SPELL_PALADIN_HOLY_SHOCK_R1_DAMAGE           = 25912,
    SPELL_PALADIN_HOLY_SHOCK_R1_HEALING          = 25914,

    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_DRUID   = 37878,
    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PALADIN = 37879,
    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PRIEST  = 37880,
    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_SHAMAN  = 37881,

    SPELL_PALADIN_DIVINE_STORM                   = 53385,
    SPELL_PALADIN_DIVINE_STORM_DUMMY             = 54171,
    SPELL_PALADIN_DIVINE_STORM_HEAL              = 54172,

    SPELL_PALADIN_EYE_FOR_AN_EYE_RANK_1          = 9799,
    SPELL_PALADIN_EYE_FOR_AN_EYE_DAMAGE          = 25997,

    SPELL_PALADIN_FORBEARANCE                    = 25771,
    SPELL_PALADIN_IMMUNE_SHIELD_MARKER           = 61988,

    SPELL_PALADIN_HAND_OF_SACRIFICE              = 6940,
    SPELL_PALADIN_DIVINE_SACRIFICE               = 64205,

    SPELL_PALADIN_DIVINE_PURPOSE_PROC            = 90174,

    SPELL_PALADIN_GLYPH_OF_SALVATION             = 63225,

    SPELL_PALADIN_RIGHTEOUS_DEFENSE_TAUNT        = 31790,

    SPELL_PALADIN_SEAL_OF_RIGHTEOUSNESS          = 25742,
    SPELL_PALADIN_SEALS_OF_COMMAND               = 85126,

    SPELL_PALADIN_DELAYED_JUDGEMENT              = 105801,

    SPELL_GENERIC_ARENA_DAMPENING                = 74410,
    SPELL_GENERIC_BATTLEGROUND_DAMPENING         = 74411,

    SPELL_GLYPH_OF_DIVINITY_AURA                 = 54939,
    SPELL_GLYPH_OF_DIVINITY_EFFECT               = 54986,

    SPELL_GLYPH_OF_HOLY_WRATH                    = 56420,
    SPELL_GLYPH_OF_LIGHT_OF_DAWN                 = 54940,

    SPELL_PALADIN_WORD_OF_GLORY                  = 85673,
    SPELL_PALADIN_DIVINE_LIGHT                   = 82326,
    SPELL_PALADIN_FLASH_OF_LIGHT                 = 19750,
    SPELL_PALADIN_HOLY_LIGHT                     = 635,
};

// 31850 - Ardent Defender
class spell_pal_ardent_defender : public SpellScriptLoader
{
    public:
        spell_pal_ardent_defender() : SpellScriptLoader("spell_pal_ardent_defender") { }

        class spell_pal_ardent_defender_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_ardent_defender_AuraScript);

            uint32 absorbPct, healPct;

            enum Spell
            {
                PAL_SPELL_ARDENT_DEFENDER_HEAL = 66235,
            };

            bool Load() override
            {
                healPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue();
                absorbPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue();
                return GetUnitOwner()->GetTypeId() == TYPEID_PLAYER;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                Unit* victim = GetTarget();
                int32 remainingHealth = victim->GetHealth() - dmgInfo.GetDamage();
                // If damage kills us
                if (remainingHealth <= 0)
                {
                    // Cast healing spell, completely avoid damage
                    absorbAmount = dmgInfo.GetDamage();

                    int32 healAmount = int32(victim->CountPctFromMaxHealth(healPct));
                    victim->SetHealth(1); // same like Cauterize - downgrade the health before the heal is casted because the absorb don't set the health low (so we have absorb health + heal)
                    victim->CastCustomSpell(victim, PAL_SPELL_ARDENT_DEFENDER_HEAL, &healAmount, NULL, NULL, true, NULL, aurEff);
                    Remove(AURA_REMOVE_BY_DEFAULT);
                }
                else
                {
                    uint32 damageToReduce = dmgInfo.GetDamage();
                    absorbAmount = CalculatePct(damageToReduce, absorbPct);
                }
            }

            void Register() override
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_ardent_defender_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_ardent_defender_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_ardent_defender_AuraScript();
        }
};

// 37877 - Blessing of Faith
class spell_pal_blessing_of_faith : public SpellScriptLoader
{
    public:
        spell_pal_blessing_of_faith() : SpellScriptLoader("spell_pal_blessing_of_faith") { }

        class spell_pal_blessing_of_faith_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_blessing_of_faith_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_BLESSING_OF_LOWER_CITY_DRUID) || !sSpellMgr->GetSpellInfo(SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PALADIN) || !sSpellMgr->GetSpellInfo(SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PRIEST) || !sSpellMgr->GetSpellInfo(SPELL_PALADIN_BLESSING_OF_LOWER_CITY_SHAMAN))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* unitTarget = GetHitUnit())
                {
                    uint32 spell_id = 0;
                    switch (unitTarget->getClass())
                    {
                        case CLASS_DRUID:
                            spell_id = SPELL_PALADIN_BLESSING_OF_LOWER_CITY_DRUID;
                            break;
                        case CLASS_PALADIN:
                            spell_id = SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PALADIN;
                            break;
                        case CLASS_PRIEST:
                            spell_id = SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PRIEST;
                            break;
                        case CLASS_SHAMAN:
                            spell_id = SPELL_PALADIN_BLESSING_OF_LOWER_CITY_SHAMAN;
                            break;
                        default:
                            return; // ignore for non-healing classes
                    }
                    Unit* caster = GetCaster();
                    caster->CastSpell(caster, spell_id, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_blessing_of_faith_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_blessing_of_faith_SpellScript();
        }
};

// Judgement - 54158
class spell_pal_judgement : public SpellScriptLoader
{
    public:
        spell_pal_judgement() : SpellScriptLoader("spell_pal_judgement") { }

        class spell_pal_judgement_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_judgement_SpellScript);

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                Player* caster = GetCaster()->ToPlayer();
                int32 spellPower = caster->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask());
                int32 attackPower = caster->GetTotalAttackPowerValue(BASE_ATTACK);
                int32 basepoints0 = 0;
                Unit::AuraApplicationMap & sealAuras = caster->GetAppliedAuras();
                for (Unit::AuraApplicationMap::iterator iter = sealAuras.begin(); iter != sealAuras.end(); iter++)
                {
                    Aura* aura = iter->second->GetBase();
                    if (aura->GetSpellInfo()->GetSpellSpecific() == SPELL_SPECIFIC_SEAL)
                    {
                        switch (aura->GetSpellInfo()->Id)
                        {
                            case 20165: // Seal of Insight
                                basepoints0 = 0.25f * spellPower + 0.16f * attackPower;
                                break;
                            case 20154: // Seal of Righteousness
                                basepoints0 = 0.32f * spellPower + 0.2f * attackPower;
                                break;
                            case 20164: // Seal of Justice
                                basepoints0 = 0.25f * spellPower + 0.16f * attackPower;
                                break;
                            case 31801: // Seal of Truth
                            {
                                basepoints0 = 0.223f * spellPower + 0.142f * attackPower;
                                // Damage is increased by 20% per stack
                                if (Aura* censure = GetHitUnit()->GetAura(31803, caster->GetGUID()))
                                    AddPct(basepoints0, censure->GetStackAmount() * 20);
                                break;
                            }
                        }
                        break;
                    }
                }
                caster->CastCustomSpell(GetHitUnit(), 54158, &basepoints0, NULL, NULL, true);
                // Long arm of the law
                if (AuraEffect* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_PALADIN, 3013, EFFECT_0))
                    if (roll_chance_i(aurEff->GetAmount()))
                        if (caster->GetDistance(GetHitUnit()) > 15.0f)
                            caster->CastSpell(caster, 87173, true);

                // Communion
                if (caster->GetAuraEffect(31876, EFFECT_1, caster->GetGUID()))
                    caster->CastSpell(caster, 57669, true);
            }

			void BypassTriggers()
			{
				// Remove frost armor after judgement casted
				// No other way to handle this, hackfix
				// Since judgement isn't a 'melee attack' and shouldn't proc certain triggers

				// Bypass frost armor
				Player* caster = GetCaster()->ToPlayer();
				if (caster->HasAura(7321))
					caster->RemoveAurasDueToSpell(7321);

				// Apply same hackfix for nature's grasp
				if (caster->HasAura(19975))
					caster->RemoveAurasDueToSpell(19975);
			}

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_judgement_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_judgement_SpellScript();
        }
};

// 64205 - Divine Sacrifice
class spell_pal_divine_sacrifice : public SpellScriptLoader
{
    public:
        spell_pal_divine_sacrifice() : SpellScriptLoader("spell_pal_divine_sacrifice") { }

        class spell_pal_divine_sacrifice_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_divine_sacrifice_AuraScript);

            uint32 groupSize, minHpPct;
            int32 remainingAmount;

            bool Load()
            {

                if (Unit* caster = GetCaster())
                {
                    if (caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        if (caster->ToPlayer()->GetGroup())
                            groupSize = caster->ToPlayer()->GetGroup()->GetMembersCount();
                        else
                            groupSize = 1;
                    }
                    else
                        return false;

                    remainingAmount = (caster->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_2].CalcValue(caster)) * groupSize);
                    minHpPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue(caster);
                    return true;
                }
                return false;
            }

            void Split(AuraEffect* /*aurEff*/, DamageInfo & /*dmgInfo*/, uint32 & splitAmount)
            {
                remainingAmount -= splitAmount;
                // break when absorbed everything it could, or if the casters hp drops below 20%
                if (Unit* caster = GetCaster())
                    if (remainingAmount <= 0 || (caster->GetHealthPct() < minHpPct))
                        caster->RemoveAura(SPELL_PALADIN_DIVINE_SACRIFICE);
            }

            void Register()
            {
                OnEffectSplit += AuraEffectSplitFn(spell_pal_divine_sacrifice_AuraScript::Split, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_divine_sacrifice_AuraScript();
        }
};

// 53385 - Divine Storm
class spell_pal_divine_storm : public SpellScriptLoader
{
    public:
        spell_pal_divine_storm() : SpellScriptLoader("spell_pal_divine_storm") { }

        class spell_pal_divine_storm_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_divine_storm_SpellScript);

            uint32 healPct;

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_DIVINE_STORM_DUMMY))
                    return false;
                return true;
            }

            bool Load()
            {
                healPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue(GetCaster());
                return true;
            }

            void CountTargets(std::list<WorldObject*>& targetList)
            {
                _targetCount = targetList.size();
            }

            void HandleEffect(SpellEffIndex effIndex)
            {
                if (_targetCount < 4)
                    PreventHitDefaultEffect(effIndex);
            }

            void TriggerHeal()
            {
                Unit* caster = GetCaster();
                caster->CastCustomSpell(SPELL_PALADIN_DIVINE_STORM_DUMMY, SPELLVALUE_BASE_POINT0, (GetHitDamage() * healPct) / 100, caster, true);
            }

        private:
            uint32 _targetCount;

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_divine_storm_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_ENERGIZE);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_divine_storm_SpellScript::CountTargets, EFFECT_2, TARGET_UNIT_SRC_AREA_ENEMY);
                AfterHit += SpellHitFn(spell_pal_divine_storm_SpellScript::TriggerHeal);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_divine_storm_SpellScript();
        }
};

// 54171 - Divine Storm (Dummy)
class spell_pal_divine_storm_dummy : public SpellScriptLoader
{
    public:
        spell_pal_divine_storm_dummy() : SpellScriptLoader("spell_pal_divine_storm_dummy") { }

        class spell_pal_divine_storm_dummy_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_divine_storm_dummy_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_DIVINE_STORM_HEAL))
                    return false;
                return true;
            }

            void CountTargets(std::list<WorldObject*>& targetList)
            {
                _targetCount = targetList.size();
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (!_targetCount || ! GetHitUnit())
                    return;

                int32 heal = GetEffectValue() / _targetCount;
                GetCaster()->CastCustomSpell(GetHitUnit(), SPELL_PALADIN_DIVINE_STORM_HEAL, &heal, NULL, NULL, true);
            }
        private:
            uint32 _targetCount;

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_divine_storm_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_divine_storm_dummy_SpellScript::CountTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_RAID);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_divine_storm_dummy_SpellScript();
        }
};

// 33695 - Exorcism and Holy Wrath Damage
class spell_pal_exorcism_and_holy_wrath_damage : public SpellScriptLoader
{
    public:
        spell_pal_exorcism_and_holy_wrath_damage() : SpellScriptLoader("spell_pal_exorcism_and_holy_wrath_damage") { }

        class spell_pal_exorcism_and_holy_wrath_damage_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_exorcism_and_holy_wrath_damage_AuraScript);

            void HandleEffectCalcSpellMod(AuraEffect const* aurEff, SpellModifier*& spellMod)
            {
                if (!spellMod)
                {
                    spellMod = new SpellModifier(aurEff->GetBase());
                    spellMod->op = SPELLMOD_DAMAGE;
                    spellMod->type = SPELLMOD_FLAT;
                    spellMod->spellId = GetId();
                    spellMod->mask[1] = 0x200002;
                }

                spellMod->value = aurEff->GetAmount();
            }

            void Register()
            {
                DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(spell_pal_exorcism_and_holy_wrath_damage_AuraScript::HandleEffectCalcSpellMod, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_exorcism_and_holy_wrath_damage_AuraScript();
        }
};

// -9799 - Eye for an Eye
class spell_pal_eye_for_an_eye : public SpellScriptLoader
{
    public:
        spell_pal_eye_for_an_eye() : SpellScriptLoader("spell_pal_eye_for_an_eye") { }

        class spell_pal_eye_for_an_eye_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_eye_for_an_eye_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_EYE_FOR_AN_EYE_DAMAGE))
                    return false;
                return true;
            }

            void HandleEffectProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                int32 damage = CalculatePct(eventInfo.GetDamageInfo()->GetDamage(), aurEff->GetAmount());
                GetTarget()->CastCustomSpell(SPELL_PALADIN_EYE_FOR_AN_EYE_DAMAGE, SPELLVALUE_BASE_POINT0, damage, eventInfo.GetProcTarget(), true, NULL, aurEff);
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_pal_eye_for_an_eye_AuraScript::HandleEffectProc, EFFECT_0, m_scriptSpellId == SPELL_PALADIN_EYE_FOR_AN_EYE_RANK_1 ? SPELL_AURA_DUMMY : SPELL_AURA_PROC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_eye_for_an_eye_AuraScript();
        }
};

// 63521 - Guarded by The Light
class spell_pal_guarded_by_the_light : public SpellScriptLoader
{
    public:
        spell_pal_guarded_by_the_light() : SpellScriptLoader("spell_pal_guarded_by_the_light") { }

        class spell_pal_guarded_by_the_light_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_guarded_by_the_light_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_DIVINE_PLEA))
                    return false;
                return true;
            }

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                // Divine Plea
                if (Aura* aura = GetCaster()->GetAura(SPELL_PALADIN_DIVINE_PLEA))
                    aura->RefreshDuration();
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_guarded_by_the_light_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_guarded_by_the_light_SpellScript();
        }
};

// 6940 - Hand of Sacrifice
class spell_pal_hand_of_sacrifice : public SpellScriptLoader
{
    public:
        spell_pal_hand_of_sacrifice() : SpellScriptLoader("spell_pal_hand_of_sacrifice") { }

        class spell_pal_hand_of_sacrifice_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_hand_of_sacrifice_AuraScript);

            int32 remainingAmount;

            bool Load()
            {
                if (Unit* caster = GetCaster())
                {
                    remainingAmount = caster->GetMaxHealth();
                    return true;
                }
                return false;
            }

            void Split(AuraEffect* /*aurEff*/, DamageInfo & /*dmgInfo*/, uint32 & splitAmount)
            {
                remainingAmount -= splitAmount;

                if (remainingAmount <= 0)
                {
                    GetTarget()->RemoveAura(SPELL_PALADIN_HAND_OF_SACRIFICE);
                }
            }

            void Register()
            {
                OnEffectSplit += AuraEffectSplitFn(spell_pal_hand_of_sacrifice_AuraScript::Split, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_hand_of_sacrifice_AuraScript();
        }
};

// 1038 - Hand of Salvation
class spell_pal_hand_of_salvation : public SpellScriptLoader
{
    public:
        spell_pal_hand_of_salvation() : SpellScriptLoader("spell_pal_hand_of_salvation") { }

        class spell_pal_hand_of_salvation_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_hand_of_salvation_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    // Glyph of Salvation
                    if (caster->GetAuraEffect(SPELL_PALADIN_GLYPH_OF_SALVATION, EFFECT_0, caster->GetGUID()))
                        PreventDefaultAction();
                }
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_pal_hand_of_salvation_AuraScript::OnApply, EFFECT_1, SPELL_AURA_MOD_TOTAL_THREAT, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_hand_of_salvation_AuraScript();
        }
};

// -20473 - Holy Shock
class spell_pal_holy_shock : public SpellScriptLoader
{
    public:
        spell_pal_holy_shock() : SpellScriptLoader("spell_pal_holy_shock") { }

        class spell_pal_holy_shock_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_holy_shock_SpellScript);

            bool Validate(SpellInfo const* spell)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_HOLY_SHOCK_R1))
                    return false;

                // can't use other spell than holy shock due to spell_ranks dependency
                if (sSpellMgr->GetFirstSpellInChain(SPELL_PALADIN_HOLY_SHOCK_R1) != sSpellMgr->GetFirstSpellInChain(spell->Id))
                    return false;

                uint8 rank = sSpellMgr->GetSpellRank(spell->Id);
                if (!sSpellMgr->GetSpellWithRank(SPELL_PALADIN_HOLY_SHOCK_R1_DAMAGE, rank, true) || !sSpellMgr->GetSpellWithRank(SPELL_PALADIN_HOLY_SHOCK_R1_HEALING, rank, true))
                    return false;

                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                {
                    uint8 rank = sSpellMgr->GetSpellRank(GetSpellInfo()->Id);
                    if (caster->IsFriendlyTo(unitTarget))
                        caster->CastSpell(unitTarget, sSpellMgr->GetSpellWithRank(SPELL_PALADIN_HOLY_SHOCK_R1_HEALING, rank), true, 0);
                    else
                        caster->CastSpell(unitTarget, sSpellMgr->GetSpellWithRank(SPELL_PALADIN_HOLY_SHOCK_R1_DAMAGE, rank), true, 0);
                }
            }

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetExplTargetUnit())
                {
                    if (!caster->IsFriendlyTo(target))
                    {
                        if (!caster->IsValidAttackTarget(target))
                            return SPELL_FAILED_BAD_TARGETS;

                        if (!caster->isInFront(target))
                            return SPELL_FAILED_UNIT_NOT_INFRONT;
                    }
                    else
                    {
                        if (!caster->IsValidAssistTarget(target))
                            return SPELL_FAILED_BAD_TARGETS;
                    }
                }
                else
                    return SPELL_FAILED_BAD_TARGETS;
                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_pal_holy_shock_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_pal_holy_shock_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_holy_shock_SpellScript();
        }
};

// Guardian of ancient kings
class spell_pal_guardian : public SpellScriptLoader
{
    public:
        spell_pal_guardian() : SpellScriptLoader("spell_pal_guardian") { }

        class spell_pal_guardian_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_guardian_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                // Choose which guardian to summon based on spec
                switch (caster->ToPlayer()->GetPrimaryTalentTree(caster->ToPlayer()->GetActiveSpec()))
                {
                    case TALENT_TREE_PALADIN_HOLY:
                        caster->RemoveAllMinionsByEntry(46499);
                        caster->CastSpell(caster, 86669, true);
                        caster->CastSpell(caster, 86674, true);
                        break;
                    case TALENT_TREE_PALADIN_PROTECTION:
                        caster->RemoveAllMinionsByEntry(46490);
                        caster->CastSpell(caster, 86659, true);
                        break;
                    case TALENT_TREE_PALADIN_RETRIBUTION:
                        caster->RemoveAllMinionsByEntry(46506);
                        caster->CastSpell(caster, 86698, true);
                        caster->CastSpell(caster, 86701, true);
                        break;
                    default:
                        return;
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pal_guardian_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_guardian_SpellScript();
        }
};

// Melee guardian spell on duration end
class spell_pal_guardian_triggered : public SpellScriptLoader
{
    public:
        spell_pal_guardian_triggered() : SpellScriptLoader("spell_pal_guardian_triggered") { }

        class spell_pal_guardian_triggered_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_guardian_triggered_AuraScript);

            void Remove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
                    return;

                Unit* target = GetTarget();
                Aura* stacks = target->GetAura(86700, target->GetGUID());
                if (!stacks)
                    return;

                int32 bp0 = sSpellMgr->GetSpellInfo(86704)->Effects[EFFECT_0].CalcValue(target) * stacks->GetStackAmount();
                target->CastCustomSpell(target, 86704, &bp0, NULL, NULL, true);
                target->RemoveAurasDueToSpell(86700);
                target->RemoveAurasDueToSpell(86701);
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_pal_guardian_triggered_AuraScript::Remove, EFFECT_2, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_guardian_triggered_AuraScript();
        }
};

// 633 - Lay on Hands
class spell_pal_lay_on_hands : public SpellScriptLoader
{
    public:
        spell_pal_lay_on_hands() : SpellScriptLoader("spell_pal_lay_on_hands") { }

        class spell_pal_lay_on_hands_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_lay_on_hands_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_FORBEARANCE))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_IMMUNE_SHIELD_MARKER))
                    return false;
                return true;
            }

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetExplTargetUnit())
                    if (caster == target)
                        if (target->HasAura(SPELL_PALADIN_FORBEARANCE) || target->HasAura(SPELL_PALADIN_IMMUNE_SHIELD_MARKER))
                            return SPELL_FAILED_TARGET_AURASTATE;

                return SPELL_CAST_OK;
            }

            void HandleScript()
            {
                Unit* caster = GetCaster();
                if (caster == GetHitUnit())
                {
                    caster->CastSpell(caster, SPELL_PALADIN_FORBEARANCE, true);
                    caster->CastSpell(caster, SPELL_PALADIN_IMMUNE_SHIELD_MARKER, true);
                }
                // Glyph of Divinity
                if (caster->HasAura(SPELL_GLYPH_OF_DIVINITY_AURA))
                    caster->CastSpell(caster, SPELL_GLYPH_OF_DIVINITY_EFFECT, true);
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_pal_lay_on_hands_SpellScript::CheckCast);
                AfterHit += SpellHitFn(spell_pal_lay_on_hands_SpellScript::HandleScript);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_lay_on_hands_SpellScript();
        }
};

// 31789 - Righteous Defense
class spell_pal_righteous_defense : public SpellScriptLoader
{
    public:
        spell_pal_righteous_defense() : SpellScriptLoader("spell_pal_righteous_defense") { }

        class spell_pal_righteous_defense_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_righteous_defense_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_RIGHTEOUS_DEFENSE_TAUNT))
                    return false;
                return true;
            }

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (caster->GetTypeId() != TYPEID_PLAYER)
                    return SPELL_FAILED_DONT_REPORT;

                if (Unit* target = GetExplTargetUnit())
                {
                    if (!target->IsFriendlyTo(caster) || target->getAttackers().empty())
                        return SPELL_FAILED_BAD_TARGETS;
                }
                else
                    return SPELL_FAILED_BAD_TARGETS;

                return SPELL_CAST_OK;
            }

            void HandleTriggerSpellLaunch(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
            }

            void HandleTriggerSpellHit(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                if (Unit* target = GetHitUnit())
                    GetCaster()->CastSpell(target, SPELL_PALADIN_RIGHTEOUS_DEFENSE_TAUNT, true);
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_pal_righteous_defense_SpellScript::CheckCast);
                //! WORKAROUND
                //! target select will be executed in hitphase of effect 0
                //! so we must handle trigger spell also in hit phase (default execution in launch phase)
                //! see issue #3718
                OnEffectLaunchTarget += SpellEffectFn(spell_pal_righteous_defense_SpellScript::HandleTriggerSpellLaunch, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
                OnEffectHitTarget += SpellEffectFn(spell_pal_righteous_defense_SpellScript::HandleTriggerSpellHit, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_righteous_defense_SpellScript();
        }
};

// 85256 - Templar's Verdict
/// Updated 4.3.4
class spell_pal_templar_s_verdict : public SpellScriptLoader
{
    public:
        spell_pal_templar_s_verdict() : SpellScriptLoader("spell_pal_templar_s_verdict") { }

        class spell_pal_templar_s_verdict_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_templar_s_verdict_SpellScript);

            bool Validate (SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_DIVINE_PURPOSE_PROC))
                    return false;

                return true;
            }

            bool Load()
            {
                if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    return false;

                if (GetCaster()->ToPlayer()->getClass() != CLASS_PALADIN)
                    return false;

                return true;
            }

            void ChangeDamage(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                int32 damage = GetHitDamage();

                if (caster->HasAura(SPELL_PALADIN_DIVINE_PURPOSE_PROC))
                    damage *= 7.84;  // 7.5*30% = 225%
                else
                {
                    switch (caster->GetPower(POWER_HOLY_POWER))
                    {
                        case 1: // 1 Holy Power
                            damage = damage;
                            break;
                        case 2: // 2 Holy Power
                            damage *= 3;    // 3*30 = 90%
                            break;
                        case 3: // 3 Holy Power
                            damage *= 7.84;  // 7.5*30% = 225%
                            break;
                    }
                }

                SetHitDamage(damage);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_templar_s_verdict_SpellScript::ChangeDamage, EFFECT_0, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_templar_s_verdict_SpellScript();
        }
};

// Light of Dawn
class spell_pal_lod : public SpellScriptLoader
{
    public:
        spell_pal_lod() : SpellScriptLoader("spell_pal_lod") { }

        class spell_pal_lod_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_lod_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                uint8 targetCount = 6;
                if (GetCaster()->HasAura(SPELL_GLYPH_OF_LIGHT_OF_DAWN))
                    targetCount = 4;

                std::list<Unit*> temp;
                for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end(); itr++)
                    if (Unit* unit = (*itr)->ToUnit())
                        temp.push_back(unit);

                targets.clear();
                temp.sort(Trinity::HealthPctOrderPred());
                if (temp.size() > targetCount)
                    temp.resize(targetCount);
                for (std::list<Unit*>::iterator itr = temp.begin(); itr != temp.end(); itr++)
                    targets.push_back((WorldObject*)(*itr));
            }

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                int32 heal = GetHitHeal() * GetCaster()->GetPower(POWER_HOLY_POWER);
                SetHitHeal(heal);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_lod_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ALLY);
                OnEffectHitTarget += SpellEffectFn(spell_pal_lod_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_lod_SpellScript();
        }
};

// Consecration
class spell_pal_consecration : public SpellScriptLoader
{
    public:
        spell_pal_consecration() : SpellScriptLoader("spell_pal_consecration") { }

        class spell_pal_consecration_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_consecration_AuraScript);

            void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
            {
                // Dummy aura is applied to enemys around us - Why?
                if (GetTarget() != GetCaster())
                    return;

                TriggerCastFlags triggerMask = TriggerCastFlags(TRIGGERED_FULL_MASK & ~TRIGGERED_DISALLOW_PROC_EVENTS);

                SpellInfo const* consecration = sSpellMgr->GetSpellInfo(81297);

                std::list<Creature*> MinionList;
                GetTarget()->GetAllMinionsByEntry(MinionList, 43499);
                for (std::list<Creature*>::iterator itr = MinionList.begin(); itr != MinionList.end(); itr++)
                {
                    SpellCastTargets targets;
                    targets.SetDst((*itr)->GetPositionX(), (*itr)->GetPositionY(), (*itr)->GetPositionZ(), 0);
                    GetTarget()->CastSpell(targets, consecration, NULL, triggerMask);
                }
            }

            void Apply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetTarget() != GetCaster())
                    return;

                std::list<Creature*> MinionList;
                GetTarget()->GetAllMinionsByEntry(MinionList, 43499);
                TempSummon* consecration = NULL;
                // Get the last summoned Consecration, save it and despawn older ones
                for (std::list<Creature*>::iterator itr = MinionList.begin(); itr != MinionList.end(); itr++)
                {
                    TempSummon* summon = (*itr)->ToTempSummon();

                    if (consecration && summon)
                    {
                        if (summon->GetTimer() > consecration->GetTimer())
                        {
                            consecration->DespawnOrUnsummon();
                            consecration = summon;
                        }
                        else
                            summon->DespawnOrUnsummon();
                    }
                    else if (summon)
                        consecration = summon;
                }
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_pal_consecration_AuraScript::Apply, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pal_consecration_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_consecration_AuraScript();
        }
};

// Word of Glory
class spell_pal_word_of_glory: public SpellScriptLoader
{
    public:
        spell_pal_word_of_glory() : SpellScriptLoader("spell_pal_word_of_glory") { }

        class spell_pal_word_of_glory_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_word_of_glory_SpellScript);

            bool Load()
            {
                hitHeal = 0;
                overheal = 0;
                return true;
            }

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                int8 holyPower = caster->GetPower(POWER_HOLY_POWER);
                if (caster->HasAura(SPELL_PALADIN_DIVINE_PURPOSE_PROC))
                    holyPower = GetCaster()->GetMaxPower(POWER_HOLY_POWER);
                int32 heal = GetHitHeal() * holyPower;
                if (caster != GetHitUnit())
                {
                    // Selfless healer increases output on other targets by x%
                    if (AuraEffect* selflessHealer = caster->GetDummyAuraEffect(SPELLFAMILY_PALADIN, 3924, EFFECT_0))
                        AddPct(heal, selflessHealer->GetAmount());
                }
                SetHitHeal(heal);
                hitHeal = heal;

                if (caster->GetDummyAuraEffect(SPELLFAMILY_PALADIN, 3026, EFFECT_0))
                    if ((caster->GetHealth() + hitHeal) > caster->GetMaxHealth())
                        overheal = (caster->GetHealth() + hitHeal) - caster->GetMaxHealth();

                if (overheal)
                    caster->CastCustomSpell(caster, 88063, &overheal, NULL, NULL, true);
            }

            void HandleHot(SpellEffIndex /*effIndex*/)
            {
                AuraEffect* glyph = GetCaster()->GetAuraEffect(93466, EFFECT_1);
                if (!glyph)
                {
                    PreventHitAura();
                    return;
                }
                uint32 newHeal = CalculatePct(hitHeal * 2, glyph->GetAmount());
                if (GetHitAura())
                    if (AuraEffect* periodic = GetHitAura()->GetEffect(EFFECT_1))
                        periodic->SetAmount(newHeal / periodic->GetTotalTicks());
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_word_of_glory_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
                OnEffectHitTarget += SpellEffectFn(spell_pal_word_of_glory_SpellScript::HandleHot, EFFECT_1, SPELL_EFFECT_APPLY_AURA);
            }
        private:
            uint32 hitHeal;
            int32 overheal;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_word_of_glory_SpellScript();
        }
};

// Divine Purpose
class spell_pal_divine_purpose : public SpellScriptLoader
{
    public:
        spell_pal_divine_purpose() : SpellScriptLoader("spell_pal_divine_purpose") { }

        class spell_pal_divine_purpose_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_divine_purpose_SpellScript);

            bool Load()
            {
                checked = false;
                return true;
            }

            void HandleOnHit()
            {
                if (checked)
                    return;

                checked = true;
                Unit* caster = GetCaster();
                if (GetHitUnit())
                    if (AuraEffect* dPurpose = caster->GetDummyAuraEffect(SPELLFAMILY_PALADIN, 2170, EFFECT_0))
                        if (roll_chance_i(dPurpose->GetAmount()))
                            caster->CastWithDelay(100, caster, 90174, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pal_divine_purpose_SpellScript::HandleOnHit);
            }

            bool checked;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_divine_purpose_SpellScript();
        }
};

// Cleanse
class spell_pal_cleanse : public SpellScriptLoader
{
    public:
        spell_pal_cleanse() : SpellScriptLoader("spell_pal_cleanse") { }

        class spell_pal_cleanse_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_cleanse_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (caster != target)
                    return;

                if (caster->GetAuraEffect(SPELL_AURA_ADD_PCT_MODIFIER, SPELLFAMILY_PALADIN, 3022, EFFECT_0))
                    for (Unit::AuraApplicationMap::iterator iter = caster->GetAppliedAuras().begin(); iter != caster->GetAppliedAuras().end();)
                    {
                        Aura const* aura = iter->second->GetBase();
                        if (aura->GetSpellInfo()->GetAllEffectsMechanicMask() & (1 << MECHANIC_SNARE | 1 << MECHANIC_ROOT))
                        {
                            caster->RemoveAurasDueToSpellByDispel(aura->GetId(), GetSpellInfo()->Id, aura->GetCasterGUID(), caster, aura->GetStackAmount());
                            break;
                        }
                        ++iter;
                    }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pal_cleanse_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_cleanse_SpellScript();
        }
};

// Inquisition
class spell_pal_inquisition : public SpellScriptLoader
{
    public:
        spell_pal_inquisition() : SpellScriptLoader("spell_pal_inquisition") { }

        class spell_pal_inquisition_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_inquisition_SpellScript);

            void HandleApply(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                int8 holyPower = caster->GetPower(POWER_HOLY_POWER);
                if (caster->HasAura(SPELL_PALADIN_DIVINE_PURPOSE_PROC))
                    holyPower = GetCaster()->GetMaxPower(POWER_HOLY_POWER);
                if (caster->HasAura(90299)) // Paladin T11 Retribution 4P Bonus
                    ++holyPower;
                Aura* aura = GetHitAura();
                aura->SetDuration(aura->GetMaxDuration() * holyPower);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_inquisition_SpellScript::HandleApply, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_inquisition_SpellScript();
        }
};

// 20154 - Seal of Righteousness
class spell_pal_seal_of_righteousness : public SpellScriptLoader
{
    public:
        spell_pal_seal_of_righteousness() : SpellScriptLoader("spell_pal_seal_of_righteousness") { }

        class spell_pal_seal_of_righteousness_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_seal_of_righteousness_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_SEAL_OF_RIGHTEOUSNESS))
                    return false;
                return true;
            }

            bool Load()
            {
                divineStormTargetsCount = 0;
                return true;
            }

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                return eventInfo.GetProcTarget();
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                Unit* caster = GetTarget();

                if (!caster)
                    return;

                if (divineStormTargetsCount > 0) // don't allow proc more than one per target of divine storm
                {
                    divineStormTargetsCount--;
                    return;
                }

                float ap = caster->GetTotalAttackPowerValue(BASE_ATTACK);
                int32 holy = caster->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_HOLY);
                holy += eventInfo.GetProcTarget()->SpellBaseDamageBonusTaken(SPELL_SCHOOL_MASK_HOLY);
                int32 bp = int32((ap * 0.011f + 0.022f * holy) * caster->GetAttackTime(BASE_ATTACK) / 1000);

                if (caster->HasAura(SPELL_PALADIN_SEALS_OF_COMMAND))
                {
                    std::list<Unit*> targets;
                    Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(caster, caster, 5);
                    Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(caster, targets, u_check);
                    caster->VisitNearbyObject(5, searcher);
                    if (!targets.empty())
                        if (eventInfo.GetDamageInfo() && eventInfo.GetDamageInfo()->GetSpellInfo() && eventInfo.GetDamageInfo()->GetSpellInfo()->Id == SPELL_PALADIN_DIVINE_STORM)
                            divineStormTargetsCount = targets.size() - 1;
                    for (std::list<Unit*>::iterator itr = targets.begin(); itr != targets.end(); ++itr)
                        if (Unit* target = (*itr)->ToUnit())
                            if (caster->isInFront(target) && caster->IsWithinMeleeRange(target))
                                caster->CastCustomSpell(SPELL_PALADIN_SEAL_OF_RIGHTEOUSNESS, SPELLVALUE_BASE_POINT0, bp, target, true, NULL, aurEff);
                } else caster->CastCustomSpell(SPELL_PALADIN_SEAL_OF_RIGHTEOUSNESS, SPELLVALUE_BASE_POINT0, bp, eventInfo.GetProcTarget(), true, NULL, aurEff);
            }

        private :
            uint32 divineStormTargetsCount;

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_pal_seal_of_righteousness_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_pal_seal_of_righteousness_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_seal_of_righteousness_AuraScript();
        }
};

class spell_pal_avenging_wrath: public SpellScriptLoader
{
    public:
        spell_pal_avenging_wrath() : SpellScriptLoader("spell_pal_avenging_wrath") { }

        class spell_pal_avenging_wrath_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_avenging_wrath_AuraScript);

            void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                if (Unit* caster = GetCaster())
                    if (caster->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_PALADIN, 3029, EFFECT_0))
                        caster->CastSpell(target, 57318, true);
            }

            void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->RemoveAurasDueToSpell(57318);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_pal_avenging_wrath_AuraScript::HandleEffectApply, EFFECT_2, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_pal_avenging_wrath_AuraScript::HandleEffectRemove, EFFECT_2, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_avenging_wrath_AuraScript();
        }
};

// Exorcism
class spell_pal_exorcism : public SpellScriptLoader
{
   public:
       spell_pal_exorcism() : SpellScriptLoader("spell_pal_exorcism") { }

       class spell_pal_exorcism_SpellScript : public SpellScript
       {
           PrepareSpellScript(spell_pal_exorcism_SpellScript);

           void RecalculateDamage(SpellEffIndex /*effIndex*/)
           {
               Unit* caster = GetCaster();
               float coeff = 0.344f;
               int32 SpellPowerBonus = caster->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask());
               int32 APbonus = caster->GetTotalAttackPowerValue(BASE_ATTACK);
               int32 bonus = std::max(SpellPowerBonus, APbonus);
               SetEffectDamage(GetEffectValue() + (bonus * coeff));
           }

           void HandleDot(SpellEffIndex /*effIndex*/)
           {
               AuraEffect* glyph = GetCaster()->GetAuraEffect(54934, EFFECT_0);
               if (!glyph)
               {
                   PreventHitAura();
                   return;
               }
               uint32 dotDamage = CalculatePct(GetHitDamage(), glyph->GetAmount());
               if (GetHitAura())
                   if (AuraEffect* periodic = GetHitAura()->GetEffect(EFFECT_1))
                       periodic->SetAmount(dotDamage / periodic->GetTotalTicks());
           }

           void Register() override
           {
               OnEffectLaunchTarget += SpellEffectFn(spell_pal_exorcism_SpellScript::RecalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
               OnEffectHitTarget += SpellEffectFn(spell_pal_exorcism_SpellScript::HandleDot, EFFECT_1, SPELL_EFFECT_APPLY_AURA);
           }
       };

       SpellScript* GetSpellScript() const override
       {
           return new spell_pal_exorcism_SpellScript();
       }
};

// Holy wrath
class spell_pal_holy_wrath : public SpellScriptLoader
{
    public:
        spell_pal_holy_wrath() : SpellScriptLoader("spell_pal_holy_wrath") { }

        class spell_pal_holy_wrath_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_holy_wrath_SpellScript);

            bool Load()
            {
                targetCount = 0;
                return true;
            }

            void CountTargets(std::list<WorldObject*>& targets)
            {
                targetCount = targets.size();
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end();)
                    if (Unit* unit = (*itr)->ToUnit())
                    {
                        WorldObject* temp = (*itr);
                        switch (unit->GetCreatureType())
                        {
                            case CREATURE_TYPE_DEMON:
                            case CREATURE_TYPE_UNDEAD:
                                itr++;
                                break;
                            case CREATURE_TYPE_ELEMENTAL:
                            case CREATURE_TYPE_DRAGONKIN:
                            {
                                if (GetCaster()->HasAura(SPELL_GLYPH_OF_HOLY_WRATH))
                                    itr++;
                                else
                                {
                                    itr++;
                                    targets.remove(temp);
                                }
                                break;
                            }
                            default:
                                itr++;
                                targets.remove(temp);
                                break;
                        }
                    }
                    else
                        itr++;
            }

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                SetHitDamage(GetHitDamage() / float(targetCount));
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_holy_wrath_SpellScript::CountTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_holy_wrath_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_pal_holy_wrath_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }

            uint32 targetCount;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_holy_wrath_SpellScript();
        }
};

// Sacred shield
class spell_pal_sacred_shield : public SpellScriptLoader
{
    public:
        spell_pal_sacred_shield() : SpellScriptLoader("spell_pal_sacred_shield") { }

        class spell_pal_sacred_shield_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_sacred_shield_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
            {
                canBeRecalculated = false;
                if (Unit* caster = GetCaster())
                {
                    float bonus = 2.8f * caster->GetTotalAttackPowerValue(BASE_ATTACK);
                    bonus = caster->ApplyEffectModifiers(GetSpellInfo(), aurEff->GetEffIndex(), bonus);
                    amount += int32(bonus);
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_sacred_shield_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_sacred_shield_AuraScript();
        }
};

// 53600 - Shield of the Righteous
 class spell_pal_shield_of_the_righteous : public SpellScriptLoader
 {
     public:
        spell_pal_shield_of_the_righteous() : SpellScriptLoader("spell_pal_shield_of_the_righteous") { }

        class spell_pal_shield_of_the_righteous_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_shield_of_the_righteous_SpellScript);

            bool Load()
            {
                if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    return false;

                if (GetCaster()->ToPlayer()->getClass() != CLASS_PALADIN)
                    return false;

                return true;
            }

            void ChangeDamage(SpellEffIndex /*effIndex*/)
            {
                int32 damage = GetEffectValue();
                int32 hp = GetCaster()->GetPower(POWER_HOLY_POWER);
                float APbonus = GetCaster()->GetTotalAttackPowerValue(BASE_ATTACK);
                int32 mult = 0;

                switch (hp)
                {
                case 2:
                    mult = 3;
                    break;
                case 3:
                    mult = 6;
                    break;
                default:
                    mult = 1;
                    break;
                }
                APbonus = APbonus * mult / 10;

                SetEffectDamage(damage * mult + int32(APbonus) - mult);
            }

            void HandleT12Bonus(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->HasAura(99074)) // Paladin T12 Protection 2P Bonus
                    {
                        int32 damage = CalculatePct(GetHitDamage(), 20);
                        caster->CastCustomSpell(GetHitUnit(), 99075, &damage, NULL, NULL, true);
                    }
                }
            }

            void Register()
            {
                OnEffectLaunchTarget += SpellEffectFn(spell_pal_shield_of_the_righteous_SpellScript::ChangeDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
                OnEffectHitTarget += SpellEffectFn(spell_pal_shield_of_the_righteous_SpellScript::HandleT12Bonus, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_shield_of_the_righteous_SpellScript();
        }
};

 class spell_pal_holy_radiance : public SpellScriptLoader
 {
 public:
     spell_pal_holy_radiance() : SpellScriptLoader("spell_pal_holy_radiance") { }

     class spell_pal_holy_radiance_SpellScript : public SpellScript
     {
         PrepareSpellScript(spell_pal_holy_radiance_SpellScript);

         bool Load()
         {
             _targets = 0;
             return true;
         }

         void HandleHeal(SpellEffIndex /*effIndex*/)
         {
             if (_targets > 6)
                 SetHitHeal(GetHitHeal() * 6 / _targets);
         }

         void FilterTargets(std::list<WorldObject*>& unitList)
         {
             _targets = unitList.size();
         }

         void Register()
         {
             OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_holy_radiance_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ALLY);
             OnEffectHitTarget += SpellEffectFn(spell_pal_holy_radiance_SpellScript::HandleHeal, EFFECT_1, SPELL_EFFECT_HEAL);
         }

         uint32 _targets;
     };

     SpellScript* GetSpellScript() const
     {
         return new spell_pal_holy_radiance_SpellScript();
     }
 };

 class spell_pal_seals_duration : public SpellScriptLoader
 {
 public:
     spell_pal_seals_duration() : SpellScriptLoader("spell_pal_seals_duration") { }

     class spell_pal_seals_duration_SpellScript : public SpellScript
     {
         PrepareSpellScript(spell_pal_seals_duration_SpellScript);

         void HandleApply(SpellEffIndex /*effIndex*/)
         {
             if (Unit* caster = GetCaster())
                 if (caster->GetTypeId() == TYPEID_PLAYER && caster->getClass() != CLASS_PALADIN)
                     if (Aura* aura = GetHitAura())
                         aura->SetDuration(60000);
         }

         void Register()
         {
             OnEffectHitTarget += SpellEffectFn(spell_pal_seals_duration_SpellScript::HandleApply, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
         }
     };

     SpellScript* GetSpellScript() const
     {
         return new spell_pal_seals_duration_SpellScript();
     }
 };

 class spell_pal_aura_mastery : public SpellScriptLoader
 {
 public:
     spell_pal_aura_mastery() : SpellScriptLoader("spell_pal_aura_mastery") { }

     class spell_pal_aura_mastery_AuraScript : public AuraScript
     {
         PrepareAuraScript(spell_pal_aura_mastery_AuraScript);

         void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
         {
             canBeRecalculated = true;
             if (Unit* caster = GetUnitOwner())
             {
                 int32 resist = caster->getLevel();

                 if (resist > 70 && resist < 81)
                     resist += (resist - 70) * 5;
                 else if (resist > 80)
                     resist += ((resist-70) * 5 + (resist - 80) * 7);

                 if (caster->HasAura(31821, caster->GetGUID()))
                     amount = resist * 2;
                 else
                     amount = resist;
             }

         }

         void Register()
         {
             DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_aura_mastery_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE);
         }
     };

     AuraScript* GetAuraScript() const
     {
         return new spell_pal_aura_mastery_AuraScript();
     }
 };

 class spell_pal_divine_protection : public SpellScriptLoader
 {
 public:
     spell_pal_divine_protection() : SpellScriptLoader("spell_pal_divine_protection") { }

     class spell_pal_divine_protection_AuraScript : public AuraScript
     {
         PrepareAuraScript(spell_pal_divine_protection_AuraScript);

         void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
         {
             // Paladin T12 Protection 4P Bonus
             if (Unit* caster = GetCaster())
                 if (caster->HasAura(99091) && caster->isAlive())
                     caster->CastSpell(caster, 99090, true);
         }

         void Register()
         {
             AfterEffectRemove += AuraEffectRemoveFn(spell_pal_divine_protection_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAL);
         }
     };

     AuraScript* GetAuraScript() const
     {
         return new spell_pal_divine_protection_AuraScript();
     }
 };

 class spell_pal_t13_2p_protection : public SpellScriptLoader
 {
 public:
     spell_pal_t13_2p_protection() : SpellScriptLoader("spell_pal_t13_2p_protection") { }

     class spell_pal_t13_2p_protection_AuraScript : public AuraScript
     {
         PrepareAuraScript(spell_pal_t13_2p_protection_AuraScript);

         bool Validate(SpellInfo const* /*spellInfo*/) override
         {
             if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_DELAYED_JUDGEMENT))
                 return false;
             return true;
         }

         void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
         {
             if (Unit* caster = eventInfo.GetActor())
             {
                 int32 absorb = CalculatePct(eventInfo.GetDamageInfo()->GetDamage(), aurEff->GetAmount());
                 caster->CastCustomSpell(caster, SPELL_PALADIN_DELAYED_JUDGEMENT, &absorb, NULL, NULL, true);
             }
         }

         void Register() override
         {
             OnEffectProc += AuraEffectProcFn(spell_pal_t13_2p_protection_AuraScript::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
         }
     };

     AuraScript* GetAuraScript() const override
     {
         return new spell_pal_t13_2p_protection_AuraScript();
     }
 };

 class spell_pal_ancient_healer_procAuraScript : public AuraScript
 {
     PrepareAuraScript(spell_pal_ancient_healer_procAuraScript);

     bool CheckProc(ProcEventInfo &eventInfo)
     {
         SpellInfo const* spellInfo = NULL;
         if (HealInfo *healInfo = eventInfo.GetHealInfo())
             spellInfo = healInfo->GetSpellInfo();

         switch (spellInfo ? spellInfo->Id : 0)
         {
             case SPELL_PALADIN_WORD_OF_GLORY:
             case SPELL_PALADIN_HOLY_SHOCK_R1_HEALING:
             case SPELL_PALADIN_HOLY_LIGHT:
             case SPELL_PALADIN_DIVINE_LIGHT:
             case SPELL_PALADIN_FLASH_OF_LIGHT:
                 return true;
             default:
                 return false;
         }
     }

     void Register() override
     {
         DoCheckProc += AuraCheckProcFn(spell_pal_ancient_healer_procAuraScript::CheckProc);
     }
 };

void AddSC_paladin_spell_scripts()
{
    new spell_pal_ardent_defender(); // Verified
	new spell_pal_blessing_of_faith(); // Verified
	new spell_pal_divine_sacrifice(); // Verified
	new spell_pal_divine_storm(); // Verified
	new spell_pal_divine_storm_dummy(); // Verified
	new spell_pal_exorcism_and_holy_wrath_damage(); // Verified
	new spell_pal_eye_for_an_eye(); // Verified
	new spell_pal_guarded_by_the_light(); // Verified
	new spell_pal_hand_of_sacrifice(); // Verified
	new spell_pal_hand_of_salvation(); // Verified
	new spell_pal_holy_shock(); // Verified
	new spell_pal_lay_on_hands(); // Verified
	new spell_pal_righteous_defense(); // Verified
	new spell_pal_seal_of_righteousness(); // Verified
	new spell_pal_templar_s_verdict(); // Verified
	new spell_pal_judgement(); // Verified
	new spell_pal_lod(); // Verified
	new spell_pal_avenging_wrath(); // Verified
	new spell_pal_inquisition(); // Verified
	new spell_pal_word_of_glory(); // Verified
	new spell_pal_sacred_shield(); // Verified
	new spell_pal_consecration(); // Verified
	new spell_pal_guardian(); // Verified
	new spell_pal_guardian_triggered(); // Verified
	new spell_pal_divine_purpose(); // Verified
	new spell_pal_cleanse(); // Verified
	new spell_pal_exorcism(); // Verified
	new spell_pal_holy_wrath(); // Verified
    new spell_pal_shield_of_the_righteous(); // Verified
    new spell_pal_holy_radiance(); // Recently Verified
    new spell_pal_seals_duration(); // Unsure of spell ids? The pala seals should not be 1 hour, but only 30 minutes?? (Not assigning any script name for now)
    new spell_pal_aura_mastery(); // Recently Verified
    new spell_pal_divine_protection(); // Verified (From a bit recently)
    new spell_pal_t13_2p_protection(); // Recently Verified
    new AuraScriptLoaderEx<spell_pal_ancient_healer_procAuraScript>("spell_pal_ancient_healer_proc"); // Recently Verified
}
