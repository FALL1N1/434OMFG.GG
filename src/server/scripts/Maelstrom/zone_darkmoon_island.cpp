/*
* Copyright (C) 2011 - 2013 ArkCORE <http://www.arkania.net/>
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
#include "ScriptedEscortAI.h"
#include "SpellMgr.h"
#include "Player.h"
#include "Creature.h"

enum Zone_darkmoon_island
{
	PLAYER_GUID = 99999,
	QUEST_HE_SHOOTS_HE_SCORES = 29438,
};

#define GOSSIP_ITEM_ARTHAS_0 "How does the Shooting Gallery work?"
#define GOSSIP_ITEM_ARTHAS_1 "Let's shoot! (Darkmoon Game Token)"

// 14841
class npc_rinling_14841 : public CreatureScript
{
public:
	npc_rinling_14841() : CreatureScript("npc_rinling_14841") { }

	enum eNpc
	{
		RINLING_GOSSIP_MENU_ID = 6225,
		RINLING_GOSSIP_TEXT_ID = 7398,
		RINLING_GOSSIP_MENU_ID2 = 6574,
		RINLING_GOSSIP_TEXT_ID2 = 18353,
		EVENT_TALK = 101,
		EVENT_TALK_DC,
	};

	bool OnGossipHello(Player* player, Creature* creature) override
	{
		player->PrepareQuestMenu(creature->GetGUID());
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ARTHAS_0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ARTHAS_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
		player->SEND_GOSSIP_MENU(RINLING_GOSSIP_TEXT_ID, creature->GetGUID());
		return true;
	}

	bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
	{
		player->PlayerTalkClass->ClearMenus();
		switch (action)
		{
		case GOSSIP_ACTION_INFO_DEF + 1:
		{
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ARTHAS_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
			player->SEND_GOSSIP_MENU(RINLING_GOSSIP_TEXT_ID2, creature->GetGUID());
			break;
		}
		case GOSSIP_ACTION_INFO_DEF + 3:
		{
			player->PrepareQuestMenu(creature->GetGUID());
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ARTHAS_0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ARTHAS_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
			player->SEND_GOSSIP_MENU(RINLING_GOSSIP_TEXT_ID, creature->GetGUID());
			break;
		}
		case GOSSIP_ACTION_INFO_DEF + 2:
		{
			Quest const* quest = sObjectMgr->GetQuestTemplate(QUEST_HE_SHOOTS_HE_SCORES);
			if (player->CanTakeQuest(quest, false))
			{
				player->AddQuest(quest, creature);
			}
			if (player->GetQuestStatus(QUEST_HE_SHOOTS_HE_SCORES) == QUEST_STATUS_INCOMPLETE)
			{
				// start shoot
			}
			player->CLOSE_GOSSIP_MENU();
			break;
		}
		}

		return true;
	}

	struct npc_rinling_14841AI : public ScriptedAI
	{
		npc_rinling_14841AI(Creature* creature) : ScriptedAI(creature) { }

		EventMap m_events;
		bool     m_talk_dc;

		void Reset() override
		{
			m_events.Reset();
			m_talk_dc = false;
		}

		void MoveInLineOfSight(Unit* who)
		{
			if (!m_talk_dc)
				if (Player* player = who->ToPlayer())
					RandomTalk();
		}

		void UpdateAI(const uint32 diff)
		{
			m_events.Update(diff);

			while (uint32 eventId = m_events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_TALK_DC:
				{
					m_talk_dc = false;
					break;
				}
				}
			}

			if (!UpdateVictim())
				return;
			else
				DoMeleeAttackIfReady();
		}

		void RandomTalk()
		{
			Talk(0);
			m_talk_dc = true;
			m_events.ScheduleEvent(EVENT_TALK_DC, urand(20000, 60000));
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_rinling_14841AI(creature);
	}
};

void AddSC_zone_darkmoon_island()
{
	new npc_rinling_14841();
}