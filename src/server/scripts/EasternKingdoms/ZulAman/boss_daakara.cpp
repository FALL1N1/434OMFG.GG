/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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
SDName: Boss_ZulJin
SD%Complete: 85%
SDComment:
EndScriptData */

#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "zulaman.h"

enum Says
{
    YELL_INTRO                    = 0,
    YELL_AGGRO                    = 1,
    YELL_TRANSFORM_TO_BEAR        = 2,
    YELL_TRANSFORM_TO_EAGLE       = 3,
    YELL_TRANSFORM_TO_LYNX        = 4,
    YELL_TRANSFORM_TO_DRAGONHAWK  = 5,
    YELL_FIRE_BREATH              = 6,
    YELL_BERSERK                  = 7,
    YELL_KILL                     = 8,
    YELL_DEATH                    = 9
};

enum Spells
{
    // Troll Form
    SPELL_WHIRLWIND                 = 17207,
    SPELL_GRIEVOUS_THROW            = 97639, // remove debuff after full healed

    // Lynx Form
    SPELL_LYNX_FORM                 = 42607,
    SPELL_CLAW_RAGE                 = 43149,
    SPELL_CLAW_RAGE_DAMAGE          = 43150,
    SPELL_CLAW_RAGE_CHARGE          = 42583,
    SPELL_LYNX_RUSH                 = 97673,
    SPELL_LYNX_RUSH_DUMMY           = 43152,

    SPELL_SUMMON_AMANI_LYNX         = 97686,
    SPELL_FERAL_SWIPE               = 97479,
    SPELL_DETECT_INVISIBILITY       = 18950,

    // Bear Form
    SPELL_CREEPING_PARALYSIS        = 43095, // should cast on the whole raid
    SPELL_SURGE                     = 42402,
    SPELL_OVERPOWER                 = 43456, // use after melee attack dodged  22:32:09.237  22:32:14.463 22:32:22.513 22:32:28.519

    // Eagle Form 22:32:49.018
    SPELL_SUMMON_CYCLONE            = 97649, // 22:32:50.235
    SPELL_ENERGY_STORM              = 43983, // 22:32:50.235
    SPELL_SWEEPING_WINDS            = 97647, // 22:32:50.235 22:32:58.674 22:33:07.098 22:33:15.538 22:33:23.978 22:33:32.433
    SPELL_LIGHTNING_TOTEM           = 97930, // 22:32:58.674 22:33:15.538 22:33:32.433
    SPELL_FEATHER_CYCLONE           = 43120,
    SPELL_CYCLONE_VISUAL            = 97835,
    SPELL_SWEEPING_WINDS_MOVEMENT   = 97826,
    SPELL_ZAP                       = 43137,
    SPELL_LIGHTING                  = 97494,

    // Dragonhawk Form
    SPELL_FLAME_WHIRL               = 43213, // trigger two spells
    SPELL_FLAME_BREATH              = 97855,
    SPELL_FLAME_BREATH_GROUND       = 97497,
    SPELL_PILLAR_OF_FIRE            = 43216,

    // Transforms:
    SPELL_SPIRIT_BEAM               = 47767,
    SPELL_SHAPE_OF_THE_BEAR         = 42594, // 15% dmg
    SPELL_SHAPE_OF_THE_EAGLE        = 42606,
    SPELL_SHAPE_OF_THE_LYNX         = 42607, // haste melee 30%
    SPELL_SHAPE_OF_THE_DRAGONHAWK   = 42608,

    SPELL_BERSERK                   = 45078
};

//coords for going for changing form
#define CENTER_X 120.148811f
#define CENTER_Y 703.713684f
#define CENTER_Z 45.111477f

struct SpiritInfoStruct
{
    uint32 entry;
    float x, y, z, orient;
};

static SpiritInfoStruct SpiritInfo[4] =
{
    {23878, 147.87f, 706.51f, 45.11f, 3.04f},
    {23880, 88.95f, 705.49f, 45.11f, 6.11f},
    {23877, 137.23f, 725.98f, 45.11f, 3.71f},
    {23879, 104.29f, 726.43f, 45.11f, 5.43f}
};

struct TransformStruct
{
    uint8  text;
    uint32 spell, unaura;
};

static TransformStruct Transform[4] =
{
    {YELL_TRANSFORM_TO_BEAR, SPELL_SHAPE_OF_THE_BEAR, SPELL_WHIRLWIND},
    {YELL_TRANSFORM_TO_EAGLE, SPELL_SHAPE_OF_THE_EAGLE, SPELL_SHAPE_OF_THE_BEAR},
    {YELL_TRANSFORM_TO_LYNX, SPELL_SHAPE_OF_THE_LYNX, SPELL_SHAPE_OF_THE_EAGLE},
    {YELL_TRANSFORM_TO_DRAGONHAWK, SPELL_SHAPE_OF_THE_DRAGONHAWK, SPELL_SHAPE_OF_THE_LYNX}
};

enum Events
{
    // Troll Form
    EVENT_WHIRLWIND = 1,
    EVENT_GRIEVOUS_THROW,

    // Lynx Form
    EVENT_CLAW_RAGE_CHARGE,
    EVENT_CLAW_RAGE_DAMAGE,
    EVENT_LYNX_RUSH,
    EVENT_SUMMON_LYNX,

    EVENT_FERAL_SWIPE,

    // Bear Form
    EVENT_CREEPING_PARALYSIS,
    EVENT_SURGE,
    EVENT_OVERPOWER,

    // Eagle Form
    EVENT_LIGHTNING_TOTEM,
    EVENT_SUMMON_FEATHER_CYCLONE,
    EVENT_SWEEPING_WINDS,
    EVENT_SWEEPING_WINDS_MOVEMENT,

    // Dragonhawk Form
    EVENT_FLAME_WHIRL,
    EVENT_FLAME_BREATH,
    EVENT_SUMMON_FLAME_BREATH,
    EVENT_PILLAR_OF_FIRE,

    // Transforms
    EVENT_CHANGE_FORM_LYNX,
    EVENT_LYNX,
    EVENT_CHANGE_FORM_BEAR,
    EVENT_BEAR,
    EVENT_CHANGE_FORM_DRAGONHAWK,
    EVENT_DRAGONHAWK,
    EVENT_CHANGE_FORM_EAGLE,
    EVENT_EAGLE,

    EVENT_BERSERK,

    EVENT_CHECK_HEALTH,
};

enum Daa_CreatureIds
{
    NPC_AMANI_LYNX                  = 52839,
    NPC_FLAME_BREATH                = 467135,
    NPC_CYCLONE                     = 24136,
    // NPC_DAAKARA                     = 23863,
    NPC_CYCLONE_TARGET              = 52865,
    NPC_LIGHTING_TOTEM              = 24224,
    NPC_PILLAR_OF_FLAMMES           = 24187,
};

class boss_daakara : public CreatureScript
{
    public:
        boss_daakara() : CreatureScript("boss_daakara") { }

        struct boss_daakaraAI : public BossAI
        {
            boss_daakaraAI(Creature* creature) : BossAI(creature, DATA_DAAKARAEVENT)
            {
            }

            void Reset()
            {
                _Reset();
                events.Reset();

                phase = 0;

                me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, 33975);
                //me->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO, 218172674);
                //me->SetByteValue(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_MELEE);

                std::list<Creature*> unitList;
                me->GetCreatureListWithEntryInGrid(unitList, NPC_CYCLONE, 100.0f);
                for (std::list<Creature*>::const_iterator itr = unitList.begin(); itr != unitList.end(); ++itr)
                    (*itr)->DespawnOrUnsummon();
                prevGUID = 0;
                me->SetReactState(REACT_AGGRESSIVE);
                me->RemoveAura(SPELL_ENERGY_STORM);
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();

                DoZoneInCombat();
                events.ScheduleEvent(EVENT_GRIEVOUS_THROW, 10000);
                events.ScheduleEvent(EVENT_WHIRLWIND, 15000);

                Talk(YELL_INTRO);
            }

            void KilledUnit(Unit* /*victim*/)
            {

                Talk(YELL_KILL);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                std::list<Creature*> unitList;
                me->GetCreatureListWithEntryInGrid(unitList, NPC_CYCLONE, 100.0f);
                for (std::list<Creature*>::const_iterator itr = unitList.begin(); itr != unitList.end(); ++itr)
                {
                    (*itr)->DespawnOrUnsummon();
                }
                Talk(YELL_DEATH);

                if (me->GetMap()->IsHeroic())
                {
                   if (!me->GetMap()->GetPlayers().isEmpty())
                   {
                       for (Map::PlayerList::const_iterator i = me->GetMap()->GetPlayers().begin(); i != me->GetMap()->GetPlayers().end(); ++i)
                       {
                            if (!i->getSource())
                                continue;

                            i->getSource()->KilledMonsterCredit(23863);
                            i->getSource()->KilledMonsterCredit(23863);
                            i->getSource()->KilledMonsterCredit(23863);
                            i->getSource()->KilledMonsterCredit(23863);
                            i->getSource()->KilledMonsterCredit(23863);
                       }
                   }
                }
            }

            void JustSummoned(Creature* summon)
            {
                if (summon->GetEntry() == NPC_LIGHTING_TOTEM)
                {
                    summon->SetReactState(REACT_AGGRESSIVE);
                    summon->SetInCombatWithZone();
                }
                BossAI::JustSummoned(summon);
            }

            void DamageTaken(Unit* who, uint32 &damage)
            {
                if (damage > 0)
                {
                    if (me->HealthBelowPct(80) && phase == 0)
                    {
                        if (roll_chance_i(50))
                        {
                            phase++;
                            events.ScheduleEvent(EVENT_CHANGE_FORM_LYNX, 1);
                        }
                        else
                        {
                            phase++;
                            events.ScheduleEvent(EVENT_CHANGE_FORM_BEAR, 1);
                        }
                    }
                    else if (me->HealthBelowPct(40) && phase == 1)
                    {
                        if (me->HasAura(SPELL_SHAPE_OF_THE_LYNX))
                            events.ScheduleEvent(EVENT_CHANGE_FORM_DRAGONHAWK, 1);
                        else if (me->HasAura(SPELL_SHAPE_OF_THE_BEAR))
                            events.ScheduleEvent(EVENT_CHANGE_FORM_EAGLE, 1);
                        phase++;
                    }

                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_GRIEVOUS_THROW:
                            DoCastRandom(SPELL_GRIEVOUS_THROW, 0.0f);
                            events.ScheduleEvent(EVENT_GRIEVOUS_THROW, 15000);
                            break;
                        case EVENT_WHIRLWIND:
                            me->CastSpell(me, SPELL_WHIRLWIND, false);
                            events.ScheduleEvent(EVENT_WHIRLWIND, 15000);
                            break;
                        case EVENT_CHANGE_FORM_LYNX:
                            me->SetReactState(REACT_PASSIVE);
                            me->GetMotionMaster()->MovePoint(0, CENTER_X, CENTER_Y, CENTER_Z);
                            if (Creature* lynxSpirit = me->GetCreature(*me, instance->GetData64(NPC_LYNX_SPIRIT)))
                                lynxSpirit->CastSpell(me, SPELL_SPIRIT_BEAM, false);
                            events.ScheduleEvent(EVENT_LYNX, 2000);
                            break;
                        case EVENT_LYNX:
                            me->SetReactState(REACT_AGGRESSIVE);
                            me->GetMotionMaster()->MoveChase(me->getVictim());
                            me->CastSpell(me, SPELL_SHAPE_OF_THE_LYNX, false);
                            events.ScheduleEvent(EVENT_CLAW_RAGE_CHARGE, 20000);
                            events.ScheduleEvent(EVENT_LYNX_RUSH, 30000);
                            events.ScheduleEvent(EVENT_SUMMON_LYNX, 3000);
                            events.CancelEvent(EVENT_GRIEVOUS_THROW);
                            events.CancelEvent(EVENT_WHIRLWIND);
                            break;
                        case EVENT_SUMMON_LYNX:
                            me->CastSpell(me, SPELL_SUMMON_AMANI_LYNX, false);
                            break;
                        case EVENT_CLAW_RAGE_CHARGE:
                            if (Unit *target = SelectTarget(SELECT_TARGET_RANDOM, 1, NonSpecificTargetSelector(prevGUID)))
                            {
                                me->SetReactState(REACT_PASSIVE);
                                prevGUID = target->GetGUID();
                                me->GetMotionMaster()->MoveChase(target);
                                me->CastSpell(target, SPELL_CLAW_RAGE_CHARGE, true);
                                events.DelayEvents(5000);
                                events.ScheduleEvent(EVENT_CLAW_RAGE_DAMAGE, 500);
                            }
                            events.ScheduleEvent(EVENT_CLAW_RAGE_CHARGE, 30000);
                            break;
                        case EVENT_CLAW_RAGE_DAMAGE:
                            rush++;
                            if (rush <= 10)
                            {
                                if (Unit *target = Unit::GetUnit(*me, prevGUID))
                                    me->CastSpell(target, SPELL_CLAW_RAGE_DAMAGE, true);
                                events.ScheduleEvent(EVENT_CLAW_RAGE_DAMAGE, 500);
                            }
                            else
                            {
                                rush = 0;
                                prevGUID = 0;
                                me->SetReactState(REACT_AGGRESSIVE);
                                me->GetMotionMaster()->MoveChase(me->getVictim());
                            }
                            break;
                        case EVENT_LYNX_RUSH:
                            rush++;
                            if (rush == 1)
                                events.DelayEvents(5000);
                            if (rush <= 8)
                            {
                                if (Unit *target = SelectTarget(SELECT_TARGET_RANDOM, 0, NonSpecificTargetSelector(prevGUID)))
                                {
                                    prevGUID = target->GetGUID();
                                    me->CastSpell(target, SPELL_LYNX_RUSH, false);
                                }
                                events.ScheduleEvent(EVENT_LYNX_RUSH, 500);
                            }
                            else
                            {
                                rush = 0;
                                prevGUID = 0;
                                events.ScheduleEvent(EVENT_LYNX_RUSH, 25000);
                            }
                            break;
                        case EVENT_CHANGE_FORM_BEAR:
                            me->GetMotionMaster()->MovePoint(0, CENTER_X, CENTER_Y, CENTER_Z);

                            if (Creature* bearSpirit = me->GetCreature(*me, instance->GetData64(NPC_BEAR_SPIRIT)))
                                bearSpirit->CastSpell(me, SPELL_SPIRIT_BEAM, false);

                            events.ScheduleEvent(EVENT_BEAR, 2000);
                            break;
                        case EVENT_BEAR:
                            me->CastSpell(me, SPELL_SHAPE_OF_THE_BEAR, false);
                            me->GetMotionMaster()->MoveChase(me->getVictim());

                            events.ScheduleEvent(EVENT_CREEPING_PARALYSIS, 1000);
                            events.ScheduleEvent(EVENT_OVERPOWER, 7000);
                            events.ScheduleEvent(EVENT_SURGE, 8000);

                            events.CancelEvent(EVENT_GRIEVOUS_THROW);
                            events.CancelEvent(EVENT_WHIRLWIND);
                            break;
                        case EVENT_CREEPING_PARALYSIS:
                            me->CastSpell(me, SPELL_CREEPING_PARALYSIS, false);
                            events.ScheduleEvent(EVENT_CREEPING_PARALYSIS, 25000);
                            break;
                        case EVENT_OVERPOWER:
                            DoCastVictim(SPELL_OVERPOWER);
                            events.ScheduleEvent(EVENT_OVERPOWER, urand(5000, 8000));
                            break;
                        case EVENT_SURGE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0))
                                me->CastSpell(target, SPELL_SURGE, false);
                            events.ScheduleEvent(EVENT_SURGE, 8000);
                            break;
                        case EVENT_CHANGE_FORM_DRAGONHAWK:
                            me->RemoveAura(SPELL_SPIRIT_BEAM);
                            me->RemoveAura(SPELL_SHAPE_OF_THE_LYNX);
                            me->GetMotionMaster()->MovePoint(0, CENTER_X, CENTER_Y, CENTER_Z);

                            if (Creature* dragonhawkSpirit = me->GetCreature(*me, instance->GetData64(NPC_DRAGONHAWK_SPIRIT)))
                                dragonhawkSpirit->CastSpell(me, SPELL_SPIRIT_BEAM, false);

                            events.ScheduleEvent(EVENT_DRAGONHAWK, 2000);
                            break;
                        case EVENT_DRAGONHAWK:
                            me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, 0);
                            me->CastSpell(me, SPELL_SHAPE_OF_THE_DRAGONHAWK, false);
                            me->GetMotionMaster()->MoveChase(me->getVictim());

                            events.ScheduleEvent(EVENT_FLAME_WHIRL, 5000);
                            events.ScheduleEvent(EVENT_FLAME_BREATH, 8000);
                            events.ScheduleEvent(EVENT_PILLAR_OF_FIRE, 10000);

                            events.CancelEvent(EVENT_CLAW_RAGE_CHARGE);
                            events.CancelEvent(EVENT_LYNX_RUSH);
                            break;
                        case EVENT_FLAME_WHIRL:
                            me->CastSpell(me, SPELL_FLAME_WHIRL, false);
                            events.ScheduleEvent(EVENT_FLAME_WHIRL, 12000);
                            break;
                        case EVENT_FLAME_BREATH:
                            DoCastVictim(SPELL_FLAME_BREATH);
                            events.ScheduleEvent(EVENT_FLAME_BREATH, 10000);
                            break;
                        case EVENT_PILLAR_OF_FIRE:
                            DoCastRandom(SPELL_PILLAR_OF_FIRE, 0.0f);
                            events.ScheduleEvent(EVENT_PILLAR_OF_FIRE, 5000);
                            break;
                        case EVENT_CHANGE_FORM_EAGLE:
                            me->RemoveAura(SPELL_SPIRIT_BEAM);
                            me->RemoveAura(SPELL_SHAPE_OF_THE_BEAR);
                            me->GetMotionMaster()->MovePoint(0, CENTER_X, CENTER_Y, CENTER_Z);

                            if (Creature* eagleSpirit = me->GetCreature(*me, instance->GetData64(NPC_EAGLE_SPIRIT)))
                                eagleSpirit->CastSpell(me, SPELL_SPIRIT_BEAM, false);

                            events.ScheduleEvent(EVENT_EAGLE, 2000);
                            break;
                        case EVENT_EAGLE:
                            me->CastSpell(me, SPELL_SHAPE_OF_THE_EAGLE, false);
                            me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, 0);
                            me->GetMotionMaster()->MoveChase(me->getVictim());

                            events.ScheduleEvent(EVENT_LIGHTNING_TOTEM, 9000);
                            events.ScheduleEvent(EVENT_SUMMON_FEATHER_CYCLONE, 1000);
                            events.ScheduleEvent(EVENT_SWEEPING_WINDS, 1100);

                            events.CancelEvent(EVENT_CREEPING_PARALYSIS);
                            events.CancelEvent(EVENT_OVERPOWER);
                            events.CancelEvent(EVENT_SURGE);
                            break;
                        case EVENT_LIGHTNING_TOTEM:
                            me->CastSpell(me, SPELL_LIGHTNING_TOTEM, false);
                            events.ScheduleEvent(EVENT_LIGHTNING_TOTEM, urand(8000, 10000));
                            break;
                        case EVENT_SUMMON_FEATHER_CYCLONE:
                        {
                            me->CastSpell(me, SPELL_SUMMON_CYCLONE, true);
                            me->CastSpell(me, SPELL_ENERGY_STORM, true);
                            break;
                        }
                        case EVENT_SWEEPING_WINDS:
                            me->CastSpell(me, SPELL_SWEEPING_WINDS, false);
                            events.ScheduleEvent(EVENT_SWEEPING_WINDS, 8500);
                            break;

                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            uint8 phase;
            uint8 claw;
            uint8 rush;
            float ownerOrientation;
            uint64 prevGUID;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_daakaraAI(creature);
        }
};

class mob_daakara_vortex : public CreatureScript
{
    public:
        mob_daakara_vortex() : CreatureScript("mob_daakara_vortex") { }

        struct mob_daakara_vortexAI : public ScriptedAI
        {
            mob_daakara_vortexAI(Creature* creature) : ScriptedAI(creature) {}

            EventMap events;

            void Reset()
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_PASSIVE);
                events.Reset();
            }

            void EnterCombat(Unit* /*target*/)
            { }

            void IsSummonedBy(Unit* summoner)
            {
                me->CastSpell(me, SPELL_CYCLONE_VISUAL, true);
                me->CastWithDelay(3000, me, SPELL_FEATHER_CYCLONE, true);
                events.ScheduleEvent(EVENT_SWEEPING_WINDS_MOVEMENT, 8000);
            }

            void JustSummoned(Creature* summon)
            {
                Position pos;
                summon->GetPosition(&pos);
                me->SetSpeed(MOVE_WALK, 2.0f);
                me->SetSpeed(MOVE_RUN, 2.0f);
                me->GetMotionMaster()->MovePoint(0, pos);
                summon->CastSpell(me, SPELL_SWEEPING_WINDS_MOVEMENT, true);
            }

            void UpdateAI(uint32 const diff)
            {
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_daakara_vortexAI(creature);
        }
};

class mob_amani_lynx : public CreatureScript
{
    public:
        mob_amani_lynx() : CreatureScript("mob_amani_lynx") { }

        struct mob_amani_lynxAI : public ScriptedAI
        {
            mob_amani_lynxAI(Creature* creature) : ScriptedAI(creature)
            {
                Instance = (InstanceScript*)creature->GetInstanceScript();
            }

            InstanceScript* Instance;
            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void JustSummoned(Creature* summon)
            {
                me->CastSpell(me, SPELL_DETECT_INVISIBILITY, false);
            }

            void EnterCombat(Unit* /*who*/)
            {
                DoZoneInCombat();

                events.ScheduleEvent(EVENT_FERAL_SWIPE, 15000);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FERAL_SWIPE:
                            me->CastSpell(me->getVictim(), SPELL_FERAL_SWIPE, false);
                            events.ScheduleEvent(EVENT_FERAL_SWIPE, 15000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_amani_lynxAI(creature);
        }
};

class spell_daakara_grievous_throw : public SpellScriptLoader
{
public:
    spell_daakara_grievous_throw() :  SpellScriptLoader("spell_daakara_grievous_throw") { }

    class spell_daakara_grievous_throw_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_daakara_grievous_throw_AuraScript);

        bool Load()
        {
            events.ScheduleEvent(EVENT_CHECK_HEALTH, 100);
            return true;
        }

        void OnUpdate(AuraEffect* aurEff, const uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CHECK_HEALTH:
                    {
                        if (GetUnitOwner()->GetHealth() == GetUnitOwner()->GetMaxHealth())
                        {
                            PreventDefaultAction();
                            Remove(AURA_REMOVE_BY_ENEMY_SPELL);
                        }
                        events.ScheduleEvent(EVENT_CHECK_HEALTH, 100);
                        break;
                    }
                }
            }
        }

        void Register()
        {
            OnEffectUpdate += AuraEffectUpdateFn(spell_daakara_grievous_throw_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_daakara_grievous_throw_AuraScript();
    }
};

class spell_daakara_charge : public SpellScriptLoader
{
public:
    spell_daakara_charge() : SpellScriptLoader("spell_daakara_charge") { }

    class spell_daakara_charge_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_daakara_charge_AuraScript);

        bool Load()
        {
            targetGUID = 0;
            return true;
        }

        void HandleApplyEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit *caster = GetCaster())
                if (Creature *daakara = caster->ToCreature())
                    if (Unit *target = daakara->AI()->SelectTarget(SELECT_TARGET_RANDOM, 1))
                    {
                        targetGUID = target->GetGUID();
                        caster->CastSpell(target, SPELL_CLAW_RAGE_CHARGE, true);
                    }
        }

        void OnUpdate(AuraEffect* aurEff)
        {
            PreventDefaultAction();
            if (Unit *owner = GetUnitOwner())
                if (Unit *target = Unit::GetUnit(*owner, targetGUID))
                    owner->CastSpell(target, SPELL_CLAW_RAGE_DAMAGE, true);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_daakara_charge_AuraScript::HandleApplyEffect, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_daakara_charge_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }

    private:
        uint64 targetGUID;
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_daakara_charge_AuraScript();
    }
};

class spell_daakara_summon_cyclon : public SpellScriptLoader
{
public:
    spell_daakara_summon_cyclon() : SpellScriptLoader("spell_daakara_summon_cyclon")
    {
    }

    class spell_daakara_summon_cyclon_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_daakara_summon_cyclon_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            GetHitUnit()->CastSpell(GetHitUnit(), uint32(GetEffectValue()), true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_daakara_summon_cyclon_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_daakara_summon_cyclon_SpellScript();
    }
};

class spell_daakara_sweeping_winds : public SpellScriptLoader
{
public:
    spell_daakara_sweeping_winds() : SpellScriptLoader("spell_daakara_sweeping_winds")
    {
    }

    class spell_daakara_sweeping_winds_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_daakara_sweeping_winds_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            GetHitUnit()->CastSpell(GetHitUnit(), uint32(GetEffectValue()), true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_daakara_sweeping_winds_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_daakara_sweeping_winds_SpellScript();
    }
};

class spell_daakara_sweeping_winds_2 : public SpellScriptLoader
{
public:
    spell_daakara_sweeping_winds_2() : SpellScriptLoader("spell_daakara_sweeping_winds_2")
    {
    }

    class spell_daakara_sweeping_winds_2_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_daakara_sweeping_winds_2_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (GetHitUnit() && GetHitUnit()->ToTempSummon())
            {
                if (Unit *target = GetHitUnit()->ToTempSummon()->GetSummoner())
                    GetHitUnit()->CastSpell(target, uint32(GetEffectValue()), true); // 97648 summon 52865
                else if (GetHitUnit() && GetHitUnit()->ToCreature())
                    GetHitUnit()->ToCreature()->DespawnOrUnsummon(1000);
            }
            else if (GetHitUnit() && GetHitUnit()->ToCreature())
                GetHitUnit()->ToCreature()->DespawnOrUnsummon(1000);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_daakara_sweeping_winds_2_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_daakara_sweeping_winds_2_SpellScript();
    }
};

class spell_daakara_wind_cyclon_dummy : public SpellScriptLoader
{
public:
    spell_daakara_wind_cyclon_dummy() : SpellScriptLoader("spell_daakara_wind_cyclon_dummy")
    {
    }

    class spell_daakara_wind_cyclon_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_daakara_wind_cyclon_dummy_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (Unit *caster = GetCaster())
            {
                std::list<Player *> _players = caster->GetPlayersInRange(5, true);
                for (std::list<Player *>::iterator itr = _players.begin(); itr != _players.end(); itr++)
                    if (Player *player = *itr)
                        if (player->HasUnitState(UNIT_STATE_CASTING))
                            caster->CastSpell(player, SPELL_ZAP, true);
            }
        }

        void Register()
        {
            OnEffectLaunch += SpellEffectFn(spell_daakara_wind_cyclon_dummy_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_daakara_wind_cyclon_dummy_SpellScript();
    }
};

void AddSC_boss_daakara()
{
    new boss_daakara();
    new mob_daakara_vortex();
    new mob_amani_lynx();
    new spell_daakara_grievous_throw();
    //    new spell_daakara_charge();
    new spell_daakara_summon_cyclon();
    new spell_daakara_sweeping_winds();
    new spell_daakara_sweeping_winds_2();
    new spell_daakara_wind_cyclon_dummy();
}
