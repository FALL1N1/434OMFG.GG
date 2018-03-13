#include "WorldPacket.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "ObjectAccessor.h"
#include "Group.h"
#include "GroupMgr.h"
#include "GuildMgr.h"
#include "Unit.h"
#include "EventProcessor.h"

PlayerbotAI::PlayerbotAI(Player* const master, Player* const bot) :
m_master(master), m_bot(bot)
{
}

PlayerbotAI::~PlayerbotAI()
{
}

void PlayerbotAI::MoveUpdate(const uint32 diff)
{
	if (!m_bot->isMoving())
	{
		i_nextMoveTime.Update(diff);
		if (i_nextMoveTime.Passed())
			MoveCalculate();
	}
}

void PlayerbotAI::MoveCalculate()
{
	float respX, respY, respZ, respO, destX, destY, destZ, travelDistZ;
	m_bot->GetPosition(respX, respY, respZ, respO);
	Map const* map = m_bot->GetBaseMap();

	const float angle = float(rand_norm()) * static_cast<float>(M_PI*2.0f);
	const float range = float(rand_norm()) * 50.0f;
	const float distanceX = range * std::cos(angle);
	const float distanceY = range * std::sin(angle);

	destX = respX + distanceX;
	destY = respY + distanceY;

	// prevent invalid coordinates generation
	Trinity::NormalizeMapCoord(destX);
	Trinity::NormalizeMapCoord(destY);

	travelDistZ = range;                                    // sin^2+cos^2=1, so travelDistZ=range^2; no need for sqrt below

	// 10.0 is the max that vmap high can check (MAX_CAN_FALL_DISTANCE)
	travelDistZ = 2.0f;

	// The fastest way to get an accurate result 90% of the time.
	// Better result can be obtained like 99% accuracy with a ray light, but the cost is too high and the code is too long.
	destZ = map->GetHeight(m_bot->GetPhaseMask(), destX, destY, respZ + travelDistZ - 2.0f, false);

	if (std::fabs(destZ - respZ) > travelDistZ)              // Map check
	{
		// Vmap Horizontal or above
		destZ = map->GetHeight(m_bot->GetPhaseMask(), destX, destY, respZ - 2.0f, true);

		if (std::fabs(destZ - respZ) > travelDistZ)
		{
			// Vmap Higher
			destZ = map->GetHeight(m_bot->GetPhaseMask(), destX, destY, respZ + travelDistZ - 2.0f, true);

			// let's forget this bad coords where a z cannot be find and retry at next tick
			if (std::fabs(destZ - respZ) > travelDistZ)
				return;
		}
	}

	i_nextMoveTime.Reset(urand(10000, 20000)); // Movement Reset timer

	m_bot->AddUnitState(UNIT_STATE_ROAMING_MOVE);
	m_bot->GetMotionMaster()->MovePoint(m_bot->GetGUIDLow(), destX, destY, destZ);
}

void PlayerbotAI::UpdateAI(const uint32 diff)
{
	uint8 chance = urand(1, 100);
	chance;
	if (chance <= 20 && (m_bot->getClass() != CLASS_WARRIOR || m_bot->getClass() != CLASS_DRUID))
		MoveUpdate(diff);
	
	if (chance > 20)
		return;
}

void PlayerbotAI::HandleMasterIncomingPacket(const WorldPacket& packet, WorldSession& session)
{
	// Nothing
}

class DelayedRespond : public BasicEvent
{
public:
	DelayedRespond(Player* player, uint32 action) : _player(player), _action(action) { }

	bool Execute(uint64, uint32) override
	{
		if (_action == 1)
			_player->GetPlayerbotAI()->HandleOnDuelReact();

		if (_action == 2)
			_player->GetPlayerbotAI()->HandleOnGroupReact();

		if (_action == 3)
			_player->GetPlayerbotAI()->HandleOnGuildReact();

		if (_action == 4)
			_player->GetPlayerbotAI()->HandleOnTradeReact();
		return true;
	}

private:
	Player* _player;
	uint32 _action;
};

// handle outgoing packets the server would send to the client
void PlayerbotAI::HandleBotOutgoingPacket(const WorldPacket& packet)
{
    switch (packet.GetOpcode())
    {
		case SMSG_DUEL_REQUESTED:
		{
			m_bot->m_Events.AddEvent(new DelayedRespond(m_bot, 1), m_bot->m_Events.CalculateTime(urand(2500, 8000)));
			break;
		}
		case SMSG_GROUP_INVITE:
		{
			m_bot->m_Events.AddEvent(new DelayedRespond(m_bot, 2), m_bot->m_Events.CalculateTime(urand(2500, 8000)));
			break;
		}
		case SMSG_GUILD_INVITE:
		{
			m_bot->m_Events.AddEvent(new DelayedRespond(m_bot, 3), m_bot->m_Events.CalculateTime(urand(2500, 8000)));
			break;
		}
		case SMSG_TRADE_STATUS:
		{
			m_bot->m_Events.AddEvent(new DelayedRespond(m_bot, 4), m_bot->m_Events.CalculateTime(urand(1500, 3000)));
			break;
		}
	}
}

void PlayerbotAI::HandleOnWhisperReact(uint64 receiver)
{
	uint8 chance = urand(1, 100);
	chance;

	if (chance <= 50)
		return;

	if (chance > 50 && chance <= 60)
		m_bot->Whisper("Just doing my thing, and leveling.", LANG_UNIVERSAL, receiver);

	if (chance > 60 && chance <= 70)
		m_bot->Whisper("Can't speak atm man, sorry.", LANG_UNIVERSAL, receiver);

	if (chance > 70 && chance <= 80)
		m_bot->Whisper("Abit busy atm man.", LANG_UNIVERSAL, receiver);

	if (chance > 80 && chance <= 85)
		m_bot->Whisper("Maybe later man", LANG_UNIVERSAL, receiver);

	if (chance > 85 && chance <= 90)
		m_bot->Whisper("Just solo playing a bit atm", LANG_UNIVERSAL, receiver);

	if (chance > 90 && chance <= 95)
		m_bot->Whisper("Maybe later", LANG_UNIVERSAL, receiver);

	if (chance > 95 && chance <= 100)
		m_bot->Whisper("Yo, just a new player chilling around here.", LANG_UNIVERSAL, receiver);
}

void PlayerbotAI::HandleOnDuelReact()
{
	uint8 chance = urand(1, 99);
	chance;

	if (chance <= 33)
		m_bot->Say("Yo, I'm a bit busy right now.", LANG_UNIVERSAL);

	if (chance > 33 && chance <= 66)
	{
		Player* player;
		Player* plTarget;

		if (!m_bot->duel)
			return;

		player = m_bot;
		plTarget = player->duel->opponent;

		if (player == player->duel->initiator || !plTarget || player == plTarget || player->duel->startTime != 0 || plTarget->duel->startTime != 0)
			return;

		time_t now = time(NULL);
		player->duel->startTimer = now;
		plTarget->duel->startTimer = now;

		player->SendDuelCountdown(3000);
		plTarget->SendDuelCountdown(3000);
	}

	if (chance > 66)
		return;
}

void PlayerbotAI::HandleOnGroupReact()
{
	uint8 chance = urand(1, 100);
	chance;

	if (chance <= 10)
		return;

	if (chance > 10 && chance <= 20)
		m_bot->Say("Not interested right now, sorry.", LANG_UNIVERSAL);

	if (chance > 20 && chance <= 35)
		m_bot->Say("Just doing leveling, maybe later.", LANG_UNIVERSAL);

	if (chance > 35 && chance <= 50)
		m_bot->Say("Heya, just solo playing atm.", LANG_UNIVERSAL);

	if (chance > 50 && chance <= 100)
	{
		Group* group = m_bot->GetGroupInvite();
		group->RemoveInvite(m_bot);

		if (group->GetLeaderGUID() == m_bot->GetGUID())
			return;

		if (group->IsFull())
		{
			m_bot->GetSession()->SendPartyResult(PARTY_OP_INVITE, "", ERR_GROUP_FULL);
			return;
		}

		Player* leader = ObjectAccessor::FindPlayer(group->GetLeaderGUID());

		if (!group->IsCreated())
		{
			if (!leader)
			{
				group->RemoveAllInvites();
				return;
			}

			ASSERT(leader);
			group->RemoveInvite(leader);
			group->Create(leader);
			sGroupMgr->AddGroup(group);
		}

		if (!group->AddMember(m_bot))
			return;

		group->BroadcastGroupUpdate();
	}

}

void PlayerbotAI::HandleOnGuildReact()
{
	uint8 chance = urand(1, 100);
	chance;

	if (chance <= 40)
		return;

	if (chance > 40 && chance <= 50)
		m_bot->Say("No thanks", LANG_UNIVERSAL);

	if (chance > 50 && chance <= 60)
		m_bot->Say("Hey, no thanks.", LANG_UNIVERSAL);

	if (chance > 60 && chance <= 70)
		m_bot->Say("Nty.", LANG_UNIVERSAL);

	if (chance > 70 && chance <= 80)
		m_bot->Say("Later sure.", LANG_UNIVERSAL);

	if (chance > 80 && chance <= 100)
	{
		if (!m_bot->GetGuildId())
			if (Guild* guild = sGuildMgr->GetGuildById(m_bot->GetGuildIdInvited()))
				guild->HandleAcceptMember(m_bot->GetSession());
	}
}

void PlayerbotAI::HandleOnTradeReact()
{
	uint8 chance = urand(1, 100);
	chance;

	if (chance <= 50)
		return;

	if (chance > 50 && chance <= 70)
	{
		m_bot->Say("No thanks", LANG_UNIVERSAL);
		if (m_bot)
			m_bot->TradeCancel(true);
	}

	if (chance > 70 && chance <= 100)
	{
		m_bot->Say("no bye", LANG_UNIVERSAL);
		if (m_bot)
			m_bot->TradeCancel(true);
	}
}