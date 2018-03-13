
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "blackwing_descent.h"

class spell_grievous_wound_bwd : public SpellScriptLoader
{
public:
    spell_grievous_wound_bwd() : SpellScriptLoader("spell_grievous_wound_bwd") { }

    class spell_grievous_wound_bwd_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_grievous_wound_bwd_AuraScript);

        void onPeriodicTick(AuraEffect const* /*aurEff*/)
        {
            if (Unit* target = GetUnitOwner())
                if (target->CountPctFromMaxHealth(90) < target->GetHealth())
                    target->RemoveAurasDueToSpell(GetSpellInfo()->Id);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_grievous_wound_bwd_AuraScript::onPeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_grievous_wound_bwd_AuraScript();
    }
};

void AddSC_blackwing_descent()
{
    new spell_grievous_wound_bwd();
};
