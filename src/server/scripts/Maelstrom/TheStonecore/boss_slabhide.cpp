/*
 * Copyright (C) 2013-2014 OMFG.GG Network <http://www.omfg.gg/>
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
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

#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "the_stonecore.h"

/*
 * TODO: in case the boss is done -  but we need for phase 2 a spline fix
 *       Achievements
 */
enum Spells
{
    SPELL_STALACTITE            = 80656,
    SPELL_STALACTITE_MISSILE    = 80643,
    SPELL_CRYSTAL_STORM         = 92265,
    SPELL_LAVA_FISSURE          = 80803,
    SPELL_LAVA_FISSURE_VISUAL   = 80798,
    SPELL_ERUPTION              = 80800,
    SPELL_SAND_BLAST            = 80807,
    SPELL_CRYSTAL_STORM_        = 92305,
    SPELL_CRYSTAL_STORM_REQ     = 92306,

    SPELL_STALACTITE_VISUAL     = 80654
};

enum Events
{
    EVENT_STALACTITE            = 1,
    EVENT_STALACTITE_CAST,
    EVENT_STALACTITE_LAND,
    EVENT_STALACTITE_LAND_END,
    EVENT_SAND_BLAST,
    EVENT_LAVA_FISSURE,
    EVENT_CRYSTAL_STORM,
};

enum Misc
{
    POINT_STALACTITE            = 1,
};

class boss_slabhide : public CreatureScript
{
public:
    boss_slabhide() : CreatureScript("boss_slabhide") {}

    struct boss_slabhideAI : public BossAI
    {
        boss_slabhideAI(Creature* creature) : BossAI(creature, DATA_SLABHIDE) {}

        void Reset()
        {
            me->SetCanFly(false);
            me->SetReactState(REACT_AGGRESSIVE);
            _Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_STALACTITE, 20000);
            events.ScheduleEvent(EVENT_SAND_BLAST, urand(5000, 8000));
            events.ScheduleEvent(EVENT_LAVA_FISSURE, urand(10000, 12000));
            _EnterCombat();
        }

        void JustSummoned(Creature* summon)
        {
            // Don't despawn triggers on death / evade
            if (summon->GetEntry() == NPC_STALACTITE_TRIGGER)
                return;

            if (summon->GetEntry() == NPC_LAVA_FISSURE)
                summon->CastSpell(summon, SPELL_LAVA_FISSURE_VISUAL, true);

            BossAI::JustSummoned(summon);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_STALACTITE:
                    events.DelayEvents(20000);
                    me->SetReactState(REACT_PASSIVE);
                    me->SetUInt64Value(UNIT_FIELD_TARGET, 0);
                    me->GetMotionMaster()->Clear();
                    me->SetHover(true);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LIFTOFF);
                    events.ScheduleEvent(EVENT_STALACTITE_CAST, 3000);
                    events.ScheduleEvent(EVENT_STALACTITE_LAND, 13000);
                    break;
                case EVENT_STALACTITE_CAST:
                    me->AddAura(SPELL_STALACTITE, me);
                    break;
                case EVENT_STALACTITE_LAND:
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
                    me->SetHover(false);
                    if (IsHeroic())
                    {
                        events.ScheduleEvent(EVENT_CRYSTAL_STORM, 6000);
                        events.ScheduleEvent(EVENT_STALACTITE_LAND_END, 2000);
                    }
                    else
                        events.ScheduleEvent(EVENT_STALACTITE_LAND_END, 1500);
                    break;
                case EVENT_STALACTITE_LAND_END:
                    me->SetReactState(REACT_AGGRESSIVE);
                    if (Unit * victim = me->getVictim())
                    {
                        me->SetUInt64Value(UNIT_FIELD_TARGET, victim->GetGUID());
                        DoStartMovement(victim);
                    }
                    events.ScheduleEvent(EVENT_STALACTITE, urand(35000, 40000));
                    break;
                case EVENT_SAND_BLAST:
                    DoCast(SPELL_SAND_BLAST);
                    events.ScheduleEvent(EVENT_SAND_BLAST, urand(12000, 15000));
                    break;
                case EVENT_LAVA_FISSURE:
                    {
                        DoCastRandom(SPELL_LAVA_FISSURE, 0.0f);
                        uint32 cd = IsHeroic() ? urand(5000, 7000) : urand(7000, 10000);
                        events.ScheduleEvent(EVENT_LAVA_FISSURE, cd);
                    }
                    break;
                case EVENT_CRYSTAL_STORM:
                    if (Unit * victim = me->getVictim())
                        me->SetUInt64Value(UNIT_FIELD_TARGET, victim->GetGUID());
                    DoCast(me, SPELL_CRYSTAL_STORM_REQ, true);
                    //                    me->AddAura(SPELL_CRYSTAL_STORM, me);
                    DoCast(me, SPELL_CRYSTAL_STORM, false);
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
        private:
            float x, y, z;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_slabhideAI(creature);
    }
};

class npc_stalactite_stalker : public CreatureScript
{
public:
    npc_stalactite_stalker() : CreatureScript("npc_stalactite_stalker") {}

    struct npc_stalactite_stalkerAI : public ScriptedAI
    {
        npc_stalactite_stalkerAI(Creature * creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }

        void Reset()
        {
            done = false;
            visual = false;
            visualTimer = 2000;
            summonTimer = 8000;
        }

        void IsSummonedBy(Unit* /*summoner*/)
        {
            me->GetPosition(x,y,z);
            me->GetMotionMaster()->Clear();
            if (me->FindNearestCreature(me->GetEntry(), 1.0f) != NULL) // more visual cleanup
                me->DespawnOrUnsummon();
            //me->NearTeleportTo(x, y, z + 50.0f, 0.0f);
            //me->SendMovementFlagUpdate();
        }

        void UpdateAI(uint32 const diff)
        {
            if (!visual)
            {
                if (visualTimer <= diff)
                {
                    visual = true;
                    me->CastSpell(x, y, z, SPELL_STALACTITE_VISUAL, true);
                }visualTimer -= diff;
            }

            if (!done)
            {
                if (summonTimer <= diff)
                {
                    done = true;
                    me->CastSpell(x, y, z, SPELL_STALACTITE_MISSILE, true);
                    me->DespawnOrUnsummon(30000);
                } else summonTimer -= diff;
            }
        }

    private:
        float x,y,z;
        bool done;
        bool visual;
        uint32 visualTimer;
        uint32 summonTimer;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_stalactite_stalkerAI(creature);
    }
};

class npc_lava_fissure : public CreatureScript
{
public:
    npc_lava_fissure() : CreatureScript("npc_lava_fissure") {}

    struct npc_lava_fissureAI : public ScriptedAI
    {
        npc_lava_fissureAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            active = false;
            activeTimer = IsHeroic() ? 3000 : 5000;
        }

        void UpdateAI(uint32 const diff)
        {
            if (!active)
            {
                if (activeTimer <= diff)
                {
                    active = true;
                    me->RemoveAllAuras();
                    DoCast(SPELL_ERUPTION);
                    me->DespawnOrUnsummon(IsHeroic() ? 30000 : 10000);
                }else activeTimer -= diff;
            }
        }

    private:
        bool active;
        uint32 activeTimer;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lava_fissureAI(creature);
    }
};

class spell_crystal_storm : public SpellScriptLoader
{
public:
    spell_crystal_storm() : SpellScriptLoader("spell_crystal_storm") { }

    class crystal_storm_filter
    {
    public:
        crystal_storm_filter(Unit* caster, std::list<GameObject*> const& stalagmiteList) : _stalagmiteList(stalagmiteList), _caster(caster) { }

        bool operator()(WorldObject* unit)
        {
            for (std::list<GameObject*>::const_iterator itr = _stalagmiteList.begin(); itr != _stalagmiteList.end(); ++itr)
                if ((*itr)->IsInBetween(_caster, unit, 1.0f) && _caster->GetDistance2d((*itr)->GetPositionX(),(*itr)->GetPositionY()) > 1.0f)
                    return true;

            return false;
        }

    private:
        std::list<GameObject*> const& _stalagmiteList;
        WorldObject* _caster;
    };

    class spell_crystal_storm_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_crystal_storm_SpellScript);

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            std::list<GameObject*> stalagmites;
            GetGameObjectListWithEntryInGrid(stalagmites, GetCaster(), GO_STALAGMITE, 100.0f);
            unitList.remove_if(crystal_storm_filter(GetCaster(), stalagmites));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_crystal_storm_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_crystal_storm_SpellScript();
    }
};

void AddSC_boss_slabhide()
{
    new boss_slabhide();
    new npc_stalactite_stalker();
    new npc_lava_fissure();
    new spell_crystal_storm();
};
