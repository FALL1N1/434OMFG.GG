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
 * Scripts for spells with SPELLFAMILY_HUNTER, SPELLFAMILY_PET and SPELLFAMILY_GENERIC spells used by hunter players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_hun_".
 */

#include "Pet.h"
#include "ScriptMgr.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

enum HunterSpells
{
    SPELL_HUNTER_ASPECT_OF_THE_BEAST_PET            = 61669,
    SPELL_HUNTER_ASPECT_OF_THE_VIPER_ENERGIZE       = 34075,
    SPELL_HUNTER_BESTIAL_WRATH                      = 19574,
    SPELL_HUNTER_GLYPH_OF_ASPECT_OF_THE_VIPER       = 56851,
    SPELL_HUNTER_INVIGORATION_TRIGGERED             = 53398,
    SPELL_HUNTER_MASTERS_CALL_TRIGGERED             = 62305,
    SPELL_HUNTER_MISDIRECTION_PROC                  = 35079,
    SPELL_HUNTER_READINESS                          = 23989,
    SPELL_HUNTER_SNIPER_TRAINING_R1                 = 53302,
    SPELL_HUNTER_SNIPER_TRAINING_BUFF_R1            = 64418,
    SPELL_HUNTER_THRILL_OF_THE_HUNT                 = 34720,
    SPELL_DRAENEI_GIFT_OF_THE_NAARU                 = 59543,
    SPELL_HUNTER_KILL_COMMAND_CRIT_10               = 60110,
    SPELL_HUNTER_KILL_COMMAND_CRIT_20               = 60113,
    SPELL_HUNTER_FOCUSED_FIRE_1                     = 35029,
    SPELL_HUNTER_FOCUSED_FIRE_2                     = 35030,
    SPELL_HUNTER_LOCK_AND_LOAD                      = 56453,
    SPELL_HUNTER_ENERGIZE                           = 91954,
    SPELL_HUNTER_IMPROVED_SERPENT_STING_R1          = 19464,
    SPELL_HUNTER_IMPROVED_SERPENT_STING_R2          = 82834,
    SPELL_HUNTER_IMPROVED_SERPENT_STING             = 83077,
    SPELL_HUNTER_SERPENT_STING                      = 1978,
    SPELL_HUNTER_AIMED_SHOT_MASTER_MARKSMAN         = 82928,
    SPELL_HUNTER_FOCUS_FIRE_ALERT                   = 88843,
    SPELL_HUNTER_PET_FRENZY                         = 19615,
    SPELL_HUNTER_FOCUS_FIRE_ENERGIZE                = 83468
};

// 13161 - Aspect of the Beast
class spell_hun_aspect_of_the_beast : public SpellScriptLoader
{
    public:
        spell_hun_aspect_of_the_beast() : SpellScriptLoader("spell_hun_aspect_of_the_beast") { }

        class spell_hun_aspect_of_the_beast_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_aspect_of_the_beast_AuraScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_HUNTER_ASPECT_OF_THE_BEAST_PET))
                    return false;
                return true;
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* caster = GetCaster()->ToPlayer())
                    if (Pet* pet = caster->GetPet())
                        pet->RemoveAurasDueToSpell(SPELL_HUNTER_ASPECT_OF_THE_BEAST_PET);
            }

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* caster = GetCaster()->ToPlayer())
                    if (caster->GetPet())
                        caster->CastSpell(caster, SPELL_HUNTER_ASPECT_OF_THE_BEAST_PET, true);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_hun_aspect_of_the_beast_AuraScript::OnApply, EFFECT_0, SPELL_AURA_UNTRACKABLE, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_hun_aspect_of_the_beast_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_UNTRACKABLE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_aspect_of_the_beast_AuraScript();
        }
};

// 34074 - Aspect of the Viper
class spell_hun_ascpect_of_the_viper : public SpellScriptLoader
{
    public:
        spell_hun_ascpect_of_the_viper() : SpellScriptLoader("spell_hun_ascpect_of_the_viper") { }

        class spell_hun_ascpect_of_the_viper_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_ascpect_of_the_viper_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_HUNTER_ASPECT_OF_THE_VIPER_ENERGIZE))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_HUNTER_GLYPH_OF_ASPECT_OF_THE_VIPER))
                    return false;
                return true;
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
            {
                PreventDefaultAction();

                uint32 maxMana = GetTarget()->GetMaxPower(POWER_MANA);
                int32 mana = CalculatePct(maxMana, GetTarget()->GetAttackTime(RANGED_ATTACK) / 1000.0f);

                if (AuraEffect const* glyph = GetTarget()->GetAuraEffect(SPELL_HUNTER_GLYPH_OF_ASPECT_OF_THE_VIPER, EFFECT_0))
                    AddPct(mana, glyph->GetAmount());

                GetTarget()->CastCustomSpell(SPELL_HUNTER_ASPECT_OF_THE_VIPER_ENERGIZE, SPELLVALUE_BASE_POINT0, mana, GetTarget(), true, NULL, aurEff);
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_hun_ascpect_of_the_viper_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_OBS_MOD_POWER);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_ascpect_of_the_viper_AuraScript();
        }
};

// 53209 - Chimera Shot
class spell_hun_chimera_shot : public SpellScriptLoader
{
    public:
        spell_hun_chimera_shot() : SpellScriptLoader("spell_hun_chimera_shot") { }

        class spell_hun_chimera_shot_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_chimera_shot_SpellScript);

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                {
                    // Chimera shot heal
                    caster->CastSpell(caster, 53353, true);
                    // Refresh serpent sting
                    if (Aura* aura = unitTarget->GetAura(1978, caster->GetGUID()))
                        aura->RefreshDuration();
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_hun_chimera_shot_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_chimera_shot_SpellScript();
        }
};

// 1978, 88453, 88466 - serpent sting
class spell_hun_serpent_sting : public SpellScriptLoader
{
    public:
        spell_hun_serpent_sting() : SpellScriptLoader("spell_hun_serpent_sting") { }

        class spell_hun_serpent_sting_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_serpent_sting_SpellScript);

            bool Load()
            {
                return true;
            }

            void HandleAfterHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (AuraEffect const* impSting = caster->GetAuraEffectOfRankedSpell(SPELL_HUNTER_IMPROVED_SERPENT_STING_R1, EFFECT_0, caster->GetGUID()))
                    {
                            SpellInfo const* sting = sSpellMgr->GetSpellInfo(SPELL_HUNTER_SERPENT_STING);

                            int32 AP = int32(caster->GetTotalAttackPowerValue(RANGED_ATTACK) * 0.08f);
                            int32 damage = sting->Effects[EFFECT_0].CalcValue(caster) + AP;
                            damage *= sting->GetMaxTicks();
                            ApplyPct(damage, impSting->GetAmount());

                            caster->CastCustomSpell(GetHitUnit(), SPELL_HUNTER_IMPROVED_SERPENT_STING, &damage, NULL, NULL, true, NULL);
                    }
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_hun_serpent_sting_SpellScript::HandleAfterHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_serpent_sting_SpellScript();
        }
};


// 781 - Disengage
class spell_hun_disengage : public SpellScriptLoader
{
    public:
        spell_hun_disengage() : SpellScriptLoader("spell_hun_disengage") { }

        class spell_hun_disengage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_disengage_SpellScript);

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (caster->GetTypeId() == TYPEID_PLAYER && !caster->isInCombat())
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                return SPELL_CAST_OK;
            }

            void HandlePostHaste(SpellEffIndex /*effIndex*/)
            {
                Unit* target = GetHitUnit();
                if (AuraEffect* postHaste = target->GetDummyAuraEffect(SPELLFAMILY_HUNTER, 5094, EFFECT_1))
                {
                    int32 bp0 = postHaste->GetAmount();
                    target->CastCustomSpell(target, 83559, &bp0, NULL, NULL, true);
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_hun_disengage_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_hun_disengage_SpellScript::HandlePostHaste, EFFECT_0, SPELL_EFFECT_LEAP_BACK);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_disengage_SpellScript();
        }
};

// 53412 - Invigoration
class spell_hun_invigoration : public SpellScriptLoader
{
    public:
        spell_hun_invigoration() : SpellScriptLoader("spell_hun_invigoration") { }

        class spell_hun_invigoration_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_invigoration_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_HUNTER_INVIGORATION_TRIGGERED))
                    return false;
                return true;
            }

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                if (Unit* unitTarget = GetHitUnit())
                    if (AuraEffect* aurEff = unitTarget->GetDummyAuraEffect(SPELLFAMILY_HUNTER, 3487, 0))
                        if (roll_chance_i(aurEff->GetAmount()))
                            unitTarget->CastSpell(unitTarget, SPELL_HUNTER_INVIGORATION_TRIGGERED, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_hun_invigoration_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_invigoration_SpellScript();
        }
};

// 53271 - Masters Call
class spell_hun_masters_call : public SpellScriptLoader
{
    public:
        spell_hun_masters_call() : SpellScriptLoader("spell_hun_masters_call") { }

        class spell_hun_masters_call_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_masters_call_SpellScript);

            SpellCastResult CheckCast()
            {
                Player* caster = GetCaster()->ToPlayer();
                if (caster && caster->GetPet() && caster->GetPet()->HasUnitState(UNIT_STATE_CONTROLLED))
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                return SPELL_CAST_OK;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* ally = GetHitUnit())
                    if (Player* caster = GetCaster()->ToPlayer())
                        if (Pet* target = caster->GetPet())
                        {
                            TriggerCastFlags castMask = TriggerCastFlags(TRIGGERED_FULL_MASK & ~TRIGGERED_IGNORE_CASTER_AURASTATE);
                            target->CastSpell(ally, GetEffectValue(), castMask);
                        }
            }

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetHitUnit())
                {
                    // Cannot be processed while pet is dead
                    TriggerCastFlags castMask = TriggerCastFlags(TRIGGERED_FULL_MASK & ~TRIGGERED_IGNORE_CASTER_AURASTATE);
                    target->CastSpell(target, SPELL_HUNTER_MASTERS_CALL_TRIGGERED, castMask);
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_hun_masters_call_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_hun_masters_call_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
                OnEffectLaunchTarget += SpellEffectFn(spell_hun_masters_call_SpellScript::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_masters_call_SpellScript();
        }
};

// 34477 - Misdirection
class spell_hun_misdirection : public SpellScriptLoader
{
    public:
        spell_hun_misdirection() : SpellScriptLoader("spell_hun_misdirection") { }

        class spell_hun_misdirection_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_misdirection_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_HUNTER_MISDIRECTION_PROC))
                    return false;
                return true;
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Player* player = caster->ToPlayer())
                    {
                        if (player->GetPet() && GetTarget()->GetRedirectThreatTarget())
                        {
                            if (player->GetPet()->GetGUID() == GetTarget()->GetRedirectThreatTarget()->GetGUID())
                            {
                                if (player->GetDummyAuraEffect(SPELLFAMILY_HUNTER, 2231, EFFECT_0))
                                    player->RemoveSpellCooldown(34477, true);
                            }
                        }
                    }
                }

                if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEFAULT)
                    GetTarget()->ResetRedirectThreat();
            }

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (Player* player = caster->ToPlayer())
                        if (player->GetPet() && GetTarget()->GetRedirectThreatTarget())
                            if (player->GetPet()->GetGUID() == GetTarget()->GetRedirectThreatTarget()->GetGUID())
                                if (player->GetDummyAuraEffect(SPELLFAMILY_HUNTER, 2231, EFFECT_0))
                                    player->RemoveSpellCooldown(34477, true);
            }

            bool CheckProc(ProcEventInfo& /*eventInfo*/)
            {
                return GetTarget()->GetRedirectThreatTarget();
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
            {
                PreventDefaultAction();
                GetTarget()->CastSpell(GetTarget(), SPELL_HUNTER_MISDIRECTION_PROC, true, NULL, aurEff);
            }

            void Register() override
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_hun_misdirection_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                AfterEffectApply += AuraEffectApplyFn(spell_hun_misdirection_AuraScript::OnApply, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                DoCheckProc += AuraCheckProcFn(spell_hun_misdirection_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_hun_misdirection_AuraScript::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
            }
        };

        class spell_hun_misdirection_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_misdirection_SpellScript);

            void HandleBeforeCast()
            {
                // we need a cleanup for the glyph
                GetCaster()->RemoveAurasDueToSpell(34477, 0, 0, AURA_REMOVE_BY_EXPIRE);
            }

            void Register() override
            {
                BeforeCast += SpellCastFn(spell_hun_misdirection_SpellScript::HandleBeforeCast);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_hun_misdirection_AuraScript();
        }

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_misdirection_SpellScript();
        }

};

// 35079 - Misdirection (Proc)
class spell_hun_misdirection_proc : public SpellScriptLoader
{
    public:
        spell_hun_misdirection_proc() : SpellScriptLoader("spell_hun_misdirection_proc") { }

        class spell_hun_misdirection_proc_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_misdirection_proc_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->ResetRedirectThreat();
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_hun_misdirection_proc_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_misdirection_proc_AuraScript();
        }
};

// 23989 - Readiness
class spell_hun_readiness : public SpellScriptLoader
{
    public:
        spell_hun_readiness() : SpellScriptLoader("spell_hun_readiness") { }

        class spell_hun_readiness_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_readiness_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Player* caster = GetCaster()->ToPlayer();
                // immediately finishes the cooldown on your other Hunter abilities except Bestial Wrath
                const SpellCooldowns& cm = caster->ToPlayer()->GetSpellCooldownMap();
                for (SpellCooldowns::const_iterator itr = cm.begin(); itr != cm.end();)
                {
                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);

                    ///! If spellId in cooldown map isn't valid, the above will return a null pointer.
                    if (spellInfo &&
                        spellInfo->SpellFamilyName == SPELLFAMILY_HUNTER &&
                        spellInfo->Id != SPELL_HUNTER_READINESS &&
                        spellInfo->Id != SPELL_HUNTER_BESTIAL_WRATH &&
                        spellInfo->Id != SPELL_DRAENEI_GIFT_OF_THE_NAARU &&
                        spellInfo->GetRecoveryTime() > 0)
                        caster->RemoveSpellCooldown((itr++)->first, true);
                    else
                        ++itr;
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_hun_readiness_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_readiness_SpellScript();
        }
};

// 37506 - Scatter Shot
class spell_hun_scatter_shot : public SpellScriptLoader
{
    public:
        spell_hun_scatter_shot() : SpellScriptLoader("spell_hun_scatter_shot") { }

        class spell_hun_scatter_shot_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_scatter_shot_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Player* caster = GetCaster()->ToPlayer();
                // break Auto Shot and autohit
                caster->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
                caster->AttackStop();
                caster->SendAttackSwingCancelAttack();

                if (Unit* target = GetHitUnit())
                    target->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_hun_scatter_shot_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_scatter_shot_SpellScript();
        }
};

#define EVENT_CHECK_MOVEMENT    1

// -53302 - Sniper Training
class spell_hun_sniper_training : public SpellScriptLoader
{
    public:
        spell_hun_sniper_training() : SpellScriptLoader("spell_hun_sniper_training") { }

        class spell_hun_sniper_training_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_sniper_training_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_HUNTER_SNIPER_TRAINING_R1) || !sSpellMgr->GetSpellInfo(SPELL_HUNTER_SNIPER_TRAINING_BUFF_R1))
                    return false;
                return true;
            }

            bool Load()
            {
                events.ScheduleEvent(EVENT_CHECK_MOVEMENT, 6000);
                return true;
            }

            void HandlePeriodic(AuraEffect const* aurEff)
            {
                PreventDefaultAction();
                if (aurEff->GetAmount() <= 0)
                {
                    Unit* target = GetTarget();
                    uint32 spellId = SPELL_HUNTER_SNIPER_TRAINING_BUFF_R1 + GetId() - SPELL_HUNTER_SNIPER_TRAINING_R1;

                    target->CastSpell(target, spellId, true, nullptr, aurEff);
                }
            }

            void HandleUpdatePeriodic(AuraEffect* aurEff)
            {
                Unit* target = GetUnitOwner();

                int32 baseAmount = aurEff->GetBaseAmount();
                int32 amount = target->isMoving() ?
                    target->CalculateSpellDamage(target, GetSpellInfo(), aurEff->GetEffIndex(), &baseAmount) :
                    aurEff->GetAmount() - 1;
                aurEff->SetAmount(amount);
            }

            void OnUpdate(AuraEffect* aurEff, const uint32 diff)
            {
                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CHECK_MOVEMENT:
                            if (GetUnitOwner()->isMoving())
                                aurEff->SetAmount(aurEff->GetBaseAmount());
                            events.ScheduleEvent(EVENT_CHECK_MOVEMENT, 100);
                            break;
                    }
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_hun_sniper_training_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
                OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_hun_sniper_training_AuraScript::HandleUpdatePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
                OnEffectUpdate += AuraEffectUpdateFn(spell_hun_sniper_training_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_sniper_training_AuraScript();
        }
};

// 1515 - Tame Beast
class spell_hun_tame_beast : public SpellScriptLoader
{
    public:
        spell_hun_tame_beast() : SpellScriptLoader("spell_hun_tame_beast") { }

        class spell_hun_tame_beast_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_tame_beast_SpellScript);

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (caster->GetTypeId() != TYPEID_PLAYER)
                    return SPELL_FAILED_DONT_REPORT;

                if (!GetExplTargetUnit())
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                if (Creature* target = GetExplTargetUnit()->ToCreature())
                {
                    if (target->getLevel() > caster->getLevel())
                        return SPELL_FAILED_HIGHLEVEL;

                    // use SMSG_PET_TAME_FAILURE?
                    if (!target->GetCreatureTemplate()->isTameable(caster->ToPlayer()->CanTameExoticPets()))
                        return SPELL_FAILED_BAD_TARGETS;

                    if (caster->GetPetGUID())
                        return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                    if (caster->GetCharmGUID())
                        return SPELL_FAILED_ALREADY_HAVE_CHARM;
                }
                else
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_hun_tame_beast_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_tame_beast_SpellScript();
        }
};

// -24604 - Furious Howl
//  53434 - Call of the Wild
class spell_hun_target_only_pet_and_owner : public SpellScriptLoader
{
    public:
        spell_hun_target_only_pet_and_owner() : SpellScriptLoader("spell_hun_target_only_pet_and_owner") { }

        class spell_hun_target_only_pet_and_owner_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_target_only_pet_and_owner_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targets.clear();
                targets.push_back(GetCaster());
                if (Unit* owner = GetCaster()->GetOwner())
                    targets.push_back(owner);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hun_target_only_pet_and_owner_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_PARTY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hun_target_only_pet_and_owner_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_CASTER_AREA_PARTY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_target_only_pet_and_owner_SpellScript();
        }
};

// Intervene
class spell_hun_intervene : public SpellScriptLoader
{
    public:
        spell_hun_intervene() : SpellScriptLoader("spell_hun_intervene") { }

        class spell_hun_intervene_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_intervene_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                    amount = caster->CountPctFromMaxHealth(amount);
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_intervene_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_hun_intervene_AuraScript();
        }
};

// -82692 - Foucus Fire
class spell_hun_focus_fire : public SpellScriptLoader
{
    public:
        spell_hun_focus_fire() : SpellScriptLoader("spell_hun_focus_fire") { }

        class spell_hun_focus_fire_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_focus_fire_AuraScript);

            bool Load()
            {
                Unit* caster = GetCaster();
                return caster && caster->GetTypeId() == TYPEID_PLAYER;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                if (GetCaster())
                    if (Pet* pet = GetCaster()->ToPlayer()->GetPet())
                        if (Aura* aur = pet->GetAura(SPELL_HUNTER_PET_FRENZY, pet->GetGUID()))
                            amount *= aur->GetStackAmount();
            }

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                int32 basepoints0 = aurEff->GetAmount();
                if (GetCaster())
                {
                    GetCaster()->RemoveAurasDueToSpell(SPELL_HUNTER_FOCUS_FIRE_ALERT);
                    if (Pet* pet = GetCaster()->ToPlayer()->GetPet())
                    {
                        if (Aura* aur = pet->GetAura(SPELL_HUNTER_PET_FRENZY, pet->GetGUID()))
                        {
                            basepoints0 *= aur->GetStackAmount();
                            GetCaster()->CastCustomSpell(pet, SPELL_HUNTER_FOCUS_FIRE_ENERGIZE, &basepoints0, NULL, NULL, true);
                            aur->Remove();
                        }
                    }
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_focus_fire_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_RANGED_HASTE);
                AfterEffectApply += AuraEffectApplyFn(spell_hun_focus_fire_AuraScript::OnApply, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_hun_focus_fire_AuraScript();
        }
};

// 56641 - Steady Shot
// 77767 - Cobra Shot
class spell_hun_energize : public SpellScriptLoader
{
    public:
        spell_hun_energize() : SpellScriptLoader("spell_hun_energize") { }

        class spell_hun_energize_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_energize_SpellScript);

            void HandleOnHit()
            {
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_HUNTER_ENERGIZE);
                if (!spellInfo)
                    return;

                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                {
                    // Refresh serpent sting
                    if (GetSpellInfo()->Id == 77767)
                    {
                        if (Aura* aura = unitTarget->GetAura(1978, GetCaster()->GetGUID()))
                        {
                            aura->SetDuration(std::min(aura->GetDuration() + GetSpellInfo()->Effects[EFFECT_1].BasePoints * 1000, aura->GetMaxDuration()));
                            aura->RecalculateAmountOfEffects();
                        }

                        if (Aura* aura = unitTarget->GetAura(88453, GetCaster()->GetGUID()))
                        {
                            aura->SetDuration(std::min(aura->GetDuration() + GetSpellInfo()->Effects[EFFECT_1].BasePoints * 1000, aura->GetMaxDuration()));
                            aura->RecalculateAmountOfEffects();
                        }
                        else if (Aura* aura = unitTarget->GetAura(88466, GetCaster()->GetGUID()))
                        {
                            aura->SetDuration(std::min(aura->GetDuration() + GetSpellInfo()->Effects[EFFECT_1].BasePoints * 1000, aura->GetMaxDuration()));
                            aura->RecalculateAmountOfEffects();
                        }
                    }
                    int32 bp0 = 0;
                    // Termination
                    if (AuraEffect* termination = caster->GetDummyAuraEffect(SPELLFAMILY_HUNTER, 2008, EFFECT_0))
                        if (unitTarget->GetHealthPct() <= 25.0f)
                            bp0 += termination->GetAmount();

                    // Glyph of Dazzled Prey
                    if (AuraEffect* glyph = caster->GetDummyAuraEffect(SPELLFAMILY_HUNTER, 2109, EFFECT_0))
                        if (unitTarget->HasAuraWithMechanic(1 << MECHANIC_DAZE))
                            bp0 += glyph->GetAmount();

                    // reward bonus energy
                    if (bp0 >= 1)
                        caster->CastCustomSpell(caster, SPELL_HUNTER_ENERGIZE, &bp0, NULL, NULL, true);
                }

            }

            void HandleAfterCast()
            {
                if (Unit* caster = GetCaster())
                {
                    // Hunter T13 2P Bonus - Steady Shot and Cobra Shot generate double the amount of focus.
                    int32 bp0 = caster->HasAura(105732, caster->GetGUID()) ? 18 : 9;
                    caster->CastCustomSpell(caster, SPELL_HUNTER_ENERGIZE, &bp0, NULL, NULL, true);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_hun_energize_SpellScript::HandleOnHit);
                AfterCast += SpellCastFn(spell_hun_energize_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_energize_SpellScript();
        }
};

// -83676 - Resistance is futile periodic trigger
class spell_hun_resistance_is_futile : public SpellScriptLoader
{
    public:
        spell_hun_resistance_is_futile() : SpellScriptLoader("spell_hun_resistance_is_futile") { }

        class spell_hun_resistance_is_futile_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_resistance_is_futile_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetUnitOwner()->GetPosition(&oldPos);
            }

            void HandleUpdatePeriodic(AuraEffect* /*aurEff*/)
            {
                if (Unit* caster = GetCaster())
                {
                    Position newPos;
                    GetUnitOwner()->GetPosition(&newPos);
                    // Proc chance every 2 yards moved
                    if (newPos.GetExactDist(&oldPos) > 2.0f)
                    {
                        oldPos.GetPosition(&newPos);
                        if (AuraEffect* aur = caster->GetDummyAuraEffect(SPELLFAMILY_HUNTER, 5119, EFFECT_0))
                            if (roll_chance_i(aur->GetAmount()))
                                caster->CastSpell(caster, 82897, true);
                    }
                }
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_hun_resistance_is_futile_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_hun_resistance_is_futile_AuraScript::HandleUpdatePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }

            Position oldPos;
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_hun_resistance_is_futile_AuraScript();
        }
};

// 2643 - Multi-shot
class spell_hun_serpent_spread : public SpellScriptLoader
{
    public:
        spell_hun_serpent_spread() : SpellScriptLoader("spell_hun_serpent_spread") { }

        class spell_hun_serpent_spread_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_serpent_spread_SpellScript);

            bool Load()
            {
                return true;
            }

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                {
                    uint32 triggeredSpell = 0;
                    if (caster->HasSpell(87934))
                        triggeredSpell = 88453;
                    else if (caster->HasSpell(87935))
                        triggeredSpell = 88466;

                    if (triggeredSpell)
                        caster->CastSpell(unitTarget, triggeredSpell, true);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_hun_serpent_spread_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_serpent_spread_SpellScript();
        }
};

// 51753 - Camouflage
class spell_hun_camouflage : public SpellScriptLoader
{
    public:
        spell_hun_camouflage() : SpellScriptLoader("spell_hun_camouflage") { }

        class spell_hun_camouflage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_camouflage_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                caster->CastSpell(caster, 51755, true);
                if (caster->ToPlayer())
                    if (Pet* pet = caster->ToPlayer()->GetPet())
                        pet->CastSpell(pet, 51755, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_hun_camouflage_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_camouflage_SpellScript();
        }
};

// 80326 - Camouflage periodic
class spell_hun_camouflage_periodic : public SpellScriptLoader
{
    public:
        spell_hun_camouflage_periodic() : SpellScriptLoader("spell_hun_camouflage_periodic") { }

        class spell_hun_camouflage_periodic_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_camouflage_periodic_AuraScript);

            void HandlePeriodic(AuraEffect const* /*aurEff*/)
            {
                if (GetTarget()->isMoving())
                    PreventDefaultAction();
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_hun_camouflage_periodic_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_hun_camouflage_periodic_AuraScript();
        }
};

class spell_hun_kill_shot : public SpellScriptLoader
{
    public:
        spell_hun_kill_shot() : SpellScriptLoader("spell_hun_kill_shot") { }

        class spell_hun_kill_shot_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_kill_shot_SpellScript);

            void HandleDamage()
            {
                Unit* caster = GetCaster();
                if (caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                Unit* target = GetHitUnit();
                int32 damage = GetHitDamage();
                // Glyph cooldown reset when target was failed to kill
                if (AuraEffect* glyph = caster->GetDummyAuraEffect(SPELLFAMILY_HUNTER, 3676, EFFECT_0))
                    if (target && target->GetHealthPct() <= glyph->GetAmount() && !caster->HasAura(90967))
                        if (int32(target->GetHealth()) > damage)
                        {
                            caster->ToPlayer()->RemoveSpellCooldown(53351, true);
                            caster->CastSpell(caster, 90967, true);
                        }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_hun_kill_shot_SpellScript::HandleDamage);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_kill_shot_SpellScript();
        }
};

// Concussive shot
class spell_hun_concussive: public SpellScriptLoader
{
    public:
        spell_hun_concussive() : SpellScriptLoader("spell_hun_concussive") { }

        class spell_hun_concussive_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_concussive_SpellScript);

            void HandleOnHit()
            {
                AuraEffect* glyph = GetCaster()->GetAuraEffect(56851, EFFECT_0);
                if (glyph)
                    return;
                if (Aura* debuff = GetHitAura())
                    if (AuraEffect* normalization = debuff->GetEffect(EFFECT_1))
                    {
                        normalization->SetAmount(0);
                        normalization->HandleEffect(GetHitUnit(), AURA_REMOVE_BY_DEFAULT, false);
                    }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_hun_concussive_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_concussive_SpellScript();
        }
};

// 34497 - Thrill of the Hunt
class spell_hun_thrill_of_the_hunt : public SpellScriptLoader
{
public:
    spell_hun_thrill_of_the_hunt() : SpellScriptLoader("spell_hun_thrill_of_the_hunt") { }

    class spell_hun_thrill_of_the_hunt_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_thrill_of_the_hunt_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_HUNTER_THRILL_OF_THE_HUNT))
                return false;
            return true;
        }

        void HandleEffectProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            int32 focus = eventInfo.GetDamageInfo()->GetSpellInfo()->CalcPowerCost(GetTarget(), SpellSchoolMask(eventInfo.GetDamageInfo()->GetSchoolMask()));
            focus = CalculatePct(focus, aurEff->GetAmount());

            GetTarget()->CastCustomSpell(GetTarget(), SPELL_HUNTER_THRILL_OF_THE_HUNT, &focus, NULL, NULL, true, NULL, aurEff);
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(spell_hun_thrill_of_the_hunt_AuraScript::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hun_thrill_of_the_hunt_AuraScript();
    }
};

class spell_hun_explosive_trap : public SpellScriptLoader
{
public:
    spell_hun_explosive_trap() : SpellScriptLoader("spell_hun_explosive_trap") { }

    class spell_hun_explosive_trap_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_explosive_trap_AuraScript);

        void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
        {
            canBeRecalculated = false;
            if (Unit* caster = GetCaster())
                amount += floor(0.0546f * caster->GetTotalAttackPowerValue(RANGED_ATTACK));
        }

        void OnPeriodic(AuraEffect const* /*aurEff*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (AuraEffect* tnt = caster->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_HUNTER, 355, EFFECT_0))
                    if (roll_chance_i(tnt->GetAmount()))
                        caster->CastSpell(caster, SPELL_HUNTER_LOCK_AND_LOAD, true, NULL, tnt);
            }
        }

        void Register() override
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_explosive_trap_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_hun_explosive_trap_AuraScript::OnPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_hun_explosive_trap_AuraScript();
    }
};

class spell_hun_lock_n_load : public SpellScriptLoader
{
public:
    spell_hun_lock_n_load() : SpellScriptLoader("spell_hun_lock_n_load") { }

    class spell_hun_lock_n_load_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_lock_n_load_SpellScript);

        void ResetCooldown(SpellEffIndex /*effIndex*/)
        {
            if (Player* caster = GetCaster()->ToPlayer())
                if (caster->HasSpellCooldown(53301)) // Explosive Shot
                    caster->RemoveSpellCooldown(53301, true);
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_hun_lock_n_load_SpellScript::ResetCooldown, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_hun_lock_n_load_SpellScript();
    }
};

class spell_hun_trap_launcher : public SpellScriptLoader
{
public:
    spell_hun_trap_launcher() : SpellScriptLoader("spell_hun_trap_launcher") { }

    class spell_hun_trap_launcher_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_trap_launcher_SpellScript);

        void HandleAfterCast()
        {
            if (Unit* caster = GetCaster())
                caster->RemoveAurasDueToSpell(77769);
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_hun_trap_launcher_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hun_trap_launcher_SpellScript();
    }
};

class spell_hun_kill_command : public SpellScriptLoader
{
public:
    spell_hun_kill_command() : SpellScriptLoader("spell_hun_kill_command") { }

    class spell_hun_kill_command_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_kill_command_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (caster->HasAura(82897)) // Resistance is Futile!
                {
                    caster->CastSpell(caster, 86316, true);
                    caster->RemoveAurasDueToSpell(82897);
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_hun_kill_command_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hun_kill_command_SpellScript();
    }
};

class spell_hun_lock_and_load_proc : public SpellScriptLoader
{
public:
    spell_hun_lock_and_load_proc() : SpellScriptLoader("spell_hun_lock_and_load_proc") { }

    class spell_hun_lock_and_load_proc_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_lock_and_load_proc_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            if (eventInfo.GetActionTarget() && eventInfo.GetActionTarget()->GetTypeId() == TYPEID_UNIT)
                if (Creature* target = eventInfo.GetActionTarget()->ToCreature())
                    if (target->IsDungeonBoss() || target->isWorldBoss()) // Lock and Load can't trigger on boss npcs
                        return false;
            return true;
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(spell_hun_lock_and_load_proc_AuraScript::CheckProc);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hun_lock_and_load_proc_AuraScript();
    }
};

// explosive trap part of tnt is handled at spell_hun_explosive_trap
class spell_hun_tnt : public SpellScriptLoader
{
public:
    spell_hun_tnt() : SpellScriptLoader("spell_hun_tnt") { }

    class spell_hun_tnt_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_tnt_AuraScript);

        void OnPeriodic(AuraEffect const* /*aurEff*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (AuraEffect* tnt = caster->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_HUNTER, 355, EFFECT_0))
                    if (roll_chance_i(tnt->GetAmount()))
                        caster->CastSpell(caster, SPELL_HUNTER_LOCK_AND_LOAD, true, NULL, tnt);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_hun_tnt_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hun_tnt_AuraScript();
    }
};

class spell_hun_fire_proc : public SpellScriptLoader
{
public:
    spell_hun_fire_proc() : SpellScriptLoader("spell_hun_fire_proc")
    {}

    class spell_hun_fire_proc_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_fire_proc_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            // Because 19464 and 82928 has the same spellfamilymask
            if (eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_HUNTER_AIMED_SHOT_MASTER_MARKSMAN)
                return true;

            return false;
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(spell_hun_fire_proc_AuraScript::CheckProc);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hun_fire_proc_AuraScript();
    }
};

void AddSC_hunter_spell_scripts()
{
    new spell_hun_aspect_of_the_beast(); // Recently Verified
    new spell_hun_ascpect_of_the_viper();// Recently Verified
    new spell_hun_chimera_shot(); // Verified
	new spell_hun_disengage(); // Verified
	new spell_hun_invigoration(); // Verified
	new spell_hun_masters_call(); // Verified
	new spell_hun_misdirection(); // Verified
	new spell_hun_misdirection_proc(); // Verified
	new spell_hun_readiness(); // Verified
	new spell_hun_scatter_shot(); // Verified
	new spell_hun_sniper_training(); // Verified
	new spell_hun_tame_beast(); // Verified
	new spell_hun_target_only_pet_and_owner(); // Verified
	new spell_hun_focus_fire(); // Verified
	new spell_hun_energize(); // Verified
	new spell_hun_resistance_is_futile(); // Verified
	new spell_hun_serpent_sting(); // Verified
	new spell_hun_serpent_spread(); // Verified
	new spell_hun_camouflage_periodic(); // Verified
	new spell_hun_camouflage(); // Verified
	new spell_hun_kill_shot(); // Verified
	new spell_hun_concussive(); // Verified
    new spell_hun_intervene(); // Verified
    new spell_hun_thrill_of_the_hunt(); // Recently Verified
    new spell_hun_explosive_trap(); // Recently Verified
    new spell_hun_lock_n_load(); // Recently Verified
    new spell_hun_trap_launcher(); // Recently Verified
    new spell_hun_kill_command(); // Verified
    new spell_hun_lock_and_load_proc(); // Recently Verified
    new spell_hun_tnt(); // Verified
    new spell_hun_fire_proc(); // Recently Verified, but a bit unsure if correct spell id(s).
}
