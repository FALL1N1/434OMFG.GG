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
 * Scripts for spells with SPELLFAMILY_MAGE and SPELLFAMILY_GENERIC spells used by mage players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_mage_".
 */

#include "Player.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Pet.h"

enum MageSpells
{
    SPELL_MAGE_PIERCING_CHILL                    = 83154,
    SPELL_MAGE_EARLY_FROST_R1                    = 83162,
    SPELL_MAGE_EARLY_FROST_R2                    = 83239,
    SPELL_MAGE_COLD_SNAP                         = 11958,
    SPELL_MAGE_FOCUS_MAGIC_PROC                  = 54648,
    SPELL_MAGE_FROST_WARDING_R1                  = 11189,
    SPELL_MAGE_FROST_WARDING_TRIGGERED           = 57776,
    SPELL_MAGE_INCANTERS_ABSORBTION_R1           = 44394,
    SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED    = 44413,
    SPELL_MAGE_IGNITE                            = 12654,
    SPELL_MAGE_MASTER_OF_ELEMENTS_ENERGIZE       = 29077,
    SPELL_MAGE_SQUIRREL_FORM                     = 32813,
    SPELL_MAGE_GIRAFFE_FORM                      = 32816,
    SPELL_MAGE_SERPENT_FORM                      = 32817,
    SPELL_MAGE_DRAGONHAWK_FORM                   = 32818,
    SPELL_MAGE_WORGEN_FORM                       = 32819,
    SPELL_MAGE_SHEEP_FORM                        = 32820,
    SPELL_MAGE_GLYPH_OF_ETERNAL_WATER            = 70937,
    SPELL_MAGE_SUMMON_WATER_ELEMENTAL_PERMANENT  = 70908,
    SPELL_MAGE_SUMMON_WATER_ELEMENTAL_TEMPORARY  = 70907,
    SPELL_MAGE_GLYPH_OF_BLAST_WAVE               = 62126,

    SPELL_MAGE_FLAMESTRIKE                       = 2120,

    SPELL_MAGE_CHILLED_R1                        = 12484,
    SPELL_MAGE_CHILLED_R2                        = 12485,

    SPELL_MAGE_CONE_OF_COLD_AURA_R1              = 11190,
    SPELL_MAGE_CONE_OF_COLD_AURA_R2              = 12489,
    SPELL_MAGE_CONE_OF_COLD_TRIGGER_R1           = 83301,
    SPELL_MAGE_CONE_OF_COLD_TRIGGER_R2           = 83302,

    SPELL_MAGE_SHATTERED_BARRIER_R1              = 44745,
    SPELL_MAGE_SHATTERED_BARRIER_R2              = 54787,
    SPELL_MAGE_SHATTERED_BARRIER_FREEZE_R1       = 55080,
    SPELL_MAGE_SHATTERED_BARRIER_FREEZE_R2       = 83073,

    SPELL_MAGE_IMPROVED_MANA_GEM_TRIGGERED       = 83098,

    SPELL_MAGE_RING_OF_FROST_SUMMON              = 82676,
    SPELL_MAGE_RING_OF_FROST_FREEZE              = 82691,
    SPELL_MAGE_RING_OF_FROST_DUMMY               = 91264,

    SPELL_MAGE_RITUAL_OF_REFRESHMENT_R1          = 74650,
    SPELL_MAGE_RITUAL_OF_REFRESHMENT_R2          = 92824,
    SPELL_MAGE_RITUAL_OF_REFRESHMENT_R3          = 92827,

    SPELL_MAGE_FINGERS_OF_FROST                  = 44544,

    SPELL_MAGE_GLYPH_OF_ICY_VEINS                = 56374,

    SPELL_MAGE_STOLEN_TIME                       = 105785,
    SPELL_MAGE_STOLEN_TIME_4P                    = 105791
};

enum MageIcons
{
    ICON_MAGE_SHATTER                            = 976,
    ICON_MAGE_IMPROVED_FLAMESTRIKE               = 37,
    ICON_MAGE_IMPROVED_FREEZE                    = 94,
    ICON_MAGE_INCANTER_S_ABSORPTION              = 2941,
    ICON_MAGE_IMPROVED_MANA_GEM                  = 1036
};

enum MagePetCalculate
{
    SPELL_MAGE_PET_SCALING_05         = 89764
};

// Incanter's Absorbtion
class spell_mage_incanters_absorbtion_base_AuraScript : public AuraScript
{
    public:
        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_INCANTERS_ABSORBTION_R1))
                return false;
            return true;
        }

        void Trigger(AuraEffect* aurEff, DamageInfo& /*dmgInfo*/, uint32& absorbAmount)
        {
            Unit* target = GetTarget();

            if (AuraEffect* talentAurEff = target->GetAuraEffectOfRankedSpell(SPELL_MAGE_INCANTERS_ABSORBTION_R1, EFFECT_0))
            {
                target->CastCustomSpell(SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED, SPELLVALUE_BASE_POINT0, absorbAmount, target, true, NULL, aurEff);
            }
        }
};

// 11113 - Blast Wave
class spell_mage_blast_wave : public SpellScriptLoader
{
    public:
        spell_mage_blast_wave() : SpellScriptLoader("spell_mage_blast_wave") { }

        class spell_mage_blast_wave_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_blast_wave_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_FLAMESTRIKE))
                    return false;
                return true;
            }

            void CountTargets(std::list<WorldObject*>& targetList)
            {
                _targetCount = targetList.size();
            }

            void HandleImprovedFlamestrike()
            {
                if (_targetCount >= 2)
                    if (AuraEffect* aurEff = GetCaster()->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_MAGE, ICON_MAGE_IMPROVED_FLAMESTRIKE, EFFECT_0))
                        if (roll_chance_i(aurEff->GetAmount()))
                        {
                            float x, y, z;
                            WorldLocation const* loc = GetExplTargetDest();
                            if (!loc)
                                return;

                            loc->GetPosition(x, y, z);
                            GetCaster()->CastSpell(x, y, z, SPELL_MAGE_FLAMESTRIKE, true);
                        }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_blast_wave_SpellScript::CountTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
                AfterCast += SpellCastFn(spell_mage_blast_wave_SpellScript::HandleImprovedFlamestrike);
            }

        private:
            uint32 _targetCount;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_blast_wave_SpellScript();
        }
};

// 82676 - Ring of Frost
class spell_mage_ring_of_frost : public SpellScriptLoader
{
    public:
        spell_mage_ring_of_frost() : SpellScriptLoader("spell_mage_ring_of_frost") { }

        class spell_mage_ring_of_frost_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_ring_of_frost_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RING_OF_FROST_SUMMON))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RING_OF_FROST_FREEZE))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RING_OF_FROST_DUMMY))
                    return false;
                return true;
            }

            void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
            {
                std::list<Creature*> MinionList;
                GetTarget()->GetAllMinionsByEntry(MinionList, GetSpellInfo()->Effects[EFFECT_0].MiscValue);
                for (std::list<Creature*>::iterator itr = MinionList.begin(); itr != MinionList.end(); itr++)
                {
                    TempSummon* ringOfFrost = (*itr)->ToTempSummon();
                    if (GetMaxDuration() - (int32)ringOfFrost->GetTimer() >= sSpellMgr->GetSpellInfo(SPELL_MAGE_RING_OF_FROST_DUMMY)->GetDuration())
                    {
                        if (!(GetTarget()->HasUnitState(UNIT_STATE_CONTROLLED)) && !GetTarget()->HasAuraType(SPELL_AURA_MOD_SILENCE) && !GetTarget()->HasAuraType(SPELL_AURA_MOD_PACIFY_SILENCE))
                            GetTarget()->CastSpell(ringOfFrost->GetPositionX(), ringOfFrost->GetPositionY(), ringOfFrost->GetPositionZ(), SPELL_MAGE_RING_OF_FROST_FREEZE, true);
                    }
                }
            }

            void Apply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                std::list<Creature*> MinionList;
                GetTarget()->GetAllMinionsByEntry(MinionList, GetSpellInfo()->Effects[EFFECT_0].MiscValue);
                TempSummon* ringOfFrost = NULL;

                // Get the last summoned RoF, save it and despawn older ones
                for (std::list<Creature*>::iterator itr = MinionList.begin(); itr != MinionList.end(); itr++)
                {
                    TempSummon* summon = (*itr)->ToTempSummon();

                    if (ringOfFrost && summon)
                    {
                        if (summon->GetTimer() > ringOfFrost->GetTimer())
                        {
                            ringOfFrost->DespawnOrUnsummon();
                            ringOfFrost = summon;
                        }
                        else
                            summon->DespawnOrUnsummon();
                    }
                    else if (summon)
                        ringOfFrost = summon;
                }
            }

            void Register() override
            {
                 OnEffectPeriodic += AuraEffectPeriodicFn(spell_mage_ring_of_frost_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
                 OnEffectApply += AuraEffectApplyFn(spell_mage_ring_of_frost_AuraScript::Apply, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mage_ring_of_frost_AuraScript();
        }
};

// 82691 - Ring of Frost (freeze effect)
class spell_mage_ring_of_frost_freeze : public SpellScriptLoader
{
    public:
        spell_mage_ring_of_frost_freeze() : SpellScriptLoader("spell_mage_ring_of_frost_freeze") { }

        class spell_mage_ring_of_frost_freeze_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_ring_of_frost_freeze_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RING_OF_FROST_SUMMON))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RING_OF_FROST_FREEZE))
                    return false;
                return true;
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                float outRadius = sSpellMgr->GetSpellInfo(SPELL_MAGE_RING_OF_FROST_SUMMON)->Effects[EFFECT_0].CalcRadius();
                float inRadius = 4.7f;

                for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end();)
                    if (Unit* unit = (*itr)->ToUnit())
                    {
                        if (unit->HasAura(SPELL_MAGE_RING_OF_FROST_DUMMY) || unit->HasAura(SPELL_MAGE_RING_OF_FROST_FREEZE) || unit->GetExactDist(GetExplTargetDest()) > outRadius || unit->GetExactDist(GetExplTargetDest()) < inRadius)
                        {
                            WorldObject* temp = (*itr);
                            itr++;
                            targets.remove(temp);
                        }
                        else
                            itr++;
                    }
                    else
                        itr++;
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_ring_of_frost_freeze_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_mage_ring_of_frost_freeze_SpellScript();
        }

        class spell_mage_ring_of_frost_freeze_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_ring_of_frost_freeze_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RING_OF_FROST_DUMMY))
                    return false;
                return true;
            }

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    GetTarget()->SetInCombatWith(caster);
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    caster->CastSpell(GetTarget(), SPELL_MAGE_RING_OF_FROST_DUMMY, true);
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectApplyFn(spell_mage_ring_of_frost_freeze_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_mage_ring_of_frost_freeze_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mage_ring_of_frost_freeze_AuraScript();
        }
};

// Pyromaniac
class spell_mage_pyromaniac : public SpellScriptLoader
{
    public:
        spell_mage_pyromaniac() : SpellScriptLoader("spell_mage_pyromaniac") { }

        class spell_mage_pyromaniac_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_pyromaniac_AuraScript);

            void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    uint32 dotCount = 0;
                    std::map<uint64, uint32> usedGuids;
                    Player::appliedAurasList const& auras = caster->ToPlayer()->appliedAuras;
                    ACE_Guard<ACE_Recursive_Thread_Mutex> g(caster->m_appliedAuraMutex);
                    for (Player::appliedAurasList::const_iterator itr = auras.begin(); itr != auras.end(); itr++)
                    {
                        Aura* aura = (*itr);
                        if (isFireDot(aura->GetId()))
                        {
                            uint64 ownerGuid = aura->GetUnitOwner()->GetGUID();
                            if (usedGuids.find(ownerGuid) == usedGuids.end())
                            {
                                dotCount++;
                                usedGuids[ownerGuid] = aura->GetId();
                            }
                        }
                    }
                    if (dotCount < 3)
                        caster->RemoveAurasDueToSpell(83582);
                }
            }

            void AfterApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (AuraEffect* pyroManiac = caster->GetDummyAuraEffect(SPELLFAMILY_MAGE, 2128, EFFECT_0))
                    {
                        uint32 dotCount = 0;
                        std::map<uint64, uint32> usedGuids;
                        Player::appliedAurasList const& auras = caster->ToPlayer()->appliedAuras;
                        ACE_Guard<ACE_Recursive_Thread_Mutex> g(caster->m_appliedAuraMutex);
                        for (Player::appliedAurasList::const_iterator itr = auras.begin(); itr != auras.end(); itr++)
                        {
                            Aura* aura = (*itr);
                            if (isFireDot(aura->GetId()))
                            {
                                uint64 ownerGuid = aura->GetUnitOwner()->GetGUID();
                                if (usedGuids.find(ownerGuid) == usedGuids.end())
                                {
                                    dotCount++;
                                    usedGuids[ownerGuid] = aura->GetId();
                                }
                            }
                        }
                        if (dotCount >= 3)
                        {
                            int32 bp0 = pyroManiac->GetAmount();
                            caster->CastCustomSpell(caster, 83582, &bp0, NULL, NULL, true);
                        }
                    }
                }
            }

            bool isFireDot(uint32 auraId)
            {
                return auraId == 44457 || auraId == 12654 || auraId == 11366 || auraId == 92315;
            }

            void Register() override
            {
                if (m_scriptSpellId == 44457 || m_scriptSpellId == 12654)
                {
                    AfterEffectApply += AuraEffectApplyFn(spell_mage_pyromaniac_AuraScript::AfterApply, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
                    AfterEffectRemove += AuraEffectRemoveFn(spell_mage_pyromaniac_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
                } else
                {
                    AfterEffectRemove += AuraEffectRemoveFn(spell_mage_pyromaniac_AuraScript::AfterRemove, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
                    AfterEffectApply += AuraEffectApplyFn(spell_mage_pyromaniac_AuraScript::AfterApply, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
                }
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mage_pyromaniac_AuraScript();
        }
};

// Frostfire Bolt
class spell_mage_ffb: public SpellScriptLoader
{
    public:
        spell_mage_ffb() : SpellScriptLoader("spell_mage_ffb") { }

        class spell_mage_ffb_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_ffb_SpellScript);

            void HandleOnHit()
            {
                if (!GetHitUnit())
                    return;

                AuraEffect* glyph = GetCaster()->GetAuraEffect(61205, EFFECT_0);
                if (Aura* debuff = GetHitAura())
                {
                    if (debuff->GetApplicationOfTarget(GetHitUnit()->GetGUID()))
                    {
                        // Modify slow based on glyph
                        if (AuraEffect* slow = debuff->GetEffect(EFFECT_0))
                        {
                            if (glyph)
                            {
                                slow->SetAmount(0);
                                slow->HandleEffect(GetHitUnit(), AURA_REMOVE_BY_DEFAULT, false);
                            }
                        }

                        // Modify dot based on glyph
                        if (AuraEffect* dot = debuff->GetEffect(EFFECT_2))
                        {
                            if (!glyph)
                            {
                                dot->SetAmount(0);
                                dot->HandleEffect(GetHitUnit(), AURA_REMOVE_BY_DEFAULT, false);
                                dot->SetPeriodic(false);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_mage_ffb_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_ffb_SpellScript();
        }
};

// Polymorph
class spell_mage_polymorph : public SpellScriptLoader
{
    public:
        spell_mage_polymorph() : SpellScriptLoader("spell_mage_polymorph") { }

        class spell_mage_polymorph_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_polymorph_AuraScript);

            void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                if (removeMode != AURA_REMOVE_BY_DAMAGE)
                    return;

                if (Unit* caster = GetCaster())
                    if (AuraEffect* aur = caster->GetDummyAuraEffect(SPELLFAMILY_MAGE, 82, EFFECT_0))
                    {
                        uint32 triggeredId = 0;
                        switch (aur->GetSpellInfo()->Id)
                        {
                            case 11210:
                                triggeredId = 83046;
                                break;
                            case 12592:
                                triggeredId = 83047;
                                break;
                        }
                        GetTarget()->CastSpell(GetTarget(), triggeredId, true);
                    }
            }

            void Register() override
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_mage_polymorph_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_MOD_CONFUSE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mage_polymorph_AuraScript();
        }
};

// 42208 - Blizzard
/// Updated 4.3.4
class spell_mage_blizzard : public SpellScriptLoader
{
   public:
       spell_mage_blizzard() : SpellScriptLoader("spell_mage_blizzard") { }

       class spell_mage_blizzard_SpellScript : public SpellScript
       {
           PrepareSpellScript(spell_mage_blizzard_SpellScript);

           bool Validate(SpellInfo const* /*spellInfo*/)
           {
               if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_CHILLED_R1))
                   return false;
               if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_CHILLED_R2))
                   return false;
               return true;
           }

           void AddChillEffect(SpellEffIndex /*effIndex*/)
           {
               Unit* caster = GetCaster();
               if (Unit* unitTarget = GetHitUnit())
               {
                   if (caster->IsScriptOverriden(GetSpellInfo(), 836))
                       caster->CastSpell(unitTarget, SPELL_MAGE_CHILLED_R1, true);
                   else if (caster->IsScriptOverriden(GetSpellInfo(), 988))
                       caster->CastSpell(unitTarget, SPELL_MAGE_CHILLED_R2, true);
               }
           }

           void Register()
           {
               OnEffectHitTarget += SpellEffectFn(spell_mage_blizzard_SpellScript::AddChillEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
           }
       };

       SpellScript* GetSpellScript() const
       {
           return new spell_mage_blizzard_SpellScript();
       }
};

// 11958 - Cold Snap
class spell_mage_cold_snap : public SpellScriptLoader
{
    public:
        spell_mage_cold_snap() : SpellScriptLoader("spell_mage_cold_snap") { }

        class spell_mage_cold_snap_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_cold_snap_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Player* caster = GetCaster()->ToPlayer();
                // immediately finishes the cooldown on Frost spells
                const SpellCooldowns& cm = caster->GetSpellCooldownMap();
                for (SpellCooldowns::const_iterator itr = cm.begin(); itr != cm.end();)
                {
                    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first))
                    {
                        if (spellInfo->SpellFamilyName == SPELLFAMILY_MAGE &&
                            ((spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_FROST) || spellInfo->Id == 92283) &&
                            spellInfo->Id != SPELL_MAGE_COLD_SNAP && spellInfo->GetRecoveryTime() > 0)
                            caster->RemoveSpellCooldown((itr++)->first, true);
                        else
                            ++itr;
                    }
                    else
                        ++itr;
                }
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_mage_cold_snap_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_cold_snap_SpellScript();
        }
};

// 120 - Cone of Cold
/// Updated 4.3.4
class spell_mage_cone_of_cold : public SpellScriptLoader
{
    public:
        spell_mage_cone_of_cold() : SpellScriptLoader("spell_mage_cone_of_cold") { }

        class spell_mage_cone_of_cold_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_cone_of_cold_SpellScript);

            void HandleConeOfColdScript(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                {
                    if (caster->HasAura(SPELL_MAGE_CONE_OF_COLD_AURA_R1)) // Improved Cone of Cold Rank 1
                        unitTarget->CastSpell(unitTarget, SPELL_MAGE_CONE_OF_COLD_TRIGGER_R1, true);
                    else if (caster->HasAura(SPELL_MAGE_CONE_OF_COLD_AURA_R2)) // Improved Cone of Cold Rank 2
                        unitTarget->CastSpell(unitTarget, SPELL_MAGE_CONE_OF_COLD_TRIGGER_R2, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_mage_cone_of_cold_SpellScript::HandleConeOfColdScript, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_cone_of_cold_SpellScript();
        }
};

// 42955 Conjure Refreshment
/// Updated 4.3.4
struct ConjureRefreshmentData
{
    uint32 minLevel;
    uint32 maxLevel;
    uint32 spellId;
};

uint8 const MAX_CONJURE_REFRESHMENT_SPELLS = 7;
ConjureRefreshmentData const _conjureData[MAX_CONJURE_REFRESHMENT_SPELLS] =
{
    { 33, 43, 92739 },
    { 44, 53, 92799 },
    { 54, 63, 92802 },
    { 64, 73, 92805 },
    { 74, 79, 74625 },
    { 80, 84, 92822 },
    { 85, 85, 92727 }
};

// 42955 - Conjure Refreshment
class spell_mage_conjure_refreshment : public SpellScriptLoader
{
    public:
        spell_mage_conjure_refreshment() : SpellScriptLoader("spell_mage_conjure_refreshment") { }

        class spell_mage_conjure_refreshment_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_conjure_refreshment_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                for (uint8 i = 0; i < MAX_CONJURE_REFRESHMENT_SPELLS; ++i)
                    if (!sSpellMgr->GetSpellInfo(_conjureData[i].spellId))
                        return false;
                return true;
            }

            bool Load()
            {
                if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                uint8 level = GetHitUnit()->getLevel();
                for (uint8 i = 0; i < MAX_CONJURE_REFRESHMENT_SPELLS; ++i)
                {
                    ConjureRefreshmentData const& spellData = _conjureData[i];
                    if (level < spellData.minLevel || level > spellData.maxLevel)
                        continue;
                    GetHitUnit()->CastSpell(GetHitUnit(), spellData.spellId);
                    break;
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_mage_conjure_refreshment_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_conjure_refreshment_SpellScript();
        }
};

// 54646 - Focus Magic
class spell_mage_focus_magic : public SpellScriptLoader
{
    public:
        spell_mage_focus_magic() : SpellScriptLoader("spell_mage_focus_magic") { }

        class spell_mage_focus_magic_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_focus_magic_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_FOCUS_MAGIC_PROC))
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
                _procTarget = GetCaster();
                return _procTarget && _procTarget->isAlive();
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
            {
                PreventDefaultAction();
                _procTarget->CastSpell(_procTarget, SPELL_MAGE_FOCUS_MAGIC_PROC, true, NULL, aurEff);
            }

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_mage_focus_magic_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_mage_focus_magic_AuraScript::HandleProc, EFFECT_1, SPELL_AURA_PROC_TRIGGER_SPELL);
            }

        private:
            Unit* _procTarget;
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_focus_magic_AuraScript();
        }
};

// 116 - Frostbolt
/// Updated 4.3.4
class spell_mage_frostbolt : public SpellScriptLoader
{
   public:
       spell_mage_frostbolt() : SpellScriptLoader("spell_mage_frostbolt") { }

       class spell_mage_frostbolt_SpellScript : public SpellScript
       {
           PrepareSpellScript(spell_mage_frostbolt_SpellScript);

           void RecalculateDamage(SpellEffIndex /*effIndex*/)
           {
               if (GetHitUnit() && GetHitUnit()->HasAuraState(AURA_STATE_FROZEN, GetSpellInfo(), GetCaster()))
               {
                   if (AuraEffect* aurEff = GetCaster()->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_MAGE, ICON_MAGE_SHATTER, EFFECT_1))
                   {
                       int32 damage = GetHitDamage();
                       AddPct(damage, aurEff->GetAmount());
                       SetHitDamage(damage);
                   }
               }
           }

           void HandleEarlyFrost()
           {
               Unit* caster = GetCaster();
               if (AuraEffect* aurEff = caster->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_MAGE, 189, 0))
               {
                   uint32 spellId = 0;
                   switch (aurEff->GetId())
                   {
                       case 83049:
                           spellId = SPELL_MAGE_EARLY_FROST_R1;
                           break;
                       case 83050:
                           spellId = SPELL_MAGE_EARLY_FROST_R2;
                           break;
                   }

                   // Check if frostbolt was affected by the modifier
                   if (Spell* spell = GetSpell())
                       if (spell->m_appliedAuras.find(spellId) != spell->m_appliedAuras.end())
                           return;

                   if (spellId && !caster->HasAura(spellId))
                   {
                       caster->CastSpell(caster, spellId, true);
                       caster->RemoveAura(94315);
                   }
               }
           }

           void Register()
           {
               AfterCast += SpellCastFn(spell_mage_frostbolt_SpellScript::HandleEarlyFrost);
               OnEffectHitTarget += SpellEffectFn(spell_mage_frostbolt_SpellScript::RecalculateDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
           }
       };

       SpellScript* GetSpellScript() const
       {
           return new spell_mage_frostbolt_SpellScript();
       }
};

// -44457 - Living Bomb
class spell_mage_living_bomb : public SpellScriptLoader
{
    public:
        spell_mage_living_bomb() : SpellScriptLoader("spell_mage_living_bomb") { }

        class spell_mage_living_bomb_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_living_bomb_AuraScript);

            bool Validate(SpellInfo const* spellInfo)
            {
                if (!sSpellMgr->GetSpellInfo(uint32(spellInfo->Effects[EFFECT_1].CalcValue())))
                    return false;
                return true;
            }

            void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                if (removeMode != AURA_REMOVE_BY_EXPIRE)
                    return;

                if (Unit* caster = GetCaster())
                    caster->CastSpell(GetTarget(), uint32(aurEff->GetAmount()), true, NULL, aurEff);
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_mage_living_bomb_AuraScript::AfterRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        class spell_mage_living_bomb_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_living_bomb_SpellScript);

            void OnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    UnitList targetList;
                    Player::appliedAurasList const& auras = caster->ToPlayer()->appliedAuras;
                    for (Player::appliedAurasList::const_iterator itr = auras.begin(); itr != auras.end(); itr++)
                    {
                        Aura* aura = (*itr);
                        if (aura->GetId() == 44457)
                            targetList.push_back(aura->GetUnitOwner());
                    }

                    if (targetList.size() > 3)
                    {
                        for (UnitList::iterator itr = targetList.begin(); itr != targetList.end(); ++itr)
                        {
                            (*itr)->RemoveAurasDueToSpell(44457, caster->GetGUID(), 0, AURA_REMOVE_BY_CANCEL);
                            break;
                        }
                    }
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_mage_living_bomb_SpellScript::OnHit);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_living_bomb_AuraScript();
        }

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_living_bomb_SpellScript();
        }
};

// 11426 - Ice Barrier
/// Updated 4.3.4
class spell_mage_ice_barrier : public SpellScriptLoader
{
   public:
       spell_mage_ice_barrier() : SpellScriptLoader("spell_mage_ice_barrier") { }

       class spell_mage_ice_barrier_AuraScript : public AuraScript
       {
           PrepareAuraScript(spell_mage_ice_barrier_AuraScript);

           void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
           {
               if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_ENEMY_SPELL)
                   return;

               if (GetTarget()->HasAura(SPELL_MAGE_SHATTERED_BARRIER_R1))
                   GetTarget()->CastSpell(GetTarget(), SPELL_MAGE_SHATTERED_BARRIER_FREEZE_R1, true);
               else if (GetTarget()->HasAura(SPELL_MAGE_SHATTERED_BARRIER_R2))
                   GetTarget()->CastSpell(GetTarget(), SPELL_MAGE_SHATTERED_BARRIER_FREEZE_R2, true);
           }

           void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
           {
               canBeRecalculated = false;
               if (Unit* caster = GetCaster())
               {
                   // +87.00% from sp bonus
                   amount += floor(0.87f * caster->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask()) + 0.5f);

                   // Glyph of Ice barrier
                   if (AuraEffect const* glyph = caster->GetDummyAuraEffect(SPELLFAMILY_MAGE, 32, EFFECT_0))
                       AddPct(amount, glyph->GetAmount());
               }
           }

           void Register() override
           {
               DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_ice_barrier_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
               AfterEffectRemove += AuraEffectRemoveFn(spell_mage_ice_barrier_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
           }
       };

       AuraScript* GetAuraScript() const override
       {
           return new spell_mage_ice_barrier_AuraScript();
       }
};

// 543 - Mage Ward
/// Updated 4.3.4
class spell_mage_mage_ward : public SpellScriptLoader
{
   public:
       spell_mage_mage_ward() : SpellScriptLoader("spell_mage_mage_ward") { }

       class spell_mage_mage_ward_AuraScript : public AuraScript
       {
           PrepareAuraScript(spell_mage_mage_ward_AuraScript);

           void HandleAbsorb(AuraEffect* /*aurEff*/, DamageInfo & /*dmgInfo*/, uint32 & absorbAmount)
           {
               if (AuraEffect* aurEff = GetTarget()->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_GENERIC, ICON_MAGE_INCANTER_S_ABSORPTION, EFFECT_0))
               {
                   int32 bp = CalculatePct(absorbAmount, aurEff->GetAmount());
                   GetTarget()->CastCustomSpell(GetTarget(), SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED, &bp, NULL, NULL, true);
               }
           }

           void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
           {
               canBeRecalculated = false;
               if (Unit* caster = GetCaster())
                   // +80.70% from sp bonus
                   amount += floor(0.807f * caster->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask()) + 0.5f);
           }

           void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
           {
               if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL)
                   // Molten Shields
                   if (GetTarget()->HasAura(11094))
                       GetTarget()->CastSpell(GetTarget(), 31643, true);
           }

           void Register() override
           {
               DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_mage_ward_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
               AfterEffectAbsorb += AuraEffectAbsorbFn(spell_mage_mage_ward_AuraScript::HandleAbsorb, EFFECT_0);
               AfterEffectRemove += AuraEffectRemoveFn(spell_mage_mage_ward_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
           }
       };

       AuraScript* GetAuraScript() const override
       {
           return new spell_mage_mage_ward_AuraScript();
       }
};

// 1463 - Mana Shield
/// Updated 4.3.4
class spell_mage_mana_shield : public SpellScriptLoader
{
    public:
       spell_mage_mana_shield() : SpellScriptLoader("spell_mage_mana_shield") { }

       class spell_mage_mana_shield_AuraScript : public AuraScript
       {
           PrepareAuraScript(spell_mage_mana_shield_AuraScript);

           void HandleAbsorb(AuraEffect* /*aurEff*/, DamageInfo & /*dmgInfo*/, uint32 & absorbAmount)
           {
               if (AuraEffect* aurEff = GetTarget()->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_GENERIC, ICON_MAGE_INCANTER_S_ABSORPTION, EFFECT_0))
               {
                   int32 bp = CalculatePct(absorbAmount, aurEff->GetAmount());
                   GetTarget()->CastCustomSpell(GetTarget(), SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED, &bp, NULL, NULL, true);
               }
           }

           void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
           {
               if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL)
                   if (GetTarget()->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_GENERIC, ICON_MAGE_INCANTER_S_ABSORPTION, EFFECT_0))
                       GetTarget()->CastSpell(GetTarget(), 86261, true);
           }

           void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
           {
               canBeRecalculated = false;
               if (Unit* caster = GetCaster())
                   // +80.7% from sp bonus
                   amount += floor(0.807f * caster->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask()) + 0.5f);
           }

           void Register() override
           {
               DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_mana_shield_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MANA_SHIELD);
               AfterEffectManaShield += AuraEffectManaShieldFn(spell_mage_mana_shield_AuraScript::HandleAbsorb, EFFECT_0);
               AfterEffectRemove += AuraEffectRemoveFn(spell_mage_mana_shield_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_MANA_SHIELD, AURA_EFFECT_HANDLE_REAL);
           }
       };

       AuraScript* GetAuraScript() const override
       {
           return new spell_mage_mana_shield_AuraScript();
       }
};

enum SilvermoonPolymorph
{
    NPC_AUROSALIA       = 18744
};

// TODO: move out of here and rename - not a mage spell
class spell_mage_polymorph_cast_visual : public SpellScriptLoader
{
    public:
        spell_mage_polymorph_cast_visual() : SpellScriptLoader("spell_mage_polymorph_visual") { }

        class spell_mage_polymorph_cast_visual_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_polymorph_cast_visual_SpellScript);

            static const uint32 PolymorhForms[6];

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                // check if spell ids exist in dbc
                for (uint32 i = 0; i < 6; i++)
                    if (!sSpellMgr->GetSpellInfo(PolymorhForms[i]))
                        return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetCaster()->FindNearestCreature(NPC_AUROSALIA, 30.0f))
                    if (target->GetTypeId() == TYPEID_UNIT)
                        target->CastSpell(target, PolymorhForms[urand(0, 5)], true);
            }

            void Register()
            {
                // add dummy effect spell handler to Polymorph visual
                OnEffectHitTarget += SpellEffectFn(spell_mage_polymorph_cast_visual_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_polymorph_cast_visual_SpellScript();
        }
};

uint32 const spell_mage_polymorph_cast_visual::spell_mage_polymorph_cast_visual_SpellScript::PolymorhForms[6] =
{
    SPELL_MAGE_SQUIRREL_FORM,
    SPELL_MAGE_GIRAFFE_FORM,
    SPELL_MAGE_SERPENT_FORM,
    SPELL_MAGE_DRAGONHAWK_FORM,
    SPELL_MAGE_WORGEN_FORM,
    SPELL_MAGE_SHEEP_FORM
};

// 5405  - Replenish Mana (Mana Gem)
/// Updated 4.3.4
class spell_mage_replenish_mana : public SpellScriptLoader
{
   public:
       spell_mage_replenish_mana() : SpellScriptLoader("spell_mage_replenish_mana") { }

       class spell_mage_replenish_mana_SpellScript : public SpellScript
       {
           PrepareSpellScript(spell_mage_replenish_mana_SpellScript);

           bool Validate(SpellInfo const* /*spellInfo*/)
           {
               if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_IMPROVED_MANA_GEM_TRIGGERED))
                   return false;
               return true;
           }

           void HandleImprovedManaGem()
           {
               if (AuraEffect* aurEff = GetCaster()->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_MAGE, ICON_MAGE_IMPROVED_MANA_GEM, EFFECT_0))
               {
                   int32 bp = CalculatePct(GetCaster()->GetMaxPower(POWER_MANA), aurEff->GetAmount());
                   GetCaster()->CastCustomSpell(GetCaster(), SPELL_MAGE_IMPROVED_MANA_GEM_TRIGGERED, &bp, &bp, NULL, true);
               }
           }

           void Register()
           {
               AfterCast += SpellCastFn(spell_mage_replenish_mana_SpellScript::HandleImprovedManaGem);
           }
       };

       SpellScript* GetSpellScript() const
       {
           return new spell_mage_replenish_mana_SpellScript();
       }
};

// 33395 Water Elemental's Freeze
/// Updated 4.3.4
class spell_mage_water_elemental_freeze : public SpellScriptLoader
{
   public:
       spell_mage_water_elemental_freeze() : SpellScriptLoader("spell_mage_water_elemental_freeze") { }

       class spell_mage_water_elemental_freeze_SpellScript : public SpellScript
       {
           PrepareSpellScript(spell_mage_water_elemental_freeze_SpellScript);

           bool Validate(SpellInfo const* /*spellInfo*/)
           {
               if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_FINGERS_OF_FROST))
                   return false;
               return true;
           }

           bool Load()
           {
               _casted = false;
               return true;
           }

           void HandleImprovedFreeze(SpellEffIndex /*effIndex*/)
           {
               Unit* owner = GetCaster()->GetOwner();
               if (!owner || _casted)
                   return;

               if (AuraEffect* aurEff = owner->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_MAGE, ICON_MAGE_IMPROVED_FREEZE, EFFECT_0))
               {
                   if (roll_chance_i(aurEff->GetAmount()))
                   {
                       _casted = true;
                       owner->CastCustomSpell(SPELL_MAGE_FINGERS_OF_FROST, SPELLVALUE_AURA_STACK, 2, owner, true);
                   }
               }
           }

           void Register()
           {
               OnEffectHitTarget += SpellEffectFn(spell_mage_water_elemental_freeze_SpellScript::HandleImprovedFreeze, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
           }

       private:
           bool _casted;
       };

       SpellScript* GetSpellScript() const
       {
           return new spell_mage_water_elemental_freeze_SpellScript();
       }
};

//  83154 - Piercing Chill
class spell_mage_piercing_chill : public SpellScriptLoader
{
    public:
        spell_mage_piercing_chill() : SpellScriptLoader("spell_mage_piercing_chill") { }

        class spell_mage_piercing_chill_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_piercing_chill_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (_targets.size())
                {
                    targets = _targets;
                    return;
                }

                if (AuraEffect* aurEff = GetCaster()->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_MAGE, 4625, EFFECT_0))
                {
                    targets.remove(GetExplTargetUnit());
                    Trinity::Containers::RandomResizeList(targets, aurEff->GetAmount());
                    targets.push_back(GetExplTargetUnit());
                    _targets = targets;
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_piercing_chill_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_piercing_chill_SpellScript::FilterTargets, EFFECT_2, TARGET_UNIT_DEST_AREA_ENEMY);
            }

        private:
           std::list<WorldObject*> _targets;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_piercing_chill_SpellScript();
        }
};

class spell_mage_cauterize : public SpellScriptLoader
{
    public:
        spell_mage_cauterize() : SpellScriptLoader("spell_mage_cauterize") { }

        class spell_mage_cauterize_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_cauterize_AuraScript);

            uint32 absorbChance, healPct;

            enum Spell
            {
                MAGE_SPELL_CAUTERIZE_HEAL = 87023,
            };

            bool Load() override
            {
                healPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue();
                absorbChance = GetSpellInfo()->Effects[EFFECT_0].CalcValue();
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
                if (remainingHealth <= 0 && !victim->ToPlayer()->HasSpellCooldown(MAGE_SPELL_CAUTERIZE_HEAL))
                {
                    if (roll_chance_i(absorbChance))
                    {
                        // Cast healing spell, completely avoid damage
                        absorbAmount = dmgInfo.GetDamage();
                        int32 healAmount = int32(victim->CountPctFromMaxHealth(healPct));
                        healAmount -= victim->GetHealth();
                        victim->CastCustomSpell(victim, MAGE_SPELL_CAUTERIZE_HEAL, NULL, &healAmount, NULL, true, NULL, aurEff);
                        victim->ToPlayer()->AddSpellCooldown(MAGE_SPELL_CAUTERIZE_HEAL, 0, time(NULL) + 60);
                    }
                }
            }

            void Register() override
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_cauterize_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_mage_cauterize_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mage_cauterize_AuraScript();
        }
};

class spell_mage_combustion : public SpellScriptLoader
{
    public:
        spell_mage_combustion() : SpellScriptLoader("spell_mage_combustion") { }

        class spell_mage_combustion_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_combustion_SpellScript);

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                int32 damageSum = 0;
                if (Unit* unitTarget = GetHitUnit())
                {
                    Unit::AuraEffectList const & aurasA = unitTarget->GetAuraEffectsByType(SPELL_AURA_PERIODIC_DAMAGE);
                    for (Unit::AuraEffectList::const_iterator itr = aurasA.begin(); itr != aurasA.end(); ++itr)
                    {
                        if (((*itr)->GetCasterGUID() != caster->GetGUID())
                            || (*itr)->GetSpellInfo()->SpellFamilyName != GetSpellInfo()->SpellFamilyName)
                            continue;

                        // Info taken from Elitist jerks
                        if ((*itr)->GetId() == 44457 || (*itr)->GetId() == 11366)
                            damageSum += (*itr)->CalculateAmount(caster) / 3;
                        else
                            damageSum += (*itr)->GetAmount() / 2;
                    }
                    if (damageSum)
                        caster->CastCustomSpell(unitTarget, 83853, &damageSum, NULL, NULL, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_mage_combustion_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_combustion_SpellScript();
        }
};

class spell_mage_impact : public SpellScriptLoader
{
    public:
        spell_mage_impact() : SpellScriptLoader("spell_mage_impact") { }

        class spell_mage_impact_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_impact_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (GetExplTargetUnit())
                    targets.remove(GetExplTargetUnit());
            }

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetExplTargetUnit())
                {
                    Unit::AuraEffectList const & aurasA = unitTarget->GetAuraEffectsByType(SPELL_AURA_PERIODIC_DAMAGE);
                    for (Unit::AuraEffectList::const_iterator itr = aurasA.begin(); itr != aurasA.end(); ++itr)
                    {
                        if (((*itr)->GetCasterGUID() != caster->GetGUID())
                            || (*itr)->GetSpellInfo()->SpellFamilyName != SPELLFAMILY_MAGE)
                            continue;

                        bool skipEffect = false;
                        if ((*itr)->GetSpellInfo()->Id == 44457)
                        {
                            uint8 targetCount = 0;
                            Player::appliedAurasList const& auras = caster->ToPlayer()->appliedAuras;
                            for (Player::appliedAurasList::const_iterator itr = auras.begin(); itr != auras.end(); itr++)
                            {
                                Aura* aura = (*itr);
                                if (aura->GetId() == 44457)
                                    targetCount++;
                            }

                            if (targetCount >= 3)
                                skipEffect = true;
                        }

                        if (!skipEffect)
                            caster->AddAuraForTarget((*itr)->GetBase(), GetHitUnit());
                    }
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_impact_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_mage_impact_SpellScript::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_impact_SpellScript();
        }
};

// Arcane potency
class spell_mage_arcane_potency : public SpellScriptLoader
{
    public:
        spell_mage_arcane_potency() : SpellScriptLoader("spell_mage_arcane_potency") { }

        class spell_mage_arcane_potency_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_arcane_potency_SpellScript);

            void HandleOnHit()
            {
                if (AuraEffect* aPotency = GetCaster()->GetDummyAuraEffect(SPELLFAMILY_MAGE, 2120, EFFECT_0))
                {
                    uint32 triggerId = 0;
                    switch (aPotency->GetId())
                    {
                        case 31571:
                            triggerId = 57529;
                            break;
                        case 31572:
                            triggerId = 57531;
                            break;
                    }
                    if (triggerId)
                        GetCaster()->CastSpell(GetCaster(), triggerId, true);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_mage_arcane_potency_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_arcane_potency_SpellScript();
        }
};

// 43987 Ritual of Refreshment
/// Updated 4.3.4
class spell_mage_ritual_of_refreshment : public SpellScriptLoader
{
    public:
        spell_mage_ritual_of_refreshment() : SpellScriptLoader("spell_mage_ritual_of_refreshment") { }

        class spell_mage_ritual_of_refreshment_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_ritual_of_refreshment_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RITUAL_OF_REFRESHMENT_R1))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RITUAL_OF_REFRESHMENT_R2))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_RITUAL_OF_REFRESHMENT_R3))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                    if (caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        caster->ToPlayer()->RemoveSpellCooldown(SPELL_MAGE_RITUAL_OF_REFRESHMENT_R1, true); // Rank 1
                        caster->ToPlayer()->RemoveSpellCooldown(SPELL_MAGE_RITUAL_OF_REFRESHMENT_R2, true); // Rank 2
                        caster->ToPlayer()->RemoveSpellCooldown(SPELL_MAGE_RITUAL_OF_REFRESHMENT_R3, true); // Rank 3

                        if (caster->getLevel() >= 85)
                            caster->CastSpell(caster, SPELL_MAGE_RITUAL_OF_REFRESHMENT_R3, true);
                        else if (caster->getLevel() > 80)
                            caster->CastSpell(caster, SPELL_MAGE_RITUAL_OF_REFRESHMENT_R2, true);
                        else if (caster->getLevel() > 75)
                            caster->CastSpell(caster, SPELL_MAGE_RITUAL_OF_REFRESHMENT_R1, true);
                    }
            }

            void Register()
            {
                OnEffectLaunch += SpellEffectFn(spell_mage_ritual_of_refreshment_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_ritual_of_refreshment_SpellScript();
        }
};

// 12472 - Icy Veins
class spell_mage_icy_veins : public SpellScriptLoader
{
public:
    spell_mage_icy_veins() : SpellScriptLoader("spell_mage_icy_veins") { }

    class spell_mage_icy_veins_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mage_icy_veins_AuraScript);

        void Apply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (caster->HasAura(SPELL_MAGE_GLYPH_OF_ICY_VEINS))
                {
                    caster->RemoveAurasByType(SPELL_AURA_HASTE_SPELLS, 0, 0, true, false);
                    caster->RemoveAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);
                }
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_mage_icy_veins_AuraScript::Apply, EFFECT_0, SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_mage_icy_veins_AuraScript();
    }
};

class spell_mage_flamestrike : public SpellScriptLoader
{
public:
    spell_mage_flamestrike() : SpellScriptLoader("spell_mage_flamestrike") { }

    class spell_mage_flamestrike_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mage_flamestrike_AuraScript);

        void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
        {
            canBeRecalculated = false;
            if (Unit* caster = GetCaster())
                amount += floor(0.061f * caster->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask()));
        }

        void Register() override
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_flamestrike_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    class spell_mage_flamestrike_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mage_flamestrike_SpellScript);

        void RemoveDynObject(SpellEffIndex /*effIndex*/)
        {
            GetCaster()->RemoveDynObject(GetSpellInfo()->Id);
        }

        void Register()
        {
            OnEffectLaunch += SpellEffectFn(spell_mage_flamestrike_SpellScript::RemoveDynObject, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_mage_flamestrike_SpellScript();
    }

    AuraScript* GetAuraScript() const override
    {
        return new spell_mage_flamestrike_AuraScript();
    }
};

class spell_mage_invisibility : public SpellScriptLoader
{
public:
    spell_mage_invisibility() : SpellScriptLoader("spell_mage_invisibility") { }

    class spell_mage_invisibility_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mage_invisibility_AuraScript);

        void ApplyEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
                if (caster->HasAura(56366)) // Glyph of invisibility
                    caster->CastSpell(caster, 87833, true);
        }

        void RemoveEffect(AuraEffect const* /* aurEff */, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
                caster->RemoveAurasDueToSpell(87833);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_mage_invisibility_AuraScript::RemoveEffect, EFFECT_1, SPELL_AURA_MOD_INVISIBILITY, AURA_EFFECT_HANDLE_REAL);
            OnEffectApply += AuraEffectApplyFn(spell_mage_invisibility_AuraScript::ApplyEffect, EFFECT_1, SPELL_AURA_MOD_INVISIBILITY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_mage_invisibility_AuraScript();
    }
};

class spell_mage_deep_freeze : public SpellScriptLoader
{
public:
    spell_mage_deep_freeze() : SpellScriptLoader("spell_mage_deep_freeze") { }

    class spell_mage_deep_freeze_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mage_deep_freeze_SpellScript);

        void HandleOnHit()
        {
            if (Unit* target = GetHitUnit())
            {
                if (Unit* caster = GetCaster())
                {
                    if (target->GetTypeId() == TYPEID_UNIT && target->IsImmunedToSpellEffect(sSpellMgr->GetSpellInfo(44572), 0))
                        caster->CastSpell(target, 71757, true);
                }
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_mage_deep_freeze_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_mage_deep_freeze_SpellScript();
    }
};

class spell_mage_pyroblast : public SpellScriptLoader
{
public:
    spell_mage_pyroblast() : SpellScriptLoader("spell_mage_pyroblast") { }

    class spell_mage_pyroblast_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mage_pyroblast_AuraScript);

        void Apply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (Unit* target = GetTarget())
                {
                    switch (GetAura()->GetSpellInfo()->Id)
                    {
                        case 11366: // Pyroblast(Level 85)
                            if (target->HasAura(92315, caster->GetGUID()))
                                target->RemoveAurasDueToSpell(92315, caster->GetGUID());
                            break;
                        case 92315: // Pyroblast!(Level 85)
                            if (target->HasAura(11366, caster->GetGUID()))
                                target->RemoveAurasDueToSpell(11366, caster->GetGUID());
                            break;
                    }
                }
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_mage_pyroblast_AuraScript::Apply, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_mage_pyroblast_AuraScript();
    }
};

class spell_mage_fingers_of_frost : public SpellScriptLoader
{
public:
    spell_mage_fingers_of_frost() : SpellScriptLoader("spell_mage_fingers_of_frost") { }

    class spell_mage_fingers_of_frost_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mage_fingers_of_frost_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            return !(eventInfo.GetHitMask() & (PROC_EX_IMMUNE | PROC_EX_MISS));
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(spell_mage_fingers_of_frost_AuraScript::CheckProc);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_mage_fingers_of_frost_AuraScript();
    }
};

class spell_mage_t13_2p : public SpellScriptLoader
{
public:
    spell_mage_t13_2p() : SpellScriptLoader("spell_mage_t13_2p") { }

    class spell_mage_t13_2p_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mage_t13_2p_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            if (!eventInfo.GetSpellInfo())
                return false;

            // Arcane Blast
            if (eventInfo.GetSpellInfo()->Id == 30451)
                return true;

            // Your Arcane Blast has a 100% chance and your Fireball, Pyroblast, Frostfire Bolt, and Frostbolt spells have a 50% chance to grant Stolen Time
            if (eventInfo.GetSpellInfo()->Id == 133 || eventInfo.GetSpellInfo()->Id == 11366 || eventInfo.GetSpellInfo()->Id == 44614 || eventInfo.GetSpellInfo()->Id == 116)
                if (roll_chance_i(50))
                    return true;

            return false;
        }

        void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            // Mage T13 4P Bonus (Arcane Power, Combustion, and Icy Veins)
            if (GetTarget()->HasAura(105790, GetTarget()->GetGUID()))
                GetTarget()->CastSpell(GetTarget(), SPELL_MAGE_STOLEN_TIME_4P, true);
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                caster->RemoveAurasDueToSpell(SPELL_MAGE_STOLEN_TIME);
                caster->RemoveAurasDueToSpell(SPELL_MAGE_STOLEN_TIME_4P);
            }
        }

        void Register()
        {
            if (m_scriptSpellId == 105788)
            {
                DoCheckProc += AuraCheckProcFn(spell_mage_t13_2p_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_mage_t13_2p_AuraScript::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
            }

            if (m_scriptSpellId == 12042)
                AfterEffectRemove += AuraEffectRemoveFn(spell_mage_t13_2p_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);

            if (m_scriptSpellId == 12472)
                AfterEffectRemove += AuraEffectRemoveFn(spell_mage_t13_2p_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK, AURA_EFFECT_HANDLE_REAL);

            if (m_scriptSpellId == 83853)
                AfterEffectRemove += AuraEffectRemoveFn(spell_mage_t13_2p_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_mage_t13_2p_AuraScript();
    }
};

void AddSC_mage_spell_scripts()
{
    new spell_mage_blast_wave(); // Verified
    new spell_mage_blizzard(); // Verified
    new spell_mage_cold_snap(); // Verified
    new spell_mage_cone_of_cold(); // Verified
    new spell_mage_conjure_refreshment(); // Verified
    new spell_mage_focus_magic(); // Verified
    new spell_mage_frostbolt(); // Verified
    new spell_mage_ice_barrier(); // Verified
    new spell_mage_living_bomb(); // Verified
    new spell_mage_mage_ward(); // Verified
    new spell_mage_mana_shield(); // Verified
    new spell_mage_polymorph_cast_visual(); // Recently Verified
	new spell_mage_replenish_mana(); // Verified
	new spell_mage_water_elemental_freeze(); // Verified
	new spell_mage_piercing_chill(); // Verified
	new spell_mage_cauterize(); // Verified
	new spell_mage_ring_of_frost(); // Verified
	new spell_mage_ring_of_frost_freeze(); // Verified
	new spell_mage_polymorph(); // Verified
	new spell_mage_arcane_potency(); // Verified
	new spell_mage_combustion(); // Verified
	new spell_mage_pyromaniac(); // Verified
	new spell_mage_impact(); // Verified
	new spell_mage_ffb(); // Verified
	new spell_mage_ritual_of_refreshment(); // Verified
	new spell_mage_icy_veins(); // Verified
    new spell_mage_flamestrike(); // Recently Verifed
    new spell_mage_invisibility(); // Verified
    new spell_mage_deep_freeze(); // Verified
    new spell_mage_pyroblast(); // Recently Verified
    new spell_mage_fingers_of_frost(); // Recently Verified
    new spell_mage_t13_2p(); // Recently Verified
}
