/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "zulgurub.h"

enum eSpells
{
    // NPC_HIGHPRIEST_KILNARA
    SPELL_AURA_KILNARA_HOVER = 97459, // 22:28:18.000
    SPELL_AURA_KILNARA_HOVER_2 = 97428, // 22:28:18.000
    SPELL_WAVE_OF_AGONY = 98270, // dummy SPELL_WAVE_OF_AGONY_SUMMON
    SPELL_WAVE_OF_AGONY_SUMMON = 96461, // summon NPC_WAVE_OF_AGONY_STALKER
    SPELL_TEARS_OF_BLOOD = 96438,
    SPELL_WAIL_OF_SORROW = 96948,
    SPELL_SHADOW_BOLT = 96956,
    SPELL_LASH_OF_ANGUISH = 96423, // 96958
    SPELL_CAVE_IN = 97380,
    SPELL_PRIME_OF_BLESSING = 96559,
    SPELL_RAGE_OF_THE_ANCIENTS = 96896,
    SPELL_PRIMAL_AWAKING = 96530,
    SPELL_CAMOUFLAGE = 96594,
    SPELL_RAVAGE = 96592,
    SPELL_VENGEFULL_SMASH = 96954, // 96593
    SPELL_SHADOWSTEP = 96683,

    // NPC_PRIDE_OF_BETHEKK
    SPELL_DARK_SLUMBER = 96446,
    SPELL_GAPING_WOUND = 97355,

    // NPC_WAVE_OF_AGONY_STALKER
    SPELL_WAVE_OF_AGONY_TRIGGER_SUMMON = 96457,
    SPELL_WAVE_OF_AGONY_EFF = 96542,
    SPELL_WAVE_OF_AGONY_PERIODIC_DUMMY = 96465,

    // NPC_WAVE_OF_AGONY
    SPELL_WAVE_OF_AGONY_AURA = 96532,

    // NPC_CAVE_IN_STALKER
    SPELL_CAVE_IN_AURA = 96935,

    SPELL_FELIN_JUMP = 97355,
};

enum eNpcs
{
    NPC_HIGHPRIEST_KILNARA = 52059,
    NPC_PRIDE_OF_BETHEKK = 52061,
    NPC_WAVE_OF_AGONY_STALKER = 52160,
    NPC_WAVE_OF_AGONY = 52147,
    NPC_CAVE_IN_STALKER = 52387,
};

enum eEvents
{
    EVENT_WAVE_OF_AGONY         = 1,
    EVENT_RETURN_TO_FIGHT       = 2,
    EVENT_TEARS_OF_BLOOD        = 3,
    EVENT_WAIL_OF_SORROW        = 4,
    EVENT_SHADOW_BOLT           = 5,
    EVENT_LASH_OF_ANGUISH       = 6,
    EVENT_MIGHT_OF_BETHEKK      = 7,
    EVENT_PRIMAL_BLESSING       = 8,
    EVENT_PRIMAL_AWAKENING      = 9,
    EVENT_CAMOUFLAGE            = 10,
    EVENT_CAMOUFLAGE_HUNTING    = 11,
    EVENT_CAMOUFLAGE_REMOVE     = 12,
    EVENT_RAVAGE                = 13,
    EVENT_VENGEFUL_SMASH        = 14,
    EVENT_CHECK_ROOM_POSITION   = 15,

    EVENT_GAPING_WOUND          = 1,
    EVENT_AWAKE                 = 2,
};

enum eTexts
{
    SAY_AGGRO                   = 0,
    SAY_WAVE_OF_AGONY           = 1,
    SAY_TRANSFROM_1             = 2,
    SAY_TRANSFROM_2             = 3,
    SAY_DEATH                   = 4,
    SAY_PLAYER_KILL             = 5,
    SAY_PLAYER_KILL_FERAL       = 6,
};

enum eMiscs
{
    ACTION_WAVE_OF_AGONY        = 1,
    TYPE_WAVE_OF_AGONY,
};

const Position CaveInSP[13]=
{
    {-11531.5f, -1605.48f, 44.4849f, 0.0f},
    {-11538.2f, -1645.08f, 44.4849f, 0.0f},
    {-11508.8f, -1604.29f, 44.4849f, 0.0f},
    {-11495.3f, -1621.67f, 44.4849f, 0.0f},
    {-11560.4f, -1627.20f, 44.4849f, 0.0f},
    {-11539.6f, -1635.53f, 44.4849f, 0.0f},
    {-11510.0f, -1619.33f, 44.4849f, 0.0f},
    {-11529.0f, -1626.86f, 44.4849f, 0.0f},
    {-11502.0f, -1632.88f, 44.4849f, 0.0f},
    {-11491.3f, -1634.09f, 44.4849f, 0.0f},
    {-11519.9f, -1640.14f, 44.4849f, 0.0f},
    {-11528.5f, -1648.30f, 44.4849f, 0.0f},
    {-11539.6f, -1617.73f, 44.4849f, 0.0f},
};

const Position PrideofBethekkSP[16]=
{
    {-11519.2f, -1605.37f, 44.4849f, 3.56047f},
    {-11518.1f, -1651.48f, 44.4849f, 2.26893f},
    {-11505.6f, -1607.56f, 44.4849f, 2.26893f},
    {-11521.8f, -1651.58f, 44.4849f, 0.97738f},
    {-11504.3f, -1645.56f, 44.4849f, 4.66003f},
    {-11517.2f, -1646.82f, 44.4849f, 3.87463f},
    {-11504.6f, -1603.33f, 44.4849f, 3.87463f},
    {-11506.6f, -1651.04f, 44.4849f, 1.60570f},
    {-11507.0f, -1644.55f, 44.4849f, 4.78220f},
    {-11519.7f, -1609.00f, 44.4849f, 2.26893f},
    {-11523.2f, -1605.96f, 44.4849f, 5.41052f},
    {-11523.2f, -1609.31f, 44.4849f, 0.97738f},
    {-11504.1f, -1650.26f, 44.4849f, 2.26893f},
    {-11520.6f, -1646.00f, 44.4849f, 4.95674f},
    {-11508.3f, -1607.37f, 44.4849f, 0.97738f},
    {-11508.7f, -1603.38f, 44.4849f, 5.41052f},
};

const float RoomCenter[2] = {-11518.8f, -1627.16f};

class boss_kilnara : public CreatureScript
{
    public:
        boss_kilnara() : CreatureScript("boss_kilnara") { }

    private:
        struct boss_kilnaraAI : public BossAI
        {
            boss_kilnaraAI(Creature* creature) : BossAI(creature, DATA_KILNARA) { }

            uint64 WaveofAgonyTargetGUID;
            bool MightofBethekk;

            void Reset()
            {
                _Reset();
                me->SetHover(true);
                me->CastWithDelay(1000, me, SPELL_AURA_KILNARA_HOVER_2, false);
                me->CastWithDelay(1000, me, SPELL_AURA_KILNARA_HOVER, false);
                me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, 43219);
                //  me->SetCurrentEquipmentId(43219);
                me->SetReactState(REACT_AGGRESSIVE);
                WaveofAgonyTargetGUID = 0;
                MightofBethekk = false;
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                for (int i = 0; i < 16; ++i)
                    me->SummonCreature(NPC_PRIDE_OF_BETHEKK, PrideofBethekkSP[i]);
            }

            void JustSummoned(Creature* summon)
            {
                switch (summon->GetEntry())
                {
                    case NPC_WAVE_OF_AGONY_STALKER:
                    {
                        me->CastSpell(summon, SPELL_WAVE_OF_AGONY_TRIGGER_SUMMON, false);
                        WaveofAgonyTargetGUID = summon->GetGUID();
                        break;
                    }
                    case NPC_WAVE_OF_AGONY:
                    {
                        summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        summon->CastSpell(summon, SPELL_WAVE_OF_AGONY_AURA, true);
                        break;
                    }
                    case NPC_CAVE_IN_STALKER:
                        summon->CastSpell(summon, SPELL_CAVE_IN_AURA, false);
                        return;
                }

                summons.Summon(summon);
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_WAVE_OF_AGONY)
                {
                    if (Creature* target = ObjectAccessor::GetCreature(*me, WaveofAgonyTargetGUID))
                    {
                        WaveofAgonyTargetGUID = 0;
                        target->DespawnOrUnsummon(500);
                        if (Creature *wave = me->FindNearestCreature(NPC_WAVE_OF_AGONY, 50.0f))
                            wave->DespawnOrUnsummon(500);
                    }
                    events.ScheduleEvent(EVENT_RETURN_TO_FIGHT, 1000);
                }
            }

            uint64 GetGUID(int32 type) const
            {
                if (type == TYPE_WAVE_OF_AGONY)
                    return WaveofAgonyTargetGUID;
                return 0;
            }

            void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/)
            {
                if (!MightofBethekk && me->GetHealthPct() <= 50.0f)
                {
                    MightofBethekk = true;
                    events.Reset();
                    events.ScheduleEvent(EVENT_CHECK_ROOM_POSITION, 1000);
                    events.ScheduleEvent(EVENT_MIGHT_OF_BETHEKK, 250);
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(SAY_AGGRO);
                me->SetHover(false);
                me->RemoveAura(SPELL_AURA_KILNARA_HOVER);
                me->RemoveAura(SPELL_AURA_KILNARA_HOVER_2);
                events.ScheduleEvent(EVENT_TEARS_OF_BLOOD, urand(5000, 10000));
                events.ScheduleEvent(EVENT_SHADOW_BOLT, urand(3000, 7000));
                events.ScheduleEvent(EVENT_LASH_OF_ANGUISH, urand(5000, 15000));
                events.ScheduleEvent(EVENT_CHECK_ROOM_POSITION, 1000);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(MightofBethekk ? SAY_PLAYER_KILL_FERAL : SAY_PLAYER_KILL);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }

            void GetInvisibleTeleportPosition(float &x, float &y, float &z)
            {

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
                    switch (eventId)
                    {
                        case EVENT_CHECK_ROOM_POSITION:
                        {
                            if (me->GetExactDist2dSq(RoomCenter[0], RoomCenter[1]) > 1500.0f)
                            {
                                ScriptedAI::EnterEvadeMode();
                                return;
                            }
                            events.ScheduleEvent(EVENT_CHECK_ROOM_POSITION, 1000);
                            break;
                        }
                        case EVENT_WAVE_OF_AGONY:
                            me->SetReactState(REACT_PASSIVE);
                            me->AttackStop();
                            me->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                            Talk(SAY_WAVE_OF_AGONY);
                            DoCastRandom(SPELL_WAVE_OF_AGONY_SUMMON, 0.0f, true);
                            events.ScheduleEvent(EVENT_TEARS_OF_BLOOD, urand(15000, 25000));
                            break;
                        case EVENT_TEARS_OF_BLOOD:
                            if (!me->HasReactState(REACT_PASSIVE))
                                DoCast(SPELL_TEARS_OF_BLOOD);
                            events.ScheduleEvent(EVENT_WAIL_OF_SORROW, 9000);
                            break;
                        case EVENT_WAIL_OF_SORROW:
                            if (!me->HasReactState(REACT_PASSIVE))
                                DoCast(SPELL_WAIL_OF_SORROW);
                            events.ScheduleEvent(EVENT_WAVE_OF_AGONY, urand(10000, 20000));
                            break;
                        case EVENT_SHADOW_BOLT:
                            if (!me->HasReactState(REACT_PASSIVE))
                                DoCastVictim(SPELL_SHADOW_BOLT);
                            events.ScheduleEvent(EVENT_SHADOW_BOLT, urand(3000, 7000));
                            break;
                        case EVENT_LASH_OF_ANGUISH:
                            if (!me->HasReactState(REACT_PASSIVE))
                                DoCastRandom(SPELL_LASH_OF_ANGUISH, 50.0f, true, -96958);
                            events.ScheduleEvent(EVENT_LASH_OF_ANGUISH, urand(5000, 15000));
                            break;
                        case EVENT_MIGHT_OF_BETHEKK:
                        {
                            for (int i = 0; i < 13; ++i)
                                me->SummonCreature(NPC_CAVE_IN_STALKER, CaveInSP[i], TEMPSUMMON_TIMED_DESPAWN, 6000);

                            Talk(SAY_TRANSFROM_1);
                            me->SetReactState(REACT_AGGRESSIVE);
                            DoCast(SPELL_CAVE_IN);
                            events.ScheduleEvent(EVENT_PRIMAL_BLESSING, 6000);
                            break;
                        }
                        case EVENT_PRIMAL_BLESSING:
                            DoCast(SPELL_PRIME_OF_BLESSING);
                            events.ScheduleEvent(EVENT_PRIMAL_AWAKENING, 2000);
                            break;
                        case EVENT_PRIMAL_AWAKENING:
                            Talk(SAY_TRANSFROM_2);
                            DoCast(SPELL_RAGE_OF_THE_ANCIENTS);
                            DoCast(SPELL_PRIMAL_AWAKING);
                            events.ScheduleEvent(EVENT_CAMOUFLAGE, urand(15000, 30000));
                            events.ScheduleEvent(EVENT_RAVAGE, urand(5000, 10000));
                            events.ScheduleEvent(EVENT_VENGEFUL_SMASH, urand(10000, 20000));
                            break;
                        case EVENT_CAMOUFLAGE:
                        {
                            DoCast(SPELL_CAMOUFLAGE);
                            events.Reset();
                            events.ScheduleEvent(EVENT_CHECK_ROOM_POSITION, 1000);
                            events.ScheduleEvent(EVENT_CAMOUFLAGE_HUNTING, 2500);
                            events.ScheduleEvent(EVENT_CAMOUFLAGE_REMOVE, 24000);
                            break;
                        }
                        case EVENT_CAMOUFLAGE_HUNTING:
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            {
                                me->CastSpell(target, SPELL_SHADOWSTEP, true);
                                me->RemoveAura(SPELL_VENGEFULL_SMASH);
                                me->RemoveAura(96594);
                                DoCast(target, SPELL_RAVAGE);
                                events.RescheduleEvent(EVENT_CAMOUFLAGE_REMOVE, 500);
                            }

                            break;
                        }
                        case EVENT_CAMOUFLAGE_REMOVE:
                            events.Reset();
                            events.ScheduleEvent(EVENT_CHECK_ROOM_POSITION, 1000);
                            events.ScheduleEvent(EVENT_CAMOUFLAGE, urand(15000, 30000));
                            events.ScheduleEvent(EVENT_RAVAGE, urand(5000, 10000));
                            events.ScheduleEvent(EVENT_VENGEFUL_SMASH, urand(10000, 20000));
                            break;
                        case EVENT_RAVAGE:
                            DoCastVictim(SPELL_RAVAGE);
                            events.ScheduleEvent(EVENT_RAVAGE, urand(10000, 20000));
                            break;
                        case EVENT_VENGEFUL_SMASH:
                            DoCast(SPELL_VENGEFULL_SMASH);
                            events.ScheduleEvent(EVENT_RAVAGE, urand(20000, 40000));
                            break;
                        case EVENT_RETURN_TO_FIGHT:
                            me->SetReactState(REACT_AGGRESSIVE);
                            break;
                    }
                }

                if (!me->HasReactState(REACT_PASSIVE))
                    DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_kilnaraAI(creature);
        }
};

class npc_pride_of_bethekk : public CreatureScript
{
    public:
        npc_pride_of_bethekk() : CreatureScript("npc_pride_of_bethekk") { }

    private:
        struct npc_pride_of_bethekkAI : public ScriptedAI
        {
            npc_pride_of_bethekkAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset()
            {
                Sleep = true;
                me->SetReactState(REACT_PASSIVE);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/)
            {
                if (Sleep)
                {
                    Sleep = false;
                    events.ScheduleEvent(EVENT_AWAKE, 300);
                }
            }

            void SpellHit(Unit* /*caster*/, SpellInfo const* spell)
            {
                if (Sleep && spell->Id == SPELL_PRIMAL_AWAKING)
                {
                    Sleep = false;
                    me->SetInCombatWithZone();
                    events.ScheduleEvent(EVENT_AWAKE, 3000);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_AWAKE:
                            me->RemoveAura(SPELL_DARK_SLUMBER);
                            me->SetReactState(REACT_AGGRESSIVE);
                            events.ScheduleEvent(EVENT_GAPING_WOUND, urand(3000, 7000));
                            break;
                        case EVENT_GAPING_WOUND:
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 50.0f, true))
                                me->CastSpell(target, SPELL_FELIN_JUMP, false, NULL, NULL, me->ToTempSummon()->GetSummoner()->GetGUID());

                            events.ScheduleEvent(EVENT_GAPING_WOUND, urand(5000, 15000));
                            break;
                        }
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            bool Sleep;
            EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_pride_of_bethekkAI(creature);
        }
};

class spell_wave_of_agony_selector : public SpellScriptLoader
{
    public:
        spell_wave_of_agony_selector() : SpellScriptLoader("spell_wave_of_agony_selector") { }

    private:
        class spell_wave_of_agony_selector_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_wave_of_agony_selector_SpellScript)

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                if (!unitList.empty())
                    Trinity::Containers::RandomResizeList(unitList, 1);
            }

            void TriggerSpell(SpellEffIndex effIndex)
            {
                GetCaster()->CastSpell(GetHitUnit(), GetSpellInfo()->Effects[effIndex].BasePoints, true);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_wave_of_agony_selector_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_wave_of_agony_selector_SpellScript::TriggerSpell, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_wave_of_agony_selector_SpellScript();
        }
};

class spell_wave_of_agony_periodic : public SpellScriptLoader
{
    public:
        spell_wave_of_agony_periodic() : SpellScriptLoader("spell_wave_of_agony_periodic") { }

    private:
        class spell_wave_of_agony_periodic_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_wave_of_agony_periodic_AuraScript)

            void HandlePeriodicTick(AuraEffect const* /*aurEff*/)
            {
                if (Creature* caster = GetCaster()->ToCreature())
                {
                    if (Unit *target = Unit::GetUnit(*caster, caster->AI()->GetGUID(TYPE_WAVE_OF_AGONY)))
                        caster->CastSpell(target, SPELL_WAVE_OF_AGONY_PERIODIC_DUMMY, true);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_wave_of_agony_periodic_AuraScript::HandlePeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_wave_of_agony_periodic_AuraScript();
        }
};

class spell_wave_of_agony_periodic_2 : public SpellScriptLoader
{
    public:
        spell_wave_of_agony_periodic_2() : SpellScriptLoader("spell_wave_of_agony_periodic_2") { }

    private:
        class spell_wave_of_agony_periodic_2_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_wave_of_agony_periodic_2_AuraScript)

            void HandlePeriodicTick(AuraEffect const* /*aurEff*/)
            {
                if (Creature* caster = GetCaster()->ToCreature())
                {
                    if (Unit *target = Unit::GetUnit(*caster, caster->AI()->GetGUID(TYPE_WAVE_OF_AGONY)))
                        caster->CastSpell(target, SPELL_WAVE_OF_AGONY_EFF, true);
                    caster->AI()->DoAction(ACTION_WAVE_OF_AGONY);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_wave_of_agony_periodic_2_AuraScript::HandlePeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_wave_of_agony_periodic_2_AuraScript();
        }
};

class spell_tears_of_blood : public SpellScriptLoader
{
    public:
        spell_tears_of_blood() : SpellScriptLoader("spell_tears_of_blood") { }

    private:
        class spell_tears_of_blood_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_tears_of_blood_SpellScript)

            void Trigger(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                GetCaster()->CastSpell(GetCaster(), GetSpellInfo()->Effects[effIndex].TriggerSpell, false);
            }

            void Register()
            {
                OnEffectLaunch += SpellEffectFn(spell_tears_of_blood_SpellScript::Trigger, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_tears_of_blood_SpellScript();
        }
};

// really ask me what is this effect for, cause this spell is already cast on effect 1 ...
class spell_gaping_wound : public SpellScriptLoader
{
public:
    spell_gaping_wound() : SpellScriptLoader("spell_gaping_wound") { }

private:
    class spell_gaping_wound_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_gaping_wound_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            GetCaster()->CastSpell(GetHitUnit(), GetEffectValue(), true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_gaping_wound_SpellScript::HandleScript, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_gaping_wound_SpellScript();
    }
};

void AddSC_boss_kilnara()
{
    new boss_kilnara();
    new npc_pride_of_bethekk();
    new spell_wave_of_agony_selector();
    new spell_wave_of_agony_periodic();
    new spell_wave_of_agony_periodic_2();
    new spell_tears_of_blood();
    //    new spell_gaping_wound();
}
