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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "MoveSplineInit.h"
#include "SpellScript.h"
#include "zulgurub.h"

enum eSpells
{
    SPELL_NEUTRALIZE_CAULDRON = 97337, // casted by NPC_VENOMGUARD_DESTROYER, hit all unit in range 22:10:27.000

    // NPC_HIGHPRIEST_VENOXIX
    SPELL_VENOM_TOTEM_BEAM_RIGHT = 96537, // casted by NPC_GENERAL_BUNNY on NPC_HIGHPRIEST_VENOXIX 22:01:41.000
    SPELL_VENOM_TOTEM_BEAM_LEFT  = 96536, // casted by NPC_GENERAL_BUNNY on NPC_HIGHPRIEST_VENOXIX 22:09:43.000

    SPELL_HELTHISS_WORD = 96560, // casted by NPC_HIGHPRIEST_VENOXIX 22:11:11.000 just at enter combat
    SPELL_HELTHISS_WHISPER = 96466, // caster by NPC_HIGHPRIEST_VENOXIX 22:01:04.000 22:01:13.000 22:01:21.000 22:11:17.000 22:11:28.000  22:11:36.000
    SPELL_TOXIC_LINK_DUMMY = 96475,
    SPELL_TOXIC_LINK = 96477, // efetc 2 dummy 96489 on removed 22:11:28.000

    SPELL_BLESSING_OF_THE_SNAKE_GOD = 96512, // casted by NPC_HIGHPRIEST_VENOXIX 22:11:45.000 trigger 96602
    SPELL_SUMMON_POOL_OF_ACRID_TEARS = 96515, // summon NPC_PULL_OF_ACRID_TEARS 22:11:49.000  22:11:52.000  22:11:58.000  22:12:02.000 22:12:05.000 22:12:11.000 22:12:15.000
    SPELL_HERTHISS_BREATH = 96509, // 22:11:53.000 22:12:07.000

    SPELL_TRANSFORM_REMOVAL_PRIMER = 96617, // 22:12:19.000 NPC_HIGHPRIEST_VENOXIX
    // move to Position: X: -11989.41 Y: -1694.411 Z: 32.31588 NPC_HIGHPRIEST_VENOXIX
    SPELL_VENOXIS_COSMETIC_MISSILE = 96634, // 22:12:23.000 casted by NPC_PULL_OF_ACRID_TEARS on NPC_HIGHPRIEST_VENOXIX
    SPELL_BLOOD_VENOM = 96842, // 22:12:26.000 forecast SPELL_BLOOD_VENOM_PLAYERS NPC_HIGHPRIEST_VENOXIX
    SPELL_BLOOD_VENOM_PLAYERS = 96637, // summon NPC_BLOOD_VENOM
    SPELL_VENOM_WITHDRAWAL = 96653, // 22:12:43.000 NPC_HIGHPRIEST_VENOXIX

    // NPC_GENERAL_BUNNY
    SPELL_SERPENTS_EYES = 96533, // 22:00:17.000
    SPELL_KITI_EYES = 96885, // 22:00:17.000
    SPELL_POISON_CLOUD = 96729, // 22:01:44.000
    SPELL_POINSON_TOTEM = 96534, // 22:01:46.000

    // NPC_VENOMOUS_EFFUSION
    SPELL_VENOMOUS_EFFUSION_AURA = 96681, // 22:11:15.000

    // NPC_VENOMOUS_EFFUSION_STALKER
    SPELL_VENOMOUS_EFFUSION_STALKER_AURA = 96678, // 22:11:15.000 trigger SPELL_VENOMOUS_EFFUSION_STALKER_AURA
    SPELL_SUMMON_VENOMOUS_EFFUSION = 96680,


    // NPC_TIKI_TORCH
    SPELL_YOGIQUE_FLAMES_AURA = 97000,

    // NPC_PULL_OF_ACRID_TEARS
    SPELL_ACRID_TEARS_AURA = 96520, // 22:11:49.000

    // NPC_BLOOD_VENOM
    SPELL_BLOODY_VENOM_AURA = 97110, // on summon 22:12:29.000
    SPELL_VENOUM_TOTEM_BEAM_CENTER = 97098, // casted by NPC_BLOOD_VENOM on NPC_HIGHPRIEST_VENOXIX on summon
    SPELL_BLOODY_VENOM_PERIDOIC_AURA = 97099, // 22:12:32.000

};

enum eNpcs
{
    NPC_VENOMGUARD_DESTROYER = 52311,

    NPC_HIGHPRIEST_VENOXIX = 52155,
    NPC_GENERAL_BUNNY = 45979,
    NPC_VENOMOUS_EFFUSION = 52288,
    NPC_VENOMOUS_EFFUSION_STALKER = 52302,
    NPC_TIKI_TORCH = 52419, // aura 97000
    NPC_PULL_OF_ACRID_TEARS = 52197, // aura 96521
    NPC_BLOOD_VENOM = 52525,
};

enum eEvents
{
    EVENT_VENOMOUS_EFFUSION                     = 1,
    EVENT_REMOVE_EFFUSION_VISUAL,
    EVENT_WHISPERS_OF_HETHISS,
    EVENT_TOXIC_LINK,
    EVENT_POOL_OF_ACRID_TEARS,
    EVENT_BREATH_OF_HETHISS,
    EVENT_BREATH_OF_HETHISS_END,
    EVENT_CHECK_ROOM_POSITION,
    EVENT_ACRID_EAT,
    EVENT_BLOOD_VENOM,
    EVENT_VENOM_WITHDRAW,
    EVENT_PHASE_1,
    EVENT_PHASE_2,
    EVENT_PHASE_3,

    POIN_HOST,
    PHASE_1,
    PHASE_2,
    PHASE_3,
};

enum eTexts
{
    SAY_AGGRO               = 0,
    SAY_TOXIC_LINK          = 1,
    SAY_TRANSFROM           = 2,
    SAY_BLOODVENOM          = 3,
    EMOTE_BLOODVENOM        = 4,
    EMOTE_VENOM_WITHDRAWAL  = 5,
    SAY_DEATH               = 6,
    SAY_PLAYER_KILL         = 7,
};

const uint32 TotemSpell[2]=
{
    96537,
    96536,
};

const Position TotemSP[2]=
{
    {-12011.0f, -1705.97f, 43.4762f, 0.523599f},
    {-12021.8f, -1688.28f, 43.4524f, 0.558505f},
};

const Position EffusionSP[2]=
{
    { -12002.73f, -1699.402f, 32.40472f, 0.0f},
    { -12014.68f, -1683.281f, 32.3824f, 0.0f},
};

#define PATH_EFUSION_LEFT 5230200
#define PATH_EFUSION_RIGHT 5230300

const Position BoosSP = {-12020.4f, -1699.78f, 39.6222f, 0.698132f};

const float RoomCenter[2] = {-12006.8f, -1699.29f};

class boss_venoxis : public CreatureScript
{
    public:
        boss_venoxis() : CreatureScript("boss_venoxis") { }


        class BloodVenomEvent : public BasicEvent
        {
        public:
            BloodVenomEvent(Creature *c) : summon(c)
            {
            }

            bool Execute(uint64 /*execTime*/, uint32 /*diff*/)
            {
                summon->CastSpell(summon, SPELL_BLOODY_VENOM_PERIDOIC_AURA, true);
                summon->ClearUnitState(UNIT_STATE_CASTING);
                summon->SetReactState(REACT_AGGRESSIVE);
                summon->UpdateSpeed(MOVE_WALK, true);
                return true;
            }

        private:
            Creature* summon;
        };


        struct boss_venoxisAI : public BossAI
        {
            boss_venoxisAI(Creature* creature) : BossAI(creature, DATA_VENOXIS) { }

            uint64 TotemGUID[2];

            void InitializeAI()
            {
                for (int i = 0; i < 2; ++i)
                    if (Creature* totem = me->SummonCreature(NPC_GENERAL_BUNNY, TotemSP[i]))
                    {
                        TotemGUID[i] = totem->GetGUID();
                        totem->CastSpell(totem, SPELL_POINSON_TOTEM, true);
                        totem->CastSpell(me, TotemSpell[i], true);
                    }

                Reset();
                buffStairs = false;
                if (Creature *c = me->SummonCreature(NPC_GENERAL_BUNNY, -12008.38f, -1704.34f, 37.5f, 0.57f))
                    c->CastSpell(c, SPELL_SERPENTS_EYES, true);
                if (Creature *c = me->SummonCreature(NPC_GENERAL_BUNNY, -12019.38f, -1686.34f, 37.5f, 0.55f))
                    c->CastSpell(c, SPELL_SERPENTS_EYES, true);
                if (Creature *c = me->SummonCreature(NPC_GENERAL_BUNNY, -12025.06f, -1666.8f, 41.47248f, 5.04f))
                    c->CastSpell(c, SPELL_SERPENTS_EYES, true);
                if (Creature *c = me->SummonCreature(NPC_GENERAL_BUNNY, -12038.09f, -1698.07f, 47.80248f, 0.08f))
                    c->CastSpell(c, SPELL_SERPENTS_EYES, true);
                if (Creature *c = me->SummonCreature(NPC_GENERAL_BUNNY, -12024.70f, -1720.47f, 47.76f, 0.92f))
                    c->CastSpell(c, SPELL_SERPENTS_EYES, true);
                //  -12025.06 Y: -1666.8 Z: 41.47248
            }

            void Reset()
            {
                _Reset();
                me->SetReactState(REACT_AGGRESSIVE);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }

            void VenomousEffusion()
            {
                for (int i = 0; i < 2; ++i)
                    if (Creature* effusion_stalker = me->SummonCreature(NPC_VENOMOUS_EFFUSION_STALKER, EffusionSP[i]))
                    {
                        if (Creature* totem = ObjectAccessor::GetCreature(*me, TotemGUID[i]))
                            totem->CastSpell(effusion_stalker, TotemSpell[i], true);

                        effusion_stalker->CastSpell(effusion_stalker, SPELL_VENOMOUS_EFFUSION_STALKER_AURA, true, NULL, NULL, me->GetGUID());
                        if (i == 0)
                            effusion_stalker->GetMotionMaster()->MovePath(PATH_EFUSION_LEFT, false);
                        else
                            effusion_stalker->GetMotionMaster()->MovePath(PATH_EFUSION_RIGHT, false);
                    }

                events.ScheduleEvent(EVENT_REMOVE_EFFUSION_VISUAL, 7000);
            }

            void JustSummoned(Creature* summon)
            {
                switch (summon->GetEntry())
                {
                    case NPC_GENERAL_BUNNY:
                        break;
                    case NPC_PULL_OF_ACRID_TEARS:
                        summon->CastSpell(summon, SPELL_ACRID_TEARS_AURA, true);
                        break;
                    case NPC_BLOOD_VENOM:
                        summon->CastSpell(summon, SPELL_BLOODY_VENOM_AURA, true);
                        summon->CastSpell(me, SPELL_VENOUM_TOTEM_BEAM_CENTER, true);
                        summon->m_Events.AddEvent(new BloodVenomEvent(summon), summon->m_Events.CalculateTime(3000));
                        // POIN_HOST
                        break;
                    case NPC_VENOMOUS_EFFUSION:
                        summon->CastSpell(summon, SPELL_VENOMOUS_EFFUSION_AURA, true);
                        break;
                }

                BossAI::JustSummoned(summon);
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(SAY_AGGRO);
                DoCast(SPELL_HELTHISS_WORD);

                events.ScheduleEvent(EVENT_PHASE_1, 0);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                for (int i = 0; i < 2; ++i)
                    if (Creature* totem = ObjectAccessor::GetCreature(*me, TotemGUID[i]))
                        totem->CastStop();
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_PLAYER_KILL);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
                std::list<Creature*> stalker;
                me->GetCreatureListWithEntryInGrid(stalker, NPC_GENERAL_BUNNY, 50.0f);
                for (std::list<Creature *>::iterator itr = stalker.begin(); itr != stalker.end(); itr++)
                    if (Creature *c = *itr)
                        if (c->HasAura(SPELL_POISON_CLOUD))
                            c->RemoveAurasDueToSpell(SPELL_POISON_CLOUD);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type != POINT_MOTION_TYPE)
                    return;
                if (id == POIN_HOST)
                {
                    me->SetFacingTo(me->GetOrientation());
                    events.ScheduleEvent(EVENT_ACRID_EAT, 1000, 0, PHASE_3);
                    events.ScheduleEvent(EVENT_BLOOD_VENOM, 3000, 0, PHASE_3);
                    events.ScheduleEvent(EVENT_VENOM_WITHDRAW, 21000, 0, PHASE_3);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!buffStairs)
                {
                    std::list<Creature*> stalker;
                    me->GetCreatureListWithEntryInGrid(stalker, NPC_GENERAL_BUNNY, 50.0f);
                    for (std::list<Creature *>::iterator itr = stalker.begin(); itr != stalker.end(); itr++)
                        if (Creature *c = *itr)
                            if (!c->HasAura(SPELL_POISON_CLOUD))
                                c->CastSpell(c, SPELL_POISON_CLOUD, true);
                    buffStairs = true;
                }

                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_PHASE_1:
                        {
                            //  22:11:11.000
                            events.Reset();
                            events.SetPhase(PHASE_1);
                            events.ScheduleEvent(EVENT_WHISPERS_OF_HETHISS, 6000, 0, PHASE_1);
                            events.ScheduleEvent(EVENT_TOXIC_LINK, 15000, 0, PHASE_1);
                            events.ScheduleEvent(EVENT_PHASE_2, 34000, 0, PHASE_1);
                            events.ScheduleEvent(EVENT_VENOMOUS_EFFUSION, 1000);
                            events.ScheduleEvent(EVENT_CHECK_ROOM_POSITION, 1000);
                            break;
                        }
                        case EVENT_PHASE_2:
                        {
                            events.Reset();
                            events.SetPhase(PHASE_2);
                            DoCast(SPELL_BLESSING_OF_THE_SNAKE_GOD); // 22:11:45.000
                            events.ScheduleEvent(EVENT_POOL_OF_ACRID_TEARS, 4000, 0, PHASE_2);
                            events.ScheduleEvent(EVENT_BREATH_OF_HETHISS, 8000, 0, PHASE_2);
                            events.ScheduleEvent(EVENT_PHASE_3, 34000, 0, PHASE_2);
                            events.ScheduleEvent(EVENT_CHECK_ROOM_POSITION, 1000);
                            break;
                        }
                        case EVENT_PHASE_3:
                        {
                            events.Reset();
                            events.SetPhase(PHASE_3);
                            me->CastSpell(me, SPELL_TRANSFORM_REMOVAL_PRIMER, true); // 22:12:19.000
                            me->SetReactState(REACT_PASSIVE);
                            me->AttackStop();
                            Position pos = me->GetHomePosition();
                            me->GetMotionMaster()->MovePoint(POIN_HOST, pos); // -11989.41f, -1694.411f, 32.31588f, 0.0f);
                            events.ScheduleEvent(EVENT_CHECK_ROOM_POSITION, 1000);
                            break;
                        }
                        case EVENT_ACRID_EAT:
                        {
                            std::list<Creature*> stalker;
                            me->GetCreatureListWithEntryInGrid(stalker, NPC_PULL_OF_ACRID_TEARS, 200.0f);
                            for (std::list<Creature *>::iterator itr = stalker.begin(); itr != stalker.end(); itr++)
                                if (Creature *c = *itr)
                                {
                                    c->RemoveAura(SPELL_ACRID_TEARS_AURA);
                                    c->CastSpell(me, SPELL_VENOXIS_COSMETIC_MISSILE, false);
                                    c->DespawnOrUnsummon(7000);
                                }
                            break;
                        }
                        case EVENT_BLOOD_VENOM:
                        {
                            DoCast(SPELL_BLOOD_VENOM);
                            break;
                        }
                        case EVENT_VENOM_WITHDRAW:
                        {
                            me->SetReactState(REACT_AGGRESSIVE);
                            if (me->getVictim())
                                AttackStart(me->getVictim());
                            me->RemoveAurasDueToSpell(SPELL_BLESSING_OF_THE_SNAKE_GOD);
                            me->RemoveAurasDueToSpell(97354);
                            DoCast(SPELL_VENOM_WITHDRAWAL);
                            events.ScheduleEvent(EVENT_PHASE_1, 10000, 0, PHASE_3);
                            break;
                        }
                        case EVENT_CHECK_ROOM_POSITION:
                        {
                            if (me->GetExactDist2dSq(RoomCenter[0], RoomCenter[1]) > 1500.0f || !BoosSP.HasInArc(M_PI, me))
                            {
                                BossAI::EnterEvadeMode();
                                return;
                            }

                            events.ScheduleEvent(EVENT_CHECK_ROOM_POSITION, 1000);
                            break;
                        }
                        case EVENT_VENOMOUS_EFFUSION:
                            VenomousEffusion();
                            break;
                        case EVENT_REMOVE_EFFUSION_VISUAL:
                        {
                            for (int i = 0; i < 2; ++i)
                                if (Creature* totem = ObjectAccessor::GetCreature(*me, TotemGUID[i]))
                                    totem->CastStop();
                            break;
                        }
                        case EVENT_WHISPERS_OF_HETHISS:
                        {
                            DoCastRandom(SPELL_HELTHISS_WHISPER, 0.0f);
                            events.ScheduleEvent(EVENT_WHISPERS_OF_HETHISS, 9000, 0, PHASE_1);
                            break;
                        }
                        case EVENT_TOXIC_LINK:
                            Talk(SAY_TOXIC_LINK);
                            DoCast(SPELL_TOXIC_LINK_DUMMY); // 22:03:40.00 22:03:58.000  22:11:26.000
                            break;
                        case EVENT_POOL_OF_ACRID_TEARS:
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 150.0f, true))
                                me->CastSpell(target, SPELL_SUMMON_POOL_OF_ACRID_TEARS, false);
                            events.ScheduleEvent(EVENT_POOL_OF_ACRID_TEARS, urand(3000, 6000), 0, PHASE_2);
                            break;
                        }
                        case EVENT_BREATH_OF_HETHISS:
                        {
                            me->SetReactState(REACT_PASSIVE);
                            me->AttackStop();
                            DoCast(SPELL_HERTHISS_BREATH);
                            events.ScheduleEvent(EVENT_BREATH_OF_HETHISS_END, 4500);
                            events.ScheduleEvent(EVENT_BREATH_OF_HETHISS, 14000, PHASE_2);
                            break;
                        }
                        case EVENT_BREATH_OF_HETHISS_END:
                        {
                            me->SetReactState(REACT_AGGRESSIVE);
                            break;
                        }
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            bool buffStairs;
        };

    private:

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_venoxisAI(creature);
        }
};

class npc_venoxis_bloodvenom : public CreatureScript
{
    public:
        npc_venoxis_bloodvenom() : CreatureScript("npc_venoxis_bloodvenom") { }

        struct npc_venoxis_bloodvenomAI : public ScriptedAI
        {
            npc_venoxis_bloodvenomAI(Creature* creature) : ScriptedAI(creature) { }

            void IsSummonedBy(Unit* summoner)
            {
                me->NearTeleportTo(summoner->GetPositionX(), summoner->GetPositionY(), summoner->GetPositionZ(), summoner->GetOrientation());
                if (Creature *c = me->FindNearestCreature(NPC_HIGHPRIEST_VENOXIX, 200.0f))
                    c->AI()->JustSummoned(me);
                me->SetReactState(REACT_PASSIVE);
            }

            void UpdateAI(uint32 const diff)
            {
            }
        };

    private:

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_venoxis_bloodvenomAI(creature);
        }
};

class npc_venomous_effusion : public CreatureScript
{
    public:
        npc_venomous_effusion() : CreatureScript("npc_venomous_effusion") { }

        struct npc_venomous_effusionAI : public ScriptedAI
        {
            npc_venomous_effusionAI(Creature* creature) : ScriptedAI(creature) { }

            void IsSummonedBy(Unit* summoner)
            {
                if (Creature *c = me->FindNearestCreature(NPC_HIGHPRIEST_VENOXIX, 200.0f))
                    c->AI()->JustSummoned(me);
            }
        };

    private:

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_venomous_effusionAI(creature);
        }
};

class spell_venomous_effusion_summon : public SpellScriptLoader
{
    public:
        spell_venomous_effusion_summon() : SpellScriptLoader("spell_venomous_effusion_summon") { }

    private:
        class spell_venomous_effusion_summon_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_venomous_effusion_summon_SpellScript);

            void ModDestHeight(SpellEffIndex /*effIndex*/)
            {
                if (Unit *caster = GetCaster())
                    if (caster->GetTypeId() == TYPEID_UNIT)
                    {
                        if (((int)caster->GetPositionX() == -11995 && (int)caster->GetPositionY() == -1696) ||
                            ((int)caster->GetPositionX() == -12006 && (int)caster->GetPositionY() == -1676))
                            caster->ToCreature()->DespawnOrUnsummon(500);
                    }
                Position offset = {0.0f, 0.0f, 1.5f, 0.0f};
                const_cast<WorldLocation*>(GetExplTargetDest())->RelocateOffset(offset);
                GetHitDest()->RelocateOffset(offset);
            }

            void Register()
            {
                OnEffectLaunch += SpellEffectFn(spell_venomous_effusion_summon_SpellScript::ModDestHeight, EFFECT_1, SPELL_EFFECT_SUMMON);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_venomous_effusion_summon_SpellScript();
        }
};

class spell_toxic_link_selector : public SpellScriptLoader
{
    public:
        spell_toxic_link_selector() : SpellScriptLoader("spell_toxic_link_selector") { }

    private:
        class spell_toxic_link_selector_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_toxic_link_selector_SpellScript)

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                if (unitList.size() > 2)
                    Trinity::Containers::RandomResizeList(unitList, 2);
            }

            void TriggerSpell(SpellEffIndex effIndex)
            {
                GetCaster()->CastSpell(GetHitUnit(), GetSpellInfo()->Effects[effIndex].BasePoints, true);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_toxic_link_selector_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_toxic_link_selector_SpellScript::TriggerSpell, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_toxic_link_selector_SpellScript();
        }
};

// 96477
class spell_toxic_link_visual : public SpellScriptLoader
{
    public:
        spell_toxic_link_visual() : SpellScriptLoader("spell_toxic_link_visual") { }


        class NonToxicLinkTarget
        {
        public:
            explicit NonToxicLinkTarget() {};

            bool operator()(WorldObject* target) const
            {
                return target->GetTypeId() != TYPEID_PLAYER || !target->ToUnit()->HasAura(SPELL_TOXIC_LINK);
            }
        };

        class spell_toxic_link_visual_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_toxic_link_visual_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                unitList.remove_if(NonToxicLinkTarget());
                if (unitList.empty())
                    GetCaster()->RemoveAura(SPELL_TOXIC_LINK);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_toxic_link_visual_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_toxic_link_visual_SpellScript();
        }
};

class spell_toxic_link_aura : public SpellScriptLoader
{
    public:
        spell_toxic_link_aura() : SpellScriptLoader("spell_toxic_link_aura") { }

    private:
        class spell_toxic_link_aura_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_toxic_link_aura_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->CastSpell((Unit*)NULL, GetSpellInfo()->Effects[EFFECT_2].BasePoints, true);
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_toxic_link_aura_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_toxic_link_aura_AuraScript();
        }
};

class spell_whispers_of_hethiss : public SpellScriptLoader
{
    public:
        spell_whispers_of_hethiss() : SpellScriptLoader("spell_whispers_of_hethiss") { }

    private:
        class spell_whispers_of_hethiss_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_whispers_of_hethiss_AuraScript)

            void HandlePeriodicTick(AuraEffect const* /*aurEff*/)
            {
                PreventDefaultAction();

                if (Unit* target = ObjectAccessor::GetUnit(*GetCaster(), GetCaster()->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT)))
                    GetCaster()->CastSpell(target, GetSpellInfo()->Effects[EFFECT_1].TriggerSpell, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_whispers_of_hethiss_AuraScript::HandlePeriodicTick, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_whispers_of_hethiss_AuraScript();
        }
};

class spell_venom_withdrawal : public SpellScriptLoader
{
    public:
        spell_venom_withdrawal() : SpellScriptLoader("spell_venom_withdrawal") { }

    private:
        class spell_venom_withdrawal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_venom_withdrawal_SpellScript);

            void LeapBack(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);

                if (Unit* caster = GetCaster())
                    caster->GetMotionMaster()->MoveJump(-12003.72f, -1688.24f, 32.30f, 20.0f, 20.0f);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_venom_withdrawal_SpellScript::LeapBack, EFFECT_2, SPELL_EFFECT_LEAP_BACK);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_venom_withdrawal_SpellScript();
        }
};

class spell_blood_venom_target_selector : public SpellScriptLoader
{
    public:
        spell_blood_venom_target_selector() : SpellScriptLoader("spell_blood_venom_target_selector") { }

        class NonPlayerTarget
        {
        public:
            explicit NonPlayerTarget() {};

            bool operator()(WorldObject* target) const
            {
                return target->GetTypeId() != TYPEID_PLAYER;
            }
        };

        class spell_blood_venom_target_selector_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_blood_venom_target_selector_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                if (!unitList.empty())
                    unitList.remove_if(NonPlayerTarget());
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_blood_venom_target_selector_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_blood_venom_target_selector_SpellScript();
        }
};

void AddSC_boss_venoxis()
{
    new boss_venoxis();
    new npc_venoxis_bloodvenom();
    new npc_venomous_effusion();
    new spell_venomous_effusion_summon();
    new spell_toxic_link_selector();
    new spell_toxic_link_visual();
    new spell_toxic_link_aura();
    new spell_whispers_of_hethiss();
    new spell_venom_withdrawal();
    new spell_blood_venom_target_selector();
}
