#include "ScriptPCH.h"
#include "ChannelMgr.h"
#include "Guild.h"
#include "GuildMgr.h"

class auto_guild_invite_gm : public PlayerScript
{
public:
	auto_guild_invite_gm() : PlayerScript("auto_guild_invite_gm") {}

	void OnLogin(Player* player)
	{
		// This script will invite the staff members into the staff guild upon logging in the first time
		// Find a better way to check if the player logged in for the first time?
		if (player->GetSession()->GetSecurity() >= 1 && player->GetTotalPlayedTime() < 2)
		{
			ObjectGuid targetGuid;
			targetGuid = player->GetGUID();
			// Guild guid for the staff guild
			if (Guild* targetGuild = sGuildMgr->GetGuildById(1))
			{
				// player's guild membership checked in AddMember before add
				targetGuild->AddMember(targetGuid);
			}
		}
	}
};

void AddSC_AutoGuildInviteGM()
{
	new auto_guild_invite_gm();
}
