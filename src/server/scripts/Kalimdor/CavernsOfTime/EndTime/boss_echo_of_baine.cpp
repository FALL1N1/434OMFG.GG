/*
 * OMFG.GG
 */


#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "PassiveAI.h"
#include "SpellScript.h"
#include "MoveSplineInit.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "endTime.h"

enum Texts
{
    TALK_ENTER_COMBAT = 0,
    TALK_INTRO        = 1, // What dark horrors have you wrought in this place? By my ancestors
    TALK_PLAYER_DIE   = 2,
    TALK_LAUNCH_TOTEM = 4,
    TALK_PULVERIZE    = 5,
    TALK_UNK2         = 6, // My wrath knows no bounds
    TALK_UNK3         = 7, // You wish to train? YOU!?
    TALK_DEATH        = 8,
};

enum Spells
{
    SPELL_LAUNCH_TOTEM_DUMMY         = 101613,
    SPELL_LAUNCH_TOTEM               = 101615,
    SPELL_SUMMON_LAUNCHED_TOTEM      = 101614,
    SPELL_MOLTEN_AXE_PERIODIC_DUMMY  = 101834,
    SPELL_MOLTEN_AXE                 = 101836,
    SPELL_MOLTEN_BLAST               = 101840,
    SPELL_MOLTEN_FISTS_PERIODIC      = 101865,
    SPELL_MOLTEN_FISTS_PROC_AURA     = 101866,
    SPELL_PULVERIZE_DUMMY            = 101625,
    SPELL_PULVERIZE                  = 101626,
    SPELL_PULVERIZE_1                = 101627,
    SPELL_PULVERIZE_DESTROY_PLATFORM = 101815,
    AURA_IN_LAVA                     = 101619,
    // NPC_BAINE_TOTEM
    SPELL_BAINE_TOTAM_AURA           = 101594,
    // players ?
    SPELL_TOTEM_BACK_PERIODIC        = 107837,
    SPELL_TOTEM_BACK                 = 101601,
};

enum Events
{
    EVENT_LAUNCH_TOTEM = 1,
    EVENT_PULVERIZE,
    EVENT_MOLTEN_AXE,
};

enum Npcs
{
    NPC_BAINE_TOTEM   = 54434,
    NPC_BAINE_TOTEM_2 = 54433, // wtf is this for ? cast veh hardcoded on NPC_BAINE_TOTEM
    NPC_ROCK_ISLAND   = 54496,
};

enum Gobs
{
    PLATFORM_1 = 209694,
    PLATFORM_2 = 209695,
    PLATFORM_3 = 209670,
    PLATFORM_4 = 209693,
};

class boss_echo_of_baine : public CreatureScript
{
public:
    boss_echo_of_baine() : CreatureScript("boss_echo_of_baine") { }
    struct boss_echo_of_baineAI : public BossAI
    {
        boss_echo_of_baineAI(Creature* creature) : BossAI(creature, BOSS_ECHO_OF_BAINE)
        {
            introDone = false;
            creature->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            creature->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
        }

        void Reset()
        {
            me->RemoveAurasDueToSpell(SPELL_MOLTEN_AXE_PERIODIC_DUMMY);
            _Reset();
        }

        void EnterEvadeMode() override
        {
            RebuiltPlatforms();
            BossAI::EnterEvadeMode();
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (introDone)
                return;

            if (!me->IsWithinDistInMap(who, 40.0f, false))
                return;

            Talk(TALK_INTRO);
            introDone = true;
        }

        void RebuiltPlatforms()
        {
            if (GameObject* platform =  me->FindNearestGameObject(PLATFORM_1, 100.0f))
                platform->SetDestructibleState(GO_DESTRUCTIBLE_REBUILDING);
            if (GameObject* platform =  me->FindNearestGameObject(PLATFORM_2, 100.0f))
                platform->SetDestructibleState(GO_DESTRUCTIBLE_REBUILDING);
            if (GameObject* platform =  me->FindNearestGameObject(PLATFORM_3, 100.0f))
                platform->SetDestructibleState(GO_DESTRUCTIBLE_REBUILDING);
            if (GameObject* platform =  me->FindNearestGameObject(PLATFORM_4, 100.0f))
                platform->SetDestructibleState(GO_DESTRUCTIBLE_REBUILDING);
            std::list<Creature *> trigList;
            me->GetCreatureListWithEntryInGrid(trigList, NPC_ROCK_ISLAND, 100.0f);
            for (auto itr = trigList.begin(); itr != trigList.end(); itr++)
                (*itr)->Respawn();
        }

        void JustDied(Unit* killer)
        {
            Talk(TALK_DEATH);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MOLTEN_FISTS_PERIODIC);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MOLTEN_FISTS_PROC_AURA);
            _JustDied();
        }

        void EnterCombat(Unit* who)
        {
            Talk(TALK_ENTER_COMBAT);
            events.ScheduleEvent(EVENT_LAUNCH_TOTEM, 10000);
            events.ScheduleEvent(EVENT_PULVERIZE, 30000);
            events.ScheduleEvent(EVENT_MOLTEN_AXE, 3000);
            Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                if (Player* player = itr->getSource())
                    if (player->GetAreaId() == 5792)
                        player->AddAura(SPELL_MOLTEN_FISTS_PERIODIC, player);

            _EnterCombat();
        }

        void JustSummoned(Creature *summon)
        {
            switch (summon->GetEntry())
            {
                case NPC_BAINE_TOTEM:
                    summon->SetReactState(REACT_PASSIVE);
                    summon->CastSpell(summon, SPELL_BAINE_TOTAM_AURA, true);
                    break;
            }
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == EFFECT_MOTION_TYPE && id == EVENT_JUMP)
                me->CastSpell(me, SPELL_PULVERIZE_1, true);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_LAUNCH_TOTEM:
                        Talk(TALK_LAUNCH_TOTEM);
                        DoCast(SPELL_LAUNCH_TOTEM_DUMMY);
                        events.ScheduleEvent(EVENT_LAUNCH_TOTEM, 30000);
                        break;
                    case EVENT_PULVERIZE:
                        Talk(TALK_PULVERIZE);
                        DoCast(SPELL_PULVERIZE_DUMMY);
                        events.ScheduleEvent(EVENT_PULVERIZE, 45000);
                        break;
                    case EVENT_MOLTEN_AXE:
                        me->CastSpell(me, SPELL_MOLTEN_AXE_PERIODIC_DUMMY, true);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        bool introDone;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_echo_of_baineAI(creature);
    }
};

class npc_baine_totem : public CreatureScript
{
public:
    npc_baine_totem() : CreatureScript("npc_baine_totem") { }

    struct npc_baine_totemAI : public ScriptedAI
    {
        npc_baine_totemAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
        }

        void OnSpellClick(Unit* player, bool& result) override
        {
            me->DespawnOrUnsummon();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_baine_totemAI(creature);
    }
};

class spell_baine_totem_launch : public SpellScriptLoader
{
public:
    spell_baine_totem_launch() : SpellScriptLoader("spell_baine_totem_launch") { }

    class spell_baine_totem_launch_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_baine_totem_launch_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if([](WorldObject* target)
                              {
                                  if (target->GetTypeId() != TYPEID_PLAYER)
                                      return true;
                                  if (target->ToPlayer()->HasAura(AURA_IN_LAVA))
                                      return true;
                                  return false;
                              });
            if (targets.empty())
            {
                std::list<Creature *> trigList;
                GetCaster()->GetCreatureListWithEntryInGrid(trigList, NPC_ROCK_ISLAND, 100.0f);
                for (auto itr = trigList.begin(); itr != trigList.end(); itr++)
                    if ((*itr)->isAlive())
                        targets.push_back(*itr);
            }
            Trinity::Containers::RandomResizeList(targets, 1);
        }

        void HandleDummy(SpellEffIndex effIndex)
        {
            if (Unit *caster = GetCaster())
                if (Unit *target = GetHitUnit())
                {
                    uint32 spellId = GetSpellInfo()->Effects[effIndex].CalcValue();
                    caster->CastSpell(target, spellId, true);
                }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_baine_totem_launch_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(spell_baine_totem_launch_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_baine_totem_launch_SpellScript();
    }
};

class spell_baine_pulverize : public SpellScriptLoader
{
public:
    spell_baine_pulverize() : SpellScriptLoader("spell_baine_pulverize") { }

    class spell_baine_pulverize_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_baine_pulverize_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if([](WorldObject* target)
                              {
                                  if (target->GetTypeId() != TYPEID_PLAYER)
                                      return true;
                                  if (target->ToPlayer()->HasAura(AURA_IN_LAVA))
                                      return true;
                                  return false;
                              });
            if (targets.empty())
            {
                std::list<Creature *> trigList;
                GetCaster()->GetCreatureListWithEntryInGrid(trigList, NPC_ROCK_ISLAND, 100.0f);
                for (auto itr = trigList.begin(); itr != trigList.end(); itr++)
                    if ((*itr)->isAlive())
                        targets.push_back(*itr);
            }
            Trinity::Containers::RandomResizeList(targets, 1);
        }


        void HandleDummy(SpellEffIndex effIndex)
        {
            if (Unit *caster = GetCaster())
                if (Unit *target = GetHitUnit())
                {
                    uint32 spellId = GetSpellInfo()->Effects[effIndex].CalcValue();
                    caster->CastSpell(target, spellId, true);
                    if (target->GetTypeId() == TYPEID_UNIT)
                        target->ToCreature()->DespawnOrUnsummon(1000);
                    else
                        if (Creature *trig = target->FindNearestCreature(NPC_ROCK_ISLAND, 15.0f))
                            trig->DespawnOrUnsummon(1000);
                }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_baine_pulverize_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
           OnEffectHitTarget += SpellEffectFn(spell_baine_pulverize_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_baine_pulverize_SpellScript();
    }
};

class spell_molten_axe_periodic_dummy : public SpellScriptLoader
{
public:
    spell_molten_axe_periodic_dummy() : SpellScriptLoader("spell_molten_axe_periodic_dummy") { }

    class spell_molten_axe_periodic_dummy_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_molten_axe_periodic_dummy_AuraScript);

        void OnPeriodic(AuraEffect const* aurEff)
        {
            if (GetTarget()->HasAura(SPELL_MOLTEN_FISTS_PROC_AURA) || GetTarget()->HasAura(SPELL_MOLTEN_AXE))
                return;

            if (GetTarget()->IsInWater())
                GetTarget()->CastSpell(GetTarget(), GetTarget()->GetTypeId() == TYPEID_PLAYER ? SPELL_MOLTEN_FISTS_PROC_AURA : SPELL_MOLTEN_AXE, true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_molten_axe_periodic_dummy_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_molten_axe_periodic_dummy_AuraScript();
    }
};

class spell_molten_axe_proc : public SpellScriptLoader
{
public:
    spell_molten_axe_proc() : SpellScriptLoader("spell_molten_axe_proc") { }

    class spell_molten_axe_proc_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_molten_axe_proc_AuraScript);

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            GetTarget()->CastSpell(eventInfo.GetActionTarget(), aurEff->GetSpellInfo()->Effects[EFFECT_0].TriggerSpell, true);
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(spell_molten_axe_proc_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_molten_axe_proc_AuraScript();
    }
};

class spell_player_launch_totem : public SpellScriptLoader
{
public:
    spell_player_launch_totem() : SpellScriptLoader("spell_player_launch_totem") { }

    class spell_player_launch_totem_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_player_launch_totem_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (Unit *caster = GetCaster())
            {
                caster->RemoveAurasDueToSpell(SPELL_TOTEM_BACK_PERIODIC);
                caster->RemoveAurasDueToSpell(SPELL_TOTEM_BACK);
            }
        }

        void Register() override
        {
            OnEffectLaunch += SpellEffectFn(spell_player_launch_totem_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_player_launch_totem_SpellScript();
    }
};

void AddSC_boss_echo_of_baine()
{
    new boss_echo_of_baine();
    new npc_baine_totem();
    new spell_baine_pulverize();
    new spell_baine_totem_launch();
    new spell_molten_axe_periodic_dummy();
    new spell_molten_axe_proc();
    new spell_player_launch_totem();
}
