/*
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
SDName: boss_Akilzon
SD%Complete: 0 %
SDComment: Missing timer for Call Lightning and Sound ID's
SQLUpdate:

EndScriptData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Cell.h"
#include "CellImpl.h"
#include "zulaman.h"
#include "Weather.h"

enum Spells
{
    // enter combat 21:56:37.967
    SPELL_STATIC_DISRUPTION             = 43622, //21:56:44.005 trigger SPELL_STATIC_DISRUPTION_TRIGGER 21:56:56.048 21:57:08.107 21:57:20.181
    SPELL_STATIC_DISRUPTION_TRIGGER     = 44008, //97298, trigger SPELL_STATIC_VISUAL
    SPELL_STATIC_VISUAL                 = 45265,

    SPELL_CALL_LIGHTNING                = 43661, // 97299 21:56:45.206 21:56:53.646 21:57:09.324 21:57:17.763

    SPELL_GUST_OF_WIND                  = 43621, // 97319,  21:56:46.423  21:57:10.525
    SPELL_TAKE_PASSENGER                = 98869, // casted on NPC_AMANI_KIDNAPPER by players 21:56:50.042 21:57:14.160
    SPELL_PLAYER_PLUCKED                = 97318, // casted on NPC_AMANI_KIDNAPPER by players 21:56:50.042 21:57:14.160

    SPELL_ELECTRIC_OVERLOAD_PRIMER      = 44735, // script effect SPELL_ELECTRICAL_STORM dummy SPELL_PLAYER_PLUCKED 21:57:24.565
    SPELL_ELECTRICAL_STORM              = 43648, // script effect 44734(not exist) trigger SPELL_ELECTRICAL_OVERLOAD_0
    SPELL_ELECTRICAL_OVERLOAD_0         = 43658, //97300, trigger SPELL_ELECTRICAL_OVERLOAD_1
    SPELL_ELECTRICAL_OVERLOAD_1         = 43657,
    SPELL_ELECTRIC_STORM_DUMMY          = 44007,
    SPELL_AUTO_TELE_CASTER              = 44006,
    SPELL_ELECTRIC_ARC                  = 43653,

    SPELL_BERSERK                       = 45078,

    // MOB_SOARING_EAGLE
    SPELL_EAGLE_SWOOP                   = 44732 // casted by 24858 21:56:52.039 21:56:54.457
};

enum Events
{
    EVENT_STATIC_DISRUPTION = 1,
    EVENT_CALL_LIGHTNING,
    EVENT_ELECTRICAL_STORM_PRE,
    EVENT_ELECTRICAL_STORM,
    EVENT_ELECTRICAL_STORM_END,
    EVENT_GUST_OF_WIND,
    EVENT_AKIL_EAGLE,
    EVENT_BERSERK,

    EVENT_MOVE_TO_TARGET,
    EVENT_MOVE_AROUND_CIRCLE,
};

enum Says
{
    SAY_AGGRO                   = 0,
    SAY_SUMMON                  = 1,
    SAY_INTRO                   = 2, // Not used in script
    SAY_ENRAGE                  = 3,
    SAY_KILL                    = 4,
    SAY_DEATH                   = 5
};

enum Misc
{
    SE_LOC_X_MAX                = 400,
    SE_LOC_X_MIN                = 335,
    SE_LOC_Y_MAX                = 1435,
    SE_LOC_Y_MIN                = 1370,
    POINT_MOVE_TARGET           = 1,
    NPC_AMANI_KIDNAPPER         = 52638,
    NPC_AKILZON_EAGLE           = 24858,
    ACTION_START_FIGHT          = 1,
};

static const Position centerPos = {376.35f, 1407.34f, 92.38f, 0.0f};

static const Position akilEagleSpawn[8] =
{
    {355.0323f, 1411.0f, 91.6518f},
    {356.8288f, 1422.785f, 91.71573f},
    {368.7743f, 1377.933f, 91.78314f},
    {361.3212f, 1380.682f, 91.76196f},
    {396.1938f, 1398.954f, 91.53065f},
    {395.2363f, 1391.221f, 91.78426f},
    {386.3385f, 1430.294f, 92.51662f},
    {379.344f, 1434.552f, 91.64027f},
};

class boss_akilzon : public CreatureScript
{
    public:
        boss_akilzon() : CreatureScript("boss_akilzon") { }

        struct boss_akilzonAI : public BossAI
        {
            boss_akilzonAI(Creature* creature) : BossAI(creature, DATA_AKILZONEVENT)
            {
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
                _Reset();
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                SetWeather(WEATHER_STATE_FINE, 0.0f);
                me->SetReactState(REACT_DEFENSIVE);
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                SetWeather(WEATHER_STATE_FOG, 0.0f);
                Talk(SAY_AGGRO);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                events.Reset();
                events.ScheduleEvent(EVENT_STATIC_DISRUPTION, 4000);
                events.ScheduleEvent(EVENT_CALL_LIGHTNING, 8000);
                events.ScheduleEvent(EVENT_ELECTRICAL_STORM_PRE, 37000);
                events.ScheduleEvent(EVENT_GUST_OF_WIND, 14000);
                events.ScheduleEvent(EVENT_AKIL_EAGLE, 18000);
                events.ScheduleEvent(EVENT_BERSERK, 600000);
            }

            void JustDied(Unit* /*killer*/)
            {
                SetWeather(WEATHER_STATE_FINE, 0.0f);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                _JustDied();
                Talk(SAY_DEATH);
            }

            void KilledUnit(Unit* /*victim*/)
            {
                Talk(SAY_KILL);
            }

            void SetWeather(uint32 weather, float grade)
            {
                Map* map = me->GetMap();
                if (!map->IsDungeon())
                    return;

                WorldPacket data(SMSG_WEATHER, (4+4+1));
                data << uint32(weather) << float(grade) << uint8(0);

                map->SendToPlayers(&data);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_STATIC_DISRUPTION:
                            if (Unit *target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100.0f, true, -SPELL_PLAYER_PLUCKED))
                                me->CastSpell(target, SPELL_STATIC_DISRUPTION, false);
                            events.ScheduleEvent(EVENT_STATIC_DISRUPTION, 12000);
                            break;
                        case EVENT_CALL_LIGHTNING:
                            DoCastVictim(SPELL_CALL_LIGHTNING);
                            events.ScheduleEvent(EVENT_CALL_LIGHTNING, 8000);
                            break;
                        case EVENT_GUST_OF_WIND:
                            if (Unit *target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100.0f, true, -SPELL_PLAYER_PLUCKED))
                                me->CastSpell(target, SPELL_GUST_OF_WIND, false);
                            events.ScheduleEvent(EVENT_GUST_OF_WIND, 24000);
                            break;
                        case EVENT_ELECTRICAL_STORM_PRE:
                            SetWeather(WEATHER_STATE_MEDIUM_RAIN, 0.75f);
                            events.ScheduleEvent(EVENT_ELECTRICAL_STORM_PRE, 43000);
                            events.ScheduleEvent(EVENT_ELECTRICAL_STORM, 7000);
                            break;
                        case EVENT_ELECTRICAL_STORM:
                            DoCast(SPELL_ELECTRIC_OVERLOAD_PRIMER);
                            events.ScheduleEvent(EVENT_ELECTRICAL_STORM_END, 8000);
                            break;
                        case EVENT_ELECTRICAL_STORM_END:
                            SetWeather(WEATHER_STATE_FOG, 0.0f);
                            break;
                        case EVENT_AKIL_EAGLE:
                            for (int i = 0; i < 8; i++)
                                if (Creature *c = me->SummonCreature(NPC_AKILZON_EAGLE, akilEagleSpawn[i]))
                                    c->AI()->DoAction(ACTION_START_FIGHT);
                            break;
                        case EVENT_BERSERK:
                            DoCast(SPELL_BERSERK);
                            break;
                    }
                }
                else
                    DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_akilzonAI(creature);
        }
};

class mob_akilzon_eagle : public CreatureScript
{
    public:
        mob_akilzon_eagle() : CreatureScript("mob_akilzon_eagle") { }

        struct mob_akilzon_eagleAI : public ScriptedAI
        {
            mob_akilzon_eagleAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset()
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetSpeed(MOVE_RUN, 1.2f);
                me->SetVisible(false);
                me->SetUnitMovementFlags(MOVEMENTFLAG_DISABLE_GRAVITY);
                events.Reset();
                ray = 15.0f;
            }

            void EnterCombat(Unit* /*who*/)
            {
                DoZoneInCombat();
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_START_FIGHT)
                {
                    ray = me->GetDistance2d(centerPos.m_positionX, centerPos.m_positionY);
                    me->SetVisible(true);
                    events.ScheduleEvent(EVENT_MOVE_AROUND_CIRCLE, 1000);
                    events.ScheduleEvent(EVENT_MOVE_TO_TARGET, urand(4000, 8000));
                }
            }

            void MoveInLineOfSight(Unit* /*who*/) {}

            void MovementInform(uint32 type, uint32 id)
            {
                if (type != POINT_MOTION_TYPE || id != POINT_MOVE_TARGET)
                    return;
                if (TargetGUID)
                {
                    if (Unit* target = Unit::GetUnit(*me, TargetGUID))
                        DoCast(target, SPELL_EAGLE_SWOOP, true);
                    TargetGUID = 0;
                    events.ScheduleEvent(EVENT_MOVE_AROUND_CIRCLE, 0);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_MOVE_TO_TARGET:
                        {
                            if (Creature *boss = me->FindNearestCreature(23574, 100.0f))
                                if (Unit *target = boss->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0))
                                {
                                    me->GetMotionMaster()->Clear();
                                    me->DisableSpline();
                                    TargetGUID = target->GetGUID();
                                    Position pos;
                                    target->GetPosition(&pos);
                                    me->GetMotionMaster()->MovePoint(POINT_MOVE_TARGET, pos.m_positionX, pos.m_positionY, me->GetPositionZ());
                                }
                            events.ScheduleEvent(EVENT_MOVE_TO_TARGET, urand(5000, 10000));
                            break;
                        }
                        case EVENT_MOVE_AROUND_CIRCLE:
                        {
                            me->GetMotionMaster()->MoveAroundPoint(centerPos, ray, urand(0, 1), uint8(300), 92.0f);
                            break;
                        }
                    }
                }
            }

        private:
            EventMap events;
            float ray;
            uint64 TargetGUID;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_akilzon_eagleAI(creature);
        }
};

class npc_amani_kidnapper : public CreatureScript
{
    public:
        npc_amani_kidnapper() : CreatureScript("npc_amani_kidnapper") { }

        struct npc_amani_kidnapperAI : public ScriptedAI
        {
            npc_amani_kidnapperAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset()
            {
                targetGUID = 0;
                me->SetReactState(REACT_PASSIVE);
            }

            void EnterCombat(Unit* /*who*/)
            {
                DoZoneInCombat();
            }

            void IsSummonedBy(Unit* summoner)
            {
                targetGUID = summoner->GetGUID();
                events.ScheduleEvent(EVENT_MOVE_TO_TARGET, 4000);
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type != POINT_MOTION_TYPE && id != POINT_MOVE_TARGET)
                    return;
                events.ScheduleEvent(EVENT_MOVE_AROUND_CIRCLE, 1000);
            }

            void UpdateAI(uint32 const diff)
            {
                // SPELL_PLAYER_PLUCKED
                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_MOVE_TO_TARGET:
                        {
                            if (Unit *target = Unit::GetUnit(*me, targetGUID))
                                target->CastSpell(me, SPELL_PLAYER_PLUCKED, true);
                            me->SetSpeed(MOVE_FLIGHT, 0.8f);
                            me->SetSpeed(MOVE_RUN, 0.8f);
                            me->SetSpeed(MOVE_WALK, 0.8f);
                            float x, y;
                            me->GetNearPoint2D(x, y, 10, me->GetOrientation());
                            me->GetMotionMaster()->MovePoint(POINT_MOVE_TARGET, x, y, me->GetPositionZ());
                            break;
                        }
                        case EVENT_MOVE_AROUND_CIRCLE:
                        {
                            me->GetMotionMaster()->MoveAroundPoint(centerPos, 10, urand(0, 1), uint8(300), 92.0f);
                            break;
                        }
                    }
                }
            }

        private:
            EventMap events;
            uint64 targetGUID;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_amani_kidnapperAI(creature);
        }
};

// 44735
class spell_electric_overload_primer : public SpellScriptLoader
{
public:
    spell_electric_overload_primer() : SpellScriptLoader("spell_electric_overload_primer")
    {
    }

    class spell_electric_overload_primer_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_electric_overload_primer_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            // SMSG_SET_PLAY_HOVER_ANIM
            GetHitUnit()->CastSpell(GetHitUnit(), SPELL_ELECTRIC_STORM_DUMMY, true);
            GetHitUnit()->AddUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY | MOVEMENTFLAG_ROOT | MOVEMENTFLAG_CAN_FLY);
            GetHitUnit()->SendMovementCanFlyChange();
            GetHitUnit()->NearTeleportTo(GetHitUnit()->GetPositionX(), GetHitUnit()->GetPositionY(), GetHitUnit()->GetPositionZ() + 10, GetHitUnit()->GetOrientation());
            //    GetHitUnit()->CastSpell(GetHitUnit(), SPELL_AUTO_TELE_CASTER, true);
            GetCaster()->CastSpell(GetHitUnit(), uint32(GetEffectValue()), true); // 43648 add aura ?
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_electric_overload_primer_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_electric_overload_primer_SpellScript();
    }
};

// 43648
class spell_electric_storm : public SpellScriptLoader
{
public:
    spell_electric_storm() : SpellScriptLoader("spell_electric_storm")
    {
    }

    class spell_electric_storm_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_electric_storm_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            GetTarget()->SetRooted(true);
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            GetTarget()->SetRooted(false);
            GetTarget()->RemoveUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY | MOVEMENTFLAG_ROOT | MOVEMENTFLAG_CAN_FLY);
            GetTarget()->SendMovementCanFlyChange();
            GetTarget()->CastSpell(GetTarget(), 53700, true);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_electric_storm_AuraScript::OnApply, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_electric_storm_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_electric_storm_AuraScript();
    }
};

// 43657
class spell_arcane_storm_damages : public SpellScriptLoader
{
public:
    spell_arcane_storm_damages() : SpellScriptLoader("spell_arcane_storm_damages") { }

    class DistanceCheck
    {
    public:
        explicit DistanceCheck(Unit* _caster) : caster(_caster) { }

        bool operator() (WorldObject* unit) const
        {
            if (caster->GetExactDist2d(unit) <= 5.0f)
                return true;
            return false;
        }

        Unit* caster;
    };


    class spell_arcane_storm_damages_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_arcane_storm_damages_SpellScript);

        bool Load()
        {
            return GetCaster()->GetTypeId() == TYPEID_PLAYER;
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (targets.empty())
                return;
            if (GetCaster())
            {
                targets.remove_if(DistanceCheck(GetCaster()));
                targets.remove(GetCaster());
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_arcane_storm_damages_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_arcane_storm_damages_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ALLY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_arcane_storm_damages_SpellScript();
    }
};

// 98869
class spell_eagle_take_passenger : public SpellScriptLoader
{
public:
    spell_eagle_take_passenger() : SpellScriptLoader("spell_eagle_take_passenger")
    {
    }

    class spell_eagle_take_passenger_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_eagle_take_passenger_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            GetHitUnit()->CastSpell(GetCaster(), uint32(GetEffectValue()), true); // SPELL_PLAYER_PLUCKED
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_eagle_take_passenger_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_eagle_take_passenger_SpellScript();
    }
};

class mob_akilzon_pre_event : public CreatureScript
{
    public:
        mob_akilzon_pre_event() : CreatureScript("mob_akilzon_pre_event") { }

        struct mob_akilzon_pre_eventAI : public ScriptedAI
        {
            mob_akilzon_pre_eventAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void Reset()
            {
                me->SetReactState(REACT_PASSIVE);
                _prGUID = 0;
            }

            void EnterCombat(Unit* /*who*/)
            {
                Talk(0);
                me->GetMotionMaster()->MovePath(me->GetEntry() * 100, false);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }


            void JustSummoned(Creature *summon)
            {
                if (summon->GetEntry() == 24159)
                    summon->GetMotionMaster()->MovePath(summon->GetEntry() * 100, false);
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type != WAYPOINT_MOTION_TYPE || id != 5)
                    return;
                me->SetVisible(false);
                if (Creature *pr = me->FindNearestCreature(24549, 100))
                {
                    _prGUID = pr->GetGUID();
                    pr->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    pr->SetReactState(REACT_AGGRESSIVE);
                }
                events.ScheduleEvent(EVENT_MOVE_TO_TARGET, 5000);
            }

            void UpdateAI(uint32 const diff)
            {
                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_MOVE_TO_TARGET:
                        {
                            if (Unit *pr = Unit::GetUnit(*me, _prGUID))
                            {
                                if (!pr->isAlive())
                                {
                                    me->DespawnOrUnsummon();
                                    return;
                                }
                            }
                            else
                                return;
                            for (int i = 0; i < 5; i++)
                                me->CastSpell(me, 43487, true);
                            events.ScheduleEvent(EVENT_MOVE_TO_TARGET, 60000);
                            break;
                        }
                    }
                }
            }

        private:
            EventMap events;
            uint64 _prGUID;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_akilzon_pre_eventAI(creature);
        }
};

void AddSC_boss_akilzon()
{
    new boss_akilzon();
    new mob_akilzon_eagle();
    new npc_amani_kidnapper();
    new spell_electric_overload_primer();
    new spell_electric_storm();
    new spell_arcane_storm_damages();
    new spell_eagle_take_passenger();

    new mob_akilzon_pre_event();
}
