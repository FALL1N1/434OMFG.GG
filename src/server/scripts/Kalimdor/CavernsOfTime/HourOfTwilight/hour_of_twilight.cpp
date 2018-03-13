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
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"
#include "hour_of_twilight.h"

enum Spells
{
    SPELL_TELEPORT_ARCURION_DEAD = 108928,
    SPELL_TELEPORT_ASIRA_DEAD = 108929,
    SPELL_TELEPORT_BENEDICTUS = 108930,

};

enum Npcs
{
    NPC_THRALL_FIRE_TOTEM = 55474,
};

/// 210026
class go_hot_time_transit_device : public GameObjectScript
{
public:
    go_hot_time_transit_device() : GameObjectScript("go_hot_time_transit_device") {}

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (player->isInCombat())
            return true;
        if (InstanceScript *instance = go->GetInstanceScript())
        {
            if (instance->GetData(DATA_EVENT_2) == DONE)
                player->CastSpell(player, SPELL_TELEPORT_BENEDICTUS, true);
            else if (instance->IsDone(DATA_ASIRA))
                player->CastSpell(player, SPELL_TELEPORT_ASIRA_DEAD, true);
            else if (instance->IsDone(DATA_ARCURION))
                player->CastSpell(player, SPELL_TELEPORT_ARCURION_DEAD, true);
        }
        return true;
    }

};


void AddSC_hour_of_twilight()
{
    new go_hot_time_transit_device();
}
