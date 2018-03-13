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

enum twilightCage
{
    NPC_BILGEWATER_LABORER  = 36722,
};

class go_azshara_twilight_cage : public GameObjectScript
{
public:
    go_azshara_twilight_cage() : GameObjectScript("go_azshara_twilight_cage") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        go->UseDoorOrButton(180);
        if (Creature* laborer = go->FindNearestCreature(NPC_BILGEWATER_LABORER, 5.0f, true))
        {
            Position pos;
            laborer->GetNearPosition(pos, 15.00f, 0.00f);
            laborer->GetMotionMaster()->MovePoint(0, pos);
            laborer->DespawnOrUnsummon(10000);
            player->KilledMonsterCredit(laborer->GetEntry(), laborer->GetGUID());
        }
        return true;
    }
};


/*
enum FadeToBlackQuest
{
    NPC_MALICION                = 36649,
    ACTION_ENTER_COMBAT         = 1,
    ACTION_LAND                 = 2,
    POINT_AIR                   = 4,
    POINT_GROUND                = 5,
    SPELL_FLAME_BURST           = 69114,
    SPELL_SHIELD_BREAK          = 69115,
    SPELL_RIDE_VEHICLE          = 43671,
    SPELL_TWILIGHT_BARRIER      = 69135,
    SPELL_SUMMON_KALECGOS       = 69178,
    EVENT_FLAME_BURST           = 1,
    EVENT_TWILIGHT_BARRIER      = 2,
    EVENT_PHASE_1               = 1,
    EVENT_PHASE_2               = 2,
    TALK_VEHICLE_ENTER          = -36897,
    TALK_KATRANA_ENTER_COMBAT   = -77866,
};

class npc_azshara_twilight_lord_katrana : public CreatureScript
{
public:
    npc_azshara_twilight_lord_katrana() : CreatureScript("npc_azshara_twilight_lord_katrana") { }

    struct npc_azshara_twilight_lord_katranaAI : public ScriptedAI
    {
        npc_azshara_twilight_lord_katranaAI(Creature* creature) : ScriptedAI(creature), summons(creature) {}

        void Reset() override
        {
            events.Reset();
            summons.DespawnAll();
            me->SummonCreature(NPC_MALICION, 4766.41f, -7336.79f, 128.913f, 1.35019f, TEMPSUMMON_DEAD_DESPAWN);
        }

        void EnterCombat(Unit* attacker)
        {
            events.SetPhase(EVENT_PHASE_1);
            if (Creature* drake = ObjectAccessor::GetCreature(*me, summons.front()))
                drake->AI()->DoAction(ACTION_ENTER_COMBAT);
            Talk(TALK_KATRANA_ENTER_COMBAT);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }

        void JustDied(Unit* killer) override
        {
            events.Reset();
            if (Creature* drake = ObjectAccessor::GetCreature(*me, summons.front()))
                drake->AI()->DoAction(ACTION_LAND);
        }

        void DamageTaken(Unit* attacker, uint32& damage) override
        {
            if (events.IsInPhase(EVENT_PHASE_1) && me->HealthBelowPctDamaged(60, damage))
            {
                events.SetPhase(EVENT_PHASE_2);
                events.ScheduleEvent(EVENT_TWILIGHT_BARRIER, urand(28000, 32000));
                DoCast(me, SPELL_TWILIGHT_BARRIER, true);
            }
        }

        void SummonedCreatureDamageTaken(Unit* attacker, Creature* summon, uint32& damage, SpellInfo const* spellInfo)
        {
            if (!me->isInCombat())
                me->AI()->AttackStart(attacker);
        }

        void SpellHit(Unit* attacker, SpellInfo const* spellInfo) override
        {
            if (spellInfo->Id == SPELL_SHIELD_BREAK)
                me->RemoveAurasDueToSpell(SPELL_TWILIGHT_BARRIER);
        }

        void UpdateAI(uint32 const diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_TWILIGHT_BARRIER:
                        DoCast(me, SPELL_TWILIGHT_BARRIER, true);
                        events.ScheduleEvent(EVENT_TWILIGHT_BARRIER, urand(28000, 32000));
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
        SummonList summons;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_azshara_twilight_lord_katranaAI(creature);
    }
};

class npc_azshara_malicion : public CreatureScript
{
public:
    npc_azshara_malicion() : CreatureScript("npc_azshara_malicion") { }

    struct npc_azshara_malicionAI : public ScriptedAI
    {
        npc_azshara_malicionAI(Creature* creature) : ScriptedAI(creature) {}

        void IsSummonedBy(Unit* summoner)
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
            case ACTION_ENTER_COMBAT:
                me->SetCanFly(true);
                me->SetDisableGravity(true);
                me->GetMotionMaster()->MovePoint(POINT_AIR, 4764.34f, -7325.118f, 142.0452f);
                break;
            case ACTION_LAND:
                events.Reset();
                DoCast(me, SPELL_SUMMON_KALECGOS, true);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                me->GetMotionMaster()->MovePoint(POINT_GROUND, 4774.59f, -7320.24f, 130.47f);
                break;
            default:
                break;
            }
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != POINT_MOTION_TYPE)
                return;

            switch (id)
            {
                case POINT_AIR:
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                    me->SetOrientation(6.1050f);
                    events.ScheduleEvent(EVENT_FLAME_BURST, 1000);
                    break;
                case POINT_GROUND:
                    if (Player* player = me->FindNearestPlayer(150.00f))
                        player->CastSpell(me, SPELL_RIDE_VEHICLE, true);
                    Talk(TALK_VEHICLE_ENTER);
                    break;
                default:
                    break;
            }
        }

        void JustDied(Unit* killer) override
        {
            events.Reset();
        }

        void UpdateAI(uint32 const diff) override
        {
            events.Update(diff);
            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_FLAME_BURST:
                        if (Player* player = me->FindNearestPlayer(80.00f))
                            DoCast(player, SPELL_FLAME_BURST, true);
                        events.ScheduleEvent(EVENT_FLAME_BURST, 5000);
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_azshara_malicionAI(creature);
    }
};*/

void AddSC_azshara()
{
    new go_azshara_twilight_cage();
    /*new npc_azshara_malicion(); // Handled through Smart AI
    new npc_azshara_twilight_lord_katrana();*/ // Handled through Smart AI
}
