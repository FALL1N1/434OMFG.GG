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
#include "ScriptedEscortAI.h"
#include "ScriptedGossip.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"

#define QUEST_SAVE_WYVERNATE 28086

enum quest_spells
{
    SPELL_FREE_WYVERN =  88532 ,
};

class npc_free_wyverns : public CreatureScript
{
public:
    npc_free_wyverns() : CreatureScript("npc_free_wyverns") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        player->PlayerTalkClass->ClearMenus();
        if (player->GetQuestStatus(QUEST_SAVE_WYVERNATE) == QUEST_STATUS_INCOMPLETE)
        {
            player->CLOSE_GOSSIP_MENU();
            player->CastSpell(player, SPELL_FREE_WYVERN, true);
            player->KilledMonsterCredit(creature->GetEntry(), creature->GetGUID());
            creature->RemoveAllAuras();
            creature->DespawnOrUnsummon(100);
        }
        return true;
    }

};


void AddSC_thousand_needles()
{
    new npc_free_wyverns();
}
