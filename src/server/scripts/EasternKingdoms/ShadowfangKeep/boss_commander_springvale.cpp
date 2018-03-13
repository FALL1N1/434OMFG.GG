
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "shadowfang_keep.h"

enum Spells
{
    SPELL_DESECRATION                      = 93687,
    SPELL_DESECRATION_TRIGGERED            = 93690,

    SPELL_MALEFIX_STRIKE                   = 93685,
    SPELL_SHIELD_OF_THE_PERFIDIOUS         = 93693,

    // heroic
    SPELL_SEPARATION_ANXIETY               = 96272,
    SPELL_WORD_OF_SHAME                    = 93852,
    SPELL_UNHOLY_EMPOWERMENT               = 93844
};

enum Events
{
    EVENT_DESECRATION                      = 1,
    EVENT_MALEFIX_STRIKE,
    EVENT_SUMMONS
};

enum Actions
{
    ACTION_RANDOM_CAST                     = 1
};

enum Yells
{
    SAY_AGGRO,
    SAY_SUMMON_GUARDS,
    SAY_KILL_1,
    SAY_KILL_2,
    SAY_DEATH
};

Position const spawnPos[2]   =
{
    {-256.5988f, 2267.021f, 100.8899f, 0.0f},
    {-247.5112f, 2232.320f,  98.5017f, 0.0f},
};

class boss_commander_springvale : public CreatureScript
{
    public:
        boss_commander_springvale() : CreatureScript("boss_commander_springvale") { }

        struct boss_commander_springvaleAI : public BossAI
        {
            boss_commander_springvaleAI(Creature * creature) : BossAI(creature, DATA_COMMANDER_SPRINGVALE_EVENT) { }

            void Reset()
            {
                _Reset();
                Achievement = true;
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WORD_OF_SHAME);
            }

            void KilledUnit(Unit* /*victim*/)
            {
                Talk(RAND(SAY_KILL_1, SAY_KILL_2));
            }

            void SpellHit(Unit* caster, const SpellInfo* spell)
            {
                if (!caster || !spell)
                    return;

                if (spell->Id != SPELL_UNHOLY_EMPOWERMENT)
                    return;

                Achievement = false;
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(SAY_AGGRO);
                events.ScheduleEvent(EVENT_DESECRATION, 10000);
                events.ScheduleEvent(EVENT_MALEFIX_STRIKE, 15000);
                if (IsHeroic())
                    events.ScheduleEvent(EVENT_SUMMONS, 40000);
            }

            void JustSummoned(Creature* summon)
            {
                BossAI::JustSummoned(summon);

                if (summon->GetEntry() == NPC_DESECRATION_TR)
                    me->AddAura(SPELL_DESECRATION_TRIGGERED, summon);
            }

            void JustDied(Unit * /*killer*/)
            {
                Talk(SAY_DEATH);
                _JustDied();
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WORD_OF_SHAME);
                if (Achievement && IsHeroic())
                    instance->DoCompleteAchievement(ACHIEVEMENT_TO_THE_GROUND);
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_RANDOM_CAST)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true))
                    {
                        if (IsHeroic())
                        {
                            switch(urand(0,1))
                            {
                                case 0:
                                    me->SetOrientation(me->GetAngle(target));
                                    DoCast(SPELL_SHIELD_OF_THE_PERFIDIOUS);
                                    break;
                                case 1:
                                    DoCastRandom(SPELL_WORD_OF_SHAME, 50, true);
                                    break;
                            }
                        }
                        else
                        {
                            me->SetOrientation(me->GetAngle(target));
                            DoCast(SPELL_SHIELD_OF_THE_PERFIDIOUS);
                        }
                    }
                }
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
                    case EVENT_DESECRATION:
                        DoCast(SPELL_DESECRATION);
                        events.ScheduleEvent(EVENT_DESECRATION, urand(14000, 20000));
                        break;
                    case EVENT_MALEFIX_STRIKE:
                        DoCastVictim(SPELL_MALEFIX_STRIKE);
                        events.ScheduleEvent(EVENT_MALEFIX_STRIKE, urand(12000, 18000));
                        break;
                    case EVENT_SUMMONS:
                        Talk(SAY_SUMMON_GUARDS);
                        me->SummonCreature(NPC_TORMENTED_OFFICER, spawnPos[0], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                        me->SummonCreature(NPC_WAILING_GUARDSMAN, spawnPos[1], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                        events.ScheduleEvent(EVENT_SUMMONS, 60000);
                        break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        private:
            bool Achievement;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_commander_springvaleAI(creature);
        }
};

class spell_springvale_unholy_power : public SpellScriptLoader
{
public:
    spell_springvale_unholy_power() : SpellScriptLoader("spell_springvale_unholy_power") { }

    class spell_springvale_unholy_power_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_springvale_unholy_power_AuraScript);

        bool Load()
        {
            _instance = GetCaster()->GetInstanceScript();
            return _instance != NULL;
        }

        void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
        {
            if (!GetCaster())
                return;

            if (Aura* aura = GetAura())
                if (aura->GetStackAmount() == 3)
                {
                    if (Creature* springvale = ObjectAccessor::GetCreature(*GetCaster(), _instance->GetData64(DATA_COMMANDER_SPRINGVALE)))
                        springvale->AI()->DoAction(ACTION_RANDOM_CAST);

                    aura->Remove();
                }
        }


        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_springvale_unholy_power_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }

        InstanceScript* _instance;
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_springvale_unholy_power_AuraScript();
    }
};

class spell_springvale_forsaken_ability : public SpellScriptLoader
{
public:
    spell_springvale_forsaken_ability() : SpellScriptLoader("spell_springvale_forsaken_ability") { }

    class spell_springvale_forsaken_ability_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_springvale_forsaken_ability_AuraScript);

        void HandleEffectPeriodic(AuraEffect const* aurEff)
        {
            if (!GetCaster() || !GetTarget())
                return;

            GetCaster()->CastSpell(GetTarget(), RAND(7038, 7039, 7040, 7041, 7042), true);

        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_springvale_forsaken_ability_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_springvale_forsaken_ability_AuraScript();
    }
};

void AddSC_boss_commander_springvale()
{
    new boss_commander_springvale();
    new spell_springvale_unholy_power();
    new spell_springvale_forsaken_ability();
}