/* ToDo
* Find the correct Sound Yells
* Implement Pistol Barrage handling
*/

#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "shadowfang_keep.h"

enum Spells
{
    SPELL_CURSED_BULLETS                   = 93629,
    SPELL_MORTAL_WOUND                     = 93675,
    SPELL_PISTOL_BARRAGE                   = 93520,
    SPELL_PISTOL_BARRAGE_DAMAGE            = 93564,
    SPELL_PISTOL_BARRAGE_TARGETS           = 96343,
    SPELL_SUMMON_GHOUL_CHANNELED           = 93707,
    SPELL_SUMMON_GHOUL_1                   = 93709,
    SPELL_SUMMON_GHOUL_2                   = 93714,
    SPELL_ACHIEVEMENT_CREDIT               = 95929
};

enum Events
{
    EVENT_CURSED_BULLETS                   = 1,
    EVENT_MORTAL_WOUND,
    EVENT_PISTOL_BARRAGE,
    EVENT_SUMMON_BLOODTHIRSTY_GHOULS
};

enum eTexts
{
    SAY_AGGRO_A,
    SAY_AGGRO_H,
    SAY_DEATH,
    SAY_KILLER_1,
    SAY_KILLER_2
};

class boss_lord_godfrey : public CreatureScript
{
    public:
        boss_lord_godfrey() : CreatureScript("boss_lord_godfrey") {}

        struct boss_lord_godfreyAI : public BossAI
        {
            boss_lord_godfreyAI(Creature* creature) : BossAI(creature, DATA_LORD_GODFREY_EVENT) 
            {
                me->ApplySpellImmune(0, IMMUNITY_ID, SPELL_PISTOL_BARRAGE_DAMAGE, true);
                me->ApplySpellImmune(0, IMMUNITY_ID, 93784, true);
            }

            void Reset()
            {
                _Reset();
            }

            void EnterCombat(Unit* who)
            {
                _EnterCombat();
                Talk(instance->GetData(TEAM_IN_INSTANCE) == TEAM_HORDE ? SAY_AGGRO_H : SAY_AGGRO_A);
                events.ScheduleEvent(EVENT_SUMMON_BLOODTHIRSTY_GHOULS, 5000);
                events.ScheduleEvent(EVENT_MORTAL_WOUND, 25000);
                events.ScheduleEvent(EVENT_PISTOL_BARRAGE, 10000);
                events.ScheduleEvent(EVENT_CURSED_BULLETS, 15000);
                Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers();
                for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                    i->getSource()->ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, ACHIEVEMENT_CRITERIA_CONDITION_NONE, SPELL_ACHIEVEMENT_CREDIT, true);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
                if (IsHeroic())
                    instance->FinishLfgDungeon(me);
            }

            void KilledUnit(Unit* victim)
            {
                Talk(RAND(SAY_KILLER_1, SAY_KILLER_2));
            }

            void JustSummoned(Creature* summon)
            {
                BossAI::JustSummoned(summon);
            }

			void SummonedCreatureDies(Creature* summoned, Unit* killer)
            {
                if (IsHeroic() && summoned->GetEntry() == 50561 && killer == me) // Bullet Time Achievement
                    instance->DoUpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_ACHIEVEMENT_CREDIT);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if(me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if(uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_SUMMON_BLOODTHIRSTY_GHOULS:
                            DoCast(me, SPELL_SUMMON_GHOUL_CHANNELED, true);
                            events.DelayEvents(3000);
                            events.ScheduleEvent(EVENT_SUMMON_BLOODTHIRSTY_GHOULS, 30000);
                            return;
                        case EVENT_MORTAL_WOUND:
                            DoCastVictim(SPELL_MORTAL_WOUND, true);
                            events.ScheduleEvent(EVENT_MORTAL_WOUND, 7000);
                            break;
                        case EVENT_PISTOL_BARRAGE:
                            DoCast(SPELL_PISTOL_BARRAGE_TARGETS);
                            DoCastVictim(SPELL_PISTOL_BARRAGE);
                            events.ScheduleEvent(EVENT_PISTOL_BARRAGE, 30000);
                            break;
                        case EVENT_CURSED_BULLETS:
                            DoCastRandom(SPELL_CURSED_BULLETS, 100.0f);
                            events.ScheduleEvent(EVENT_CURSED_BULLETS, 15000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_lord_godfreyAI(creature);
        }
};

class spell_godfrey_summon_ghouls : public SpellScriptLoader
{
public:
    spell_godfrey_summon_ghouls() : SpellScriptLoader("spell_godfrey_summon_ghouls") { }

    class spell_godfrey_summon_ghouls_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_godfrey_summon_ghouls_AuraScript);

        void HandlePeriodic(AuraEffect const* /*aurEff*/)
        {
            GetCaster()->CastSpell(GetCaster(), RAND(SPELL_SUMMON_GHOUL_1, SPELL_SUMMON_GHOUL_2), true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_godfrey_summon_ghouls_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_godfrey_summon_ghouls_AuraScript();
    }
};

class spell_pistol_barrage_trigger : public SpellScriptLoader
{
public:
    spell_pistol_barrage_trigger() : SpellScriptLoader("spell_pistol_barrage_trigger") { }

    class spell_pistol_barrage_trigger_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pistol_barrage_trigger_SpellScript);

        void SpawnInAngle(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            uint32 entry = uint32(GetSpellInfo()->Effects[effIndex].MiscValue);
            for(uint8 i = 0; i < 6; ++i)
            {
                float angle = -5 * M_PI / 24 + M_PI * i / 12;
                float x, y, z;
                GetCaster()->GetClosePoint(x, y, z, GetCaster()->GetObjectSize(), 30.0f, angle);
                GetCaster()->SummonCreature(entry, x, y, z, 0, TEMPSUMMON_TIMED_DESPAWN, 10000);
            }
        }

        void Register()
        {
            OnEffectLaunch += SpellEffectFn(spell_pistol_barrage_trigger_SpellScript::SpawnInAngle, EFFECT_0, SPELL_EFFECT_SUMMON);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_pistol_barrage_trigger_SpellScript();
    }
};


class spell_pistol_barrage : public SpellScriptLoader
{
public:
    spell_pistol_barrage() : SpellScriptLoader("spell_pistol_barrage") { }

    class spell_pistol_barrage_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pistol_barrage_AuraScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_PISTOL_BARRAGE))
                return false;
            return true;
        }

        void OnPeriodic(AuraEffect const* aurEff)
        {
            if (Unit* caster = GetCaster())
                caster->CastSpell((Unit*)NULL, SPELL_PISTOL_BARRAGE_DAMAGE, true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_pistol_barrage_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_pistol_barrage_AuraScript();
    }
};

class spell_cursed_bullets : public SpellScriptLoader
{
public:
    spell_cursed_bullets() : SpellScriptLoader("spell_cursed_bullets") { }

    class spell_cursed_bullets_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_cursed_bullets_AuraScript);

        void HandleEffectPeriodicUpdate(AuraEffect* aurEff)
        {
            uint32 bp = (GetOwner()->GetMap()->IsHeroic() ? 10000 : 50);
            aurEff->SetAmount(bp * aurEff->GetTickNumber());
        }

        void Register()
        {
            OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_cursed_bullets_AuraScript::HandleEffectPeriodicUpdate, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_cursed_bullets_AuraScript();
    }
};

void AddSC_boss_lord_godfrey()
{
    new boss_lord_godfrey();
    new spell_godfrey_summon_ghouls();
    new spell_pistol_barrage_trigger();
    new spell_pistol_barrage();
    new spell_cursed_bullets();
}
