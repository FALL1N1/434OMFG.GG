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
 * Scripts for spells with SPELLFAMILY_DRUID and SPELLFAMILY_GENERIC spells used by druid players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_dru_".
 */

#include "Player.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Containers.h"

enum DruidSpells
{
    SPELL_DRUID_MOONFIRE                    = 8921,
    SPELL_DRUID_SUNFIRE                     = 93402,
    SPELL_DRUID_WRATH                       = 5176,
    SPELL_DRUID_STARFIRE                    = 2912,
    SPELL_DRUID_STARSURGE                   = 78674,
    SPELL_DRUID_ECLIPSE_GENERAL_ENERGIZE    = 89265,
    SPELL_DRUID_STARSURGE_ENERGIZE          = 86605,
    SPELL_DRUID_LUNAR_ECLIPSE_MARKER        = 67484, // Will make the yellow arrow on eclipse bar point to the blue side (lunar)
    SPELL_DRUID_SOLAR_ECLIPSE_MARKER        = 67483, // Will make the yellow arrow on eclipse bar point to the yellow side (solar)
    SPELL_DRUID_SOLAR_ECLIPSE               = 48517,
    SPELL_DRUID_LUNAR_ECLIPSE               = 48518,
    SPELL_DRUID_ENRAGE_MOD_DAMAGE           = 51185,
    SPELL_DRUID_GLYPH_OF_TYPHOON            = 62135,
    SPELL_DRUID_IDOL_OF_FERAL_SHADOWS       = 34241,
    SPELL_DRUID_IDOL_OF_WORSHIP             = 60774,
    SPELL_DRUID_INCREASED_MOONFIRE_DURATION = 38414,
    SPELL_DRUID_GLYPH_OF_STARFIRE           = 54846,
    SPELL_DRUID_KING_OF_THE_JUNGLE          = 48492,
    SPELL_DRUID_LIFEBLOOM_ENERGIZE          = 64372,
    SPELL_DRUID_LIFEBLOOM_FINAL_HEAL        = 33778,
    SPELL_DRUID_LIVING_SEED_HEAL            = 48503,
    SPELL_DRUID_LIVING_SEED_PROC            = 48504,
    SPELL_DRUID_NATURES_SPLENDOR            = 57865,
    SPELL_DRUID_SURVIVAL_INSTINCTS          = 50322,
    SPELL_DRUID_SAVAGE_ROAR                 = 62071,
    SPELL_DRUID_TIGER_S_FURY_ENERGIZE       = 51178,
    SPELL_DRUID_ITEM_T8_BALANCE_RELIC       = 64950,
    SPELL_DRUID_PULVERIZE_AURA              = 80951,
    SPELL_DRUID_LACERATE_BEAR               = 33745,
    SPELL_DRUID_BERSERK_BEAR                = 93622,
    SPELL_DRUID_BERSERK                     = 50334,
    SPELL_DRUID_MANGLE_BEAR                 = 33878,
    SPELL_DRUID_NATURES_SWIFTNESS           = 17116,
    SPELL_DRUID_GLYPH_OF_INNERVATE          = 54832,
    SPELL_DRUID_INNERVATE_GLYPH_EFFECT      = 54833,
    SPELL_DRUID_T12_FERAL_4P_BONUS          = 99009,
    SPELL_DRUID_SAVAGE_DEFENSE_TRIGGERED    = 105725
};

// 2912, 5176, 78674 - Starfire, Wrath, and Starsurge
class spell_dru_eclipse_energize : public SpellScriptLoader
{
    public:
        spell_dru_eclipse_energize() : SpellScriptLoader("spell_dru_eclipse_energize") { }

        class spell_dru_eclipse_energize_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_eclipse_energize_SpellScript);

            int32 energizeAmount;

            bool Load() override
            {
                if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    return false;

                if (GetCaster()->ToPlayer()->getClass() != CLASS_DRUID)
                    return false;

                energizeAmount = 0;

                return true;
            }

            void HandleEnergize(SpellEffIndex /*effIndex*/)
            {
                Player* caster = GetCaster()->ToPlayer();

                // No boomy, no deal.
                if (caster->GetPrimaryTalentTree(caster->GetActiveSpec()) != TALENT_TREE_DRUID_BALANCE)
                    return;

                bool lunar = false;
                bool solar = false;
                switch (GetSpellInfo()->Id)
                {
                    case SPELL_DRUID_SUNFIRE:
                    case SPELL_DRUID_MOONFIRE:
                    {
                        // Lunar shower
                        if (!caster->GetAuraEffect(SPELL_AURA_ADD_PCT_MODIFIER, SPELLFAMILY_DRUID, 3698, EFFECT_0))
                            return;
                        energizeAmount = 8;
                        break;
                    }
                    case SPELL_DRUID_WRATH:
                    case SPELL_DRUID_STARFIRE:
                    {
                        energizeAmount = GetSpellInfo()->Effects[EFFECT_1].BasePoints;

                        // Don't add Euphoria beenfit when already in eclipse state
                        if (!caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE) && !caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE))
                        {
                            // Druid T12 Balance 4P Bonus
                            if (caster->HasAura(99049))
                                energizeAmount += GetSpellInfo()->Id == SPELL_DRUID_WRATH ? 3 : 5;

                            // Euphoria
                            if (AuraEffect* euphoria = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 4431, EFFECT_0))
                            {
                                if (roll_chance_i(euphoria->GetAmount()))
                                {
                                    energizeAmount *= 2;
                                    if (GetSpellInfo()->Id == SPELL_DRUID_WRATH)
                                    {
                                        caster->SetWrathCastCount(caster->GetWrathCastCount() + 1); // Euphoria counts as two wrath casts
                                        if (caster->GetWrathCastCount() == 3) // we need here a special check because next setcount can reset the count to 1
                                            energizeAmount += 1;
                                    }
                                }
                            }
                        }

                        // wrath cast sequence bonus energy
                        if (GetSpellInfo()->Id == SPELL_DRUID_WRATH)
                        {
                            caster->SetWrathCastCount(caster->GetWrathCastCount() + 1);
                            if (caster->GetWrathCastCount() == 3)
                                energizeAmount += 1;

                        }
                        break;
                    }
                    default:
                        energizeAmount = GetSpellInfo()->Effects[EFFECT_1].BasePoints;
                        break;
                }

                switch (GetSpellInfo()->Id)
                {
                    case SPELL_DRUID_SUNFIRE:
                    case SPELL_DRUID_WRATH:
                    {
                        energizeAmount = -energizeAmount;
                        // If we are set to fill the lunar side or we've just logged in with 0 power..
                        if (!caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE_MARKER) && (caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE_MARKER) || caster->GetPower(POWER_ECLIPSE) == 0))
                        {
                            caster->CastCustomSpell(caster, SPELL_DRUID_ECLIPSE_GENERAL_ENERGIZE, &energizeAmount, 0, 0, true);
                            // If the energize was due to 0 power, cast the eclipse marker aura
                            if (!caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE_MARKER))
                                caster->CastSpell(caster, SPELL_DRUID_LUNAR_ECLIPSE_MARKER, true);

                            lunar = caster->GetPower(POWER_ECLIPSE) == -100;
                        }
                        // The energizing effect brought us out of the solar eclipse, remove the aura
						if (caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE) && caster->GetPower(POWER_ECLIPSE) <= 0)
						{
							caster->RemoveAurasDueToSpell(SPELL_DRUID_SOLAR_ECLIPSE);
							caster->RemoveAurasDueToSpell(94338);
						}
							break;
                    }
                    case SPELL_DRUID_MOONFIRE:
                    case SPELL_DRUID_STARFIRE:
                    {
                        // If we are set to fill the solar side or we've just logged in with 0 power..
                        if (!caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE_MARKER) && (caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE_MARKER) || caster->GetPower(POWER_ECLIPSE) == 0))
                        {
                            caster->CastCustomSpell(caster, SPELL_DRUID_ECLIPSE_GENERAL_ENERGIZE, &energizeAmount, 0, 0, true);
                            // If the energize was due to 0 power, cast the eclipse marker aura
                            if (!caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE_MARKER))
                                caster->CastSpell(caster, SPELL_DRUID_SOLAR_ECLIPSE_MARKER, true);

                            solar = caster->GetPower(POWER_ECLIPSE) == 100;
                        }
                        // The energizing effect brought us out of the lunar eclipse, remove the aura
                        if (caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE) && caster->GetPower(POWER_ECLIPSE) >= 0)
                            caster->RemoveAura(SPELL_DRUID_LUNAR_ECLIPSE);
                        break;
                    }
                    case SPELL_DRUID_STARSURGE:
                    {
                        // If we are set to fill the solar side or we've just logged in with 0 power (confirmed with sniffs)
                        if (!caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE_MARKER) && (caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE_MARKER) || caster->GetPower(POWER_ECLIPSE) == 0))
                        {
                            caster->CastCustomSpell(caster, SPELL_DRUID_STARSURGE_ENERGIZE, &energizeAmount, 0, 0, true);
                            solar = caster->GetPower(POWER_ECLIPSE) == 100;

                            // If the energize was due to 0 power, cast the eclipse marker aura
                            if (!caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE_MARKER))
                                caster->CastSpell(caster, SPELL_DRUID_SOLAR_ECLIPSE_MARKER, true);
                        }
                        else if (!caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE_MARKER) && caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE_MARKER))
                        {
                            energizeAmount = -energizeAmount;
                            caster->CastCustomSpell(caster, SPELL_DRUID_STARSURGE_ENERGIZE, &energizeAmount, 0, 0, true);
                            lunar = caster->GetPower(POWER_ECLIPSE) == -100;
                        }

                        // The energizing effect brought us out of the lunar eclipse, remove the aura
                        if (caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE) && caster->GetPower(POWER_ECLIPSE) >= 0)
                            caster->RemoveAura(SPELL_DRUID_LUNAR_ECLIPSE);
                        // The energizing effect brought us out of the solar eclipse, remove the aura
						else if (caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE) && caster->GetPower(POWER_ECLIPSE) <= 0)
						{
							caster->RemoveAurasDueToSpell(SPELL_DRUID_SOLAR_ECLIPSE);
							caster->RemoveAurasDueToSpell(94338);
						}
                        break;
                    }
                }
                if (solar || lunar)
                {
                    // Euphoria mana regen
                    if (AuraEffect* euphoria = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 4431, EFFECT_2))
                        caster->CastCustomSpell(81070, SPELLVALUE_BASE_POINT0, euphoria->GetAmount(), caster, TRIGGERED_FULL_MASK);

                    // Remove Nature's grace cooldown
                    caster->RemoveSpellCooldown(16886);

                    if (solar)
                        // Sunfire
                        if (caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 3262, EFFECT_0))
                            caster->CastCustomSpell(94338, SPELLVALUE_BASE_POINT0, SPELL_DRUID_SUNFIRE, caster, TRIGGERED_FULL_MASK);

                    if (caster->HasAura(90163)) // Astral Alignment
                        caster->CastCustomSpell(90164, SPELLVALUE_AURA_STACK, 3, NULL, true);

                    // reset wrath cast sequence
                    caster->SetWrathCastCount(0);
                }
            }

            void Register() override
            {
                if (m_scriptSpellId == SPELL_DRUID_MOONFIRE || m_scriptSpellId == SPELL_DRUID_SUNFIRE)
                    OnEffectHitTarget += SpellEffectFn(spell_dru_eclipse_energize_SpellScript::HandleEnergize, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
                else
                    OnEffectHitTarget += SpellEffectFn(spell_dru_eclipse_energize_SpellScript::HandleEnergize, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dru_eclipse_energize_SpellScript;
        }

        class spell_dru_moonfire_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_moonfire_AuraScript);

            void HandleApply(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                AuraEffect* effect2 = GetEffect(EFFECT_2);
                if (!effect2)
                    return;
                effect2->SetAmount(0);
                if (Unit* caster = GetCaster())
                {
                    Unit::appliedAurasList::reverse_iterator itr = std::find(caster->appliedAuras.rbegin(), caster->appliedAuras.rend(), GetAura());
                    if (itr != caster->appliedAuras.rend() && itr != caster->appliedAuras.rbegin())
                        caster->appliedAuras.splice(caster->appliedAuras.end(), caster->appliedAuras, --(itr.base()));
                }
            }

            void Register()
            {
                if (m_scriptSpellId == SPELL_DRUID_MOONFIRE)
                    OnEffectApply += AuraEffectApplyFn(spell_dru_moonfire_AuraScript::HandleApply, EFFECT_2, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_moonfire_AuraScript();
        }
};

class spell_dru_energize : public SpellScriptLoader
{
    public:
        spell_dru_energize() : SpellScriptLoader("spell_dru_energize") { }

        class spell_dru_energize_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_energize_SpellScript);

            bool Load()
            {
                if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    return false;

                if (GetCaster()->ToPlayer()->getClass() != CLASS_DRUID)
                    return false;

                return true;
            }

            void OnHit()
            {
                Player* caster = GetCaster()->ToPlayer();

                // No boomy, no deal.
                if (caster->GetPrimaryTalentTree(caster->GetActiveSpec()) != TALENT_TREE_DRUID_BALANCE)
                    return;

                bool lunar = caster->GetPower(POWER_ECLIPSE) == -100;
                bool solar = caster->GetPower(POWER_ECLIPSE) == 100;
                if (caster->GetPower(POWER_ECLIPSE) > 0)
                {
                    if (!caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE_MARKER))
                        caster->CastSpell(caster, SPELL_DRUID_SOLAR_ECLIPSE_MARKER, true);
                }
                else if (caster->GetPower(POWER_ECLIPSE) < 0)
                {
                    if (!caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE_MARKER))
                        caster->CastSpell(caster, SPELL_DRUID_LUNAR_ECLIPSE_MARKER, true);
                }
                if (solar || lunar)
                {
                    // Euphoria mana regen
                    if (AuraEffect* euphoria = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 4431, EFFECT_2))
                        caster->CastCustomSpell(81070, SPELLVALUE_BASE_POINT0, euphoria->GetAmount(), caster, TRIGGERED_FULL_MASK);

                    // Remove Nature's grace cooldown
                    caster->RemoveSpellCooldown(16886);

                    if (solar)
                    {
                        // Sunfire
                        if (caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 3262, EFFECT_0))
                            caster->CastCustomSpell(94338, SPELLVALUE_BASE_POINT0, SPELL_DRUID_SUNFIRE, caster, TRIGGERED_FULL_MASK);

                        if (caster->HasAura(SPELL_DRUID_SOLAR_ECLIPSE_MARKER))
                        {
                            caster->RemoveAurasDueToSpell(SPELL_DRUID_SOLAR_ECLIPSE_MARKER);
                            caster->CastSpell(caster, SPELL_DRUID_LUNAR_ECLIPSE_MARKER, true);
                        }

                        caster->CastSpell(caster, 48517, true);
                    }

                    if (lunar)
                    {
                        if (caster->HasAura(SPELL_DRUID_LUNAR_ECLIPSE_MARKER))
                        {
                            caster->RemoveAurasDueToSpell(SPELL_DRUID_LUNAR_ECLIPSE_MARKER);
                            caster->CastSpell(caster, SPELL_DRUID_SOLAR_ECLIPSE_MARKER, true);
                        }

                        caster->CastSpell(caster, 48518, true);
                    }
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_dru_energize_SpellScript::OnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_energize_SpellScript;
        }
};

// Starsurge
class spell_dru_starsurge : public SpellScriptLoader
{
    public:
        spell_dru_starsurge() : SpellScriptLoader("spell_dru_starsurge") { }

        class spell_dru_starsurge_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_starsurge_SpellScript);

            void HandleProc(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                if (caster->HasAura(93400))
                    // Check if starsurge was affected by active starsurge proc
                    if (Spell* spell = GetSpell())
                        if (spell->m_appliedAuras.find(93400) == spell->m_appliedAuras.end())
                            caster->ToPlayer()->RemoveSpellCooldown(78674, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dru_starsurge_SpellScript::HandleProc, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_starsurge_SpellScript();
        }
};

// -1850 - Dash
class spell_dru_dash : public SpellScriptLoader
{
    public:
        spell_dru_dash() : SpellScriptLoader("spell_dru_dash") { }

        class spell_dru_dash_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_dash_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                // do not set speed if not in cat form
                if (GetUnitOwner()->GetShapeshiftForm() != FORM_CAT)
                    amount = 0;
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_dash_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_INCREASE_SPEED);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_dash_AuraScript();
        }
};

// Innervate
class spell_dru_innervate : public SpellScriptLoader
{
    public:
        spell_dru_innervate() : SpellScriptLoader("spell_dru_innervate") { }

        class spell_dru_innervate_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_innervate_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetUnitOwner())
                    {
                        int32 pct = amount / 4;
                        // If cast on self Innervate regenerates 15% more mana
                        if (caster == target)
                        {
                            pct *= 4;
                            if (AuraEffect* dreamState = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 2255, EFFECT_0))
                                pct += dreamState->GetAmount();
                        }
                        amount = (target->GetMaxPower(POWER_MANA) * (pct / 100.0f)) / aurEff->GetTotalTicks();
                    }
                }
            }

            void AfterApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    // Glyph of Innervate
                    if (caster->HasAura(SPELL_DRUID_GLYPH_OF_INNERVATE, caster->GetGUID()))
                    {
                        if (caster != GetTarget())
                            caster->CastSpell(caster, SPELL_DRUID_INNERVATE_GLYPH_EFFECT, true);
                    }
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_innervate_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_ENERGIZE);
                AfterEffectApply += AuraEffectApplyFn(spell_dru_innervate_AuraScript::AfterApply, EFFECT_0, SPELL_AURA_PERIODIC_ENERGIZE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_innervate_AuraScript();
        }
};

// Solar Beam
class spell_dru_beam : public SpellScriptLoader
{
    public:
        spell_dru_beam() : SpellScriptLoader("spell_dru_beam") { }

        class spell_dru_beam_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_beam_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) { return true; }

            void HandleEffectPeriodic(AuraEffect const* aurEff)
            {
                if (DynamicObject* dyn = GetTarget()->GetDynObject(aurEff->GetId()))
                    GetTarget()->CastSpell(dyn->GetPositionX(), dyn->GetPositionY(), dyn->GetPositionZ(), 81261, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_beam_AuraScript::HandleEffectPeriodic, EFFECT_2, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_beam_AuraScript();
        }
};

// Rejuvenation
class spell_dru_rejuv : public SpellScriptLoader
{
    public:
        spell_dru_rejuv() : SpellScriptLoader("spell_dru_rejuv") { }

        class spell_dru_rejuv_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_rejuv_AuraScript);

            void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    uint32 rejuvCount = 0;
                    Unit::appliedAurasList const& auras = caster->appliedAuras;
                    for (Unit::appliedAurasList::const_iterator itr = auras.begin(); itr != auras.end(); itr++)
                    {
                        if (Aura* aura = (*itr))
                        {
                            if (aura->GetId() == GetSpellInfo()->Id)
                                rejuvCount++;
                        }
                    }
                    if (rejuvCount < 3)
                        caster->RemoveAurasDueToSpell(96206);
                }
            }

            void AfterApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (AuraEffect* naturesBounty = caster->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_DRUID, 197, EFFECT_0))
                    {
                        uint32 rejuvCount = 0;
                        Player::appliedAurasList const& auras = caster->ToPlayer()->appliedAuras;
                        for (Player::appliedAurasList::const_iterator itr = auras.begin(); itr != auras.end(); itr++)
                        {
                            Aura* aura = (*itr);
                            if (aura->GetId() == GetSpellInfo()->Id)
                                rejuvCount++;
                        }
                        if (rejuvCount >= 3)
                        {
                            int32 bp0 = -naturesBounty->GetSpellInfo()->Effects[EFFECT_1].BasePoints;
                            caster->CastCustomSpell(caster, 96206, &bp0, NULL, NULL, true);
                        }
                    }

                    // Gift of the earthmother
                    if (AuraEffect* earthMother = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 3186, EFFECT_0))
                    {
                        int32 bp0 = CalculatePct((aurEff->GetAmount() * aurEff->GetTotalTicks()), earthMother->GetAmount());
                        caster->CastCustomSpell(GetUnitOwner(), 64801, &bp0, NULL, NULL, true);
                    }

                    // Druid T13 Restoration 4P Bonus (Rejuvenation)
                    if (AuraEffect* t13_4p = caster->GetAuraEffect(105770, EFFECT_0, GetCasterGUID()))
                    {
                        if (roll_chance_i(t13_4p->GetAmount()))
                        {
                            uint32 duration = GetAura()->CalcMaxDuration(caster) * 2;
                            GetAura()->SetMaxDuration(duration);
                            GetAura()->SetDuration(duration);
                        }
                    }
                }
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dru_rejuv_AuraScript::AfterApply, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_rejuv_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_rejuv_AuraScript();
        }
};

// Wild mushroom
class spell_dru_wild_mushroom : public SpellScriptLoader
{
    public:
        spell_dru_wild_mushroom() : SpellScriptLoader("spell_dru_wild_mushroom") { }

        class spell_dru_wild_mushroom_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_wild_mushroom_SpellScript);

            void HandleSummon()
            {
                TempSummon* oldestSummon = NULL;
                uint32 duration = 300000;
                std::list<Creature*> MinionList;
                GetCaster()->GetAllMinionsByEntry(MinionList, GetSpellInfo()->Effects[EFFECT_0].MiscValue);
                if (MinionList.size() > uint32(GetSpellInfo()->Effects[EFFECT_0].BasePoints))
                {
                    for (std::list<Creature*>::iterator itr = MinionList.begin(); itr != MinionList.end(); itr++)
                    {
                        TempSummon* temp  = (*itr)->ToTempSummon();
                        if (temp->GetTimer() < duration)
                        {
                            duration = temp->GetTimer();
                            oldestSummon = temp;
                        }
                    }
                }

                if (oldestSummon)
                    oldestSummon->UnSummon();
            }

            void Register() override
            {
                AfterCast += SpellCastFn(spell_dru_wild_mushroom_SpellScript::HandleSummon);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dru_wild_mushroom_SpellScript();
        }
};

// Wild mushroom detonation
class spell_dru_wild_mushroom_detonation : public SpellScriptLoader
{
    public:
        spell_dru_wild_mushroom_detonation() : SpellScriptLoader("spell_dru_wild_mushroom_detonation") { }

        class spell_dru_wild_mushroom_detonation_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_wild_mushroom_detonation_SpellScript);

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                float max_range = GetSpellInfo()->GetMaxRange();
                std::list<Creature*> MinionList;
                caster->GetAllMinionsByEntry(MinionList, 47649);
                if (!MinionList.size())
                    return SPELL_FAILED_BAD_TARGETS;

                // we need a second check because the mushrooms dont despawn instant after death
                bool mushroomAlive = false;
                for (std::list<Creature*>::iterator itr = MinionList.begin(); itr != MinionList.end(); itr++)
                {
                    if ((*itr)->isAlive())
                        mushroomAlive = true;

                    Position shroomPos;
                    (*itr)->GetPosition(&shroomPos);
                    if (!caster->IsWithinDist3d(&shroomPos, max_range))
                        return SPELL_FAILED_OUT_OF_RANGE;
                }

                if (!mushroomAlive)
                    return SPELL_FAILED_BAD_TARGETS;

                return SPELL_CAST_OK;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                float max_range = GetSpellInfo()->GetMaxRange();
                std::list<Creature*> MinionList;
                caster->GetAllMinionsByEntry(MinionList, 47649);
                for (std::list<Creature*>::iterator itr = MinionList.begin(); itr != MinionList.end(); itr++)
                {
                    if (!(*itr)->isAlive())
                        continue;

                    Position shroomPos;
                    (*itr)->GetPosition(&shroomPos);
                    if (caster->IsWithinDist3d(&shroomPos, max_range))
                    {
                        caster->CastSpell((*itr)->GetPositionX(), (*itr)->GetPositionY(), (*itr)->GetPositionZ(), 78777, true);
                        (*itr)->Kill((*itr));
                        (*itr)->ToTempSummon()->UnSummon(2000);
                        // Fungal Growth
                        if (AuraEffect* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 2681, EFFECT_0))
                        {
                            uint32 triggeredId = 0;
                            switch (aurEff->GetAmount())
                            {
                                case 25:
                                    triggeredId = 81291;
                                    break;
                                case 50:
                                    triggeredId = 81283;
                                    break;
                            }
                            caster->CastSpell((*itr)->GetPositionX(), (*itr)->GetPositionY(), (*itr)->GetPositionZ(), triggeredId, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dru_wild_mushroom_detonation_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_dru_wild_mushroom_detonation_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_wild_mushroom_detonation_SpellScript();
        }
};


// -5229 - Enrage
class spell_dru_enrage : public SpellScriptLoader
{
    public:
        spell_dru_enrage() : SpellScriptLoader("spell_dru_enrage") { }

        class spell_dru_enrage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_enrage_SpellScript);

            void OnHit()
            {
                // King of the jungle
                if (AuraEffect const* aurEff = GetHitUnit()->GetDummyAuraEffect(SPELLFAMILY_DRUID, 2850, EFFECT_0))
                    GetHitUnit()->CastCustomSpell(SPELL_DRUID_ENRAGE_MOD_DAMAGE, SPELLVALUE_BASE_POINT0, aurEff->GetAmount(), GetHitUnit(), true);
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_dru_enrage_SpellScript::OnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_enrage_SpellScript();
        }
};

// Stampede
class spell_dru_stampede : public SpellScriptLoader
{
    public:
        spell_dru_stampede() : SpellScriptLoader("spell_dru_stampede") { }

        class spell_dru_stampede_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_stampede_SpellScript);

            void OnHit()
            {
                Unit* caster = GetCaster();
                uint32 triggeredId = 0;
                switch (GetSpellInfo()->Id)
                {
                    case 16979: // Feral charge bear
                        if (AuraEffect* stampede = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 3930, EFFECT_0))
                        {
                            switch (stampede->GetAmount())
                            {
                                case 5:
                                    triggeredId = 81016;
                                    break;
                                case 10:
                                    triggeredId = 81017;
                                    break;
                            }
                        }
                        break;
                    case 49376: // Feral charge cat
                    case 5217:  // Tiger's Fury 
                        if (AuraEffect* stampede = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 3930, EFFECT_1))
                        {
                            switch (stampede->GetAmount())
                            {
                                case 3:
                                    triggeredId = 81021;
                                    break;
                                case 6:
                                    triggeredId = 81022;
                                    break;
                            }
                            caster->CastSpell(caster, 109881, true);
                        }
                        break;
                }
                if ((GetSpellInfo()->Id == 5217) && !GetCaster()->HasAura(105735))
                    return;
                if (triggeredId)
                    caster->CastSpell(caster, triggeredId, true);
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_dru_stampede_SpellScript::OnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_stampede_SpellScript();
        }
};

// Feral swiftness
class spell_dru_feral_swiftness : public SpellScriptLoader
{
    public:
        spell_dru_feral_swiftness() : SpellScriptLoader("spell_dru_feral_swiftness") { }

        class spell_dru_feral_swiftness_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_feral_swiftness_SpellScript);

            void OnHit()
            {
                if (AuraEffect const* feralSwiftness = GetCaster()->GetAuraEffect(SPELL_AURA_MOD_DODGE_PERCENT, SPELLFAMILY_DRUID, 67, EFFECT_0))
                {
                    uint32 chance = feralSwiftness->GetId() == 24867 ? 50 : 100;
                    if (roll_chance_i(chance))
                        GetCaster()->CastSpell(GetHitUnit(), 97985, true);
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_dru_feral_swiftness_SpellScript::OnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_feral_swiftness_SpellScript();
        }
};

// Skull Bash
class spell_dru_skull_bash : public SpellScriptLoader
{
    public:
        spell_dru_skull_bash() : SpellScriptLoader("spell_dru_skull_bash") { }

        class spell_dru_skull_bash_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_skull_bash_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* victim = GetHitUnit();
                caster->CastSpell(victim, 93983, true);
                caster->CastSpell(victim, 93985, true);
                // Brutal impact
                if (AuraEffect* aurEff = caster->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_DRUID, 473, EFFECT_0))
                {
                    uint32 triggeredId = 0;
                    switch (aurEff->GetId())
                    {
                        case 16940:
                            triggeredId = 82364;
                            break;
                        case 16941:
                            triggeredId = 82365;
                            break;
                    }
                    caster->CastSpell(victim, triggeredId, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dru_skull_bash_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_skull_bash_SpellScript();
        }
};

// Primal madness
class spell_dru_primal_madness : public SpellScriptLoader
{
    public:
        spell_dru_primal_madness() : SpellScriptLoader("spell_dru_primal_madness") { }

        class spell_dru_primal_madness_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_primal_madness_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                // Primal Madness
                if (AuraEffect const* primalMadness = GetCaster()->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_DRUID, 1181, EFFECT_1))
                {
                    uint32 triggeredId = 0;
                    switch (primalMadness->GetId())
                    {
                        case 80316:
                            triggeredId = 80879;
                            break;
                        case 80317:
                            triggeredId = 80886;
                            break;
                    }
                    GetCaster()->CastSpell(GetCaster(), triggeredId, true);
                }
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    caster->RemoveAurasDueToSpell(80879);
                    caster->RemoveAurasDueToSpell(80886);
                }
            }

            void Register() override
            {
                if (m_scriptSpellId == 50334)
                {
                    AfterEffectApply += AuraEffectApplyFn(spell_dru_primal_madness_AuraScript::OnApply, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
                    AfterEffectRemove += AuraEffectRemoveFn(spell_dru_primal_madness_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
                } else
                {
                    AfterEffectApply += AuraEffectApplyFn(spell_dru_primal_madness_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
                    AfterEffectRemove += AuraEffectRemoveFn(spell_dru_primal_madness_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
                }
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_primal_madness_AuraScript();
        }
};

// Feral aggression
class spell_dru_feral_aggression : public SpellScriptLoader
{
    public:
        spell_dru_feral_aggression() : SpellScriptLoader("spell_dru_feral_aggression") { }

        class spell_dru_feral_aggression_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_feral_aggression_SpellScript);

            void OnHitTarget(SpellEffIndex effIndex)
            {
                if (Unit* caster = GetCaster())
                    if (AuraEffect const* feralSwiftness = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 960, EFFECT_0))
                    {
                        PreventHitDefaultEffect(effIndex);
                        caster->CastCustomSpell(91565, SPELLVALUE_AURA_STACK, feralSwiftness->GetAmount(), GetHitUnit(), true);
                    }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dru_feral_aggression_SpellScript::OnHitTarget, EFFECT_0, SPELL_EFFECT_TRIGGER_SPELL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_feral_aggression_SpellScript();
        }
};

// Pulverize
class spell_dru_pulverize : public SpellScriptLoader
{
    public:
        spell_dru_pulverize() : SpellScriptLoader("spell_dru_pulverize") { }

        class spell_dru_pulverize_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_pulverize_SpellScript);

            void HandleOnHit()
            {
                if (AuraEffect const* lacerate = GetHitUnit()->GetAuraEffect(SPELL_DRUID_LACERATE_BEAR, EFFECT_0, GetCaster()->GetGUID()))
                {
                    int32 stacks = lacerate->GetBase()->GetStackAmount();
                    uint32 damage = GetSpellInfo()->Effects[EFFECT_2].CalcValue(GetCaster()) * (GetSpellInfo()->Effects[EFFECT_0].BasePoints / 100.0f);
                    damage *= stacks;
                    SetHitDamage(GetHitDamage() + damage);
                    Unit* caster = GetCaster();
                    if (caster)
                    {
                        Aura* auraPulver = caster->AddAura(SPELL_DRUID_PULVERIZE_AURA, caster);
                        if (auraPulver)
                        {
                            AuraEffect* effectPulver = auraPulver->GetEffect(EFFECT_0);
                            if (effectPulver)
                                effectPulver->ChangeAmount(effectPulver->GetBaseAmount() * stacks);
                        }
                    }
                    lacerate->GetBase()->Remove();
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_pulverize_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_pulverize_SpellScript();
        }
};

// Barkskin
class spell_dru_barkskin : public SpellScriptLoader
{
    public:
        spell_dru_barkskin() : SpellScriptLoader("spell_dru_barkskin") { }

        class spell_dru_barkskin_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_barkskin_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                if (caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 689, EFFECT_0))
                    caster->CastSpell(caster, 63058, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_barkskin_SpellScript::HandleOnHit);
            }
        };

        class spell_dru_barkskin_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_barkskin_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
                    return;

                // Druid T12 Feral 4P Bonus
                if (Unit* caster = GetCaster())
                    if (caster->HasAura(99009))
                        caster->CastSpell(caster, 99011, true);
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_barkskin_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_barkskin_SpellScript();
        }

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_barkskin_AuraScript();
        }
};

// Harmony (Mastery)
class spell_dru_harmony : public SpellScriptLoader
{
    public:
        spell_dru_harmony() : SpellScriptLoader("spell_dru_harmony") { }

        class spell_dru_harmony_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_harmony_SpellScript);

            void OnHit()
            {
                if (AuraEffect const* mastery = GetCaster()->GetAuraEffect(77495, EFFECT_0, GetCaster()->GetGUID()))
                {
                    int32 bp0 = mastery->GetAmount();
                    GetCaster()->CastCustomSpell(GetCaster(), 100977, &bp0, NULL, NULL, true);
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_dru_harmony_SpellScript::OnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_harmony_SpellScript();
        }
};

// Empowered touch
class spell_dru_emp_touch : public SpellScriptLoader
{
    public:
        spell_dru_emp_touch() : SpellScriptLoader("spell_dru_emp_touch") { }

        class spell_dru_emp_touch_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_emp_touch_SpellScript);

            void OnHit()
            {
                if (AuraEffect const* aurEff = GetCaster()->GetDummyAuraEffect(SPELLFAMILY_DRUID, 2251, EFFECT_1))
                {
                    Aura* latestFlower = NULL;
                    int32 currentDuration = 0;
                    Unit::appliedAurasList const& auras = GetCaster()->appliedAuras;

                    for (Unit::appliedAurasList::const_reverse_iterator itr = auras.rbegin(); itr != auras.rend(); ++itr)
                    {
                        if ((*itr)->GetId() == 33763) // lifebloom
                        {
                            if (currentDuration < (*itr)->GetDuration())
                            {
                                currentDuration = (*itr)->GetDuration();
                                latestFlower = (*itr);
                            }
                        }
                    }

                    // Empowered Touch now also affects Regrowth. 
                    // In addition, after Tree of Life is no longer active, this talent will only refresh the most recently cast or refreshed Lifebloom, and will not refresh other copies of Lifebloom.
                    if (GetSpellInfo()->Id == 33763 || GetCaster()->HasAura(33891) || (latestFlower && (latestFlower->GetUnitOwner() == GetHitUnit())))
                        if (roll_chance_i(aurEff->GetAmount()))
                            GetCaster()->CastSpell(GetHitUnit(), 88433, true);
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_dru_emp_touch_SpellScript::OnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_emp_touch_SpellScript();
        }
};

// Empowered touch
class spell_dru_emp_touch_triggered : public SpellScriptLoader
{
    public:
        spell_dru_emp_touch_triggered() : SpellScriptLoader("spell_dru_emp_touch_triggered") { }

        class spell_dru_emp_touch_triggered_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_emp_touch_triggered_SpellScript);

            void OnHit()
            {
                Unit* caster = GetCaster();
                if (caster)
                {
                    if (Aura* aura = GetHitUnit()->GetAura(33763, caster->GetGUID()))
                    {
                        aura->RefreshDuration();
                        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                            if (aura->HasEffect(i))
                                aura->GetEffect(i)->CalculatePeriodic(caster, false, false);
                        // Revitalize - Replenishment
                        if (caster->GetAuraEffect(SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE, SPELLFAMILY_DRUID, 2862, EFFECT_0))
                            caster->CastSpell(caster, 57669, true);
                    }
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_dru_emp_touch_triggered_SpellScript::OnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_emp_touch_triggered_SpellScript();
        }
};

// Efflorescence
class spell_sha_efflorescence : public SpellScriptLoader
{
    public:
        spell_sha_efflorescence() : SpellScriptLoader("spell_sha_efflorescence") { }

        class spell_sha_efflorescence_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_efflorescence_AuraScript);

            void HandleEffectPeriodic(AuraEffect const* aurEff)
            {
                if (DynamicObject* dyn = GetTarget()->GetDynObject(aurEff->GetId()))
                    GetTarget()->CastSpell(dyn->GetPositionX(), dyn->GetPositionY(), dyn->GetPositionZ(), 81269, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_sha_efflorescence_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sha_efflorescence_AuraScript();
        }
};

// Efflorescence heal
class spell_sha_efflorescence_heal : public SpellScriptLoader
{
    public:
        spell_sha_efflorescence_heal() : SpellScriptLoader("spell_sha_efflorescence_heal") { }

        class spell_sha_efflorescence_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_efflorescence_heal_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                std::list<Unit*> temp;
                for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end(); itr++)
                    if (Unit* unit = (*itr)->ToUnit())
                        temp.push_back(unit);

                targets.clear();
                temp.sort(Trinity::HealthPctOrderPred());
                if (temp.size() > 3)
                    temp.resize(3);
                for (std::list<Unit*>::iterator itr = temp.begin(); itr != temp.end(); itr++)
                    targets.push_back((WorldObject*)(*itr));
            }

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                if (AuraEffect* aurEff = GetCaster()->GetAuraEffect(81262, EFFECT_1, GetCaster()->GetGUID()))
                    SetHitHeal(aurEff->GetAmount());
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_efflorescence_heal_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
                OnEffectHitTarget += SpellEffectFn(spell_sha_efflorescence_heal_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sha_efflorescence_heal_SpellScript();
        }
};

// 54846 - Glyph of Starfire
class spell_dru_glyph_of_starfire : public SpellScriptLoader
{
public:
    spell_dru_glyph_of_starfire() : SpellScriptLoader("spell_dru_glyph_of_starfire") { }

    class spell_dru_glyph_of_starfire_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dru_glyph_of_starfire_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_DRUID_INCREASED_MOONFIRE_DURATION))
                return false;
            return true;
        }

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            if (Unit* unitTarget = GetHitUnit())
                if (AuraEffect const* aurEff = unitTarget->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_DRUID, 0x2, 0, 0, caster->GetGUID()))
                {
                    Aura* aura = aurEff->GetBase();
                    aura->SetDuration(aura->GetDuration() + 3000);
                    aura->SetMaxDuration(aura->GetMaxDuration() + 3000);
                }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_dru_glyph_of_starfire_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_dru_glyph_of_starfire_SpellScript();
    }
};

// 54845 - Glyph of Starfire
class spell_dru_glyph_of_starfire_proc : public SpellScriptLoader
{
public:
    spell_dru_glyph_of_starfire_proc() : SpellScriptLoader("spell_dru_glyph_of_starfire_proc") { }

    class spell_dru_glyph_of_starfire_proc_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dru_glyph_of_starfire_proc_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_DRUID_GLYPH_OF_STARFIRE))
                return false;
            return true;
        }

        void HandleEffectProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            Unit* caster = GetCaster();
            if (Unit* unitTarget = eventInfo.GetProcTarget())
            {
                if (const Aura* moonfire = unitTarget->GetAura(SPELL_DRUID_MOONFIRE, caster->GetGUID()))
                {
                    // Only last applied moonfire can be increased
                    Unit::appliedAurasList const& auras = caster->appliedAuras;
                    for (Unit::appliedAurasList::const_reverse_iterator itr = auras.rbegin(); itr != auras.rend(); ++itr)
                    {
                        if ((*itr)->GetId() == SPELL_DRUID_MOONFIRE)
                            if ((*itr) != moonfire)
                                return;
                            else
                                break;
                    }

                    if (AuraEffect* eff = moonfire->GetEffect(EFFECT_2))
                    {
                        if (eff->GetAmount() < 3)
                            eff->SetAmount(eff->GetAmount() + 1);
                        else
                            return;
                    }
                    else
                        return;
                    GetTarget()->CastSpell(eventInfo.GetProcTarget(), SPELL_DRUID_GLYPH_OF_STARFIRE, true, NULL, aurEff);
                }
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_dru_glyph_of_starfire_proc_AuraScript::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };
    
    AuraScript* GetAuraScript() const
    {
        return new spell_dru_glyph_of_starfire_proc_AuraScript();
    }
};

// 34246 - Idol of the Emerald Queen
// 60779 - Idol of Lush Moss
class spell_dru_idol_lifebloom : public SpellScriptLoader
{
    public:
        spell_dru_idol_lifebloom() : SpellScriptLoader("spell_dru_idol_lifebloom") { }

        class spell_dru_idol_lifebloom_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_idol_lifebloom_AuraScript);

            void HandleEffectCalcSpellMod(AuraEffect const* aurEff, SpellModifier*& spellMod)
            {
                if (!spellMod)
                {
                    spellMod = new SpellModifier(GetAura());
                    spellMod->op = SPELLMOD_DOT;
                    spellMod->type = SPELLMOD_FLAT;
                    spellMod->spellId = GetId();
                    spellMod->mask = GetSpellInfo()->Effects[aurEff->GetEffIndex()].SpellClassMask;
                }
                spellMod->value = aurEff->GetAmount() / 7;
            }

            void Register()
            {
                DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(spell_dru_idol_lifebloom_AuraScript::HandleEffectCalcSpellMod, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_idol_lifebloom_AuraScript();
        }
};

// -5570 - Insect Swarm
class spell_dru_insect_swarm : public SpellScriptLoader
{
    public:
        spell_dru_insect_swarm() : SpellScriptLoader("spell_dru_insect_swarm") { }

        class spell_dru_insect_swarm_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_insect_swarm_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, int32 & amount, bool & /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                    if (AuraEffect const* relicAurEff = caster->GetAuraEffect(SPELL_DRUID_ITEM_T8_BALANCE_RELIC, EFFECT_0))
                        amount += relicAurEff->GetAmount() / aurEff->GetTotalTicks();
            }

            void Register()
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_insect_swarm_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_insect_swarm_AuraScript();
        }
};

// -33763 - Lifebloom
class spell_dru_lifebloom : public SpellScriptLoader
{
    public:
        spell_dru_lifebloom() : SpellScriptLoader("spell_dru_lifebloom") { }

        class spell_dru_lifebloom_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_lifebloom_AuraScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_DRUID_LIFEBLOOM_FINAL_HEAL))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_DRUID_LIFEBLOOM_ENERGIZE))
                    return false;
                return true;
            }

            void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                // Final heal only on duration end and dispel
                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE || GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL)
                {
                    // final heal
                    int32 stack = GetStackAmount();
                    int32 healAmount = aurEff->GetAmount();
                    if (Unit* caster = GetCaster())
                    {
                        healAmount = caster->SpellHealingBonusDone(GetTarget(), GetSpellInfo(), healAmount, DOT, 0);
                        healAmount = caster->SpellHealingBonusDone(GetTarget(), GetSpellInfo(), healAmount, HEAL, stack);
                        // Gift of the earthmother
                        if (AuraEffect* earthMother = caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 3186, EFFECT_1))
                            AddPct(healAmount, earthMother->GetAmount());

                        GetTarget()->CastCustomSpell(GetTarget(), SPELL_DRUID_LIFEBLOOM_FINAL_HEAL, &healAmount, NULL, NULL, true, NULL, aurEff, GetCasterGUID());
                        return;
                    }

                    GetTarget()->CastCustomSpell(GetTarget(), SPELL_DRUID_LIFEBLOOM_FINAL_HEAL, &healAmount, NULL, NULL, true, NULL, aurEff, GetCasterGUID());
                }
            }

            void HandleDispel(DispelInfo* dispelInfo)
            {
                if (Unit* target = GetUnitOwner())
                {
                    if (AuraEffect const* aurEff = GetEffect(EFFECT_1))
                    {
                        // final heal
                        int32 healAmount = aurEff->GetAmount();
                        if (Unit* caster = GetCaster())
                        {
                            healAmount = caster->SpellHealingBonusDone(target, GetSpellInfo(), healAmount, DOT, 0);
                            healAmount = target->SpellHealingBonusDone(caster, GetSpellInfo(), healAmount, HEAL, dispelInfo->GetRemovedCharges());
                            target->CastCustomSpell(target, SPELL_DRUID_LIFEBLOOM_FINAL_HEAL, &healAmount, NULL, NULL, true, NULL, NULL, GetCasterGUID());
                            return;
                        }
                        target->CastCustomSpell(target, SPELL_DRUID_LIFEBLOOM_FINAL_HEAL, &healAmount, NULL, NULL, true, NULL, NULL, GetCasterGUID());
                    }
                }
            }

            void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
            {
                Unit* caster = GetCaster();
                Unit* target = GetTarget();
                if (!caster || !target || caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                // Revitalize - Replenishment
                if (caster->GetAuraEffect(SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE, SPELLFAMILY_DRUID, 2862, EFFECT_0))
                    caster->CastSpell(caster, 57669, true);

                if (mode == AURA_EFFECT_HANDLE_REAPPLY)
                    return;

                // Lifebloom is not limited to targets if Tree of life is active
                if (caster->GetDummyAuraEffect(SPELLFAMILY_DRUID, 2257, EFFECT_1) || !GetAura()->m_needsUnregister)
                    return;

                Player::appliedAurasList const& auras = caster->ToPlayer()->appliedAuras;
                ACE_Guard<ACE_Recursive_Thread_Mutex> g(caster->m_appliedAuraMutex);
                for (Player::appliedAurasList::const_iterator itr = auras.begin(); itr != auras.end();)
                {
                    Aura* aura = (*itr);
                    if (aura == GetAura() || !aura->m_needsUnregister)
                    {
                        itr++;
                        continue;
                    }
                    if (aura->GetId() == GetId())
                    {
                        itr++;
                        aura->Remove();
                    }
                    else
                        itr++;
                }
            }


            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dru_lifebloom_AuraScript::HandleApply, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_lifebloom_AuraScript::AfterRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                AfterDispel += AuraDispelFn(spell_dru_lifebloom_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_lifebloom_AuraScript();
        }
};

// Faerie Fire
class spell_dru_faerie_fire : public SpellScriptLoader
{
    public:
        spell_dru_faerie_fire() : SpellScriptLoader("spell_dru_faerie_fire") { }

        class spell_dru_faerie_fire_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_faerie_fire_AuraScript);

            void HandleDispel(DispelInfo* /*dispelInfo*/)
            {
                if (Unit* target = GetUnitOwner())
                    target->RemoveAura(GetId());
            }

            void Register() override
            {
                AfterDispel += AuraDispelFn(spell_dru_faerie_fire_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_faerie_fire_AuraScript();
        }
};

// 60089 Faerie Fire (Feral) (Level 85)
class spell_dru_faerie_fire_damage : public SpellScriptLoader
{
public:
    spell_dru_faerie_fire_damage() : SpellScriptLoader("spell_dru_faerie_fire_damage") { }

    class spell_dru_faerie_fire_damage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dru_faerie_fire_damage_SpellScript);

        void HandleAuraRemove(SpellEffIndex /*effIndex*/)
        {
            GetHitUnit()->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TAKE_DAMAGE2);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_dru_faerie_fire_damage_SpellScript::HandleAuraRemove, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dru_faerie_fire_damage_SpellScript();
    }
};

// -48496 - Living Seed
class spell_dru_living_seed : public SpellScriptLoader
{
    public:
        spell_dru_living_seed() : SpellScriptLoader("spell_dru_living_seed") { }

        class spell_dru_living_seed_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_living_seed_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_DRUID_LIVING_SEED_PROC))
                    return false;
                return true;
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                int32 amount = CalculatePct(eventInfo.GetHealInfo()->GetHeal(), aurEff->GetAmount());
                GetTarget()->CastCustomSpell(SPELL_DRUID_LIVING_SEED_PROC, SPELLVALUE_BASE_POINT0, amount, eventInfo.GetProcTarget(), true, NULL, aurEff);
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_dru_living_seed_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_living_seed_AuraScript();
        }
};

// 48504 - Living Seed (Proc)
class spell_dru_living_seed_proc : public SpellScriptLoader
{
    public:
        spell_dru_living_seed_proc() : SpellScriptLoader("spell_dru_living_seed_proc") { }

        class spell_dru_living_seed_proc_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_living_seed_proc_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_DRUID_LIVING_SEED_HEAL))
                    return false;
                return true;
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
            {
                PreventDefaultAction();
                GetTarget()->CastCustomSpell(SPELL_DRUID_LIVING_SEED_HEAL, SPELLVALUE_BASE_POINT0, aurEff->GetAmount(), GetTarget(), true, NULL, aurEff);
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_dru_living_seed_proc_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_living_seed_proc_AuraScript();
        }
};

// Frenzied regeneration
class spell_dru_frenzied_regen : public SpellScriptLoader
{
    public:
        spell_dru_frenzied_regen() : SpellScriptLoader("spell_dru_frenzied_regen") { }

        class spell_dru_frenzied_regen_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_frenzied_regen_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                amount = GetUnitOwner()->CountPctFromMaxHealth(amount);
            }

            void AfterApply(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                // t13 4p bonus
                if (GetCaster()->HasAura(105735))
                    GetCaster()->CastSpell(GetCaster(), 105737, false);
            }

            void Register()
            {
                if (m_scriptSpellId == 22842)
                    AfterEffectApply += AuraEffectApplyFn(spell_dru_frenzied_regen_AuraScript::AfterApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);

                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_frenzied_regen_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_INCREASE_HEALTH_2);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_frenzied_regen_AuraScript();
        }
};

// -16972 - Predatory Strikes
class spell_dru_predatory_strikes : public SpellScriptLoader
{
    public:
        spell_dru_predatory_strikes() : SpellScriptLoader("spell_dru_predatory_strikes") { }

        class spell_dru_predatory_strikes_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_predatory_strikes_AuraScript);

            void UpdateAmount(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* target = GetTarget()->ToPlayer())
                    target->UpdateAttackPowerAndDamage();
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dru_predatory_strikes_AuraScript::UpdateAmount, EFFECT_ALL, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_predatory_strikes_AuraScript::UpdateAmount, EFFECT_ALL, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_predatory_strikes_AuraScript();
        }
};

// 33851 - Primal Tenacity
class spell_dru_primal_tenacity : public SpellScriptLoader
{
    public:
        spell_dru_primal_tenacity() : SpellScriptLoader("spell_dru_primal_tenacity") { }

        class spell_dru_primal_tenacity_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_primal_tenacity_AuraScript);

            uint32 absorbPct;

            bool Load()
            {
                absorbPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue(GetCaster());
                return true;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                // reduces all damage taken while Stunned in Cat Form
                if (GetTarget()->GetShapeshiftForm() == FORM_CAT && GetTarget()->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED) && GetTarget()->HasAuraWithMechanic(1<<MECHANIC_STUN))
                    absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
            }

            void Register()
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_primal_tenacity_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_dru_primal_tenacity_AuraScript::Absorb, EFFECT_1);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_primal_tenacity_AuraScript();
        }
};

// -1079 - Rip
class spell_dru_rip : public SpellScriptLoader
{
    public:
        spell_dru_rip() : SpellScriptLoader("spell_dru_rip") { }

        class spell_dru_rip_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_rip_AuraScript);

            bool Load()
            {
                Unit* caster = GetCaster();
                return caster && caster->GetTypeId() == TYPEID_PLAYER;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
            {
                canBeRecalculated = false;

                if (Unit* caster = GetCaster())
                {
                    // 0.0207 * $AP * cp
                    uint8 cp = caster->ToPlayer()->GetComboPoints();
                    float coeff = cp * 0.0207f;
                    amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * coeff);
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_rip_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_rip_AuraScript();
        }
};

// - Rake
class spell_dru_rake : public SpellScriptLoader
{
    public:
        spell_dru_rake() : SpellScriptLoader("spell_dru_rake") { }

        class spell_dru_rake_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_rake_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
            {
                canBeRecalculated = false;

                if (Unit* caster = GetCaster())
                {
                    float coeff = 0.441f;
                    if (AuraEffect* carnage = caster->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_DRUID, 2725, EFFECT_0))
                        coeff = carnage->GetId() == 80314 ? 0.588f : 0.735f;

                    amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * coeff) / aurEff->GetTotalTicks();
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_rake_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_rake_AuraScript();
        }
};

// 52610 - Savage Roar
class spell_dru_savage_roar : public SpellScriptLoader
{
    public:
        spell_dru_savage_roar() : SpellScriptLoader("spell_dru_savage_roar") { }

        class spell_dru_savage_roar_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_savage_roar_SpellScript);

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (caster->GetShapeshiftForm() != FORM_CAT)
                    return SPELL_FAILED_ONLY_SHAPESHIFT;

                return SPELL_CAST_OK;
            }

            void HandleOnHit()
            {
                if (Player* player = GetHitUnit()->ToPlayer())
                {
                    if (player->HasAura(SPELL_DRUID_T12_FERAL_4P_BONUS))
                    {
                        int32 triggerChance = player->GetComboPoints() * 20;
                        if (roll_chance_i(triggerChance))
                            if (Aura* berserker = player->GetAura(SPELL_DRUID_BERSERK, player->GetGUID()))
                                berserker->SetDuration(berserker->GetDuration() + 2000);
                    }
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dru_savage_roar_SpellScript::CheckCast);
                OnHit += SpellHitFn(spell_dru_savage_roar_SpellScript::HandleOnHit);
            }
        };

        class spell_dru_savage_roar_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_savage_roar_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_DRUID_SAVAGE_ROAR))
                    return false;
                return true;
            }

            void AfterApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                target->CastSpell(target, SPELL_DRUID_SAVAGE_ROAR, true, NULL, aurEff, GetCasterGUID());
            }

            void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->RemoveAurasDueToSpell(SPELL_DRUID_SAVAGE_ROAR);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dru_savage_roar_AuraScript::AfterApply, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_savage_roar_AuraScript::AfterRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_savage_roar_SpellScript();
        }

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_savage_roar_AuraScript();
        }
};

// -50294 - Starfall (AOE)
class spell_dru_starfall_aoe : public SpellScriptLoader
{
    public:
        spell_dru_starfall_aoe() : SpellScriptLoader("spell_dru_starfall_aoe") { }

        class spell_dru_starfall_aoe_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_starfall_aoe_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targets.remove(GetExplTargetUnit());
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dru_starfall_aoe_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_starfall_aoe_SpellScript();
        }
};

// -50286 - Starfall (Dummy)
class spell_dru_starfall_dummy : public SpellScriptLoader
{
    public:
        spell_dru_starfall_dummy() : SpellScriptLoader("spell_dru_starfall_dummy") { }

        class spell_dru_starfall_dummy_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_starfall_dummy_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                Trinity::Containers::RandomResizeList(targets, 2);
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                // Shapeshifting into an animal form or mounting cancels the effect
                if (caster->GetCreatureType() == CREATURE_TYPE_BEAST || caster->IsMounted())
                {
                    if (SpellInfo const* spellInfo = GetTriggeringSpell())
                        caster->RemoveAurasDueToSpell(spellInfo->Id);
                    return;
                }

                // Hotfix (2011-08-04): "Starfall should no longer strike targets with which the druid is not in combat."
                // Hotfix (2011-07-01): "is not breaking crowd control effects"
                if (!GetHitUnit()->isInCombat() || GetHitUnit()->HasBreakableByDamageCrowdControlAura(caster))
                    return;

                // Any effect which causes you to lose control of your character will supress the starfall effect.
                if (caster->HasUnitState(UNIT_STATE_CONTROLLED))
                    return;

                caster->CastSpell(GetHitUnit(), uint32(GetEffectValue()), true);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dru_starfall_dummy_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_dru_starfall_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_starfall_dummy_SpellScript();
        }
};

// 61336 - Survival Instincts
class spell_dru_survival_instincts : public SpellScriptLoader
{
    public:
        spell_dru_survival_instincts() : SpellScriptLoader("spell_dru_survival_instincts") { }

        class spell_dru_survival_instincts_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_survival_instincts_SpellScript);

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (!caster->IsInFeralForm())
                    return SPELL_FAILED_ONLY_SHAPESHIFT;

                return SPELL_CAST_OK;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_dru_survival_instincts_SpellScript::CheckCast);
            }
        };

        class spell_dru_survival_instincts_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_survival_instincts_AuraScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_DRUID_SURVIVAL_INSTINCTS))
                    return false;
                return true;
            }

            void AfterApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                target->CastSpell(target, SPELL_DRUID_SURVIVAL_INSTINCTS, true);
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dru_survival_instincts_AuraScript::AfterApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dru_survival_instincts_SpellScript();
        }

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_survival_instincts_AuraScript();
        }
};

// 40121 - Swift Flight Form (Passive)
class spell_dru_swift_flight_passive : public SpellScriptLoader
{
    public:
        spell_dru_swift_flight_passive() : SpellScriptLoader("spell_dru_swift_flight_passive") { }

        class spell_dru_swift_flight_passive_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_swift_flight_passive_AuraScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                if (Player* caster = GetCaster()->ToPlayer())
                    if (caster->GetSkillValue(SKILL_RIDING) >= 375)
                        amount = 310;
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_swift_flight_passive_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_swift_flight_passive_AuraScript();
        }
};

// -5217 - Tiger's Fury
class spell_dru_tiger_s_fury : public SpellScriptLoader
{
    public:
        spell_dru_tiger_s_fury() : SpellScriptLoader("spell_dru_tiger_s_fury") { }

        class spell_dru_tiger_s_fury_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_tiger_s_fury_SpellScript);

            void OnHit()
            {
                // King of the jungle
                if (AuraEffect const* aurEff = GetHitUnit()->GetDummyAuraEffect(SPELLFAMILY_DRUID, 2850, EFFECT_1))
                    GetHitUnit()->CastCustomSpell(SPELL_DRUID_TIGER_S_FURY_ENERGIZE, SPELLVALUE_BASE_POINT0, aurEff->GetAmount(), GetHitUnit(), true);
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_dru_tiger_s_fury_SpellScript::OnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_tiger_s_fury_SpellScript();
        }
};

// -61391 - Typhoon
class spell_dru_typhoon : public SpellScriptLoader
{
    public:
        spell_dru_typhoon() : SpellScriptLoader("spell_dru_typhoon") { }

        class spell_dru_typhoon_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_typhoon_SpellScript);

            void HandleKnockBack(SpellEffIndex effIndex)
            {
                // Glyph of Typhoon
                if (GetCaster()->HasAura(SPELL_DRUID_GLYPH_OF_TYPHOON))
                    PreventHitDefaultEffect(effIndex);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dru_typhoon_SpellScript::HandleKnockBack, EFFECT_0, SPELL_EFFECT_KNOCK_BACK);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_typhoon_SpellScript();
        }
};

// 70691 - Item T10 Restoration 4P Bonus
class spell_dru_t10_restoration_4p_bonus : public SpellScriptLoader
{
    public:
        spell_dru_t10_restoration_4p_bonus() : SpellScriptLoader("spell_dru_t10_restoration_4p_bonus") { }

        class spell_dru_t10_restoration_4p_bonus_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_t10_restoration_4p_bonus_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster()->ToPlayer()->GetGroup())
                {
                    targets.clear();
                    targets.push_back(GetCaster());
                }
                else
                {
                    targets.remove(GetExplTargetUnit());
                    std::list<Unit*> tempTargets;
                    for (std::list<WorldObject*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                        if ((*itr)->GetTypeId() == TYPEID_PLAYER && GetCaster()->IsInRaidWith((*itr)->ToUnit()))
                            tempTargets.push_back((*itr)->ToUnit());

                    if (tempTargets.empty())
                    {
                        targets.clear();
                        FinishCast(SPELL_FAILED_DONT_REPORT);
                        return;
                    }

                    Unit* target = Trinity::Containers::SelectRandomContainerElement(tempTargets);
                    targets.clear();
                    targets.push_back(target);
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dru_t10_restoration_4p_bonus_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_t10_restoration_4p_bonus_SpellScript();
        }
};

// Thorns
class spell_dru_thorns : public SpellScriptLoader
{
    public:
        spell_dru_thorns() : SpellScriptLoader("spell_dru_thorns") { }

        class spell_dru_thorns_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_thorns_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    float coeff = 0.168f;
                    if (caster->HasSpell(33876))
                        amount = GetSpellInfo()->Effects[EFFECT_0].CalcValue(caster) + caster->GetTotalAttackPowerValue(BASE_ATTACK) * coeff;
                    else
                        amount = GetSpellInfo()->Effects[EFFECT_0].CalcValue(caster) + caster->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask()) * coeff;
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_thorns_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_DAMAGE_SHIELD);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_thorns_AuraScript();
        }
};

// Lacerate
class spell_dru_lacerate : public SpellScriptLoader
{
public:
    spell_dru_lacerate() : SpellScriptLoader("spell_dru_lacerate") { }

    class spell_dru_lacerate_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dru_lacerate_AuraScript);

        void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
        {
            Unit* caster = GetCaster();
            if (caster && caster->GetTypeId() == TYPEID_PLAYER && caster->ToPlayer()->HasSpell(SPELL_DRUID_BERSERK))
                if (rand() % 2)
                {
                    caster->AddAura(SPELL_DRUID_BERSERK_BEAR, caster);
                    caster->ToPlayer()->RemoveSpellCooldown(SPELL_DRUID_MANGLE_BEAR, true);
                }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_lacerate_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dru_lacerate_AuraScript();
    }
};

// Mangle Bear
class spell_dru_mangle : public SpellScriptLoader
{
public:
    spell_dru_mangle() : SpellScriptLoader("spell_dru_mangle") { }

    class spell_dru_mangle_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dru_mangle_SpellScript);

        void RemoveBerserk()
        {
            if (GetSpellInfo()->Id == 33878)
                GetCaster()->RemoveAura(SPELL_DRUID_BERSERK_BEAR);

            if (GetSpellInfo()->Id == 33876)
                if (GetCaster()->HasAura(90165)) // Druid T11 Feral 4P Bonus
                    GetCaster()->CastSpell(GetCaster(), 90166, true);
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_dru_mangle_SpellScript::RemoveBerserk);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dru_mangle_SpellScript();
    }
};

class spell_dru_glyph_healing_touch : public SpellScriptLoader
{
public:
    spell_dru_glyph_healing_touch() : SpellScriptLoader("spell_dru_glyph_healing_touch") { }

    class spell_dru_glyph_healing_touch_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dru_glyph_healing_touch_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_DRUID_NATURES_SWIFTNESS))
                return false;
            return true;
        }

        void OnProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
        {
            if (GetOwner()->GetTypeId() == TYPEID_PLAYER)
            {
                Player* player = GetOwner()->ToPlayer();
                uint32 cooldown = player->GetSpellCooldownDelay(SPELL_DRUID_NATURES_SWIFTNESS);
                if (cooldown)
                {
                    int32 cooldownMod = 0;
                    if (cooldown > (uint32)aurEff->GetBaseAmount())
                    {
                        cooldownMod = -aurEff->GetBaseAmount();
                        cooldown += cooldownMod;
                    }
                    else
                        cooldownMod = -(int32)cooldown;

                    player->AddSpellCooldown(SPELL_DRUID_NATURES_SWIFTNESS, 0, uint32(time(NULL) + cooldown));

                    WorldPacket data(SMSG_MODIFY_COOLDOWN, 4+8+4);
                    data << uint32(SPELL_DRUID_NATURES_SWIFTNESS);  // Spell ID
                    data << uint64(player->GetGUID());              // Player GUID
                    data << int32(cooldownMod * 1000);              // Cooldown mod in milliseconds
                    player->GetSession()->SendPacket(&data);
                }
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_dru_glyph_healing_touch_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dru_glyph_healing_touch_AuraScript();
    }
};

class spell_dru_regrowth : public SpellScriptLoader
{
public:
    spell_dru_regrowth() : SpellScriptLoader("spell_dru_regrowth") { }

    class spell_dru_regrowth_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dru_regrowth_AuraScript);

        bool Load()
        {
            Unit* caster = GetCaster();
            return caster && caster->HasAura(54743);
        }

        void OnPeriodic(AuraEffect const* /*aurEff*/)
        {
            if (Unit* target = GetTarget())
                if (target->GetHealthPct() <= 50)
                    if (Aura* aura = GetAura())
                        aura->RefreshDuration();
        }

        void AfterApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                // Druid T13 Restoration 4P Bonus (Rejuvenation)
                if (AuraEffect* t13_4p = caster->GetAuraEffect(105770, EFFECT_0, GetCasterGUID()))
                {
                    if (roll_chance_i(t13_4p->GetAmount()))
                    {
                        uint32 duration = GetAura()->CalcMaxDuration(caster) * 2;
                        GetAura()->SetMaxDuration(duration);
                        GetAura()->SetDuration(duration);
                    }
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_regrowth_AuraScript::OnPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_HEAL);
            AfterEffectApply += AuraEffectApplyFn(spell_dru_regrowth_AuraScript::AfterApply, EFFECT_1, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dru_regrowth_AuraScript();
    }
};

class spell_dru_t12_helper : public SpellScriptLoader
{
public:
    spell_dru_t12_helper() : SpellScriptLoader("spell_dru_t12_helper") { }

    class spell_dru_t12_helper_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dru_t12_helper_AuraScript);

        bool Validate(SpellInfo const* /*spell*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_DRUID_BERSERK))
                return false;
            return true;
        }

        void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
        {
            if (Player* player = GetTarget()->ToPlayer())
            {
                int32 triggerChance = player->GetComboPoints() * 20;
                if (roll_chance_i(triggerChance))
                    if (Aura* berserker = player->GetAura(SPELL_DRUID_BERSERK, player->GetGUID()))
                        berserker->SetDuration(berserker->GetDuration() + 2000);
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_dru_t12_helper_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dru_t12_helper_AuraScript();
    }
};

class spell_dru_blood_in_the_water : public SpellScriptLoader
{
public:
    spell_dru_blood_in_the_water() : SpellScriptLoader("spell_dru_blood_in_the_water") { }

    class spell_dru_blood_in_the_water_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dru_blood_in_the_water_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            if (Unit* target = eventInfo.GetActionTarget())
            {
                //  Druid T13 Feral 2P Bonus (Savage Defense and Blood In The Water)
                uint32 healthPct = eventInfo.GetActor()->HasAura(105725, eventInfo.GetActor()->GetGUID()) ? 60 : 25;
                return target->HealthBelowPct(healthPct);
            }
            return false;
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(spell_dru_blood_in_the_water_AuraScript::CheckProc);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dru_blood_in_the_water_AuraScript();
    }
};

class spell_dru_t13_2p_bear : public SpellScriptLoader
{
public:
    spell_dru_t13_2p_bear() : SpellScriptLoader("spell_dru_t13_2p_bear") { }

    class spell_dru_t13_2p_bear_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dru_t13_2p_bear_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            return eventInfo.GetActor() && eventInfo.GetActor()->HasAura(SPELL_DRUID_PULVERIZE_AURA);
        }

        void HandleEffectProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
        {
            eventInfo.GetActor()->CastSpell(eventInfo.GetActor(), SPELL_DRUID_SAVAGE_DEFENSE_TRIGGERED, true);
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(spell_dru_t13_2p_bear_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_dru_t13_2p_bear_AuraScript::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dru_t13_2p_bear_AuraScript();
    }
};

void AddSC_druid_spell_scripts()
{
    new spell_dru_dash(); // Verified
    new spell_dru_eclipse_energize(); // Verified
    new spell_dru_enrage(); // Verified
    new spell_dru_glyph_of_starfire(); // Verified
    new spell_dru_glyph_of_starfire_proc(); // Recently Verified
	new spell_dru_idol_lifebloom(); // Verified
	new spell_dru_insect_swarm(); // Verified
	new spell_dru_lifebloom(); // Verified
	new spell_dru_living_seed(); // Verified
	new spell_dru_living_seed_proc(); // Verified
	new spell_dru_predatory_strikes(); // Verified
    new spell_dru_primal_tenacity(); // Recently Verified
    new spell_dru_rip(); // Verified
    new spell_dru_savage_roar(); // Verified
    new spell_dru_starfall_aoe(); // Recently Verified
	new spell_dru_starfall_dummy(); // Verified
	new spell_dru_survival_instincts(); // Verified
	new spell_dru_swift_flight_passive(); // Verified
	new spell_dru_tiger_s_fury(); // Verified // Verified
    new spell_dru_typhoon();
	new spell_dru_t10_restoration_4p_bonus(); // Verified
	new spell_dru_rejuv(); // Verified
	new spell_dru_emp_touch(); // Verified
	new spell_dru_emp_touch_triggered(); // Verified
	new spell_sha_efflorescence(); // Verified
	new spell_sha_efflorescence_heal(); // Verified
	new spell_dru_harmony(); // Verified
	new spell_dru_feral_swiftness(); // Verified
	new spell_dru_feral_aggression(); // Verified
	new spell_dru_stampede(); // Verified
	new spell_dru_skull_bash(); // Verified
	new spell_dru_primal_madness(); // Verified
	new spell_dru_pulverize(); // Verified
	new spell_dru_innervate(); // Verified
	new spell_dru_beam(); // Verified
	new spell_dru_wild_mushroom(); // Verified // Verified
    new spell_dru_wild_mushroom_detonation();
	new spell_dru_rake(); // Verified
	new spell_dru_frenzied_regen(); // Verified
	new spell_dru_barkskin(); // Verified
	new spell_dru_energize(); // Verified
	new spell_dru_starsurge(); // Verified
	new spell_dru_faerie_fire(); // Verified
    new spell_dru_faerie_fire_damage(); // Recently Verified
    new spell_dru_thorns(); // Verified
    new spell_dru_lacerate(); // Verified
    new spell_dru_mangle(); // Recently Verified
    new spell_dru_glyph_healing_touch(); // Recently Verified
    new spell_dru_regrowth(); // Recently Verified
    new spell_dru_t12_helper(); // Recently Verified
    new spell_dru_blood_in_the_water(); // Recently Verified
    new spell_dru_t13_2p_bear(); // Recently Verified
}
