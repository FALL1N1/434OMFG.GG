/*
* Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
*
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
#include "InstanceScript.h"
#include "dragonsoul.h"

class npc_eiendormi : public CreatureScript
{
public:
    npc_eiendormi() : CreatureScript("npc_eiendormi") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (InstanceScript* instance = creature->GetInstanceScript())
        {
            creature->CastSpell(creature, SPELL_WARDEN_RING_YELLOW);

            if (instance->GetBossState(DATA_MORCHOK) == DONE)
            {
                // player->TeleportTo(967, -1870.114f, -3077.727f, -176.308f, 0.390f);
                if (Creature *t = player->SummonCreature(572880, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(),
					player->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 300 * IN_MILLISECONDS))
					player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, t, false);
        }
            }

        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }
};

class npc_valeera_tele : public CreatureScript
{
public:
    npc_valeera_tele() : CreatureScript("npc_valeera_tele") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (InstanceScript* instance = creature->GetInstanceScript())
        {
            creature->CastSpell(creature, SPELL_WARDEN_RING_GREEN);

            if (instance->GetBossState(DATA_MORCHOK) == DONE)

            {
                if (Creature *t = player->SummonCreature(572890, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(),
                    player->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 300 * IN_MILLISECONDS))
                    player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, t, false);
            }
        }

        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }
};

class npc_nethestrasz : public CreatureScript
{
public:
    npc_nethestrasz() : CreatureScript("npc_nethestrasz") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (InstanceScript* instance = creature->GetInstanceScript())
        {
            creature->CastSpell(creature, SPELL_WARDEN_RING_ORANGE);

            if (instance->GetBossState(DATA_YORSAHJ) == DONE && instance->GetBossState(DATA_ZONOZZ) == DONE)
            {
                if (Creature *t = player->SummonCreature(572870, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(),
                    player->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 300 * IN_MILLISECONDS))
                    player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, t, false);
            }
        }

        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }
};

class travel_to_wyrmrest_base : public CreatureScript
{
public:
    travel_to_wyrmrest_base() : CreatureScript("travel_to_wyrmrest_base") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (InstanceScript* instance = creature->GetInstanceScript())
        {
            if (!instance->IsEncounterInProgress())
            {
                player->TeleportTo(967, -1793.22f, -2391.78f, 45.604f, 5.871f);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            }
        }

        player->CLOSE_GOSSIP_MENU();
        return true;
    }
};

class travel_to_wyrmrest_summit : public CreatureScript
{
public:
    travel_to_wyrmrest_summit() : CreatureScript("travel_to_wyrmrest_summit") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (InstanceScript* instance = creature->GetInstanceScript())
        {
            if (!instance->IsEncounterInProgress())
            {
                player->TeleportTo(967, -1786.92f, -2393.18f, 341.355f, 6.141f);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            }
        }

        player->CLOSE_GOSSIP_MENU();
        return true;
    }
};

class travel_to_the_eye_of_eternity : public CreatureScript
{
public:
    travel_to_the_eye_of_eternity() : CreatureScript("travel_to_the_eye_of_eternity") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (InstanceScript* instance = creature->GetInstanceScript())
        {
            if (instance->GetBossState(DATA_YORSAHJ) == DONE && instance->GetBossState(DATA_ZONOZZ) == DONE)
            {
                player->TeleportTo(967, 13629.7f, 13611.9f, 123.482f, 3.468f);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            }
        }

        player->CLOSE_GOSSIP_MENU();
        return true;
    }
};

class travel_to_the_deck_of_the_skyfire : public CreatureScript
{
public:
    travel_to_the_deck_of_the_skyfire() : CreatureScript("travel_to_the_deck_of_the_skyfire") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (InstanceScript* instance = creature->GetInstanceScript())
        {
            if (instance->GetBossState(DATA_ULTRAXION) == DONE)
            {
                player->TeleportTo(967, 13397.298f, -12131.405f, 153.8732f, 3.152f);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            }
        }

        player->CLOSE_GOSSIP_MENU();
        return true;
    }
};

void AddSC_dragon_soul_teleport_and_transport()
{
    new npc_eiendormi();
    new npc_valeera_tele();
    new npc_nethestrasz();
    new travel_to_wyrmrest_base;
    new travel_to_wyrmrest_summit;
    new travel_to_the_eye_of_eternity;
    new travel_to_the_deck_of_the_skyfire;
}