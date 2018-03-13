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
 * Scripts for spells with SPELLFAMILY_WARLOCK and SPELLFAMILY_GENERIC spells used by warlock players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_warl_".
 */

#include "GridNotifiers.h"
#include "Player.h"
#include "Pet.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"

enum WarlockSpells
{
    SPELL_WARLOCK_BANE_OF_DOOM_EFFECT               = 18662,
    SPELL_WARLOCK_CURSE_OF_DOOM_EFFECT              = 18662,
    SPELL_WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST         = 62388,
    SPELL_WARLOCK_DEMONIC_CIRCLE_SUMMON             = 48018,
    SPELL_WARLOCK_DEMONIC_CIRCLE_TELEPORT           = 48020,
    SPELL_WARLOCK_DEMONIC_EMPOWERMENT_FELGUARD      = 54508,
    SPELL_WARLOCK_DEMONIC_EMPOWERMENT_FELHUNTER     = 54509,
    SPELL_WARLOCK_DEMONIC_EMPOWERMENT_IMP           = 54444,
    SPELL_WARLOCK_DEMONIC_EMPOWERMENT_SUCCUBUS      = 54435,
    SPELL_WARLOCK_DEMONIC_EMPOWERMENT_VOIDWALKER    = 54443,
    SPELL_WARLOCK_DEMON_SOUL_IMP                    = 79459,
    SPELL_WARLOCK_DEMON_SOUL_FELHUNTER              = 79460,
    SPELL_WARLOCK_DEMON_SOUL_FELGUARD               = 79462,
    SPELL_WARLOCK_DEMON_SOUL_SUCCUBUS               = 79463,
    SPELL_WARLOCK_DEMON_SOUL_VOIDWALKER             = 79464,
    SPELL_WARLOCK_FEL_SYNERGY_HEAL                  = 54181,
    SPELL_WARLOCK_HAUNT                             = 48181,
    SPELL_WARLOCK_HAUNT_HEAL                        = 48210,
    SPELL_WARLOCK_IMMOLATE                          = 348,
    SPELL_WARLOCK_CREATE_HEALTHSTONE                = 34130,
    SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_BUFF_R1    = 60955,
    SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_BUFF_R2    = 60956,
    SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_R1         = 18703,
    SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_R2         = 18704,
    SPELL_WARLOCK_IMPROVED_SOULFIRE                 = 85383,
    SPELL_WARLOCK_LIFE_TAP_ENERGIZE                 = 31818,
    SPELL_WARLOCK_LIFE_TAP_ENERGIZE_2               = 32553,
    SPELL_WARLOCK_SOULSHATTER                       = 32835,
    SPELL_WARLOCK_UNSTABLE_AFFLICTION               = 30108,
    SPELL_WARLOCK_UNSTABLE_AFFLICTION_DISPEL        = 31117,
    SPELL_WARLOCK_JINX_ENERGY                       = 85540,
    SPELL_WARLOCK_JINX_RAGE                         = 85539,
    SPELL_WARLOCK_JINX_RUNIC_POWER                  = 85541,
    SPELL_WARLOCK_JINX_FOCUS                        = 85542,
    SPELL_WARLOCK_JINX_R1                           = 18179,
    SPELL_WARLOCK_JINX_R2                           = 85479,
    SPELL_WARLOCK_JINX_CoE_R1                       = 85547,
    SPELL_WARLOCK_JINX_CoE_R2                       = 86105,
    SPELL_WARLOCK_CoE                               = 1490,
    SPELL_WARLOCK_DRAIN_LIFE_HEALTH                 = 89653,
    SPELL_WARLOCK_SOUL_SWAP_OVERRIDE                = 86211,
    SPELL_WARLOCK_SOUL_SWAP_EXHALE                  = 86213,
    SPELL_WARLOCK_SOUL_SWAP_GLYPH                   = 56226,
    SPELL_WARLOCK_SOUL_SWAP_COOLDOWN                = 94229,
    SPELL_WARLOCK_SOUL_SWAP_VISUAL                  = 92795,
    SPELL_WARLOCK_SOUL_SWAP_FREE_DOTS_COSTS         = 92794,
    SPELL_WARLOCK_SOULBURN                          = 74434,
    SPELL_WARLOCK_SOULBURN_SEED_OF_CORRUPTION       = 86664,
    SPELL_WARLOCK_SOULBURN_SOC_VISUAL               = 93313,
    SPELL_WARLOCK_SOULSHARD_ADD                     = 87388,
    SPELL_WARLOCK_PANDEMIC                          = 92931,
    SPELL_WARLOCK_SOULBURN_SEARING_PAIN             = 79440,
    SPELL_WARLOCK_SOULBURN_DEMONIC_CIRCLE           = 79438,
    SPELL_WARLOCK_SOUL_HARVEST_SOULSHARD            = 101977,
    SPELL_WARLOCK_FEL_ARMOR_HEAL                    = 96379,
    SPELL_WARLOCK_SHADOW_WARD                       = 6229,
    SPELL_WARLOCK_NETHER_WARD                       = 91711,
    SPELL_WARLOCK_METHAMORPHOSIS_FORM               = 47241,
    SPELL_WARLOCK_NETHER_WARD_TALENT                = 91713,
    SPELL_WARLOCK_SOULSTONE                         = 20707,
    SPELL_WARLOCK_SOULSTONE_RESURRECT               = 95750,
    SPELL_WARLOCK_BANE_OF_HAVOC                     = 80240,
    SPELL_WARLOCK_BANE_OF_HAVOC_DAMAGE              = 85455,
    SPELL_WARLOCK_BANE_OF_HAVOC_TRACK               = 85466
};

enum WarlockSpellIcons
{
    WARLOCK_ICON_ID_IMPROVED_LIFE_TAP               = 208,
    WARLOCK_ICON_ID_MANA_FEED                       = 1982,
    WARLOCK_ICON_SOULFIRE                           = 184,
    WARLOCK_ICON_IMPENDING_DOOM                     = 195
};

// 710 - Banish
/// Updated 4.3.4
class spell_warl_banish : public SpellScriptLoader
{
    public:
        spell_warl_banish() : SpellScriptLoader("spell_warl_banish") { }

        class spell_warl_banish_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_banish_SpellScript);

            bool Load()
            {
                _removed = false;
                return true;
            }

            void HandleBanish()
            {
                /// Casting Banish on a banished target will cancel the effect
                /// Check if the target already has Banish, if so, do nothing.
                if (Unit* target = GetHitUnit())
                {
                    if (target->GetAuraEffect(SPELL_AURA_SCHOOL_IMMUNITY, SPELLFAMILY_WARLOCK, 0, 0x08000000, 0))
                    {
                        // No need to remove old aura since its removed due to not stack by current Banish aura
                        PreventHitDefaultEffect(EFFECT_0);
                        PreventHitDefaultEffect(EFFECT_1);
                        PreventHitDefaultEffect(EFFECT_2);
                        _removed = true;
                    }
                }
            }

            void RemoveAura()
            {
                if (_removed)
                    PreventHitAura();
            }

            void Register()
            {
                BeforeHit += SpellHitFn(spell_warl_banish_SpellScript::HandleBanish);
                AfterHit += SpellHitFn(spell_warl_banish_SpellScript::RemoveAura);
            }

            bool _removed;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_banish_SpellScript();
        }
};

class spell_warl_curse_of_weakness : public SpellScriptLoader
{
    public:
        spell_warl_curse_of_weakness() : SpellScriptLoader("spell_warl_curse_of_weakness") { }

        class spell_warl_curse_of_weakness_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_curse_of_weakness_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_ENERGY))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_RAGE))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_RUNIC_POWER))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_FOCUS))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_R1))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_R2))
                    return false;

                return true;
            }

            void HandleJinx()
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || ! target)
                    return;

                uint32 trigerred_spell = 0;
                switch (target->getPowerType())
                {
                    case POWER_RAGE:
                        trigerred_spell = SPELL_WARLOCK_JINX_RAGE;
                        break;
                    case POWER_FOCUS:
                        trigerred_spell = SPELL_WARLOCK_JINX_FOCUS;
                        break;
                    case POWER_ENERGY:
                        trigerred_spell = SPELL_WARLOCK_JINX_ENERGY;
                        break;
                    case POWER_RUNIC_POWER:
                        trigerred_spell = SPELL_WARLOCK_JINX_RUNIC_POWER;
                        break;
                    default:
                        return;
                }

                if (AuraEffect const* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_WARLOCK, 5002, EFFECT_1))
                {
                    int32 basepoints0 = aurEff->GetAmount();
                    caster->CastCustomSpell(target, trigerred_spell, &basepoints0, NULL, NULL, true);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_curse_of_weakness_SpellScript::HandleJinx);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_curse_of_weakness_SpellScript();
        }
};

// 17962 - Conflagrate - Updated to 4.3.4
class spell_warl_conflagrate : public SpellScriptLoader
{
    public:
        spell_warl_conflagrate() : SpellScriptLoader("spell_warl_conflagrate") { }

        class spell_warl_conflagrate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_conflagrate_SpellScript);

            bool Load()
            {
                immolateAmount = 0;
                return true;
            }

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_IMMOLATE))
                    return false;
                return true;
            }

            SpellCastResult CheckCast()
            {
                if (Unit* target = GetExplTargetUnit())
                    if (AuraEffect const* aurEff = target->GetAuraEffect(SPELL_WARLOCK_IMMOLATE, EFFECT_2, GetCaster()->GetGUID()))
                        immolateAmount = aurEff->GetAmount() * aurEff->GetTotalTicks();

                return SPELL_CAST_OK;
            }

            void HandleHit(SpellEffIndex /*effIndex*/)
            {
                if (!GetSpellInfo())
                    return;

                if (Unit* target = GetHitUnit())
                {
                    int32 damage = CalculatePct(immolateAmount, GetSpellInfo()->Effects[EFFECT_1].CalcValue(GetCaster()));
                    damage *= target->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, GetSpellInfo()->GetSchoolMask());
                    SetHitDamage(damage);
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_warl_conflagrate_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_warl_conflagrate_SpellScript::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }

            uint32 immolateAmount;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_conflagrate_SpellScript();
        }
};

// Rain of fire
class spell_warl_rain_of_fire : public SpellScriptLoader
{
    public:
        spell_warl_rain_of_fire() : SpellScriptLoader("spell_warl_rain_of_fire") { }

        class spell_warl_rain_of_fire_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_rain_of_fire_SpellScript);

            void HandleHit(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (AuraEffect const* aurEff = caster->GetAuraEffect(SPELL_AURA_PROC_TRIGGER_SPELL, SPELLFAMILY_WARLOCK, 11, EFFECT_0))
                    if (roll_chance_i(aurEff->GetSpellInfo()->Effects[EFFECT_1].BasePoints))
                        caster->CastSpell(GetHitUnit(), 85387, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_rain_of_fire_SpellScript::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_rain_of_fire_SpellScript();
        }
};

// Seduce
class spell_warl_seduction : public SpellScriptLoader
{
    public:
        spell_warl_seduction() : SpellScriptLoader("spell_warl_seduction") { }

        class spell_warl_seduction_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_seduction_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster()->GetOwner();
                if (!caster)
                    return;
                Unit* target = GetHitUnit();
                // Glyph removes dots on hit
                if (caster->GetDummyAuraEffect(SPELLFAMILY_WARLOCK, 48, EFFECT_0))
                {
                    if (!target)
                        return;

                    target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE, 0, target->GetAura(32409)); // SW:D shall not be removed.
                    target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
                    target->RemoveAurasByType(SPELL_AURA_PERIODIC_LEECH);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_seduction_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_seduction_SpellScript();
        }
};

// 6201 - Create Healthstone
class spell_warl_create_healthstone : public SpellScriptLoader
{
    public:
        spell_warl_create_healthstone() : SpellScriptLoader("spell_warl_create_healthstone") { }

        class spell_warl_create_healthstone_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_create_healthstone_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_CREATE_HEALTHSTONE))
                    return false;

                return true;
            }

            SpellCastResult CheckCast()
            {
                if (Player* caster = GetCaster()->ToPlayer())
                {
                    uint32 healthstoneId = sSpellMgr->GetSpellInfo(SPELL_WARLOCK_CREATE_HEALTHSTONE)->Effects[0].ItemType;
                    ItemPosCountVec dest;
                    InventoryResult msg = caster->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, healthstoneId, 1, NULL);
                    if (msg != EQUIP_ERR_OK)
                        return SPELL_FAILED_TOO_MANY_OF_ITEM;
                }
                return SPELL_CAST_OK;
            }

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                GetCaster()->CastSpell(GetCaster(), SPELL_WARLOCK_CREATE_HEALTHSTONE, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_create_healthstone_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
                OnCheckCast += SpellCheckCastFn(spell_warl_create_healthstone_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_create_healthstone_SpellScript();
        }
};

// 603 - Bane of Doom
/// Updated 4.3.4
class spell_warl_bane_of_doom : public SpellScriptLoader
{
    public:
        spell_warl_bane_of_doom() : SpellScriptLoader("spell_warl_bane_of_doom") { }

        class spell_warl_curse_of_doom_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_curse_of_doom_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_BANE_OF_DOOM_EFFECT))
                    return false;
                return true;
            }

            bool Load()
            {
                return GetCaster() && GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void onPeriodicTick(AuraEffect const* /*aurEff*/)
            {

                if (AuraEffect* aurEff = GetCaster()->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_WARLOCK, WARLOCK_ICON_IMPENDING_DOOM, EFFECT_0))
                {
                    int32 basepoints = aurEff->GetAmount() + 20;
                    if (roll_chance_i(basepoints))
                        GetCaster()->CastSpell(GetTarget(), SPELL_WARLOCK_BANE_OF_DOOM_EFFECT, true);
                }
                else if (roll_chance_i(20))
                    GetCaster()->CastSpell(GetTarget(), SPELL_WARLOCK_BANE_OF_DOOM_EFFECT, true);
            }

            void Register()
            {
                 OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_curse_of_doom_AuraScript::onPeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_curse_of_doom_AuraScript();
        }
};

// 48018 - Demonic Circle: Summon
/// Updated 4.3.4
class spell_warl_demonic_circle_summon : public SpellScriptLoader
{
    public:
        spell_warl_demonic_circle_summon() : SpellScriptLoader("spell_warl_demonic_circle_summon") { }

        class spell_warl_demonic_circle_summon_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_demonic_circle_summon_AuraScript);

            void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
            {
                // If effect is removed by expire remove the summoned demonic circle too.
                if (!(mode & AURA_EFFECT_HANDLE_REAPPLY))
                    GetTarget()->RemoveGameObject(GetId(), true);

                GetTarget()->RemoveAura(SPELL_WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST);
            }

            void HandleDummyTick(AuraEffect const* /*aurEff*/)
            {
                if (GameObject* circle = GetTarget()->GetGameObject(GetId()))
                {
                    // Here we check if player is in demonic circle teleport range, if so add
                    // WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST; allowing him to cast the WARLOCK_DEMONIC_CIRCLE_TELEPORT.
                    // If not in range remove the WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST.

                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMONIC_CIRCLE_TELEPORT);

                    if (GetTarget()->IsWithinDist(circle, spellInfo->GetMaxRange(true)))
                    {
                        if (!GetTarget()->HasAura(SPELL_WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST))
                            GetTarget()->CastSpell(GetTarget(), SPELL_WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST, true);
                    }
                    else
                        GetTarget()->RemoveAura(SPELL_WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST);
                }
            }

            void Register()
            {
                OnEffectRemove += AuraEffectApplyFn(spell_warl_demonic_circle_summon_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_demonic_circle_summon_AuraScript::HandleDummyTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_demonic_circle_summon_AuraScript();
        }
};

// 48020 - Demonic Circle: Teleport
/// Updated 4.3.4
class spell_warl_demonic_circle_teleport : public SpellScriptLoader
{
    public:
        spell_warl_demonic_circle_teleport() : SpellScriptLoader("spell_warl_demonic_circle_teleport") { }

        class spell_warl_demonic_circle_teleport_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_demonic_circle_teleport_AuraScript);

            void HandleTeleport(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Player* player = GetTarget()->ToPlayer())
                {
                    if (GameObject* circle = player->GetGameObject(SPELL_WARLOCK_DEMONIC_CIRCLE_SUMMON))
                    {
                        player->NearTeleportTo(circle->GetPositionX(), circle->GetPositionY(), circle->GetPositionZ(), circle->GetOrientation());
                        player->RemoveAurasWithMechanic((1 << MECHANIC_SNARE));
                        if (aurEff->GetBase()->IsChangeBySoulBurn())
                            player->CastSpell(player, SPELL_WARLOCK_SOULBURN_DEMONIC_CIRCLE, true);
                    }
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_warl_demonic_circle_teleport_AuraScript::HandleTeleport, EFFECT_0, SPELL_AURA_MECHANIC_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        class spell_warl_demonic_circle_teleport_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_demonic_circle_teleport_SpellScript);

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                    if (caster->HasAura(SPELL_WARLOCK_SOULBURN))
                    {
                        GetSpell()->SetChangeBySoulBurn(true);
                        caster->RemoveAurasDueToSpell(SPELL_WARLOCK_SOULBURN);
                    }
            }

            SpellCastResult CheckCast()
            {
                if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMONIC_CIRCLE_TELEPORT))
                    if (Player* player = GetCaster()->ToPlayer())
                        if (GameObject* circle = player->GetGameObject(SPELL_WARLOCK_DEMONIC_CIRCLE_SUMMON))
                            if (player->IsWithinDist(circle, spellInfo->GetMaxRange(true)))
                                return SPELL_CAST_OK;
                return SPELL_FAILED_OUT_OF_RANGE;
            }

            void Register()
            {
                OnCast += SpellCastFn(spell_warl_demonic_circle_teleport_SpellScript::HandleOnCast);
                OnCheckCast += SpellCheckCastFn(spell_warl_demonic_circle_teleport_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_demonic_circle_teleport_SpellScript();
        }

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_demonic_circle_teleport_AuraScript();
        }
};

// 77801 - Demon Soul - Updated to 4.3.4
class spell_warl_demon_soul : public SpellScriptLoader
{
    public:
        spell_warl_demon_soul() : SpellScriptLoader("spell_warl_demon_soul") { }

        class spell_warl_demon_soul_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_demon_soul_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMON_SOUL_IMP))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMON_SOUL_FELHUNTER))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMON_SOUL_FELGUARD))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMON_SOUL_SUCCUBUS))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMON_SOUL_VOIDWALKER))
                    return false;
                return true;
            }

            void OnHitTarget(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Creature* targetCreature = GetHitCreature())
                {
                    if (targetCreature->isPet())
                    {
                        CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(targetCreature->GetEntry());
                        switch (ci->family)
                        {
                            case CREATURE_FAMILY_SUCCUBUS:
                                caster->CastSpell(caster, SPELL_WARLOCK_DEMON_SOUL_SUCCUBUS);
                                break;
                            case CREATURE_FAMILY_VOIDWALKER:
                                caster->CastSpell(caster, SPELL_WARLOCK_DEMON_SOUL_VOIDWALKER);
                                break;
                            case CREATURE_FAMILY_FELGUARD:
                                caster->CastSpell(caster, SPELL_WARLOCK_DEMON_SOUL_FELGUARD);
                                break;
                            case CREATURE_FAMILY_FELHUNTER:
                                caster->CastSpell(caster, SPELL_WARLOCK_DEMON_SOUL_FELHUNTER);
                                break;
                            case CREATURE_FAMILY_IMP:
                                caster->CastSpell(caster, SPELL_WARLOCK_DEMON_SOUL_IMP);
                                break;
                            default:
                                break;
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_demon_soul_SpellScript::OnHitTarget, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_demon_soul_SpellScript;
        }
};

// 47193 - Demonic Empowerment
/// Updated 4.3.4
class spell_warl_demonic_empowerment : public SpellScriptLoader
{
    public:
        spell_warl_demonic_empowerment() : SpellScriptLoader("spell_warl_demonic_empowerment") { }

        class spell_warl_demonic_empowerment_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_demonic_empowerment_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMONIC_EMPOWERMENT_SUCCUBUS) || !sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMONIC_EMPOWERMENT_VOIDWALKER) || !sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMONIC_EMPOWERMENT_FELGUARD) || !sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMONIC_EMPOWERMENT_FELHUNTER) || !sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMONIC_EMPOWERMENT_IMP))
                    return false;
                return true;
            }

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                if (Creature* targetCreature = GetHitCreature())
                {
                    if (targetCreature->isPet())
                    {
                        CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(targetCreature->GetEntry());
                        switch (ci->family)
                        {
                            case CREATURE_FAMILY_SUCCUBUS:
                                targetCreature->CastSpell(targetCreature, SPELL_WARLOCK_DEMONIC_EMPOWERMENT_SUCCUBUS, true);
                                break;
                            case CREATURE_FAMILY_VOIDWALKER:
                            {
                                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DEMONIC_EMPOWERMENT_VOIDWALKER);
                                int32 hp = int32(targetCreature->CountPctFromMaxHealth(GetCaster()->CalculateSpellDamage(targetCreature, spellInfo, 0)));
                                targetCreature->CastCustomSpell(targetCreature, SPELL_WARLOCK_DEMONIC_EMPOWERMENT_VOIDWALKER, &hp, NULL, NULL, true);
                                break;
                            }
                            case CREATURE_FAMILY_FELGUARD:
                                targetCreature->CastSpell(targetCreature, SPELL_WARLOCK_DEMONIC_EMPOWERMENT_FELGUARD, true);
                                break;
                            case CREATURE_FAMILY_FELHUNTER:
                                targetCreature->CastSpell(targetCreature, SPELL_WARLOCK_DEMONIC_EMPOWERMENT_FELHUNTER, true);
                                break;
                            case CREATURE_FAMILY_IMP:
                                targetCreature->CastSpell(targetCreature, SPELL_WARLOCK_DEMONIC_EMPOWERMENT_IMP, true);
                                break;
                            default:
                                break;
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_demonic_empowerment_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_demonic_empowerment_SpellScript();
        }
};

// 47422 - Everlasting Affliction
/// Updated 4.3.4
class spell_warl_everlasting_affliction : public SpellScriptLoader
{
    public:
        spell_warl_everlasting_affliction() : SpellScriptLoader("spell_warl_everlasting_affliction") { }

        class spell_warl_everlasting_affliction_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_everlasting_affliction_SpellScript);

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                if (Unit* unitTarget = GetHitUnit())
                    // Refresh corruption on target
                    if (AuraEffect* aur = unitTarget->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_WARLOCK, 0x2, 0, 0, GetCaster()->GetGUID()))
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
                OnEffectHitTarget += SpellEffectFn(spell_warl_everlasting_affliction_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_everlasting_affliction_SpellScript();
        }
};

// 6353 Soulfire
// Updated 4.3.4
class spell_warl_soulfire : public SpellScriptLoader
{
    public:
        spell_warl_soulfire() : SpellScriptLoader("spell_warl_soulfire") { }

        class spell_warl_soulfire_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soulfire_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_IMPROVED_SOULFIRE))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (AuraEffect* aurEff = caster->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_WARLOCK, WARLOCK_ICON_SOULFIRE, EFFECT_0))
                    {
                        int32 basepoints = aurEff->GetAmount();
                        caster->CastCustomSpell(caster, SPELL_WARLOCK_IMPROVED_SOULFIRE, &basepoints, NULL, NULL, true);
                    }
                }
            }

            void HandleT13Bonus()
            {
                // Item - Warlock T13 4P Bonus (Soulburn)
                if (GetCaster()->HasAura(105787, GetCaster()->GetGUID()))
                    if (GetCaster()->HasAura(SPELL_WARLOCK_SOULBURN, GetCaster()->GetGUID()))
                        GetCaster()->CastSpell(GetCaster(), SPELL_WARLOCK_SOULSHARD_ADD, true);
            }
            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_soulfire_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
                OnCast += SpellCastFn(spell_warl_soulfire_SpellScript::HandleT13Bonus);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soulfire_SpellScript();
        }
};

// 77799 - Fel Flame - Updated to 4.3.4
class spell_warl_fel_flame : public SpellScriptLoader
{
    public:
        spell_warl_fel_flame() : SpellScriptLoader("spell_warl_fel_flame") { }

        class spell_warl_fel_flame_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_fel_flame_SpellScript);

            void OnHitTarget(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                Aura* aura = target->GetAura(SPELL_WARLOCK_UNSTABLE_AFFLICTION, caster->GetGUID());
                if (!aura)
                    aura = target->GetAura(SPELL_WARLOCK_IMMOLATE, caster->GetGUID());

                if (!aura)
                    return;

                int32 newDuration = aura->GetDuration() + GetSpellInfo()->Effects[EFFECT_1].CalcValue() * 1000;
                aura->SetDuration(std::min(newDuration, aura->GetMaxDuration()));
                aura->RecalculateAmountOfEffects();
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_fel_flame_SpellScript::OnHitTarget, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_fel_flame_SpellScript;
        }
};

// -47230 - Fel Synergy
class spell_warl_fel_synergy : public SpellScriptLoader
{
    public:
        spell_warl_fel_synergy() : SpellScriptLoader("spell_warl_fel_synergy") { }

        class spell_warl_fel_synergy_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_fel_synergy_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_FEL_SYNERGY_HEAL))
                    return false;
                return true;
            }

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                return GetTarget()->GetGuardianPet() && eventInfo.GetDamageInfo()->GetDamage();
            }

            void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                int32 heal = CalculatePct(int32(eventInfo.GetDamageInfo()->GetDamage()), aurEff->GetAmount());
                GetTarget()->CastCustomSpell(SPELL_WARLOCK_FEL_SYNERGY_HEAL, SPELLVALUE_BASE_POINT0, heal, (Unit*)NULL, true, NULL, aurEff); // TARGET_UNIT_PET
            }

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_warl_fel_synergy_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_warl_fel_synergy_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_fel_synergy_AuraScript();
        }
};

// 48181 - Haunt
/// Updated 4.3.4
class spell_warl_haunt : public SpellScriptLoader
{
    public:
        spell_warl_haunt() : SpellScriptLoader("spell_warl_haunt") { }

        class spell_warl_haunt_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_haunt_SpellScript);

            void HandleOnHit()
            {
                if (Aura* aura = GetHitAura())
                    if (AuraEffect* aurEff = aura->GetEffect(EFFECT_1))
                        aurEff->SetAmount(CalculatePct(aurEff->GetAmount(), GetHitDamage()));
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_haunt_SpellScript::HandleOnHit);
            }
        };

        class spell_warl_haunt_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_haunt_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_HAUNT_HEAL))
                    return false;
                return true;
            }

            void HandleRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    int32 amount = aurEff->GetAmount();
                    GetTarget()->CastCustomSpell(caster, SPELL_WARLOCK_HAUNT_HEAL, &amount, NULL, NULL, true, NULL, aurEff, GetCasterGUID());
                }
            }

            void Register()
            {
                OnEffectRemove += AuraEffectApplyFn(spell_warl_haunt_AuraScript::HandleRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_haunt_SpellScript();
        }

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_haunt_AuraScript();
        }
};

// 755 - Health Funnel
/// Updated 4.3.4
class spell_warl_health_funnel : public SpellScriptLoader
{
    public:
        spell_warl_health_funnel() : SpellScriptLoader("spell_warl_health_funnel") { }

        class spell_warl_health_funnel_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_health_funnel_AuraScript);

            void ApplyEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                Unit* target = GetTarget();
                if (caster->HasAura(SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_R2))
                    target->CastSpell(target, SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_BUFF_R2, true);
                else if (caster->HasAura(SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_R1))
                    target->CastSpell(target, SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_BUFF_R1, true);
            }

            void RemoveEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                target->RemoveAurasDueToSpell(SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_BUFF_R1);
                target->RemoveAurasDueToSpell(SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_BUFF_R2);
            }

            void DealFunnelDamage(AuraEffect const* /*aurEff*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                uint32 funnelDamage = caster->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_2].BasePoints);
                if (AuraEffect* aurEff = caster->GetAuraEffect(SPELL_AURA_ADD_PCT_MODIFIER, SPELLFAMILY_WARLOCK, 153, EFFECT_1))
                    funnelDamage = AddPct(funnelDamage, aurEff->GetAmount());

                uint32 funnelAbsorb = 0;
                caster->DealDamageMods(caster, funnelDamage, &funnelAbsorb);
                caster->SendSpellNonMeleeDamageLog(caster, GetId(), funnelDamage, GetSpellInfo()->GetSchoolMask(), funnelAbsorb, 0, false, 0, false);

                CleanDamage cleanDamage = CleanDamage(0, 0, BASE_ATTACK, MELEE_HIT_NORMAL);
                caster->DealDamage(caster, funnelDamage, &cleanDamage, NODAMAGE, GetSpellInfo()->GetSchoolMask(), GetSpellInfo(), true);
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_warl_health_funnel_AuraScript::RemoveEffect, EFFECT_0, SPELL_AURA_OBS_MOD_HEALTH, AURA_EFFECT_HANDLE_REAL);
                OnEffectApply += AuraEffectApplyFn(spell_warl_health_funnel_AuraScript::ApplyEffect, EFFECT_0, SPELL_AURA_OBS_MOD_HEALTH, AURA_EFFECT_HANDLE_REAL);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_health_funnel_AuraScript::DealFunnelDamage, EFFECT_0, SPELL_AURA_OBS_MOD_HEALTH);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_health_funnel_AuraScript();
        }
};

// 1454 - Life Tap
/// Updated 4.3.4
class spell_warl_life_tap : public SpellScriptLoader
{
    public:
        spell_warl_life_tap() : SpellScriptLoader("spell_warl_life_tap") { }

        class spell_warl_life_tap_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_life_tap_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_LIFE_TAP_ENERGIZE) || !sSpellMgr->GetSpellInfo(SPELL_WARLOCK_LIFE_TAP_ENERGIZE_2))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Player* caster = GetCaster()->ToPlayer();
                if (Unit* target = GetHitUnit())
                {
                    int32 damage = caster->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_2].CalcValue());
                    int32 mana = CalculatePct(damage, GetSpellInfo()->Effects[EFFECT_1].CalcValue());

                    // Shouldn't Appear in Combat Log
                    target->ModifyHealth(-damage);

                    // Improved Life Tap mod
                    if (AuraEffect const* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_WARLOCK, WARLOCK_ICON_ID_IMPROVED_LIFE_TAP, 0))
                        AddPct(mana, aurEff->GetAmount());

                    caster->CastCustomSpell(target, SPELL_WARLOCK_LIFE_TAP_ENERGIZE, &mana, NULL, NULL, false);

                    // Mana Feed
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_WARLOCK, WARLOCK_ICON_ID_MANA_FEED, 0))
                    {
                        int32 manaFeedVal = aurEff->GetAmount();
                        ApplyPct(manaFeedVal, mana);
                        if (caster->GetGuardianPet())
                            caster->CastCustomSpell(caster, SPELL_WARLOCK_LIFE_TAP_ENERGIZE_2, &manaFeedVal, NULL, NULL, true, NULL);
                    }
                }
            }

            SpellCastResult CheckCast()
            {
                if (int32(GetCaster()->GetHealth()) > int32(GetCaster()->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_2].CalcValue())))
                    return SPELL_CAST_OK;
                return SPELL_FAILED_FIZZLE;
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_life_tap_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
                OnCheckCast += SpellCheckCastFn(spell_warl_life_tap_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_life_tap_SpellScript();
        }
};

// 27285 - Seed of Corruption
/// Updated 4.3.4
class spell_warl_seed_of_corruption : public SpellScriptLoader
{
    public:
        spell_warl_seed_of_corruption() : SpellScriptLoader("spell_warl_seed_of_corruption") { }

        class spell_warl_seed_of_corruption_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_seed_of_corruption_SpellScript);

            bool inSoulBurn;

            bool Load()
            {
                inSoulBurn = false;
                return true;
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (GetExplTargetUnit())
                    targets.remove(GetExplTargetUnit());
            }

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                if (!inSoulBurn)
                {
                    if (GetSpell()->IsChangeBySoulBurn())
                    {
                        inSoulBurn = true;
                        caster->CastSpell(caster, SPELL_WARLOCK_SOULSHARD_ADD, true);
                    }
                }

                if (inSoulBurn)
                    if (Unit* target = GetHitUnit())
                        caster->CastSpell(target, 172, true);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warl_seed_of_corruption_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
                OnHit += SpellHitFn(spell_warl_seed_of_corruption_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_seed_of_corruption_SpellScript();
        }
};

// -7235 - Shadow Ward
// -91711 - Nether Ward
class spell_warl_wards : public SpellScriptLoader
{
    public:
        spell_warl_wards() : SpellScriptLoader("spell_warl_wards") { }

        class spell_warl_wards_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_wards_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
            {
                canBeRecalculated = false;
                if (Unit* caster = GetCaster())
                {
                    // +80.70% from sp bonus
                    float bonus = 0.807f;

                    bonus *= caster->SpellBaseHealingBonusDone(GetSpellInfo()->GetSchoolMask());
                    bonus *= caster->CalculateLevelPenalty(GetSpellInfo());

                    amount += int32(bonus);
                }
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& /*absorbAmount*/)
            {
                Unit* target = GetTarget();

                if (AuraEffect* talent = target->GetDummyAuraEffect(SPELLFAMILY_WARLOCK, 1985, EFFECT_0))
                {
                    uint32 spellId = 0;
                    switch (dmgInfo.GetSchoolMask())
                    {
                        case SPELL_SCHOOL_MASK_HOLY:
                            spellId = 54370;
                            break;
                        case SPELL_SCHOOL_MASK_FIRE:
                            spellId = 54371;
                            break;
                        case SPELL_SCHOOL_MASK_FROST:
                            spellId = 54372;
                            break;
                        case SPELL_SCHOOL_MASK_ARCANE:
                            spellId = 54373;
                            break;
                        case SPELL_SCHOOL_MASK_SHADOW:
                            spellId = 54374;
                            break;
                        case SPELL_SCHOOL_MASK_NATURE:
                            spellId = 54375;
                            break;
                        default:
                            break;
                    }
                    if (spellId)
                    {
                        int32 bp0 = -talent->GetAmount();
                        target->CastCustomSpell(target, spellId, &bp0, NULL, NULL, true);
                    }
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_wards_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_warl_wards_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_wards_AuraScript();
        }
};

// 29858 - Soulshatter
/// Updated 4.3.4
class spell_warl_soulshatter : public SpellScriptLoader
{
    public:
        spell_warl_soulshatter() : SpellScriptLoader("spell_warl_soulshatter") { }

        class spell_warl_soulshatter_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soulshatter_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOULSHATTER))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetHitUnit())
                    if (target->CanHaveThreatList() && target->getThreatManager().getThreat(caster) > 0.0f)
                        caster->CastSpell(target, SPELL_WARLOCK_SOULSHATTER, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_soulshatter_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soulshatter_SpellScript();
        }
};

// 30108, 34438, 34439, 35183 - Unstable Affliction
/// Updated 4.3.4
class spell_warl_unstable_affliction : public SpellScriptLoader
{
    public:
        spell_warl_unstable_affliction() : SpellScriptLoader("spell_warl_unstable_affliction") { }

        class spell_warl_unstable_affliction_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_unstable_affliction_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_UNSTABLE_AFFLICTION_DISPEL))
                    return false;

                return true;
            }

            void HandleDispel(DispelInfo* dispelInfo)
            {
                if (Unit* caster = GetCaster())
                    if (AuraEffect const* aurEff = GetFirstEffectOfType(SPELL_AURA_PERIODIC_DAMAGE))
                    {
                        int32 damage = aurEff->GetAmount() * 9;
                        // backfire damage and silence
                        caster->CastCustomSpell(dispelInfo->GetDispeller(), SPELL_WARLOCK_UNSTABLE_AFFLICTION_DISPEL, &damage, NULL, NULL, true, NULL, aurEff);
                    }
            }

            void Register()
            {
                AfterDispel += AuraDispelFn(spell_warl_unstable_affliction_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_unstable_affliction_AuraScript();
        }
};

class spell_warl_curse_of_the_elements : public SpellScriptLoader
{
    public:
        spell_warl_curse_of_the_elements() : SpellScriptLoader("spell_warl_curse_of_the_elements") { }

        class spell_warl_curse_of_the_elements_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_curse_of_the_elements_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_R1))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_R2))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_CoE_R1))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_JINX_CoE_R2))
                    return false;

                return true;
            }

            void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetTarget();
                if (!caster || ! target)
                    return;

                if (caster->HasAura(SPELL_WARLOCK_JINX_R2))
                    caster->CastSpell(target, SPELL_WARLOCK_JINX_CoE_R2, true);
                else if (caster->HasAura(SPELL_WARLOCK_JINX_R1))
                    caster->CastSpell(target, SPELL_WARLOCK_JINX_CoE_R1, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_curse_of_the_elements_AuraScript::HandleEffectPeriodic, EFFECT_2, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_curse_of_the_elements_AuraScript();
        }
};

class spell_warl_jinx_coe : public SpellScriptLoader
{
    public:
        spell_warl_jinx_coe() : SpellScriptLoader("spell_warl_jinx_coe") { }

        class spell_warl_jinx_coe_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_jinx_coe_SpellScript);

            void FilterTargetsInitial(std::list<WorldObject*>& targets)
            {
                targets.remove_if(Trinity::UnitAuraCheck(true, SPELL_WARLOCK_CoE));
                if (targets.size() > 15)
                    targets.resize(15);

                sharedTargets = targets;
            }

            void FilterTargetsSubsequent(std::list<WorldObject*>& targets)
            {
                targets = sharedTargets;
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warl_jinx_coe_SpellScript::FilterTargetsInitial, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warl_jinx_coe_SpellScript::FilterTargetsSubsequent, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
            }

            std::list<WorldObject*> sharedTargets;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_jinx_coe_SpellScript();
        }
};

class spell_warl_drain_life : public SpellScriptLoader
{
    public:
        spell_warl_drain_life() : SpellScriptLoader("spell_warl_drain_life") { }

        class spell_warl_drain_life_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_drain_life_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DRAIN_LIFE_HEALTH))
                    return false;

                return true;
            }

            void onPeriodicTick(AuraEffect const* /*aurEff*/)
            {
                if (Unit* caster = GetCaster())
                {
                    int32 baseAmount = caster->CalculateSpellDamage(caster, sSpellMgr->GetSpellInfo(SPELL_WARLOCK_DRAIN_LIFE_HEALTH), 0);

                    if (AuraEffect const* aurEff = caster->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_WARLOCK, 3223, 0))
                        if (caster->HealthBelowPct(25))
                            baseAmount += int32(aurEff->GetAmount());

                    caster->CastCustomSpell(caster, SPELL_WARLOCK_DRAIN_LIFE_HEALTH, &baseAmount, NULL, NULL, true);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_drain_life_AuraScript::onPeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_drain_life_AuraScript();
        }
};

class spell_warl_soul_swap : public SpellScriptLoader
{
    public:
        spell_warl_soul_swap() : SpellScriptLoader("spell_warl_soul_swap") { }

        class spell_warl_soul_swap_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soul_swap_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_OVERRIDE))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_GLYPH))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_COOLDOWN))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_VISUAL))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_FREE_DOTS_COSTS))
                    return false;

                return true;
            }

            std::list<uint32> dotsList;

            void HandleSoulSwap(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || ! target)
                    return;

                caster->CastSpell(caster, SPELL_WARLOCK_SOUL_SWAP_OVERRIDE, true);
                target->CastSpell(caster, SPELL_WARLOCK_SOUL_SWAP_VISUAL, true);

                for (std::list<uint32>::const_iterator itr = dotsList.begin(); itr != dotsList.end(); itr++)
                {
                    if (!caster->HasAura(SPELL_WARLOCK_SOUL_SWAP_GLYPH))
                        target->RemoveAura(*itr, caster->GetGUID());
                }

                caster->SaveSoulSwapDotsListAndTarget(target->GetGUID(), dotsList);
            }

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                Unit* target = GetExplTargetUnit();
                if (!caster || ! target)
                    return SPELL_FAILED_BAD_TARGETS;

                Unit::AuraEffectList const& auras = target->GetAuraDotsByCaster(caster->GetGUID());
                if (auras.empty())
                    return SPELL_FAILED_BAD_TARGETS;

                for (Unit::AuraEffectList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
                {
                    if ((*itr)->GetSpellInfo()->SpellFamilyFlags & sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_FREE_DOTS_COSTS)->Effects[0].SpellClassMask)
                        dotsList.push_back((*itr)->GetId());
                }

                return dotsList.empty() ? SPELL_FAILED_BAD_TARGETS : SPELL_CAST_OK;
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_soul_swap_SpellScript::HandleSoulSwap, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
                OnCheckCast += SpellCheckCastFn(spell_warl_soul_swap_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soul_swap_SpellScript();
        }
};

class spell_warl_soul_swap_exhale : public SpellScriptLoader
{
    public:
        spell_warl_soul_swap_exhale() : SpellScriptLoader("spell_warl_soul_swap_exhale") { }

        class spell_warl_soul_swap_exhale_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soul_swap_exhale_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_OVERRIDE))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_SWAP_FREE_DOTS_COSTS))
                    return false;

                return true;
            }

            std::list<uint32> dotsList;

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                {
                    dotsList = caster->GetSoulSwapDotsList();
                    caster->RemoveAurasDueToSpell(SPELL_WARLOCK_SOUL_SWAP_OVERRIDE);
                }
            }

            void HandleSoulSwapExhale(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || ! target)
                    return;

                if (dotsList.empty())
                    return;

                for (std::list<uint32>::const_iterator itr = dotsList.begin(); itr != dotsList.end(); ++itr)
                {
                    caster->CastSpell(caster, SPELL_WARLOCK_SOUL_SWAP_FREE_DOTS_COSTS, true);
                    caster->CastSpell(target, *itr, true);
                }

                caster->RemoveAurasDueToSpell(SPELL_WARLOCK_SOUL_SWAP_FREE_DOTS_COSTS); // use for prevention
            }

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                Unit* target = GetExplTargetUnit();
                if (!caster || ! target)
                    return SPELL_FAILED_BAD_TARGETS;

                if (caster->GetSoulSwapTarget() == target->GetGUID())
                    return SPELL_FAILED_BAD_TARGETS;

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnEffectLaunchTarget += SpellEffectFn(spell_warl_soul_swap_exhale_SpellScript::HandleSoulSwapExhale, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
                OnCheckCast += SpellCheckCastFn(spell_warl_soul_swap_exhale_SpellScript::CheckCast);
                OnCast += SpellCastFn(spell_warl_soul_swap_exhale_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soul_swap_exhale_SpellScript();
        }
};

class spell_warl_soul_swap_override : public SpellScriptLoader
{
    public:
        spell_warl_soul_swap_override() : SpellScriptLoader("spell_warl_soul_swap_override") { }

        class spell_warl_soul_swap_override_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_soul_swap_override_AuraScript);

            void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    caster->ClearSoulSwapDotsList();

                    if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_CANCEL)
                        if (caster->HasAura(SPELL_WARLOCK_SOUL_SWAP_GLYPH))
                            caster->CastSpell(caster, SPELL_WARLOCK_SOUL_SWAP_COOLDOWN, false);
                }
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_warl_soul_swap_override_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_soul_swap_override_AuraScript();
        }
};

class spell_warl_soulburn : public SpellScriptLoader
{
    public:
        spell_warl_soulburn() : SpellScriptLoader("spell_warl_soulburn") { }

        class spell_warl_soulburn_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soulburn_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOULBURN_SOC_VISUAL))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOULBURN_SEED_OF_CORRUPTION))
                    return false;

                return true;
            }

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                    if (caster->HasSpell(SPELL_WARLOCK_SOULBURN_SEED_OF_CORRUPTION))
                        caster->CastSpell(caster, SPELL_WARLOCK_SOULBURN_SOC_VISUAL, true);
            }

            void Register()
            {
                OnCast += SpellCastFn(spell_warl_soulburn_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soulburn_SpellScript();
        }
};

class spell_warl_soulburn_seed_of_corruption : public SpellScriptLoader
{
    public:
        spell_warl_soulburn_seed_of_corruption() : SpellScriptLoader("spell_warl_soulburn_seed_of_corruption") { }

        class spell_warl_soulburn_seed_of_corruption_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_soulburn_seed_of_corruption_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOULBURN))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOULBURN_SEED_OF_CORRUPTION))
                    return false;

                return true;
            }

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                    if (caster->HasAura(SPELL_WARLOCK_SOULBURN) && caster->HasSpell(SPELL_WARLOCK_SOULBURN_SEED_OF_CORRUPTION))
                    {
                        GetSpell()->SetChangeBySoulBurn(true);
                        caster->RemoveAurasDueToSpell(SPELL_WARLOCK_SOULBURN);
                    }
            }

            void Register()
            {
                OnCast += SpellCastFn(spell_warl_soulburn_seed_of_corruption_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_soulburn_seed_of_corruption_SpellScript();
        }
};

class spell_warl_pandemic : public SpellScriptLoader
{
    public:
        spell_warl_pandemic() : SpellScriptLoader("spell_warl_pandemic") { }

        class spell_warl_pandemic_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_pandemic_SpellScript);

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                if (Unit* unitTarget = GetHitUnit())
                    // Refresh untable affliction on target
                    if (AuraEffect* aur = unitTarget->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_WARLOCK, 0, 0x100, 0, GetCaster()->GetGUID()))
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
                OnEffectHitTarget += SpellEffectFn(spell_warl_pandemic_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_pandemic_SpellScript();
        }
};

class spell_warl_drain_soul : public SpellScriptLoader
{
    public:
        spell_warl_drain_soul() : SpellScriptLoader("spell_warl_drain_soul") { }

        class spell_warl_drain_soul_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_drain_soul_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_PANDEMIC))
                    return false;

                return true;
            }

            void onPeriodicTick(AuraEffect const* /*aurEff*/)
            {
                Unit* target = GetTarget();
                Unit* caster = GetCaster();
                if (!target || !caster)
                    return;

                if (target->HealthBelowPct(25))
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_WARLOCK, 4554, EFFECT_1))
                        if (Aura *aura = aurEff->GetBase())
                            if (roll_chance_i(aura->GetSpellInfo()->Effects[EFFECT_0].BasePoints))
                                caster->CastSpell(target, SPELL_WARLOCK_PANDEMIC, true);

            }

            void RemoveEffect(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
                {
                    if (Player *caster = GetCaster()->ToPlayer())
                    {
                        if (Unit *target = GetTarget())
                        {
                            if (caster->isHonorOrXPTarget(target))
                            {
                                if (caster->HasAura(58070)) // Glyph of Drain Soul
                                    caster->CastSpell(caster, 58068, true);

                                caster->CastSpell(caster, 79264, true, 0, aurEff);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_drain_soul_AuraScript::onPeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
                OnEffectRemove += AuraEffectRemoveFn(spell_warl_drain_soul_AuraScript::RemoveEffect, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_drain_soul_AuraScript();
        }
};

class spell_warl_soulburn_healthstone : public SpellScriptLoader
{
    public:
        spell_warl_soulburn_healthstone() : SpellScriptLoader("spell_warl_soulburn_healthstone") { }

        class spell_warl_soulburn_healthstone_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_soulburn_healthstone_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                amount = GetUnitOwner()->CountPctFromMaxHealth(amount);
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_soulburn_healthstone_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_INCREASE_HEALTH);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_soulburn_healthstone_AuraScript();
        }
};

class spell_warl_searing_pain : public SpellScriptLoader
{
    public:
        spell_warl_searing_pain() : SpellScriptLoader("spell_warl_searing_pain") { }

        class spell_warl_searing_pain_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_searing_pain_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOULBURN_SEARING_PAIN))
                    return false;
                return true;
            }

            void HandleOnHit(SpellEffIndex /*effIndex*/)
            {
                if (Player* caster = GetCaster()->ToPlayer())
                    if (GetHitUnit())
                        if (GetSpell()->IsChangeBySoulBurn())
                            caster->CastSpell(caster, SPELL_WARLOCK_SOULBURN_SEARING_PAIN, true);
            }

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                    if (caster->HasAura(SPELL_WARLOCK_SOULBURN))
                    {
                        GetSpell()->SetChangeBySoulBurn(true);
                        caster->RemoveAurasDueToSpell(SPELL_WARLOCK_SOULBURN);
                    }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_searing_pain_SpellScript::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
                OnCast += SpellCastFn(spell_warl_searing_pain_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_searing_pain_SpellScript();
        }
};

class spell_warl_soul_harvest : public SpellScriptLoader
{
    public:
        spell_warl_soul_harvest() : SpellScriptLoader("spell_warl_soul_harvest") { }

        class spell_warl_soul_harvest_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_soul_harvest_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOUL_HARVEST_SOULSHARD))
                    return false;
                return true;
            }

            void onPeriodicTick(AuraEffect const* /*aurEff*/)
            {
                if (Unit* caster = GetCaster())
                    if (caster->GetPower(POWER_SOUL_SHARDS) != caster->GetMaxPower(POWER_SOUL_SHARDS))
                        caster->CastSpell(caster, SPELL_WARLOCK_SOUL_HARVEST_SOULSHARD, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_soul_harvest_AuraScript::onPeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_soul_harvest_AuraScript();
        }
};

class spell_warl_fel_armor : public SpellScriptLoader
{
    public:
        spell_warl_fel_armor() : SpellScriptLoader("spell_warl_fel_armor") { }

        class spell_warl_fel_armor_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_fel_armor_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_FEL_ARMOR_HEAL))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SHADOW_WARD))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_NETHER_WARD))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_NETHER_WARD_TALENT))
                    return false;

                return true;
            }

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                return eventInfo.GetDamageInfo()->GetDamage() && !eventInfo.GetDamageInfo()->GetSpellInfo()->IsTargetingArea() &&
                    !eventInfo.GetDamageInfo()->GetSpellInfo()->IsAffectingArea() && !eventInfo.GetDamageInfo()->GetSpellInfo()->IsEffectAffectingOrTargetingArea();
            }

            void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                Unit *caster = GetCaster();
                if (!caster)
                    return;

                PreventDefaultAction();

                int32 heal = CalculatePct(int32(eventInfo.GetDamageInfo()->GetDamage()), aurEff->GetAmount());
                caster->CastCustomSpell(caster, SPELL_WARLOCK_FEL_ARMOR_HEAL, &heal, NULL, NULL, true, NULL, aurEff, GetCasterGUID());
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
            {
                canBeRecalculated = false;
                if (Unit* caster = GetCaster())
                {
                    if (caster->HasAura(SPELL_WARLOCK_NETHER_WARD_TALENT)) // Nether Ward
                        amount = SPELL_WARLOCK_NETHER_WARD;
                    else
                        amount = SPELL_WARLOCK_SHADOW_WARD;
                }
                else
                    amount = SPELL_WARLOCK_SHADOW_WARD;
            }

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_warl_fel_armor_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_warl_fel_armor_AuraScript::OnProc, EFFECT_1, SPELL_AURA_DUMMY);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_fel_armor_AuraScript::CalculateAmount, EFFECT_2, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_fel_armor_AuraScript();
        }
};

class spell_warl_mana_feed : public SpellScriptLoader
{
    public:
        spell_warl_mana_feed() : SpellScriptLoader("spell_warl_mana_feed") { }

        class spell_warl_mana_feed_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_mana_feed_SpellScript);

            void HandleEnergize(SpellEffIndex /*effIndex*/)
            {
                Unit* target = GetHitUnit();
                if (!target)
                    return;

                if (AuraEffect* aur = target->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_WARLOCK, 1982, EFFECT_2))
                    SetEffectDamage(aur->GetAmount());
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_mana_feed_SpellScript::HandleEnergize, EFFECT_0, SPELL_EFFECT_ENERGIZE_PCT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_mana_feed_SpellScript();
        }
};

class spell_warl_impending_doom : public SpellScriptLoader
{
    public:
        spell_warl_impending_doom() : SpellScriptLoader("spell_warl_impending_doom") { }

        class spell_warl_impending_doom_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_impending_doom_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_METHAMORPHOSIS_FORM))
                    return false;

                return true;
            }

            bool CheckProc(ProcEventInfo& /*eventInfo*/)
            {
                return GetCaster()->ToPlayer() && GetCaster()->ToPlayer()->HasSpellCooldown(SPELL_WARLOCK_METHAMORPHOSIS_FORM);
            }

            void OnProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
            {
                Player* caster = GetCaster()->ToPlayer();
                if (!caster)
                    return;

                PreventDefaultAction();

                int32 cooldown = caster->GetSpellCooldownDelay(SPELL_WARLOCK_METHAMORPHOSIS_FORM);
                int32 amount = aurEff->GetAmount();

                if (cooldown < amount)
                    cooldown = 0;
                else
                    cooldown -= amount;

                caster->AddSpellCooldown(SPELL_WARLOCK_METHAMORPHOSIS_FORM, 0, uint32(time(NULL) + cooldown));
                WorldPacket data(SMSG_MODIFY_COOLDOWN, 4+8+4);
                data << uint32(SPELL_WARLOCK_METHAMORPHOSIS_FORM);
                data << uint64(caster->GetGUID());
                data << int32(-(amount*1000));
                caster->GetSession()->SendPacket(&data);
            }

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_warl_impending_doom_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_warl_impending_doom_AuraScript::OnProc, EFFECT_1, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_impending_doom_AuraScript();
        }
};

class spell_warl_cremation : public SpellScriptLoader
{
    public:
        spell_warl_cremation() : SpellScriptLoader("spell_warl_cremation") { }

        class spell_warl_cremation_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_cremation_SpellScript);

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                if (Unit* unitTarget = GetHitUnit())
                    // Refresh immolate on target
                    if (AuraEffect* aur = unitTarget->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_WARLOCK, 0x00000004, 0, 0, GetCaster()->GetGUID()))
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
                OnEffectHitTarget += SpellEffectFn(spell_warl_cremation_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_cremation_SpellScript();
        }
};

class spell_warl_shadowburn : public SpellScriptLoader
{
    public:
        spell_warl_shadowburn() : SpellScriptLoader("spell_warl_shadowburn") { }

        class spell_warl_shadowburn_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_shadowburn_SpellScript);

            void HandleDamage()
            {
                Unit* caster = GetCaster();
                if (caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                Unit* target = GetHitUnit();
                int32 damage = GetHitDamage();
                // Glyph cooldown reset when target was failed to kill
                if (AuraEffect* glyph = caster->GetDummyAuraEffect(SPELLFAMILY_WARLOCK, 1590, EFFECT_0))
                    if (target && target->GetHealthPct() <= glyph->GetAmount() && !caster->HasAura(91001))
                        if (int32(target->GetHealth()) > damage)
                        {
                            caster->ToPlayer()->RemoveSpellCooldown(17877, true);
                            caster->CastSpell(caster, 91001, true);
                        }

            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_shadowburn_SpellScript::HandleDamage);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_shadowburn_SpellScript();
        }
};

class spell_warl_demon_armor : public SpellScriptLoader
{
    public:
        spell_warl_demon_armor() : SpellScriptLoader("spell_warl_demon_armor") { }

        class spell_warl_demon_armor_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_demon_armor_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SHADOW_WARD))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_NETHER_WARD))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_NETHER_WARD_TALENT))
                    return false;

                return true;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
            {
                canBeRecalculated = false;
                if (Unit* caster = GetCaster())
                {
                    if (caster->HasAura(SPELL_WARLOCK_NETHER_WARD_TALENT)) // Nether Ward
                        amount = SPELL_WARLOCK_NETHER_WARD;
                    else
                        amount = SPELL_WARLOCK_SHADOW_WARD;
                }
                else
                    amount = SPELL_WARLOCK_SHADOW_WARD;
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_demon_armor_AuraScript::CalculateAmount, EFFECT_2, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_demon_armor_AuraScript();
        }
};

class spell_warl_soul_link : public SpellScriptLoader
{
    public:
        spell_warl_soul_link() : SpellScriptLoader("spell_warl_soul_link") { }

        class spell_warl_soul_link_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_soul_link_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
            {
                canBeRecalculated = false;
                if (Unit* caster = GetCaster())
                    if (Unit* owner = caster->GetOwner())
                        if (AuraEffect* glyph = owner->GetDummyAuraEffect(SPELLFAMILY_WARLOCK, 173, EFFECT_0))
                            amount += glyph->GetAmount();
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_soul_link_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SPLIT_DAMAGE_PCT);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_soul_link_AuraScript();
        }
};

class spell_warl_shadowburn_aura : public SpellScriptLoader
{
    public:
        spell_warl_shadowburn_aura() : SpellScriptLoader("spell_warl_shadowburn_aura") { }

        class spell_warl_shadowburn_aura_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_shadowburn_aura_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_SOULSHARD_ADD))
                    return false;

                return true;
            }

            void RemoveEffect(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster() && GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
                {
                    if (Player* caster = GetCaster()->ToPlayer())
                    {
                        if (caster->isHonorOrXPTarget(GetTarget()))
                        {
                            int32 basepoints = aurEff->GetAmount();
                            caster->CastCustomSpell(caster, SPELL_WARLOCK_SOULSHARD_ADD, &basepoints, NULL, NULL, true, NULL, aurEff);
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_warl_shadowburn_aura_AuraScript::RemoveEffect, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_warl_shadowburn_aura_AuraScript();
        }
};

// Fear
class spell_warl_fear: public SpellScriptLoader
{
    public:
        spell_warl_fear() : SpellScriptLoader("spell_warl_fear") { }

        class spell_warl_fear_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_fear_SpellScript);

            void HandleOnHit()
            {
                if (!GetHitUnit())
                    return;

                AuraEffect* glyph = GetCaster()->GetAuraEffect(56244, EFFECT_0);
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

                if (glyph)
                    GetCaster()->CastSpell(GetCaster(), 91168);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_warl_fear_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_fear_SpellScript();
        }
};

// Devour Magic
class spell_warl_devour: public SpellScriptLoader
{
    public:
        spell_warl_devour() : SpellScriptLoader("spell_warl_devour") { }

        class spell_warl_devour_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_devour_SpellScript);

            void HandleOnDispel()
            {
                Unit* caster = GetCaster();
                if (Unit* owner = caster->GetOwner())
                {
                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(19658);
                    int32 heal_amount = spellInfo->Effects[EFFECT_0].CalcValue(caster) + (owner->SpellBaseDamageBonusDone(spellInfo->GetSchoolMask()) * 0.5f * 0.3f);
                    caster->CastCustomSpell(caster, 19658, &heal_amount, NULL, NULL, true);
                    // Glyph of Felhunter
                    if (owner->GetAura(56249))
                        owner->CastCustomSpell(owner, 19658, &heal_amount, NULL, NULL, true);
                }
            }

            void Register()
            {
                OnSuccessfulDispel += SpellDispelFn(spell_warl_devour_SpellScript::HandleOnDispel);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_warl_devour_SpellScript();
        }
};

class spell_soulstone : public SpellScriptLoader
{
public:
    spell_soulstone() : SpellScriptLoader("spell_soulstone") { }

    class spell_soulstone_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_soulstone_SpellScript);

        void HandleScript(SpellEffIndex effectIndex)
        {
            if (GetHitUnit() && GetHitUnit()->isDead())
            {
                PreventHitAura();
                PreventHitEffect(effectIndex);
                GetCaster()->CastSpell(GetHitUnit(), SPELL_WARLOCK_SOULSTONE_RESURRECT, true);
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_soulstone_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_soulstone_SpellScript();
    }
};

class spell_warl_bane_of_havoc : public SpellScriptLoader
{
public:
    spell_warl_bane_of_havoc() : SpellScriptLoader("spell_warl_bane_of_havoc") { }

    class spell_warl_bane_of_havoc_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_bane_of_havoc_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
                caster->CastSpell(caster, SPELL_WARLOCK_BANE_OF_HAVOC_TRACK, true);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_warl_bane_of_havoc_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_warl_bane_of_havoc_AuraScript();
    }
};

class spell_warl_bane_of_havoc_track : public SpellScriptLoader
{
public:
    spell_warl_bane_of_havoc_track() : SpellScriptLoader("spell_warl_bane_of_havoc_track") { }

    class spell_warl_bane_of_havoc_track_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_bane_of_havoc_track_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_WARLOCK_BANE_OF_HAVOC_DAMAGE))
                return false;
            return true;
        }

        void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            int32 damage = int32(eventInfo.GetDamageInfo()->GetDamage()) * 0.15f;

            if (Unit* caster = GetCaster())
            {
                Unit::AuraList& scAuras = caster->GetSingleCastAuras();

                for (Unit::AuraList::iterator itr = scAuras.begin(); itr != scAuras.end(); ++itr)
                {
                    if ((*itr)->GetSpellInfo()->Id == SPELL_WARLOCK_BANE_OF_HAVOC)
                    {
                        if (Unit* target = (*itr)->GetUnitOwner())
                            if (caster->GetDistance2d(target) < 40.0f && target != eventInfo.GetActionTarget())
                                caster->CastCustomSpell(target, SPELL_WARLOCK_BANE_OF_HAVOC_DAMAGE, &damage, NULL, NULL, true);
                        break;
                    }
                }
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_warl_bane_of_havoc_track_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_warl_bane_of_havoc_track_AuraScript();
    }
};

class spell_warl_incinerate : public SpellScriptLoader
{
public:
    spell_warl_incinerate() : SpellScriptLoader("spell_warl_incinerate") { }

    class spell_warl_incinerate_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_warl_incinerate_SpellScript);

        void HandleHit(SpellEffIndex /*effIndex*/)
        {
            if (Unit* target = GetHitUnit())
            {
                if (target->HasAuraState(AURA_STATE_CONFLAGRATE))
                    if (target->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_WARLOCK, 0x4, 0, 0))
                        SetHitDamage(GetHitDamage() + GetHitDamage() / 6);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_warl_incinerate_SpellScript::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }

    };

    SpellScript* GetSpellScript() const
    {
        return new spell_warl_incinerate_SpellScript();
    }
};

void AddSC_warlock_spell_scripts()
{
    new spell_warl_bane_of_doom(); // Verified
    new spell_warl_banish(); // Verified
    new spell_warl_curse_of_weakness(); // Verified 
    new spell_warl_conflagrate(); // Verified 
    new spell_warl_create_healthstone(); // Verified
    new spell_warl_demonic_circle_summon(); // Verified
    new spell_warl_demonic_circle_teleport(); // Verified
    new spell_warl_demonic_empowerment(); // Verified 
    new spell_warl_demon_soul(); // Verified
    new spell_warl_everlasting_affliction(); // Verified
    new spell_warl_fel_flame(); // Verified 
    new spell_warl_fel_synergy(); // Verified
    new spell_warl_haunt(); // Verified 
    new spell_warl_health_funnel(); // Verified
    new spell_warl_life_tap(); // Verified
    new spell_warl_seed_of_corruption(); // Verified
    new spell_warl_soulshatter(); // Verified
    new spell_warl_unstable_affliction(); // Verified
    new spell_warl_curse_of_the_elements(); // Verified
    new spell_warl_jinx_coe(); // Verified
    new spell_warl_drain_life(); // Verified 
    new spell_warl_soul_swap(); // Verified
    new spell_warl_soul_swap_exhale(); // Verified 
    new spell_warl_soul_swap_override(); // Verified 
    new spell_warl_soulburn(); // Verified 
    new spell_warl_soulburn_seed_of_corruption(); // Verified 
    new spell_warl_pandemic(); // Verified 
    new spell_warl_drain_soul(); // Verified 
    new spell_warl_soulburn_healthstone(); // Verified 
    new spell_warl_searing_pain(); // Verified 
    new spell_warl_soul_harvest(); // Verified 
    new spell_warl_fel_armor(); // Verified 
    new spell_warl_mana_feed(); // Verified 
    new spell_warl_impending_doom(); // Verified
    new spell_warl_cremation(); // Verified
    new spell_warl_demon_armor(); // Verified
    new spell_warl_soulfire(); // Verified
    new spell_warl_rain_of_fire(); // Verified
    new spell_warl_seduction(); // Verified
    new spell_warl_shadowburn(); // Verified
    new spell_warl_soul_link(); // Verified
    new spell_warl_shadowburn_aura(); // Verified
    new spell_warl_wards(); // Verified
    new spell_warl_fear(); // Verified
    new spell_warl_devour(); // Verified
    new spell_soulstone(); // Recently Verified
    new spell_warl_bane_of_havoc(); // Verified
    new spell_warl_bane_of_havoc_track(); // Recently Verified
    new spell_warl_incinerate(); // Recently Verified
}
