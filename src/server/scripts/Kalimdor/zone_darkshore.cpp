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


#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"
#include "ScriptedFollowerAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"
#include "Vehicle.h"
#include "WorldSession.h"
#include "Object.h"

// To Do add creature text

/*####
# npc_kerlonian
####*/

enum eKerlonian
{
	SAY_KER_START = -1000434,

	EMOTE_KER_SLEEP_1 = -1000435,
	EMOTE_KER_SLEEP_2 = -1000436,
	EMOTE_KER_SLEEP_3 = -1000437,

	SAY_KER_SLEEP_1 = -1000438,
	SAY_KER_SLEEP_2 = -1000439,
	SAY_KER_SLEEP_3 = -1000440,
	SAY_KER_SLEEP_4 = -1000441,

	EMOTE_KER_AWAKEN = -1000445,

	SAY_KER_ALERT_1 = -1000442,
	SAY_KER_ALERT_2 = -1000443,

	SAY_KER_END = -1000444,

	SPELL_SLEEP_VISUAL = 25148,
	SPELL_AWAKEN = 17536,
	QUEST_SLEEPER_AWAKENED = 5321,
	NPC_LILADRIS = 11219,                    //attackers entries unknown
	FACTION_KER_ESCORTEE = 113
};

//TODO: make concept similar as "ringo" -escort. Find a way to run the scripted attacks, _if_ player are choosing road.
class npc_kerlonian : public CreatureScript
{
public:
	npc_kerlonian() : CreatureScript("npc_kerlonian") { }

	bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
	{
		if (quest->GetQuestId() == QUEST_SLEEPER_AWAKENED)
		{
			if (npc_kerlonianAI* pKerlonianAI = CAST_AI(npc_kerlonian::npc_kerlonianAI, creature->AI()))
			{
				creature->SetStandState(UNIT_STAND_STATE_STAND);
				//                DoScriptText(SAY_KER_START, creature, player);
				pKerlonianAI->StartFollow(player, FACTION_KER_ESCORTEE, quest);
			}
		}

		return true;
	}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_kerlonianAI(creature);
	}

	struct npc_kerlonianAI : public FollowerAI
	{
		npc_kerlonianAI(Creature* creature) : FollowerAI(creature) { }

		uint32 m_uiFallAsleepTimer;

		void Reset()
		{
			m_uiFallAsleepTimer = urand(10000, 45000);
		}

		void MoveInLineOfSight(Unit* who)
		{
			FollowerAI::MoveInLineOfSight(who);

			if (!me->getVictim() && !HasFollowState(STATE_FOLLOW_COMPLETE) && who->GetEntry() == NPC_LILADRIS)
			{
				if (me->IsWithinDistInMap(who, INTERACTION_DISTANCE * 5))
				{
					if (Player* player = GetLeaderForFollower())
					{
						if (player->GetQuestStatus(QUEST_SLEEPER_AWAKENED) == QUEST_STATUS_INCOMPLETE)
							player->GroupEventHappens(QUEST_SLEEPER_AWAKENED, me);

						//                        Talk(SAY_KER_END);
					}

					SetFollowComplete();
				}
			}
		}

		void SpellHit(Unit* /*pCaster*/, const SpellInfo* spell)
		{
			if (HasFollowState(STATE_FOLLOW_INPROGRESS | STATE_FOLLOW_PAUSED) && spell->Id == SPELL_AWAKEN)
				ClearSleeping();
		}

		void SetSleeping()
		{
			SetFollowPaused(true);

			//            DoScriptText(RAND(EMOTE_KER_SLEEP_1, EMOTE_KER_SLEEP_2, EMOTE_KER_SLEEP_3), me);

			//            DoScriptText(RAND(SAY_KER_SLEEP_1, SAY_KER_SLEEP_2, SAY_KER_SLEEP_3, SAY_KER_SLEEP_4), me);

			me->SetStandState(UNIT_STAND_STATE_SLEEP);
			DoCast(me, SPELL_SLEEP_VISUAL, false);
		}

		void ClearSleeping()
		{
			me->RemoveAurasDueToSpell(SPELL_SLEEP_VISUAL);
			me->SetStandState(UNIT_STAND_STATE_STAND);

			//            DoScriptText(EMOTE_KER_AWAKEN, me);

			SetFollowPaused(false);
		}

		void UpdateFollowerAI(uint32 const uiDiff)
		{
			if (!UpdateVictim())
			{
				if (!HasFollowState(STATE_FOLLOW_INPROGRESS))
					return;

				if (!HasFollowState(STATE_FOLLOW_PAUSED))
				{
					if (m_uiFallAsleepTimer <= uiDiff)
					{
						SetSleeping();
						m_uiFallAsleepTimer = urand(25000, 90000);
					}
					else
						m_uiFallAsleepTimer -= uiDiff;
				}

				return;
			}

			DoMeleeAttackIfReady();
		}
	};
};

/*####
# npc_prospector_remtravel
####*/

enum eRemtravel
{
	SAY_REM_START = -1000327,
	SAY_REM_AGGRO = -1000328,
	SAY_REM_RAMP1_1 = -1000329,
	SAY_REM_RAMP1_2 = -1000330,
	SAY_REM_BOOK = -1000331,
	SAY_REM_TENT1_1 = -1000332,
	SAY_REM_TENT1_2 = -1000333,
	SAY_REM_MOSS = -1000334,
	EMOTE_REM_MOSS = -1000335,
	SAY_REM_MOSS_PROGRESS = -1000336,
	SAY_REM_PROGRESS = -1000337,
	SAY_REM_REMEMBER = -1000338,
	EMOTE_REM_END = -1000339,

	FACTION_ESCORTEE = 10,
	QUEST_ABSENT_MINDED_PT2 = 731,
	NPC_GRAVEL_SCOUT = 2158,
	NPC_GRAVEL_BONE = 2159,
	NPC_GRAVEL_GEO = 2160
};

class npc_prospector_remtravel : public CreatureScript
{
public:
	npc_prospector_remtravel() : CreatureScript("npc_prospector_remtravel") { }

	bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
	{
		if (quest->GetQuestId() == QUEST_ABSENT_MINDED_PT2)
		{
			if (npc_escortAI* pEscortAI = CAST_AI(npc_prospector_remtravel::npc_prospector_remtravelAI, creature->AI()))
				pEscortAI->Start(false, false, player->GetGUID());

			creature->setFaction(FACTION_ESCORTEE);
		}

		return true;
	}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_prospector_remtravelAI(creature);
	}

	struct npc_prospector_remtravelAI : public npc_escortAI
	{
		npc_prospector_remtravelAI(Creature* creature) : npc_escortAI(creature) {}

		void WaypointReached(uint32 i)
		{
			Player* player = GetPlayerForEscort();

			if (!player)
				return;

			switch (i)
			{
			case 0:
				//                    DoScriptText(SAY_REM_START, me, player);
				break;
			case 5:
				//                    DoScriptText(SAY_REM_RAMP1_1, me, player);
				break;
			case 6:
				DoSpawnCreature(NPC_GRAVEL_SCOUT, -10.0f, 5.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
				DoSpawnCreature(NPC_GRAVEL_BONE, -10.0f, 7.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
				break;
			case 9:
				//                    DoScriptText(SAY_REM_RAMP1_2, me, player);
				break;
			case 14:
				//                    DoScriptText(SAY_REM_BOOK, me, player);
				break;
			case 15:
				//                    DoScriptText(SAY_REM_TENT1_1, me, player);
				break;
			case 16:
				DoSpawnCreature(NPC_GRAVEL_SCOUT, -10.0f, 5.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
				DoSpawnCreature(NPC_GRAVEL_BONE, -10.0f, 7.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
				break;
			case 17:
				//                    DoScriptText(SAY_REM_TENT1_2, me, player);
				break;
			case 26:
				//                    DoScriptText(SAY_REM_MOSS, me, player);
				break;
			case 27:
				//                    DoScriptText(EMOTE_REM_MOSS, me, player);
				break;
			case 28:
				//                    DoScriptText(SAY_REM_MOSS_PROGRESS, me, player);
				break;
			case 29:
				DoSpawnCreature(NPC_GRAVEL_SCOUT, -15.0f, 3.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
				DoSpawnCreature(NPC_GRAVEL_BONE, -15.0f, 5.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
				DoSpawnCreature(NPC_GRAVEL_GEO, -15.0f, 7.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
				break;
			case 31:
				//                    DoScriptText(SAY_REM_PROGRESS, me, player);
				break;
			case 41:
				//                    DoScriptText(SAY_REM_REMEMBER, me, player);
				break;
			case 42:
				//                    DoScriptText(EMOTE_REM_END, me, player);
				player->GroupEventHappens(QUEST_ABSENT_MINDED_PT2, me);
				break;
			}
		}

		void Reset() {}

		void EnterCombat(Unit* who)
		{
			//            if (rand()%2)
			//                DoScriptText(SAY_REM_AGGRO, me, who);
		}

		void JustSummoned(Creature* /*summoned*/)
		{
			//unsure if it should be any
			//summoned->AI()->AttackStart(me);
		}
	};
};

/*####
# npc_threshwackonator
####*/

enum eThreshwackonator
{
	EMOTE_START = -1000325, //signed for 4966
	SAY_AT_CLOSE = -1000326, //signed for 4966
	QUEST_GYROMAST_REV = 2078,
	NPC_GELKAK = 6667,
	FACTION_HOSTILE = 14
};

#define GOSSIP_ITEM_INSERT_KEY  "[PH] Insert key"

class npc_threshwackonator : public CreatureScript
{
public:
	npc_threshwackonator() : CreatureScript("npc_threshwackonator") { }

	bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
	{
		player->PlayerTalkClass->ClearMenus();
		if (action == GOSSIP_ACTION_INFO_DEF + 1)
		{
			player->CLOSE_GOSSIP_MENU();

			if (npc_threshwackonatorAI* pThreshAI = CAST_AI(npc_threshwackonator::npc_threshwackonatorAI, creature->AI()))
			{
				//                DoScriptText(EMOTE_START, creature);
				pThreshAI->StartFollow(player);
			}
		}

		return true;
	}

	bool OnGossipHello(Player* player, Creature* creature)
	{
		if (player->GetQuestStatus(QUEST_GYROMAST_REV) == QUEST_STATUS_INCOMPLETE)
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_INSERT_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

		player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
		return true;
	}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_threshwackonatorAI(creature);
	}

	struct npc_threshwackonatorAI : public FollowerAI
	{
		npc_threshwackonatorAI(Creature* creature) : FollowerAI(creature) { }

		void Reset() { }

		void MoveInLineOfSight(Unit* who)
		{
			FollowerAI::MoveInLineOfSight(who);

			if (!me->getVictim() && !HasFollowState(STATE_FOLLOW_COMPLETE) && who->GetEntry() == NPC_GELKAK)
			{
				if (me->IsWithinDistInMap(who, 10.0f))
				{
					//                    DoScriptText(SAY_AT_CLOSE, who);
					DoAtEnd();
				}
			}
		}

		void DoAtEnd()
		{
			me->setFaction(FACTION_HOSTILE);

			if (Player* holder = GetLeaderForFollower())
				me->AI()->AttackStart(holder);

			SetFollowComplete();
		}
	};
};

// 32911 Gershala Nightwhisper
class npc_gershala_nightwhisper_32911 : public CreatureScript
{
public:
	npc_gershala_nightwhisper_32911() : CreatureScript("npc_gershala_nightwhisper_32911") { }

	enum eQuest
	{
		QUEST_THE_LAST_WAVE_OF_SURVIVORS = 13518,
		NPC_GERSHALA_NIGHTWHISPER = 32911,
		PLAYER_GUID = 99999,
		NPC_RESCUE_SENTINEL = 32969,
		ACTION_START_RESCUE = 101,
	};

	bool OnGossipHello(Player* player, Creature* creature) override
	{
		creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
		if (player->GetQuestStatus(QUEST_THE_LAST_WAVE_OF_SURVIVORS) == QUEST_STATUS_INCOMPLETE)
			if (!player->GetReqKillOrCastCurrentCount(QUEST_THE_LAST_WAVE_OF_SURVIVORS, NPC_GERSHALA_NIGHTWHISPER))
				if (Creature* npc = creature->SummonCreature(NPC_RESCUE_SENTINEL, 7422.14f, 151.08f, 1.70f, 0.328f, TEMPSUMMON_TIMED_DESPAWN, 120000))
				{
					player->SEND_GOSSIP_MENU(14300, creature->GetGUID());
					player->KilledMonsterCredit(NPC_GERSHALA_NIGHTWHISPER);
					npc->setFaction(35);
					npc->GetAI()->SetGUID(creature->GetGUID(), NPC_GERSHALA_NIGHTWHISPER);
					npc->GetAI()->SetGUID(player->GetGUID(), PLAYER_GUID);
					npc->GetAI()->DoAction(ACTION_START_RESCUE);
					return true;
				}

		return false;
	}
};

// 33093 Cerellean Whiteclaw
class npc_cerellean_whiteclaw_33093 : public CreatureScript
{
public:
	npc_cerellean_whiteclaw_33093() : CreatureScript("npc_cerellean_whiteclaw_33093") { }

	enum eQuest
	{
		QUEST_THE_LAST_WAVE_OF_SURVIVORS = 13518,
		NPC_CERELLEAN_WHITECLAW = 33093,
		PLAYER_GUID = 99999,
		NPC_RESCUE_SENTINEL = 32969,
		ACTION_START_RESCUE = 101,
	};

	bool OnGossipHello(Player* player, Creature* creature) override
	{
		creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
		if (player->GetQuestStatus(QUEST_THE_LAST_WAVE_OF_SURVIVORS) == QUEST_STATUS_INCOMPLETE)
			if (!player->GetReqKillOrCastCurrentCount(QUEST_THE_LAST_WAVE_OF_SURVIVORS, NPC_CERELLEAN_WHITECLAW))
				if (Creature* npc = creature->SummonCreature(NPC_RESCUE_SENTINEL, 7375.88f, 113.33f, 12.38f, 1.865f, TEMPSUMMON_TIMED_DESPAWN, 120000))
				{
					player->SEND_GOSSIP_MENU(14297, creature->GetGUID());
					player->KilledMonsterCredit(NPC_CERELLEAN_WHITECLAW);
					npc->setFaction(35);
					npc->GetAI()->SetGUID(creature->GetGUID(), NPC_CERELLEAN_WHITECLAW);
					npc->GetAI()->SetGUID(player->GetGUID(), PLAYER_GUID);
					npc->GetAI()->DoAction(ACTION_START_RESCUE);
					return true;
				}

		return false;
	}
};

// 33094 Volcor
class npc_volcor_33094 : public CreatureScript
{
public:
	npc_volcor_33094() : CreatureScript("npc_volcor_33094") { }

	enum eQuest
	{
		QUEST_THE_LAST_WAVE_OF_SURVIVORS = 13518,
		NPC_VOLCOR = 33094,
		PLAYER_GUID = 99999,
		NPC_RESCUE_SENTINEL = 32969,
		ACTION_START_RESCUE = 101,
	};

	bool OnGossipHello(Player* player, Creature* creature) override
	{
		creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
		if (player->GetQuestStatus(QUEST_THE_LAST_WAVE_OF_SURVIVORS) == QUEST_STATUS_INCOMPLETE)
			if (!player->GetReqKillOrCastCurrentCount(QUEST_THE_LAST_WAVE_OF_SURVIVORS, NPC_VOLCOR))
				if (Creature* npc = creature->SummonCreature(NPC_RESCUE_SENTINEL, 7431.35f, 87.78f, 2.16f, 1.212f, TEMPSUMMON_TIMED_DESPAWN, 120000))
				{
					player->SEND_GOSSIP_MENU(14298, creature->GetGUID());
					player->KilledMonsterCredit(NPC_VOLCOR);
					npc->setFaction(35);
					npc->GetAI()->SetGUID(creature->GetGUID(), NPC_VOLCOR);
					npc->GetAI()->SetGUID(player->GetGUID(), PLAYER_GUID);
					npc->GetAI()->DoAction(ACTION_START_RESCUE);
					return true;
				}

		return false;
	}
};

// 33095 Shaldyn
class npc_shaldyn_33095 : public CreatureScript
{
public:
	npc_shaldyn_33095() : CreatureScript("npc_shaldyn_33095") { }

	enum eQuest
	{
		QUEST_THE_LAST_WAVE_OF_SURVIVORS = 13518,
		NPC_SHALDYN = 33095,
		PLAYER_GUID = 99999,
		NPC_RESCUE_SENTINEL = 32969,
		ACTION_START_RESCUE = 101,
	};

	bool OnGossipHello(Player* player, Creature* creature) override
	{
		creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
		if (player->GetQuestStatus(QUEST_THE_LAST_WAVE_OF_SURVIVORS) == QUEST_STATUS_INCOMPLETE)
			if (!player->GetReqKillOrCastCurrentCount(QUEST_THE_LAST_WAVE_OF_SURVIVORS, NPC_SHALDYN))
				if (Creature* npc = creature->SummonCreature(NPC_RESCUE_SENTINEL, 7289.61f, 215.36f, 0.53f, 1.299f, TEMPSUMMON_TIMED_DESPAWN, 120000))
				{
					player->SEND_GOSSIP_MENU(14299, creature->GetGUID());
					player->KilledMonsterCredit(NPC_SHALDYN);
					npc->setFaction(35);
					npc->GetAI()->SetGUID(creature->GetGUID(), NPC_SHALDYN);
					npc->GetAI()->SetGUID(player->GetGUID(), PLAYER_GUID);
					npc->GetAI()->DoAction(ACTION_START_RESCUE);
					return true;
				}

		return false;
	}
};

// 32969
class npc_lordanel_sentinel_32969 : public CreatureScript
{
public:
	npc_lordanel_sentinel_32969() : CreatureScript("npc_lordanel_sentinel_32969") { }

	enum eRescue
	{
		QUEST_THE_LAST_WAVE_OF_SURVIVORS = 13518,
		NPC_GERSHALA_NIGHTWHISPER = 32911,
		NPC_CERELLEAN_WHITECLAW = 33093,
		NPC_VOLCOR = 33094,
		NPC_SHALDYN = 33095,
		PLAYER_GUID = 99999,
		NPC_RESCUE_SENTINEL = 32969,
		NPC_FLYING_MOUNT = 43751,
		NPC_RIDING_MOUNT = 33358,
		ACTION_START_RESCUE = 101,
		EVENT_MOVE_RESCUE = 101,
		EVENT_UNMOUNT_1,
		EVENT_SAY_0,
		EVENT_SAY_1,
		EVENT_DESPAWN_1,
		EVENT_DESPAWN_2,
		EVENT_DESPAWN_3,
		MOVE_SENTINEL_TO_RESCUE = 101,
	};

	struct npc_lordanel_sentinel_32969AI : public ScriptedAI
	{
		npc_lordanel_sentinel_32969AI(Creature* creature) : ScriptedAI(creature) { }

		EventMap m_events;
		uint64   m_npc_to_rescueGUID;
		uint64   m_playerGUID;
		uint64   m_mountGUID;
		uint64   m_sentinelGUID;
		bool     m_isFlying;
		bool     m_isRiding;
		uint32   m_say0;
		uint32   m_say1;

		void Reset() override
		{
			m_events.Reset();
			m_npc_to_rescueGUID = NULL;
			m_playerGUID = NULL;
			m_mountGUID = NULL;
			m_sentinelGUID = NULL;
			m_isFlying = false;
			m_isRiding = false;
			m_say0 = 0;
			m_say1 = 0;
		}

		void SetGUID(uint64 guid, int32 id = 0)
		{
			switch (id)
			{
			case PLAYER_GUID:
			{
				m_playerGUID = guid;
				break;
			}
			case NPC_CERELLEAN_WHITECLAW:
			{
				m_say0 = 0;
				m_say1 = 1;
				m_npc_to_rescueGUID = guid;
				break;
			}
			case NPC_GERSHALA_NIGHTWHISPER:
			{
				m_say0 = 2;
				m_say1 = 3;
				m_npc_to_rescueGUID = guid;
				m_isFlying = true;
				break;
			}
			case NPC_SHALDYN:
			{
				m_say0 = 4;
				m_say1 = 5;
				m_npc_to_rescueGUID = guid;
				m_isFlying = true;
				break;
			}
			case NPC_VOLCOR:
			{
				m_say0 = 6;
				m_say1 = 7;
				m_npc_to_rescueGUID = guid;
				m_isRiding = true;
				break;
			}
			}
		}

		void DoAction(const int32 param) override
		{
			switch (param)
			{
			case ACTION_START_RESCUE:
			{
				m_sentinelGUID = me->GetGUID();
				m_events.ScheduleEvent(EVENT_MOVE_RESCUE, 1000);
			}
			}
		}

		void UpdateAI(const uint32 diff) override
		{
			m_events.Update(diff);

			while (uint32 eventId = m_events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_MOVE_RESCUE:
				{
					Creature* mount = NULL;
					if (m_isFlying)
					{
						Position pos;
						me->GetPosition(&pos);
						pos.m_positionZ += 3.0f;
						if (mount = me->SummonCreature(NPC_FLYING_MOUNT, pos, TEMPSUMMON_TIMED_DESPAWN, 100000))
						{
							mount->setFaction(35);
							m_mountGUID = mount->GetGUID();
							me->EnterVehicle(mount);
						}
					}
					else if (m_isRiding)
					{
						if (mount = me->SummonCreature(NPC_RIDING_MOUNT, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 100000))
						{
							mount->setFaction(35);
							m_mountGUID = mount->GetGUID();
							me->EnterVehicle(mount);
						}
					}
					else
						mount = me;

					if (Creature* npc = sObjectAccessor->GetCreature(*me, m_npc_to_rescueGUID))
						if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
						{
							float angle = npc->GetAngle(mount->GetPositionX(), mount->GetPositionY());
							Position pos;
							npc->GetNearPosition(pos, 2.0f, angle);
							if (m_isFlying)
								pos.m_positionZ += 2.0f;
							mount->GetMotionMaster()->MovePoint(MOVE_SENTINEL_TO_RESCUE, pos);
						}
					m_events.ScheduleEvent(EVENT_UNMOUNT_1, 4500);
					break;
				}
				case EVENT_UNMOUNT_1:
				{
					if (m_isFlying || m_isRiding)
						if (Creature* npc = sObjectAccessor->GetCreature(*me, m_mountGUID))
						{
							me->ExitVehicle();
							npc->DespawnOrUnsummon(250);
						}

					m_events.ScheduleEvent(EVENT_SAY_0, 500);
					break;
				}
				case EVENT_SAY_0:
				{
					Talk(m_say0);
					m_events.ScheduleEvent(EVENT_SAY_1, 5000);
					break;
				}
				case EVENT_SAY_1:
				{
					Talk(m_say1);
					m_events.ScheduleEvent(EVENT_DESPAWN_1, 5000);
					break;
				}
				case EVENT_DESPAWN_1:
				{
					me->DespawnOrUnsummon(500);
					break;
				}
				}
			}
		}
	};

	CreatureAI* GetAI(Creature* creature) const override
	{
		return new npc_lordanel_sentinel_32969AI(creature);
	}
};

void AddSC_darkshore()
{
	new npc_kerlonian();
	new npc_prospector_remtravel();
	new npc_threshwackonator();
	new npc_gershala_nightwhisper_32911();
	new npc_cerellean_whiteclaw_33093();
	new npc_volcor_33094();
	new npc_shaldyn_33095();
	new npc_lordanel_sentinel_32969();
}