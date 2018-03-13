/*
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2006-2010 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

 /* ScriptData
SDName: Boss Erunak Stonespeaker & Mindbender Ghur'sha
SD%Complete: 90%
SDComment: sometimes Mindbender attacks while somebody is enslaved.
SDCategory: Throne of the Tides
EndScriptData */

#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "throne_of_the_tides.h"

#define SPELL_ABSORB_MAGIC           DUNGEON_MODE(76307,91492)
#define SPELL_LAVA_BOLT              DUNGEON_MODE(76171,91412)
#define SPELL_ENSLAVE                DUNGEON_MODE(76207, 91413)

enum Spells
{
// Erunak Stonespeaker
    SPELL_EARTH_SHARDS           = 84931,
    SPELL_EARTH_SHARD_AURA       = 84935,
    SPELL_EARTH_SHARD_SUMMON     = 84934,
    SPELL_EMBERSTRIKE            = 76165,
    SPELL_MAGMA_SPLASH           = 76170,
// Mindbender Ghur'sha
    SPELL_ENSLAVE_BUFF           = 76213, // Should be in SPELL_LINKED_SPELL with SPELL_ENSLAVE

    SPELL_MIND_FOG_SUMMON        = 76234,
    SPELL_MIND_FOG_AURA          = 76230,
    SPELL_MIND_FOG_VISUAL        = 76231,
    SPELL_UNRELENTING_AGONY      = 76339,
};

enum Yells
{
    SAY_PHASE_1_END_MINDBENDER      = -1643011,
    SAY_PHASE_1_END_ERUNAK          = -1643012,
    SAY_MIND_CONTROL_1              = -1643013,
    SAY_MIND_CONTROL_2              = -1643014,
    SAY_MIND_CONTROL_3              = -1643015,
    SAY_MIND_FOG                    = -1643016,
    SAY_DEATH                       = -1643017,
    SAY_WIN_ERUNAK                  = -1643018,
};

enum ErunakPhase
{
    PHASE_ERUNAK,
    PHASE_PLAYER_CONTROLLED,
    PHASE_GURSHA
};

// predicate function to select not charmed target
struct NotCharmedTargetSelector : public std::unary_function<Unit*, bool>
{
    NotCharmedTargetSelector() {}

    bool operator()(Unit const* target) const
    {
        return target->GetTypeId() == TYPEID_PLAYER && !target->isCharmed();
    }
};

class boss_erunak_stonespeaker : public CreatureScript
{
public:
    boss_erunak_stonespeaker() : CreatureScript("boss_erunak_stonespeaker") {}

    struct boss_erunak_stonespeakerAI : public ScriptedAI
    {
        boss_erunak_stonespeakerAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            if (instance)
            {
                instance->SetData(DATA_ERUNAK_STONESPEAKER, NOT_STARTED);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ENSLAVE_BUFF);
            }
            EarthShardsTimer = 20000;
            EmberstrikeTimer = urand(12000,20000);
            LavaBoltTimer = 6500;
            MagmaSplashTimer = 15000;
            mui_phaseChange = 5000;
            CastTimer = 3000;
            start = false;
            _phase = PHASE_ERUNAK;
            //            me->GetMotionMaster()->MoveTargetedHome();
        }

        void JustReachedHome()
        {
            me->SetStandState(UNIT_STAND_STATE_STAND);
            if (instance)
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ENSLAVE_BUFF);
        }

        bool isEventAction(const uint32 diff)
        {
            if ((_phase == PHASE_ERUNAK && (EarthShardsTimer <= diff ||
                                            EmberstrikeTimer <= diff ||
                                            LavaBoltTimer <= diff ||
                                            MagmaSplashTimer <= diff ||
                                            CastTimer <= diff)) ||
                (_phase == PHASE_PLAYER_CONTROLLED && CastTimer <= diff))
                return true;
            return false;
        }

        void updateEventAction(const uint32 diff)
        {
            if (_phase == PHASE_ERUNAK)
            {
                EarthShardsTimer -= diff;
                EmberstrikeTimer -= diff;
                LavaBoltTimer -= diff;
                MagmaSplashTimer -= diff;
            }
            CastTimer -= diff;
        }

        void EnterCombat(Unit* /*who*/)
        {
            if (instance)
                instance->SetData(DATA_ERUNAK_STONESPEAKER, IN_PROGRESS);
            start = true;
            enslaveTargetGUID = me->GetGUID();
        }

        void EnslaveCheckLife()
        {
            if (Unit *enslaveTarget = Unit::GetUnit(*me, enslaveTargetGUID))
                if (enslaveTarget->HealthBelowPct(50) || (enslaveTarget->GetTypeId() == TYPEID_PLAYER && !enslaveTarget->HasAura(SPELL_ENSLAVE)))
                {
                    enslaveTarget->RemoveAurasDueToSpell(SPELL_ENSLAVE_BUFF);
                    if (enslaveTarget->GetTypeId() != TYPEID_PLAYER && enslaveTarget->GetEntry() == me->GetEntry())
                    {
                        me->CombatStop(true);
                        me->SetReactState(REACT_PASSIVE);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        me->setFaction(35);
                        me->SetStandState(UNIT_STAND_STATE_KNEEL);
                        Talk(0);
                    }
                    if (Creature *mindbender = Unit::GetCreature((*me), instance->GetData64(DATA_MINDBENDER_GHURSHA)))
                    {
                        _phase = PHASE_GURSHA;
                        mindbender->AI()->SetGUID(enslaveTargetGUID, 1);
                        enslaveTarget->RemoveVehicleKit();
                        if (enslaveTarget->GetTypeId() == TYPEID_PLAYER)
                        {
                            WorldPacket data(SMSG_PLAYER_VEHICLE_DATA, enslaveTarget->GetPackGUID().size()+4);
                            data.appendPackGUID(enslaveTarget->GetGUID());
                            data << uint32(0);
                            enslaveTarget->SendMessageToSet(&data, true);
                        }
                        enslaveTargetGUID = 0;
                    }
                }
        }

        void SetGUID(uint64 guid, int32 type = 0)
        {
            enslaveTargetGUID = guid;
            start = true;
            if (guid == 0)
                _phase = PHASE_GURSHA;
            else if (Unit *enslaveTarget = Unit::GetUnit(*me, enslaveTargetGUID))
            {
                if (enslaveTarget->GetTypeId() == TYPEID_PLAYER)
                    _phase = PHASE_PLAYER_CONTROLLED;
                else
                    _phase = PHASE_ERUNAK;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!start)
                return;
            if (_phase == PHASE_ERUNAK && !UpdateVictim())
                Reset();
            if (_phase == PHASE_GURSHA)
            {
                if (mui_phaseChange <= diff)
                {
                    if (Creature *mindbender = Unit::GetCreature((*me), instance->GetData64(DATA_MINDBENDER_GHURSHA)))
                        mindbender->AI()->SetGUID(enslaveTargetGUID, 0);
                    mui_phaseChange = 15000;
                }
                else
                    mui_phaseChange -= diff;
                return ;
            }
            EnslaveCheckLife();
            if (isEventAction(diff))
            {
                if (Unit *unit = Unit::GetUnit(*me, enslaveTargetGUID))
                {
                    switch (_phase)
                    {
                        case PHASE_PLAYER_CONTROLLED:
                        {
                            if (CastTimer <= diff)
                            {
                                CastTimer = 3000;
                                if (Unit *target = SelectTarget(SELECT_TARGET_RANDOM, 0, NotCharmedTargetSelector()))
                                {
                                    switch (unit->getClass())
                                    {
                                        case CLASS_DRUID:
                                            if (urand(0, 1))
                                                unit->CastSpell(target, 8921, false);
                                            else
                                                unit->CastSpell(unit, 774, false);
                                            break;
                                        case CLASS_HUNTER:
                                            unit->CastSpell(target, RAND(2643, 1978), false);
                                            break;
                                        case CLASS_MAGE:
                                            unit->CastSpell(target, RAND(44614, 30455), false);
                                            break;
                                        case CLASS_WARLOCK:
                                            unit->CastSpell(target, RAND(980, 686), true);
                                            break;
                                        case CLASS_WARRIOR:
                                            unit->CastSpell(target, RAND(46924, 845), false);
                                            break;
                                        case CLASS_PALADIN:
                                            if (urand(0, 1))
                                                unit->CastSpell(target, 853, false);
                                            else
                                                unit->CastSpell(unit, 20473, false);
                                            break;
                                        case CLASS_PRIEST:
                                            if (urand(0, 1))
                                                unit->CastSpell(target, 34914, false);
                                            else
                                                unit->CastSpell(unit, 139, false);
                                            break;
                                        case CLASS_SHAMAN:
                                            if (urand(0, 1))
                                                unit->CastSpell(target, 421, false);
                                            else
                                                unit->CastSpell(unit, 61295, false);
                                            break;
                                        case CLASS_ROGUE:
                                            unit->CastSpell(target, RAND(16511, 1329), false);
                                            break;
                                        case CLASS_DEATH_KNIGHT:
                                            if (urand(0, 1))
                                                unit->CastSpell(target, 45462, true);
                                            else
                                                unit->CastSpell(target, 49184, true);
                                            break;
                                        default:
                                            break;
                                    }
                                }
                            }
                            break;
                        }
                        case PHASE_ERUNAK:
                        {
                            if (CastTimer <= diff)
                                CastTimer = 5000;
                            if (EmberstrikeTimer <= diff)
                            {
                                DoCastVictim(SPELL_EMBERSTRIKE);
                                EmberstrikeTimer = 15000;
                            }
                            if (EarthShardsTimer <= diff)
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                                    DoCast(target, SPELL_EARTH_SHARDS);
                                //                                DoCast(me, SPELL_EARTH_SHARD_SUMMON);
                                EarthShardsTimer = 20000;
                            }
                            if (LavaBoltTimer <= diff)
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                                    me->CastSpell(target, SPELL_LAVA_BOLT, true);
                                LavaBoltTimer = 10000;
                            }
                            if (MagmaSplashTimer <= diff)
                            {
                                DoCast(me, SPELL_MAGMA_SPLASH);
                                MagmaSplashTimer = 15000;
                            }
                            break;
                        }
                        default :
                            break;
                    }
                }
            }
            else
                updateEventAction(diff);

            DoMeleeAttackIfReady();
        }
    private :
        InstanceScript* instance;
        uint32 EarthShardsTimer;
        uint32 EmberstrikeTimer;
        uint32 LavaBoltTimer;
        uint32 MagmaSplashTimer;
        uint32 mui_enslavedAction;
        bool start;
        uint32 mui_phaseChange;
        uint32 CastTimer;
        uint64 enslaveTargetGUID;
        uint8 _phase;
    };

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_erunak_stonespeakerAI (creature);
    }
};

class boss_mindbender_ghursha : public CreatureScript
{
public:
    boss_mindbender_ghursha() : CreatureScript("boss_mindbender_ghursha") { }

    struct boss_mindbender_ghurshaAI : public ScriptedAI
    {
        boss_mindbender_ghurshaAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            summons.DespawnAll();
            if (Creature *erunak = Unit::GetCreature((*me), instance->GetData64(DATA_ERUNAK_STONESPEAKER)))
                if (!erunak->isAlive())
                    me->DespawnOrUnsummon();
        }

        void JustDied(Unit* /*killer*/)
        {
            if (instance)
            {
                instance->SetData(DATA_ERUNAK_STONESPEAKER, DONE);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ENSLAVE_BUFF);

                if (Creature *erunak = Unit::GetCreature((*me), instance->GetData64(DATA_ERUNAK_STONESPEAKER)))
                {
                    erunak->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    erunak->AI()->Talk(1);
                    erunak->DespawnOrUnsummon(60000);
                }
            }
            summons.DespawnAll();
        }

        void JustReachedHome()
        {
            if (Creature *erunak = Unit::GetCreature((*me), instance->GetData64(DATA_ERUNAK_STONESPEAKER)))
            {
                me->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, erunak, false);
                erunak->setFaction(14);
                erunak->CombatStop(false);
                erunak->SetReactState(REACT_AGGRESSIVE);
                erunak->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                erunak->AI()->EnterEvadeMode();
            }
        }

        void SetGUID(uint64 guid, int32 type)
        {
            if (type == 0)
            {
                if (Unit *unit = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true)) // tank or not ?
                {
                    enslaveTargetGUID = unit->GetGUID();
                    EnslaveTarget(unit, true);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                }
            }
            else
            {
                EnslaveTarget(Unit::GetCreature(*me, guid), false);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }
        }

        void EnslaveTarget(Unit *target, bool apply)
        {
            if (apply)
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            else
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            if (!target)
                return;
            if (Creature *erunak = Unit::GetCreature((*me), instance->GetData64(DATA_ERUNAK_STONESPEAKER)))
            {
                uint32 vehicleId = 786; // from sniff
                if (apply)
                {
                    Talk(0);
                    if (target->GetVehicleKit() == NULL)
                    {
                        if (target->CreateVehicleKit(vehicleId, me->GetEntry()) && target->GetTypeId() == TYPEID_PLAYER)
                        {
                            WorldPacket data(SMSG_PLAYER_VEHICLE_DATA, target->GetPackGUID().size()+4);
                            data.appendPackGUID(target->GetGUID());
                            data << uint32(apply ? vehicleId : 0);
                            target->SendMessageToSet(&data, true);
                            target->ToPlayer()->SendOnCancelExpectedVehicleRideAura();
                        }
                    }
                    me->CastSpell(target, SPELL_ENSLAVE, true); // player only ?
                    //                    target->CastSpell(target, SPELL_ENSLAVE_BUFF, true); spell linked spell !!!
                    erunak->AI()->SetGUID(target->GetGUID());
                }
            }
        }

        void EnterCombat(Unit* /*who*/)
        {
            AbsorbMagicTimer = 20000;
            MindFogTimer = urand(6000, 12000);
            UnrelentingAgonyTimer = 10000;
        }


        void JustSummoned(Creature* summon)
        {
            summon->CastSpell(summon,SPELL_MIND_FOG_AURA, false);
            summon->CastSpell(summon,SPELL_MIND_FOG_VISUAL, false);
            summon->SetReactState(REACT_PASSIVE);
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE);
            summons.Summon(summon);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim() || me->GetVehicle() != NULL)
                return;
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (AbsorbMagicTimer <= diff)
            {
                DoCast(me, SPELL_ABSORB_MAGIC);
                AbsorbMagicTimer = urand(15000, 20000);
            }
            else
                AbsorbMagicTimer -= diff;

            if (MindFogTimer <= diff)
            {
                DoCast(me, SPELL_MIND_FOG_SUMMON);
                Talk(1);
                MindFogTimer = 18000;
            }
            else
                MindFogTimer -= diff;

            if (UnrelentingAgonyTimer <= diff)
            {
                Talk(2);
                DoCastAOE(SPELL_UNRELENTING_AGONY);
                UnrelentingAgonyTimer = 20000;
            }
            else
                UnrelentingAgonyTimer -= diff;

            DoMeleeAttackIfReady();
        }


    private :
        InstanceScript *instance;
        SummonList summons;
        uint32 AbsorbMagicTimer;
        uint32 MindFogTimer;
        uint32 UnrelentingAgonyTimer;
        uint64 enslaveTargetGUID;
    };

    CreatureAI* GetAI(Creature* creature)const
    {
        return new boss_mindbender_ghurshaAI(creature);
    }
};

class npc_erunak_pic : public CreatureScript
{
public:
    npc_erunak_pic() : CreatureScript("npc_erunak_pic") { }

    struct npc_erunak_picAI : public ScriptedAI
    {
        npc_erunak_picAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }
    private :
        InstanceScript* instance;
        uint32 mui_despawn;

        void Reset()
        {
            DoCast(me, SPELL_EARTH_SHARD_AURA);
            mui_despawn = 12000;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE|UNIT_FLAG_DISABLE_MOVE);
        }

        void UpdateAI(const uint32 diff)
        {
            if (mui_despawn <= diff)
            {
                me->DespawnOrUnsummon();
                mui_despawn = 1000;
            }
            else mui_despawn -= diff;
        }

    };

    CreatureAI* GetAI(Creature *creature) const
    {
        return new npc_erunak_picAI (creature);
    }
};

void AddSC_boss_erunak_stonespeaker()
{
    new boss_erunak_stonespeaker();
    new boss_mindbender_ghursha();
    new npc_erunak_pic();
}
