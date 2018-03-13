/*
* ToDo:
* Find the correct Sound Texts from DBC's
* Find a better Way for his Archangel Cast
*/

#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "shadowfang_keep.h"

enum Spells
{
    // Normal Mode
    SPELL_ASPHYXIATE                       = 93423,
    SPELL_PAIN_AND_SUFFERING               = 93581,
    SPELL_PAIN_AND_SUFFERING_HC            = 93712,
    SPELL_STAY_OF_EXECUTION                = 93468,
    SPELL_STAY_OF_EXECUTION_HC             = 93705,

    // Heroic Mode
    SPELL_DARK_ARCHANGEL_FORM              = 93757,
    SPELL_CALAMITY                         = 93812,
    SPELL_WRACKING_PAIN                    = 93720
};

enum Events
{
    EVENT_ASPHYXIATE                       = 1,
    EVENT_PAIN_AND_SUFFERING,
    EVENT_STAY_OF_EXECUTION,
    EVENT_WRACKING_PAIN,
    EVENT_CONTINUE_COMBAT
};

enum Texts
{
    SAY_AGGRO,
    SAY_DEATH,
    SAY_KILLER_1,
    SAY_KILLER_2,
    SAY_ARCHANGEL,
    SAY_STAYEXE,
    SAY_ASPHY
};

class boss_baron_ashbury : public CreatureScript
{
public:
    boss_baron_ashbury() : CreatureScript("boss_baron_ashbury") { }

    struct boss_baron_ashburyAI : public BossAI
    {
        boss_baron_ashburyAI(Creature* creature) : BossAI(creature, DATA_BARON_ASHBURY_EVENT) { }

        void Reset()
        {
            _Reset();
            me->SetReactState(REACT_AGGRESSIVE);
            enraged = false;
            me->RemoveAllAuras();
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WRACKING_PAIN);
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            Talk(SAY_AGGRO);

            events.ScheduleEvent(EVENT_ASPHYXIATE, 18000);
            events.ScheduleEvent(EVENT_PAIN_AND_SUFFERING, 10000);
            events.ScheduleEvent(EVENT_WRACKING_PAIN, 60000);
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            Talk(SAY_DEATH);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_DARK_ARCHANGEL_FORM);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WRACKING_PAIN);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CALAMITY);
        }

        void KilledUnit(Unit* victim)
        {
            Talk(RAND(SAY_KILLER_1, SAY_KILLER_2));
        }

        void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/)
        {
            if (IsHeroic())
                if(!enraged && HealthBelowPct(20))
                {
                    enraged = true;
                    Talk(SAY_ARCHANGEL);
                    DoCast(me, SPELL_DARK_ARCHANGEL_FORM);
                }
        }

        void SpellHit(Unit* /*caster*/, const SpellInfo* spell)
        {
            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                if (spell->Effects[i].Effect == SPELL_EFFECT_INTERRUPT_CAST || spell->Effects[i].Mechanic == MECHANIC_INTERRUPT)
                    me->SetReactState(REACT_AGGRESSIVE);
        }

        void UpdateAI(uint32 const diff)
        {
            if(!UpdateVictim())
                return;

            events.Update(diff);

            if(me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if(uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                case EVENT_ASPHYXIATE:
                    Talk(SAY_ASPHY);
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    DoCast(SPELL_ASPHYXIATE);
                    events.ScheduleEvent(EVENT_STAY_OF_EXECUTION, 6500);
                    events.ScheduleEvent(EVENT_ASPHYXIATE, urand(40000, 45000));
                    break;
                case EVENT_PAIN_AND_SUFFERING:
                    DoCastRandom(SPELL_PAIN_AND_SUFFERING, 50, true);
                    events.ScheduleEvent(EVENT_PAIN_AND_SUFFERING, 30000);
                    break;
                case EVENT_STAY_OF_EXECUTION:
                    Talk(SAY_STAYEXE);
                    DoCast(SPELL_STAY_OF_EXECUTION);
                    events.ScheduleEvent(EVENT_CONTINUE_COMBAT, 8500);
                    break;
                case EVENT_WRACKING_PAIN:
                    DoCast(SPELL_WRACKING_PAIN);
                    events.ScheduleEvent(EVENT_WRACKING_PAIN, 45000);
                    break;
                case EVENT_CONTINUE_COMBAT:
                    me->SetReactState(REACT_AGGRESSIVE);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        bool enraged;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_baron_ashburyAI(creature);
    }
};

class spell_ashbury_archangel : public SpellScriptLoader
{
public:
    spell_ashbury_archangel() : SpellScriptLoader("spell_ashbury_archangel") { }

    class spell_ashbury_archangel_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ashbury_archangel_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            if (Unit* target = GetHitUnit())
            {
                target->CastSpell(target, uint32(GetEffectValue()), true);
                target->CastSpell(target, SPELL_CALAMITY, true);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_ashbury_archangel_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_ashbury_archangel_SpellScript();
    }
};

class spell_ashbury_asphyxiate : public SpellScriptLoader
{
public:
    spell_ashbury_asphyxiate() : SpellScriptLoader("spell_ashbury_asphyxoate") { }

    class spell_ashbury_asphyxiate_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ashbury_asphyxiate_SpellScript);

        void HandleOnHit()
        {
            if (Unit* target = GetHitUnit())
            {
                maxhp = target->GetMaxHealth() / 6;
                if (target->GetHealth() > maxhp)
                    SetHitDamage(maxhp);
                else
                    SetHitDamage(target->GetHealth() - 1);
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_ashbury_asphyxiate_SpellScript::HandleOnHit);
        }
    private:
        uint32 maxhp;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_ashbury_asphyxiate_SpellScript();
    }
};

void AddSC_boss_baron_ashbury()
{
    new boss_baron_ashbury();
    new spell_ashbury_archangel();
    new spell_ashbury_asphyxiate();
}
