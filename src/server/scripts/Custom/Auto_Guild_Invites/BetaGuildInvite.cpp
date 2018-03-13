#include "ScriptPCH.h"
#include "ChannelMgr.h"
#include "Guild.h"
#include "GuildMgr.h"

class auto_guild_invite : public PlayerScript
{
public:
	auto_guild_invite() : PlayerScript("auto_guild_invite") {}

	void OnLogin(Player* player) 
	{
		// Mainaz: This script will invite the players into the beta-guild upon logging in the first time
		// Find a better way to check if the player logged in for the first time (there was a flag, can't find it atm tho)
		if (player->GetSession()->GetSecurity() == 0 && player->GetTotalPlayedTime() < 2)
		{
			ObjectGuid targetGuid;
			targetGuid = player->GetGUID();
			// Guild guid for the beta testing guild
			if (Guild* targetGuild = sGuildMgr->GetGuildById(2))
			{
				targetGuild->AddMember(targetGuid);
			}
		}
	}
};

void AddSC_AutoGuildInvite()
{
	new auto_guild_invite();
}
