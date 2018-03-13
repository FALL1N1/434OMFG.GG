/*
 * Copyright (C) 2013-2014 OMFG.GG Network <http://www.omfg.gg/>
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
#include "SpellAuraEffects.h"
#include "ScriptedEscortAI.h"
#include "hour_of_twilight.h"

enum Yells
{
    TALK_THRALL_INTRO           = -53046,
    TALK_THRALL_INTRO_2         = -53826,
    TALK_THRALL_MAGIC           = -53050,
    TALK_THRALL_LOOK_OUT        = -54489,
    TALK_THRALL_HURRY           = -53048,
    TALK_THRALL_AMBUSH          = -54490,
    TALK_THRALL_TAKE_A_MOMENT   = -54491,

    TALK_THRALL_2_SHOW_YOURSELF = -53049,
    TALK_THRALL_2_ENTER_COMBAT  = -54177,
    TALK_THRALL_2_PHASE_2       = -53963,
    TALK_THRALL_2_DISCOVERED    = -54294,
    TALK_THRALL_2_FOLLOW_ME     = -54446,

    TALK_ARCURION_INTRO         = -53797,
    TALK_ARCURION_DESTROY_ALL   = -53798,
    TALK_ARCURION_YOU_WILL_GO   = -53803,
    TALK_ARCURION_VISIBLE       = -53818,
    TALK_ARCURION_YOU_RE_MORTAL = -54495,
    TALK_EMOTE_TWILIGHT_FORCES  = -54176,
    TALK_ARCURION_FREEZE        = -54496,
    TALK_ARCURION_FREEZE_2      = -54497,
    TALK_ARCURION_FREEZE_3      = -54498,
    TALK_EMOTE_FREEZE_THRALL    = -54199,
    TALK_ARCURION_PHASE_TWO     = -54097,
    TALK_ARCURION_DEATH         = -54502,
    TALK_ARCURION_KILL_PLAYER   = -54499,
    TALK_ARCURION_KILL_PLAYER_2 = -54500,
    TALK_ARCURION_KILL_PLAYER_3 = -54501,
};

enum Spells
{
    // enter combat 11:15:36.078
    SPELL_HANDS_OF_FROST                   = 102593,
    SPELL_CHAINS_OF_FROST                  = 102582,
    SPELL_ICY_TOMB                         = 103252,
    SPELL_ICY_TOMB_TRIGGER                 = 103251,
    SPELL_ICY_TOMB_SUMMON_PERIODIC         = 103250,
    SPELL_ICY_TOMB_SUMMON                  = 103249,
    SPELL_TORRENT_OF_FROST                 = 104050,
    SPELL_TORRENT_OF_FROST_DUMMY           = 104055,
    SPELL_TORRENT_OF_FROST_PERIODIC        = 103962,
    SPELL_TORRENT_OF_FROST_DAMAGE          = 103904,
    SPELL_TORRENT_OF_FROST_DUMMY_2         = 104058,
    //    SPELL_SUMMON_FROZEN_SERVITOR     = 102187,
    // NPC_ICY_TOMB
    //    SPELL_DELETE_ICY_TOMB            = 102722,
    // NPC_FROZEN_SERVITOR
    // out of boss fight
    SPELL_VISUAL_ICY_BOULDER_CRASH_DUMMY   = 105432,
    SPELL_ICY_BOULDER_CRASH_PERIODIC       = 105433,
    SPELL_VISUAL_ICY_BOULDER_CRASH         = 102199,
    // NPC_FROZEN_BOSS_SERVITOR
    SPELL_ICY_SERVITOR_SPAWN               = 102300,
    SPELL_ICY_SERVITOR_DISABLE             = 102443,
    SPELL_ICY_BOULDER                      = 102480,
    SPELL_VISUAL_ICY_BOULDER_TARGET        = 102198,
    // NPC_ACURION_SPAWN_STALKER
    SPELL_ARCURION_SPAWN_VISUAL            = 104767,
    // NPC_THRALL_EVENT_1
    SPELL_FROZEN_SPAWN_VISUAL              = 103595,
    SPELL_LAVA_BURST_BOSS                  = 103923,
    SPELL_LAVA_BURST_ADDS                  = 102475,
    SPELL_WOLF_GHOST                       = 2645,
    SPELL_BLOODLUST                        = 103834,
    SPELL_KILL_ICY_TOMB                    = 103921,

    SPELL_EYE_STALKER_VISUAL               = 101913,
    // 102059
    SPELL_DESTROY_WALL                     = 104540,
};

enum Events
{
    EVENT_HAND_OF_FROST = 1,
    EVENT_CHAIN_OF_FROST,
    EVENT_ICY_TOMB,
    EVENT_TORRENT_OF_FROST,
    EVENT_INTRO,
    EVENT_INTRO_1,
    EVENT_INTRO_2,
    EVENT_INTRO_3,
    EVENT_STEP_1,
    EVENT_STEP_2,
    EVENT_STEP_3,
    EVENT_SUMMON_FROZEN_SERVITOR,
    EVENT_SUMMON_WAVE,
    EVENT_SUMMON_WAVE_2,
    EVENT_SUMMON_WAVE_3,
    EVENT_LAVA_BURST,
    EVENT_LAVA_BURST_INFIGHT,
    EVENT_LAVA_BURST_BOSS,
    EVENT_RESET_ENCOUNTER,
    EVENT_END_ESCORT_PART,
    EVENT_START_ESCORT,
};

enum ActionsBoss
{
    ACTION_SPAWN = 1,
    ACTION_RESPAWN = 3,
};

enum eSummonGroups
{
    SUMMON_GROUP_1 = 1,
    SUMMON_GROUP_2,
    SUMMON_GROUP_3
};

enum eNpcs
{
    NPC_ICY_TOMB                      = 54995,
    NPC_ACURION_SPAWN_STALKER         = 57197,
    NPC_FROZEN_SERVITOR_SPAWN_POINT   = 54598,
    NPC_INFIGHT_FROZEN_SERVITOR       = 54600,
    NPC_THRALL_FIRE_TOTEM             = 55474,
    NPC_SECOND_SUM                    = 55559,
    NPC_FIRST_SUM                     = 54555,
    NPC_FROZEN_SERVITOR_SPAWN_STALKER_ENTRANCE = 55389,
    NPC_FROZEN_SERVITOR_SPAWN_STALKER = 54560,
    NPC_FROZEN_SERVITOR_SPAWN_STALKER_2 = 55384,
    NPC_FROZEN_SERVITOR_SPAWN_STALKER_3 = 55385,
    NPC_FROZEN_SHARD                  = 55563,
    NPC_EYE_STALKER                   = 54522,
    NPC_WALL_TRIGGER                  = 55728,
};

class boss_arcurion : public CreatureScript
{
public:
    boss_arcurion() : CreatureScript("boss_arcurion") { }

    struct boss_arcurionAI : public BossAI
    {
        boss_arcurionAI(Creature* creature) : BossAI(creature, DATA_ARCURION)
        {
            _introDone = false;
        }

        void Reset() override
        {
            _phase2 = false;
            _introDone = false;
            me->SetReactState(REACT_PASSIVE);
            me->SetVisible(false);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
            RemoveEncounterFrame();
            _Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            AddEncounterFrame();
            if (Creature* thrall = me->FindNearestCreature(NPC_THRALL_EVENT_1, 100.0f, true))
            {
                thrall->SetReactState(REACT_AGGRESSIVE);
                thrall->SetInCombatWith(me);
            }

            events.ScheduleEvent(EVENT_HAND_OF_FROST, 0);
            events.ScheduleEvent(EVENT_CHAIN_OF_FROST, 12000);
            events.ScheduleEvent(EVENT_ICY_TOMB, 30000);
            events.ScheduleEvent(EVENT_SUMMON_FROZEN_SERVITOR, 0);
            _EnterCombat();
        }

        void JustSummoned(Creature* summon) override
        {
            switch (summon->GetEntry())
            {
                case NPC_INFIGHT_FROZEN_SERVITOR:
                    summon->SetInCombatWithZone();
                    break;
                default:
                    break;
            }
            BossAI::JustSummoned(summon);
        }

        void JustDied(Unit* /*killer*/) override
        {
            RemoveEncounterFrame();
            Talk(TALK_ARCURION_DEATH);
            if (Creature* thrall = instance->GetCreature(DATA_THRALL_EVENT_1))
                thrall->AI()->DoAction(ACTION_STOP_COMBAT);
            _JustDied();
        }

        void KilledUnit(Unit* victim) override
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(RAND(TALK_ARCURION_KILL_PLAYER, TALK_ARCURION_KILL_PLAYER_2, TALK_ARCURION_KILL_PLAYER_3));
        }

        void DamageTaken(Unit* /*caster*/, uint32& damage) override
        {
            if (!_phase2 && me->HealthBelowPctDamaged(31, damage))
            {
                if (Creature* thrall = instance->GetCreature(DATA_THRALL_EVENT_1))
                    thrall->AI()->Talk(TALK_THRALL_2_PHASE_2);

                TalkWithDelay(6000, TALK_ARCURION_PHASE_TWO);
                events.Reset();
                events.ScheduleEvent(EVENT_TORRENT_OF_FROST, 0);
                if (Creature* icetomb = me->FindNearestCreature(NPC_ICY_TOMB, 500.0f, true))
                    icetomb->Kill(icetomb);
                _phase2 = true;
            }
        }

        void DoAction(int32 const id)
        {
            switch (id)
            {
                case ACTION_SPAWN:
                    if (Creature* stalker = me->FindNearestCreature(NPC_ACURION_SPAWN_STALKER, 20.0f, true))
                        stalker->CastSpell(stalker, SPELL_ARCURION_SPAWN_VISUAL, true);
                    me->SetVisible(true);
                    Talk(TALK_ARCURION_VISIBLE);
                    TalkWithDelay(11000, TALK_ARCURION_YOU_RE_MORTAL);
                    events.ScheduleEvent(EVENT_INTRO_3, 18000);
                    break;
                case ACTION_RESPAWN:
                    _DespawnAtEvade();
                    break;
            }
        }

        void UpdateAI(const uint32 diff) override
        {
            if (!UpdateVictim() && _introDone)
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_INTRO_3:
                        _introDone = true;
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                        Talk(TALK_EMOTE_TWILIGHT_FORCES, NULL, CHAT_MSG_RAID_BOSS_EMOTE);
                        if (Creature* thrall = instance->GetCreature(DATA_THRALL_EVENT_1))
                            thrall->AI()->Talk(TALK_THRALL_2_ENTER_COMBAT);
                        break;
                    case EVENT_HAND_OF_FROST:
                        DoCastVictim(SPELL_HANDS_OF_FROST);
                        events.ScheduleEvent(EVENT_HAND_OF_FROST, 5000);
                        break;
                    case EVENT_CHAIN_OF_FROST:
                        DoCast(SPELL_CHAINS_OF_FROST);
                        events.ScheduleEvent(EVENT_CHAIN_OF_FROST, 15000);
                        break;
                    case EVENT_ICY_TOMB:
                        Talk(RAND(TALK_ARCURION_FREEZE, TALK_ARCURION_FREEZE_2, TALK_ARCURION_FREEZE_3));
                        Talk(TALK_EMOTE_FREEZE_THRALL, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                        if (Creature* thrall = instance->GetCreature(DATA_THRALL_EVENT_1))
                        {
                            thrall->CastWithDelay(3000, thrall, SPELL_ICY_TOMB_TRIGGER, true);
                            me->CastSpell(thrall, SPELL_ICY_TOMB, true);
                        }
                        events.ScheduleEvent(EVENT_ICY_TOMB, 60000);
                        break;
                    case EVENT_TORRENT_OF_FROST:
                        if (Creature* thrall = instance->GetCreature(DATA_THRALL_EVENT_1))
                            thrall->AI()->DoAction(ACTION_BOSS_PHASE_3);
                        DoCast(SPELL_TORRENT_OF_FROST);
                        break;
                    case EVENT_SUMMON_FROZEN_SERVITOR:
                    {
                        std::list<Creature*> trig;
                        me->GetCreatureListWithEntryInGrid(trig, NPC_FROZEN_SERVITOR_SPAWN_POINT, 1000.0f);
                        if (!trig.empty())
                        {
                            Creature* spawnPoint = Trinity::Containers::SelectRandomContainerElement(trig);
                            me->SummonCreature(NPC_INFIGHT_FROZEN_SERVITOR, *spawnPoint);
                        }
                        events.ScheduleEvent(EVENT_SUMMON_FROZEN_SERVITOR, 4000);
                        break;
                    }
                    default:
                       break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        bool _introDone, _phase2;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetHourOfTwilightAI<boss_arcurionAI>(creature);
    }
};

class SummonWaveEvent : public BasicEvent
{
public:
    SummonWaveEvent(Creature* summoner, Position const& pos, bool crystal) : _summoner(summoner), _position(pos)
    {
    }

    bool Execute(uint64 /*execTime*/, uint32 /*diff*/) override
    {
        _summoner->SummonCreature(crystal ? NPC_SECOND_SUM : NPC_FIRST_SUM , _position);
        return true;
    }

private:
    Creature* _summoner;
    Position _position;
    bool crystal;
};

class npc_thrall_hot_0 : public CreatureScript
{
public:
    npc_thrall_hot_0() : CreatureScript("npc_thrall_hot_0")
    {}

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();
        npc_thrall_hot_0AI* ai = CAST_AI(npc_thrall_hot_0::npc_thrall_hot_0AI, creature->AI());
        if (!ai)
            return false;

        ai->Talk(TALK_THRALL_INTRO);
        ai->events.ScheduleEvent(EVENT_INTRO, 7000);

        player->CLOSE_GOSSIP_MENU();
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        return true;
    }

    struct npc_thrall_hot_0AI : public npc_escortAI
    {
        friend class npc_thrall_hot_0;

        npc_thrall_hot_0AI(Creature* creature) : npc_escortAI(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
            killCount = 0;
            if (instance->GetData(DATA_EVENT_0) == DONE)
                me->SetVisible(false);
        }

        void Reset() override
        {
            me->SetWalk(false);
            events.CancelEvent(EVENT_RESET_ENCOUNTER);
            events.CancelEvent(EVENT_LAVA_BURST);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.CancelEvent(EVENT_RESET_ENCOUNTER);
            events.ScheduleEvent(EVENT_RESET_ENCOUNTER, 1000);
        }

        void JustDied(Unit* /*killer*/) override
        {
            ResetEncounter();
        }

        void DoAction(int32 const id) override
        {
            switch (id)
            {
                case ACTION_KILL_CREATURE:
                    killCount++;
                    switch (killCount)
                    {
                        case 2:
                            Talk(TALK_THRALL_INTRO_2);
                            events.CancelEvent(EVENT_LAVA_BURST);
                            events.ScheduleEvent(EVENT_START_ESCORT, 6000);
                            break;
                        case 5:
                            Talk(TALK_THRALL_HURRY);
                            events.CancelEvent(EVENT_LAVA_BURST);
                            events.ScheduleEvent(EVENT_SUMMON_WAVE_3, 3000);
                            break;
                        case 11:
                            events.CancelEvent(EVENT_LAVA_BURST);
                            events.ScheduleEvent(EVENT_STEP_3, 3000);
                            break;
                        default:
                            break;
                    }
                    break;
            }
        }

        void WaypointReached(uint32 point) override
        {
            switch (point)
            {
                case 3:
                    me->SummonCreature(NPC_EYE_STALKER, 4893.465f, 22.122f, 33.32f, 1.45f);
                    break;
                case 4:
                {
                    Talk(TALK_THRALL_MAGIC);
                    SetEscortPaused(true);
                    events.ScheduleEvent(EVENT_SUMMON_WAVE_2, 4000);
                    break;
                }
                case 7:
                    events.ScheduleEvent(EVENT_INTRO_2, 3000);
                    break;
                default:
                    break;
            }
        }

        void JustSummoned(Creature* summon) override
        {
            switch (summon->GetEntry())
            {
                case NPC_EYE_STALKER:
                    summon->CastSpell(summon, SPELL_EYE_STALKER_VISUAL, true);
                    break;
                case NPC_FROZEN_SERVITOR_SPAWN_STALKER_ENTRANCE:
                case NPC_FROZEN_SERVITOR_SPAWN_STALKER_2:
                case NPC_FROZEN_SERVITOR_SPAWN_STALKER_3:
                    summon->CastSpell(summon, SPELL_FROZEN_SPAWN_VISUAL, true);
                    me->m_Events.AddEvent(new SummonWaveEvent(me, *summon, false), me->m_Events.CalculateTime(500));
                    break;
                case NPC_FROZEN_SERVITOR_SPAWN_STALKER:
                    summon->CastSpell(summon, SPELL_FROZEN_SPAWN_VISUAL, true);
                    me->m_Events.AddEvent(new SummonWaveEvent(me, *summon, true), me->m_Events.CalculateTime(500));
                    break;
                default:
                    if (killCount >= 5)
                        summon->AI()->AttackStart(me);
                    break;
            }
            summons.Summon(summon);
        }

        void ResetEncounter()
        {
            events.Reset();
            summons.DespawnAll();
            killCount = 0;
            me->Kill(me);
            me->Respawn(true);
        }

        void UpdateAI(uint32 const diff) override
        {
            npc_escortAI::UpdateAI(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_START_ESCORT:
                        SetDespawnAtFar(false);
                        SetDespawnAtEnd(false);
                        Start(true, true, 0, 0, false, false);
                        instance->SetData(DATA_EVENT_0, IN_PROGRESS);
                        summons.DoZoneInCombat(NPC_SECOND_SUM);
                        break;
                    case EVENT_LAVA_BURST:
                        DoCast(SPELL_LAVA_BURST_ADDS);
                        events.ScheduleEvent(EVENT_LAVA_BURST, 2500);
                        break;
                    case EVENT_SUMMON_WAVE_2:
                        Talk(TALK_THRALL_LOOK_OUT);
                        if (Creature* arcurion = instance->GetCreature(DATA_ARCURION))
                            arcurion->AI()->Talk(TALK_ARCURION_DESTROY_ALL, 0, CHAT_MSG_MONSTER_YELL, TEXT_RANGE_MAP);

                        if (Creature *eye = me->FindNearestCreature(NPC_EYE_STALKER, 50.0f, true))
                        {
                            eye->RemoveAurasDueToSpell(SPELL_EYE_STALKER_VISUAL);
                            eye->CastSpell(eye, SPELL_FROZEN_SPAWN_VISUAL, true);
                            eye->DespawnOrUnsummon(3000);
                        }
                        me->SummonCreatureGroup(SUMMON_GROUP_2);
                        summons.DoZoneInCombat(NPC_SECOND_SUM);
                        summons.DoZoneInCombat(NPC_FIRST_SUM);
                        events.ScheduleEvent(EVENT_LAVA_BURST, 2500);
                        break;
                    case EVENT_SUMMON_WAVE_3:
                        Talk(TALK_THRALL_AMBUSH);
                        events.ScheduleEvent(EVENT_LAVA_BURST, 2500);
                        me->SummonCreatureGroup(SUMMON_GROUP_3);
                        summons.DoZoneInCombat(NPC_SECOND_SUM);
                        summons.DoZoneInCombat(NPC_FROZEN_SHARD);
                        break;
                    case EVENT_INTRO:
                        me->SetFacingTo(4.712389f);
                        if (Creature* arcurion = instance->GetCreature(DATA_ARCURION))
                            arcurion->AI()->Talk(TALK_ARCURION_INTRO, 0, CHAT_MSG_MONSTER_YELL, TEXT_RANGE_MAP);
                        me->SummonCreatureGroup(SUMMON_GROUP_1);
                        break;
                    case EVENT_INTRO_2:
                        Talk(TALK_THRALL_TAKE_A_MOMENT);
                        events.ScheduleEvent(EVENT_END_ESCORT_PART, 5000);
                        break;
                    case EVENT_END_ESCORT_PART:
                        if (Creature* arcurion = instance->GetCreature(DATA_ARCURION))
                            arcurion->AI()->Talk(TALK_ARCURION_YOU_WILL_GO, 0, CHAT_MSG_MONSTER_YELL, TEXT_RANGE_MAP);
                        instance->SetData(DATA_EVENT_0, DONE);
                        me->DespawnOrUnsummon();
                        break;
                    case EVENT_RESET_ENCOUNTER:
                    {
                        bool isGroupWipe = true;
                        Map::PlayerList const& playerList = me->GetMap()->GetPlayers();
                        for (Map::PlayerList::const_iterator i = playerList.begin(); i != playerList.end(); ++i)
                            if (i->getSource()->isAlive() && !i->getSource()->isGameMaster())
                            {
                                isGroupWipe = false;
                                break;
                            }

                        if (!instance->IsDone(DATA_ARCURION))
                        {
                            if (isGroupWipe)
                                ResetEncounter();
                            else
                                events.ScheduleEvent(EVENT_RESET_ENCOUNTER, 1000);
                        }
                        break;
                    }
                    case EVENT_STEP_3:
                        SetEscortPaused(false);
                        break;
                    default:
                        break;
                }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

    private:
        InstanceScript* instance;
        EventMap events;
        uint32 killCount;
        uint32 step;
        SummonList summons;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetHourOfTwilightAI<npc_thrall_hot_0AI>(creature);
    }
};

class npc_thrall_hot_1 : public CreatureScript
{
public:
    npc_thrall_hot_1() : CreatureScript("npc_thrall_hot_1")
    {
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        InstanceScript* instance = creature->GetInstanceScript();
        npc_thrall_hot_1AI* ai = CAST_AI(npc_thrall_hot_1::npc_thrall_hot_1AI, creature->AI());
        if (instance)
        {
            if (instance->IsDone(DATA_ARCURION))
            {
                creature->AI()->Talk(TALK_THRALL_2_FOLLOW_ME);
                creature->CastSpell(creature, SPELL_WOLF_GHOST, true);
                ai->SetEscortPaused(false);
            }
            else
            {
                ai->Start(true, true, 0, 0, false, false);
                ai->SetDespawnAtFar(false);
                ai->SetDespawnAtEnd(false);
            }
        }

        player->CLOSE_GOSSIP_MENU();
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        InstanceScript* instance = creature->GetInstanceScript();
        if (instance && instance->IsDone(DATA_ARCURION))
        {
            player->PrepareGossipMenu(creature, 13164, false);
            player->SendPreparedGossip(creature);
        }
        else
        {
            player->PrepareGossipMenu(creature, 13183, true);
            player->SendPreparedGossip(creature);
        }
        return true;
    }

    struct npc_thrall_hot_1AI : public npc_escortAI
    {
        friend class npc_thrall_hot_1;

        npc_thrall_hot_1AI(Creature* creature) : npc_escortAI(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
            if (instance->GetData(DATA_EVENT_1) == DONE || (instance->GetData(DATA_EVENT_0) != DONE && !instance->IsDone(DATA_ARCURION)))
                me->SetVisible(false);
            me->SetReactState(REACT_PASSIVE);
        }

        void Reset() override
        {
            instance->SetBossState(DATA_THRALL_EVENT_1, NOT_STARTED);
            me->SetWalk(false);
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.Reset();
            events.ScheduleEvent(EVENT_LAVA_BURST, 1000);
            events.ScheduleEvent(EVENT_RESET_ENCOUNTER, 1000);
            instance->SetBossState(DATA_THRALL_EVENT_1, IN_PROGRESS);
        }

        void JustDied(Unit* /*killer*/) override {}

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            switch (summon->GetEntry())
            {
                case NPC_ICY_TOMB:
                    summon->CastSpell(me, SPELL_DELETE_ICY_TOMB, true);
                    me->RemoveAurasDueToSpell(SPELL_ICY_TOMB_TRIGGER);
                    return;
            }
        }

        void JustSummoned(Creature* summon) override
        {
            switch (summon->GetEntry())
            {
                case NPC_ICY_TOMB:
                    summon->SetReactState(REACT_PASSIVE);
                    break;
                default:
                    break;
            }
            summons.Summon(summon);
        }

        void WaypointReached(uint32 point)
        {
            switch (point)
            {
                case 3:
                    me->SetReactState(REACT_PASSIVE);
                    SetEscortPaused(true);
                    TalkWithDelay(500, TALK_THRALL_2_SHOW_YOURSELF);
                    events.ScheduleEvent(EVENT_INTRO, 2000);
                    break;
                case 4:
                    SetEscortPaused(true);
                    events.ScheduleEvent(EVENT_INTRO_1, 1000);
                    break;
                case 5:
                    SetEscortPaused(true);
                    DoCast(SPELL_DESTROY_WALL);
                    break;
                case 23:
                    events.ScheduleEvent(EVENT_END_ESCORT_PART, 2000);
                    break;
                default:
                    break;
            }
        }

        void DoAction(const int32 action) override
        {
            switch (action)
            {
                case ACTION_BOSS_PHASE_3:
                    me->CastSpell(me, SPELL_BLOODLUST, true);
                    break;
                case ACTION_STOP_COMBAT:
                    events.Reset();
                    me->SetReactState(REACT_PASSIVE);
                    me->ClearInCombat();
                    TalkWithDelay(TALK_THRALL_2_DISCOVERED, 7000);
                    events.ScheduleEvent(EVENT_INTRO_2, 13000);
                    break;
            }
        }

        void SpellHitTarget(Unit* victim, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_DESTROY_WALL && victim->GetEntry() == NPC_WALL_TRIGGER)
            {
                if (GameObject* iceWall = me->FindNearestGameObject(210048, 100.0f))
                {
                    iceWall->SetGoState(GO_STATE_ACTIVE);
                    events.ScheduleEvent(EVENT_INTRO_3, 1000);
                }
            }
        }

        void EnterEvadeMode() override {}

        void ResetEncounter()
        {
            summons.DespawnAll();

            me->DespawnCreaturesInArea(NPC_INFIGHT_FROZEN_SERVITOR, 500.0f);

            me->ClearInCombat();

            if (Creature* arcurion = instance->GetCreature(DATA_ARCURION))
                arcurion->AI()->DoAction(ACTION_RESPAWN);

            me->Respawn(true);
        }

        void UpdateAI(uint32 const diff) override
        {
            npc_escortAI::UpdateAI(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_INTRO:
                        if (Creature* arcurion = instance->GetCreature(DATA_ARCURION))
                            arcurion->AI()->DoAction(ACTION_SPAWN);
                        SetEscortPaused(false);
                        break;
                    case EVENT_INTRO_1:
                        me->SetReactState(REACT_AGGRESSIVE);
                        break;
                    case EVENT_INTRO_2:
                        SetEscortPaused(false);
                        break;
                    case EVENT_INTRO_3:
                        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        break;
                    case EVENT_LAVA_BURST:
                        if (me->FindNearestCreature(NPC_INFIGHT_FROZEN_SERVITOR, 500.0f, true))
                            DoCast(SPELL_LAVA_BURST_ADDS);
                        else
                            DoCast(SPELL_LAVA_BURST_BOSS);
                        events.ScheduleEvent(EVENT_LAVA_BURST, 2500);
                        break;
                    case EVENT_END_ESCORT_PART:
                        instance->SetData(DATA_EVENT_1, DONE);
                        me->DespawnOrUnsummon();
                        break;
                    case EVENT_RESET_ENCOUNTER:
                    {
                        bool isGroupWipe = true;
                        Map::PlayerList const& playerList = me->GetMap()->GetPlayers();
                        for (Map::PlayerList::const_iterator i = playerList.begin(); i != playerList.end(); ++i)
                            if (i->getSource()->isAlive() && !i->getSource()->isGameMaster())
                            {
                                isGroupWipe = false;
                                break;
                            }

                        if (!instance->IsDone(DATA_ARCURION))
                        {
                            if (isGroupWipe)
                                ResetEncounter();
                            else
                                events.ScheduleEvent(EVENT_RESET_ENCOUNTER, 1000);
                        }
                        break;
                    }
                    default:
                        break;
                }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

    private:
        InstanceScript* instance;
        EventMap events;
        SummonList summons;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetHourOfTwilightAI<npc_thrall_hot_1AI>(creature);
    }
};

class spell_arcurion_icy_tomb_delete : public SpellScriptLoader
{
public:
    spell_arcurion_icy_tomb_delete() : SpellScriptLoader("spell_arcurion_icy_tomb_delete") { }

    class spell_arcurion_icy_tomb_delete_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_arcurion_icy_tomb_delete_SpellScript);

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            if (Unit* target = GetHitUnit())
            {
                target->RemoveAurasDueToSpell(SPELL_ICY_TOMB_TRIGGER);
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_arcurion_icy_tomb_delete_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_arcurion_icy_tomb_delete_SpellScript();
    }
};

class spell_hot_thrall_0_lava_lash : public SpellScriptLoader
{
public:
    spell_hot_thrall_0_lava_lash() : SpellScriptLoader("spell_hot_thrall_0_lava_lash")
    {}

    class spell_hot_thrall_0_lava_lash_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hot_thrall_0_lava_lash_SpellScript);

        void AddTarget(WorldObject*& targets, uint32 entry)
        {
            if (Creature *add = GetCaster()->FindNearestCreature(entry, 200.0f, true))
                targets = (WorldObject*)add;
        }

        void FilterTargets(WorldObject*& targets)
        {
            AddTarget(targets, NPC_FIRST_SUM);
            AddTarget(targets, NPC_INFIGHT_FROZEN_SERVITOR);
            AddTarget(targets, NPC_FROZEN_SHARD);
            AddTarget(targets, NPC_SECOND_SUM);
        }

        void Register()
        {
            OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_hot_thrall_0_lava_lash_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_NEARBY_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hot_thrall_0_lava_lash_SpellScript();
    }
};

void AddSC_boss_arcurion()
{
    new boss_arcurion();
    new npc_thrall_hot_0();
    new npc_thrall_hot_1();
    new spell_arcurion_icy_tomb_delete();
    new spell_hot_thrall_0_lava_lash();
}
