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

};

enum Spells
{
    SPELL_BLINK                       = 101812,
    SPELL_FLARECORE_MISSILE           = 101927,
    SPELL_FROSTBOLT_VOLLEY            = 101810,
    SPELL_PYROBLAST                   = 101809,
    SPELL_FROST_BLADES                = 101339, // Summon Frost Blades spell. Use with JustSummoned to send them away in a straight line, otherwise npc works ok.

    // NPC_FLARECORE
    SPELL_FLARECORE_TARGET_SELECTOR   = 101588, // dummy should cast SPELL_UNSTABLE_FLARE on hit, on expire timer cast SPELL_TIME_EXPIRE_FLARE
    SPELL_UNSTABLE_FLARE              = 101980,
    SPELL_TIME_EXPIRE_FLARE           = 101587,

    // NPC_FROST_BLADE
    SPELL_FROST_BLADES_AURA           = 101338, // trigger SPELL_FROST_BLADES_STUN
    SPELL_FROST_BLADES_STUN           = 101337,
};

enum Events
{
    EVENT_FLARECORE = 1,
    EVENT_FROSTBOLT,
    EVENT_PYROBLAST,
    EVENT_PRE_FROSTBLADES,
    EVENT_FROSTBLADES,
};

enum Npcs
{
    NPC_FROST_BLADE  = 54494,
    NPC_FLARECORE    = 54446,
    NPC_BLINK_TARGET = 54542,

};

class boss_echo_of_jaina : public CreatureScript
{
public:
    boss_echo_of_jaina() : CreatureScript("boss_echo_of_jaina") { }
    struct boss_echo_of_jainaAI : public BossAI
    {
        boss_echo_of_jainaAI(Creature* creature) : BossAI(creature, BOSS_ECHO_OF_JAINA) 
        {
            creature->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            creature->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
        }

        void Reset()
        {
            _orientation = 0.0f;
            RespawnBlinkTrigger();
            _Reset();
        }

        void RespawnBlinkTrigger()
        {
            std::list<Creature *> trigList;
            me->GetCreatureListWithEntryInGrid(trigList, NPC_BLINK_TARGET, 1000.0f);
            for (auto itr = trigList.begin(); itr != trigList.end(); itr++)
                (*itr)->Respawn();
        }

        void JustDied(Unit* killer)
        {
            // fixes combat bug - we currently dont know what the players hold in combat
            Map::PlayerList const& playerList = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator i = playerList.begin(); i != playerList.end(); ++i)
                if (Player* player = i->getSource())
                    if (player->isAlive())
                        player->CombatStop(false);

            _JustDied();
        }

        void EnterCombat(Unit* who)
        {
            me->SetReactState(REACT_AGGRESSIVE);
            events.ScheduleEvent(EVENT_PYROBLAST, 500);
            events.ScheduleEvent(EVENT_PRE_FROSTBLADES, 20000);
            events.ScheduleEvent(EVENT_FLARECORE, 14000);
            _EnterCombat();
        }

        void JustSummoned(Creature* summon)
        {
            summon->SetReactState(REACT_PASSIVE);
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            switch (summon->GetEntry())
            {
                case NPC_FLARECORE:
                    summon->CastSpell(summon, SPELL_FLARECORE_TARGET_SELECTOR, true);
                    break;
                case NPC_FROST_BLADE:
                {
                    summon->CastSpell(summon, SPELL_FROST_BLADES_AURA, true);
                    Position pos;
                    me->GetPosition(&pos);
                    me->NormalizeOrientation(_orientation);
                    me->GetNearPosition(pos, 100.0f, _orientation);
                    summon->GetMotionMaster()->MovePoint(0, pos.m_positionX, pos.m_positionY, pos.m_positionZ);
                    _orientation += M_PI / 12.0f;
                    break;
                }
            }
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
                    case EVENT_PYROBLAST:
                        DoCastVictim(SPELL_PYROBLAST);
                        events.ScheduleEvent(EVENT_PYROBLAST, 3500);
                        break;
                    case EVENT_FROSTBOLT:
                        DoCast(SPELL_FROSTBOLT_VOLLEY);
                        events.ScheduleEvent(EVENT_FROSTBOLT, 25000);
                        break;
                    case EVENT_FLARECORE:
                        DoCastRandom(SPELL_FLARECORE_MISSILE, 0.0f);
                        events.ScheduleEvent(EVENT_FLARECORE, 20000);
                        break;
                    case EVENT_PRE_FROSTBLADES:
                        DoCast(SPELL_BLINK);
                        events.DelayEvents(2500);
                        events.ScheduleEvent(EVENT_PRE_FROSTBLADES, 25000);
                        events.ScheduleEvent(EVENT_FROSTBLADES, 1000);
                        break;
                    case EVENT_FROSTBLADES:
                        if (Creature *blink = me->FindNearestCreature(NPC_BLINK_TARGET, 10.0f))
                            blink->DespawnOrUnsummon();
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            _orientation = me->GetAngle(target) - M_PI / 12.0f - me->GetOrientation();
                        for (uint8 i = 0; i < 3; i++)
                            me->CastSpell(me, SPELL_FROST_BLADES, true);
                        events.ScheduleEvent(EVENT_FROSTBOLT, 0); // always first spell after frostblades
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        float _orientation;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_echo_of_jainaAI(creature);
    }
};

// 101616
class spell_jaina_flarecore_dummy : public SpellScriptLoader
{
public:
    spell_jaina_flarecore_dummy() : SpellScriptLoader("spell_jaina_flarecore_dummy") { }

    class spell_jaina_flarecore_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_jaina_flarecore_dummy_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit *caster = GetCaster())
                if (Unit* target = GetHitUnit())
                {
                    caster->CastSpell(caster, SPELL_UNSTABLE_FLARE, true);
                    if (caster->GetTypeId() == TYPEID_UNIT)
                        caster->ToCreature()->DespawnOrUnsummon(1000);
                }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_jaina_flarecore_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_jaina_flarecore_dummy_SpellScript();
    }
};

// 101588
class spell_jaina_flamecore_periodic : public SpellScriptLoader
{
public:
    spell_jaina_flamecore_periodic() : SpellScriptLoader("spell_jaina_flamecore_periodic") { }

    class spell_jaina_flamecore_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_jaina_flamecore_periodic_AuraScript);

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
        {
            AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
            if (removeMode != AURA_REMOVE_BY_EXPIRE)
                return;
            if (Unit *caster = GetCaster())
            {
                caster->CastSpell(caster, SPELL_TIME_EXPIRE_FLARE, true);
                if (caster->GetTypeId() == TYPEID_UNIT)
                    caster->ToCreature()->DespawnOrUnsummon(1000);
            }
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_jaina_flamecore_periodic_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_jaina_flamecore_periodic_AuraScript();
    }
};

class go_jaina_staff_fragment : public GameObjectScript
{
public:
    go_jaina_staff_fragment() : GameObjectScript("go_jaina_staff_fragment") {}

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (go->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE))
            return true;
        go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
        if (InstanceScript* instance = go->GetInstanceScript())
            instance->SetData(DATA_FRAGMENTS, 0);
        player->PlayerTalkClass->ClearMenus();
        go->Delete(1000);
        return true;
    }
};


void AddSC_boss_echo_of_jaina()
{
    new boss_echo_of_jaina();
    new spell_jaina_flarecore_dummy();
    new spell_jaina_flamecore_periodic();
    new go_jaina_staff_fragment();
}
