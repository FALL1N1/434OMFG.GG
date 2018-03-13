
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "grimbatol.h"

enum Texts
{
    SAY_ENGAGE          = 0,
    SAY_EVENT           = 1,
    SAY_TROGGS          = 2,
    SAY_BOMB            = 3,
    SAY_SLAY            = 4,
    SAY_DEATH           = 5,
    EMOTE_BLITZ         = 6,
    EMOTE_GROUND_SIEGE  = 7,
    EMOTE_FRENZY        = 8
};

enum Spells
{
    SPELL_BLEEDING_WOUND            = 74846,
    SPELL_BLEEDING_WOUND_H          = 91937,
    SPELL_GROUND_SIEGE              = 74634,
    SPELL_GROUND_SIEGE_H            = 90249,

    SPELL_BLITZ_TRIGGER             = 74668,

    SPELL_BLITZ                     = 74670,
    SPELL_BLITZ_H                   = 90250,
    SPELL_FRENZY                    = 74853,
    // Adds
    SPELL_CLAW_PUNCTURE             = 76507,
    SPELL_CLAW_PUNCTURE_H           = 90212,
    SPELL_MODGUD_MALICE             = 74699,
    SPELL_MODGUDS_MALADY            = 74837,
    SPELL_MODGUD_MALICE_H           = 90169,
    SPELL_MODGUD_MALICE_BUFF        = 90170,
};

struct Locations
{
    float x, y, z, o;
};

static Locations SpawnPosition[]=
{
    {-722.15f, -442.53f, 268.77f, 0.54f},
    {-702.22f, -450.9f, 268.77f, 1.34f}
};

class boss_general_umbriss : public CreatureScript
{
    public:

        boss_general_umbriss() : CreatureScript("boss_general_umbriss") {}

        struct boss_general_umbrissAI : public ScriptedAI
        {
            boss_general_umbrissAI(Creature *c) : ScriptedAI(c)
            {
                instance = c->GetInstanceScript();
            }

            void Reset()
            {
                woundTimer = 12000;
                groundTimer = 20000;
                blitzTimer = 10000;
                addTimer = 18000;
                groundTimer2 = 200000;
                tempGUID = 0;
                isMalignantSpawn = false;
                enrage = false;
                me->RemoveAurasDueToSpell(SPELL_MODGUD_MALICE);
                me->RemoveAurasDueToSpell(SPELL_FRENZY);
                me->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
                if (instance)
                    instance->SetData(DATA_GENERAL_UMBRISS_EVENT, NOT_STARTED);
            }

            void EnterCombat(Unit* /*who*/)
            {
                if (instance)
                    instance->SetData(DATA_GENERAL_UMBRISS_EVENT, IN_PROGRESS);
                Talk(SAY_ENGAGE);
            }

            void JustDied(Unit* /*killer*/)
            {
                Talk(SAY_DEATH);
                if (instance)
                {
                    instance->SetData(DATA_GENERAL_UMBRISS_EVENT, DONE);
                    if (IsHeroic() && me->HasAura(SPELL_MODGUD_MALICE_H))
                        instance->DoCompleteAchievement(ACHIEV_UMBRAGE_FOR_UMBRISS);
                }
            }

            void KilledUnit(Unit* /*victim*/)
            {
                Talk(SAY_SLAY);
            }

            void UpdateAI(const uint32 uiDiff)
            {
                if (!UpdateVictim())
                    return;

                if (woundTimer <= uiDiff)
                {
                    DoCastVictim(SPELL_BLEEDING_WOUND);
                    woundTimer = 25000;
                }
                else
                    woundTimer -= uiDiff;

                if (groundTimer <= uiDiff)
                {
                    if (Unit* unit = SelectTarget(SELECT_TARGET_RANDOM, 1))
                    {
                        Talk(EMOTE_GROUND_SIEGE);
                        Position pos;
                        unit->GetPosition(&pos);
                        if (Creature* temp = me->SummonCreature(NPC_GROUND_SIEGE_STALKER, pos, TEMPSUMMON_TIMED_DESPAWN, 5000))
                        {
                            me->StopMoving();
                            me->SetFacingToObject(temp);
                            me->SendMeleeAttackStop();
                            DoCast(me, SPELL_GROUND_SIEGE);
                        }
                    }
                    blitzTimer = 12000;
                    groundTimer = 20000;
                }
                else
                    groundTimer -= uiDiff;

                if (blitzTimer <= uiDiff)
                {
                    if (Unit* unit = SelectTarget(SELECT_TARGET_RANDOM, 0, NonTankTargetSelector(me)))
                    {
                        Talk(EMOTE_BLITZ, unit->GetGUID());
                        if (Creature *c = me->SummonCreature(NPC_TRIGGER_BLITZ, unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 5000))
                        {
                            me->StopMoving();
                            me->SetFacingToObject(c);
                            me->SendMeleeAttackStop();
                            me->CastSpell(c, SPELL_BLITZ, false);
                        }
                    }
                    groundTimer = 13000;
                    blitzTimer = 20000;
                }
                else
                    blitzTimer -= uiDiff;

                if(!enrage)
                {
                    if(addTimer <= uiDiff)
                    {
                        Talk(SAY_TROGGS);
                        SpawnMalignant();
                        SpawnAdd();
                        addTimer = 60000;

                    }
                    else
                        addTimer -= uiDiff;

                    if (HealthBelowPct(30))
                    {
                        enrage = true;
                        Talk(EMOTE_FRENZY);
                        DoCast(SPELL_FRENZY);
                    }
                }

                DoMeleeAttackIfReady();
            }

            void SpawnAdd()
            {
                for(int i = 0; i < 3; i ++)
                    SummonCreatureWithRandomTarget(NPC_TROGG_DWELLER, SpawnPosition[0].x,SpawnPosition[0].y,SpawnPosition[0].z,SpawnPosition[0].o);
            }

            void SpawnMalignant()
            {
                if (!isMalignantSpawn && !enrage)
                {
                    SummonCreatureWithRandomTarget(NPC_MALIGNANT_TROGG, SpawnPosition[1].x,SpawnPosition[1].y,SpawnPosition[1].z,SpawnPosition[1].o);
                    isMalignantSpawn = true;
                }
            }

            void SummonCreatureWithRandomTarget(uint32 creatureId, float x, float y, float z, float o)
            {
                Creature* Summoned = me->SummonCreature(creatureId, x, y, z, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 5000);
                if (Summoned)
                {
                    Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0);
                    if (pTarget)
                    {
                        Summoned->AddThreat(pTarget, 1.0f);
                        if (Summoned->AI())
                            Summoned->AI()->AttackStart(pTarget);
                    }
                }
            }


            bool isMalignantSpawn;
        private :
            uint32 blitzTimer;
            uint32 groundTimer;
            uint32 groundTimer2;
            uint32 woundTimer;
            uint32 addTimer;
            bool enrage;
            uint64 tempGUID;
            InstanceScript* instance;
        };

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new boss_general_umbrissAI(pCreature);
        }
};

class npc_malignant : public CreatureScript
{
    public:

        npc_malignant() : CreatureScript("npc_malignant")
        {
        }

        struct npc_malignantAI : public ScriptedAI
        {
            npc_malignantAI(Creature *c) : ScriptedAI(c)
            {
                me->AddAura(SPELL_MODGUD_MALICE, me);
                die = false;
            }

            bool die;

            void DamageTaken(Unit* /*done_by*/, uint32& damage)
            {
                if(die)
                {
                    damage = 0;
                    return;
                }

                if(damage >= me->GetHealth())
                {
                    die = true;
                    damage = 0;
                    
                    if (IsHeroic())
                        DoCast(SPELL_MODGUD_MALICE_BUFF);

                    DoCast(SPELL_MODGUDS_MALADY);

                    if (Creature* umbriss = me->FindNearestCreature(BOSS_GENERAL_UMBRISS, 100.0f))
                        if (umbriss->AI())
                            CAST_AI(boss_general_umbriss::boss_general_umbrissAI, umbriss->AI())->isMalignantSpawn = false;

                    me->DespawnOrUnsummon();
                }
            }

            void UpdateAI(const uint32 uiDiff)
            {
                if (!UpdateVictim())
                    return;

                DoSpellAttackIfReady(SPELL_CLAW_PUNCTURE);
            }
        };

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_malignantAI(pCreature);
        }
};

class npc_trogg_dweller : public CreatureScript
{
    public:

        npc_trogg_dweller() : CreatureScript("npc_trogg_dweller")
        {
        }

        struct npc_trogg_dwellerAI : public ScriptedAI
        {
            npc_trogg_dwellerAI(Creature *c) : ScriptedAI(c) {}

            void UpdateAI(const uint32 uiDiff)
            {
                if (!UpdateVictim())
                    return;

                DoSpellAttackIfReady(SPELL_CLAW_PUNCTURE);
            }
        };

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_trogg_dwellerAI(pCreature);
        }
};

class spell_bleeding_wound : public SpellScriptLoader
{
public:
    spell_bleeding_wound() : SpellScriptLoader("spell_bleeding_wound") { }

    class spell_bleeding_wound_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_bleeding_wound_AuraScript);

        void HandlePeriodic(AuraEffect const* /*aurEff*/)
        {
            if (GetUnitOwner()->HealthAbovePct(90))
            {
                PreventDefaultAction();
                Remove(AURA_REMOVE_BY_ENEMY_SPELL);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_bleeding_wound_AuraScript::HandlePeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_bleeding_wound_AuraScript();
    }
};

void AddSC_boss_general_umbriss()
{
    new boss_general_umbriss();
    new npc_malignant();
    new npc_trogg_dweller();
    new spell_bleeding_wound();
}
