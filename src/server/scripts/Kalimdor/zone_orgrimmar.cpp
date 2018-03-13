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

/* ScriptData
SDName: Orgrimmar
SD%Complete: 0
SDComment: Quest support:
SDCategory: Orgrimmar
EndScriptData */

/* ContentData
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"

#include "Vehicle.h"

// 52514 
class item_thonks_spyglass_52514 : public ItemScript
{
public:
	item_thonks_spyglass_52514() : ItemScript("item_thonks_spyglass_52514") { }

	bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
	{
		if (player->GetQuestStatus(25187) == QUEST_STATUS_INCOMPLETE)
		{
			if (!player->GetReqKillOrCastCurrentCount(25187, 39357))
				player->CastSpell(player, 73741, TRIGGERED_NONE);
			else if (!player->GetReqKillOrCastCurrentCount(25187, 39358))
				player->CastSpell(player, 73763, TRIGGERED_NONE);
			else if (!player->GetReqKillOrCastCurrentCount(25187, 39359))
				player->CastSpell(player, 73764, TRIGGERED_NONE);
			else if (!player->GetReqKillOrCastCurrentCount(25187, 39360))
				player->CastSpell(player, 73765, TRIGGERED_NONE);
		}

		return false;
	}
};

// 39320 Raggan
class npc_durotar_watershed_telescope_39320 : public CreatureScript
{
public:
	npc_durotar_watershed_telescope_39320() : CreatureScript("npc_durotar_watershed_telescope_39320") { }

	enum eQuest
	{
		EVENT_TIMEOUT = 1,
		EVENT_EMOTE = 2,
		EVENT_FINALE = 3,
	};

	struct npc_durotar_watershed_telescope_39320AI : public ScriptedAI
	{
		npc_durotar_watershed_telescope_39320AI(Creature* creature) : ScriptedAI(creature) { }

		uint64 m_playerGUID;
		uint64 m_homeGUID;
		EventMap m_events;
		Position pos;

		void InitializeAI() override
		{
			m_playerGUID = me->GetCharmerOrOwnerOrOwnGUID();
			m_events.ScheduleEvent(EVENT_TIMEOUT, 60000);
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
			if (Player* player = ObjectAccessor::GetPlayer(*me, m_playerGUID))
				if (GameObject* gobject = player->FindNearestGameObject(301050, 10.0f))
				{
					Position playerPos;
					player->GetPosition(&playerPos);
				}
		}

		void PassengerBoarded(Unit* passenger, int8 seatId, bool apply) override
		{
			me->SetSpeed(MOVE_RUN, 12.0f);
			me->GetMotionMaster()->MovePath(3932001, false);
		}

		void MovementInform(uint32 type, uint32 pointId) override
		{
			if (type == WAYPOINT_MOTION_TYPE)
				if (pointId == 2)
					m_events.ScheduleEvent(EVENT_EMOTE, 4000);
				else if (pointId == 3)
					m_events.ScheduleEvent(EVENT_FINALE, 1000);
		}

		void UpdateAI(const uint32 diff) override
		{
			m_events.Update(diff);

			while (uint32 eventId = m_events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_TIMEOUT:
				{
					me->DespawnOrUnsummon(100);
					break;
				}
				case EVENT_EMOTE:
				{
					if (Creature* npc = me->FindNearestCreature(39326, 30.0f))
						npc->HandleEmote(EMOTE_ONESHOT_ROAR);
					break;
				}
				case EVENT_FINALE:
				{
					if (m_playerGUID)
						if (Player* player = ObjectAccessor::GetPlayer(*me, m_playerGUID))
						{
							if (Player* player = me->GetPlayer(*me, m_playerGUID))
								player->KilledMonsterCredit(39357, m_playerGUID);
							//player->Dismount();
							//player->RemoveAurasDueToSpell(73741);
							//player->ExitVehicle();
							player->RemoveAurasByType(SPELL_AURA_MOUNTED);
							player->ExitVehicle();
							player->TeleportTo(1, 391.126221f, -4582.271484f, 76.625534f, 1.183319f);
							me->DespawnOrUnsummon(100);
						}
					break;
				}
				}
			}
		}
	};

	CreatureAI* GetAI(Creature* creature) const override
	{
		return new npc_durotar_watershed_telescope_39320AI(creature);
	}
};

// 39345 Tekla
class npc_durotar_watershed_telescope_39345 : public CreatureScript
{
public:
	npc_durotar_watershed_telescope_39345() : CreatureScript("npc_durotar_watershed_telescope_39345") { }

	enum eQuest
	{
		EVENT_TIMEOUT = 1,
		EVENT_FINALE = 2,
	};

	struct npc_durotar_watershed_telescope_39345AI : public ScriptedAI
	{
		npc_durotar_watershed_telescope_39345AI(Creature* creature) : ScriptedAI(creature) { }

		uint64 m_playerGUID;
		uint64 m_homeGUID;
		EventMap m_events;
		Position pos;

		void InitializeAI() override
		{
			m_playerGUID = me->GetCharmerOrOwnerOrOwnGUID();
			m_events.ScheduleEvent(EVENT_TIMEOUT, 60000);
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
			if (Player* player = ObjectAccessor::GetPlayer(*me, m_playerGUID))
				if (GameObject* gobject = player->FindNearestGameObject(301050, 10.0f))
				{
					Position playerPos;
					player->GetPosition(&playerPos);
					m_homeGUID = gobject->GetGUID();
				}
		}

		void PassengerBoarded(Unit* passenger, int8 seatId, bool apply) override
		{
			me->SetSpeed(MOVE_RUN, 12.0f);
			me->GetMotionMaster()->MovePath(3934501, false);
		}

		void MovementInform(uint32 type, uint32 pointId) override
		{
			if (type == WAYPOINT_MOTION_TYPE && pointId == 3)
				m_events.ScheduleEvent(EVENT_FINALE, 1000);
		}

		void UpdateAI(const uint32 diff) override
		{
			m_events.Update(diff);

			while (uint32 eventId = m_events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_TIMEOUT:
				{
					me->DespawnOrUnsummon(100);
					break;
				}
				case EVENT_FINALE:
				{
					if (m_playerGUID)
						if (Player* player = ObjectAccessor::GetPlayer(*me, m_playerGUID))
						{
							if (Player* player = me->GetPlayer(*me, m_playerGUID))
								player->KilledMonsterCredit(39358);
							player->ExitVehicle();
							player->TeleportTo(1, 391.126221f, -4582.271484f, 76.625534f, 1.183319f);
							me->DespawnOrUnsummon(100);
						}
					break;
				}
				}
			}
		}
	};

	CreatureAI* GetAI(Creature* creature) const override
	{
		return new npc_durotar_watershed_telescope_39345AI(creature);
	}
};

// 39346 Misha
class npc_durotar_watershed_telescope_39346 : public CreatureScript
{
public:
	npc_durotar_watershed_telescope_39346() : CreatureScript("npc_durotar_watershed_telescope_39346") { }

	enum eQuest
	{
		EVENT_TIMEOUT = 1,
		EVENT_FINALE = 2,
	};

	struct npc_durotar_watershed_telescope_39346AI : public ScriptedAI
	{
		npc_durotar_watershed_telescope_39346AI(Creature* creature) : ScriptedAI(creature) { }

		uint64 m_playerGUID;
		uint64 m_homeGUID;
		EventMap m_events;
		Position pos;

		void InitializeAI() override
		{
			m_playerGUID = me->GetCharmerOrOwnerOrOwnGUID();
			m_events.ScheduleEvent(EVENT_TIMEOUT, 60000);
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
			if (Player* player = ObjectAccessor::GetPlayer(*me, m_playerGUID))
				if (GameObject* gobject = player->FindNearestGameObject(301050, 10.0f))
				{
					Position playerPos;
					player->GetPosition(&playerPos);
				}
		}

		void PassengerBoarded(Unit* passenger, int8 seatId, bool apply) override
		{
			me->SetSpeed(MOVE_RUN, 12.0f);
			me->GetMotionMaster()->MovePath(3934601, false);
		}

		void MovementInform(uint32 type, uint32 pointId) override
		{
			if (type == WAYPOINT_MOTION_TYPE && pointId == 3)
				m_events.ScheduleEvent(EVENT_FINALE, 1000);
		}

		void UpdateAI(const uint32 diff) override
		{
			m_events.Update(diff);

			while (uint32 eventId = m_events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_TIMEOUT:
				{
					me->DespawnOrUnsummon(100);
					break;
				}
				case EVENT_FINALE:
				{
					if (m_playerGUID)
						if (Player* player = ObjectAccessor::GetPlayer(*me, m_playerGUID))
						{
							if (Player* player = me->GetPlayer(*me, m_playerGUID))
								player->KilledMonsterCredit(39359);
							player->ExitVehicle();
							player->TeleportTo(1, 391.126221f, -4582.271484f, 76.625534f, 1.183319f);
							me->DespawnOrUnsummon(100);
						}
					break;
				}
				}
			}
		}
	};

	CreatureAI* GetAI(Creature* creature) const override
	{
		return new npc_durotar_watershed_telescope_39346AI(creature);
	}
};

// 39347 Zen'Taji
class npc_durotar_watershed_telescope_39347 : public CreatureScript
{
public:
	npc_durotar_watershed_telescope_39347() : CreatureScript("npc_durotar_watershed_telescope_39347") { }

	enum eQuest
	{
		EVENT_TIMEOUT = 1,
		EVENT_FINALE = 2,
	};

	struct npc_durotar_watershed_telescope_39347AI : public ScriptedAI
	{
		npc_durotar_watershed_telescope_39347AI(Creature* creature) : ScriptedAI(creature) { }

		uint64 m_playerGUID;
		uint64 m_homeGUID;
		EventMap m_events;
		Position pos;

		void InitializeAI() override
		{
			m_playerGUID = me->GetCharmerOrOwnerOrOwnGUID();
			m_events.ScheduleEvent(EVENT_TIMEOUT, 60000);
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
			if (Player* player = ObjectAccessor::GetPlayer(*me, m_playerGUID))
				if (GameObject* gobject = player->FindNearestGameObject(301050, 10.0f))
				{
					Position playerPos;
					player->GetPosition(&playerPos);
				}
		}

		void PassengerBoarded(Unit* passenger, int8 seatId, bool apply) override
		{
			me->SetSpeed(MOVE_RUN, 12.0f);
			me->GetMotionMaster()->MovePath(3934701, false);
		}

		void MovementInform(uint32 type, uint32 pointId) override
		{
			if (type == WAYPOINT_MOTION_TYPE && pointId == 3)
				m_events.ScheduleEvent(EVENT_FINALE, 1000);
		}

		void UpdateAI(const uint32 diff) override
		{
			m_events.Update(diff);

			while (uint32 eventId = m_events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_TIMEOUT:
				{
					me->DespawnOrUnsummon(100);
					break;
				}
				case EVENT_FINALE:
				{
					if (m_playerGUID)
						if (Player* player = ObjectAccessor::GetPlayer(*me, m_playerGUID))
						{
							if (Player* player = me->GetPlayer(*me, m_playerGUID))
								player->KilledMonsterCredit(39360);
							player->ExitVehicle();
							player->TeleportTo(1, 391.126221f, -4582.271484f, 76.625534f, 1.183319f);
							me->DespawnOrUnsummon(100);
						}
					break;
				}
				}
			}
		}
	};

	CreatureAI* GetAI(Creature* creature) const override
	{
		return new npc_durotar_watershed_telescope_39347AI(creature);
	}
};

void AddSC_orgrimmar()
{
	new item_thonks_spyglass_52514();
	new npc_durotar_watershed_telescope_39320();
	new npc_durotar_watershed_telescope_39345();
	new npc_durotar_watershed_telescope_39346();
	new npc_durotar_watershed_telescope_39347();
}
