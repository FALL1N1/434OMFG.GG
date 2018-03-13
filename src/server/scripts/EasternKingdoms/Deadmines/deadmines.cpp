/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Deadmines
SD%Complete: 0
SDComment: Placeholder
SDCategory: Deadmines
EndScriptData */

#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "deadmines.h"

#define SPELL_TRAP_EXPLOSION 90392

/*class go_canon_defias : public GameObjectScript
{
public:
    go_canon_defias() : GameObjectScript("go_canon_defias") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        go->SendCustomAnim(0);
        go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
        if (GameObject *door = player->FindNearestGameObject(GO_IRON_CLAD_DOOR, 100))
        {
            door->UseDoorOrButton();
            door->EnableCollision(false);
        }
        if (Creature *c = go->FindNearestCreature(45979, 20.0f))
        {
            c->AI()->Talk(2);
            c->CastSpell(c, SPELL_TRAP_EXPLOSION, true);
        }
        if (Creature *trash = go->SummonCreature(48502, -101.09f, -673.01f, 7.51f, 1.80f))
            trash->SetInCombatWithZone();
        if (Creature *trash = go->SummonCreature(48505, -98.48f, -672.39f, 7.51f, 1.80f))
            trash->SetInCombatWithZone();
        if (Creature *trash = go->SummonCreature(48417, -97.46f, -675.72f, 7.51f, 1.80f))
            trash->SetInCombatWithZone();
        if (Creature *trash = go->SummonCreature(48417, -100.88f, -676.35f, 7.51f, 1.80f))
            trash->SetInCombatWithZone();
        return true;
    }
};*/

enum npcs_defias_canon_spells
{
    SPELL_LAUNCH_BULLET = 89757,
};

enum lastEventSpells
{
    SPELL_SIT_DOWN = 89279,

    SPELL_STUN_PLAYERS = 92100,
    SPELL_NIGHTAMRE_ELEXIR_DUMMY = 92113,
    SPELL_VANESSA_BLACKOUT = 92120,

    //    SPELL_MAGMA_VEHICLE_DUMMY   = 92378, // trigger 92379 cast by 49454
    //    SPELL_MAGMA_VEHICLE_CONTROL = 92379, // hit 49454

    //    SPELL_NIGHTMARE_FLAME = 92169, // entry 51594
};

enum lastEventsNpcs
{
    //    NPC_VANCLEEF_EVENT = 49429,
    NPC_ANCHOR_BUNNY = 51624,
    //    NPC_MAGMA_VEHICLE = 49454,
};

enum lastEventEnents
{
    EVENT_INTRO = 1,
    EVENT_GLACIATE,
    ACTION_START_EVENT,
};

enum gameobjectnpcs
{
	NPC_DEFIAS_SHADOWGUARD = 48505,
	NPC_DEFIAS_ENFORCER = 48502,
	NPC_DEFIAS_BLOODWIZARD = 48417,
};

Position const SourcePosition[8] = {
	{ -30.2622f, -793.069f, 19.237f },
	{ -72.1059f, -786.894f, 39.5538f },
	{ -58.6424f, -787.132f, 39.3505f },
	{ -82.3142f, -775.5f, 26.8933f },
	{ -46.901f, -783.155f, 18.4898f },
	{ -89.2569f, -782.528f, 17.2564f },
	{ -122.925f, -388.813f, 59.0769f },
	{ -40.0035f, -793.302f, 39.4754f },
};

Position const TargetPosition[8] = {
	{ 0.512153f, -768.229f, 9.80134f },
	{ -72.559f, -731.221f, 8.5869f },
	{ -49.3264f, -730.056f, 9.32048f },
	{ -100.849f, -703.773f, 9.29407f },
	{ -30.6337f, -727.731f, 8.52102f },
	{ -88.4253f, -724.722f, 8.67503f },
	{ -91.9409f, -375.307f, 57.9774f },
	{ -12.0556f, -740.252f, 9.10946f },
};

class boss_vancleef_event : public CreatureScript
{
public:
    boss_vancleef_event() : CreatureScript("boss_vancleef_event") { }

    struct boss_vancleef_eventAI : public ScriptedAI
    {
        boss_vancleef_eventAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            _introCount = 0;
            me->CastSpell(me, SPELL_SIT_DOWN, true);
            me->SetReactState(REACT_PASSIVE);
            _events.Reset();
            DoAction(ACTION_START_EVENT);
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_START_EVENT:
                    _events.ScheduleEvent(EVENT_INTRO, 9000);
                    break;
            }
        }

        void UpdateAI(uint32 const diff)
        {
            _events.Update(diff);

            if (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_INTRO:
                    {
                        uint32 timer = 5000;
                        switch (_introCount)
                        {
                            case 0:
                                Talk(0);
                                timer = 4000;
                                break;
                            case 1:
                                Talk(1);
                                me->RemoveAura(SPELL_SIT_DOWN);
                                me->GetMotionMaster()->MoveJump(-64.5677f, -820.161f, 41.123f, 20.0f, 20.0f);
                                timer = 11000;
                                break;
                            case 2:
                                Talk(2);
                                me->GetMotionMaster()->MovePoint(0, -64.93489f, -832.3496f, 41.36931f);
                                timer = 10000;
                                break;
                            case 3:
                                Talk(3);
                                timer = 6000;
                                break;
                            case 4:
                                Talk(4);
                                DoCast(SPELL_STUN_PLAYERS);
                                timer = 5000;
                                break;
                            case 5:
                                Talk(5);
                                DoCast(SPELL_NIGHTAMRE_ELEXIR_DUMMY);
                                timer = 5000;
                                break;
                            case 6:
                                Talk(6);
                                DoCast(SPELL_VANESSA_BLACKOUT);
                                instance->SetData(DATA_START_VANCLEEF_EVENT, IN_PROGRESS);
                                timer = 5000;
                                break;
                            case 7:
                                me->DespawnOrUnsummon(1000);
                                break;
                            default:
                                return;
                        }
                        _events.ScheduleEvent(EVENT_INTRO, timer);
                        _introCount++;
                        break;
                    }
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        InstanceScript *instance;
        EventMap _events;
        uint32 _introCount;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_vancleef_eventAI(creature);
    }
};

// 92378
class spell_magma_vehicle_dummy : public SpellScriptLoader
{
public:
    spell_magma_vehicle_dummy() : SpellScriptLoader("spell_magma_vehicle_dummy") { }

    class spell_magma_vehicle_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_magma_vehicle_dummy_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            _seat = 0;
            return true;
        }

        void HandleScriptEffect(SpellEffIndex effIndex)
        {
            if (Unit *caster = GetCaster())
                if (Unit *target = GetHitUnit())
                    if (target->GetTypeId() == TYPEID_PLAYER)
                    {
                        _seat++;
                        uint32 spellId = GetSpellInfo()->Effects[effIndex].CalcValue();
                        target->SetDisableGravity(true); // hack fix camera fall on vehicle
                        target->SendMovementDisableGravity(); // hack fix camera fall on vehicle
                        target->CastCustomSpell(spellId, SPELLVALUE_BASE_POINT0, _seat, caster, false);
                    }
        }

    private:
        uint8 _seat;

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_magma_vehicle_dummy_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_magma_vehicle_dummy_SpellScript();
    }
};

class boss_glubtok_event : public CreatureScript
{
public:
    boss_glubtok_event() : CreatureScript("boss_glubtok_event") { }

    struct boss_glubtok_eventAI : public ScriptedAI
    {
        boss_glubtok_eventAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            me->RemoveAllAuras();
            _events.Reset();
            _events.ScheduleEvent(EVENT_GLACIATE, 10000);
        }

        void EnterCombat(Unit * /*who*/)
        {

        }

        void JustDied(Unit * /*killer*/)
        {
            if (instance)
                instance->SetData(DATA_GLUBTOK_EVENT, DONE);
        }

        void JustSummoned(Creature * summon)
        {
            if (summon->GetEntry() == NPC_COLLAPSING_ICICLE)
                summon->CastSpell(summon, SPELL_GLACIATE_DUMMY, false);
        }

        void UpdateAI(uint32 const diff)
        {
            _events.Update(diff);

            if (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_GLACIATE:
                    {
                        Unit *target = SelectTarget(SELECT_TARGET_RANDOM, 0);
                        if (!target && instance)
                        {
                            std::list<Unit *> targets;
                            Map::PlayerList const &PlayerList = instance->instance->GetPlayers();
                            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                                if (Player *pl = i->getSource())
                                    if (pl->isAlive())
                                        targets.push_back(pl);
                            if (!targets.empty())
                                Trinity::Containers::RandomResizeList(targets, 1);
                            target = targets.front();
                        }
                        if (target)
                        {
                            Position pos;
                            target->GetPosition(&pos);
                            if (Creature *c = me->SummonCreature(45979, pos, TEMPSUMMON_TIMED_DESPAWN, 10000))
                                me->CastSpell(c, SPELL_SUMMON_ICETOMB, true);
                        }
                        _events.ScheduleEvent(EVENT_GLACIATE, 3000);
                        break;
                    }
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        InstanceScript *instance;
        EventMap _events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_glubtok_eventAI(creature);
    }
};

// 92202
class spell_glaciate_dummy : public SpellScriptLoader
{
public:
    spell_glaciate_dummy() : SpellScriptLoader("spell_glaciate_dummy") { }

    class spell_glaciate_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_glaciate_dummy_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void HandleDummyEffect(SpellEffIndex effIndex)
        {
            if (Unit *target = GetHitUnit())
            {
                if (target->ToTempSummon())
                    if (Unit *owner = target->ToTempSummon()->GetSummoner())
                    {
                        if (owner->GetEntry() != NPC_COLLAPSING_ICICLE)
                        {
                            owner->CastSpell(owner, SPELL_GLACIATE, false);
                            target->CastSpell(target, SPELL_SUMMON_ICETOMB, true);
                        }
                    }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_glaciate_dummy_SpellScript::HandleDummyEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_glaciate_dummy_SpellScript();
    }
};
/*
// 49481
class npc_glubtok_icicle : public CreatureScript
{
public:
    npc_glubtok_icicle() : CreatureScript("npc_glubtok_icicle") {}

    struct npc_glubtok_icicleAI : public ScriptedAI
    {
        npc_glubtok_icicleAI(Creature * creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
        }

        void IsSummonedBy(Unit *who)
        {
            if (instance)
                if (Creature *c = Unit::GetCreature(*me, instance->GetData64(BOSS_GLUBTOK_EVENT_DATA)))
                {
                    if (who->GetTypeId() == TYPEID_UNIT && who->GetEntry() != me->GetEntry())
                        me->SetVisible(false);
                    c->AI()->JustSummoned(me);
                }
        }

    private:
        InstanceScript *instance;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_glubtok_icicleAI(creature);
    }
};*/

// 92210
class spell_gb_summon_icicle_target_selector : public SpellScriptLoader
{
public:
    spell_gb_summon_icicle_target_selector() : SpellScriptLoader("spell_gb_summon_icicle_target_selector") { }

    class spell_gb_summon_icicle_target_selector_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_gb_summon_icicle_target_selector_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void HandleDummyEffect(SpellEffIndex effIndex)
        {
            if (Unit *target = GetHitUnit())
            {
                uint32 spellId = GetSpellInfo()->Effects[effIndex].CalcValue();
                target->CastSpell(target, spellId, true);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_gb_summon_icicle_target_selector_SpellScript::HandleDummyEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_gb_summon_icicle_target_selector_SpellScript();
    }
};

enum helxevEvts
{
    EVENT_RIDE_PLAYERS = 1,
    EVENT_ACTIVATE_SPIDERS,
    ACTION_RIDE_END = 5, // dont change this value
};

enum heEvSpells
{
    SPELL_HELIX_SET_VEHICLE = 88337,
    SPELL_HELIX_RIDE_FACING_TARGET = 88349,
};
/*
class boss_helix_gearbreaker_event : public CreatureScript
{
public:
    boss_helix_gearbreaker_event() : CreatureScript("boss_helix_gearbreaker_event") { }

    struct boss_helix_gearbreaker_eventAI : public ScriptedAI
    {
        boss_helix_gearbreaker_eventAI(Creature* creature) : ScriptedAI(creature), _summons(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            _events.Reset();
            _summons.DespawnAll();
            me->SetReactState(REACT_PASSIVE);
        }

        void EnterCombat(Unit* who)
        {
            Talk(1);
            if (instance)
                instance->SetData(DATA_HELIX_EVENT, IN_PROGRESS);
            _events.ScheduleEvent(EVENT_ACTIVATE_SPIDERS, 1000);
            _events.ScheduleEvent(EVENT_RIDE_PLAYERS, 5000);
        }

        void JustDied(Unit* /*killer*//*)
        {
            if (instance)
                instance->SetData(DATA_HELIX_EVENT, DONE);
            _events.Reset();
            _summons.DespawnAll();
        }


        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_START_FIGHT:
                {
                    me->RemoveAllAuras();
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NON_ATTACKABLE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    Talk(0);
                    break;
                }
                case ACTION_RIDE_END:
                {
                    me->ExitVehicle();
                    break;
                }
            }
        }

        void JustSummoned(Creature* summon)
        {
            _summons.Summon(summon);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            _events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_RIDE_PLAYERS:
                        DoCast(SPELL_HELIX_SET_VEHICLE);
                        DoCast(SPELL_HELIX_RIDE_FACING_TARGET);
                        _events.ScheduleEvent(EVENT_RIDE_PLAYERS, 12000);
                        break;
                    case EVENT_ACTIVATE_SPIDERS:
                    {
                        std::list<Creature* > triggerList;
                        me->GetCreatureListWithEntryInGrid(triggerList, NPC_DARKWEB_DEVOURER, 100.0f);
                        me->GetCreatureListWithEntryInGrid(triggerList, NPC_CHATTERING_HORROR, 100.0f);
                        me->GetCreatureListWithEntryInGrid(triggerList, NPC_NIGHTMARE_SKITTERLING, 100.0f);
                        Trinity::Containers::RandomResizeList(triggerList, std::max(1, rand() % 2));
                        for (std::list<Creature* >::iterator itr = triggerList.begin(); itr != triggerList.end(); itr++)
                            if (Creature *spider = *itr)
                                if (spider->HasReactState(REACT_PASSIVE))
                                {
                                    spider->SetReactState(REACT_AGGRESSIVE);
                                    spider->SetInCombatWithZone();
                                }
                        _events.ScheduleEvent(EVENT_ACTIVATE_SPIDERS, 1000);
                        break;
                    }
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        EventMap _events;
        SummonList _summons;
        InstanceScript *instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_helix_gearbreaker_eventAI(creature);
    }
};*/

enum lgpEvents
{
    EVENT_ROTATE = 1,
};

class npc_vanessa_lighting_patter : public CreatureScript
{
public:
    npc_vanessa_lighting_patter() : CreatureScript("npc_vanessa_lighting_patter") { }

    struct npc_vanessa_lighting_patterAI : public ScriptedAI
    {
        npc_vanessa_lighting_patterAI(Creature* creature) : ScriptedAI(creature), _summons(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            start = false;
            cnt = 0;
            _events.Reset();
            _summons.DespawnAll();
            me->SetReactState(REACT_PASSIVE);
            bool turnRight = rand() % 2;
            me->SetHover(true);
        }

        void EnterCombat(Unit* who)
        {
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_ACTIVATE:
                    me->CastSpell(me, SPELL_LIGHTING, true);
                    start = false;
                    _summons.DespawnAll();
                    _events.ScheduleEvent(EVENT_ROTATE, 0);
                    break;
                case ACTION_DESACTIVATE:
                    me->RemoveAura(SPELL_LIGHTING);
                    start = true;
                    _events.CancelEvent(EVENT_ROTATE);
                    break;
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            _events.Reset();
            _summons.DespawnAll();
        }

        void JustSummoned(Creature* summon)
        {
            cnt++;
            summon->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, cnt, me, false);
            summon->AddAura(SPELL_LIGHTING_2, summon);
            _summons.Summon(summon);
        }

        void UpdateAI(uint32 const diff)
        {
            _events.Update(diff);

            if (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ROTATE:
                        if (!start)
                            for (int i = 0; i < 8; i++)
                                me->SummonCreature(NPC_VANESSA_LIGHTING_PLATTER_PASSENGER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f);
                        me->GetMotionMaster()->MoveRotate(20000, turnRight ? ROTATE_DIRECTION_RIGHT : ROTATE_DIRECTION_LEFT);
                        start = true;
                        _events.ScheduleEvent(EVENT_ROTATE, 20000);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        EventMap _events;
        SummonList _summons;
        InstanceScript *instance;
        int cnt;
        bool start, turnRight;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_vanessa_lighting_patterAI(creature);
    }
};

enum reeperEventDatas
{
    EVENT_RREP_SPIRIT = 1,
    SPELL_SPRIT_HIT = 59304,
};

/*class boss_foe_reeper_event : public CreatureScript
{
public:
    boss_foe_reeper_event() : CreatureScript("boss_foe_reeper_event") { }

    struct boss_foe_reeper_eventAI : public ScriptedAI
    {
        boss_foe_reeper_eventAI(Creature* creature) : ScriptedAI(creature), _summons(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            _events.Reset();
            _summons.DespawnAll();
            me->SetReactState(REACT_PASSIVE);
        }

        void EnterCombat(Unit* who)
        {
            if (instance)
                instance->SetData(DATA_REEPER_EVENT, IN_PROGRESS);
            _events.ScheduleEvent(EVENT_RREP_SPIRIT, 1000);
        }

        void JustDied(Unit* /*killer*//*)
        {
            if (instance)
                instance->SetData(DATA_REEPER_EVENT, DONE);
            _events.Reset();
            _summons.DespawnAll();
        }


        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_START_FIGHT:
                {
                    me->RemoveAllAuras();
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NON_ATTACKABLE);
                    me->SetReactState(REACT_DEFENSIVE);
                    break;
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            _events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_RREP_SPIRIT:
                        DoCastVictim(SPELL_SPRIT_HIT);
                        _events.ScheduleEvent(EVENT_RREP_SPIRIT, 5000);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        EventMap _events;
        SummonList _summons;
        InstanceScript *instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_foe_reeper_eventAI(creature);
    }
};*/

class npc_heric_harringtom : public CreatureScript
{
public:
    npc_heric_harringtom() : CreatureScript("npc_heric_harringtom") { }

    struct npc_heric_harringtomAI : public ScriptedAI
    {
        npc_heric_harringtomAI(Creature* creature) : ScriptedAI(creature), _summons(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            Talk(0);
            cnt = 3;
            me->SummonCreature(NPC_ENRAGED_WORGEN, PartWorgen[0]);
            me->SummonCreature(NPC_ENRAGED_WORGEN, PartWorgen[1]);
            me->SummonCreature(NPC_ENRAGED_WORGEN, PartWorgen[2]);
        }

        void JustSummoned(Creature* summon)
        {
            summon->AI()->AttackStart(me);
            _summons.Summon(summon);
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/)
        {
            --cnt;
            if (cnt <= 0)
            {
                me->CastSpell(me, SPELL_ADRENALINE, true);
                if (instance)
                    instance->SetData(DATA_WORGEN_EVENT, DONE);
                me->DespawnOrUnsummon(5000);
            }
        }

        void EnterCombat(Unit* who)
        {
        }

        void JustDied(Unit* /*killer*/)
        {
            if (instance)
                instance->SetData(DATA_WORGEN_EVENT, FAIL);
            _summons.DespawnAll();
        }


        void UpdateAI(uint32 const diff)
        {

        }
    private:
        EventMap _events;
        SummonList _summons;
        InstanceScript *instance;
        int cnt;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_heric_harringtomAI(creature);
    }
};

class npc_emma_harringtom : public CreatureScript
{
public:
    npc_emma_harringtom() : CreatureScript("npc_emma_harringtom") { }

    struct npc_emma_harringtomAI : public ScriptedAI
    {
        npc_emma_harringtomAI(Creature* creature) : ScriptedAI(creature), _summons(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            Talk(0);
            cnt = 3;
            me->SummonCreature(NPC_ENRAGED_WORGEN, PartWorgen[3]);
            me->SummonCreature(NPC_ENRAGED_WORGEN, PartWorgen[4]);
            me->SummonCreature(NPC_ENRAGED_WORGEN, PartWorgen[5]);
        }

        void JustSummoned(Creature* summon)
        {
            summon->AI()->AttackStart(me);
            _summons.Summon(summon);
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/)
        {
            --cnt;
            if (cnt <= 0)
            {
                me->CastSpell(me, SPELL_ADRENALINE, true);
                if (instance)
                    instance->SetData(DATA_WORGEN_EVENT, DONE);
                me->DespawnOrUnsummon(5000);
            }
        }

        void EnterCombat(Unit* who)
        {
        }

        void JustDied(Unit* /*killer*/)
        {
            if (instance)
                instance->SetData(DATA_WORGEN_EVENT, FAIL);
            _summons.DespawnAll();
        }


        void UpdateAI(uint32 const diff)
        {

        }
    private:
        EventMap _events;
        SummonList _summons;
        InstanceScript *instance;
        int cnt;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_emma_harringtomAI(creature);
    }
};

class npc_callisa_harringtom : public CreatureScript
{
public:
    npc_callisa_harringtom() : CreatureScript("npc_callisa_harringtom") { }

    struct npc_callisa_harringtomAI : public ScriptedAI
    {
        npc_callisa_harringtomAI(Creature* creature) : ScriptedAI(creature), _summons(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            Talk(0);
            start = false;
        }

        void JustSummoned(Creature* summon)
        {
            me->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, summon, false);
            _summons.Summon(summon);
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/)
        {
            if (instance)
                instance->SetData(DATA_WORGEN_EVENT, DONE);
            me->DespawnOrUnsummon(5000);
        }

        void EnterCombat(Unit* who)
        {
        }

        void JustDied(Unit* /*killer*/)
        {
            if (instance)
                instance->SetData(DATA_WORGEN_EVENT, FAIL);
            _summons.DespawnAll();
        }


        void UpdateAI(uint32 const diff)
        {
            if (start)
                return;
            start = true;
            me->SummonCreature(NPC_JAMES_HARRINGTOM, PartHarringtom[2]);
        }
    private:
        EventMap _events;
        SummonList _summons;
        InstanceScript *instance;
        bool start;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_callisa_harringtomAI(creature);
    }
};

class npc_vanessa_paper : public CreatureScript
{
public:
    npc_vanessa_paper() : CreatureScript("npc_vanessa_paper") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (InstanceScript *instance = creature->GetInstanceScript())
            if (!instance->IsDone(BOSS_EVENT_VANCLEEF) && instance->GetBossState(BOSS_EVENT_VANCLEEF) != IN_PROGRESS && instance->GetData64(BOSS_EVENT_VANCLEEF) == 0 && instance->IsDone(BOSS_CAPTAIN_COOKIE))
            {
                instance->SetData(DATA_PREPARE_VANCLEEF_EVENT, IN_PROGRESS);
                creature->DespawnOrUnsummon(1000);
            }
        return true;
    }
};

#define DOSSIP_TEL_1 "Press the button labeled 'Wood and Lumber'."
#define DOSSIP_TEL_2 "Press the button labeled 'Metal and Scrap'."
#define DOSSIP_TEL_3 "Boat"

enum teleporterSpells
{
    SPELL_TELEPORT_ENTRANCE = 79590, // unused ?
};

// 208002
class go_deadmine_teleporter : public GameObjectScript
{
public:
    go_deadmine_teleporter() : GameObjectScript("go_deadmine_teleporter") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (InstanceScript *instance = go->GetInstanceScript())
        {
            go->SendCustomAnim(0);
            if (instance->IsDone(BOSS_HELIX_DATA))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, DOSSIP_TEL_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
            if (instance->IsDone(BOSS_FOE_REAPER_5000_DATA))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, DOSSIP_TEL_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            if (instance->IsDone(BOSS_EVENT_VANCLEEF))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, DOSSIP_TEL_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(12691, go->GetGUID());
        }
        return true;
    }

    bool OnGossipSelect(Player* player, GameObject* go, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF:
            player->NearTeleportTo(-304.96f, -491.73f, 50.04f, 5.92f);
            break;
        case GOSSIP_ACTION_INFO_DEF + 1:
            player->NearTeleportTo(-201.09f, -605.88f, 20.12f, 1.86f);
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            player->NearTeleportTo(-129.64f, -789.04f, 18.16f, 5.57f);
            break;
        }
        return true;
    }
};

// 49454
class npc_vancleef_trap : public CreatureScript
{
public:
    npc_vancleef_trap() : CreatureScript("npc_vancleef_trap") { }


    struct npc_vancleef_trapAI : public PassiveAI
    {
        npc_vancleef_trapAI(Creature* creature) : PassiveAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
            _reaperCount = 4;
            start = false;
        }

        void PassengerBoarded(Unit* player, int8 seatId, bool apply)
        {
            if (!apply)
            {
                player->SetDisableGravity(false);
                player->SendMovementDisableGravity();
                return;
            }
            player->SetDisableGravity(true);
            player->SendMovementDisableGravity();
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_INIT_EVENT:
                {
                    Position pos;
                    me->GetPosition(&pos);
                    pos.m_positionZ += 20.0f;
                    if (Creature *marker = me->SummonCreature(49552, pos))
                        me->CastSpell(marker, SPELL_ROPE_RAY, true);
                    me->CastSpell(me, SPELL_MAGMA_VEHICLE_DUMMY, true);
                    break;
                }
                case ACTION_EJECT_PASSENGERS:
                {
                    me->CastSpell(me, SPELL_EJECT_ALL_PASSENGERS, true);
                    me->DespawnOrUnsummon(1);
                    if (instance)
                        instance->SetData(DATA_TRAP_COMPLETE, DONE);
                    break;
                }
                case ACTION_EMOTE_TRAP:
                {
                    if (start)
                        return;
                    start = true;
                    Talk(0);
                    break;
                }
            }
        }

        void UpdateAI(const uint32 diff)
        {
        }

    private:
        bool start;
        int8 _reaperCount;
        InstanceScript* instance;
        Position _spawnPos;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_vancleef_trapAI (creature);
    }
};

class npc_trap_vapor_event : public CreatureScript
{
public:
    npc_trap_vapor_event() : CreatureScript("npc_trap_vapor_event") {}

    struct npc_trap_vapor_eventAI : public PassiveAI
    {
        npc_trap_vapor_eventAI(Creature* pCreature) : PassiveAI(pCreature)
        {
        }

        void Reset()
        {
            me->RemoveAura(SPELL_AURA_VAPOR_EVENT);
        }

        void OnSpellClick(Unit* player, bool& result)
        {
            me->CastSpell(me, SPELL_AURA_VAPOR_EVENT, true);
        }
    };

    CreatureAI* GetAI(Creature *pCreature) const
    {
        return new npc_trap_vapor_eventAI(pCreature);
    }
};

// 207079
class go_liberate_singe : public GameObjectScript
{
public:
    go_liberate_singe() : GameObjectScript("go_liberate_singe") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
        go->SendCustomAnim(0);
        if (Creature *target = go->FindNearestCreature(48279, 20))
            if (!forceMoonkiAttack(go->FindNearestCreature(48440, 5), target, player))
                if (!forceMoonkiAttack(go->FindNearestCreature(48441, 5), target, player))
                    if (!forceMoonkiAttack(go->FindNearestCreature(48442, 5), target, player))
                        forceMoonkiAttack(go->FindNearestCreature(48278, 5), target, player);
        return true;
    }

    bool forceMoonkiAttack(Creature *moonki, Creature *target, Player *player)
    {
        if (moonki)
        {
            moonki->setFaction(player->getFaction());
            moonki->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
            moonki->SetReactState(REACT_AGGRESSIVE);
            moonki->Attack(target, true);
            return true;
        }
        return false;
    }
};

enum minorSpells
{
    SPELL_REGENRATE_STRENGHT = 91009,
    SPELL_TUNNEL = 90994,
};

enum minorEvents
{
    EVENT_TUNNEL = 1,
};

// 48419, 48420
class npc_deadmine_minor : public CreatureScript
{
public:
    npc_deadmine_minor() : CreatureScript("npc_deadmine_minor") { }

    struct npc_deadmine_minorAI : public ScriptedAI
    {
        npc_deadmine_minorAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            work = true;
            _events.Reset();
        }

        void EnterCombat(Unit * /*who*/)
        {
            work = false;
            if (IsHeroic())
                me->CastWithDelay(10000, me, SPELL_REGENRATE_STRENGHT, true);
            _events.ScheduleEvent(EVENT_TUNNEL, 10000);
        }

        void UpdateAI(const uint32 diff)
        {
            if (work)
                me->HandleEmoteCommand(467);

            if (!UpdateVictim())
                return;

            _events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_TUNNEL:
                        DoCastRandom(SPELL_TUNNEL, 0.0f);
                        _events.ScheduleEvent(EVENT_TUNNEL, 10000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private :
        InstanceScript *instance;
        EventMap _events;
        bool work;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_deadmine_minorAI (creature);
    }
};

enum defiasSumSpells
{
    SPELL_SUMMONER_SHIELD = 92001,
    SPELL_HOLY_FIRE = 91004,
};

enum defiasSumEvents
{
    EVENT_HOLLY_FIRE = 1,
};

// 48418
class npc_deadmine_defias_summoner : public CreatureScript
{
public:
    npc_deadmine_defias_summoner() : CreatureScript("npc_deadmine_defias_summoner") { }

    struct npc_deadmine_defias_summonerAI : public ScriptedAI
    {
        npc_deadmine_defias_summonerAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            me->GetMotionMaster()->MoveRandom(5);
            _events.Reset();
        }

        void EnterCombat(Unit * /*who*/)
        {
            if (IsHeroic())
                me->CastWithDelay(10000, me, SPELL_REGENRATE_STRENGHT, true);
            DoCast(SPELL_SUMMONER_SHIELD);
            _events.ScheduleEvent(EVENT_HOLLY_FIRE, 0);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            _events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_TUNNEL:
                        DoCastRandom(SPELL_HOLY_FIRE, 0.0f);
                        _events.ScheduleEvent(EVENT_HOLLY_FIRE, 5000);
                        break;
                    default:
                        break;
                }
            }

        }

    private :
        InstanceScript *instance;
        EventMap _events;
        bool work;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_deadmine_defias_summonerAI (creature);
    }
};
/*
enum Envoker
{
	SPELL_RENEGADE = 90047,
	SPELL_SHIELD = 92001

};

class npc_defias_envoker : public CreatureScript
{
public:
	npc_defias_envoker() : CreatureScript("npc_defias_envoker") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_defias_envokerAI(creature);
	}

	struct npc_defias_envokerAI : public ScriptedAI
	{
		npc_defias_envokerAI(Creature* creature) : ScriptedAI(creature){}

		uint32 HolyfireTimer;
		uint32 ShieldTimer;

		void Reset()
		{
			HolyfireTimer = 4000;
			ShieldTimer = 8000;
		}

		void UpdateAI(uint32 const diff)
		{
			if (HolyfireTimer <= diff)
			{
				if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
					DoCast(target, SPELL_HOLY_FIRE);
				HolyfireTimer = urand(8000, 11000);
			}
			else HolyfireTimer -= diff;

			if (ShieldTimer <= diff)
			{
				if (IsHeroic())
				{
					DoCast(me, SPELL_SHIELD);
					ShieldTimer = urand(18000, 20000);
				}
			}
			else ShieldTimer -= diff;

			DoMeleeAttackIfReady();
		}
	};
};*/

enum Lackey
{
	SPELL_AXE_HEAD = 90098,
	SPELL_ENRAGE = 8599,
};

class npc_oaf_lackey : public CreatureScript
{
public:
	npc_oaf_lackey() : CreatureScript("npc_oaf_lackey") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_oaf_lackeyAI(creature);
	}

	struct npc_oaf_lackeyAI : public ScriptedAI
	{
		npc_oaf_lackeyAI(Creature* creature) : ScriptedAI(creature){}

		uint32 AxeHeadTimer;

		bool below;

		void Reset()
		{
			AxeHeadTimer = 4000;
			below = true;
		}

		void UpdateAI(uint32 const diff)
		{
			if (AxeHeadTimer <= diff)
			{
				DoCastVictim(SPELL_AXE_HEAD);
				AxeHeadTimer = urand(18000, 21000);
			}
			else AxeHeadTimer -= diff;

			if (HealthBelowPct(35) && !below)
			{
				DoCast(me, SPELL_ENRAGE);
				below = true;
			}

			DoMeleeAttackIfReady();
		}

        void JustDied(Unit* killer)
        {
            // if last guard died (25y check)  - open door
            if (me->FindNearestCreature(me->GetEntry(), 25, false))
                if (GameObject* door = me->FindNearestGameObject(17153, 20))
                {
                    door->SetLootState(GO_READY);
                    door->UseDoorOrButton(10000, false);
                }
        }
	};
};

/*enum Overseer
{
	SPELL_MOTIVATE = 91036,
	SPELL_THREATENING = 91034,
};

class npc_goblin_overseer : public CreatureScript
{
public:
	npc_goblin_overseer() : CreatureScript("npc_goblin_overseer") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_goblin_overseerAI(creature);
	}

	struct npc_goblin_overseerAI : public ScriptedAI
	{
		npc_goblin_overseerAI(Creature* creature) : ScriptedAI(creature){}

		uint32 MotivateTimer;

		bool threat;

		void Reset()
		{
			MotivateTimer = 4000;
			threat = true;
		}

		void UpdateAI(uint32 const diff)
		{
			if (MotivateTimer <= diff)
			{
				if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
					DoCast(target, SPELL_MOTIVATE);
				MotivateTimer = urand(8000, 11000);
			}
			else MotivateTimer -= diff;

			if (HealthBelowPct(50) && !threat)
			{
				DoCast(me, SPELL_THREATENING);
				threat = true;
			}

			DoMeleeAttackIfReady();
		}
	};
};*/

class npc_mining_monkey : public CreatureScript
{
public:
	npc_mining_monkey() : CreatureScript("npc_mining_monkey") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_mining_monkeyAI(creature);
	}

	struct npc_mining_monkeyAI : public ScriptedAI
	{
		npc_mining_monkeyAI(Creature* creature) : ScriptedAI(creature)
		{
			instance = creature->GetInstanceScript();
		}

		InstanceScript* instance;
		uint32 Phase;
		uint32 uiTimer;

		void Reset()
		{
			Phase = 1;
			uiTimer = 2000;
		}

		void DamageTaken(Unit* /*attacker*/, uint32& damage)
		{
			if (!me)
				return;

			if (Phase == 1)
			{
				if (me->GetHealth() - damage <= me->GetMaxHealth()*0.15)
				{
					Phase++;
				}
			}
		}

		void EnterCombat(Unit* /*who*/)
		{
			if (!me)
				return;
		}

		void UpdateAI(uint32 const uiDiff)
		{
			if (!me || !me->AI() || !UpdateVictim())
				return;

			switch (Phase)
			{
			case 1:
				if (Unit* victim = me->getVictim())
				{
					if (me->IsInRange(victim, 0, 35.0f, true))
					{
						me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
						me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
						if (uiTimer <= uiDiff)
						{
							me->CastSpell(victim, IsHeroic() ? SPELL_THROW_H : SPELL_THROW);
							uiTimer = 2000;
						}
						else
							uiTimer -= uiDiff;
					}
					else
					{
						me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
						me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
					}
				}
				break;
			case 2:
				Talk(0);
				me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
				Phase++;
				break;
			default:
				me->DoFleeToGetAssistance();
				break;
			}
		}
	};
};

class npc_goblin_engineer : public CreatureScript
{
public:
	npc_goblin_engineer() : CreatureScript("npc_goblin_engineer") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_goblin_engineerAI(creature);
	}

	struct npc_goblin_engineerAI : public ScriptedAI
	{
		npc_goblin_engineerAI(Creature* creature) : ScriptedAI(creature)
		{
			instance = creature->GetInstanceScript();
		}

		InstanceScript* instance;
		void Reset()
		{
			if (!me)
				return;

			if (me->FindNearestGameObject(GO_FOE_ENTRANCE, 20.0f))
				me->AddAura(78087, me);
			else
				me->AddAura(57626, me);
		}

		void UpdateAI(uint32 const uiDiff) { }
	};
};

enum eSpell
{
	SPELL_EYE_GOUGE = 90913,
	SPELL_EYE_GOUGE_H = 90918,
	SPELL_EYE_PECK = 90920,
	SPELL_EYE_PECK_H = 90921,
};

class npc_deadmines_bird : public CreatureScript
{
public:
	npc_deadmines_bird() : CreatureScript("npc_deadmines_bird") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_deadmines_birdAI(creature);
	}

	struct npc_deadmines_birdAI : public ScriptedAI
	{
		npc_deadmines_birdAI(Creature* creature) : ScriptedAI(creature)
		{
			instance = creature->GetInstanceScript();
		}

		InstanceScript* instance;
		uint32 uiTimerEyePeck;
		uint32 uiTimerEyeGouge;

		void Reset()
		{
			uiTimerEyePeck = urand(4000, 4900);
			uiTimerEyeGouge = urand(7000, 9000);
		}

		void UpdateAI(uint32 const uiDiff)
		{
			if (!me)
				return;

			if (!UpdateVictim())
				return;

			if (uiTimerEyeGouge <= uiDiff)
			{
				if (Unit* victim = me->getVictim())
				{
					me->CastSpell(victim, IsHeroic() ? SPELL_EYE_GOUGE_H : SPELL_EYE_GOUGE);
				}
				uiTimerEyeGouge = urand(9000, 12000);
				return;
			}
			else
				uiTimerEyeGouge -= uiDiff;

			if (uiTimerEyePeck <= uiDiff)
			{
				if (Unit* victim = me->getVictim())
				{
					me->CastSpell(victim, IsHeroic() ? SPELL_EYE_PECK_H : SPELL_EYE_PECK);
				}
				uiTimerEyePeck = urand(16000, 19000);
				return;
			}
			else
				uiTimerEyePeck -= uiDiff;

			DoMeleeAttackIfReady();
		}
	};
};

/***
* TODO: this npc is also used in Ship and require 48522
*/
class npc_defias_cannon : public CreatureScript
{
public:
	npc_defias_cannon() : CreatureScript("npc_defias_cannon") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_defias_cannonAI(creature);
	}

	struct npc_defias_cannonAI : public ScriptedAI
	{
		npc_defias_cannonAI(Creature* creature) : ScriptedAI(creature), TargetGUID(0)
		{
			instance = creature->GetInstanceScript();
		}

		InstanceScript* instance;
		uint32 Phase;
		uint32 CannonBlast_Timer;
		uint64 TargetGUID;

		void Reset()
		{
			Phase = 0;
			CannonBlast_Timer = DATA_CANNON_BLAST_TIMER;
			if (!me)
				return;

			if (!ObjectAccessor::GetCreature(*me, TargetGUID))
				GetCreature();
		}

		bool GetSupporter()
		{
			Creature* supporter = me->FindNearestCreature(NPC_OGRE_HENCHMAN, 7.0f, true);
			if (supporter)
				return true;

			supporter = me->FindNearestCreature(NPC_DEFIAS_PIRATE, 5.0f, true);
			if (supporter)
				return true;

			return false;
		}

		void EnterCombat(Unit* /*who*/) {}

		void GetCreature()
		{
			if (!me)
				return;

			for (uint8 i = 0; i <= 7; i++)
			{
				if (me->IsInDist(&SourcePosition[i], 1.0f))
				{
					TargetGUID = me->SummonCreature(NPC_SCORCH_MARK_BUNNY_JMF, TargetPosition[i])->GetGUID();
					break;
				}
			}
		}

		void UpdateAI(uint32 const uiDiff)
		{
			if (!me)
				return;

			if (Phase == 0)
			{
				if (CannonBlast_Timer <= uiDiff)
				{
					if (!GetSupporter())
					{
						me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
						Phase++;
					}
					else if (Creature* target = ObjectAccessor::GetCreature(*me, TargetGUID))
					{
						me->CastSpell(target, SPELL_CANNONBALL);
					}
					CannonBlast_Timer = urand(3000, 5000);
				}
				else
					CannonBlast_Timer -= uiDiff;
			}
		}
	};
};

class go_heavy_door : public GameObjectScript
{
public:
	go_heavy_door() : GameObjectScript("go_heavy_door") {}

	void MoveNearCreature(GameObject* me, uint32 entry, uint32 ragne)
	{
		if (!me)
			return;

		std::list<Creature*> creature_list;
		me->GetCreatureListWithEntryInGrid(creature_list, entry, ragne);

		creature_list.sort(Trinity::ObjectDistanceOrderPred(me));
		for (std::list<Creature*>::iterator itr = creature_list.begin(); itr != creature_list.end(); ++itr)
		{
			if ((*itr) && (*itr)->isAlive() && (*itr)->GetTypeId() == TYPEID_UNIT && (*itr)->HasAura(78087))
			{
				(*itr)->GetMotionMaster()->MoveCharge(
					me->GetPositionX(),
					me->GetPositionY(),
					me->GetPositionZ(),
					5.0f
					);
				(*itr)->DespawnOrUnsummon(3000);
				(*itr)->AI()->Talk(0);
			}
		}
	}

	bool OnGossipHello(Player* player, GameObject* go)
	{
		if (!go || !player)
			return false;

		MoveNearCreature(go, 48439, 50.0f);
		MoveNearCreature(go, 48280, 50.0f);

		return true;
	}
};

/**
* explode door and say mobs after Door to attack tank...
*/
class go_defias_cannon : public GameObjectScript
{
public:
	go_defias_cannon() : GameObjectScript("go_defias_cannon") {}

	bool OnGossipHello(Player* player, GameObject* go)
	{
		if (!go || !player)
			return false;

		InstanceScript* instance = go->GetInstanceScript();
		if (GameObject* ironCladDoor = go->FindNearestGameObject(GO_IRON_CLAD_DOOR, 30.0f))
		{
			go->SetGoState(GO_STATE_ACTIVE);
			go->PlayDistanceSound(SOUND_CANNONFIRE, player);
			ironCladDoor->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);
			ironCladDoor->PlayDistanceSound(SOUND_DESTROYDOOR, player);

			MoveCreatureInside(go, NPC_DEFIAS_SHADOWGUARD);
			MoveCreatureInside(go, NPC_DEFIAS_ENFORCER);
			MoveCreatureInside(go, NPC_DEFIAS_BLOODWIZARD);
			if (Creature* bunny = go->SummonCreature(NPC_GENERAL_PURPOSE_BUNNY_JMF, go->GetPositionX(), go->GetPositionY(), go->GetPositionZ()))
			{
				bunny->AI()->Talk(0);
			}
		}
		return true;
	}

	void MoveCreatureInside(GameObject* go, uint32 entry)
	{
		if (!go || entry <= 0)
			return;

		if (Creature* defias = go->FindNearestCreature(entry, 20.0f))
		{
			defias->SetWalk(false);
			defias->GetMotionMaster()->MovePoint(0, -102.7f, -655.9f, defias->GetPositionZ());
		}
	}
};

void AddSC_deadmines()
{
    //new go_canon_defias();
    new boss_vancleef_event();
    new spell_magma_vehicle_dummy();
    new boss_glubtok_event();
    new spell_glaciate_dummy();
    //new npc_glubtok_icicle();
    new spell_gb_summon_icicle_target_selector();
    //new boss_helix_gearbreaker_event();
    new npc_vanessa_lighting_patter();
    // new boss_foe_reeper_event();
    new npc_heric_harringtom();
    new npc_emma_harringtom();
    new npc_callisa_harringtom();
    new npc_vanessa_paper();
    new go_deadmine_teleporter();
    new npc_vancleef_trap();
    new npc_trap_vapor_event();
    new go_liberate_singe();
    new npc_deadmine_minor();
    new npc_deadmine_defias_summoner();
	//new npc_defias_envoker();
	new npc_oaf_lackey();
	//new npc_goblin_overseer();
	new npc_mining_monkey();
	new npc_goblin_engineer();
	new npc_deadmines_bird();
	new npc_defias_cannon();
	new go_heavy_door();
	new go_defias_cannon();
}
