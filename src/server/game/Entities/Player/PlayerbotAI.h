#ifndef _PLAYERBOTAI_H
#define _PLAYERBOTAI_H

#include "Common.h"

class WorldPacket;
class Player;
class Unit;
class Object;
class Item;
class PlayerbotMgr;

class PlayerbotAI
{

public:
	PlayerbotAI(Player * const master, Player * const bot);
	virtual ~PlayerbotAI();

    void UpdateAI(const uint32 p_time);

    // This is called by WorldSession.cpp
    // It provides a view of packets normally sent to the client.
    // Since there is no client at the other end, the packets are dropped of course.
    // For a list of opcodes that can be caught see Opcodes.cpp (SMSG_* opcodes only)
	void HandleMasterIncomingPacket(const WorldPacket& packet, WorldSession& session);
    void HandleBotOutgoingPacket(const WorldPacket& packet);

	void HandleOnWhisperReact(uint64 receiver);
	void HandleOnDuelReact();
	void HandleOnGroupReact();
	void HandleOnGuildReact();
	void HandleOnTradeReact();
	void MoveUpdate(const uint32 diff);
	void MoveCalculate();

private:
	// it is safe to keep these back reference pointers because m_bot
	// owns the "this" object and m_master owns m_bot. The owner always cleans up.
    Player* const m_bot;
    Player* const m_master;

	TimeTrackerSmall i_nextMoveTime;
};

#endif
