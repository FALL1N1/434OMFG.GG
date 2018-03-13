/*
 * Copyright (C) 2013-2014 OMFG.GG Network <http://www.omfg.gg/>
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

#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"
#include "bastion_of_twilight.h"

class spell_blade_barrier : public SpellScriptLoader
{
public:
    spell_blade_barrier() : SpellScriptLoader("spell_blade_barrier") { }

    class spell_blade_barrier_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_blade_barrier_AuraScript);

        void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
        {
            // Set absorbtion amount to unlimited
            amount = -1;
        }

        void Absorb(AuraEffect* /* aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
        {
            uint32 damage = dmgInfo.GetDamage();
            if (damage <= 10000)
                absorbAmount = damage - 1;
        }

        void Register() override
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_blade_barrier_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            OnEffectAbsorb += AuraEffectAbsorbFn(spell_blade_barrier_AuraScript::Absorb, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_blade_barrier_AuraScript();
    }
};

void AddSC_bastion_of_twilight()
{
    new spell_blade_barrier(); // Recently Verified
}
