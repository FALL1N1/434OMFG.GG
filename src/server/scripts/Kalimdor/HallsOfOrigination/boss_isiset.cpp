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

#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "halls_of_origination.h"

enum Events
{
    EVENT_CELESTIAL_CALL                = 1,
    EVENT_ASTRAL_RAIN,
    EVENT_VEIL_OF_SKY,
    EVENT_SUPERNOVA
};

enum Spells
{
    SPELL_SUM_ASTRAL_RAIN               = 74262,
    SPELL_SUM_CELESTIAL_CALL            = 74263,
    SPELL_SUM_VEIL_OF_SKY               = 74261,

    SPELL_SUPERNOVA                     = 74136,
    SPELL_ENERGY_FLUX_SUMMON            = 74041,
    SPELL_ENERGY_FLUX_BEAM              = 74043,
    SPELL_ENERGY_FLUX_DAMAGE            = 74044,
    SPELL_VEIL_OF_SKY_DAMAGE            = 79370
};

enum eMisc
{
    DATA_RAIN,
    DATA_CALL,
    DATA_VEIL,

    DISPLAYID_INVISIBLE            = 11686,
};

enum Quotes
{
    SAY_AGGRO,
    SAY_DEATH,
    SAY_SLAY,
    SAY_SUPERNOVA,
    EMOTE_SUPERNOVA
};

// Different ids for 3 stages
static const uint32 SPELL_CELESTIAL_CALL[3] = {74362, 74355, 74364};
static const uint32 SPELL_ASTRAL_RAIN[3] = {74134, 74365, 74371};
static const uint32 SPELL_VEIL_OF_SKY[3] = {74133, 74372, 74373};
static const uint32 SPELL_SUM_AVATAR[3] = {SPELL_SUM_ASTRAL_RAIN, SPELL_SUM_CELESTIAL_CALL, SPELL_SUM_VEIL_OF_SKY};
static const uint32 DATA_AVATAR[3] = {DATA_ASTRAL_RAIN_GUID, DATA_CELESTIAL_CALL_GUID, DATA_VEIL_OF_SKY_GUID};


class boss_isiset : public CreatureScript
{
public:
    boss_isiset() : CreatureScript("boss_isiset") { }

    struct boss_isisetAI : public BossAI
    {
        boss_isisetAI(Creature* creature) : BossAI(creature, DATA_ISISET) { }

        void Reset()
        {
            stage = 0;
            split = false;
            memset(&abilities, true, sizeof(abilities));
            me->RestoreDisplayId();
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            _Reset();
        }

        void DamageTaken(Unit* /*done_by*/, uint32 &damage)
        {
            if (split)
                damage = 0;
            else if(stage < 2)
            {
                if (me->HealthBelowPctDamaged(stage ? 33 : 66, damage))
                {
                    ++stage;
                    split = true;
                    damage = 0;
                    me->SetReactState(REACT_PASSIVE);
                    me->RemoveAllAuras();
                    me->StopMoving();
                    me->SetDisplayId(DISPLAYID_INVISIBLE);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

                    // summon avatars
                    for (int i = 0; i < 3; ++i)
                    {
                        if (abilities[i])
                            DoCast(SPELL_SUM_AVATAR[i]);
                    }
                }
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            Talk(SAY_DEATH);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            _JustDied();
        }

        void KillerUnit(Unit* /*victim*/)
        {
            Talk(SAY_SLAY);
        }

        void JustSummoned(Creature* summon)
        {
            BossAI::JustSummoned(summon);
            switch (summon->GetEntry())
            {
            case NPC_ASTRAL_RAIN:
            case NPC_CELESTIAL_CALL:
            case NPC_VEIL_OF_SKY:
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, summon);
                break;
            case NPC_SPATIAL_FLUX:
                summon->SetReactState(REACT_PASSIVE);
                summon->StopMoving();
                summon->CastWithDelay(10000, summon, SPELL_ENERGY_FLUX_SUMMON, true, true);
                summon->CastWithDelay(10000, summon, SPELL_ENERGY_FLUX_BEAM, true, true);
                break;
            default:
                break;
            }
        }

        void EnterCombat(Unit* /*who*/)
        {
            Talk(SAY_AGGRO);
            events.ScheduleEvent(EVENT_CELESTIAL_CALL, urand(5000, 8000));
            events.ScheduleEvent(EVENT_ASTRAL_RAIN, urand(10000, 12000));
            events.ScheduleEvent(EVENT_VEIL_OF_SKY, urand(8000, 10000));
            events.ScheduleEvent(EVENT_SUPERNOVA, urand(15000, 20000));
            me->SummonCreature(NPC_SPATIAL_FLUX, -482.35f, 414.21f, 343.94f, 4.27f);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            _EnterCombat();
        }

        void SummonedCreatureDies(Creature* creature, Unit* /*killer*/)
        {
            if (split)
            {
                switch (creature->GetEntry())
                {
                case NPC_ASTRAL_RAIN:
                    abilities[DATA_RAIN] = false;
                    events.CancelEvent(EVENT_ASTRAL_RAIN);
                    break;
                case NPC_CELESTIAL_CALL:
                    abilities[DATA_CALL] = false;
                    events.CancelEvent(EVENT_CELESTIAL_CALL);
                    break;
                case NPC_VEIL_OF_SKY:
                    abilities[DATA_VEIL] = false;
                    events.CancelEvent(EVENT_VEIL_OF_SKY);
                    break;
                default:
                    return;
                }

                for (int i = 0; i < 3; ++i)
                    if (Creature* avatar = Creature::GetCreature(*me, instance->GetData64(DATA_AVATAR[i])))
                    {
                        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, avatar);
                        avatar->DespawnOrUnsummon();
                    }

                split = false;
                me->RestoreDisplayId();
                me->SetReactState(REACT_AGGRESSIVE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                DoStartMovement(me->getVictim());
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim() || split)
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_CELESTIAL_CALL:
                    DoCast(SPELL_CELESTIAL_CALL[stage]);
                    events.ScheduleEvent(EVENT_CELESTIAL_CALL, urand(20000, 30000));
                    break;
                case EVENT_VEIL_OF_SKY:
                    DoCast(me, SPELL_VEIL_OF_SKY[stage], false);
                    events.ScheduleEvent(EVENT_VEIL_OF_SKY, urand(20000, 30000));
                    break;
                case EVENT_ASTRAL_RAIN:
                    DoCast(SPELL_ASTRAL_RAIN[stage]);
                    events.ScheduleEvent(EVENT_ASTRAL_RAIN, stage == 2 ? 50000 : urand(15000, 20000));
                    break;
                case EVENT_SUPERNOVA:
                    Talk(SAY_SUPERNOVA);
                    Talk(EMOTE_SUPERNOVA);
                    DoCast(SPELL_SUPERNOVA);
                    events.ScheduleEvent(EVENT_SUPERNOVA, urand(20000, 25000));
                    break;
                }
            }

            DoMeleeAttackIfReady();

            EnterEvadeIfOutOfCombatArea(diff);
        }
    private:
        uint8 stage;
        bool split;
        bool abilities[3];
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_isisetAI(creature);
    }
};

class DontLooksDirectlyInGazeCheck
{
public:
    DontLooksDirectlyInGazeCheck(WorldObject* caster) : _caster(caster) { }

    bool operator() (WorldObject* unit)
    {
        Position pos;
        _caster->GetPosition(&pos);
        return !unit->HasInArc(static_cast<float>(M_PI), &pos);
    }

private:
    WorldObject* _caster;
};

class npc_energy_flux : public CreatureScript
{
public:
    npc_energy_flux() : CreatureScript("npc_energy_flux") { }

    struct npc_energy_fluxAI : public ScriptedAI
    {
        npc_energy_fluxAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            me->DespawnOrUnsummon(7000);
        }

        void EnterEvadeMode()
        {
            ScriptedAI::EnterEvadeMode();
        }

        void Reset()
        {
            DoCast(SPELL_ENERGY_FLUX_DAMAGE);
            if (Unit* victim = me->FindNearestPlayer(20.0f))
            {
                me->SetDisplayId(DISPLAYID_INVISIBLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->GetMotionMaster()->Clear();
                me->AddThreat(victim, 1.0f);
                me->GetMotionMaster()->MoveFollow(victim, 0.0f, 0.0f);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_energy_fluxAI(creature);
    }
};

class npc_spatial_flux : public CreatureScript
{
public:
    npc_spatial_flux() : CreatureScript("npc_spatial_flux") { }

    struct npc_spatial_fluxAI : public ScriptedAI
    {
        npc_spatial_fluxAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void UpdateAI(uint32 const diff)
        {}


    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_spatial_fluxAI(creature);
    }
};

class spell_isiset_supernova : public SpellScriptLoader
{
public:
    spell_isiset_supernova() : SpellScriptLoader("spell_isiset_supernova") { }

    class spell_isiset_supernova_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_isiset_supernova_SpellScript);

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(DontLooksDirectlyInGazeCheck(GetCaster()));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_isiset_supernova_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_isiset_supernova_SpellScript();
    }
};

class spell_veil_of_sky : public SpellScriptLoader
{
public:
    spell_veil_of_sky() : SpellScriptLoader("spell_veil_of_sky") { }

    class spell_veil_of_sky_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_veil_of_sky_AuraScript);

        void HandleAfterEffectManaShield(AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& absorbAmount)
        {
            if (aurEff && dmgInfo.GetAttacker() && (dmgInfo.GetSchoolMask() & SPELL_SCHOOL_MASK_MAGIC))
            {
                const SpellInfo* veil = sSpellMgr->GetSpellInfo(aurEff->GetId());
                const SpellEffectInfo dummy = veil->Effects[EFFECT_1];
                if (dummy.BasePoints)
                {
                    int32 amount = dmgInfo.GetAbsorb() * dummy.BasePoints * 0.01f;
                    if (GetOwner()->GetTypeId() == TYPEID_UNIT)
                        GetOwner()->ToUnit()->CastCustomSpell(dmgInfo.GetAttacker(), SPELL_VEIL_OF_SKY_DAMAGE, &amount, 0, 0, true);
                }
            }
        }

        void Register()
        {
            AfterEffectManaShield += AuraEffectManaShieldFn(spell_veil_of_sky_AuraScript::HandleAfterEffectManaShield, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_veil_of_sky_AuraScript();
    }
};

void AddSC_boss_isiset()
{
    new boss_isiset();
    new npc_energy_flux();
    new npc_spatial_flux();
    new spell_isiset_supernova();
    new spell_veil_of_sky();
}
