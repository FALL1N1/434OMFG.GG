
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "shadowfang_keep.h"

enum Spells
{
    SPELL_SUMMON_WORGEN_SPIRIT      = 93857,
    SPELL_VEIL_OF_SHADOW            = 23224,
    SPELL_CURSED_VEIL_H             = 93956,
};

enum Events
{
    EVENT_VEIL_OF_SHADOW            = 1,
    EVENT_CURSED_VEIL
};

enum Texts
{
    SAY_AGGRO,
    SAY_DEATH,
    SAY_KILLER_1,
    SAY_KILLER_2
};

enum Action
{
    ACTION_SUMMON                   = 1,
};

enum Summons
{
    RETHILGORE                      = 50835,
    ODO_THE_BLINDWATCHER            = 50857,
    LUPINE_SPECTRE                  = 50923,
    RAZORCLAW_THE_BUTCHER           = 50869,
    WOLF_MASTER_NANDOS              = 50851,
};

uint32 spellIds [4]     = { 93859, 93896, 93921, 93925};

class boss_baron_silverlaine : public CreatureScript
{
    public:
        boss_baron_silverlaine() : CreatureScript("boss_baron_silverlaine") { }

        struct boss_baron_silverlaineAI : public BossAI
        {
            boss_baron_silverlaineAI(Creature* creature) : BossAI(creature, DATA_BARON_SILVERLAINE_EVENT) { }

            void Reset()
            {
                _Reset();
                isFirst = false;
                isSecond = false;
                isThird = false;
                spellSet.clear();
                for (uint8 i=0; i < 4; ++i)
                    spellSet.insert(spellIds[i]);
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(SAY_AGGRO);
                events.ScheduleEvent(DUNGEON_MODE(EVENT_VEIL_OF_SHADOW, EVENT_CURSED_VEIL), 10000);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
            }

            void JustSummoned(Creature* summon) 
            {
                BossAI::JustSummoned(summon);
                if (summon->GetEntry() == 50934)
                    summon->AI()->DoCast(93860);
                DoZoneInCombat(summon);
            }

            void KilledUnit(Unit* victim)
            {
                Talk(RAND(SAY_KILLER_1, SAY_KILLER_2));
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_SUMMON)
                {
                    if (spellSet.empty())
                        return;

                    uint32 summonspell = Trinity::Containers::SelectRandomContainerElement(spellSet);
                    DoCast(summonspell);
                    spellSet.erase(summonspell);
                }
            }

            void DamageTaken(Unit* /*dealer*/, uint32& /*damage*/)
            {
                if (IsHeroic())
                {
                    if (HealthBelowPct(90) && !isFirst)
                    {
                        isFirst = true;
                        DoCast(SPELL_SUMMON_WORGEN_SPIRIT);
                    }
                    else if (HealthBelowPct(60) && !isSecond)
                    {
                        isSecond = true;
                        DoCast(SPELL_SUMMON_WORGEN_SPIRIT);
                    }
                    else if (HealthBelowPct(30) && !isThird)
                    {
                        isThird = true;
                        DoCast(SPELL_SUMMON_WORGEN_SPIRIT);
                    }
                }
                else if (HealthBelowPct(60) && !isFirst)
                {
                    isFirst = true;
                    DoCast(SPELL_SUMMON_WORGEN_SPIRIT);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                    case EVENT_VEIL_OF_SHADOW:
                        DoCast(SPELL_VEIL_OF_SHADOW);
                        events.ScheduleEvent(EVENT_VEIL_OF_SHADOW, 10000);
                        break;
                    case EVENT_CURSED_VEIL:
                        DoCast(SPELL_CURSED_VEIL_H);
                        events.ScheduleEvent(EVENT_CURSED_VEIL, urand(20000, 40000));
                        break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        private:
            std::set<uint32> spellSet;
            bool isFirst;
            bool isSecond;
            bool isThird;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_baron_silverlaineAI(creature);
        }
};

class spell_summon_worgen_spirits : public SpellScriptLoader
{
public:
    spell_summon_worgen_spirits() : SpellScriptLoader("spell_summon_worgen_spirits") { }

    class spell_summon_worgen_spirits_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_summon_worgen_spirits_SpellScript);

        bool Load()
        {
            _instance = GetCaster()->GetInstanceScript();
            return _instance != NULL;
        }
        
        void AfterCast(SpellEffIndex /*effIndex*/)
        {
            if (Creature* silverlaine = ObjectAccessor::GetCreature(*GetCaster(), _instance->GetData64(DATA_BARON_SILVERLAINE)))
                silverlaine->AI()->DoAction(ACTION_SUMMON);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_summon_worgen_spirits_SpellScript::AfterCast, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }

        InstanceScript* _instance;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_summon_worgen_spirits_SpellScript();
    }
};

void AddSC_boss_baron_silverlaine()
{
    new boss_baron_silverlaine();
    new spell_summon_worgen_spirits();
}