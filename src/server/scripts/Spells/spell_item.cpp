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
 * Scripts for spells with SPELLFAMILY_GENERIC spells used by items.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_item_".
 */

#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SkillDiscovery.h"
#include "Battleground.h"

// Generic script for handling item dummy effects which trigger another spell.
class spell_item_trigger_spell : public SpellScriptLoader
{
    private:
        uint32 _triggeredSpellId;

    public:
        spell_item_trigger_spell(const char* name, uint32 triggeredSpellId) : SpellScriptLoader(name), _triggeredSpellId(triggeredSpellId) { }

        class spell_item_trigger_spell_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_trigger_spell_SpellScript);
        private:
            uint32 _triggeredSpellId;

        public:
            spell_item_trigger_spell_SpellScript(uint32 triggeredSpellId) : SpellScript(), _triggeredSpellId(triggeredSpellId) { }

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(_triggeredSpellId))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Item* item = GetCastItem())
                    caster->CastSpell(caster, _triggeredSpellId, true, item);
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_item_trigger_spell_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_trigger_spell_SpellScript(_triggeredSpellId);
        }
};

// 26400 - Arcane Shroud
class spell_item_arcane_shroud : public SpellScriptLoader
{
    public:
        spell_item_arcane_shroud() : SpellScriptLoader("spell_item_arcane_shroud") { }

        class spell_item_arcane_shroud_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_item_arcane_shroud_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                int32 diff = GetUnitOwner()->getLevel() - 60;
                if (diff > 0)
                    amount += 2 * diff;
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_item_arcane_shroud_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_THREAT);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_item_arcane_shroud_AuraScript();
        }
};

// 64411 - Blessing of Ancient Kings (Val'anyr, Hammer of Ancient Kings)
enum BlessingOfAncientKings
{
    SPELL_PROTECTION_OF_ANCIENT_KINGS   = 64413
};

class spell_item_blessing_of_ancient_kings : public SpellScriptLoader
{
    public:
        spell_item_blessing_of_ancient_kings() : SpellScriptLoader("spell_item_blessing_of_ancient_kings") { }

        class spell_item_blessing_of_ancient_kings_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_item_blessing_of_ancient_kings_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PROTECTION_OF_ANCIENT_KINGS))
                    return false;
                return true;
            }

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                return eventInfo.GetProcTarget();
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                int32 absorb = int32(CalculatePct(eventInfo.GetHealInfo()->GetHeal(), 15.0f));
                if (AuraEffect* protEff = eventInfo.GetProcTarget()->GetAuraEffect(SPELL_PROTECTION_OF_ANCIENT_KINGS, 0, eventInfo.GetActor()->GetGUID()))
                {
                    // The shield can grow to a maximum size of 20,000 damage absorbtion
                    protEff->SetAmount(std::min<int32>(protEff->GetAmount() + absorb, 20000));

                    // Refresh and return to prevent replacing the aura
                    protEff->GetBase()->RefreshDuration();
                }
                else
                    GetTarget()->CastCustomSpell(SPELL_PROTECTION_OF_ANCIENT_KINGS, SPELLVALUE_BASE_POINT0, absorb, eventInfo.GetProcTarget(), true, NULL, aurEff);
            }

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_item_blessing_of_ancient_kings_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_item_blessing_of_ancient_kings_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_item_blessing_of_ancient_kings_AuraScript();
        }
};

// 8342  - Defibrillate (Goblin Jumper Cables) have 33% chance on success
// 22999 - Defibrillate (Goblin Jumper Cables XL) have 50% chance on success
// 54732 - Defibrillate (Gnomish Army Knife) have 67% chance on success
enum Defibrillate
{
    SPELL_GOBLIN_JUMPER_CABLES_FAIL     = 8338,
    SPELL_GOBLIN_JUMPER_CABLES_XL_FAIL  = 23055
};

class spell_item_defibrillate : public SpellScriptLoader
{
    public:
        spell_item_defibrillate(char const* name, uint8 chance, uint32 failSpell = 0) : SpellScriptLoader(name), _chance(chance), _failSpell(failSpell) { }

        class spell_item_defibrillate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_defibrillate_SpellScript);

        public:
            spell_item_defibrillate_SpellScript(uint8 chance, uint32 failSpell) : SpellScript(), _chance(chance), _failSpell(failSpell) { }

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (_failSpell && !sSpellMgr->GetSpellInfo(_failSpell))
                    return false;
                return true;
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                if (roll_chance_i(_chance))
                {
                    PreventHitDefaultEffect(effIndex);
                    if (_failSpell)
                        GetCaster()->CastSpell(GetCaster(), _failSpell, true, GetCastItem());
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_defibrillate_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_RESURRECT);
            }

        private:
            uint8 _chance;
            uint32 _failSpell;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_defibrillate_SpellScript(_chance, _failSpell);
        }

    private:
        uint8 _chance;
        uint32 _failSpell;
};

// http://www.wowhead.com/item=6522 Deviate Fish
// 8063 Deviate Fish
enum DeviateFishSpells
{
    SPELL_SLEEPY            = 8064,
    SPELL_INVIGORATE        = 8065,
    SPELL_SHRINK            = 8066,
    SPELL_PARTY_TIME        = 8067,
    SPELL_HEALTHY_SPIRIT    = 8068,
};

class spell_item_deviate_fish : public SpellScriptLoader
{
    public:
        spell_item_deviate_fish() : SpellScriptLoader("spell_item_deviate_fish") { }

        class spell_item_deviate_fish_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_deviate_fish_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                for (uint32 spellId = SPELL_SLEEPY; spellId <= SPELL_HEALTHY_SPIRIT; ++spellId)
                    if (!sSpellMgr->GetSpellInfo(spellId))
                        return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                uint32 spellId = urand(SPELL_SLEEPY, SPELL_HEALTHY_SPIRIT);
                caster->CastSpell(caster, spellId, true, NULL);
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_item_deviate_fish_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_deviate_fish_SpellScript();
        }
};

// http://www.wowhead.com/item=47499 Flask of the North
// 67019 Flask of the North
enum FlaskOfTheNorthSpells
{
    SPELL_FLASK_OF_THE_NORTH_SP = 67016,
    SPELL_FLASK_OF_THE_NORTH_AP = 67017,
    SPELL_FLASK_OF_THE_NORTH_STR = 67018,
};

class spell_item_flask_of_the_north : public SpellScriptLoader
{
    public:
        spell_item_flask_of_the_north() : SpellScriptLoader("spell_item_flask_of_the_north") { }

        class spell_item_flask_of_the_north_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_flask_of_the_north_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_FLASK_OF_THE_NORTH_SP) || !sSpellMgr->GetSpellInfo(SPELL_FLASK_OF_THE_NORTH_AP) || !sSpellMgr->GetSpellInfo(SPELL_FLASK_OF_THE_NORTH_STR))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                std::vector<uint32> possibleSpells;
                switch (caster->getClass())
                {
                    case CLASS_WARLOCK:
                    case CLASS_MAGE:
                    case CLASS_PRIEST:
                        possibleSpells.push_back(SPELL_FLASK_OF_THE_NORTH_SP);
                        break;
                    case CLASS_DEATH_KNIGHT:
                    case CLASS_WARRIOR:
                        possibleSpells.push_back(SPELL_FLASK_OF_THE_NORTH_STR);
                        break;
                    case CLASS_ROGUE:
                    case CLASS_HUNTER:
                        possibleSpells.push_back(SPELL_FLASK_OF_THE_NORTH_AP);
                        break;
                    case CLASS_DRUID:
                    case CLASS_PALADIN:
                        possibleSpells.push_back(SPELL_FLASK_OF_THE_NORTH_SP);
                        possibleSpells.push_back(SPELL_FLASK_OF_THE_NORTH_STR);
                        break;
                    case CLASS_SHAMAN:
                        possibleSpells.push_back(SPELL_FLASK_OF_THE_NORTH_SP);
                        possibleSpells.push_back(SPELL_FLASK_OF_THE_NORTH_AP);
                        break;
                }

                caster->CastSpell(caster, possibleSpells[irand(0, (possibleSpells.size() - 1))], true, NULL);
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_item_flask_of_the_north_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_flask_of_the_north_SpellScript();
        }
};

// http://www.wowhead.com/item=10645 Gnomish Death Ray
// 13280 Gnomish Death Ray
enum GnomishDeathRay
{
    SPELL_GNOMISH_DEATH_RAY_SELF = 13493,
    SPELL_GNOMISH_DEATH_RAY_TARGET = 13279,
};

class spell_item_gnomish_death_ray : public SpellScriptLoader
{
    public:
        spell_item_gnomish_death_ray() : SpellScriptLoader("spell_item_gnomish_death_ray") { }

        class spell_item_gnomish_death_ray_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_gnomish_death_ray_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_GNOMISH_DEATH_RAY_SELF) || !sSpellMgr->GetSpellInfo(SPELL_GNOMISH_DEATH_RAY_TARGET))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetHitUnit())
                {
                    if (urand(0, 99) < 15)
                        caster->CastSpell(caster, SPELL_GNOMISH_DEATH_RAY_SELF, true, NULL);    // failure
                    else
                        caster->CastSpell(target, SPELL_GNOMISH_DEATH_RAY_TARGET, true, NULL);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_gnomish_death_ray_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_gnomish_death_ray_SpellScript();
        }
};

// http://www.wowhead.com/item=27388 Mr. Pinchy
// 33060 Make a Wish
enum MakeAWish
{
    SPELL_MR_PINCHYS_BLESSING       = 33053,
    SPELL_SUMMON_MIGHTY_MR_PINCHY   = 33057,
    SPELL_SUMMON_FURIOUS_MR_PINCHY  = 33059,
    SPELL_TINY_MAGICAL_CRAWDAD      = 33062,
    SPELL_MR_PINCHYS_GIFT           = 33064,
};

class spell_item_make_a_wish : public SpellScriptLoader
{
    public:
        spell_item_make_a_wish() : SpellScriptLoader("spell_item_make_a_wish") { }

        class spell_item_make_a_wish_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_make_a_wish_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MR_PINCHYS_BLESSING) || !sSpellMgr->GetSpellInfo(SPELL_SUMMON_MIGHTY_MR_PINCHY) || !sSpellMgr->GetSpellInfo(SPELL_SUMMON_FURIOUS_MR_PINCHY) || !sSpellMgr->GetSpellInfo(SPELL_TINY_MAGICAL_CRAWDAD) || !sSpellMgr->GetSpellInfo(SPELL_MR_PINCHYS_GIFT))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                uint32 spellId = SPELL_MR_PINCHYS_GIFT;
                switch (urand(1, 5))
                {
                    case 1: spellId = SPELL_MR_PINCHYS_BLESSING; break;
                    case 2: spellId = SPELL_SUMMON_MIGHTY_MR_PINCHY; break;
                    case 3: spellId = SPELL_SUMMON_FURIOUS_MR_PINCHY; break;
                    case 4: spellId = SPELL_TINY_MAGICAL_CRAWDAD; break;
                }
                caster->CastSpell(caster, spellId, true, NULL);
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_item_make_a_wish_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_make_a_wish_SpellScript();
        }
};

// http://www.wowhead.com/item=32686 Mingo's Fortune Giblets
// 40802 Mingo's Fortune Generator
class spell_item_mingos_fortune_generator : public SpellScriptLoader
{
    public:
        spell_item_mingos_fortune_generator() : SpellScriptLoader("spell_item_mingos_fortune_generator") { }

        class spell_item_mingos_fortune_generator_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_mingos_fortune_generator_SpellScript);

            void HandleDummy(SpellEffIndex effIndex)
            {
                // Selecting one from Bloodstained Fortune item
                uint32 newitemid;
                switch (urand(1, 20))
                {
                    case 1:  newitemid = 32688; break;
                    case 2:  newitemid = 32689; break;
                    case 3:  newitemid = 32690; break;
                    case 4:  newitemid = 32691; break;
                    case 5:  newitemid = 32692; break;
                    case 6:  newitemid = 32693; break;
                    case 7:  newitemid = 32700; break;
                    case 8:  newitemid = 32701; break;
                    case 9:  newitemid = 32702; break;
                    case 10: newitemid = 32703; break;
                    case 11: newitemid = 32704; break;
                    case 12: newitemid = 32705; break;
                    case 13: newitemid = 32706; break;
                    case 14: newitemid = 32707; break;
                    case 15: newitemid = 32708; break;
                    case 16: newitemid = 32709; break;
                    case 17: newitemid = 32710; break;
                    case 18: newitemid = 32711; break;
                    case 19: newitemid = 32712; break;
                    case 20: newitemid = 32713; break;
                    default:
                        return;
                }

                CreateItem(effIndex, newitemid);
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_item_mingos_fortune_generator_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_mingos_fortune_generator_SpellScript();
        }
};

// 71875, 71877 - Item - Black Bruise: Necrotic Touch Proc
enum NecroticTouch
{
    SPELL_ITEM_NECROTIC_TOUCH_PROC  = 71879
};

class spell_item_necrotic_touch : public SpellScriptLoader
{
    public:
        spell_item_necrotic_touch() : SpellScriptLoader("spell_item_necrotic_touch") { }

        class spell_item_necrotic_touch_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_item_necrotic_touch_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_ITEM_NECROTIC_TOUCH_PROC))
                    return false;
                return true;
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                int32 bp = CalculatePct(int32(eventInfo.GetDamageInfo()->GetDamage()), aurEff->GetAmount());
                GetTarget()->CastCustomSpell(SPELL_ITEM_NECROTIC_TOUCH_PROC, SPELLVALUE_BASE_POINT0, bp, GetTarget(), true, NULL, aurEff);
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_item_necrotic_touch_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_item_necrotic_touch_AuraScript();
        }
};

// http://www.wowhead.com/item=10720 Gnomish Net-o-Matic Projector
// 13120 Net-o-Matic
enum NetOMaticSpells
{
    SPELL_NET_O_MATIC_TRIGGERED1 = 16566,
    SPELL_NET_O_MATIC_TRIGGERED2 = 13119,
    SPELL_NET_O_MATIC_TRIGGERED3 = 13099,
};

class spell_item_net_o_matic : public SpellScriptLoader
{
    public:
        spell_item_net_o_matic() : SpellScriptLoader("spell_item_net_o_matic") { }

        class spell_item_net_o_matic_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_net_o_matic_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_NET_O_MATIC_TRIGGERED1) || !sSpellMgr->GetSpellInfo(SPELL_NET_O_MATIC_TRIGGERED2) || !sSpellMgr->GetSpellInfo(SPELL_NET_O_MATIC_TRIGGERED3))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetHitUnit())
                {
                    uint32 spellId = SPELL_NET_O_MATIC_TRIGGERED3;
                    uint32 roll = urand(0, 99);
                    if (roll < 2)                            // 2% for 30 sec self root (off-like chance unknown)
                        spellId = SPELL_NET_O_MATIC_TRIGGERED1;
                    else if (roll < 4)                       // 2% for 20 sec root, charge to target (off-like chance unknown)
                        spellId = SPELL_NET_O_MATIC_TRIGGERED2;

                    GetCaster()->CastSpell(target, spellId, true, NULL);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_net_o_matic_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_net_o_matic_SpellScript();
        }
};

// http://www.wowhead.com/item=8529 Noggenfogger Elixir
// 16589 Noggenfogger Elixir
enum NoggenfoggerElixirSpells
{
    SPELL_NOGGENFOGGER_ELIXIR_TRIGGERED1 = 16595,
    SPELL_NOGGENFOGGER_ELIXIR_TRIGGERED2 = 16593,
    SPELL_NOGGENFOGGER_ELIXIR_TRIGGERED3 = 16591,
};

class spell_item_noggenfogger_elixir : public SpellScriptLoader
{
    public:
        spell_item_noggenfogger_elixir() : SpellScriptLoader("spell_item_noggenfogger_elixir") { }

        class spell_item_noggenfogger_elixir_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_noggenfogger_elixir_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_NOGGENFOGGER_ELIXIR_TRIGGERED1) || !sSpellMgr->GetSpellInfo(SPELL_NOGGENFOGGER_ELIXIR_TRIGGERED2) || !sSpellMgr->GetSpellInfo(SPELL_NOGGENFOGGER_ELIXIR_TRIGGERED3))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                uint32 spellId = SPELL_NOGGENFOGGER_ELIXIR_TRIGGERED3;
                switch (urand(1, 3))
                {
                    case 1: spellId = SPELL_NOGGENFOGGER_ELIXIR_TRIGGERED1; break;
                    case 2: spellId = SPELL_NOGGENFOGGER_ELIXIR_TRIGGERED2; break;
                }

                caster->CastSpell(caster, spellId, true, NULL);
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_item_noggenfogger_elixir_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_noggenfogger_elixir_SpellScript();
        }
};

// 17512 - Piccolo of the Flaming Fire
class spell_item_piccolo_of_the_flaming_fire : public SpellScriptLoader
{
    public:
        spell_item_piccolo_of_the_flaming_fire() : SpellScriptLoader("spell_item_piccolo_of_the_flaming_fire") { }

        class spell_item_piccolo_of_the_flaming_fire_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_piccolo_of_the_flaming_fire_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                if (Player* target = GetHitPlayer())
                    target->HandleEmoteCommand(EMOTE_STATE_DANCE);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_piccolo_of_the_flaming_fire_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_piccolo_of_the_flaming_fire_SpellScript();
        }
};

// http://www.wowhead.com/item=6657 Savory Deviate Delight
// 8213 Savory Deviate Delight
enum SavoryDeviateDelight
{
    SPELL_FLIP_OUT_MALE     = 8219,
    SPELL_FLIP_OUT_FEMALE   = 8220,
    SPELL_YAAARRRR_MALE     = 8221,
    SPELL_YAAARRRR_FEMALE   = 8222,
};

class spell_item_savory_deviate_delight : public SpellScriptLoader
{
    public:
        spell_item_savory_deviate_delight() : SpellScriptLoader("spell_item_savory_deviate_delight") { }

        class spell_item_savory_deviate_delight_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_savory_deviate_delight_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                for (uint32 spellId = SPELL_FLIP_OUT_MALE; spellId <= SPELL_YAAARRRR_FEMALE; ++spellId)
                    if (!sSpellMgr->GetSpellInfo(spellId))
                        return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                uint32 spellId = 0;
                switch (urand(1, 2))
                {
                    // Flip Out - ninja
                    case 1: spellId = (caster->getGender() == GENDER_MALE ? SPELL_FLIP_OUT_MALE : SPELL_FLIP_OUT_FEMALE); break;
                    // Yaaarrrr - pirate
                    case 2: spellId = (caster->getGender() == GENDER_MALE ? SPELL_YAAARRRR_MALE : SPELL_YAAARRRR_FEMALE); break;
                }
                caster->CastSpell(caster, spellId, true, NULL);
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_item_savory_deviate_delight_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_savory_deviate_delight_SpellScript();
        }
};

// 48129 - Scroll of Recall
// 60320 - Scroll of Recall II
// 60321 - Scroll of Recall III
enum ScrollOfRecall
{
    SPELL_SCROLL_OF_RECALL_I                = 48129,
    SPELL_SCROLL_OF_RECALL_II               = 60320,
    SPELL_SCROLL_OF_RECALL_III              = 60321,
    SPELL_LOST                              = 60444,
    SPELL_SCROLL_OF_RECALL_FAIL_ALLIANCE_1  = 60323,
    SPELL_SCROLL_OF_RECALL_FAIL_HORDE_1     = 60328,
};

class spell_item_scroll_of_recall : public SpellScriptLoader
{
    public:
        spell_item_scroll_of_recall() : SpellScriptLoader("spell_item_scroll_of_recall") { }

        class spell_item_scroll_of_recall_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_scroll_of_recall_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                Unit* caster = GetCaster();
                uint8 maxSafeLevel = 0;
                switch (GetSpellInfo()->Id)
                {
                    case SPELL_SCROLL_OF_RECALL_I:  // Scroll of Recall
                        maxSafeLevel = 40;
                        break;
                    case SPELL_SCROLL_OF_RECALL_II:  // Scroll of Recall II
                        maxSafeLevel = 70;
                        break;
                    case SPELL_SCROLL_OF_RECALL_III:  // Scroll of Recal III
                        maxSafeLevel = 80;
                        break;
                    default:
                        break;
                }

                if (caster->getLevel() > maxSafeLevel)
                {
                    caster->CastSpell(caster, SPELL_LOST, true);

                    // ALLIANCE from 60323 to 60330 - HORDE from 60328 to 60335
                    uint32 spellId = SPELL_SCROLL_OF_RECALL_FAIL_ALLIANCE_1;
                    if (GetCaster()->ToPlayer()->GetTeam() == HORDE)
                        spellId = SPELL_SCROLL_OF_RECALL_FAIL_HORDE_1;

                    GetCaster()->CastSpell(GetCaster(), spellId + urand(0, 7), true);

                    PreventHitDefaultEffect(effIndex);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_scroll_of_recall_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_TELEPORT_UNITS);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_scroll_of_recall_SpellScript();
        }
};

// 71169 - Shadow's Fate (Shadowmourne questline)
enum ShadowsFate
{
    SPELL_SOUL_FEAST        = 71203,
    QUEST_A_FEAST_OF_SOULS  = 24547
};

class spell_item_shadows_fate : public SpellScriptLoader
{
    public:
        spell_item_shadows_fate() : SpellScriptLoader("spell_item_shadows_fate") { }

        class spell_item_shadows_fate_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_item_shadows_fate_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SOUL_FEAST))
                    return false;
                if (!sObjectMgr->GetQuestTemplate(QUEST_A_FEAST_OF_SOULS))
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
                return _procTarget && _procTarget->GetTypeId() == TYPEID_PLAYER && _procTarget->ToPlayer()->GetQuestStatus(QUEST_A_FEAST_OF_SOULS) == QUEST_STATUS_INCOMPLETE;
            }

            void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
            {
                PreventDefaultAction();
                GetTarget()->CastSpell(_procTarget, SPELL_SOUL_FEAST, true);
            }

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_item_shadows_fate_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_item_shadows_fate_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }

        private:
            Unit* _procTarget;
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_item_shadows_fate_AuraScript();
        }
};

enum Shadowmourne
{
    SPELL_SHADOWMOURNE_CHAOS_BANE_DAMAGE    = 71904,
    SPELL_SHADOWMOURNE_SOUL_FRAGMENT        = 71905,
    SPELL_SHADOWMOURNE_VISUAL_LOW           = 72521,
    SPELL_SHADOWMOURNE_VISUAL_HIGH          = 72523,
    SPELL_SHADOWMOURNE_CHAOS_BANE_BUFF      = 73422,
};

// 71903 - Item - Shadowmourne Legendary
class spell_item_shadowmourne : public SpellScriptLoader
{
    public:
        spell_item_shadowmourne() : SpellScriptLoader("spell_item_shadowmourne") { }

        class spell_item_shadowmourne_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_item_shadowmourne_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SHADOWMOURNE_CHAOS_BANE_DAMAGE))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_SHADOWMOURNE_SOUL_FRAGMENT))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_SHADOWMOURNE_CHAOS_BANE_BUFF))
                    return false;
                return true;
            }

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                if (GetTarget()->HasAura(SPELL_SHADOWMOURNE_CHAOS_BANE_BUFF)) // cant collect shards while under effect of Chaos Bane buff
                    return false;
                return eventInfo.GetProcTarget() && eventInfo.GetProcTarget()->isAlive();
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                GetTarget()->CastSpell(GetTarget(), SPELL_SHADOWMOURNE_SOUL_FRAGMENT, true, NULL, aurEff);

                // this can't be handled in AuraScript of SoulFragments because we need to know victim
                if (Aura* soulFragments = GetTarget()->GetAura(SPELL_SHADOWMOURNE_SOUL_FRAGMENT))
                {
                    if (soulFragments->GetStackAmount() >= 10)
                    {
                        GetTarget()->CastSpell(eventInfo.GetProcTarget(), SPELL_SHADOWMOURNE_CHAOS_BANE_DAMAGE, true, NULL, aurEff);
                        soulFragments->Remove();
                    }
                }
            }

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_item_shadowmourne_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_item_shadowmourne_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_item_shadowmourne_AuraScript();
        }
};

// 71905 - Soul Fragment
class spell_item_shadowmourne_soul_fragment : public SpellScriptLoader
{
    public:
        spell_item_shadowmourne_soul_fragment() : SpellScriptLoader("spell_item_shadowmourne_soul_fragment") { }

        class spell_item_shadowmourne_soul_fragment_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_item_shadowmourne_soul_fragment_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SHADOWMOURNE_VISUAL_LOW) || !sSpellMgr->GetSpellInfo(SPELL_SHADOWMOURNE_VISUAL_HIGH) || !sSpellMgr->GetSpellInfo(SPELL_SHADOWMOURNE_CHAOS_BANE_BUFF))
                    return false;
                return true;
            }

            void OnStackChange(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                switch (GetStackAmount())
                {
                    case 1:
                        target->CastSpell(target, SPELL_SHADOWMOURNE_VISUAL_LOW, true);
                        break;
                    case 6:
                        target->RemoveAurasDueToSpell(SPELL_SHADOWMOURNE_VISUAL_LOW);
                        target->CastSpell(target, SPELL_SHADOWMOURNE_VISUAL_HIGH, true);
                        break;
                    case 10:
                        target->RemoveAurasDueToSpell(SPELL_SHADOWMOURNE_VISUAL_HIGH);
                        target->CastSpell(target, SPELL_SHADOWMOURNE_CHAOS_BANE_BUFF, true);
                        break;
                    default:
                        break;
                }
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                target->RemoveAurasDueToSpell(SPELL_SHADOWMOURNE_VISUAL_LOW);
                target->RemoveAurasDueToSpell(SPELL_SHADOWMOURNE_VISUAL_HIGH);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_item_shadowmourne_soul_fragment_AuraScript::OnStackChange, EFFECT_0, SPELL_AURA_MOD_STAT, AuraEffectHandleModes(AURA_EFFECT_HANDLE_REAL | AURA_EFFECT_HANDLE_REAPPLY));
                AfterEffectRemove += AuraEffectRemoveFn(spell_item_shadowmourne_soul_fragment_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_STAT, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_item_shadowmourne_soul_fragment_AuraScript();
        }
};

// http://www.wowhead.com/item=7734 Six Demon Bag
// 14537 Six Demon Bag
enum SixDemonBagSpells
{
    SPELL_FROSTBOLT                 = 11538,
    SPELL_POLYMORPH                 = 14621,
    SPELL_SUMMON_FELHOUND_MINION    = 14642,
    SPELL_FIREBALL                  = 15662,
    SPELL_CHAIN_LIGHTNING           = 21179,
    SPELL_ENVELOPING_WINDS          = 25189,
};

class spell_item_six_demon_bag : public SpellScriptLoader
{
    public:
        spell_item_six_demon_bag() : SpellScriptLoader("spell_item_six_demon_bag") { }

        class spell_item_six_demon_bag_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_six_demon_bag_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_FROSTBOLT) || !sSpellMgr->GetSpellInfo(SPELL_POLYMORPH) || !sSpellMgr->GetSpellInfo(SPELL_SUMMON_FELHOUND_MINION) || !sSpellMgr->GetSpellInfo(SPELL_FIREBALL) || !sSpellMgr->GetSpellInfo(SPELL_CHAIN_LIGHTNING) || !sSpellMgr->GetSpellInfo(SPELL_ENVELOPING_WINDS))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetHitUnit())
                {
                    uint32 spellId = 0;
                    uint32 rand = urand(0, 99);
                    if (rand < 25)                      // Fireball (25% chance)
                        spellId = SPELL_FIREBALL;
                    else if (rand < 50)                 // Frostball (25% chance)
                        spellId = SPELL_FROSTBOLT;
                    else if (rand < 70)                 // Chain Lighting (20% chance)
                        spellId = SPELL_CHAIN_LIGHTNING;
                    else if (rand < 80)                 // Polymorph (10% chance)
                    {
                        spellId = SPELL_POLYMORPH;
                        if (urand(0, 100) <= 30)        // 30% chance to self-cast
                            target = caster;
                    }
                    else if (rand < 95)                 // Enveloping Winds (15% chance)
                        spellId = SPELL_ENVELOPING_WINDS;
                    else                                // Summon Felhund minion (5% chance)
                    {
                        spellId = SPELL_SUMMON_FELHOUND_MINION;
                        target = caster;
                    }

                    caster->CastSpell(target, spellId, true, GetCastItem());
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_six_demon_bag_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_six_demon_bag_SpellScript();
        }
};

// 28862 - The Eye of Diminution
class spell_item_the_eye_of_diminution : public SpellScriptLoader
{
    public:
        spell_item_the_eye_of_diminution() : SpellScriptLoader("spell_item_the_eye_of_diminution") { }

        class spell_item_the_eye_of_diminution_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_item_the_eye_of_diminution_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                int32 diff = GetUnitOwner()->getLevel() - 60;
                if (diff > 0)
                    amount += diff;
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_item_the_eye_of_diminution_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_THREAT);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_item_the_eye_of_diminution_AuraScript();
        }
};

// http://www.wowhead.com/item=44012 Underbelly Elixir
// 59640 Underbelly Elixir
enum UnderbellyElixirSpells
{
    SPELL_UNDERBELLY_ELIXIR_TRIGGERED1 = 59645,
    SPELL_UNDERBELLY_ELIXIR_TRIGGERED2 = 59831,
    SPELL_UNDERBELLY_ELIXIR_TRIGGERED3 = 59843,
};

class spell_item_underbelly_elixir : public SpellScriptLoader
{
    public:
        spell_item_underbelly_elixir() : SpellScriptLoader("spell_item_underbelly_elixir") { }

        class spell_item_underbelly_elixir_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_underbelly_elixir_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }
            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_UNDERBELLY_ELIXIR_TRIGGERED1) || !sSpellMgr->GetSpellInfo(SPELL_UNDERBELLY_ELIXIR_TRIGGERED2) || !sSpellMgr->GetSpellInfo(SPELL_UNDERBELLY_ELIXIR_TRIGGERED3))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                uint32 spellId = SPELL_UNDERBELLY_ELIXIR_TRIGGERED3;
                switch (urand(1, 3))
                {
                    case 1: spellId = SPELL_UNDERBELLY_ELIXIR_TRIGGERED1; break;
                    case 2: spellId = SPELL_UNDERBELLY_ELIXIR_TRIGGERED2; break;
                }
                caster->CastSpell(caster, spellId, true, NULL);
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_item_underbelly_elixir_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_underbelly_elixir_SpellScript();
        }
};

enum AirRifleSpells
{
    SPELL_AIR_RIFLE_HOLD_VISUAL = 65582,
    SPELL_AIR_RIFLE_SHOOT       = 67532,
    SPELL_AIR_RIFLE_SHOOT_SELF  = 65577,
};

class spell_item_red_rider_air_rifle : public SpellScriptLoader
{
    public:
        spell_item_red_rider_air_rifle() : SpellScriptLoader("spell_item_red_rider_air_rifle") { }

        class spell_item_red_rider_air_rifle_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_red_rider_air_rifle_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_AIR_RIFLE_HOLD_VISUAL) || !sSpellMgr->GetSpellInfo(SPELL_AIR_RIFLE_SHOOT) || !sSpellMgr->GetSpellInfo(SPELL_AIR_RIFLE_SHOOT_SELF))
                    return false;
                return true;
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                Unit* caster = GetCaster();
                if (Unit* target = GetHitUnit())
                {
                    caster->CastSpell(caster, SPELL_AIR_RIFLE_HOLD_VISUAL, true);
                    // needed because this spell shares GCD with its triggered spells (which must not be cast with triggered flag)
                    if (Player* player = caster->ToPlayer())
                        player->GetGlobalCooldownMgr().CancelGlobalCooldown(GetSpellInfo());
                    if (urand(0, 4))
                        caster->CastSpell(target, SPELL_AIR_RIFLE_SHOOT, false);
                    else
                        caster->CastSpell(caster, SPELL_AIR_RIFLE_SHOOT_SELF, false);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_red_rider_air_rifle_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_red_rider_air_rifle_SpellScript();
        }
};

enum GenericData
{
    SPELL_ARCANITE_DRAGONLING           = 19804,
    SPELL_BATTLE_CHICKEN                = 13166,
    SPELL_MECHANICAL_DRAGONLING         = 4073,
    SPELL_MITHRIL_MECHANICAL_DRAGONLING = 12749,
};

enum CreateHeartCandy
{
    ITEM_HEART_CANDY_1 = 21818,
    ITEM_HEART_CANDY_2 = 21817,
    ITEM_HEART_CANDY_3 = 21821,
    ITEM_HEART_CANDY_4 = 21819,
    ITEM_HEART_CANDY_5 = 21816,
    ITEM_HEART_CANDY_6 = 21823,
    ITEM_HEART_CANDY_7 = 21822,
    ITEM_HEART_CANDY_8 = 21820,
};

class spell_item_create_heart_candy : public SpellScriptLoader
{
    public:
        spell_item_create_heart_candy() : SpellScriptLoader("spell_item_create_heart_candy") { }

        class spell_item_create_heart_candy_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_create_heart_candy_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                if (Player* target = GetHitPlayer())
                {
                    static const uint32 items[] = {ITEM_HEART_CANDY_1, ITEM_HEART_CANDY_2, ITEM_HEART_CANDY_3, ITEM_HEART_CANDY_4, ITEM_HEART_CANDY_5, ITEM_HEART_CANDY_6, ITEM_HEART_CANDY_7, ITEM_HEART_CANDY_8};
                    target->AddItem(items[urand(0, 7)], 1);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_create_heart_candy_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_create_heart_candy_SpellScript();
        }
};

class spell_item_book_of_discovery : public SpellScriptLoader
{
    public:
        spell_item_book_of_discovery(char const* scriptName) : SpellScriptLoader(scriptName) { }

        class spell_item_book_of_discovery_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_book_of_discovery_SpellScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            SpellCastResult CheckRequirement()
            {
                if (HasDiscoveredAllSpells(GetSpellInfo()->Id, GetCaster()->ToPlayer()))
                {
                    SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_LEARNED_EVERYTHING);
                    return SPELL_FAILED_CUSTOM_ERROR;
                }

                return SPELL_CAST_OK;
            }

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                Player* caster = GetCaster()->ToPlayer();
                uint32 spellId = GetSpellInfo()->Id;

                // learn random explicit discovery recipe (if any)
                if (uint32 discoveredSpellId = GetExplicitDiscoverySpell(spellId, caster))
                    caster->learnSpell(discoveredSpellId, false);
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_item_book_of_discovery_SpellScript::CheckRequirement);
                OnEffectHitTarget += SpellEffectFn(spell_item_book_of_discovery_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_book_of_discovery_SpellScript();
        }
};

enum GiftOfTheHarvester
{
    NPC_GHOUL   = 28845,
    MAX_GHOULS  = 5,
};

class spell_item_gift_of_the_harvester : public SpellScriptLoader
{
    public:
        spell_item_gift_of_the_harvester() : SpellScriptLoader("spell_item_gift_of_the_harvester") {}

        class spell_item_gift_of_the_harvester_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_gift_of_the_harvester_SpellScript);

            SpellCastResult CheckRequirement()
            {
                std::list<Creature*> ghouls;
                GetCaster()->GetAllMinionsByEntry(ghouls, NPC_GHOUL);
                if (ghouls.size() >= MAX_GHOULS)
                {
                    SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_TOO_MANY_GHOULS);
                    return SPELL_FAILED_CUSTOM_ERROR;
                }

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_item_gift_of_the_harvester_SpellScript::CheckRequirement);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_gift_of_the_harvester_SpellScript();
        }
};

enum Sinkholes
{
    NPC_SOUTH_SINKHOLE      = 25664,
    NPC_NORTHEAST_SINKHOLE  = 25665,
    NPC_NORTHWEST_SINKHOLE  = 25666,
};

class spell_item_map_of_the_geyser_fields : public SpellScriptLoader
{
    public:
        spell_item_map_of_the_geyser_fields() : SpellScriptLoader("spell_item_map_of_the_geyser_fields") {}

        class spell_item_map_of_the_geyser_fields_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_map_of_the_geyser_fields_SpellScript);

            SpellCastResult CheckSinkholes()
            {
                Unit* caster = GetCaster();
                if (caster->FindNearestCreature(NPC_SOUTH_SINKHOLE, 30.0f, true) ||
                    caster->FindNearestCreature(NPC_NORTHEAST_SINKHOLE, 30.0f, true) ||
                    caster->FindNearestCreature(NPC_NORTHWEST_SINKHOLE, 30.0f, true))
                    return SPELL_CAST_OK;

                SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_MUST_BE_CLOSE_TO_SINKHOLE);
                return SPELL_FAILED_CUSTOM_ERROR;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_item_map_of_the_geyser_fields_SpellScript::CheckSinkholes);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_map_of_the_geyser_fields_SpellScript();
        }
};

enum VanquishedClutchesSpells
{
    SPELL_CRUSHER       = 64982,
    SPELL_CONSTRICTOR   = 64983,
    SPELL_CORRUPTOR     = 64984,
};

class spell_item_vanquished_clutches : public SpellScriptLoader
{
    public:
        spell_item_vanquished_clutches() : SpellScriptLoader("spell_item_vanquished_clutches") { }

        class spell_item_vanquished_clutches_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_vanquished_clutches_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_CRUSHER) || !sSpellMgr->GetSpellInfo(SPELL_CONSTRICTOR) || !sSpellMgr->GetSpellInfo(SPELL_CORRUPTOR))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                uint32 spellId = RAND(SPELL_CRUSHER, SPELL_CONSTRICTOR, SPELL_CORRUPTOR);
                Unit* caster = GetCaster();
                caster->CastSpell(caster, spellId, true);
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_item_vanquished_clutches_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_vanquished_clutches_SpellScript();
        }
};

enum AshbringerSounds
{
    SOUND_ASHBRINGER_1  = 8906,                             // "I was pure once"
    SOUND_ASHBRINGER_2  = 8907,                             // "Fought for righteousness"
    SOUND_ASHBRINGER_3  = 8908,                             // "I was once called Ashbringer"
    SOUND_ASHBRINGER_4  = 8920,                             // "Betrayed by my order"
    SOUND_ASHBRINGER_5  = 8921,                             // "Destroyed by Kel'Thuzad"
    SOUND_ASHBRINGER_6  = 8922,                             // "Made to serve"
    SOUND_ASHBRINGER_7  = 8923,                             // "My son watched me die"
    SOUND_ASHBRINGER_8  = 8924,                             // "Crusades fed his rage"
    SOUND_ASHBRINGER_9  = 8925,                             // "Truth is unknown to him"
    SOUND_ASHBRINGER_10 = 8926,                             // "Scarlet Crusade  is pure no longer"
    SOUND_ASHBRINGER_11 = 8927,                             // "Balnazzar's crusade corrupted my son"
    SOUND_ASHBRINGER_12 = 8928,                             // "Kill them all!"
};

class spell_item_ashbringer : public SpellScriptLoader
{
    public:
        spell_item_ashbringer() : SpellScriptLoader("spell_item_ashbringer") {}

        class spell_item_ashbringer_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_ashbringer_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void OnDummyEffect(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);

                Player* player = GetCaster()->ToPlayer();
                uint32 sound_id = RAND( SOUND_ASHBRINGER_1, SOUND_ASHBRINGER_2, SOUND_ASHBRINGER_3, SOUND_ASHBRINGER_4, SOUND_ASHBRINGER_5, SOUND_ASHBRINGER_6,
                                SOUND_ASHBRINGER_7, SOUND_ASHBRINGER_8, SOUND_ASHBRINGER_9, SOUND_ASHBRINGER_10, SOUND_ASHBRINGER_11, SOUND_ASHBRINGER_12 );

                // Ashbringers effect (spellID 28441) retriggers every 5 seconds, with a chance of making it say one of the above 12 sounds
                if (urand(0, 60) < 1)
                    player->PlayDirectSound(sound_id, player);
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_item_ashbringer_SpellScript::OnDummyEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_ashbringer_SpellScript();
        }
};

enum MagicEater
{
    SPELL_WILD_MAGIC                             = 58891,
    SPELL_WELL_FED_1                             = 57288,
    SPELL_WELL_FED_2                             = 57139,
    SPELL_WELL_FED_3                             = 57111,
    SPELL_WELL_FED_4                             = 57286,
    SPELL_WELL_FED_5                             = 57291,
};

class spell_magic_eater_food : public SpellScriptLoader
{
    public:
        spell_magic_eater_food() : SpellScriptLoader("spell_magic_eater_food") {}

        class spell_magic_eater_food_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_magic_eater_food_AuraScript);

            void HandleTriggerSpell(AuraEffect const* /*aurEff*/)
            {
                PreventDefaultAction();
                Unit* target = GetTarget();
                switch (urand(0, 5))
                {
                    case 0:
                        target->CastSpell(target, SPELL_WILD_MAGIC, true);
                        break;
                    case 1:
                        target->CastSpell(target, SPELL_WELL_FED_1, true);
                        break;
                    case 2:
                        target->CastSpell(target, SPELL_WELL_FED_2, true);
                        break;
                    case 3:
                        target->CastSpell(target, SPELL_WELL_FED_3, true);
                        break;
                    case 4:
                        target->CastSpell(target, SPELL_WELL_FED_4, true);
                        break;
                    case 5:
                        target->CastSpell(target, SPELL_WELL_FED_5, true);
                        break;
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_magic_eater_food_AuraScript::HandleTriggerSpell, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_magic_eater_food_AuraScript();
        }
};

class spell_item_shimmering_vessel : public SpellScriptLoader
{
    public:
        spell_item_shimmering_vessel() : SpellScriptLoader("spell_item_shimmering_vessel") { }

        class spell_item_shimmering_vessel_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_shimmering_vessel_SpellScript);

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                if (Creature* target = GetHitCreature())
                    target->setDeathState(JUST_RESPAWNED);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_shimmering_vessel_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_shimmering_vessel_SpellScript();
        }
};

enum PurifyHelboarMeat
{
    SPELL_SUMMON_PURIFIED_HELBOAR_MEAT      = 29277,
    SPELL_SUMMON_TOXIC_HELBOAR_MEAT         = 29278,
};

class spell_item_purify_helboar_meat : public SpellScriptLoader
{
    public:
        spell_item_purify_helboar_meat() : SpellScriptLoader("spell_item_purify_helboar_meat") { }

        class spell_item_purify_helboar_meat_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_purify_helboar_meat_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SUMMON_PURIFIED_HELBOAR_MEAT) ||  !sSpellMgr->GetSpellInfo(SPELL_SUMMON_TOXIC_HELBOAR_MEAT))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Unit* caster = GetCaster();
                caster->CastSpell(caster, roll_chance_i(50) ? SPELL_SUMMON_PURIFIED_HELBOAR_MEAT : SPELL_SUMMON_TOXIC_HELBOAR_MEAT, true, NULL);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_purify_helboar_meat_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_purify_helboar_meat_SpellScript();
        }
};

enum CrystalPrison
{
    OBJECT_IMPRISONED_DOOMGUARD     = 179644,
};

class spell_item_crystal_prison_dummy_dnd : public SpellScriptLoader
{
    public:
        spell_item_crystal_prison_dummy_dnd() : SpellScriptLoader("spell_item_crystal_prison_dummy_dnd") { }

        class spell_item_crystal_prison_dummy_dnd_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_crystal_prison_dummy_dnd_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sObjectMgr->GetGameObjectTemplate(OBJECT_IMPRISONED_DOOMGUARD))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                if (Creature* target = GetHitCreature())
                    if (target->isDead() && !target->isPet())
                    {
                        GetCaster()->SummonGameObject(OBJECT_IMPRISONED_DOOMGUARD, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation(), 0, 0, 0, 0, uint32(target->GetRespawnTime()-time(NULL)));
                        target->DespawnOrUnsummon();
                    }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_crystal_prison_dummy_dnd_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_crystal_prison_dummy_dnd_SpellScript();
        }
};

enum ReindeerTransformation
{
    SPELL_FLYING_REINDEER_310                   = 44827,
    SPELL_FLYING_REINDEER_280                   = 44825,
    SPELL_FLYING_REINDEER_60                    = 44824,
    SPELL_REINDEER_100                          = 25859,
    SPELL_REINDEER_60                           = 25858,
};

class spell_item_reindeer_transformation : public SpellScriptLoader
{
    public:
        spell_item_reindeer_transformation() : SpellScriptLoader("spell_item_reindeer_transformation") { }

        class spell_item_reindeer_transformation_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_reindeer_transformation_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_FLYING_REINDEER_310) || !sSpellMgr->GetSpellInfo(SPELL_FLYING_REINDEER_280)
                    || !sSpellMgr->GetSpellInfo(SPELL_FLYING_REINDEER_60) || !sSpellMgr->GetSpellInfo(SPELL_REINDEER_100)
                    || !sSpellMgr->GetSpellInfo(SPELL_REINDEER_60))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Unit* caster = GetCaster();
                if (caster->HasAuraType(SPELL_AURA_MOUNTED))
                {
                    float flyspeed = caster->GetSpeedRate(MOVE_FLIGHT);
                    float speed = caster->GetSpeedRate(MOVE_RUN);

                    caster->RemoveAurasByType(SPELL_AURA_MOUNTED);
                    //5 different spells used depending on mounted speed and if mount can fly or not

                    if (flyspeed >= 4.1f)
                        // Flying Reindeer
                        caster->CastSpell(caster, SPELL_FLYING_REINDEER_310, true); //310% flying Reindeer
                    else if (flyspeed >= 3.8f)
                        // Flying Reindeer
                        caster->CastSpell(caster, SPELL_FLYING_REINDEER_280, true); //280% flying Reindeer
                    else if (flyspeed >= 1.6f)
                        // Flying Reindeer
                        caster->CastSpell(caster, SPELL_FLYING_REINDEER_60, true); //60% flying Reindeer
                    else if (speed >= 2.0f)
                        // Reindeer
                        caster->CastSpell(caster, SPELL_REINDEER_100, true); //100% ground Reindeer
                    else
                        // Reindeer
                        caster->CastSpell(caster, SPELL_REINDEER_60, true); //60% ground Reindeer
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_reindeer_transformation_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_item_reindeer_transformation_SpellScript();
    }
};

enum NighInvulnerability
{
    SPELL_NIGH_INVULNERABILITY                  = 30456,
    SPELL_COMPLETE_VULNERABILITY                = 30457,
};

class spell_item_nigh_invulnerability : public SpellScriptLoader
{
    public:
        spell_item_nigh_invulnerability() : SpellScriptLoader("spell_item_nigh_invulnerability") { }

        class spell_item_nigh_invulnerability_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_nigh_invulnerability_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_NIGH_INVULNERABILITY) || !sSpellMgr->GetSpellInfo(SPELL_COMPLETE_VULNERABILITY))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Unit* caster = GetCaster();
                if (Item* castItem = GetCastItem())
                {
                    if (roll_chance_i(86))                  // Nigh-Invulnerability   - success
                        caster->CastSpell(caster, SPELL_NIGH_INVULNERABILITY, true, castItem);
                    else                                    // Complete Vulnerability - backfire in 14% casts
                        caster->CastSpell(caster, SPELL_COMPLETE_VULNERABILITY, true, castItem);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_nigh_invulnerability_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_nigh_invulnerability_SpellScript();
        }
};

enum Poultryzer
{
    SPELL_POULTRYIZER_SUCCESS    = 30501,
    SPELL_POULTRYIZER_BACKFIRE   = 30504,
};

class spell_item_poultryizer : public SpellScriptLoader
{
    public:
        spell_item_poultryizer() : SpellScriptLoader("spell_item_poultryizer") { }

        class spell_item_poultryizer_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_poultryizer_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_POULTRYIZER_SUCCESS) || !sSpellMgr->GetSpellInfo(SPELL_POULTRYIZER_BACKFIRE))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                if (GetCastItem() && GetHitUnit())
                    GetCaster()->CastSpell(GetHitUnit(), roll_chance_i(80) ? SPELL_POULTRYIZER_SUCCESS : SPELL_POULTRYIZER_BACKFIRE, true, GetCastItem());
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_poultryizer_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_poultryizer_SpellScript();
        }
};

enum SocretharsStone
{
    SPELL_SOCRETHAR_TO_SEAT     = 35743,
    SPELL_SOCRETHAR_FROM_SEAT   = 35744,
};

class spell_item_socrethars_stone : public SpellScriptLoader
{
    public:
        spell_item_socrethars_stone() : SpellScriptLoader("spell_item_socrethars_stone") { }

        class spell_item_socrethars_stone_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_socrethars_stone_SpellScript);

            bool Load()
            {
                return (GetCaster()->GetAreaId() == 3900 || GetCaster()->GetAreaId() == 3742);
            }
            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SOCRETHAR_TO_SEAT) || !sSpellMgr->GetSpellInfo(SPELL_SOCRETHAR_FROM_SEAT))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Unit* caster = GetCaster();
                switch (caster->GetAreaId())
                {
                    case 3900:
                        caster->CastSpell(caster, SPELL_SOCRETHAR_TO_SEAT, true);
                        break;
                    case 3742:
                        caster->CastSpell(caster, SPELL_SOCRETHAR_FROM_SEAT, true);
                        break;
                    default:
                        return;
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_socrethars_stone_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_socrethars_stone_SpellScript();
        }
};

enum DemonBroiledSurprise
{
    QUEST_SUPER_HOT_STEW                    = 11379,
    SPELL_CREATE_DEMON_BROILED_SURPRISE     = 43753,
    NPC_ABYSSAL_FLAMEBRINGER                = 19973,
};

class spell_item_demon_broiled_surprise : public SpellScriptLoader
{
    public:
        spell_item_demon_broiled_surprise() : SpellScriptLoader("spell_item_demon_broiled_surprise") { }

        class spell_item_demon_broiled_surprise_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_demon_broiled_surprise_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_CREATE_DEMON_BROILED_SURPRISE) || !sObjectMgr->GetCreatureTemplate(NPC_ABYSSAL_FLAMEBRINGER) || !sObjectMgr->GetQuestTemplate(QUEST_SUPER_HOT_STEW))
                    return false;
                return true;
            }

            bool Load()
            {
               return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Unit* player = GetCaster();
                player->CastSpell(player, SPELL_CREATE_DEMON_BROILED_SURPRISE, false);
            }

            SpellCastResult CheckRequirement()
            {
                Player* player = GetCaster()->ToPlayer();
                if (player->GetQuestStatus(QUEST_SUPER_HOT_STEW) != QUEST_STATUS_INCOMPLETE)
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                if (Creature* creature = player->FindNearestCreature(NPC_ABYSSAL_FLAMEBRINGER, 10, false))
                    if (creature->isDead())
                        return SPELL_CAST_OK;
                return SPELL_FAILED_NOT_HERE;
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_demon_broiled_surprise_SpellScript::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
                OnCheckCast += SpellCheckCastFn(spell_item_demon_broiled_surprise_SpellScript::CheckRequirement);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_demon_broiled_surprise_SpellScript();
        }
};

enum CompleteRaptorCapture
{
    SPELL_RAPTOR_CAPTURE_CREDIT     = 42337,
};

class spell_item_complete_raptor_capture : public SpellScriptLoader
{
    public:
        spell_item_complete_raptor_capture() : SpellScriptLoader("spell_item_complete_raptor_capture") { }

        class spell_item_complete_raptor_capture_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_complete_raptor_capture_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_RAPTOR_CAPTURE_CREDIT))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Unit* caster = GetCaster();
                if (GetHitCreature())
                {
                    GetHitCreature()->DespawnOrUnsummon();

                    //cast spell Raptor Capture Credit
                    caster->CastSpell(caster, SPELL_RAPTOR_CAPTURE_CREDIT, true, NULL);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_complete_raptor_capture_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_complete_raptor_capture_SpellScript();
        }
};

enum ImpaleLeviroth
{
    NPC_LEVIROTH                = 26452,
    SPELL_LEVIROTH_SELF_IMPALE  = 49882,
};

class spell_item_impale_leviroth : public SpellScriptLoader
{
    public:
        spell_item_impale_leviroth() : SpellScriptLoader("spell_item_impale_leviroth") { }

        class spell_item_impale_leviroth_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_impale_leviroth_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sObjectMgr->GetCreatureTemplate(NPC_LEVIROTH))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                if (Unit* target = GetHitCreature())
                    if (target->GetEntry() == NPC_LEVIROTH && !target->HealthBelowPct(95))
                        target->CastSpell(target, SPELL_LEVIROTH_SELF_IMPALE, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_impale_leviroth_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_impale_leviroth_SpellScript();
        }
};

enum BrewfestMountTransformation
{
    SPELL_MOUNT_RAM_100                         = 43900,
    SPELL_MOUNT_RAM_60                          = 43899,
    SPELL_MOUNT_KODO_100                        = 49379,
    SPELL_MOUNT_KODO_60                         = 49378,
    SPELL_BREWFEST_MOUNT_TRANSFORM              = 49357,
    SPELL_BREWFEST_MOUNT_TRANSFORM_REVERSE      = 52845,
};

class spell_item_brewfest_mount_transformation : public SpellScriptLoader
{
    public:
        spell_item_brewfest_mount_transformation() : SpellScriptLoader("spell_item_brewfest_mount_transformation") { }

        class spell_item_brewfest_mount_transformation_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_brewfest_mount_transformation_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MOUNT_RAM_100) || !sSpellMgr->GetSpellInfo(SPELL_MOUNT_RAM_60) || !sSpellMgr->GetSpellInfo(SPELL_MOUNT_KODO_100) || !sSpellMgr->GetSpellInfo(SPELL_MOUNT_KODO_60))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Player* caster = GetCaster()->ToPlayer();
                if (caster->HasAuraType(SPELL_AURA_MOUNTED))
                {
                    caster->RemoveAurasByType(SPELL_AURA_MOUNTED);
                    uint32 spell_id;

                    switch (GetSpellInfo()->Id)
                    {
                        case SPELL_BREWFEST_MOUNT_TRANSFORM:
                            if (caster->GetSpeedRate(MOVE_RUN) >= 2.0f)
                                spell_id = caster->GetTeam() == ALLIANCE ? SPELL_MOUNT_RAM_100 : SPELL_MOUNT_KODO_100;
                            else
                                spell_id = caster->GetTeam() == ALLIANCE ? SPELL_MOUNT_RAM_60 : SPELL_MOUNT_KODO_60;
                            break;
                        case SPELL_BREWFEST_MOUNT_TRANSFORM_REVERSE:
                            if (caster->GetSpeedRate(MOVE_RUN) >= 2.0f)
                                spell_id = caster->GetTeam() == HORDE ? SPELL_MOUNT_RAM_100 : SPELL_MOUNT_KODO_100;
                            else
                                spell_id = caster->GetTeam() == HORDE ? SPELL_MOUNT_RAM_60 : SPELL_MOUNT_KODO_60;
                            break;
                        default:
                            return;
                    }
                    caster->CastSpell(caster, spell_id, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_brewfest_mount_transformation_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_brewfest_mount_transformation_SpellScript();
        }
};

enum NitroBoots
{
    SPELL_NITRO_BOOSTS_SUCCESS           = 54861,
    SPELL_NITRO_BOOSTS_BACKFIRE          = 46014,
    SPELL_NITRO_BOOSTS_ROCKET_FUEL_LEAK  = 94794,
};

class spell_item_nitro_boots : public SpellScriptLoader
{
    public:
        spell_item_nitro_boots() : SpellScriptLoader("spell_item_nitro_boots") { }

        class spell_item_nitro_boots_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_nitro_boots_SpellScript);

            bool Load()
            {
                if (!GetCastItem())
                    return false;
                return true;
            }

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_NITRO_BOOSTS_SUCCESS) || !sSpellMgr->GetSpellInfo(SPELL_NITRO_BOOSTS_BACKFIRE))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Unit* caster = GetCaster();

                int32 FAILED = SPELL_NITRO_BOOSTS_BACKFIRE;
                int32 SUCCESS = SPELL_NITRO_BOOSTS_SUCCESS;

                if (caster->GetMap()->Instanceable())
                    FAILED = SPELL_NITRO_BOOSTS_ROCKET_FUEL_LEAK;

                caster->CastSpell(caster, roll_chance_i(95) ? SUCCESS : FAILED, true, GetCastItem());
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_nitro_boots_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_nitro_boots_SpellScript();
        }
};

enum TeachLanguage
{
    SPELL_LEARN_GNOMISH_BINARY      = 50242,
    SPELL_LEARN_GOBLIN_BINARY       = 50246,
};

class spell_item_teach_language : public SpellScriptLoader
{
    public:
        spell_item_teach_language() : SpellScriptLoader("spell_item_teach_language") { }

        class spell_item_teach_language_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_teach_language_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_LEARN_GNOMISH_BINARY) || !sSpellMgr->GetSpellInfo(SPELL_LEARN_GOBLIN_BINARY))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Player* caster = GetCaster()->ToPlayer();

                if (roll_chance_i(34))
                    caster->CastSpell(caster, caster->GetTeam() == ALLIANCE ? SPELL_LEARN_GNOMISH_BINARY : SPELL_LEARN_GOBLIN_BINARY, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_teach_language_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_teach_language_SpellScript();
        }
};

enum RocketBoots
{
    SPELL_ROCKET_BOOTS_PROC      = 30452,
};

class spell_item_rocket_boots : public SpellScriptLoader
{
    public:
        spell_item_rocket_boots() : SpellScriptLoader("spell_item_rocket_boots") { }

        class spell_item_rocket_boots_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_rocket_boots_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_ROCKET_BOOTS_PROC))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Player* caster = GetCaster()->ToPlayer();
                if (Battleground* bg = caster->GetBattleground())
                    bg->EventPlayerDroppedFlag(caster);

                caster->RemoveSpellCooldown(SPELL_ROCKET_BOOTS_PROC);
                caster->CastSpell(caster, SPELL_ROCKET_BOOTS_PROC, true, NULL);
            }

            SpellCastResult CheckCast()
            {
                if (GetCaster()->IsInWater())
                    return SPELL_FAILED_ONLY_ABOVEWATER;
                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_item_rocket_boots_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_item_rocket_boots_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_rocket_boots_SpellScript();
        }
};

enum PygmyOil
{
    SPELL_PYGMY_OIL_PYGMY_AURA      = 53806,
    SPELL_PYGMY_OIL_SMALLER_AURA    = 53805,
};

class spell_item_pygmy_oil : public SpellScriptLoader
{
    public:
        spell_item_pygmy_oil() : SpellScriptLoader("spell_item_pygmy_oil") { }

        class spell_item_pygmy_oil_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_pygmy_oil_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PYGMY_OIL_PYGMY_AURA) || !sSpellMgr->GetSpellInfo(SPELL_PYGMY_OIL_SMALLER_AURA))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Unit* caster = GetCaster();
                if (Aura* aura = caster->GetAura(SPELL_PYGMY_OIL_PYGMY_AURA))
                    aura->RefreshDuration();
                else
                {
                    aura = caster->GetAura(SPELL_PYGMY_OIL_SMALLER_AURA);
                    if (!aura || aura->GetStackAmount() < 5 || !roll_chance_i(50))
                         caster->CastSpell(caster, SPELL_PYGMY_OIL_SMALLER_AURA, true);
                    else
                    {
                        aura->Remove();
                        caster->CastSpell(caster, SPELL_PYGMY_OIL_PYGMY_AURA, true);
                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_pygmy_oil_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_pygmy_oil_SpellScript();
        }
};

class spell_item_unusual_compass : public SpellScriptLoader
{
    public:
        spell_item_unusual_compass() : SpellScriptLoader("spell_item_unusual_compass") { }

        class spell_item_unusual_compass_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_unusual_compass_SpellScript);

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Unit* caster = GetCaster();
                caster->SetFacingTo(frand(0.0f, 62832.0f) / 10000.0f);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_unusual_compass_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_unusual_compass_SpellScript();
        }
};

enum ChickenCover
{
    SPELL_CHICKEN_NET               = 51959,
    SPELL_CAPTURE_CHICKEN_ESCAPE    = 51037,
    QUEST_CHICKEN_PARTY             = 12702,
    QUEST_FLOWN_THE_COOP            = 12532,
};

class spell_item_chicken_cover : public SpellScriptLoader
{
    public:
        spell_item_chicken_cover() : SpellScriptLoader("spell_item_chicken_cover") { }

        class spell_item_chicken_cover_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_chicken_cover_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_CHICKEN_NET) || !sSpellMgr->GetSpellInfo(SPELL_CAPTURE_CHICKEN_ESCAPE) || !sObjectMgr->GetQuestTemplate(QUEST_CHICKEN_PARTY) || !sObjectMgr->GetQuestTemplate(QUEST_FLOWN_THE_COOP))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Player* caster = GetCaster()->ToPlayer();
                if (Unit* target = GetHitUnit())
                {
                    if (!target->HasAura(SPELL_CHICKEN_NET) && (caster->GetQuestStatus(QUEST_CHICKEN_PARTY) == QUEST_STATUS_INCOMPLETE || caster->GetQuestStatus(QUEST_FLOWN_THE_COOP) == QUEST_STATUS_INCOMPLETE))
                    {
                        caster->CastSpell(caster, SPELL_CAPTURE_CHICKEN_ESCAPE, true);
                        target->Kill(target);
                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_chicken_cover_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_chicken_cover_SpellScript();
        }
};

enum Refocus
{
    SPELL_AIMED_SHOT    = 19434,
    SPELL_MULTISHOT     = 2643,
    SPELL_VOLLEY        = 42243,
};

class spell_item_muisek_vessel : public SpellScriptLoader
{
    public:
        spell_item_muisek_vessel() : SpellScriptLoader("spell_item_muisek_vessel") { }

        class spell_item_muisek_vessel_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_muisek_vessel_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Creature* target = GetHitCreature())
                    if (target->isDead())
                        target->DespawnOrUnsummon();
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_muisek_vessel_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_muisek_vessel_SpellScript();
        }
};

enum GreatmothersSoulcather
{
    SPELL_FORCE_CAST_SUMMON_GNOME_SOUL = 46486,
};
class spell_item_greatmothers_soulcatcher : public SpellScriptLoader
{
public:
    spell_item_greatmothers_soulcatcher() : SpellScriptLoader("spell_item_greatmothers_soulcatcher") { }

    class spell_item_greatmothers_soulcatcher_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_item_greatmothers_soulcatcher_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (GetHitUnit())
                GetCaster()->CastSpell(GetCaster(), SPELL_FORCE_CAST_SUMMON_GNOME_SOUL);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_item_greatmothers_soulcatcher_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_item_greatmothers_soulcatcher_SpellScript();
    }
};

class spell_item_healthstone : public SpellScriptLoader
{
    public:
        spell_item_healthstone() : SpellScriptLoader("spell_item_healthstone") { }

        class spell_item_healthstone_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_healthstone_SpellScript);

            bool inSoulburn;

            bool Load()
            {
                inSoulburn = false;
                return true;
            }

            void HandleOnHit(SpellEffIndex effIndex)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                uint32 amount = CalculatePct(caster->GetCreateHealth(), GetSpellInfo()->Effects[effIndex].BasePoints);

                SetEffectDamage(amount);

                if (inSoulburn)
                    caster->CastSpell(caster, 79437, true);
            }

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                    if (caster->HasAura(74434))
                    {
                        inSoulburn = true;
                        caster->RemoveAurasDueToSpell(74434);
                    }
            }

            void Register()
            {
                OnEffectLaunchTarget += SpellEffectFn(spell_item_healthstone_SpellScript::HandleOnHit, EFFECT_0, SPELL_EFFECT_HEAL);
                OnCast += SpellCastFn(spell_item_healthstone_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_healthstone_SpellScript();
        }
};

enum EnhancedSpells
{
    SPELL_ENHANCED_STRENGTH         = 79638,
    SPELL_ENHANCED_AGILITY          = 79639,
    SPELL_ENHANCED_INTELLECT        = 79640
};

class spell_item_flask_of_enhancement : public SpellScriptLoader
{
public:
    spell_item_flask_of_enhancement() : SpellScriptLoader("spell_item_flask_of_enhancement") {}

    class spell_item_flask_of_enhancement_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_item_flask_of_enhancement_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            if(!caster || caster->GetTypeId() != TYPEID_PLAYER)
                return;

            Player* player = caster->ToPlayer();
            // strength
            float stat = player->GetStat(STAT_STRENGTH);
            uint32 trigger_spell_id = SPELL_ENHANCED_STRENGTH;
            // agility
            if (player->GetStat(STAT_AGILITY) > stat)
            {
                trigger_spell_id = SPELL_ENHANCED_AGILITY;
                stat = player->GetStat(STAT_AGILITY);
            }
            // intellect
            if (player->GetStat(STAT_INTELLECT) > stat)
            {
                trigger_spell_id = SPELL_ENHANCED_INTELLECT;
                stat = player->GetStat(STAT_INTELLECT);
            }
            player->CastSpell(player, trigger_spell_id, true);
        }

        void Register()
        {
            OnEffectLaunch += SpellEffectFn(spell_item_flask_of_enhancement_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_item_flask_of_enhancement_SpellScript();
    }
};

enum FoodAmount
{
    FOOD_AMOUNT_60,
    FOOD_AMOUNT_90
};

enum SpellWellFed
{
    SPELL_VERY_SATISFIED            = 84354,

    SPELL_WELL_FED_STRENGTH_60      = 87556,
    SPELL_WELL_FED_AGILITY_60       = 87557,
    SPELL_WELL_FED_INTELLECT_60     = 87558,
    SPELL_WELL_FED_SPIRIT_60        = 87559,
    SPELL_WELL_FED_DODGE_60         = 87564,

    SPELL_WELL_FED_STRENGTH_90      = 87545,
    SPELL_WELL_FED_AGILITY_90       = 87546,
    SPELL_WELL_FED_INTELLECT_90     = 87547,
    SPELL_WELL_FED_SPIRIT_90        = 87548,
    SPELL_WELL_FED_DODGE_90         = 87554
};

enum FoodTriggeringSpell
{
    SPELL_GOBLIN_BARBECUE   = 84352
};

class spell_item_food : public SpellScriptLoader
{
public:
    spell_item_food(char const* scriptName, FoodAmount foodAmount, SpellEffIndex effectIndex) : SpellScriptLoader(scriptName),
        _foodAmount(foodAmount), _effectIndex(effectIndex) { }

    class spell_item_food_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_item_food_AuraScript);

    public:
        spell_item_food_AuraScript(FoodAmount foodAmount, SpellEffIndex effectIndex) : AuraScript(),
            _foodAmount(foodAmount), _effectIndex(effectIndex) {}


        void CastWellFed(AuraEffect const* /*aurEff*/)
        {
            PreventDefaultAction();

            Unit* caster = GetCaster();
            if(!caster || caster->GetTypeId() != TYPEID_PLAYER)
                return;

            Player* player = caster->ToPlayer();
            uint32 trigger_spell_id = 0;

            if (!GetEffect(_effectIndex) || GetEffect(_effectIndex)->GetBaseAmount() == 0)
                return;

            if (GetEffect(_effectIndex) && GetEffect(_effectIndex)->GetBaseAmount() == SPELL_GOBLIN_BARBECUE)
            {
                trigger_spell_id = SPELL_VERY_SATISFIED;
                player->CastSpell(caster, trigger_spell_id, true);
                return;
            }

            switch (player->GetPrimaryTalentTree(player->GetActiveSpec()))
            {
                // Dodge
            case TALENT_TREE_WARRIOR_PROTECTION:
            case TALENT_TREE_PALADIN_PROTECTION:
            case TALENT_TREE_DEATH_KNIGHT_BLOOD:
                trigger_spell_id = _foodAmount == FOOD_AMOUNT_60 ? SPELL_WELL_FED_DODGE_60 : SPELL_WELL_FED_DODGE_90;
                break;

                // Strength
            case TALENT_TREE_WARRIOR_ARMS:
            case TALENT_TREE_WARRIOR_FURY:
            case TALENT_TREE_PALADIN_RETRIBUTION:
            case TALENT_TREE_DEATH_KNIGHT_FROST:
            case TALENT_TREE_DEATH_KNIGHT_UNHOLY:
                trigger_spell_id = _foodAmount == FOOD_AMOUNT_60 ? SPELL_WELL_FED_STRENGTH_60 : SPELL_WELL_FED_STRENGTH_90;
                break;

                // Agility
            case TALENT_TREE_HUNTER_BEAST_MASTERY:
            case TALENT_TREE_HUNTER_MARKSMANSHIP:
            case TALENT_TREE_HUNTER_SURVIVAL:
            case TALENT_TREE_ROGUE_ASSASSINATION:
            case TALENT_TREE_ROGUE_COMBAT:
            case TALENT_TREE_ROGUE_SUBTLETY:
            case TALENT_TREE_SHAMAN_ENHANCEMENT:
            case TALENT_TREE_DRUID_FERAL_COMBAT:
                trigger_spell_id = _foodAmount == FOOD_AMOUNT_60 ? SPELL_WELL_FED_AGILITY_60 : SPELL_WELL_FED_AGILITY_90;
                break;

                // Intellect
            case TALENT_TREE_PRIEST_DISCIPLINE:
            case TALENT_TREE_PRIEST_SHADOW:
            case TALENT_TREE_SHAMAN_ELEMENTAL:
            case TALENT_TREE_MAGE_ARCANE:
            case TALENT_TREE_MAGE_FIRE:
            case TALENT_TREE_MAGE_FROST:
            case TALENT_TREE_WARLOCK_AFFLICTION:
            case TALENT_TREE_WARLOCK_DEMONOLOGY:
            case TALENT_TREE_WARLOCK_DESTRUCTION:
            case TALENT_TREE_DRUID_BALANCE:
            case TALENT_TREE_PALADIN_HOLY:
            case TALENT_TREE_PRIEST_HOLY:
            case TALENT_TREE_SHAMAN_RESTORATION:
            case TALENT_TREE_DRUID_RESTORATION:
                trigger_spell_id = _foodAmount == FOOD_AMOUNT_60 ? SPELL_WELL_FED_INTELLECT_60 : SPELL_WELL_FED_INTELLECT_90;
                break;

            default:
                break;
            }

            player->CastSpell(player, trigger_spell_id, true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_item_food_AuraScript::CastWellFed, _effectIndex, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }

        FoodAmount _foodAmount;
        SpellEffIndex _effectIndex;
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_item_food_AuraScript(_foodAmount, _effectIndex);
    }

private:
    FoodAmount _foodAmount;
    SpellEffIndex _effectIndex;
};

enum SpellFoodRaid
{
    SPELL_FOOD_RAID_FOOD_60     = 80168,
    SPELL_FOOD_RAID_DRINK_60    = 80166,
    SPELL_FOOD_RAID_FOOD_90     = 80169,
    SPELL_FOOD_RAID_DRINK_90    = 80167
};

class spell_item_food_raid : public SpellScriptLoader
{
public:
    spell_item_food_raid(char const* scriptName, FoodAmount foodAmount) : SpellScriptLoader(scriptName),
        _foodAmount(foodAmount) {}

    class spell_item_food_raid_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_item_food_raid_SpellScript);

    public:
        spell_item_food_raid_SpellScript(FoodAmount foodAmount) : SpellScript(),
            _foodAmount(foodAmount) {}

        void HandleDummyFood(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            if(!caster || caster->GetTypeId() != TYPEID_PLAYER)
                return;

            Player* player = caster->ToPlayer();
            player->CastSpell(player, _foodAmount == FOOD_AMOUNT_60 ? SPELL_FOOD_RAID_FOOD_60 : SPELL_FOOD_RAID_FOOD_90, true);
        }

        void HandleDummyDrink(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            if(!caster || caster->GetTypeId() != TYPEID_PLAYER)
                return;

            Player* player = caster->ToPlayer();
            int32 spellId = GetSpellInfo()->Id;
            player->CastCustomSpell(_foodAmount == FOOD_AMOUNT_60 ? SPELL_FOOD_RAID_DRINK_60 : SPELL_FOOD_RAID_DRINK_90, SPELLVALUE_BASE_POINT2, spellId, player, TRIGGERED_FULL_MASK);
        }

        void Register()
        {
            OnEffectLaunch += SpellEffectFn(spell_item_food_raid_SpellScript::HandleDummyFood, EFFECT_1, SPELL_EFFECT_DUMMY);
            OnEffectLaunch += SpellEffectFn(spell_item_food_raid_SpellScript::HandleDummyDrink, EFFECT_2, SPELL_EFFECT_DUMMY);
        }

    private:
        FoodAmount _foodAmount;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_item_food_raid_SpellScript(_foodAmount);
    }

private:
    FoodAmount _foodAmount;
};

class spell_item_jarre_of_ancient_remede : public SpellScriptLoader
{
    public:
        spell_item_jarre_of_ancient_remede() : SpellScriptLoader("spell_item_jarre_of_ancient_remede") { }

        class spell_item_jarre_of_ancient_remede_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_item_jarre_of_ancient_remede_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                return true;
            }

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                if (eventInfo.GetHitMask() & PROC_EX_INTERNAL_DOT)
                    return false;
                return true;
            }

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_item_jarre_of_ancient_remede_AuraScript::CheckProc);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_item_jarre_of_ancient_remede_AuraScript();
        }
};

class spell_item_bird_shot : public SpellScriptLoader
{
    public:
        spell_item_bird_shot() : SpellScriptLoader("spell_item_bird_shot") { }

        class spell_item_bird_shot_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_bird_shot_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                    if (Player *player = caster->ToPlayer())
                        if (Unit* target = GetHitUnit())
                            if (Creature *flameDruid = target->ToCreature())
                                if (flameDruid->GetEntry() == 52661)
                                {
                                    target->CastSpell(target, 98633, true, NULL);
                                    player->KilledMonsterCredit(53251, 0);
                                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_bird_shot_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_bird_shot_SpellScript();
        }
};

class spell_item_tholo_thermometer : public SpellScriptLoader
{
    public:
        spell_item_tholo_thermometer() : SpellScriptLoader("spell_item_tholo_thermometer") { }

        class spell_item_tholo_thermometer_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_tholo_thermometer_SpellScript);

            bool Load()
            {
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                    if (Player *player = caster->ToPlayer())
                    {
                        if (player->FindNearestCreature(53190, 10.0f))
                            player->KilledMonsterCredit(53190);
                        else if (player->FindNearestCreature(53191, 10.0f))
                            player->KilledMonsterCredit(53191);
                        else if (player->FindNearestCreature(53192, 10.0f))
                            player->KilledMonsterCredit(53192);
                    }
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_item_tholo_thermometer_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_tholo_thermometer_SpellScript();
        }
};

// 96947 97130
class spell_item_loom_of_fate : public SpellScriptLoader
{
    public:
        spell_item_loom_of_fate() : SpellScriptLoader("spell_item_loom_of_fate") { }

        class spell_item_loom_of_fate_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_item_loom_of_fate_AuraScript);

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                if (!eventInfo.GetDamageInfo())
                    return false;

                if (Unit* caster = eventInfo.GetActionTarget())
                    if (caster->HealthAbovePct(35) && !caster->HasSpellCooldown(GetId()))
                        if (caster->HealthBelowPctDamaged(35, eventInfo.GetDamageInfo()->GetDamage()))
                            return true;
                return false;
            }

            void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                if (Unit* caster = eventInfo.GetActionTarget())
                {
                    caster->AddSpellCooldown(GetId(), 0, (time(NULL) + 60));
                    caster->CastSpell(caster, GetSpellInfo()->Effects[EFFECT_0].TriggerSpell, true);
                }
            }

            void Register()
            {
                DoCheckProc += AuraCheckProcFn(spell_item_loom_of_fate_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_item_loom_of_fate_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_item_loom_of_fate_AuraScript();
        }
};

// 96887 97119
class spell_item_lightning_capacitor : public SpellScriptLoader
{
public:
    spell_item_lightning_capacitor() : SpellScriptLoader("spell_item_lightning_capacitor") { }

    class spell_item_lightning_capacitor_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_item_lightning_capacitor_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            if (GetTarget()->HasSpellCooldown(GetId()))
                return false;
            return (eventInfo.GetHitMask() & PROC_EX_CRITICAL_HIT);
        }

        void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            if (Unit* caster = GetCaster())
            {
                caster->CastSpell(caster, 96890, true);

                if (Aura* aur = caster->GetAura(96890))
                {
                    if (roll_chance_i(aur->GetStackAmount() * 10))
                    {
                        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(GetId()))
                        {
                            int32 damage = spellInfo->Effects[EFFECT_0].CalcValue(caster);
                            damage *= aur->GetStackAmount();
                            caster->CastCustomSpell(96891, SPELLVALUE_BASE_POINT0, damage, eventInfo.GetActionTarget(), true);
                            aur->Remove(AURA_REMOVE_BY_DEFAULT);
                        }
                    }
                }
                caster->AddSpellCooldown(GetId(), 0, (time(NULL) + 3));
            }
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_item_lightning_capacitor_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_item_lightning_capacitor_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_item_lightning_capacitor_AuraScript();
    }
};

class spell_item_stay_of_execution : public SpellScriptLoader
{
public:
    spell_item_stay_of_execution() : SpellScriptLoader("spell_item_stay_of_execution") { }

    class spell_item_stay_of_execution_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_item_stay_of_execution_AuraScript);

        void Absorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
        {
            uint32 damageToReduce = dmgInfo.GetDamage();
            absorbAmount = CalculatePct(damageToReduce, 20);
        }

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
                caster->CastCustomSpell(96993, SPELLVALUE_BASE_POINT0, CalculatePct(aurEff->GetBaseAmount() - aurEff->GetAmount(), 8), caster, true);
        }

        void Register() override
        {
            OnEffectAbsorb += AuraEffectAbsorbFn(spell_item_stay_of_execution_AuraScript::Absorb, EFFECT_0);
            OnEffectRemove += AuraEffectRemoveFn(spell_item_stay_of_execution_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_item_stay_of_execution_AuraScript();
    }
};

// 96976 97138
class spell_item_matrix_restabilizer : public SpellScriptLoader
{
public:
    spell_item_matrix_restabilizer() : SpellScriptLoader("spell_item_matrix_restabilizer") { }

    class spell_item_matrix_restabilizer_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_item_matrix_restabilizer_AuraScript);

        void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
        {
            if (Player* player = GetTarget()->ToPlayer())
            {
                if (player->HasSpellCooldown(GetId()))
                    return;
                uint32 spellId[2][3] = { { 96977, 96978, 96979 }, { 97139, 97140, 97141 } };
                uint32 maxstat = 0;
                uint32 maxvalue = 0;
                for (uint8 i = 0; i < 3; i++)
                {
                    if (SpellInfo const* spell = sSpellMgr->GetSpellInfo(spellId[0][i]))
                        for (uint32 rating = 0; rating < MAX_COMBAT_RATING; ++rating)
                            if (spell->Effects[EFFECT_0].MiscValue & (1 << rating))
                            {
                                uint32 temp = player->GetCombatRating(CombatRating(rating));
                                if (temp > maxvalue)
                                {
                                    maxstat = rating;
                                    maxvalue = temp;
                                }
                            }
                }

                switch (maxstat)
                {
                    case CR_HASTE_RANGED:
                    case CR_HASTE_SPELL:
                    case CR_HASTE_MELEE:
                        player->CastSpell(player, GetId() == 96976 ? 96977 : 97139, true);
                        break;
                    case CR_CRIT_RANGED:
                    case CR_CRIT_SPELL:
                    case CR_CRIT_MELEE:
                        player->CastSpell(player, GetId() == 96976 ? 96978 : 97140, true);
                        break;
                    default:
                        player->CastSpell(player, GetId() == 96976 ? 96979 : 97141, true);
                        break;
                }
                player->AddSpellCooldown(GetId(), 0, (time(NULL) + 105));
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_item_matrix_restabilizer_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_item_matrix_restabilizer_AuraScript();
    }
};

enum Tarecgosa
{
    SPELL_WRATH_OF_TERECGOSA_PROC = 101085,
};

class spell_wrath_of_tarecgosa : public SpellScriptLoader
{
public:
    spell_wrath_of_tarecgosa() : SpellScriptLoader("spell_wrath_of_tarecgosa") { }

    class spell_wrath_of_tarecgosa_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_wrath_of_tarecgosa_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            return eventInfo.GetDamageInfo()->GetDamage() > 0;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            if (Unit* caster = eventInfo.GetActor())
            {
                if (Unit* target = eventInfo.GetActionTarget())
                {
                    int32 damage = eventInfo.GetDamageInfo()->GetDamage();
                    caster->CastCustomSpell(target, SPELL_WRATH_OF_TERECGOSA_PROC, &damage, NULL, NULL, TRIGGERED_FULL_MASK, NULL, aurEff);
                }
            }
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_wrath_of_tarecgosa_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_wrath_of_tarecgosa_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL_COPY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_wrath_of_tarecgosa_AuraScript();
    }
};

class spell_nick_of_time : public SpellScriptLoader
{
public:
    spell_nick_of_time() : SpellScriptLoader("spell_nick_of_time") { }

    class spell_nick_of_time_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nick_of_time_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (!targets.empty())
            {
                std::list<Unit*> temp;
                for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end(); itr++)
                    if (Unit* unit = (*itr)->ToUnit())
                        temp.push_back(unit);

                targets.clear();
                temp.sort(Trinity::HealthPctOrderPred());
                if (temp.size() > 1)
                    temp.resize(1);
                for (std::list<Unit*>::iterator itr = temp.begin(); itr != temp.end(); itr++)
                    targets.push_back((WorldObject*)(*itr));
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_nick_of_time_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_nick_of_time_SpellScript();
    }
};

class spell_item_indomitable : public SpellScriptLoader
{
public:
    spell_item_indomitable() : SpellScriptLoader("spell_item_indomitable") { }

    class spell_item_indomitable_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_item_indomitable_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            if (Unit* actionTarget = eventInfo.GetActionTarget())
            {
                if (!eventInfo.GetDamageInfo())
                    return false;

                if (!actionTarget->HealthBelowPctDamaged(50, eventInfo.GetDamageInfo()->GetDamage()))
                    return false;

                return !actionTarget->HasSpellCooldown(GetId());
            }
            return false;
        }

        void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
        {
            if (Unit* actionTarget = eventInfo.GetActionTarget())
            {
                if (AuraEffect* aurEff = GetAura()->GetEffect(EFFECT_1))
                {
                    int32 absorb = CalculatePct(eventInfo.GetDamageInfo()->GetDamage(), aurEff->GetAmount());
                    actionTarget->CastCustomSpell(actionTarget, 108008, &absorb, NULL, NULL, true);
                    actionTarget->AddSpellCooldown(GetId(), 0, (time(NULL) + 60));
                }
            }
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_item_indomitable_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_item_indomitable_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_item_indomitable_AuraScript();
    }
};

class spell_item_cleansing_flames : public SpellScriptLoader
{
public:
    spell_item_cleansing_flames() : SpellScriptLoader("spell_item_cleansing_flames") { }

    class spell_item_cleansing_flames_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_item_cleansing_flames_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove(GetCaster());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_item_cleansing_flames_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ALLY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_item_cleansing_flames_SpellScript();
    }
};

enum LightningStrikeSpells
{
    SPELL_LIGHTNING_STRIKE_LFR  = 109721,
    SPELL_LIGHTNING_STRIKE_NH   = 107994,
    SPELL_LIGHTNING_STRIKE_HC   = 109724,
};

class spell_item_lightning_strike : public SpellScriptLoader
{
public:
    spell_item_lightning_strike() : SpellScriptLoader("spell_item_lightning_strike") { }

    class spell_item_lightning_strike_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_item_lightning_strike_SpellScript);

        void HandleHit(SpellEffIndex /*effIndex*/)
        {
            float apbonus = 0.0f;
            switch (GetSpellInfo()->Id)
            {
                case SPELL_LIGHTNING_STRIKE_LFR:
                    apbonus = 0.266f;
                    break;
                case SPELL_LIGHTNING_STRIKE_NH:
                    apbonus = 0.3f;
                    break;
                case SPELL_LIGHTNING_STRIKE_HC:
                    apbonus = 0.339f;
                    break;
                default:
                    break;
            }

            int32 baseDamage = GetEffectValue() + (apbonus * GetCaster()->GetTotalAttackPowerValue(GetCaster()->getClass() == CLASS_HUNTER ? RANGED_ATTACK : BASE_ATTACK));
            SetEffectDamage(baseDamage);
        }

        void Register()
        {
            OnEffectLaunchTarget += SpellEffectFn(spell_item_lightning_strike_SpellScript::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_item_lightning_strike_SpellScript();
    }
};

enum TransformSpells
{
    SPELL_NIGHTELF_FEMALE   = 74590,
    SPELL_BLOODELF_MALE     = 74591,
    SPELL_GNOME_MALE        = 74592,
    SPELL_GOBLIN_MALE       = 74593,
    SPELL_HUMAN_FEMALE      = 74594,
    SPELL_GOBLIN_MALE_2     = 74595,
    SPELL_UNDEAD_FEMALE     = 74596,
    SPELL_UNDEAD_MALE       = 74597,
    SPELL_GOBLIN_FEMALE     = 74598,
    SPELL_BLOODELF_MALE_2   = 74599,
    SPELL_TROLL_FEMALE      = 74600,
    SPELL_BLOODELF_FEMALE   = 74601,
    SPELL_GOBLIN_MALE_3     = 74602,
    SPELL_ORC_MALE          = 74603,
    SPELL_GOBLIN_FEMALE_2   = 74604,
    ITEM_FADED_WIZARD_HAT   = 53057
};

class spell_item_faded_wizard_hat : public SpellScriptLoader
{
public:
    spell_item_faded_wizard_hat() : SpellScriptLoader("spell_item_faded_wizard_hat") { }

    class spell_item_faded_wizard_hat_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_item_faded_wizard_hat_AuraScript);

        void AfterApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(urand(SPELL_NIGHTELF_FEMALE, SPELL_GOBLIN_FEMALE_2)))
                if (CreatureTemplate* cInfo = const_cast<CreatureTemplate*>(sObjectMgr->GetCreatureTemplate(spellInfo->Effects[EFFECT_0].MiscValue)))
                    GetTarget()->SetDisplayId(cInfo->Modelid1);
        }

        void CalcPeriodic(AuraEffect const* /*effect*/, bool& isPeriodic, int32& amplitude)
        {
            isPeriodic = true;
            amplitude = 1000;
        }

        void OnPeriodicTick(AuraEffect const* /*aurEff*/)
        {
            if (Player* player = GetTarget()->ToPlayer())
            {
                Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD);
                if (!item || item->GetEntry() != ITEM_FADED_WIZARD_HAT)
                    Remove(AURA_REMOVE_BY_CANCEL);
            }
        }

        void Register()
        {
            AfterEffectApply += AuraEffectRemoveFn(spell_item_faded_wizard_hat_AuraScript::AfterApply, EFFECT_0, SPELL_AURA_TRANSFORM, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_item_faded_wizard_hat_AuraScript::CalcPeriodic, EFFECT_0, SPELL_AURA_TRANSFORM);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_item_faded_wizard_hat_AuraScript::OnPeriodicTick, EFFECT_0, SPELL_AURA_TRANSFORM);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_item_faded_wizard_hat_AuraScript;
    }
};

#define SPELL_MARLS_SHIELD    78986

class spell_item_break_marls_trance : public SpellScriptLoader
{
public:
    spell_item_break_marls_trance() : SpellScriptLoader("spell_item_break_marls_trance") { }

    class spell_item_break_marls_trance_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_item_break_marls_trance_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (GetHitUnit()->GetTypeId() != TYPEID_UNIT)
                return;

            GetHitUnit()->RemoveAurasDueToSpell(SPELL_MARLS_SHIELD);
            GetHitUnit()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC);
            GetHitUnit()->ToCreature()->AI()->AttackStart(GetCaster());
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_item_break_marls_trance_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_item_break_marls_trance_SpellScript();
    }
};

const Position dynamitePosition[32] =
{
    (413.171f, -267.326f, 66.7613f, 4.49720f),
    (413.179f, -266.781f, 66.5540f, 4.32441f),
    (413.459f, -266.095f, 66.3264f, 4.32441f),
    (413.276f, -265.509f, 65.9180f, 3.09919f),
    (413.828f, -266.527f, 66.4776f, 2.92528f),
    (413.863f, -267.176f, 66.7896f, 2.77998f),
    (414.033f, -265.530f, 66.0355f, 3.83627f),
    (414.365f, -266.202f, 66.4011f, 2.88515f),

    (446.408f, -209.571f, 59.9053f, 4.08524f),
    (445.823f, -208.994f, 60.0038f, 4.08524f),
    (445.160f, -208.514f, 60.1231f, 4.08524f),
    (444.541f, -207.936f, 60.1957f, 5.75370f),
    (446.435f, -208.796f, 59.8646f, 4.15542f),
    (445.740f, -208.364f, 59.9708f, 4.15542f),
    (445.145f, -207.993f, 60.0616f, 4.15542f),
    (446.861f, -209.329f, 59.8159f, 2.51630f),

    (599.121f, -151.207f, 21.3293f, 6.12925f),
    (599.479f, -151.861f, 21.3359f, 0.31730f),
    (599.922f, -152.703f, 21.3559f, 0.63146f),
    (601.133f, -153.297f, 21.2469f, 1.86139f),
    (602.608f, -152.630f, 21.0355f, 2.17555f),
    (600.573f, -148.680f, 20.9707f, 5.23782f),
    (599.115f, -149.521f, 21.2019f, 5.23782f),
    (598.203f, -150.478f, 21.3618f, 0.16100f),

    (551.036f, -137.346f, 24.8785f, 2.22896f),
    (550.328f, -138.340f, 24.9151f, 2.22896f),
    (550.804f, -138.153f, 24.8594f, 2.54312f),
    (549.978f, -138.513f, 25.0311f, 2.22896f),
    (548.365f, -138.573f, 25.2200f, 0.76340f),
    (549.349f, -139.318f, 24.9276f, 1.40586f),
    (551.737f, -137.621f, 24.7567f, 2.84393f),
    (551.942f, -136.951f, 24.7602f, 3.26176f)
};

enum dynamiteBundle
{
    GOB_DYNAMITE_BUNDLE     = 203236,
    NPC_DYNAMITE_TRIGGER_1  = 41314,
    NPC_DYNAMITE_TRIGGER_2  = 41315,
    NPC_DYNAMITE_TRIGGER_3  = 41316,
    NPC_DYNAMITE_TRIGGER_4  = 41317
};

class spell_item_planting_dynamite_bundle : public SpellScriptLoader
{
public:
    spell_item_planting_dynamite_bundle() : SpellScriptLoader("spell_item_planting_dynamite_bundle") { }

    class spell_item_planting_dynamite_bundle_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_item_planting_dynamite_bundle_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (GetHitUnit()->GetTypeId() != TYPEID_UNIT || GetCaster()->GetTypeId() != TYPEID_PLAYER)
                return;

            GetCaster()->ToPlayer()->KilledMonsterCredit(GetHitUnit()->GetEntry(), GetHitUnit()->GetGUID());
            // This part doesn't work... I dont know why... The dynamite (just a visual) doesn't spawn...
            //int8 min = 0;
            //int8 max = 0;
            //switch (GetHitUnit()->GetEntry())
            //{
            //    case NPC_DYNAMITE_TRIGGER_1:
            //        min = 0;
            //        max = 7;
            //        break;
            //    case NPC_DYNAMITE_TRIGGER_2:
            //        min = 8;
            //        max = 15;
            //        break;
            //    case NPC_DYNAMITE_TRIGGER_3:
            //        min = 16;
            //        max = 23;
            //        break;
            //    case NPC_DYNAMITE_TRIGGER_4:
            //        min = 24;
            //        max = 31;
            //        break;
            //}
            //for (uint8 i = min; i < max; i++)
            //    if (GameObject* go = GetCaster()->SummonGameObject(GOB_DYNAMITE_BUNDLE, dynamitePosition[i].GetPositionX(), dynamitePosition[i].GetPositionY(), dynamitePosition[i].GetPositionZ(), dynamitePosition[i].GetOrientation(), 0, 0, 0, 0, 1000))
            //        go->Delete(20000);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_item_planting_dynamite_bundle_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_item_planting_dynamite_bundle_SpellScript();
    }
};

// Crazy Alchemist's Potion
enum CrazyAlchemistsPotion
{
	SPELL_SPEED = 53908,
	SPELL_WILD_MAGIC_POTION = 53909,
	SPELL_INDESTRUCTIBLE = 53762,
	SPELL_RESTORE_MANA = 43186,
	SPELL_HEALING_POTION = 43185,
	SPELL_FIRE_PROTECTION = 53911,
};

class spell_item_crazy_alchemists_potion : public SpellScriptLoader
{
public:
	spell_item_crazy_alchemists_potion() : SpellScriptLoader("spell_item_crazy_alchemists_potion") { }

	class spell_item_crazy_alchemists_potion_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_item_crazy_alchemists_potion_SpellScript);

		void HandleHeal(SpellEffIndex /*effIndex*/)
		{
			if (Unit* caster = GetOriginalCaster())
			{
				uint32 spellId = 0;
				uint32 roll = urand(0, 9);
				if (roll > 5)
					return;

				switch (roll)
				{
				case 0: spellId = SPELL_SPEED; break;
				case 1: spellId = SPELL_WILD_MAGIC_POTION; break;
				case 2: spellId = SPELL_INDESTRUCTIBLE; break;
				case 3: spellId = SPELL_RESTORE_MANA; break;
				case 4: spellId = SPELL_HEALING_POTION; break;
				case 5: spellId = SPELL_FIRE_PROTECTION; break;
				default: break;
				}

				if (spellId)
					caster->AddAura(spellId, caster);
			}
		}

		void Register()
		{
			OnEffectHitTarget += SpellEffectFn(spell_item_crazy_alchemists_potion_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_item_crazy_alchemists_potion_SpellScript();
	}
};


class spell_item_flameseers_staff_flamebreaker : public SpellScriptLoader
{
public:
	spell_item_flameseers_staff_flamebreaker() : SpellScriptLoader("spell_item_flameseers_staff_flamebreaker") { }

	class spell_item_flameseers_staff_flamebreaker_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_item_flameseers_staff_flamebreaker_AuraScript);

		void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
		{
			if (!GetCaster() || !GetTarget())
				return;

			if (GetTarget()->GetEntry() == 38896)
			{
				GetCaster()->CastSpell(GetTarget(), 74723, true);
				GetTarget()->Kill(GetTarget());
			}
		}

		void Register()
		{
			OnEffectRemove += AuraEffectRemoveFn(spell_item_flameseers_staff_flamebreaker_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_item_flameseers_staff_flamebreaker_AuraScript();
	}
};

class spell_item_flameseers_staff_weakening : public SpellScriptLoader
{
public:
	spell_item_flameseers_staff_weakening() : SpellScriptLoader("spell_item_flameseers_staff_weakening") { }

	class spell_item_flameseers_staff_weakening_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_item_flameseers_staff_weakening_AuraScript);

		void HandlePeriodic(AuraEffect const* aurEff)
		{
			if (aurEff->GetBase()->GetOwner()->GetEntry() == 40147)
				if (GetCaster()->GetTypeId() == TYPEID_PLAYER)
				{
					uint8 stacks = GetAura()->GetStackAmount();
					GetAura()->SetStackAmount(stacks + 1);
					if (stacks >= 19)
						GetCaster()->ToPlayer()->KilledMonsterCredit(40334, 0);
				}
		}


		void Register()
		{
			OnEffectPeriodic += AuraEffectPeriodicFn(spell_item_flameseers_staff_weakening_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_item_flameseers_staff_weakening_AuraScript();
	}
};

void AddSC_item_spell_scripts()
{
    // 23074 Arcanite Dragonling
    new spell_item_trigger_spell("spell_item_arcanite_dragonling", SPELL_ARCANITE_DRAGONLING); // Verified
    // 23133 Gnomish Battle Chicken
	new spell_item_trigger_spell("spell_item_gnomish_battle_chicken", SPELL_BATTLE_CHICKEN); // Verified
    // 23076 Mechanical Dragonling
	new spell_item_trigger_spell("spell_item_mechanical_dragonling", SPELL_MECHANICAL_DRAGONLING); // Verified
    // 23075 Mithril Mechanical Dragonling
	new spell_item_trigger_spell("spell_item_mithril_mechanical_dragonling", SPELL_MITHRIL_MECHANICAL_DRAGONLING); // Verified

    new spell_item_arcane_shroud();// Verified
    new spell_item_blessing_of_ancient_kings();// Verified
    new spell_item_defibrillate("spell_item_goblin_jumper_cables", 67, SPELL_GOBLIN_JUMPER_CABLES_FAIL); // Verified
    new spell_item_defibrillate("spell_item_goblin_jumper_cables_xl", 50, SPELL_GOBLIN_JUMPER_CABLES_XL_FAIL); // Verified
    new spell_item_defibrillate("spell_item_gnomish_army_knife", 33); // Verified
    new spell_item_deviate_fish(); // Verified
    new spell_item_flask_of_the_north(); // Verified
    new spell_item_gnomish_death_ray(); // Verified
    new spell_item_make_a_wish(); // Verified
	new spell_item_mingos_fortune_generator(); // Verified
	new spell_item_necrotic_touch(); // Verified
	new spell_item_net_o_matic(); // Verified
	new spell_item_noggenfogger_elixir(); // Verified
	new spell_item_piccolo_of_the_flaming_fire(); // Verified
	new spell_item_savory_deviate_delight(); // Verified
	new spell_item_scroll_of_recall(); // Verified
	new spell_item_shadows_fate(); // Verified
	new spell_item_shadowmourne(); // Verified
	new spell_item_shadowmourne_soul_fragment(); // Verified
	new spell_item_six_demon_bag(); // Verified
	new spell_item_the_eye_of_diminution(); // Verified
	new spell_item_underbelly_elixir(); // Verified
	new spell_item_red_rider_air_rifle(); // Verified

    new spell_item_create_heart_candy(); // Verified
    new spell_item_book_of_discovery("spell_item_book_of_glyph_mastery"); // Verified
    new spell_item_book_of_discovery("spell_item_tome_of_discovery"); // Recently Verified
	new spell_item_gift_of_the_harvester(); // Verified
	new spell_item_map_of_the_geyser_fields(); // Verified
	new spell_item_vanquished_clutches(); // Verified

	new spell_item_ashbringer(); // Verified
	new spell_magic_eater_food(); // Verified
	new spell_item_shimmering_vessel(); // Verified
	new spell_item_purify_helboar_meat(); // Verified
	new spell_item_crystal_prison_dummy_dnd(); // Verified
	new spell_item_reindeer_transformation(); // Verified
	new spell_item_nigh_invulnerability(); // Verified
	new spell_item_poultryizer(); // Verified
	new spell_item_socrethars_stone(); // Verified
	new spell_item_demon_broiled_surprise(); // Verified
	new spell_item_complete_raptor_capture(); // Verified
	new spell_item_impale_leviroth(); // Verified
	new spell_item_brewfest_mount_transformation(); // Verified
	new spell_item_nitro_boots(); // Verified
	new spell_item_teach_language(); // Verified
	new spell_item_rocket_boots(); // Verified
	new spell_item_pygmy_oil(); // Verified
	new spell_item_unusual_compass(); // Verified
	new spell_item_chicken_cover(); // Verified
	new spell_item_muisek_vessel(); // Verified 
    new spell_item_greatmothers_soulcatcher(); // Verified
	new spell_item_healthstone(); // Verified
	new spell_item_flask_of_enhancement(); // Verified
    new spell_item_food("Food_60_2", FOOD_AMOUNT_60, EFFECT_2); // Possible Recently Verified
    new spell_item_food("Food_90_1", FOOD_AMOUNT_90, EFFECT_1); // Possible Recently Verified
    new spell_item_food("Food_90_2", FOOD_AMOUNT_90, EFFECT_2); // Possible Recently Verified
    new spell_item_food_raid("Food_Raid_60", FOOD_AMOUNT_60); // Possible Recently Verified 
    new spell_item_food_raid("Food_Raid_90", FOOD_AMOUNT_90); // Possible Recently Verified
    new spell_item_jarre_of_ancient_remede(); // Recently Verified, BUT really unsure if it's correct spell (ids)??
    new spell_item_bird_shot(); // Recently Verified, most likely correct spell id
    new spell_item_tholo_thermometer(); // Recently Verified

    new spell_item_loom_of_fate(); // Recently Verified
    new spell_item_lightning_capacitor(); // Recently Verified
    new spell_item_stay_of_execution(); // Verified
    new spell_item_matrix_restabilizer(); // Recently Verified
    new spell_wrath_of_tarecgosa(); // Verified
    new spell_nick_of_time(); // Recently Verified
    new spell_item_indomitable(); // Recently Verified
    new spell_item_cleansing_flames(); // Recently Verified, I think it's the correct spell id
    new spell_item_lightning_strike(); // Recently Verified, might be the correct spell id
    new spell_item_faded_wizard_hat(); // Recently Verified
    // 57185 - Demoniac Vessel
    new spell_item_break_marls_trance(); // Verified
    // 56018 - Dynamite Bundle
    new spell_item_planting_dynamite_bundle(); // Verified
	new spell_item_crazy_alchemists_potion(); // Verified

	new spell_item_flameseers_staff_flamebreaker(); // Verified
 	new spell_item_flameseers_staff_weakening(); // Verified
}
