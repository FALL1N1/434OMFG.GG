#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "endTime.h"
#include "Spell.h"
#include "GameObjectAI.h"

enum InstanceTeleporter
{
    START_TELEPORT         = 102564,
    DRAKE_SANCTUM_RUBIS    = 102579,
    DRAKE_SANCTUM_EMERAUD  = 104761,
    DRAKE_SANCTUM_BRONZE   = 104764,
    DRAKE_SANCTUM_OBSIDIEN = 103868,
    DRAKE_SANCTUM_AZUR     = 102126,
};

struct randBoss
{
    InstanceTeleporter boss1;
    InstanceTeleporter boss2;
    std::string text1;
    std::string text2;
    Bosses eb1;
    Bosses eb2;
};

randBoss const combi[6] =
{
    { DRAKE_SANCTUM_RUBIS,   DRAKE_SANCTUM_EMERAUD,  "Ruby Dragonshrine",    "Emerald Dragonshrine",   BOSS_ECHO_OF_SYLVANAS, BOSS_ECHO_OF_TYRANDE },
    { DRAKE_SANCTUM_RUBIS,   DRAKE_SANCTUM_AZUR,     "Ruby Dragonshrine",    "Azure Dragonshrine",     BOSS_ECHO_OF_SYLVANAS, BOSS_ECHO_OF_JAINA   },
    { DRAKE_SANCTUM_RUBIS,   DRAKE_SANCTUM_OBSIDIEN, "Ruby Dragonshrine",    "Obsidian Dragonshrine",  BOSS_ECHO_OF_SYLVANAS, BOSS_ECHO_OF_BAINE   },
    { DRAKE_SANCTUM_EMERAUD, DRAKE_SANCTUM_AZUR,     "Emerald Dragonshrine", "Azure Dragonshrine",     BOSS_ECHO_OF_TYRANDE,  BOSS_ECHO_OF_JAINA   },
    { DRAKE_SANCTUM_EMERAUD, DRAKE_SANCTUM_OBSIDIEN, "Emerald Dragonshrine", "Obsidian Dragonshrine",  BOSS_ECHO_OF_TYRANDE,  BOSS_ECHO_OF_BAINE   },
    { DRAKE_SANCTUM_AZUR,    DRAKE_SANCTUM_OBSIDIEN, "Azure Dragonshrine",   "Obsidian Dragonshrine",  BOSS_ECHO_OF_JAINA,    BOSS_ECHO_OF_BAINE   }
};

class go_end_time_teleporter : public GameObjectScript
{
public:
    go_end_time_teleporter() : GameObjectScript("go_end_time_teleporter")
    {
    }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        uint8 _combi = sWorld->getWorldState(WS_DAILY_ENDTIME_BOSSES);
        if (player->isInCombat())
            return true;
        if (InstanceScript *instance = go->GetInstanceScript())
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Back to Nozdormu", GOSSIP_SENDER_MAIN, START_TELEPORT);
            if (!instance->IsDone(combi[_combi].eb1))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, combi[_combi].text1, GOSSIP_SENDER_MAIN, combi[_combi].boss1);
            if (instance->IsDone(combi[_combi].eb1) && !instance->IsDone(combi[_combi].eb2))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, combi[_combi].text2, GOSSIP_SENDER_MAIN, combi[_combi].boss2);
            if (!instance->IsDone(BOSS_MUROZOND) && instance->IsDone(combi[_combi].eb1) && instance->IsDone(combi[_combi].eb2))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Bronze Dragonshrine", GOSSIP_SENDER_MAIN, DRAKE_SANCTUM_BRONZE);
        }
        player->SEND_GOSSIP_MENU(player->GetGossipTextId(go), go->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, GameObject* go, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (player->isInCombat())
            return true;
        player->CastSpell(player, action, true);
        return true;
    }
};

void AddSC_end_time_teleporter()
{
    new go_end_time_teleporter();
}
