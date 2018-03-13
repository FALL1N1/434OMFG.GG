/*
 * Copyright (C) 2013-2015 OMFG.GG Network <http://www.omfg.gg/>
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

#include "ScriptPCH.h"
#include "well_of_eternity.h"

/*
TODO:
- Implement Outro
- Implement coldflame (Frost Magus)
*/

enum Talks
{
    TALK_QUEEN_ENTER_COMBAT             = -55919,
    TALK_QUEEN_ENTER_COMBAT_2           = -53300,
    TALK_QUEEN_TOTAL_OBEDIENCE          = -55920,
    TALK_QUEEN_TOTAL_OBEDIENCE_EMOTE    = -54845,
    TALK_QUEEN_INTERRUPTED              = -53305,
    TALK_QUEEN_COMBAT_END               = -53280,
    TALK_FROST_ENTER_COMBAT             = -53285,
};

enum Spells
{
    SPELL_SHROUD_OF_LUMINOSITY  = 102915,
    SPELL_SERVANT_OF_THE_QUEEN  = 102334,
    SPELL_TOTAL_OBEDIENCE       = 103241,
    SPELL_TOTAL_OBEDIENCE_STUN  = 110096,
    SPELL_ACTIVATE_WAVE_ONE     = 102681,
    SPELL_ACTIVATE_WAVE_TWO     = 102684,
    SPELL_ACTIVATE_WAVE_THREE   = 102685,
    SPELL_RIDE_VEHICLE_AZSHARA  = 93970,

    // Frost Magus
    SPELL_ICE_FLING             = 102478,
    SPELL_COLDFLAME_PERIODIC    = 102465,
    SPELL_COLDFLAME_AREA        = 102466,
    SPELL_BLADES_OF_ICE         = 102467,
    SPELL_FROST_CHANNELING      = 110492,

    // Fire Magus
    SPELL_FIREBALL              = 102265,
    SPELL_FIREBOMB              = 102482,
    SPELL_BLAST_WAVE            = 102483,
    SPELL_FIRE_CHANNELING       = 110494,

    // Arcane Magus
    SPELL_ARCANE_SHOCK          = 102463,
    SPELL_ARCANE_CHANNELING     = 110495,
    SPELL_HAMMER_OF_RECKONING   = 102453,
    SPELL_HAMMER_OF_RECKONING_2 = 102454,

    // Arcane Bomb
    SPELL_ARCANE_BOMB_VISUAL    = 109122,
    SPELL_ARCANE_BOMB           = 102455,
    SPELL_ARCANE_BOMB_GROUND    = 102460,

    // Hand of the Queen
    SPELL_RIDE_VEHICLE          = 46598,
    SPELL_PUPPET_STRING         = 102319,
    SPELL_PUPPET_STRING_2       = 102318,
    SPELL_PUPPET_STRING_3       = 102315,
    SPELL_PUPPET_STRING_HOVER   = 102312,

    // Wave Helpers
    SPELL_WAVE_1                = 97848,
    SPELL_WAVE_2                = 97849,
    SPELL_WAVE_3                = 97850,
};

enum Events
{
    EVENT_SERVANT_OF_THE_QUEEN  = 1,
    EVENT_TOTAL_OBEDIENCE       = 2,
    EVENT_ACTIVATE_MAGES        = 3,
    EVENT_PLAYER_ALIVE_CHECK    = 4,
    EVENT_PLAYER_ACTION         = 5,

    // Fire Mage
    EVENT_FIREBALL              = 1,
    EVENT_FIREBOMB              = 2,
    EVENT_BLAST_WAVE            = 3,
    EVENT_ENTER_COMBAT          = 4,

    // Arcane Mage
    EVENT_ARCANE_BOMB           = 1,
    EVENT_ARCANE_SHOCK          = 2,

    // Frost Mage
    EVENT_ICE_FLING             = 1,
    EVENT_COLD_FLAME            = 2,
    EVENT_BLADES_OF_ICE         = 3
};

static const WellOfEternityEventSpawns mageSpawnPos[6] =
{
    { NPC_FIRE_MAGE,    3453.030f, -5282.740f, 230.031f, 4.4505f },
    { NPC_FROST_MAGE,   3443.540f, -5280.370f, 230.031f, 4.6600f },
    { NPC_ARCANE_MAGE,  3461.139f, -5283.169f, 230.031f, 4.3109f },
    { NPC_FIRE_MAGE,    3435.590f, -5278.350f, 230.031f, 4.5029f },
    { NPC_FROST_MAGE,   3469.729f, -5282.430f, 230.031f, 4.5378f },
    { NPC_ARCANE_MAGE,  3428.429f, -5274.589f, 230.031f, 4.2062f }
};

const Position deathPos = { 3456.799f, -5245.124f, 229.949f, 0.000f };

#define QUEST_THE_VAINGLORIOUS  30100
#define POINT_GROUND    1

class HandPositionRelocateHack : public BasicEvent
{
public:
    HandPositionRelocateHack(Creature* owner) : _owner(owner) {}

    bool Execute(uint64 execTime, uint32 /*diff*/) override
    {
        if (_owner->GetVehicleBase())
        {
            Position pos(*_owner->GetVehicleBase());
            _owner->Relocate(pos);
            _owner->m_Events.AddEvent(this, execTime + 100);
            return false;
        }
        return true;
    }

private:
    Creature* _owner;
};

class boss_woe_queen_azshara : public CreatureScript
{
public:
    boss_woe_queen_azshara() : CreatureScript("boss_woe_queen_azshara") {}

    struct boss_woe_queen_azsharaAI : public BossAI
    {
        boss_woe_queen_azsharaAI(Creature* creature) : BossAI(creature, DATA_QUEEN_AZSHARA) 
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            me->SetReactState(REACT_PASSIVE);
        }

        void Reset() override
        {
            _Reset();
            waveCounter = 0;
            mageKillCounter = 0;
            summonCount = 0;
            for (int8 i = 0; i < 6; i++)
                me->SummonCreature(mageSpawnPos[i].npcId, mageSpawnPos[i].x, mageSpawnPos[i].y, mageSpawnPos[i].z, mageSpawnPos[i].orientation, TEMPSUMMON_MANUAL_DESPAWN);
            DoCast(me, SPELL_SHROUD_OF_LUMINOSITY, true);
        }

        void EnterCombat(Unit* victim) override
        {
            Talk(TALK_QUEEN_ENTER_COMBAT);
            TalkWithDelay(9000, TALK_QUEEN_ENTER_COMBAT_2);
            events.ScheduleEvent(EVENT_ACTIVATE_MAGES, 16000);
            events.ScheduleEvent(EVENT_SERVANT_OF_THE_QUEEN, 24000);
            events.ScheduleEvent(EVENT_PLAYER_ACTION, 25000);
            events.ScheduleEvent(EVENT_TOTAL_OBEDIENCE, 36000);
            events.ScheduleEvent(EVENT_PLAYER_ALIVE_CHECK, 1000);
            _EnterCombat();
        }

        void JustDied(Unit* killer) override
        {
            _JustDied();
        }

        void JustSummoned(Creature* summon) override
        {
            summonCount++;
            switch (summon->GetEntry())
            {
                case NPC_FIRE_MAGE:
                    if (summonCount == 1)
                        summon->AddAura(SPELL_WAVE_1, summon);
                    else
                        summon->AddAura(SPELL_WAVE_2, summon);
                    summon->CastSpell(summon, SPELL_FIRE_CHANNELING, false);
                    break;
                case NPC_ARCANE_MAGE:
                    if (summonCount == 3)
                        summon->AddAura(SPELL_WAVE_2, summon);
                    else
                        summon->AddAura(SPELL_WAVE_3, summon);
                    summon->CastSpell(summon, SPELL_ARCANE_CHANNELING, false);
                    break;
                case NPC_FROST_MAGE:
                    if (summonCount == 2)
                        summon->AddAura(SPELL_WAVE_1, summon);
                    else
                        summon->AddAura(SPELL_WAVE_3, summon);
                    summon->CastSpell(summon, SPELL_FROST_CHANNELING, false);
                    break;
                default:
                    break;
            }
            BossAI::JustSummoned(summon);
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            switch (summon->GetEntry())
            {
                case NPC_HAND_OF_THE_QUEEN:
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PUPPET_STRING, summon->GetGUID());
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PUPPET_STRING_2, summon->GetGUID());
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PUPPET_STRING_3, summon->GetGUID());
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PUPPET_STRING_HOVER, summon->GetGUID());
                    break;
                case NPC_ARCANE_MAGE:
                case NPC_FIRE_MAGE:
                case NPC_FROST_MAGE:
                    if (++mageKillCounter >= 6)
                    {
                        Talk(TALK_QUEEN_COMBAT_END);
                        me->setFaction(35);
                        me->CombatStop(true);
                        me->RemoveAurasDueToSpell(SPELL_SHROUD_OF_LUMINOSITY);
                        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SERVANT_OF_THE_QUEEN);
                        if (GameObject* elune = instance->GetGameObject(DATA_LIGHT_OF_ELUNE))
                            elune->SetRespawnTime(elune->GetRespawnDelay());

                        if (GameObject* cache = instance->GetGameObject(DATA_ROYAL_CACHE))
                        {
                            cache->SetRespawnTime(cache->GetRespawnDelay());
                            cache->SetLootRecipient(me->GetLootRecipient());
                        }

                        Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();
                        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                            if (Player* player = itr->getSource())
                                if (player->GetQuestStatus(QUEST_THE_VAINGLORIOUS) == QUEST_STATUS_INCOMPLETE)
                                    player->KilledMonsterCredit(me->GetEntry());

                        if (Creature* nozdormu = instance->GetCreature(DATA_NOZDORMU))
                            nozdormu->NearTeleportTo(NozdormuPositions[1]);

                        me->DespawnOrUnsummon(10000);
                        _JustDied();
                    }
                    break;
                default:
                    break;
            }
        }

        void SpellHit(Unit* attacker, SpellInfo const* spellInfo) override
        {
            if (Spell* spell = me->GetCurrentSpell(CURRENT_GENERIC_SPELL))
            {
                if (spell->GetSpellInfo()->Id == SPELL_TOTAL_OBEDIENCE)
                {
                    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                    {
                        if (spellInfo->Effects[i].Mechanic == MECHANIC_INTERRUPT || spellInfo->Mechanic == MECHANIC_INTERRUPT)
                        {
                            Talk(TALK_QUEEN_INTERRUPTED);
                            me->InterruptSpell(CURRENT_GENERIC_SPELL);
                            break;
                        }
                    }
                }
            }
        }

        void UpdateAI(uint32 const diff) override
        {
            events.Update(diff);

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_SERVANT_OF_THE_QUEEN:
                        if (Player* target = SelectRandomTarget(false))
                        {
                            Position pos(*target);
                            if (Creature* hand = me->SummonCreature(NPC_HAND_OF_THE_QUEEN, pos, TEMPSUMMON_DEAD_DESPAWN))
                            {
                                hand->CastSpell(target, SPELL_SERVANT_OF_THE_QUEEN, true);
                                hand->m_Events.AddEvent(new HandPositionRelocateHack(hand), hand->m_Events.CalculateTime(1000));
                            }
                        }
                        events.ScheduleEvent(EVENT_SERVANT_OF_THE_QUEEN, 26000);
                        break;
                    case EVENT_TOTAL_OBEDIENCE:
                        Talk(TALK_QUEEN_TOTAL_OBEDIENCE);
                        Talk(TALK_QUEEN_TOTAL_OBEDIENCE_EMOTE, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                        DoCast(me, SPELL_TOTAL_OBEDIENCE);
                        events.ScheduleEvent(EVENT_TOTAL_OBEDIENCE, 37000);
                        break;
                    case EVENT_ACTIVATE_MAGES:
                        DoCast(me, ++waveCounter == 1 ? SPELL_ACTIVATE_WAVE_ONE : (waveCounter == 2 ? SPELL_ACTIVATE_WAVE_TWO : SPELL_ACTIVATE_WAVE_THREE), true);
                        if (waveCounter < 3)
                            events.ScheduleEvent(EVENT_ACTIVATE_MAGES, 36000);
                        break;
                    case EVENT_PLAYER_ALIVE_CHECK:
                    {
                        bool alivePlayerFound = false;
                        Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();
                        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                            if (Player* player = itr->getSource())
                                if (player->isAlive() && !player->isGameMaster())
                                {
                                    alivePlayerFound = true;
                                    break;
                                }

                        if (!alivePlayerFound)
                            EnterEvadeMode();
                        else
                            events.ScheduleEvent(EVENT_PLAYER_ALIVE_CHECK, 1000);
                        break;
                    }
                    case EVENT_PLAYER_ACTION:
                    {
                        Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();
                        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                        {
                            if (Player* player = itr->getSource())
                            {
                                if (player->isCharmed() && !player->HasUnitState(UNIT_STATE_CASTING) && player->isAlive())
                                {
                                    if (Player* target = player->getVictim() ? 
                                        ((player->getVictim()->ToPlayer() && player->getVictim()->isAlive() && !player->isCharmed()) ? player->getVictim()->ToPlayer() : SelectRandomTarget(true, player)) 
                                        : SelectRandomTarget(true, player))
                                    {
                                        player->GetMotionMaster()->MoveChase(target, 0.0f, 0.0f, 0, MOTION_SLOT_CONTROLLED);
                                        player->SetTarget(target->GetGUID());

                                        switch (player->getClass())
                                        {
                                            case CLASS_DRUID:
                                                if (urand(0, 1))
                                                    player->CastSpell(target, 8921, false);
                                                else
                                                    player->CastSpell(player, 774, false);
                                                break;
                                            case CLASS_HUNTER:
                                                player->CastSpell(target, RAND(2643, 1978), false);
                                                break;
                                            case CLASS_MAGE:
                                                player->CastSpell(target, RAND(44614, 30455), false);
                                                break;
                                            case CLASS_WARLOCK:
                                                player->CastSpell(target, RAND(980, 686), true);
                                                break;
                                            case CLASS_WARRIOR:
                                                player->CastSpell(target, RAND(46924, 845), false);
                                                break;
                                            case CLASS_PALADIN:
                                                if (urand(0, 1))
                                                    player->CastSpell(target, 853, false);
                                                else
                                                    player->CastSpell(player, 20473, false);
                                                break;
                                            case CLASS_PRIEST:
                                                if (urand(0, 1))
                                                    player->CastSpell(target, 34914, false);
                                                else
                                                    player->CastSpell(player, 139, false);
                                                break;
                                            case CLASS_SHAMAN:
                                                if (urand(0, 1))
                                                    player->CastSpell(target, 421, false);
                                                else
                                                    player->CastSpell(player, 61295, false);
                                                break;
                                            case CLASS_ROGUE:
                                                player->CastSpell(target, RAND(16511, 1329), false);
                                                break;
                                            case CLASS_DEATH_KNIGHT:
                                                if (urand(0, 1))
                                                    player->CastSpell(target, 45462, true);
                                                else
                                                    player->CastSpell(target, 49184, true);
                                                break;
                                            default:
                                                break;
                                        }
                                    }
                                    else
                                        me->Kill(player);
                                }
                            }
                        }
                        events.ScheduleEvent(EVENT_PLAYER_ACTION, 1500);
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    private:
        uint8 waveCounter;
        uint8 summonCount;
        uint8 mageKillCounter;
        Player* SelectRandomTarget(bool includeTank, Player* exceptPlayer = NULL)
        {
            std::list<HostileReference*> const& threatlist = me->getThreatManager().getThreatList();
            std::list<Player*> tempTargets;

            if (threatlist.empty())
                return NULL;

            for (std::list<HostileReference*>::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
                if (Unit* refTarget = (*itr)->getTarget())
                    if (refTarget->GetTypeId() == TYPEID_PLAYER && !refTarget->isCharmed())
                        tempTargets.push_back(refTarget->ToPlayer());

            if (!includeTank)
            {
                tempTargets.remove_if([](WorldObject* target){
                    return target->ToPlayer()->HasTankSpec();
                });
            }

            if (exceptPlayer)
                tempTargets.remove(exceptPlayer);

            if (tempTargets.empty())
                return NULL;

            return Trinity::Containers::SelectRandomContainerElement(tempTargets);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_woe_queen_azsharaAI(creature);
    }
};

class npc_woe_fire_mage : public CreatureScript
{
public:
    npc_woe_fire_mage() : CreatureScript("npc_woe_fire_mage") {}

    struct npc_woe_fire_mageAI : public ScriptedAI
    {
        npc_woe_fire_mageAI(Creature* creature) : ScriptedAI(creature) 
        {
            isActive = false;
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
        }

        bool CanAIAttack(Unit const* target) const
        {
            return isActive;
        }

        void SpellHit(Unit* attacker, SpellInfo const* spellInfo)
        {
            if (!isActive && spellInfo->SpellIconID == 5611)
            {
                me->RemoveAurasDueToSpell(SPELL_WAVE_1);
                me->RemoveAurasDueToSpell(SPELL_WAVE_2);
                events.ScheduleEvent(EVENT_ENTER_COMBAT, 3000);
                events.ScheduleEvent(EVENT_FIREBALL, 3000);
                events.ScheduleEvent(EVENT_FIREBOMB, urand(20000, 30000));
                events.ScheduleEvent(EVENT_BLAST_WAVE, urand(20000, 30000));
            }
        }

        void JustDied(Unit* killer)
        {
            events.Reset();
        }

        void UpdateAI(uint32 const diff) override
        {
            if (isActive && !UpdateVictim())
                return;

            events.Update(diff);

            if (isActive && me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_ENTER_COMBAT:
                        isActive = true;
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                        if (Player* player = me->FindNearestPlayer(200.00f))
                            me->AI()->AttackStart(player);
                        break;
                    case EVENT_FIREBALL:
                        DoCastVictim(SPELL_FIREBALL);
                        events.ScheduleEvent(EVENT_FIREBALL, urand(2500, 3000));
                        break;
                    case EVENT_FIREBOMB:
                        DoCastRandom(SPELL_FIREBOMB, 45.00f);
                        events.ScheduleEvent(EVENT_FIREBOMB, urand(15000, 30000));
                        break;
                    case EVENT_BLAST_WAVE:
                        DoCast(me, SPELL_BLAST_WAVE);
                        events.ScheduleEvent(EVENT_BLAST_WAVE, urand(15000, 30000));
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
        private:
            EventMap events;
            bool isActive;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_woe_fire_mageAI(creature);
    }
};

class npc_woe_arcane_mage : public CreatureScript
{
public:
    npc_woe_arcane_mage() : CreatureScript("npc_woe_arcane_mage") {}

    struct npc_woe_arcane_mageAI : public ScriptedAI
    {
        npc_woe_arcane_mageAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            isActive = false;
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
        }

        bool CanAIAttack(Unit const* target) const override
        {
            return isActive;
        }

        void SpellHit(Unit* attacker, SpellInfo const* spellInfo) override
        {
            if (!isActive && spellInfo->SpellIconID == 5611)
            {
                me->RemoveAurasDueToSpell(SPELL_WAVE_2);
                me->RemoveAurasDueToSpell(SPELL_WAVE_3);
                events.ScheduleEvent(EVENT_ENTER_COMBAT, 3000);
                events.ScheduleEvent(EVENT_ARCANE_BOMB, urand(5000, 10000));
                events.ScheduleEvent(EVENT_ARCANE_SHOCK, urand(10000, 15000));
            }
        }

        void JustSummoned(Creature* summon) override
        {
            switch (summon->GetEntry())
            {
                case NPC_HAMMER_OF_DIVINITY:
                {
                    summon->CastSpell(summon, SPELL_ARCANE_BOMB_VISUAL, true);
                    Position pos(*summon);
                    pos.m_positionZ = 231.9485f;
                    summon->GetMotionMaster()->MovePoint(POINT_GROUND, pos);
                    break;
                }
                case NPC_HAMMER_OF_DIVINITY_2:
                    summon->CastSpell(summon, SPELL_ARCANE_BOMB_GROUND, true);
                    break;
                default:
                    break;
            }
        }

        void SummonedMovementInform(Creature* summon, uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE || id != POINT_GROUND)
                return;

            if (summon->GetEntry() == NPC_HAMMER_OF_DIVINITY)
            {
                summon->RemoveAurasDueToSpell(SPELL_ARCANE_BOMB_VISUAL);
                if (Creature* groundVisual = summon->FindNearestCreature(NPC_HAMMER_OF_DIVINITY_2, 20.00f))
                    groundVisual->RemoveAurasDueToSpell(SPELL_ARCANE_BOMB_GROUND);
                summon->CastSpell(summon, SPELL_ARCANE_BOMB, true, NULL, NULL, me->GetGUID());
            }
        }

        void UpdateAI(uint32 const diff) override
        {
            if (isActive && !UpdateVictim())
                return;

            events.Update(diff);

            if (isActive && me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_ENTER_COMBAT:
                        isActive = true;
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                        if (Player* player = me->FindNearestPlayer(200.00f))
                            me->AI()->AttackStart(player);
                        break;
                    case EVENT_ARCANE_BOMB:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.00f, true))
                        {
                            DoCast(target, SPELL_HAMMER_OF_RECKONING, true);
                            DoCast(target, SPELL_HAMMER_OF_RECKONING_2, true);
                        }
                        events.ScheduleEvent(EVENT_ARCANE_BOMB, urand(15000, 20000));
                        break;
                    case EVENT_ARCANE_SHOCK:
                        DoCast(me, SPELL_ARCANE_SHOCK, false);
                        events.ScheduleEvent(EVENT_ARCANE_SHOCK, urand(20000, 30000));
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
            bool isActive;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_woe_arcane_mageAI(creature);
    }
};

class npc_woe_frost_mage : public CreatureScript
{
public:
    npc_woe_frost_mage() : CreatureScript("npc_woe_frost_mage") {}

    struct npc_woe_frost_mageAI : public ScriptedAI
    {
        npc_woe_frost_mageAI(Creature* creature) : ScriptedAI(creature) 
        {
            isActive = false;
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
        }

        bool CanAIAttack(Unit const* target) const
        {
            return isActive;
        }

        void SpellHit(Unit* attacker, SpellInfo const* spellInfo) override
        {
            if (!isActive && spellInfo->SpellIconID == 5611)
            {
                Talk(TALK_FROST_ENTER_COMBAT);
                me->RemoveAurasDueToSpell(SPELL_WAVE_1);
                me->RemoveAurasDueToSpell(SPELL_WAVE_3);
                events.ScheduleEvent(EVENT_ENTER_COMBAT, 3000);
                events.ScheduleEvent(EVENT_ICE_FLING, urand(5000, 10000));
                events.ScheduleEvent(EVENT_COLD_FLAME, urand(10000, 15000));
                events.ScheduleEvent(EVENT_BLADES_OF_ICE, urand(15000, 20000));
            }
        }

        void UpdateAI(uint32 const diff) override
        {
            if (isActive && !UpdateVictim())
                return;

            events.Update(diff);

            if (isActive && me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_ENTER_COMBAT:
                        isActive = true;
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                        if (Player* player = me->FindNearestPlayer(200.00f))
                            me->AI()->AttackStart(player);
                        break;
                    case EVENT_ICE_FLING:
                        DoCast(me, SPELL_ICE_FLING, false);
                        events.ScheduleEvent(EVENT_ICE_FLING, urand(2500, 5000));
                        break;
                    case EVENT_COLD_FLAME:
                        //NYI
                        break;
                    case EVENT_BLADES_OF_ICE:
                        DoCastRandom(SPELL_BLADES_OF_ICE, 40.00f);
                        events.ScheduleEvent(EVENT_BLADES_OF_ICE, urand(20000, 30000));
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
        bool isActive;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_woe_frost_mageAI(creature);
    }
};

class spell_woe_servant_of_the_queen : public SpellScriptLoader
{
public:
    spell_woe_servant_of_the_queen() : SpellScriptLoader("spell_woe_servant_of_the_queen") { }

    class spell_woe_servant_of_the_queen_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_woe_servant_of_the_queen_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.sort(Trinity::ObjectDistanceOrderPred(GetCaster()));
            if (targets.size() > 1)
                targets.resize(1);
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_woe_servant_of_the_queen_SpellScript::FilterTargets, EFFECT_ALL, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    class spell_woe_servant_of_the_queen_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_woe_servant_of_the_queen_AuraScript);

        void AfterApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* hand = GetCaster())
            {
                hand->CastSpell(GetTarget(), SPELL_RIDE_VEHICLE, true);
                hand->CastWithDelay(500, GetTarget(), SPELL_PUPPET_STRING, true);
                hand->CastWithDelay(500, GetTarget(), SPELL_PUPPET_STRING_2, true);
                hand->CastWithDelay(500, GetTarget(), SPELL_PUPPET_STRING_3, true);
                hand->CastWithDelay(500, GetTarget(), SPELL_PUPPET_STRING_HOVER, true);
            }
        }

        void Register() override
        {
            AfterEffectApply += AuraEffectApplyFn(spell_woe_servant_of_the_queen_AuraScript::AfterApply, EFFECT_0, SPELL_AURA_SET_VEHICLE_ID, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_woe_servant_of_the_queen_AuraScript();
    }

    SpellScript* GetSpellScript() const override
    {
        return new spell_woe_servant_of_the_queen_SpellScript();
    }
};

class PlayerDistanceChecker : public BasicEvent
{
public:
    PlayerDistanceChecker(Unit* owner, bool initialMovement) : _owner(owner), _initialMovement(initialMovement) {}

    bool Execute(uint64 execTime, uint32 /*diff*/) override
    {
        if (_initialMovement)
        {
            _owner->GetMotionMaster()->MovePoint(0, deathPos);
            _initialMovement = false;
        }
        else if (_owner->GetDistance2d(deathPos.GetPositionX(), deathPos.GetPositionY()) <= 2.00f)
        {
            _owner->Kill(_owner);
            return true;
        }

        _owner->m_Events.AddEvent(this, execTime + 500);
        return false;
    }

private:
    Unit* _owner;
    bool _initialMovement;
};

class spell_woe_total_obedience : public SpellScriptLoader
{
public:
    spell_woe_total_obedience() : SpellScriptLoader("spell_woe_total_obedience") { }

    class spell_woe_total_obedience_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_woe_total_obedience_AuraScript);

        void AfterApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            Position pos(*GetTarget());
            if (Unit* hand = GetTarget()->SummonCreature(NPC_HAND_OF_THE_QUEEN, pos))
            {
                hand->AddUnitTypeMask(UNIT_MASK_ACCESSORY);
                hand->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                hand->CastSpell(GetTarget(), SPELL_RIDE_VEHICLE, true);
                hand->CastWithDelay(500, GetTarget(), SPELL_PUPPET_STRING, true);
                hand->CastWithDelay(500, GetTarget(), SPELL_PUPPET_STRING_2, true);
                hand->CastWithDelay(500, GetTarget(), SPELL_PUPPET_STRING_3, true);
                hand->CastWithDelay(500, GetTarget(), SPELL_PUPPET_STRING_HOVER, true);
                GetTarget()->m_Events.AddEvent(new PlayerDistanceChecker(GetTarget(), true), GetTarget()->m_Events.CalculateTime(600));
            }
        }

        void Register() override
        {
            AfterEffectApply += AuraEffectApplyFn(spell_woe_total_obedience_AuraScript::AfterApply, EFFECT_1, SPELL_AURA_SET_VEHICLE_ID, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_woe_total_obedience_AuraScript();
    }
};

class spell_woe_mage_wave_selector : public SpellScriptLoader
{
public:
    spell_woe_mage_wave_selector() : SpellScriptLoader("spell_woe_mage_wave_selector") { }

    class spell_woe_mage_wave_selector_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_woe_mage_wave_selector_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            switch (GetSpellInfo()->Id)
            {
                case SPELL_ACTIVATE_WAVE_ONE:
                    targets.remove_if([](WorldObject* target){
                        return !target->ToUnit()->HasAura(SPELL_WAVE_1);
                    });
                    break;
                case SPELL_ACTIVATE_WAVE_TWO:
                    targets.remove_if([](WorldObject* target){
                        return !target->ToUnit()->HasAura(SPELL_WAVE_2);
                    });
                    break;
                case SPELL_ACTIVATE_WAVE_THREE:
                    targets.remove_if([](WorldObject* target){
                        return !target->ToUnit()->HasAura(SPELL_WAVE_3);
                    });
                    break;
                default:
                    break;
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_woe_mage_wave_selector_SpellScript::FilterTargets, EFFECT_ALL, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_woe_mage_wave_selector_SpellScript();
    }
};

class spell_woe_ice_fling : public SpellScriptLoader
{
public:
    spell_woe_ice_fling() : SpellScriptLoader("spell_woe_ice_fling") { }

    class spell_woe_ice_fling_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_woe_ice_fling_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (!targets.empty())
            {
                WorldObject* target = Trinity::Containers::SelectRandomContainerElement(targets);

                targets.remove_if([](WorldObject* target){
                    return target->GetTypeId() == TYPEID_PLAYER && target->ToPlayer()->HasTankSpec();
                });

                if (!targets.empty())
                   target = Trinity::Containers::SelectRandomContainerElement(targets);

                targets.clear();
                targets.push_back(target);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_woe_ice_fling_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_woe_ice_fling_SpellScript();
    }
};

class spell_woe_hammer_of_reckoning : public SpellScriptLoader
{
public:
    spell_woe_hammer_of_reckoning() : SpellScriptLoader("spell_woe_hammer_of_reckoning") { }

    class spell_woe_hammer_of_reckoning_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_woe_hammer_of_reckoning_SpellScript);

        void ChangeSummonPos(SpellEffIndex /*effIndex*/)
        {
            WorldLocation summonPos = *GetExplTargetDest();
            Position offset = { 0.0f, 0.0f, 14.0f, 0.0f };
            summonPos.RelocateOffset(offset);
            SetExplTargetDest(summonPos);
            GetHitDest()->RelocateOffset(offset);
        }

        void ModDestHeight(SpellEffIndex /*effIndex*/)
        {
            Position offset = { 0.0f, 0.0f, 14.0f, 0.0f };
            const_cast<WorldLocation*>(GetExplTargetDest())->RelocateOffset(offset);
        }

        void Register()
        {
            OnEffectHit += SpellEffectFn(spell_woe_hammer_of_reckoning_SpellScript::ChangeSummonPos, EFFECT_0, SPELL_EFFECT_SUMMON);
            OnEffectLaunch += SpellEffectFn(spell_woe_hammer_of_reckoning_SpellScript::ModDestHeight, EFFECT_0, SPELL_EFFECT_SUMMON);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_woe_hammer_of_reckoning_SpellScript();
    }
};

void AddSC_boss_queen_azshara()
{
    new boss_woe_queen_azshara();
    new npc_woe_fire_mage();
    new npc_woe_arcane_mage();
    new npc_woe_frost_mage();
    new spell_woe_servant_of_the_queen();
    new spell_woe_total_obedience();
    new spell_woe_mage_wave_selector();
    new spell_woe_ice_fling();
    new spell_woe_hammer_of_reckoning();
}