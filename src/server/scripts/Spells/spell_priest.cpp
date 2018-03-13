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
 * Scripts for spells with SPELLFAMILY_PRIEST and SPELLFAMILY_GENERIC spells used by priest players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_pri_".
 */

#include "Player.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "GridNotifiers.h"

enum PriestSpells
{
    SPELL_PRIEST_GLYPH_OF_LIGHTWELL                 = 55673,
    SPELL_PRIEST_GLYPH_OF_PRAYER_OF_HEALING_HEAL    = 56161,
    SPELL_PRIEST_GLYPH_OF_SHADOW                    = 107906,
    SPELL_PRIEST_GUARDIAN_SPIRIT_HEAL               = 48153,
    SPELL_PRIEST_LEAP_OF_FAITH                      = 73325,
    SPELL_PRIEST_LEAP_OF_FAITH_EFFECT               = 92832,
    SPELL_PRIEST_LEAP_OF_FAITH_EFFECT_TRIGGER       = 92833,
    SPELL_PRIEST_LEAP_OF_FAITH_TRIGGERED            = 92572,
    SPELL_PRIEST_PENANCE_R1                         = 47540,
    SPELL_PRIEST_PENANCE_R1_DAMAGE                  = 47758,
    SPELL_PRIEST_PENANCE_R1_HEAL                    = 47757,
    SPELL_PRIEST_REFLECTIVE_SHIELD_R1               = 33201,
    SPELL_PRIEST_REFLECTIVE_SHIELD_TRIGGERED        = 33619,
    SPELL_PRIEST_SHADOWFORM_VISUAL_WITHOUT_GLYPH    = 107903,
    SPELL_PRIEST_SHADOWFORM_VISUAL_WITH_GLYPH       = 107904,
    SPELL_PRIEST_SHADOW_WORD_DEATH                  = 32409,
    SPELL_PRIEST_GLYPH_OF_SPIRIT_TAP_AURA           = 63237,
    SPELL_PRIEST_GLYPH_OF_SPIRIT_TAP_EFFECT         = 81301,
    SPELL_PRIEST_T9_HEALING_2P                      = 67201,
    SPELL_PRIEST_VAMPIRIC_TOUCH_DISPEL              = 64085,
    SPELL_PRIEST_REVELATIONS                        = 88627,
    SPELL_PRIEST_RENEW                              = 139,
    SPELL_PRIEST_SANCTUARY_4YD_DUMMY                = 88667,
    SPELL_PRIEST_SANCTUARY_4YD_HEAL                 = 88668,
    SPELL_PRIEST_SANCTUARY_8YD_DUMMY                = 88685,
    SPELL_PRIEST_SANCTUARY_8YD_HEAL                 = 88686
};

enum PriestSpellIcons
{
    PRIEST_ICON_ID_BORROWED_TIME                    = 2899,
    PRIEST_ICON_ID_PAIN_AND_SUFFERING               = 2874,
};

// 55680 - Glyph of Prayer of Healing
class spell_pri_glyph_of_prayer_of_healing : public SpellScriptLoader
{
    public:
        spell_pri_glyph_of_prayer_of_healing() : SpellScriptLoader("spell_pri_glyph_of_prayer_of_healing") { }

        class spell_pri_glyph_of_prayer_of_healing_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_glyph_of_prayer_of_healing_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_GLYPH_OF_PRAYER_OF_HEALING_HEAL))
                    return false;
                return true;
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                SpellInfo const* triggeredSpellInfo = sSpellMgr->GetSpellInfo(SPELL_PRIEST_GLYPH_OF_PRAYER_OF_HEALING_HEAL);
                int32 heal = int32(CalculatePct(int32(eventInfo.GetHealInfo()->GetHeal()), aurEff->GetAmount()) / triggeredSpellInfo->GetMaxTicks());
                GetTarget()->CastCustomSpell(SPELL_PRIEST_GLYPH_OF_PRAYER_OF_HEALING_HEAL, SPELLVALUE_BASE_POINT0, heal, eventInfo.GetProcTarget(), true, NULL, aurEff);
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_pri_glyph_of_prayer_of_healing_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_glyph_of_prayer_of_healing_AuraScript();
        }
};

// 47788 - Guardian Spirit
class spell_pri_guardian_spirit : public SpellScriptLoader
{
    public:
        spell_pri_guardian_spirit() : SpellScriptLoader("spell_pri_guardian_spirit") { }

        class spell_pri_guardian_spirit_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_guardian_spirit_AuraScript);

            uint32 healPct;

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_GUARDIAN_SPIRIT_HEAL))
                    return false;
                return true;
            }

            bool Load()
            {
                healPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue();
                return true;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                Unit* target = GetTarget();
                if (dmgInfo.GetDamage() < target->GetHealth())
                    return;

                int32 healAmount = int32(target->CountPctFromMaxHealth(healPct));
                // remove the aura now, we don't want 40% healing bonus
                Remove(AURA_REMOVE_BY_ENEMY_SPELL);
                target->CastCustomSpell(target, SPELL_PRIEST_GUARDIAN_SPIRIT_HEAL, &healAmount, NULL, NULL, true);
                absorbAmount = dmgInfo.GetDamage();
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_guardian_spirit_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_pri_guardian_spirit_AuraScript::Absorb, EFFECT_1);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_guardian_spirit_AuraScript();
        }
};

// Cure Disease
class spell_pri_cure_disease : public SpellScriptLoader
{
    public:
        spell_pri_cure_disease() : SpellScriptLoader("spell_pri_cure_disease") { }

        class spell_pri_cure_disease_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_cure_disease_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                if (GetHitUnit() != caster)
                    return;

                // Body and soul
                if (AuraEffect* bodyAndSoul = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 2218, EFFECT_1))
                    if (roll_chance_i(bodyAndSoul->GetAmount()))
                        caster->CastSpell(caster, 64136, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_cure_disease_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_cure_disease_SpellScript();
        }
};

// 92833 - Leap of Faith
class spell_pri_leap_of_faith_effect_trigger : public SpellScriptLoader
{
    public:
        spell_pri_leap_of_faith_effect_trigger() : SpellScriptLoader("spell_pri_leap_of_faith_effect_trigger") { }

        class spell_pri_leap_of_faith_effect_trigger_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_leap_of_faith_effect_trigger_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_LEAP_OF_FAITH_EFFECT))
                    return false;
                return true;
            }

            void HandleEffectDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Position destPos;
                GetHitDest()->GetPosition(&destPos);

                // Body and soul
                if (AuraEffect* bodyAndSoul = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 2218, EFFECT_0))
                {
                    switch (bodyAndSoul->GetAmount())
                    {
                        case 30:
                            caster->CastSpell(GetHitUnit(), 64128, true);
                            break;
                        case 60:
                            caster->CastSpell(GetHitUnit(), 65081, true);
                            break;
                    }
                }

                SpellCastTargets targets;
                targets.SetDst(destPos);
                targets.SetUnitTarget(caster);
                GetHitUnit()->CastSpell(targets, sSpellMgr->GetSpellInfo(GetEffectValue()), NULL);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pri_leap_of_faith_effect_trigger_SpellScript::HandleEffectDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_leap_of_faith_effect_trigger_SpellScript();
        }
};

// -7001 - Lightwell Renew
class spell_pri_lightwell_renew : public SpellScriptLoader
{
    public:
        spell_pri_lightwell_renew() : SpellScriptLoader("spell_pri_lightwell_renew") { }

        class spell_pri_lightwell_renew_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_lightwell_renew_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    amount = int32(((GetSpellInfo()->Effects[EFFECT_0].CalcValue(caster) + ((caster->SpellBaseHealingBonusDone(GetSpellInfo()->GetSchoolMask()) * 0.308f))) * 1.15f));
                    // Bonus from Glyph of Lightwell
                    if (AuraEffect* modHealing = caster->GetAuraEffect(SPELL_PRIEST_GLYPH_OF_LIGHTWELL, EFFECT_0))
                        AddPct(amount, modHealing->GetAmount());
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_lightwell_renew_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_lightwell_renew_AuraScript();
        }
};

// 8129 - Mana Burn
class spell_pri_mana_burn : public SpellScriptLoader
{
    public:
        spell_pri_mana_burn() : SpellScriptLoader("spell_pri_mana_burn") { }

        class spell_pri_mana_burn_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_mana_burn_SpellScript);

            void HandleAfterHit()
            {
                if (Unit* unitTarget = GetHitUnit())
                    unitTarget->RemoveAurasWithMechanic((1 << MECHANIC_FEAR) | (1 << MECHANIC_POLYMORPH));
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_pri_mana_burn_SpellScript::HandleAfterHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_mana_burn_SpellScript;
        }
};

// 48045 Mind Sear
class spell_pri_mind_sear_spell : public SpellScriptLoader
{
    public:
        spell_pri_mind_sear_spell() : SpellScriptLoader("spell_pri_mind_sear_spell") { }

        class spell_pri_mind_sear_spell_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_mind_sear_spell_SpellScript);

            SpellCastResult CheckCast()
            {
                if (GetCaster() == GetExplTargetUnit())
                    return SPELL_FAILED_BAD_TARGETS;

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_pri_mind_sear_spell_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_mind_sear_spell_SpellScript();
        }
};

// 47948 - Pain and Suffering (Proc)
class spell_pri_pain_and_suffering_proc : public SpellScriptLoader
{
    public:
        spell_pri_pain_and_suffering_proc() : SpellScriptLoader("spell_pri_pain_and_suffering_proc") { }

        class spell_pri_pain_and_suffering_proc_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_pain_and_suffering_proc_SpellScript);

            void HandleEffectScriptEffect(SpellEffIndex /*effIndex*/)
            {
                // Refresh Shadow Word: Pain on target
                if (Unit* unitTarget = GetHitUnit())
                    if (AuraEffect* aur = unitTarget->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_PRIEST, 0x8000, 0, 0, GetCaster()->GetGUID()))
                    {
                        aur->GetBase()->RefreshDuration();
                        if (Unit *caster = GetCaster())
                            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                                if (aur->GetBase()->HasEffect(i))
                                    aur->GetBase()->GetEffect(i)->CalculatePeriodic(caster, false, false);
                    }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pri_pain_and_suffering_proc_SpellScript::HandleEffectScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_pain_and_suffering_proc_SpellScript;
        }
};

// 4.3.4 updated
class spell_pri_mind_blast : public SpellScriptLoader
{
    public:
        spell_pri_mind_blast() : SpellScriptLoader("spell_pri_mind_blast") { }

        class spell_pri_mind_blast_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_mind_blast_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                {
                    if (Aura* aur = caster->GetAuraOfRankedSpell(15273, caster->GetGUID()))
                    {
                        int32 chance = aur->GetSpellInfo()->Effects[EFFECT_1].BasePoints;
                        if (caster->GetShapeshiftForm() == FORM_SHADOW && roll_chance_i(chance))
                            caster->CastSpell(unitTarget, 48301, true);
                    }
                    // Remove Mind spike debuff
                    unitTarget->RemoveAurasDueToSpell(87178, caster->GetGUID());
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_mind_blast_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_mind_blast_SpellScript;
        }
};

// 4.3.4 updated
class spell_pri_mind_spike : public SpellScriptLoader
{
    public:
        spell_pri_mind_spike() : SpellScriptLoader("spell_pri_mind_spike") { }

        class spell_pri_mind_spike_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_mind_spike_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                {
                    unitTarget->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE, caster->GetGUID(), NULL, true, false, SPELLFAMILY_PRIEST, SPELL_SCHOOL_MASK_SHADOW);
                    unitTarget->RemoveAurasByType(SPELL_AURA_PERIODIC_LEECH, caster->GetGUID(), NULL, true, false, SPELLFAMILY_PRIEST, SPELL_SCHOOL_MASK_SHADOW);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pri_mind_spike_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_mind_spike_SpellScript;
        }
};

// 4.3.4 updated
class spell_pri_fade : public SpellScriptLoader
{
    public:
        spell_pri_fade() : SpellScriptLoader("spell_pri_fade") { }

        class spell_pri_fade_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_fade_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                if (AuraEffect* aur = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 2901, EFFECT_0))
                    if (roll_chance_i(aur->GetBaseAmount()))
                        caster->RemoveMovementImpairingAuras();
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_fade_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_fade_SpellScript;
        }
};

// 47540 - Penance
class spell_pri_penance : public SpellScriptLoader
{
    public:
        spell_pri_penance() : SpellScriptLoader("spell_pri_penance") { }

        class spell_pri_penance_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_penance_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* spellInfo)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_PENANCE_R1))
                    return false;
                // can't use other spell than this penance due to spell_ranks dependency
                if (sSpellMgr->GetFirstSpellInChain(SPELL_PRIEST_PENANCE_R1) != sSpellMgr->GetFirstSpellInChain(spellInfo->Id))
                    return false;

                uint8 rank = sSpellMgr->GetSpellRank(spellInfo->Id);
                if (!sSpellMgr->GetSpellWithRank(SPELL_PRIEST_PENANCE_R1_DAMAGE, rank, true))
                    return false;
                if (!sSpellMgr->GetSpellWithRank(SPELL_PRIEST_PENANCE_R1_HEAL, rank, true))
                    return false;

                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                {
                    if (!unitTarget->isAlive())
                        return;

                    uint8 rank = sSpellMgr->GetSpellRank(GetSpellInfo()->Id);

                    if (caster->IsFriendlyTo(unitTarget))
                        caster->CastSpell(unitTarget, sSpellMgr->GetSpellWithRank(SPELL_PRIEST_PENANCE_R1_HEAL, rank), false, 0);
                    else
                        caster->CastSpell(unitTarget, sSpellMgr->GetSpellWithRank(SPELL_PRIEST_PENANCE_R1_DAMAGE, rank), false, 0);

                    if (caster->HasAura(89911)) // Item - Priest T11 Healer 4P Bonus
                        caster->CastSpell(caster, 89913, true);
                }
            }

            SpellCastResult CheckCast()
            {
                Player* caster = GetCaster()->ToPlayer();
                if (Unit* target = GetExplTargetUnit())
                    if (!caster->IsFriendlyTo(target) && !caster->IsValidAttackTarget(target) || caster->IsFriendlyTo(target) && !caster->IsValidAssistTarget(target))
                        return SPELL_FAILED_BAD_TARGETS;
                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pri_penance_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
                OnCheckCast += SpellCheckCastFn(spell_pri_penance_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_penance_SpellScript;
        }
};

// -17 - Power Word: Shield
class spell_pri_power_word_shield : public SpellScriptLoader
{
    public:
        spell_pri_power_word_shield() : SpellScriptLoader("spell_pri_power_word_shield") { }

        class spell_pri_power_word_shield_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_power_word_shield_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_REFLECTIVE_SHIELD_TRIGGERED))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_REFLECTIVE_SHIELD_R1))
                    return false;
                return true;
            }

            void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
            {
                canBeRecalculated = false;

                if (!GetCaster())
                    return;

                if (Player* caster = GetCaster()->ToPlayer())
                {
                    // +80.68% from sp bonus
                    float bonus = 0.87f;
                    bonus *= caster->SpellBaseHealingBonusDone(GetSpellInfo()->GetSchoolMask());

                    // Improved PW: Shield: its weird having a SPELLMOD_ALL_EFFECTS here but its blizzards doing :)
                    // Improved PW: Shield is only applied at the spell healing bonus because it was already applied to the base value in CalculateSpellDamage
                    bonus = caster->ApplyEffectModifiers(GetSpellInfo(), aurEff->GetEffIndex(), bonus);
                    bonus *= caster->CalculateLevelPenalty(GetSpellInfo());

                    amount += int32(bonus);

                    float tmpMod = 0.0f;
                    // Improved Power Word: Shield
                    if (AuraEffect const* impShield = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 566, EFFECT_0))
                        tmpMod += impShield->GetAmount();

                    // Twin Disciplines
                    if (AuraEffect const* twinDisciplines = caster->GetAuraEffect(SPELL_AURA_ADD_PCT_MODIFIER, SPELLFAMILY_PRIEST, 0x400000, 0, 0, GetCasterGUID()))
                        tmpMod += twinDisciplines->GetAmount();

                    // Shield discipline
                    if (AuraEffect const* mastery = caster->GetDummyAuraEffect(SPELLFAMILY_HUNTER, 566, EFFECT_0))
                        tmpMod += caster->GetMasteryAmount(77484, EFFECT_0);

                    if (Unit* victim = GetUnitOwner())
                    {
                        Unit::AuraEffectList const& mHealingDonePctWithAura = caster->GetAuraEffectsByType(SPELL_AURA_MOD_HEALING_DONE_PERCENT_WITH_SPECIFIC_AURA);
                        for (Unit::AuraEffectList::const_iterator i = mHealingDonePctWithAura.begin(); i != mHealingDonePctWithAura.end(); ++i)
                            if (victim->HasAura(99252) && (*i)->GetSpellInfo()->Id == 99263)
                                tmpMod += (*i)->GetAmount();
                    }

                    if (tmpMod)
                        AddPct(amount, tmpMod);

                    // Focused Power
                    amount *= caster->GetTotalAuraMultiplier(SPELL_AURA_MOD_HEALING_DONE_PERCENT);

                    // Priest T13 Healer 4P Bonus (Holy Word and Power Word: Shield)
                    if (AuraEffect* t13_4p = caster->GetAuraEffect(105832, EFFECT_0, GetCasterGUID()))
                        if (roll_chance_i(t13_4p->GetAmount()))
                            amount *= 2;
                }
            }

            void ReflectDamage(AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& absorbAmount)
            {
                Unit* target = GetTarget();
                if (dmgInfo.GetAttacker() == target)
                    return;

                if (Unit* caster = GetCaster())
                    if (AuraEffect* talentAurEff = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 4880, EFFECT_0))
                    {
                        int32 bp = CalculatePct(absorbAmount, talentAurEff->GetAmount());
                        target->CastCustomSpell(dmgInfo.GetAttacker(), SPELL_PRIEST_REFLECTIVE_SHIELD_TRIGGERED, &bp, NULL, NULL, true, NULL, aurEff);
                    }
            }

            void HandleApplyEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    // Body and soul
                    if (AuraEffect* bodyAndSoul = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 2218, EFFECT_0))
                    {
                        switch (bodyAndSoul->GetAmount())
                        {
                            case 30:
                                caster->CastSpell(GetTarget(), 64128, true);
                                break;
                            case 60:
                                caster->CastSpell(GetTarget(), 65081, true);
                                break;
                        }
                    }

                    if (caster == GetTarget())
                        // PvP 4 piece - Holy walk
                        if (caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 5338, EFFECT_0))
                            caster->CastSpell(caster, 96219, true);
                }
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_pri_power_word_shield_AuraScript::HandleApplyEffect, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_power_word_shield_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                AfterEffectAbsorb += AuraEffectAbsorbFn(spell_pri_power_word_shield_AuraScript::ReflectDamage, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_power_word_shield_AuraScript();
        }
};

// 33110 - Prayer of Mending Heal
class spell_pri_prayer_of_mending_heal : public SpellScriptLoader
{
    public:
        spell_pri_prayer_of_mending_heal() : SpellScriptLoader("spell_pri_prayer_of_mending_heal") { }

        class spell_pri_prayer_of_mending_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_prayer_of_mending_heal_SpellScript);

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetOriginalCaster())
                {
                    if (Aura* prayer = GetHitUnit()->GetAura(41635, caster->GetGUID()))
                        if (prayer->GetCharges() == prayer->CalcMaxCharges())
                        {
                            // Glyph of prayer of mending
                            if (AuraEffect* glyph = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 2219, EFFECT_0))
                            {
                                int32 heal = GetHitHeal();
                                AddPct(heal, glyph->GetAmount());
                                SetHitHeal(heal);
                            }
                        }

                    if (AuraEffect* aurEff = caster->GetAuraEffect(SPELL_PRIEST_T9_HEALING_2P, EFFECT_0))
                    {
                        int32 heal = GetHitHeal();
                        AddPct(heal, aurEff->GetAmount());
                        SetHitHeal(heal);
                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pri_prayer_of_mending_heal_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_prayer_of_mending_heal_SpellScript();
        }
};

// 139 - Renew
class spell_pri_renew : public SpellScriptLoader
{
    public:
        spell_pri_renew() : SpellScriptLoader("spell_pri_renew") { }

        class spell_pri_renew_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_renew_AuraScript);

            bool Load() override
            {
                Unit* caster = GetCaster();
                return caster && caster->GetTypeId() == TYPEID_PLAYER;
            }

            void HandleApplyEffect(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    // Divine touch
                    if (AuraEffect const* divineTouch = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 3021, EFFECT_0))
                    {
                        int32 basepoints0 = CalculatePct((aurEff->GetAmount() * aurEff->GetTotalTicks()), divineTouch->GetAmount());
                        caster->CastCustomSpell(GetTarget(), 63544, &basepoints0, NULL, NULL, true, NULL, divineTouch);
                    }
                }
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectApplyFn(spell_pri_renew_AuraScript::HandleApplyEffect, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_renew_AuraScript();
        }
};

// Inner Fire
class spell_pri_inner_fire : public SpellScriptLoader
{
    public:
        spell_pri_inner_fire() : SpellScriptLoader("spell_pri_inner_fire") { }

        class spell_pri_inner_fire_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_inner_fire_AuraScript);

            void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                if (AuraEffect* aurEff = target->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 51, EFFECT_0))
                    target->CastCustomSpell(91724, SPELLVALUE_BASE_POINT0, -aurEff->GetAmount(), target, true);
            }

            void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->RemoveAurasDueToSpell(91724);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_pri_inner_fire_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_MOD_RESISTANCE_PCT, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_pri_inner_fire_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_MOD_RESISTANCE_PCT, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_inner_fire_AuraScript();
        }
};

// 32379 - Shadow Word Death
class spell_pri_shadow_word_death : public SpellScriptLoader
{
    public:
        spell_pri_shadow_word_death() : SpellScriptLoader("spell_pri_shadow_word_death") { }

        class spell_pri_shadow_word_death_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_shadow_word_death_SpellScript);

            void HandleDamage()
            {
                Unit* caster = GetCaster();
                if (caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                Unit* target = GetHitUnit();
                int32 damage = GetTrueDamage();
                int32 targetHealth = target->GetHealth();
                // Glyph cooldown reset when target was failed to kill
                if (AuraEffect* glyph = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 1980, EFFECT_0))
                {
                    if (target && target->GetHealthPct() <= glyph->GetAmount() && !caster->HasAura(95652))
                    {
                        if (targetHealth > damage)
                        {
                            caster->ToPlayer()->RemoveSpellCooldown(32379, true);
                            caster->CastSpell(caster, 95652, true);
                        }
                    }
                }
                // Glyph of Spirit Tap
                if (caster->HasAura(SPELL_PRIEST_GLYPH_OF_SPIRIT_TAP_AURA) && damage > targetHealth && caster->ToPlayer()->isHonorOrXPTarget(target))
                    caster->CastSpell(caster, SPELL_PRIEST_GLYPH_OF_SPIRIT_TAP_EFFECT, true);

                // Pain and Suffering reduces damage
                if (AuraEffect* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, PRIEST_ICON_ID_PAIN_AND_SUFFERING, EFFECT_1))
                    AddPct(damage, aurEff->GetAmount());

                if (damage < targetHealth)
                { 
                    // Priest T13 Shadow 2P Bonus (Shadow Word: Death)
                    if (caster->HasAura(105843, caster->GetGUID()))
                        damage = CalculatePct(damage, 5);
                    caster->CastCustomSpell(caster, SPELL_PRIEST_SHADOW_WORD_DEATH, &damage, 0, 0, true);
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_pri_shadow_word_death_SpellScript::HandleDamage);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_shadow_word_death_SpellScript();
        }
};

// 15473 - Shadowform
class spell_pri_shadowform : public SpellScriptLoader
{
    public:
        spell_pri_shadowform() : SpellScriptLoader("spell_pri_shadowform") { }

        class spell_pri_shadowform_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_shadowform_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PRIEST_SHADOWFORM_VISUAL_WITHOUT_GLYPH) ||
                    !sSpellMgr->GetSpellInfo(SPELL_PRIEST_SHADOWFORM_VISUAL_WITH_GLYPH))
                    return false;
                return true;
            }

            void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->CastSpell(GetTarget(), GetTarget()->HasAura(SPELL_PRIEST_GLYPH_OF_SHADOW) ? SPELL_PRIEST_SHADOWFORM_VISUAL_WITH_GLYPH : SPELL_PRIEST_SHADOWFORM_VISUAL_WITHOUT_GLYPH, true);
            }

            void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->RemoveAurasDueToSpell(SPELL_PRIEST_SHADOWFORM_VISUAL_WITH_GLYPH);
                GetTarget()->RemoveAurasDueToSpell(SPELL_PRIEST_SHADOWFORM_VISUAL_WITHOUT_GLYPH);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_pri_shadowform_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_pri_shadowform_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_shadowform_AuraScript();
        }
};

// 34914 - Vampiric Touch
class spell_pri_vampiric_touch : public SpellScriptLoader
{
    public:
        spell_pri_vampiric_touch() : SpellScriptLoader("spell_pri_vampiric_touch") { }

        class spell_pri_vampiric_touch_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_vampiric_touch_AuraScript);

            void HandleDispel(DispelInfo* dispelInfo)
            {
                if (Unit* caster = GetCaster())
                    if (GetUnitOwner())
                        if (AuraEffect const* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 1869, EFFECT_1))
                            if (roll_chance_i(aurEff->GetSpellInfo()->Effects[EFFECT_0].BasePoints))
                                if (Unit* dispeller = dispelInfo->GetDispeller())
                                    dispeller->CastSpell(dispeller, 87204, true);
            }

            void Register()
            {
                AfterDispel += AuraDispelFn(spell_pri_vampiric_touch_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_vampiric_touch_AuraScript();
        }
};

// Shadow orbs
class spell_pri_shadow_orbs : public SpellScriptLoader
{
    public:
        spell_pri_shadow_orbs() : SpellScriptLoader("spell_pri_shadow_orbs") { }

        class spell_pri_shadow_orbs_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_shadow_orbs_AuraScript);

            void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetStackAmount() == GetSpellInfo()->StackAmount)
                    GetTarget()->CastSpell(GetTarget(), 93683, true);
            }

            void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->RemoveAurasDueToSpell(93683);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_pri_shadow_orbs_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_pri_shadow_orbs_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_shadow_orbs_AuraScript();
        }
};

// 527 - Dispel Magic
class spell_pri_dispel_magic : public SpellScriptLoader
{
    public:
        spell_pri_dispel_magic() : SpellScriptLoader("spell_pri_dispel_magic") { }

        class spell_pri_dispel_magic_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_dispel_magic_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                {
                    if (caster->IsFriendlyTo(unitTarget))
                        caster->CastSpell(unitTarget, 97690, true);
                    else
                        caster->CastSpell(unitTarget, 97691, true);
                }
            }

            SpellCastResult CheckCast()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetExplTargetUnit())
                        if (caster->IsFriendlyTo(target) && caster != target)
                            if (!caster->HasAura(33167))
                                return SPELL_FAILED_BAD_TARGETS;
                }

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_pri_dispel_magic_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_pri_dispel_magic_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_dispel_magic_SpellScript();
        }
};

// 89485 - Inner Focus
class spell_pri_inner_focus : public SpellScriptLoader
{
    public:
        spell_pri_inner_focus() : SpellScriptLoader("spell_pri_inner_focus") { }

        class spell_pri_inner_focus_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_inner_focus_SpellScript);

            void HandleOnCast()
            {
                Unit* caster = GetCaster();
                if (AuraEffect* aurEff = caster->GetAuraEffectOfRankedSpell(89488, EFFECT_0))
                {
                    uint32 spellId = 0;
                    switch (aurEff->GetId())
                    {
                        case 89488:
                            spellId = 96266;
                            break;
                        case 89489:
                            spellId = 96267;
                            break;
                    }
                    caster->CastSpell(caster, spellId, true);
                }
            }

            void Register()
            {
                OnCast += SpellCastFn(spell_pri_inner_focus_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_inner_focus_SpellScript();
        }
};

// 94472, 81751 - Atonement heal
class spell_pri_atonement_heal : public SpellScriptLoader
{
    public:
        spell_pri_atonement_heal() : SpellScriptLoader("spell_pri_atonement_heal") { }

        class spell_pri_atonement_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_atonement_heal_SpellScript);

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetOriginalCaster())
                    if (GetHitUnit()->GetGUID() == caster->GetGUID())
                    {
                        int32 heal = GetHitHeal() / 2.0f;
                        SetHitHeal(heal);
                    }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pri_atonement_heal_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_atonement_heal_SpellScript();
        }
};

// 81781 Power Word: Barrier
class spell_pri_power_word_barrier: public SpellScriptLoader
{
public:
    spell_pri_power_word_barrier() : SpellScriptLoader("spell_pri_power_word_barrier") {}

    class spell_pri_power_word_barrier_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_power_word_barrier_AuraScript);

        bool Load() override
        {
            if (Unit* caster = GetCaster())
                if (Unit* owner = caster->GetOwner())
                    if (owner->GetTypeId() == TYPEID_PLAYER && owner->HasAura(55689))
                        return false;

            return true;
        }

        void HandlePeriodicTriggerSpell(AuraEffect const* /*aurEff*/)
        {
            PreventDefaultAction();
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_power_word_barrier_AuraScript::HandlePeriodicTriggerSpell, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pri_power_word_barrier_AuraScript();
    }
};

// 81208, 81206 Chakra: Serenity and Chakra: Sanctuary spell swap supressor
class spell_pri_chakra_swap_supressor: public SpellScriptLoader
{
public:
    spell_pri_chakra_swap_supressor() : SpellScriptLoader("spell_pri_chakra_swap_supressor") {}

    class spell_pri_chakra_swap_supressor_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pri_chakra_swap_supressor_SpellScript);

        void PreventSwapApplicationOnCaster(WorldObject*& target)
        {
            // If the caster has the Revelations talent (88627) The chakra: serenity aura (81208) and the chakra: sanctuary
            // (81206) swaps the Holy Word: Chastise spell (the one that you learn when you spec into the holy tree)
            // for a Holy Word: Serenity spell (88684) or a Holy Word: Sanctuary (88684), if the caster doesnt have the
            // talent, lets just block the swap effect.
            if (!GetCaster()->HasAura(SPELL_PRIEST_REVELATIONS))
                target = NULL;
        }

        void Register()
        {
            OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_pri_chakra_swap_supressor_SpellScript::PreventSwapApplicationOnCaster, EFFECT_2, TARGET_UNIT_CASTER);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_pri_chakra_swap_supressor_SpellScript();
    }
};

// 81585 Chakra: Serenity, Renew spell duration reset
class spell_pri_chakra_serenity_proc : public SpellScriptLoader
{
public:
    spell_pri_chakra_serenity_proc() : SpellScriptLoader("spell_pri_chakra_serenity_proc") {}

    class spell_pri_chakra_serenity_proc_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pri_chakra_serenity_proc_SpellScript);

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            Unit* target = GetHitUnit();

            if (!target)
                return;

            if (Aura* renew = target->GetAura(SPELL_PRIEST_RENEW, GetCaster()->GetGUID()))
            {
                renew->RefreshDuration();
                if (Unit *caster = GetCaster())
                    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                        if (renew->HasEffect(i))
                            renew->GetEffect(i)->CalculatePeriodic(caster, false, false);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_pri_chakra_serenity_proc_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_pri_chakra_serenity_proc_SpellScript();
    }
};

// 88685, 88687 Chakra: Sanctuary GTAoe effect
class spell_pri_chakra_sanctuary_heal: public SpellScriptLoader
{
public:
    spell_pri_chakra_sanctuary_heal() : SpellScriptLoader("spell_pri_chakra_sanctuary_heal") {}

    class spell_pri_chakra_sanctuary_heal_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pri_chakra_sanctuary_heal_SpellScript);

        float x, y, z;

        bool Load()
        {
            if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                return false;

            GetExplTargetDest()->GetPosition(x, y, z);
            return true;
        }

        void HandleExtraEffect()
        {
            if (GetSpellInfo()->Id == SPELL_PRIEST_SANCTUARY_8YD_DUMMY)
                GetCaster()->CastSpell(x, y, z, SPELL_PRIEST_SANCTUARY_4YD_DUMMY, true);
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_pri_chakra_sanctuary_heal_SpellScript::HandleExtraEffect);
        }
    };

    class spell_pri_chakra_sanctuary_heal_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_chakra_sanctuary_heal_AuraScript);

        bool Load()
        {
            if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                return false;

            return true;
        }

        void HandlePeriodicDummy(AuraEffect const* /*aurEff*/)
        {
            Unit* caster = GetCaster();
            DynamicObject* dynObj = caster->GetDynObject(GetId());

            if (caster && dynObj && caster->GetMapId() == dynObj->GetMapId())
            {
                float x, y, z;
                dynObj->GetPosition(x, y, z);

                switch(GetSpellInfo()->Id)
                {
                    case SPELL_PRIEST_SANCTUARY_8YD_DUMMY:
                    {
                        caster->CastSpell(x, y, z, SPELL_PRIEST_SANCTUARY_8YD_HEAL, true);
                        break;
                    }
                    case SPELL_PRIEST_SANCTUARY_4YD_DUMMY:
                    {
                        caster->CastSpell(x, y, z, SPELL_PRIEST_SANCTUARY_4YD_HEAL, true);
                        break;
                    }
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_chakra_sanctuary_heal_AuraScript::HandlePeriodicDummy, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_pri_chakra_sanctuary_heal_SpellScript();
    }

    AuraScript* GetAuraScript() const
    {
        return new spell_pri_chakra_sanctuary_heal_AuraScript();
    }
};

class spell_pri_pw_sanctuary_heal : public SpellScriptLoader
{
public:
    spell_pri_pw_sanctuary_heal() : SpellScriptLoader("spell_pri_pw_sanctuary_heal") { }

    class spell_pri_pw_sanctuary_heal_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pri_pw_sanctuary_heal_SpellScript);

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
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pri_pw_sanctuary_heal_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
            OnEffectHitTarget += SpellEffectFn(spell_pri_pw_sanctuary_heal_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
        }

        uint32 _targets;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_pri_pw_sanctuary_heal_SpellScript();
    }
};

// Psychic scream
class spell_pri_fear: public SpellScriptLoader
{
    public:
        spell_pri_fear() : SpellScriptLoader("spell_pri_fear") { }

        class spell_pri_fear_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_fear_SpellScript);

            void HandleOnHit()
            {
                if (!GetHitUnit())
                    return;

                AuraEffect* glyph = GetCaster()->GetAuraEffect(55676, EFFECT_0);
                if (Aura* debuff = GetHitAura())
                {
                    if (debuff->GetApplicationOfTarget(GetHitUnit()->GetGUID()))
                    {
                        // Remove root effect
                        if (AuraEffect* root = debuff->GetEffect(EFFECT_2))
                            if (!glyph)
                                root->HandleEffect(GetHitUnit(), AURA_REMOVE_BY_DEFAULT, false);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_fear_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_fear_SpellScript();
        }
};

// Friendly Dispel magic
class spell_pri_friendly_dispel: public SpellScriptLoader
{
    public:
        spell_pri_friendly_dispel() : SpellScriptLoader("spell_pri_friendly_dispel") { }

        class spell_pri_friendly_dispel_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_friendly_dispel_SpellScript);

            void HandleOnDispel()
            {
                // Glyph of dispel magic
                if (AuraEffect* aura = GetCaster()->GetAuraEffect(55677, EFFECT_0))
                {
                    int32 bp0 = int32(GetHitUnit()->CountPctFromMaxHealth(aura->GetAmount()));
                    GetCaster()->CastCustomSpell(GetHitUnit(), 56131, &bp0, NULL, NULL, true);
                }
            }

            void Register()
            {
                OnSuccessfulDispel += SpellDispelFn(spell_pri_friendly_dispel_SpellScript::HandleOnDispel);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_friendly_dispel_SpellScript();
        }
};

class spell_pri_blessed_resilience : public SpellScriptLoader
{
public:
    spell_pri_blessed_resilience() : SpellScriptLoader("spell_pri_blessed_resilience") { }

    class spell_pri_blessed_resilience_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_blessed_resilience_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            if (eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->HasAura(SPELL_AURA_PERIODIC_DAMAGE))
                return false;

            float damage = eventInfo.GetDamageInfo()->GetDamage();
            if (damage > GetCaster()->CountPctFromMaxHealth(10) || eventInfo.GetHitMask() & PROC_HIT_CRITICAL)
                return true;
            return false;
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(spell_pri_blessed_resilience_AuraScript::CheckProc);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_pri_blessed_resilience_AuraScript();
    }
};

class spell_pri_chakra : public SpellScriptLoader
{
public:
    spell_pri_chakra() : SpellScriptLoader("spell_pri_chakra") { }

    class spell_pri_chakra_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_chakra_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget()->HasAura(89911)) // Priest T11 Healer 4P Bonus
                GetTarget()->CastSpell(GetTarget(), 89912, TRIGGERED_FULL_MASK);
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            GetTarget()->RemoveAurasDueToSpell(89912);
            GetTarget()->RemoveAurasDueToSpell(81207);
        }

        void Register()
        {
            if (m_scriptSpellId == 81206)
            {
                OnEffectApply += AuraEffectApplyFn(spell_pri_chakra_AuraScript::OnApply, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_pri_chakra_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
            } else if (m_scriptSpellId == 81208)
            {
                OnEffectApply += AuraEffectApplyFn(spell_pri_chakra_AuraScript::OnApply, EFFECT_0, SPELL_AURA_ADD_FLAT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_pri_chakra_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_ADD_FLAT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
            } else if (m_scriptSpellId == 81209)
            {
                OnEffectApply += AuraEffectApplyFn(spell_pri_chakra_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_pri_chakra_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
            }
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_pri_chakra_AuraScript();
    }
};

class spell_priest_spirit_of_redemption : public SpellScriptLoader
{
public:
    spell_priest_spirit_of_redemption() : SpellScriptLoader("spell_priest_spirit_of_redemption") {}

    class spell_priest_spirit_of_redemption_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_priest_spirit_of_redemption_AuraScript);

        bool Load()
        {
            Unit* caster = GetCaster();
            return caster && caster->GetTypeId() == TYPEID_PLAYER;
        }

        void HandleApplyEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                caster->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                caster->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
            }
        }

        void HandleRemoveEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                caster->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                caster->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
            }
        }

        void Register() override
        {
            OnEffectApply += AuraEffectApplyFn(spell_priest_spirit_of_redemption_AuraScript::HandleApplyEffect, EFFECT_1, SPELL_AURA_WATER_BREATHING, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            OnEffectRemove += AuraEffectRemoveFn(spell_priest_spirit_of_redemption_AuraScript::HandleRemoveEffect, EFFECT_1, SPELL_AURA_WATER_BREATHING, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_priest_spirit_of_redemption_AuraScript();
    }
};

class spell_pri_holy_nova : public SpellScriptLoader
{
public:
    spell_pri_holy_nova() : SpellScriptLoader("spell_pri_holy_nova") { }

    class spell_pri_holy_nova_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pri_holy_nova_SpellScript);

        bool Load()
        {
            _targets = 0;
            return true;
        }

        void HandleHeal(SpellEffIndex /*effIndex*/)
        {
            // Healing is divided among the number of targets healed.  These effects cause no threat.
            SetHitHeal(GetHitHeal() / _targets);
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            _targets = unitList.size();
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pri_holy_nova_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
            OnEffectHitTarget += SpellEffectFn(spell_pri_holy_nova_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
        }

        uint32 _targets;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_pri_holy_nova_SpellScript();
    }
};

#define EVENT_CHECK_DOTS    1
class spell_pri_devouring_plague : public SpellScriptLoader
{
public:
    spell_pri_devouring_plague() : SpellScriptLoader("spell_pri_devouring_plague") { }

    class spell_pri_devouring_plague_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_devouring_plague_AuraScript);

        bool Load()
        {   // Priest T12 Shadow 4P Bonus
            if (GetCaster() && GetCaster()->HasAura(99157))
            {
                events.ScheduleEvent(EVENT_CHECK_DOTS, 100);
                return true;
            }
            else
                return false;
        }

        void OnUpdate(AuraEffect* /*aurEff*/, const uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CHECK_DOTS:
                    {
                        if (Unit* target = GetUnitOwner())
                        {
                            if (Unit* caster = GetCaster())
                            {
                                if (target->HasAura(589, caster->GetGUID()) && target->HasAura(34914, caster->GetGUID()))
                                    caster->CastSpell(caster, 99158, true);
                                else
                                    caster->RemoveAurasDueToSpell(99158);
                            }
                        }
                        events.ScheduleEvent(EVENT_CHECK_DOTS, 100);
                        break;
                    }
                }
            }
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
                caster->RemoveAurasDueToSpell(99158);
        }

        void Register()
        {
            OnEffectUpdate += AuraEffectUpdateFn(spell_pri_devouring_plague_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_PERIODIC_LEECH);
            AfterEffectRemove += AuraEffectRemoveFn(spell_pri_devouring_plague_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_LEECH, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_pri_devouring_plague_AuraScript();
    }
};

class spell_pri_shadow_orb_SpellScript : public SpellScript
{
    PrepareSpellScript(spell_pri_shadow_orb_SpellScript);

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        if (Player* caster = GetCaster()->ToPlayer())
        {
            // Shadow orbs
            if (AuraEffect* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_GENERIC, 4941, EFFECT_0))
            {
                uint32 stacks = aurEff->GetBase()->GetStackAmount();

                int32 damage = GetHitDamage();

                float amount = caster->GetMasteryAmount(77486, EFFECT_0);

                amount = aurEff->GetAmount() / (2.0f * stacks) + amount;

                SetHitDamage(damage + CalculatePct(damage, amount * stacks));

                int32 dmgamount = amount;

                caster->CastCustomSpell(caster, 95799, &dmgamount, &dmgamount, NULL, true);

                aurEff->GetBase()->Remove();
            }
        }
    }

    void Register()
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_shadow_orb_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

class spell_pri_temporal_boon : public SpellScriptLoader
{
public:
    spell_pri_temporal_boon() : SpellScriptLoader("spell_pri_temporal_boon") { }

    class spell_pri_temporal_boon_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pri_temporal_boon_SpellScript);

        void HandleChangeDuration()
        {
            if (Player* caster = GetCaster()->ToPlayer())
            {
                // this aura will last only 10 seconds for Discipline priests and 23 seconds for Shadow and Holy priests
                if (caster->GetPrimaryTalentTree(caster->GetActiveSpec()) == TALENT_TREE_PRIEST_DISCIPLINE)
                {
                    if (Aura* aura = caster->GetAura(GetSpellInfo()->Id))
                    {
                        aura->SetMaxDuration(10 * IN_MILLISECONDS);
                        aura->SetDuration(10 * IN_MILLISECONDS);
                    }
                }
            }
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_pri_temporal_boon_SpellScript::HandleChangeDuration);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_pri_temporal_boon_SpellScript();
    }
};

void AddSC_priest_spell_scripts()
{
    new spell_pri_glyph_of_prayer_of_healing(); // Verified
	new spell_pri_guardian_spirit(); // Verified
	new spell_pri_leap_of_faith_effect_trigger(); // Verified
	new spell_pri_lightwell_renew(); // Verified
	new spell_pri_mana_burn(); // Verified
	new spell_pri_mind_sear_spell(); // Verified
	new spell_pri_pain_and_suffering_proc(); // Verified
	new spell_pri_penance(); // Verified
	new spell_pri_power_word_shield(); // Verified
	new spell_pri_prayer_of_mending_heal(); // Verified
    new spell_pri_renew(); // Verified
	new spell_pri_shadow_word_death(); // Verified
	new spell_pri_shadowform(); // Verified
	new spell_pri_vampiric_touch(); // Verified
	new spell_pri_inner_focus(); // Verified
	new spell_pri_atonement_heal(); // Verified
	new spell_pri_mind_blast(); // Verified
	new spell_pri_fade(); // Verified
	new spell_pri_dispel_magic(); // Verified
	new spell_pri_shadow_orbs(); // Verified
	new spell_pri_mind_spike(); // Verified
	new spell_pri_power_word_barrier(); // Verified
	new spell_pri_cure_disease(); // Verified
	new spell_pri_inner_fire(); // Verified
	new spell_pri_chakra_swap_supressor(); // Verified
	new spell_pri_chakra_serenity_proc(); // Verified
	new spell_pri_chakra_sanctuary_heal(); // Verified
    new spell_pri_pw_sanctuary_heal(); // Recently Verified, but a bit unsure about spell id
	new spell_pri_fear(); // Verified
    new spell_pri_friendly_dispel(); // Verified
    new spell_pri_blessed_resilience(); // Recently Verified
    new spell_pri_chakra(); // Recently Verified, but a bit unsure about spell id
    new spell_priest_spirit_of_redemption(); // Recently Verified
    new spell_pri_holy_nova(); // Recently Verified
    new spell_pri_devouring_plague(); // Recently Verified
    new SpellScriptLoaderEx<spell_pri_shadow_orb_SpellScript>("spell_pri_shadow_orb"); // Recently Verified
    new spell_pri_temporal_boon(); // Recently Verified
}
