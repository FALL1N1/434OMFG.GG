/*
* Copyright (C) 2013-2014 OMFG.GG Network <http://www.omfg.gg/>
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

#include "Player.h"
#include "Pet.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

enum DeathKnightSpells
{
    SPELL_DK_ANTI_MAGIC_SHELL_TALENT = 51052,
    SPELL_DK_BLOOD_BOIL_TRIGGERED = 65658,
    SPELL_DK_BLOOD_GORGED_HEAL = 50454,
    SPELL_DK_DEATH_COIL_DAMAGE = 47632,
    SPELL_DK_DEATH_COIL_HEAL = 47633,
    SPELL_DK_DEATH_STRIKE_HEAL = 45470,
    SPELL_DK_GHOUL_EXPLODE = 47496,
    SPELL_DK_GLYPH_OF_ICEBOUND_FORTITUDE = 58625,
    SPELL_DK_RUNIC_POWER_ENERGIZE = 49088,
    SPELL_DK_SCOURGE_STRIKE_TRIGGERED = 70890,
    SPELL_DK_WILL_OF_THE_NECROPOLIS_TALENT_R1 = 49189,
    SPELL_DK_WILL_OF_THE_NECROPOLIS_AURA_R1 = 52284,
    SPELL_DK_UNHOLY_PRESENCE = 48265,
    SPELL_DK_IMPROVED_UNHOLY_PRESENCE_TRIGGERED = 63622,
    SPELL_DK_ITEM_SIGIL_VENGEFUL_HEART = 64962,
    SPELL_DK_ITEM_T8_MELEE_4P_BONUS = 64736,
    SPELL_DK_GLYPH_OF_SCOURGE_STRIKE = 58642,
    SPELL_DK_VAMPIRIC_BROOD = 105588,
    SPELL_DK_BLOOD_PRESENCE = 48263,
	SPELL_DK_DEATH_STRIKE_DAMAGE= 49998
};

enum DeathKnightSpellIcons
{
    DK_ICON_ID_IMPROVED_DEATH_STRIKE = 2751
};

enum DKPetCalculate
{
    SPELL_DEATH_KNIGHT_RUNE_WEAPON_02 = 51906,
    SPELL_DEATH_KNIGHT_PET_SCALING_01 = 54566,
    SPELL_DEATH_KNIGHT_PET_SCALING_02 = 51996,
    SPELL_DEATH_KNIGHT_PET_SCALING_03 = 61697,
    SPELL_NIGHT_OF_THE_DEAD = 55620,
    ENTRY_ARMY_OF_THE_DEAD_GHOUL = 24207,
    SPELL_DEATH_KNIGHT_GLYPH_OF_GHOUL = 58686,
};

enum spellScriptEvents
{
    EVENT_ENABLE_SPELL_PROC = 1,
};

// 50462 - Anti-Magic Shell (on raid member)
class spell_dk_anti_magic_shell_raid : public SpellScriptLoader
{
public:
    spell_dk_anti_magic_shell_raid() : SpellScriptLoader("spell_dk_anti_magic_shell_raid") { }

    class spell_dk_anti_magic_shell_raid_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_anti_magic_shell_raid_AuraScript);

        uint32 absorbPct;

        bool Load()
        {
            absorbPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue(GetCaster());
            return true;
        }

        void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
        {
            // TODO: this should absorb limited amount of damage, but no info on calculation formula
            amount = -1;
        }

        void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, uint32 & absorbAmount)
        {
            absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_anti_magic_shell_raid_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_shell_raid_AuraScript::Absorb, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_anti_magic_shell_raid_AuraScript();
    }
};

// Necrotic strike
class spell_dk_necrotic : public SpellScriptLoader
{
public:
    spell_dk_necrotic() : SpellScriptLoader("spell_dk_necrotic") { }

    class spell_dk_necrotic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_necrotic_AuraScript);

        void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
        {
            if (Unit* caster = GetCaster())
            {
                int32 absorbAmount = caster->GetTotalAttackPowerValue(BASE_ATTACK) * 0.7f;
                caster->ApplyResilience(GetUnitOwner(), &absorbAmount);
                if (AuraEffect* necrotic = GetUnitOwner()->GetAuraEffect(73975, EFFECT_0, caster->GetGUID()))
                    absorbAmount += necrotic->GetAmount();
                amount = int32(absorbAmount);
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_necrotic_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_HEAL_ABSORB);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_necrotic_AuraScript();
    }
};

// Festering strike
class spell_dk_festering : public SpellScriptLoader
{
public:
    spell_dk_festering() : SpellScriptLoader("spell_dk_festering") { }

    class spell_dk_festering_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_festering_SpellScript);

        void OnHitTarget(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            Unit* target = GetHitUnit();

            int32 newDuration = GetSpellInfo()->Effects[EFFECT_2].BasePoints * 1000;
            // Increase chains of ice
            if (Aura* aura = target->GetAura(45524, caster->GetGUID()))
                aura->SetDuration(newDuration + aura->GetDuration());
            // Increase Blood plague
            if (Aura* aura = target->GetAura(55078, caster->GetGUID()))
                aura->SetDuration(newDuration + aura->GetDuration());
            // Increase frost fever
            if (Aura* aura = target->GetAura(55095, caster->GetGUID()))
                aura->SetDuration(newDuration + aura->GetDuration());
            // Increase Ebon Plaguebringer
            if (Aura* aura = target->GetAura(65142, caster->GetGUID()))
                aura->SetDuration(newDuration + aura->GetDuration());
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_dk_festering_SpellScript::OnHitTarget, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_dk_festering_SpellScript;
    }
};

// Chilblains
class spell_dk_chilblains : public SpellScriptLoader
{
public:
    spell_dk_chilblains() : SpellScriptLoader("spell_dk_chilblains") { }

    class spell_dk_chilblains_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_chilblains_SpellScript);

        void OnHitTarget(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            Unit* target = GetHitUnit();
            if (AuraEffect* chilblains = caster->GetAuraEffect(SPELL_AURA_ADD_TARGET_TRIGGER, SPELLFAMILY_DEATHKNIGHT, 143, EFFECT_0))
            {
                uint32 triggeredId = 0;
                switch (chilblains->GetId())
                {
                case 50040:
                    triggeredId = 96293;
                    break;
                case 50041:
                    triggeredId = 96294;
                    break;
                }
                caster->CastSpell(target, triggeredId, true);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_dk_chilblains_SpellScript::OnHitTarget, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_chilblains_SpellScript;
    }
};

// Pillar of frost
class spell_dk_pillar_of_frost : public SpellScriptLoader
{
public:
    spell_dk_pillar_of_frost() : SpellScriptLoader("spell_dk_pillar_of_frost") { }

    class spell_dk_pillar_of_frost_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_pillar_of_frost_AuraScript);

        void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            if (target->GetDummyAuraEffect(SPELLFAMILY_DEATHKNIGHT, 4919, EFFECT_0))
                target->CastSpell(target, 90259, true);
        }

        void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            GetTarget()->RemoveAurasDueToSpell(90259);
        }

        void Register()
        {
            AfterEffectApply += AuraEffectApplyFn(spell_dk_pillar_of_frost_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            AfterEffectRemove += AuraEffectRemoveFn(spell_dk_pillar_of_frost_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_pillar_of_frost_AuraScript();
    }
};

// Strangulate
class spell_dk_strangulate : public SpellScriptLoader
{
public:
    spell_dk_strangulate() : SpellScriptLoader("spell_dk_strangulate") { }

    class spell_dk_strangulate_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_strangulate_SpellScript);

        bool Load() override
        {
            _wasCasting = false;
            return true;
        }

        void HandleBeforeHit()
        {
            if (GetHitUnit())
                _wasCasting = GetHitUnit()->IsNonMeleeSpellCasted(false, false, true);
        }

        void OnHitTarget(SpellEffIndex /*effIndex*/)
        {
            if (!_wasCasting)
                return;

            Unit* caster = GetCaster();
            if (AuraEffect* glyph = caster->GetDummyAuraEffect(SPELLFAMILY_DEATHKNIGHT, 2027, EFFECT_0))
                if (Aura* strangulate = GetHitAura())
                {
                    strangulate->SetMaxDuration(strangulate->GetDuration() + glyph->GetAmount());
                    strangulate->SetDuration(strangulate->GetDuration() + glyph->GetAmount());
                }
        }

        void Register() override
        {
            BeforeHit += SpellHitFn(spell_dk_strangulate_SpellScript::HandleBeforeHit);
            OnEffectHitTarget += SpellEffectFn(spell_dk_strangulate_SpellScript::OnHitTarget, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        }

    private:
        bool _wasCasting;
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_dk_strangulate_SpellScript;
    }
};

// 48707 - Anti-Magic Shell (on self)
class spell_dk_anti_magic_shell_self : public SpellScriptLoader
{
public:
    spell_dk_anti_magic_shell_self() : SpellScriptLoader("spell_dk_anti_magic_shell_self") { }

    class spell_dk_anti_magic_shell_self_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_anti_magic_shell_self_AuraScript);

        uint32 hpPct, totalAbsorb;
        bool Load()
        {
            hpPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue(GetCaster());
            totalAbsorb = GetCaster()->CountPctFromMaxHealth(hpPct);
            return true;
        }

        void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, uint32 & absorbAmount)
        {
            PreventDefaultAction();
            absorbAmount = std::min(CalculatePct(dmgInfo.GetDamage(), absorbAmount), GetTarget()->CountPctFromMaxHealth(hpPct));
            if (absorbAmount > totalAbsorb)
            {
                absorbAmount = totalAbsorb;
                aurEff->GetBase()->Remove();
            }
            else
                totalAbsorb -= absorbAmount;

            dmgInfo.AbsorbDamage(absorbAmount);

            Unit* target = GetTarget();
            // Magic Suppression
            if (AuraEffect* aurEff = target->GetAuraEffectOfRankedSpell(49224, EFFECT_0))
            {
                // max amount is guessed - absorb part is confirmed
                int32 energizeAmount = std::min(aurEff->GetAmount() * 40, int32(absorbAmount / 10));
                target->CastCustomSpell(target, SPELL_DK_RUNIC_POWER_ENERGIZE, &energizeAmount, NULL, NULL, true, NULL, aurEff);
            }
        }

        void Register()
        {
            OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_shell_self_AuraScript::Absorb, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_anti_magic_shell_self_AuraScript();
    }
};

// 50461 - Anti-Magic Zone
class spell_dk_anti_magic_zone : public SpellScriptLoader
{
public:
    spell_dk_anti_magic_zone() : SpellScriptLoader("spell_dk_anti_magic_zone") { }

    class spell_dk_anti_magic_zone_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_anti_magic_zone_AuraScript);

        uint32 absorbPct;

        bool Load()
        {
            absorbPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue(GetCaster());
            return true;
        }

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_DK_ANTI_MAGIC_SHELL_TALENT))
                return false;
            return true;
        }

        void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
        {
            if (!GetCaster()->GetOwner())
                return;

            SpellInfo const* talentSpell = sSpellMgr->GetSpellInfo(SPELL_DK_ANTI_MAGIC_SHELL_TALENT);
            amount = talentSpell->Effects[EFFECT_0].CalcValue(GetCaster());
            if (Unit* owner = GetCaster()->GetOwner())
                amount += int32(2 * owner->GetTotalAttackPowerValue(BASE_ATTACK));
        }

        void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, uint32 & absorbAmount)
        {
            absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_anti_magic_zone_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_zone_AuraScript::Absorb, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_anti_magic_zone_AuraScript();
    }
};

// 48721 - Blood Boil
class spell_dk_blood_boil : public SpellScriptLoader
{
public:
    spell_dk_blood_boil() : SpellScriptLoader("spell_dk_blood_boil") { }

    class spell_dk_blood_boil_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_blood_boil_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_DK_BLOOD_BOIL_TRIGGERED))
                return false;
            return true;
        }

        bool Load()
        {
            _executed = false;
            return GetCaster()->GetTypeId() == TYPEID_PLAYER && GetCaster()->getClass() == CLASS_DEATH_KNIGHT;
        }

        void HandleAfterHit()
        {
            if (_executed || !GetHitUnit())
                return;

            _executed = true;
            GetCaster()->CastSpell(GetCaster(), SPELL_DK_BLOOD_BOIL_TRIGGERED, true);
        }

        void Register()
        {
            AfterHit += SpellHitFn(spell_dk_blood_boil_SpellScript::HandleAfterHit);
        }

        bool _executed;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_blood_boil_SpellScript();
    }
};

// Improved Unholy presence
class spell_dk_imp_presences : public SpellScriptLoader
{
public:
    spell_dk_imp_presences() : SpellScriptLoader("spell_dk_imp_presences") { }

    class spell_dk_imp_presences_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_imp_presences_SpellScript);

        void HandleOnHit()
        {
            GetCaster()->SetPower(POWER_RUNIC_POWER, 0);
            // Improved Unholy Presence
            if (AuraEffect* aur = GetCaster()->GetDummyAuraEffect(SPELLFAMILY_DEATHKNIGHT, 2633, EFFECT_0))
            {
                if (aur->GetId() == 50392 || aur->GetId() == 50391)
                {
                    int32 bp0 = aur->GetAmount();
                    GetCaster()->CastCustomSpell(GetCaster(), 63622, &bp0, NULL, NULL, true);
                }
            }
            else
                GetCaster()->RemoveAurasDueToSpell(63622);
            // Improved Blood Presence
            if (AuraEffect* aur = GetCaster()->GetDummyAuraEffect(SPELLFAMILY_GENERIC, 2636, EFFECT_2))
            {
                if (aur->GetId() == 50371 || aur->GetId() == 50365)
                {
                    int32 bp0 = aur->GetAmount();
                    GetCaster()->CastCustomSpell(GetCaster(), 63611, &bp0, NULL, NULL, true);
                }
            }
            else
                GetCaster()->RemoveAurasDueToSpell(63611);

            // Improved Frost Presence
            if (AuraEffect* aur = GetCaster()->GetDummyAuraEffect(SPELLFAMILY_DEATHKNIGHT, 2632, EFFECT_0))
            {
                if (aur->GetId() == 50385 || aur->GetId() == 50384)
                {
                    int32 bp0 = aur->GetAmount();
                    GetCaster()->CastCustomSpell(GetCaster(), 63621, &bp0, NULL, NULL, true);
                }
            }
            else
                GetCaster()->RemoveAurasDueToSpell(63621);
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_dk_imp_presences_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_imp_presences_SpellScript();
    }
};

// Dark transformation
class spell_dk_dark_transformation : public SpellScriptLoader
{
public:
    spell_dk_dark_transformation() : SpellScriptLoader("spell_dk_dark_transformation") { }

    class spell_dk_dark_transformation_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_dark_transformation_SpellScript);

        void HandleOnHit()
        {
            GetHitUnit()->RemoveAurasDueToSpell(91342);
            GetCaster()->RemoveAurasDueToSpell(93426);
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_dk_dark_transformation_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_dark_transformation_SpellScript();
    }
};

// Pestilence
class spell_dk_pestilence : public SpellScriptLoader
{
public:
    spell_dk_pestilence() : SpellScriptLoader("spell_dk_pestilence") { }

    class spell_dk_pestilence_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_pestilence_SpellScript);

        void HandleBeforeHit()
        {
            GetCaster()->AddAura(76243, GetCaster());
        }

        void HandleAfterHit()
        {
            GetCaster()->RemoveAurasDueToSpell(76243);
        }

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (Unit* target = GetHitUnit())
            {
                if (target != GetExplTargetUnit())
                {
                    if (GetExplTargetUnit()->HasAura(55078, GetCaster()->GetGUID()))
                        GetCaster()->CastSpell(target, 55078, true);
                    if (GetExplTargetUnit()->HasAura(55095, GetCaster()->GetGUID()))
                        GetCaster()->CastSpell(target, 55095, true);
                    if (GetExplTargetUnit()->HasAura(65142, GetCaster()->GetGUID()))
                        GetCaster()->CastSpell(target, 65142, true);
                    GetExplTargetUnit()->CastSpell(target, 91939, true);
                }
            }
        }

        void Register() override
        {
            BeforeHit += SpellHitFn(spell_dk_pestilence_SpellScript::HandleBeforeHit);
            AfterHit += SpellHitFn(spell_dk_pestilence_SpellScript::HandleAfterHit);
            OnEffectHitTarget += SpellEffectFn(spell_dk_pestilence_SpellScript::HandleScript, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_dk_pestilence_SpellScript();
    }
};

// Death's advance
class spell_dk_deaths_advance : public SpellScriptLoader
{
public:
    spell_dk_deaths_advance() : SpellScriptLoader("spell_dk_deaths_advance") { }

    class spell_dk_deaths_advance_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_deaths_advance_AuraScript);

        void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
        {
            Player* player = GetCaster()->ToPlayer();
            if (!player)
                return;

            uint8 unholyRunesOnCooldown = 0;
            for (uint32 i = 2; i < 4; ++i)
                if (player->GetRuneCooldown(i))
                    unholyRunesOnCooldown++;
            if (unholyRunesOnCooldown == 2)
                RefreshDuration();
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_deaths_advance_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_deaths_advance_AuraScript();
    }
};

// 50453 - Bloodworms Health Leech
class spell_dk_blood_gorged : public SpellScriptLoader
{
public:
    spell_dk_blood_gorged() : SpellScriptLoader("spell_dk_blood_gorged") { }

    class spell_dk_blood_gorged_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_blood_gorged_AuraScript);

        void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
        {
            Unit* target = GetTarget();
            if (Aura* aur = target->GetAura(81277))
                if (roll_chance_i(aur->GetStackAmount() * 10))
                {
                    target->CastSpell((Unit*)NULL, 81280, true);
                    if (Creature* creature = target->ToCreature())
                        creature->DespawnOrUnsummon(1000);
                }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_dk_blood_gorged_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_blood_gorged_AuraScript();
    }
};

// Resilient Infection
class spell_dk_resilient_infection : public SpellScriptLoader
{
public:
    spell_dk_resilient_infection() : SpellScriptLoader("spell_sha_resilient_infection") { }

    class spell_dk_resilient_infection_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_resilient_infection_AuraScript);

        void HandleDispel(DispelInfo* /*dispelInfo*/)
        {
            if (Unit* caster = GetCaster())
            {
                // Resilient Infection
                if (AuraEffect const* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_GENERIC, 1910, EFFECT_0))
                    if (roll_chance_i(aurEff->GetAmount()))
                    {
                        int32 bp0 = 0;
                        int32 bp1 = 0;
                        if (GetId() == 55095)
                            bp0 = 1;
                        else
                            bp1 = 1;

                        caster->CastCustomSpell(caster, 90721, &bp0, &bp1, NULL, true);
                    }
            }
        }

        void Register()
        {
            AfterDispel += AuraDispelFn(spell_dk_resilient_infection_AuraScript::HandleDispel);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_resilient_infection_AuraScript();
    }
};

// -47541, 52375, 59134, -62900 - Death Coil
class spell_dk_death_coil : public SpellScriptLoader
{
public:
    spell_dk_death_coil() : SpellScriptLoader("spell_dk_death_coil") { }

    class spell_dk_death_coil_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_death_coil_SpellScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_DK_DEATH_COIL_DAMAGE) || !sSpellMgr->GetSpellInfo(SPELL_DK_DEATH_COIL_HEAL))
                return false;
            return true;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            int32 damage = GetEffectValue();
            Unit* caster = GetCaster();
            if (Unit* target = GetHitUnit())
            {
                if (caster->IsFriendlyTo(target))
                {
                    int32 bp = int32(damage * 3.5f);
                    caster->CastCustomSpell(target, SPELL_DK_DEATH_COIL_HEAL, &bp, NULL, NULL, true);
                }
                else
                {
                    if (AuraEffect const* auraEffect = caster->GetAuraEffect(SPELL_DK_ITEM_SIGIL_VENGEFUL_HEART, EFFECT_1))
                        damage += auraEffect->GetBaseAmount();
                    caster->CastCustomSpell(target, SPELL_DK_DEATH_COIL_DAMAGE, &damage, NULL, NULL, true);
                }
            }
        }

        SpellCastResult CheckCast()
        {
            Unit* caster = GetCaster();
            if (Unit* target = GetExplTargetUnit())
            {
                if (caster->GetDistance2d(target) > 30.0f)
                    return SPELL_FAILED_OUT_OF_RANGE;

                if (!caster->IsFriendlyTo(target) && !caster->isInFront(target, M_PI))
                    return SPELL_FAILED_UNIT_NOT_INFRONT;

                if (target->IsFriendlyTo(caster) && target->GetCreatureType() != CREATURE_TYPE_UNDEAD)
                    return SPELL_FAILED_BAD_TARGETS;
            }
            else
                return SPELL_FAILED_BAD_TARGETS;

            return SPELL_CAST_OK;
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_dk_death_coil_SpellScript::CheckCast);
            OnEffectHitTarget += SpellEffectFn(spell_dk_death_coil_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_death_coil_SpellScript();
    }
};

// 52751 - Death Gate
class spell_dk_death_gate : public SpellScriptLoader
{
public:
    spell_dk_death_gate() : SpellScriptLoader("spell_dk_death_gate") {}

    class spell_dk_death_gate_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_death_gate_SpellScript);

        SpellCastResult CheckClass()
        {
            if (GetCaster()->getClass() != CLASS_DEATH_KNIGHT)
            {
                SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_MUST_BE_DEATH_KNIGHT);
                return SPELL_FAILED_CUSTOM_ERROR;
            }

            return SPELL_CAST_OK;
        }

        void HandleScript(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            if (Unit* target = GetHitUnit())
                target->CastSpell(target, GetEffectValue(), false);
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_dk_death_gate_SpellScript::CheckClass);
            OnEffectHitTarget += SpellEffectFn(spell_dk_death_gate_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_death_gate_SpellScript();
    }
};

// 49560 - Death Grip
class spell_dk_death_grip : public SpellScriptLoader
{
public:
    spell_dk_death_grip() : SpellScriptLoader("spell_dk_death_grip") { }

    class spell_dk_death_grip_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_death_grip_SpellScript);

        SpellCastResult CheckPvPRange()
        {
            Unit* caster = GetCaster();
            if (Unit* target = GetExplTargetUnit())
                // Mainaz: should be 8.0f, but we have to add target size (1.5f)
                if (target->GetTypeId() == TYPEID_PLAYER && caster->GetExactDist(target) < 8.0f)
                    return SPELL_FAILED_TOO_CLOSE;

            return SPELL_CAST_OK;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            int32 damage = GetEffectValue();
            Position const* pos = GetExplTargetDest();
            if (Unit* target = GetHitUnit())
                if (!target->HasAuraType(SPELL_AURA_DEFLECT_SPELLS)) // Deterrence
                    target->CastSpell(pos->GetPositionX(), pos->GetPositionY(), pos->GetPositionZ(), damage, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_dk_death_grip_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            OnCheckCast += SpellCheckCastFn(spell_dk_death_grip_SpellScript::CheckPvPRange);
        }

    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_death_grip_SpellScript();
    }
};

// 48743 - Death Pact
class spell_dk_death_pact : public SpellScriptLoader
{
public:
    spell_dk_death_pact() : SpellScriptLoader("spell_dk_death_pact") { }

    class spell_dk_death_pact_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_death_pact_SpellScript);

        SpellCastResult CheckCast()
        {
            // Check if we have valid targets, otherwise skip spell casting here
            if (Player* player = GetCaster()->ToPlayer())
                for (Unit::ControlList::const_iterator itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
                    if (Creature* undeadPet = (*itr)->ToCreature())
                        if (undeadPet->isAlive() &&
                            undeadPet->GetOwnerGUID() == player->GetGUID() &&
                            undeadPet->GetCreatureType() == CREATURE_TYPE_UNDEAD &&
                            undeadPet->IsWithinDist(player, 100.0f, false))
                        {
                            if (undeadPet->HasUnitState(UNIT_STATE_CONTROLLED))
                                return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
                            else
                                return SPELL_CAST_OK;
                        }

            return SPELL_FAILED_NO_PET;
        }

        void FilterTargets(std::list<WorldObject*>& targetList)
        {
            Unit* target = NULL;
            if (Unit* selected = GetCaster()->ToPlayer()->GetSelectedUnit())
                if (selected->isAlive() && selected->GetOwnerGUID() == GetCaster()->GetGUID() && selected->GetCreatureType() == CREATURE_TYPE_UNDEAD)
                    target = selected;

            if (!target)
            {
                for (std::list<WorldObject*>::iterator itr = targetList.begin(); itr != targetList.end(); ++itr)
                {
                    if (Unit* unit = (*itr)->ToUnit())
                        if (unit->isAlive() && unit->GetOwnerGUID() == GetCaster()->GetGUID() && unit->GetCreatureType() == CREATURE_TYPE_UNDEAD)
                        {
                            if (unit->GetEntry() == 26125) //Ghoul 
                            {
                                Unit* ghoul = unit;
                                target = ghoul;
                            }
                            else if (unit->GetEntry() == 27829) //gargyole
                            {
                                Unit* gargoyle = unit;
                                target = gargoyle;
                                break;
                            }
                        }
                }
            }

            targetList.clear();
            if (target)
                targetList.push_back(target);
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_dk_death_pact_SpellScript::CheckCast);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dk_death_pact_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ALLY);
        }

    private:
        Unit* target;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_death_pact_SpellScript();
    }
};

// -49998 - Death Strike
class spell_dk_death_strike : public SpellScriptLoader
{
public:
    spell_dk_death_strike() : SpellScriptLoader("spell_dk_death_strike") { }

    class spell_dk_death_strike_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_death_strike_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_DK_DEATH_STRIKE_HEAL))
                return false;
            return true;
        }

        void HandleOnHit()
        {
            Unit* caster = GetCaster();
            if (Unit* target = GetHitUnit())
            {
				// Mainaz: For some reason it seems that immunity check is not triggered for Death strike
				SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_DK_DEATH_STRIKE_DAMAGE);
				if (target->IsImmunedToDamage(spellInfo))
					return;

                healAmount = 0;

                uint32 count = target->GetDiseasesByCaster(caster->GetGUID());
                healAmount = caster->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_2].BasePoints);
                // 20% of damage taken in the last 5 seconds
                if (AuraEffect* aurEff = caster->GetAuraEffect(89832, EFFECT_0))
                    healAmount += aurEff->GetAmount() * 0.2f;
                // Improved Death Strike
                if (AuraEffect const* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_DEATHKNIGHT, DK_ICON_ID_IMPROVED_DEATH_STRIKE, EFFECT_2))
                    AddPct(healAmount, aurEff->GetAmount());
                // Dark Succor
                if (healAmount < (int32)(caster->GetMaxHealth() / 5))
                {
                    if (Aura* succor = caster->GetAura(101568))
                    {
                        if (!caster->HasAura(48263))
                        {
                            healAmount = caster->GetMaxHealth() / 5;
                            caster->RemoveAura(succor);
                        }
                    }
                }

                caster->CastCustomSpell(caster, SPELL_DK_DEATH_STRIKE_HEAL, &healAmount, NULL, NULL, false);
            }
        }

        void HandleAfterHit()
        {
            Unit* caster = GetCaster();
            if (Unit* unitTarget = GetHitUnit())
            {
                // Mastery adds blood shield when in blood presence
                if (AuraEffect const* mastery = caster->GetDummyAuraEffect(SPELLFAMILY_DEATHKNIGHT, 2624, EFFECT_0))
                    if (caster->HasAura(48263))
                    {
                        int32 shieldPct = 50.0f + mastery->GetAmount();
                        int32 shieldValue = healAmount * shieldPct / 100.0f;

                        caster->CastCustomSpell(caster, 77535, &shieldValue, NULL, NULL, true);
                    }
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_dk_death_strike_SpellScript::HandleOnHit);
            AfterHit += SpellHitFn(spell_dk_death_strike_SpellScript::HandleAfterHit);
        }

    private:
        int32 healAmount;

    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_death_strike_SpellScript();
    }
};

// 59754 Rune Tap - Party
class spell_dk_rune_tap_party : public SpellScriptLoader
{
public:
    spell_dk_rune_tap_party() : SpellScriptLoader("spell_dk_rune_tap_party") { }

    class spell_dk_rune_tap_party_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_rune_tap_party_SpellScript);

        void CheckTargets(std::list<WorldObject*>& targets)
        {
            targets.remove(GetCaster());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dk_rune_tap_party_SpellScript::CheckTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_PARTY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_rune_tap_party_SpellScript();
    }
};

// 55090 - Scourge Strike (55265, 55270, 55271)
class spell_dk_scourge_strike : public SpellScriptLoader
{
public:
    spell_dk_scourge_strike() : SpellScriptLoader("spell_dk_scourge_strike") { }

    class spell_dk_scourge_strike_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_scourge_strike_SpellScript);
        float multiplier;

        bool Load()
        {
            multiplier = 1.0f;
            return true;
        }

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_DK_SCOURGE_STRIKE_TRIGGERED))
                return false;
            return true;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            if (Unit* unitTarget = GetHitUnit())
            {
                multiplier = (GetEffectValue() * unitTarget->GetDiseasesByCaster(caster->GetGUID()) / 100.f);
                // Death Knight T8 Melee 4P Bonus
                if (AuraEffect const* aurEff = caster->GetAuraEffect(SPELL_DK_ITEM_T8_MELEE_4P_BONUS, EFFECT_0))
                    AddPct(multiplier, aurEff->GetAmount());
                // Glyph of Scourge Strike
                if (AuraEffect const* glyph = caster->GetAuraEffect(SPELL_DK_GLYPH_OF_SCOURGE_STRIKE, EFFECT_0))
                    AddPct(multiplier, glyph->GetAmount());
            }
        }

        void HandleAfterHit()
        {
            Unit* caster = GetCaster();
            if (Unit* unitTarget = GetHitUnit())
            {
                int32 bp = GetTrueDamage() * multiplier;

                // Unholy Mastery
                if (AuraEffect const* aurEff = caster->GetAuraEffect(77515, EFFECT_0))
                    AddPct(bp, aurEff->GetAmount());

                caster->CastCustomSpell(unitTarget, SPELL_DK_SCOURGE_STRIKE_TRIGGERED, &bp, NULL, NULL, true);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_dk_scourge_strike_SpellScript::HandleDummy, EFFECT_2, SPELL_EFFECT_DUMMY);
            AfterHit += SpellHitFn(spell_dk_scourge_strike_SpellScript::HandleAfterHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_scourge_strike_SpellScript();
    }
};

// 49145 - Spell Deflection
class spell_dk_spell_deflection : public SpellScriptLoader
{
public:
    spell_dk_spell_deflection() : SpellScriptLoader("spell_dk_spell_deflection") { }

    class spell_dk_spell_deflection_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_spell_deflection_AuraScript);

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
            // You have a chance equal to your Parry chance
            if ((dmgInfo.GetDamageType() == SPELL_DIRECT_DAMAGE) && roll_chance_f(GetTarget()->GetUnitParryChance()))
                absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_spell_deflection_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_spell_deflection_AuraScript::Absorb, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_spell_deflection_AuraScript();
    }
};

// 55233 - Vampiric Blood, 105588 - Vampiric Brood
class spell_dk_vampiric_blood : public SpellScriptLoader
{
public:
    spell_dk_vampiric_blood() : SpellScriptLoader("spell_dk_vampiric_blood") { }

    class spell_dk_vampiric_blood_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_vampiric_blood_AuraScript);

        void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
        {
            amount = GetUnitOwner()->CountPctFromMaxHealth(amount);
        }

        void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* target = GetTarget())
                if (Aura* t13 = target->GetAura(105587, target->GetGUID()))
                {
                    int32 baseHealthPctDone = 25;
                    int32 baseHealthPct = 15;
                    if (AuraEffect* glyph = target->GetAuraEffect(58676, EFFECT_0, target->GetGUID()))
                    {
                        baseHealthPctDone = 20;
                        baseHealthPct = 0;
                        target->CastCustomSpell(target, SPELL_DK_VAMPIRIC_BROOD, &baseHealthPctDone, &baseHealthPct, 0, true);
                    }
                    else
                        target->CastSpell(target, SPELL_DK_VAMPIRIC_BROOD, true);
                }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_vampiric_blood_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_INCREASE_HEALTH);
            if (m_scriptSpellId == 55233)
                AfterEffectApply += AuraEffectApplyFn(spell_dk_vampiric_blood_AuraScript::HandleEffectApply, EFFECT_1, SPELL_AURA_MOD_INCREASE_HEALTH, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_vampiric_blood_AuraScript();
    }
};

class spell_dk_activate_runestrike : public SpellScriptLoader
{
public:
    spell_dk_activate_runestrike() : SpellScriptLoader("spell_dk_activate_runestrike") { }

    class spell_dk_activate_runestrike_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_activate_runestrike_AuraScript);

        void ApplyEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
                caster->ModifyAuraState(AURA_STATE_DEFENSE, true);
        }

        void RemoveEffect(AuraEffect const* /* aurEff */, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
                caster->ModifyAuraState(AURA_STATE_DEFENSE, false);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_dk_activate_runestrike_AuraScript::RemoveEffect, EFFECT_0, SPELL_AURA_MOD_BASE_RESISTANCE_PCT, AURA_EFFECT_HANDLE_REAL);
            OnEffectApply += AuraEffectApplyFn(spell_dk_activate_runestrike_AuraScript::ApplyEffect, EFFECT_0, SPELL_AURA_MOD_BASE_RESISTANCE_PCT, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_activate_runestrike_AuraScript();
    }
};

class spell_dk_hungering_cold : public SpellScriptLoader
{
public:
    spell_dk_hungering_cold() : SpellScriptLoader("spell_dk_hungering_cold") { }

    class spell_dk_hungering_cold_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_hungering_cold_SpellScript);

        void OnHitTarget(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            Unit* target = GetHitUnit();
            if (caster && target)
                caster->CastSpell(target, 55095, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_dk_hungering_cold_SpellScript::OnHitTarget, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_hungering_cold_SpellScript;
    }
};

class spell_dk_blood_burst : public SpellScriptLoader
{
public:
    spell_dk_blood_burst() : SpellScriptLoader("spell_dk_blood_burst") { }

    class spell_dk_blood_burst_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_blood_burst_SpellScript);

        void HandleHeal(SpellEffIndex /*effIndex*/)
        {
            uint32 heal = 0;

            if (Unit* caster = GetCaster())
                if (Aura* aura = caster->GetAura(81277))
                    heal = caster->CountPctFromMaxHealth(aura->GetStackAmount() * 10);

            SetHitHeal(heal);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_dk_blood_burst_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_blood_burst_SpellScript();
    }
};

class spell_dk_howling_blast : public SpellScriptLoader
{
public:
    spell_dk_howling_blast() : SpellScriptLoader("spell_dk_howling_blast") { }

    class spell_dk_howling_blast_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_howling_blast_SpellScript);

        bool Load()
        {
            firstTarget = true;
            return true;
        }

        void HandleDamage(SpellEffIndex /*effIndex*/)
        {
            if (firstTarget)
                firstTarget = false;
            else
            {
                if (GetCaster())
                    if (Aura *aura = GetCaster()->GetAura(53386))
                        aura->ModCharges(1);
                SetHitDamage(GetHitDamage() / 2);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_dk_howling_blast_SpellScript::HandleDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
        }

        bool firstTarget;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_howling_blast_SpellScript();
    }
};

class spell_dk_frost_fever : public SpellScriptLoader
{
public:
    spell_dk_frost_fever() : SpellScriptLoader("spell_dk_frost_fever") { }

    class spell_dk_frost_fever_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_frost_fever_AuraScript);

        void RemoveEffect(AuraEffect const* /* aurEff */, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (Unit* target = GetTarget())
                {
                    if (target->HasAura(50434, caster->GetGUID()))
                        target->RemoveAurasDueToSpell(50434, caster->GetGUID());
                    else if (target->HasAura(50435, caster->GetGUID()))
                        target->RemoveAurasDueToSpell(50435, caster->GetGUID());
                }
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_dk_frost_fever_AuraScript::RemoveEffect, EFFECT_1, SPELL_AURA_MOD_MELEE_RANGED_HASTE_2, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_frost_fever_AuraScript();
    }
};

class spell_dk_killing_machine : public SpellScriptLoader
{
public:
    spell_dk_killing_machine() : SpellScriptLoader("spell_dk_killing_machine") { }

    class spell_dk_killing_machine_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_killing_machine_AuraScript);

        void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (caster->HasAura(90459)) // Item - Death Knight T11 DPS 4P Bonus
                    caster->CastSpell(caster, 90507, true);
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_dk_killing_machine_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_killing_machine_AuraScript();
    }
};

// dk t12 dps 4p bonus 98996
class spell_dk_t12_helper : public SpellScriptLoader
{
public:
    spell_dk_t12_helper() : SpellScriptLoader("spell_dk_t12_helper") { }

    class spell_dk_t12_helper_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_t12_helper_AuraScript);

        void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
        {
            if (Player* player = GetCaster()->ToPlayer())
            {
                int32 damage = CalculatePct(eventInfo.GetDamageInfo()->GetDamage(), 6);
                player->CastCustomSpell(eventInfo.GetActionTarget(), 99000, &damage, NULL, NULL, true);
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_dk_t12_helper_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_t12_helper_AuraScript();
    }
};

// 81256 Dancing rune weapon
class spell_dk_damcing_rune_weapon : public SpellScriptLoader
{
public:
    spell_dk_damcing_rune_weapon() : SpellScriptLoader("spell_dk_damcing_rune_weapon") { }

    class spell_dk_damcing_rune_weapon_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_damcing_rune_weapon_AuraScript);

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            // DK T12 Protection 4P Bonus
            if (Unit* caster = GetCaster())
                if (caster->HasAura(98966) && caster->isAlive())
                    caster->CastSpell(caster, 101162, true);
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_dk_damcing_rune_weapon_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_PARRY_PERCENT, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_damcing_rune_weapon_AuraScript();
    }
};

// 49576 - Death Grip Initial
class spell_dk_death_grip_initial : public SpellScriptLoader
{
public:
    spell_dk_death_grip_initial() : SpellScriptLoader("spell_dk_death_grip_initial") { }

    class spell_dk_death_grip_initial_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_death_grip_initial_SpellScript);

        SpellCastResult CheckCast()
        {
            Unit* caster = GetCaster();
            // Death Grip should not be castable while jumping/falling
            if (caster->HasUnitState(UNIT_STATE_JUMPING) || caster->HasUnitMovementFlag(MOVEMENTFLAG_FALLING))
                return SPELL_FAILED_MOVING;

            // Patch 4.0.6 - Minimum range has been changed to 8 yards in PvP.
            Unit* target = GetExplTargetUnit();
            if (target && target->GetTypeId() == TYPEID_PLAYER)
                if (caster->GetDistance(target) < 8.f)
                    return SPELL_FAILED_TOO_CLOSE;

            return SPELL_CAST_OK;
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_dk_death_grip_initial_SpellScript::CheckCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_death_grip_initial_SpellScript();
    }
};

// Death grip dummy
class spell_dk_death_grip_dummy : public SpellScriptLoader
{
public:
    spell_dk_death_grip_dummy() : SpellScriptLoader("spell_dk_death_grip_dummy") { }

    class spell_dk_death_grip_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_death_grip_dummy_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            Unit* spellTarget = GetSpell()->m_targets.GetUnitTarget();
            if (Unit *target = GetHitUnit())
            {
                if (spellTarget && caster == target) // Spell reflect cast
                    spellTarget->CastSpell(caster, 49560, true);
                else
                    caster->CastSpell(GetHitUnit(), 49560, true);
            }
        }

        void HandleOnHit()
        {
            SpellInfo const* triggered = sSpellMgr->GetSpellInfo(49560);
            Unit* caster = GetCaster();
            // Glyph of resilient grip
            if (AuraEffect* glyph = caster->GetAuraEffect(59309, EFFECT_0, caster->GetGUID()))
                if (GetHitUnit()->IsImmunedToSpellEffect(triggered, EFFECT_0) && GetHitUnit()->IsImmunedToSpellEffect(triggered, EFFECT_2))
                    caster->CastSpell(caster, 90289, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_dk_death_grip_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            OnHit += SpellHitFn(spell_dk_death_grip_dummy_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_death_grip_dummy_SpellScript();
    }
};

class spell_dk_chill_of_the_grave : public SpellScriptLoader
{
public:
    spell_dk_chill_of_the_grave() : SpellScriptLoader("spell_dk_chill_of_the_grave") { }

    class spell_dk_chill_of_the_grave_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_chill_of_the_grave_AuraScript);

        bool Load()
        {
            canProc = true;
            return true;
        }

        void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            if (canProc)
            {
                canProc = false;
                int32 value = aurEff->GetAmount();
                GetTarget()->CastCustomSpell(GetTarget(), GetSpellInfo()->Effects[EFFECT_0].TriggerSpell, &value, NULL, NULL, TRIGGERED_FULL_MASK, NULL, aurEff);
                events.ScheduleEvent(EVENT_ENABLE_SPELL_PROC, 400);
            }
        }

        void OnUpdate(AuraEffect* aurEff, const uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ENABLE_SPELL_PROC:
                    canProc = true;
                    break;
                }
            }
        }

    private:
        bool canProc;

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_dk_chill_of_the_grave_AuraScript::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE);
            OnEffectUpdate += AuraEffectUpdateFn(spell_dk_chill_of_the_grave_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_chill_of_the_grave_AuraScript();
    }
};

class spell_dk_t13_2p_blood : public SpellScriptLoader
{
public:
    spell_dk_t13_2p_blood() : SpellScriptLoader("spell_dk_t13_2p_blood") { }

    class spell_dk_t13_2p_blood_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_t13_2p_blood_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            if (!eventInfo.GetDamageInfo())
                return false;

            return GetTarget() && GetTarget()->HealthBelowPctDamaged(35, eventInfo.GetDamageInfo()->GetDamage());
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(spell_dk_t13_2p_blood_AuraScript::CheckProc);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_t13_2p_blood_AuraScript();
    }
};

class spell_dk_freezing_fog : public SpellScriptLoader
{
public:
    spell_dk_freezing_fog() : SpellScriptLoader("spell_dk_freezing_fog")
    {}

    class spell_dk_freezing_fog_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_freezing_fog_AuraScript);

        bool HandleProc(ProcEventInfo& eventInfo)
        {
            bool canProc = true;
            if (eventInfo.GetSpellInfo()->Id == 49184 && GetTarget())
            {
                if (Player* player = GetTarget()->ToPlayer())
                {
                    if (player->HasSpellCooldown(GetId()))
                        canProc = false;
                    else
                    {
                        canProc = true;
                        player->AddSpellCooldown(GetId(), 0, time(NULL) + 1);
                    }
                }
            }
            return canProc;
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_dk_freezing_fog_AuraScript::HandleProc);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_freezing_fog_AuraScript();
    }
};

class spell_dk_blood_presence : public SpellScriptLoader
{
public:
    spell_dk_blood_presence() : SpellScriptLoader("spell_dk_blood_presence") { }

    class spell_dk_blood_presence_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_blood_presence_AuraScript);

        void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            GetCaster()->ModifyAuraState(AURA_STATE_DEFENSE, true);
        }

        void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            GetCaster()->ModifyAuraState(AURA_STATE_DEFENSE, false);
        }

        void Register()
        {
            AfterEffectApply += AuraEffectApplyFn(spell_dk_blood_presence_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_MOD_BASE_RESISTANCE_PCT, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            AfterEffectRemove += AuraEffectRemoveFn(spell_dk_blood_presence_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_MOD_BASE_RESISTANCE_PCT, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_blood_presence_AuraScript();
    }
};

void AddSC_deathknight_spell_scripts()
{
    new spell_dk_anti_magic_shell_raid(); // Verified
    new spell_dk_anti_magic_shell_self(); // Verified
    new spell_dk_anti_magic_zone(); // Verified
    new spell_dk_blood_boil(); // Verified
    new spell_dk_blood_gorged(); // Verified
    new spell_dk_death_coil(); // Verified
    new spell_dk_death_gate(); // Verified
    new spell_dk_death_grip(); // Verified
    new spell_dk_death_pact(); // Verified
    new spell_dk_death_strike(); // Verified
    new spell_dk_rune_tap_party(); // Verified
    new spell_dk_scourge_strike(); // Verified
    new spell_dk_spell_deflection(); // Recently Verified
    new spell_dk_vampiric_blood(); // Verified
    new spell_dk_resilient_infection(); // Recently Verified
    new spell_dk_pestilence(); // Verified
    new spell_dk_deaths_advance(); // Verified
    new spell_dk_imp_presences(); // Verified
    new spell_dk_dark_transformation(); // Verified
    new spell_dk_necrotic(); // Verifie
    new spell_dk_festering(); // Verified
    new spell_dk_chilblains(); // Verified
    new spell_dk_death_grip_initial(); // Verified
    new spell_dk_death_grip_dummy(); // Verified
    new spell_dk_strangulate(); // Verified
    new spell_dk_pillar_of_frost(); // Verified
    new spell_dk_activate_runestrike(); // Recently Verified
    new spell_dk_hungering_cold(); // Recently Verified
    new spell_dk_blood_burst(); // Recently Verified
    new spell_dk_howling_blast(); // Recently Verified
    new spell_dk_frost_fever(); // Recently Verified
    new spell_dk_killing_machine(); // Recently Verified
    new spell_dk_t12_helper(); // Recently Verified
    new spell_dk_damcing_rune_weapon(); // Recently Verified
    new spell_dk_chill_of_the_grave(); // Recently Verified
    new spell_dk_t13_2p_blood(); // Recently Verified, BUT really unsure about spell id.
    new spell_dk_freezing_fog(); // Recently Verified
}