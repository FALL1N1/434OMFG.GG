/*
 * Copyright (C) 2009-2012 Project Frostmourne <http://www.frostmourne-wow.eu/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
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

#include "icecrown_citadel.h"
#include "Group.h"

#define GOSSIP_ITEM_LEADER    "Wir suchen eine neue Herausforderung"

/*
Author: Bonitas
Status: Normal: 100 % Heroic: 100 %
*/

enum Texts
{
    SAY_INTRO                       = 0,
    SAY_ADD_SPAWN                   = 1,
    SAY_BERSERK                     = 2,
    SAY_KILLED_UNIT                 = 3,
    SAY_SWARMING_SHADOWS            = 4,
};

enum Spells
{
    // Gunship Ersatzboss
    SPELL_ARRIVAL                   = 64997,
    SPELL_BONE_SLICE                = 69055,
    SPELL_SWARMING_SHADOWS          = 71264,
    SPELL_SHROUD_OF_SORROW          = 70986,
    SPELL_FATAL_ATRACTION           = 41001,
    SPELL_PACKT_OF_THE_DARKFALLEN   = 71340,
    SPELL_TELE_VISUAL               = 73078,

    //Adds
    SPELL_SEARING_LIGHT             = 65121, //Add 1 Holy - 8y range
    SPELL_LEGION_FLAME              = 68125, //Add 2 Fire - movement 10 2 Targets 25 6 Targets
    SPELL_MANA_BARRIER              = 70842, //Add 3 Water - DPS!!!
    SPELL_UNYIELDING_PAIN           = 57381,
    SPELL_SOULSTORM                 = 68872,
    SPELL_SOULSTORM_VISUAL          = 68870, //Add 4 Shadow - everybody must be in hitbox!!!
    SPELL_FROST_AURA                = 71052, //Add 5 Frost HPS
    SPELL_PERMAEATING_CHILL         = 70109, //Add 5
    SPELL_LEECHING_SWARM            = 66118, //Add 6 Nature HPS
    SPELL_AURA_OF_DESIRE            = 41350, //Add 7 Desire 50% DMG reflected, Healing increased 100%
    SPELL_AURA_OF_DESIRE_DAMAGE     = 41352,
    SPELL_BERSERKER                 = 68335, //Add 8 Berserk 50%
    SPELL_GAS_NOVA                  = 45855, //Add 9 Gas
    SPELL_CARRION_SWARM             = 31306, //Add 10 Plague
};

enum Events
{
    EVENT_BONE_SLICE                    = 1,
    EVENT_SWARMING_SHADOWS              = 2,
    EVENT_FATAL_ATRACTION               = 3,
    EVENT_PACKT_OF_THE_DARKFALLEN       = 4,
    EVENT_SUMMON_ADDS                   = 5,
    EVENT_FIRE                          = 6,
    EVENT_SHADOW                        = 7,
    EVENT_FROST                         = 8,
    EVENT_WATER                         = 9,
    EVENT_NATURE                        = 10,
    EVENT_HOLY                          = 11,
    EVENT_BERSERK                       = 12,
    EVENT_DESIRE                        = 13,
    EVENT_SET_NORMAL                    = 14,
    EVENT_SPECIAL                       = 15,
    EVENT_ENRAGE                        = 16,
    EVENT_CHECK_TANK                    = 17,
    EVENT_ENRAGE_2                      = 18,
    EVENT_INTRO                         = 19,
    EVENT_INTRO2                        = 20,
    EVENT_GAS                           = 21,
    EVENT_PLAGUE                        = 22,
};

enum Actions
{
    ACTION_HOLY                 = 1,
    ACTION_FIRE                 = 2,
    ACTION_WATER                = 3,
    ACTION_SHADOW               = 4,
    ACTION_FROST                = 5,
    ACTION_NATURE               = 6,
    ACTION_DESIRE               = 7,
    ACTION_BERSEKER             = 8,
    ACTION_ADD_DIED             = 9,
    ACTION_INTRO                = 10,
    ACTION_GAS                  = 11,
    ACTION_PLAGUE               = 12,
};

enum MovementPoints
{
    POINT_CENTER    = 1,
};

enum Phases
{
    PHASE_NORMAL        = 1,
    PHASE_SPECIAL       = 2,
    PHASE_ADDS_MOVE     = 3,
};

enum Adds
{
    ADD_HOLY    = 100000,
    ADD_FIRE    = 100001,
    ADD_SHADOW  = 100002,
    ADD_FROST   = 100003,
    ADD_WATER   = 100004,
    ADD_NATURE  = 100005,
    ADD_DESIRE  = 100006,
    ADD_BERSERK = 100007,
    ADD_GAS     = 100013,
    ADD_PLAGUE  = 100014,
};

Position const centerPosBeemz  = {4181.3984f, 2484.4849f, 211.0332f, 3.1288f};

const float centerPosition[4] = {4181.3984f, 2484.4849f, 211.0332f, 3.1288f};

Position const AddSpawn[4]   =
{
    {4258.33f, 2484.29f, 211.033f, 3.14338f},
    {4181.09f, 2561.40f, 211.033f, 4.57847f},
    {4181.14f, 2407.12f, 211.033f, 1.62228f},
    {4128.79f, 2484.17f, 211.033f, 6.27942f},
};

Position const AddEnrageSpawn[10]   =
{
    {4153.36f, 2525.18f, 211.033f, 5.3478f},
    {4223.64f, 2512.48f, 211.033f, 3.8124f},
    {4210.11f, 2442.05f, 211.033f, 2.3005f},
    {4139.91f, 2456.51f, 211.033f, 0.6126f},
    {4155.44f, 2442.21f, 211.033f, 0.8906f},
    {4223.23f, 2456.34f, 211.033f, 2.3640f},
    {4209.04f, 2526.87f, 211.033f, 4.0754f},
    {4138.42f, 2512.19f, 211.033f, 5.5558f},
    {4186.77f, 2400.8f, 211.033f, 1.60464f},
    {4186.46f, 2568.6f, 211.033f, 4.70304f},
};

class boss_gunship_ersatz : public CreatureScript
{
public:
    boss_gunship_ersatz() : CreatureScript("boss_gunship_ersatz") { }

    struct boss_gunship_ersatzAI : public BossAI
    {
        boss_gunship_ersatzAI(Creature* creature) : BossAI(creature, DATA_GUNSHIP_ERSATZBOSS)
        {
            maxHealth = me->GetHealth();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_IMMUNE_TO_PC);
            justSummoned = true;
        }

        void Reset()
        {
            _Reset();
            me->RemoveAllAuras();
            enrage = false;
            me->SetMaxHealth(maxHealth);
            instance->SetData(DATA_BOSS_GUNSHIP_ERSATZ,NOT_STARTED);
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            // gunship achievement
            instance->DoCompleteAchievement(RAID_MODE(4536, 4612));
            instance->SetData(DATA_BOSS_GUNSHIP_ERSATZ, DONE);
        }

        void SummonAdds()
        {
            std::list<uint32> entries;
            entries.push_back(ADD_HOLY);
            entries.push_back(ADD_FIRE);
            entries.push_back(ADD_SHADOW);
            entries.push_back(ADD_FROST);
            entries.push_back(ADD_WATER);
            entries.push_back(ADD_NATURE);
            entries.push_back(ADD_DESIRE);
            entries.push_back(ADD_BERSERK);
            entries.push_back(ADD_GAS);
            entries.push_back(ADD_PLAGUE);

            Trinity::Containers::RandomResizeList(entries, uint32(IsHeroic() ? 4 : 3));
            uint8 i = 0;
            for (std::list<uint32>::const_iterator itr = entries.begin(); itr != entries.end(); ++itr, ++i)
                me->SummonCreature(*itr, AddSpawn[i]);
        }

        void SummonAllAdds()
        {
            std::list<uint32> entries;
            entries.push_back(ADD_HOLY);
            entries.push_back(ADD_FIRE);
            entries.push_back(ADD_SHADOW);
            entries.push_back(ADD_FROST);
            entries.push_back(ADD_WATER);
            entries.push_back(ADD_NATURE);
            entries.push_back(ADD_DESIRE);
            entries.push_back(ADD_BERSERK);
            entries.push_back(ADD_GAS);
            entries.push_back(ADD_PLAGUE);

            Trinity::Containers::RandomResizeList(entries, 10);
            uint8 i = 0;
            for (std::list<uint32>::const_iterator itr = entries.begin(); itr != entries.end(); ++itr, ++i)
                me->SummonCreature(*itr, AddEnrageSpawn[i]);
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_HOLY:
                    events.ScheduleEvent(EVENT_HOLY,100);
                    break;
                case ACTION_FIRE:
                    events.ScheduleEvent(EVENT_FIRE,200);
                    break;
                case ACTION_SHADOW:
                    me->CastSpell(me, SPELL_SOULSTORM_VISUAL, true);
                    me->AddAura(SPELL_SOULSTORM, me);
                    break;
                case ACTION_FROST:
                    DoCast(me, SPELL_FROST_AURA);
                    DoCast(me, SPELL_PERMAEATING_CHILL);
                    break;
                case ACTION_NATURE:
                    DoCastAOE(SPELL_LEECHING_SWARM);
                    break;
                case ACTION_DESIRE:
                    DoCast(me, SPELL_AURA_OF_DESIRE);
                    break;
                case ACTION_BERSEKER:
                    me->AddAura(SPELL_BERSERKER,me);
                    break;
                case ACTION_WATER:
                    me->SetMaxHealth(me->GetHealth());
                    me->SetPower(POWER_MANA, me->GetMaxPower(POWER_MANA));
                    DoCast(me, SPELL_MANA_BARRIER, true);
                    enrage = true;
                    events.ScheduleEvent(EVENT_ENRAGE,40000);
                    break;
                case ACTION_ADD_DIED:
                    for (SummonList::iterator i = summons.begin(); i != summons.end(); ++i)
                    {
                        Creature* summon = ObjectAccessor::GetCreature(*me, *i);
                        if (summon)
                        {
                            summon->RemoveAllAuras();
                            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                        };
                    }
                    break;
                case ACTION_INTRO:
                    DoCast(me, SPELL_ARRIVAL, true);
                    instance->SetData(DATA_BOSS_GUNSHIP_ERSATZ,IN_PROGRESS);
                    events.ScheduleEvent(EVENT_INTRO,8000);
                    break;
                case ACTION_GAS:
                    events.ScheduleEvent(EVENT_GAS,4000);
                    break;
                case ACTION_PLAGUE:
                    events.ScheduleEvent(EVENT_PLAGUE,6000);
                    break;
                default:
                    break;
            }
        }

        void KilledUnit(Unit* )
        {
            Talk(SAY_KILLED_UNIT);
        }

        void EnterCombat(Unit* /*attacker*/)
        {
            _EnterCombat();
            events.ScheduleEvent(EVENT_BONE_SLICE, urand(3000,10000));
            events.ScheduleEvent(EVENT_SWARMING_SHADOWS , 15000);
            events.ScheduleEvent(RAND(EVENT_FATAL_ATRACTION,EVENT_PACKT_OF_THE_DARKFALLEN) , 5000);
            events.ScheduleEvent(RAND(EVENT_FATAL_ATRACTION,EVENT_PACKT_OF_THE_DARKFALLEN), 20000);
            events.ScheduleEvent(EVENT_SUMMON_ADDS, 30000);
            events.ScheduleEvent(EVENT_ENRAGE_2 , 476000); // 6 Add Phasen
        }

        void DamageTaken(Unit* damageDealer, uint32& damage)
        {
             if (me->HasAura(SPELL_MANA_BARRIER) && int32(damage) > me->GetPower(POWER_MANA))
             {
                 damage -= me->GetPower(POWER_MANA);
                 me->RemoveAurasDueToSpell(SPELL_MANA_BARRIER);
                 me->SetPower(POWER_MANA, me->GetMaxPower(POWER_MANA));
                 enrage = false;
             }

             if (damageDealer == me)
                 return;

             if (damageDealer->HasAura(SPELL_AURA_OF_DESIRE))
             {
                 int32 bp0 = damage / 2;
                 me->CastCustomSpell(damageDealer, SPELL_AURA_OF_DESIRE_DAMAGE, &bp0, NULL, NULL, true);
             }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!justSummoned)
                if (!UpdateVictim())
                    return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_BONE_SLICE:
                        DoCastVictim(SPELL_BONE_SLICE);
                        events.ScheduleEvent(EVENT_BONE_SLICE, urand(3000,10000));
                        break;
                    case EVENT_SWARMING_SHADOWS:
                    {
                        std::list<Player*> targets;
                        SelectRandomTarget(&targets);
                        Trinity::Containers::RandomResizeList<Player*>(targets, uint32(Is25ManRaid() ? 4 : 2));
                        for (std::list<Player*>::iterator itr = targets.begin(); itr != targets.end(); ++itr)
                            DoCast(*itr, SPELL_SWARMING_SHADOWS);
                        events.ScheduleEvent(EVENT_SWARMING_SHADOWS , 35000, 0, PHASE_NORMAL);
                        Talk(SAY_SWARMING_SHADOWS);
                        break;
                    }
                    case EVENT_FATAL_ATRACTION:
                    {
                        std::list<Player*> targets;
                        SelectRandomTarget(&targets);
                        uint32 targetCount = 2;
                        // not sure if this works also for this Spell (Spell needs script anyway...)
                        if (IsHeroic())
                            ++targetCount;
                        if (Is25ManRaid())
                            ++targetCount;
                        Trinity::Containers::RandomResizeList<Player*>(targets, targetCount);
                        if (targets.size() > 1)
                            for (std::list<Player*>::iterator itr = targets.begin(); itr != targets.end(); ++itr)
                                DoCast(*itr, SPELL_FATAL_ATRACTION);
                        events.ScheduleEvent(EVENT_FATAL_ATRACTION , 35000, 0, PHASE_NORMAL);
                        break;
                    }
                    case EVENT_PACKT_OF_THE_DARKFALLEN:
                        {
                            std::list<Player*> targets;
                            SelectRandomTarget(&targets);
                            uint32 targetCount = 2;
                        if (IsHeroic())
                            ++targetCount;
                        if (Is25ManRaid())
                            ++targetCount;
                        Trinity::Containers::RandomResizeList<Player*>(targets, targetCount);
                        if (targets.size() > 1)
                            for (std::list<Player*>::iterator itr = targets.begin(); itr != targets.end(); ++itr)
                                DoCast(*itr, SPELL_PACKT_OF_THE_DARKFALLEN);
                        break;
                    }
                    case EVENT_SUMMON_ADDS:
                        if (!events.IsInPhase(PHASE_NORMAL))
                            events.SetPhase(PHASE_NORMAL);
                        me->RemoveAllAuras();
                        me->GetMotionMaster()->Clear();
                        me->SetWalk(false);
                        me->SetMaxHealth(maxHealth);
                        DoCast(me, SPELL_TELE_VISUAL);
                        DoTeleportTo(centerPosition);
                        me->UpdatePosition(centerPosBeemz,true);
                        Talk(SAY_ADD_SPAWN);
                        SummonAdds();
                        events.SetPhase(PHASE_ADDS_MOVE);
                        events.ScheduleEvent(EVENT_CHECK_TANK, 4000, 0 , PHASE_ADDS_MOVE);
                        events.ScheduleEvent(EVENT_SUMMON_ADDS, 76000);
                        events.ScheduleEvent(EVENT_SPECIAL, 36000);
                        break;
                    case EVENT_SPECIAL:
                        events.SetPhase(PHASE_SPECIAL);
                        me->SetWalk(true);
                        me->GetMotionMaster()->MoveChase(me->getVictim());
                        break;
                    case EVENT_HOLY:
                        DoCastToAllHostilePlayers(SPELL_SEARING_LIGHT);
                        events.RescheduleEvent(EVENT_HOLY, 11000, 0,PHASE_SPECIAL);
                        break;
                    case EVENT_FIRE:
                    {
                        std::list<Player*> targets;
                        SelectRandomTarget(&targets);
                        uint32 targetCount = 2;
                        if (IsHeroic())
                            if (Is25ManRaid())
                                targetCount += 2;
                            else ++targetCount;
                        if (Is25ManRaid())
                            targetCount += 2;
                        Trinity::Containers::RandomResizeList<Player*>(targets, uint32(Is25ManRaid() ? 6 : 2));
                        for (std::list<Player*>::iterator itr = targets.begin(); itr != targets.end(); ++itr)
                            DoCast(*itr, SPELL_LEGION_FLAME);
                        events.RescheduleEvent(EVENT_FIRE, 21000, 0,PHASE_SPECIAL);
                        break;
                    }
                    case EVENT_ENRAGE:
                        if (enrage)
                        {
                            DoCast(me, SPELL_BERSERK);
                            Talk(SAY_BERSERK);
                        }
                        break;
                    case EVENT_ENRAGE_2:
                        events.CancelEvent(EVENT_SUMMON_ADDS);
                        SummonAllAdds();
                        Talk(SAY_BERSERK);
                        break;
                    case EVENT_CHECK_TANK:
                    {
                        Unit* target = me->SelectNearestPlayer(3.0f);
                        if (!target)
                            DoCastAOE(SPELL_UNYIELDING_PAIN);
                        else
                            AttackStart(target);

                        events.ScheduleEvent(EVENT_CHECK_TANK, 2000, 0,PHASE_ADDS_MOVE);
                        break;
                    }
                    case EVENT_INTRO:
                        Talk(SAY_INTRO);
                        events.ScheduleEvent(EVENT_INTRO2, 13000);
                        break;
                    case EVENT_INTRO2:
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                        me->SetReactState(REACT_AGGRESSIVE);
                        justSummoned = false;
                        break;
                    case EVENT_GAS:
                        DoCastAOE(SPELL_GAS_NOVA);
                        events.RescheduleEvent(EVENT_GAS, 12000, 0,PHASE_SPECIAL);
                        break;
                    case EVENT_PLAGUE:
                        DoCastVictim(SPELL_CARRION_SWARM);
                        events.RescheduleEvent(EVENT_PLAGUE, 21000, 0,PHASE_SPECIAL);
                    default:
                        break;
                }
            }
        DoMeleeAttackIfReady();
        }

    private:
        uint32 maxHealth;
        bool enrage;
        bool justSummoned;
        Player* SelectRandomTarget(std::list<Player*>* targetList = NULL )
        {
            std::list<HostileReference*> const& threatlist = me->getThreatManager().getThreatList();
            std::list<Player*> tempTargets;

            if (threatlist.empty())
                return NULL;

            for (std::list<HostileReference*>::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
                if (Unit* refTarget = (*itr)->getTarget())
                    if (refTarget != me->getVictim() && refTarget->GetTypeId() == TYPEID_PLAYER && !refTarget->HasAura(SPELL_PACKT_OF_THE_DARKFALLEN) && !refTarget->HasAura(SPELL_FATAL_ATRACTION) && !refTarget->HasAura(SPELL_SWARMING_SHADOWS))
                        tempTargets.push_back(refTarget->ToPlayer());

            if (tempTargets.empty())
                return NULL;

            if (targetList)
            {
                *targetList = tempTargets;
                return NULL;
            }

            return Trinity::Containers::SelectRandomContainerElement(tempTargets);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return GetIcecrownCitadelAI<boss_gunship_ersatzAI>(creature);
    }
};

class npc_gunship_ersatz_add : public CreatureScript
{
public:
    npc_gunship_ersatz_add() : CreatureScript("npc_gunship_ersatz_add") { }

    struct npc_gunship_ersatz_addAI : public ScriptedAI
    {
        npc_gunship_ersatz_addAI(Creature* creature) : ScriptedAI(creature), _instance(creature->GetInstanceScript())
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
        }

        void Reset()
        {
            float healthmod = 1.0f;
            if (Is25ManRaid())
                healthmod *= 4.0f;
            if (IsHeroic())
                healthmod *= 1.5f;
            me->SetMaxHealth(me->GetHealth() * healthmod);
            me->SetHealth(me->GetMaxHealth());
            me->SetSpeed(MOVE_WALK, 0.3f);  // walk
            me->SetSpeed(MOVE_RUN, 0.3f);   // run
            me->SetReactState(REACT_PASSIVE);
            move = false;
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
        }

        void MovementInform(uint32 uiType, uint32 uiId)
        {
            if (uiId != POINT_CENTER && uiType != POINT_MOTION_TYPE)
                return;

            switch (me->GetEntry())
            {
                case ADD_HOLY:
                    if (Creature* ersatzboss = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_GUNSHIP_ERSATZBOSS)))
                        ersatzboss->AI()->DoAction(ACTION_HOLY);
                    break;
                case ADD_FIRE:
                    if (Creature* ersatzboss = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_GUNSHIP_ERSATZBOSS)))
                        ersatzboss->AI()->DoAction(ACTION_FIRE);
                    break;
                case ADD_SHADOW:
                    if (Creature* ersatzboss = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_GUNSHIP_ERSATZBOSS)))
                        ersatzboss->AI()->DoAction(ACTION_SHADOW);
                    break;
                case ADD_FROST:
                    if (Creature* ersatzboss = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_GUNSHIP_ERSATZBOSS)))
                        ersatzboss->AI()->DoAction(ACTION_FROST);
                    break;
                case ADD_WATER:
                    if (Creature* ersatzboss = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_GUNSHIP_ERSATZBOSS)))
                        ersatzboss->AI()->DoAction(ACTION_WATER);
                    break;
                case ADD_NATURE:
                    if (Creature* ersatzboss = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_GUNSHIP_ERSATZBOSS)))
                        ersatzboss->AI()->DoAction(ACTION_NATURE);
                    break;
                case ADD_DESIRE:
                    if (Creature* ersatzboss = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_GUNSHIP_ERSATZBOSS)))
                        ersatzboss->AI()->DoAction(ACTION_DESIRE);
                    break;
                case ADD_BERSERK:
                        if (Creature* ersatzboss = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_GUNSHIP_ERSATZBOSS)))
                        ersatzboss->AI()->DoAction(ACTION_BERSEKER);
                    break;
                case ADD_GAS:
                    if (Creature* ersatzboss = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_GUNSHIP_ERSATZBOSS)))
                        ersatzboss->AI()->DoAction(ACTION_GAS);
                    break;
                case ADD_PLAGUE:
                    if (Creature* ersatzboss = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_GUNSHIP_ERSATZBOSS)))
                        ersatzboss->AI()->DoAction(ACTION_PLAGUE);
                    break;
                default:
                    break;
            }

            me->DespawnOrUnsummon(3000);
        }

        void JustDied(Unit* /* killer */)
        {
            if (Creature* ersatzboss = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_GUNSHIP_ERSATZBOSS)))
                ersatzboss->AI()->DoAction(ACTION_ADD_DIED);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!move)
            {
                me->GetMotionMaster()->MovePoint(POINT_CENTER, centerPosBeemz);
                move = true;
            }
        }
    private:
        InstanceScript* _instance;
        bool move;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gunship_ersatz_addAI(creature);
    }
};

class npc_spawn_ersatzboss : public CreatureScript
{
public:
    npc_spawn_ersatzboss() : CreatureScript("npc_spawn_ersatzboss") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*Sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        player->PlayerTalkClass->SendCloseGossip();
        if (action == GOSSIP_ACTION_INFO_DEF+1)
        {
           if (Creature* gunship = creature->SummonCreature(NPC_GUNSHIP_ERSAZBOSS, centerPosBeemz))
           {
                gunship->GetAI()->DoAction(ACTION_INTRO);
                creature->DespawnOrUnsummon();
           }
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        player->PlayerTalkClass->ClearMenus();
        if (InstanceScript* instance = player->GetInstanceScript())
            if ((instance->GetData(DATA_BOSS_GUNSHIP_ERSATZ) != DONE && instance->GetData(DATA_BOSS_GUNSHIP_ERSATZ) != IN_PROGRESS)|| player->isGameMaster())
                if ((player->GetGroup() && player->GetGroup()->IsLeader(player->GetGUID())) || player->isGameMaster())
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_LEADER, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }
};

class spell_icc_80_damage4effAll : public SpellScriptLoader
{
public:
    spell_icc_80_damage4effAll() : SpellScriptLoader("spell_icc_80_damage4effAll") { }

    class spell_icc_80_damage4effAll_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_icc_80_damage4effAll_SpellScript)

            bool Load()
        {
            if (Unit* caster = GetCaster())
            {
                if (InstanceScript* instance = caster->GetInstanceScript())
                {
                    if (caster->GetMapId() == 631 )
                    {
                        SetSpellValue(SPELLVALUE_BASE_POINT0, (GetSpellInfo()->Effects[EFFECT_0].BasePoints+1) * 4);
                        SetSpellValue(SPELLVALUE_BASE_POINT1, (GetSpellInfo()->Effects[EFFECT_1].BasePoints+1) * 4);
                        SetSpellValue(SPELLVALUE_BASE_POINT2, (GetSpellInfo()->Effects[EFFECT_2].BasePoints+1) * 4);
                    }
                }
            }
            return true;
        }

        void Register(){}
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_icc_80_damage4effAll_SpellScript();
    }
};

void AddSC_boss_highking_beemz()
{
    new boss_gunship_ersatz();
    new npc_gunship_ersatz_add();
    new npc_spawn_ersatzboss();
    new spell_icc_80_damage4effAll();
}
