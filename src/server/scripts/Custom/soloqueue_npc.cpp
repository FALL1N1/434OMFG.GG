#include "ScriptMgr.h"
#include "ArenaTeamMgr.h"
#include "Common.h"
#include "DisableMgr.h"
#include "BattlegroundMgr.h"
#include "Battleground.h"
#include "ArenaTeam.h"
#include "Language.h"
#include "SoloQueue.h"
#include "BattlegroundQueue.h"

class npc_solo_queue : public CreatureScript
{
public:
    npc_solo_queue() : CreatureScript("npc_solo_queue")
    {
        for (int i = 0; i < TALENT_CAT_MELEE; i++)
            cache3v3Queue[i] = 0;
        lastFetchQueueList = 0;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (!sWorld->getBoolConfig(CONFIG_ARENA_SOLO_QUEUE_ENABLED))
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Solo queue is currently disabled!", GOSSIP_SENDER_MAIN, 0);
            player->SEND_GOSSIP_MENU(60015, creature->GetGUID());
            return true;
        }

        fetchQueueList();
        std::stringstream infoQueue;
        infoQueue << "Solo queue statistics:\n\n";
        infoQueue << "Solo teams in queue: " << cache3v3Queue[TALENT_CAT_UNKNOWN] << "\n";
        infoQueue << "\n";
        infoQueue << "Queued melees: " << cache3v3Queue[TALENT_CAT_MELEE] << "\n";
        infoQueue << "Queued casters: " << cache3v3Queue[TALENT_CAT_RANGE] << "\n";
        infoQueue << "Queued healers: " << cache3v3Queue[TALENT_CAT_HEALER] << "\n";

        if (player->getLevel() < 85)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, infoQueue.str().c_str(), GOSSIP_SENDER_MAIN, 0);
            player->SEND_GOSSIP_MENU(60015, creature->GetGUID());
            return true;
        }

        if (sSoloQueueMgr->IsPlayerInSoloQueue(player))
            player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_INTERACT_1, "Leave Solo queue", GOSSIP_SENDER_MAIN, 3, "Are you sure you want to remove the solo queue?", 0, false);

        if (player->GetArenaTeamId(ArenaTeam::GetSlotByType(ARENA_TEAM_5v5)) == NULL)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "Create new Solo arena team", GOSSIP_SENDER_MAIN, 1);
        else
        {
            if (ArenaTeam* at = sArenaTeamMgr->GetArenaTeamById(player->GetArenaTeamId(ArenaTeam::GetSlotByType(ARENA_TEAM_5v5))))
            {
                if (at->IsSoloQueueTeam())
                {
                    if (!player->InBattlegroundQueueForBattlegroundQueueType(BATTLEGROUND_QUEUE_2v2) &&
                        !player->InBattlegroundQueueForBattlegroundQueueType(BATTLEGROUND_QUEUE_3v3) &&
                        !player->InBattlegroundQueueForBattlegroundQueueType(BATTLEGROUND_QUEUE_5v5) &&
                        !sSoloQueueMgr->IsPlayerInSoloQueue(player))
                    {
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "Queue up for 1vs1 Skirmish \n", GOSSIP_SENDER_MAIN, 20);
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "Queue up for 3vs3 Arena \n", GOSSIP_SENDER_MAIN, 2);
                    }
                }
                else
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "Your 5vs5 team is not a solo queue team!\nLeave the 5vs5 team and create a solo queue team!\n", GOSSIP_SENDER_MAIN, 21);
            }
        }

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, infoQueue.str().c_str(), GOSSIP_SENDER_MAIN, 0);
        player->SEND_GOSSIP_MENU(60015, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {
            case 1: // Create new solo queue team
            {
                uint8 slot = ArenaTeam::GetSlotByType(ARENA_TEAM_5v5);

                if (player->GetArenaTeamId(slot))
                {
                    player->GetSession()->SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, player->GetName(), "", ERR_ALREADY_IN_ARENA_TEAM);
                    player->CLOSE_GOSSIP_MENU();
                    return true;
                }

                // Teamname = playername
                // if team name exist, we have to choose another name (playername + number)
                int i = 1;
                std::stringstream teamName;
                teamName << player->GetName();
                do
                {
                    if (sArenaTeamMgr->GetArenaTeamByName(teamName.str()) != NULL) // teamname exist, so choose another name
                    {
                        teamName.str(std::string());
                        teamName << player->GetName() << i++;
                    }
                    else
                        break;
                } while (i < 100);

                // Create arena team
                ArenaTeam* arenaTeam = new ArenaTeam();
                if (!arenaTeam->Create(player->GetGUID(), ARENA_TEAM_5v5, teamName.str(), 4282747090, 58, 4282908642, 4, 4287950591, true))
                {
                    delete arenaTeam;
                    player->CLOSE_GOSSIP_MENU();
                    return true;
                }
                sArenaTeamMgr->AddArenaTeam(arenaTeam);
                arenaTeam->AddMember(player->GetGUID());
                break;
            }
            case 2: // Join 3 v 3 solo queue
            {
                sSoloQueueMgr->AddPlayer(player);
                break;
            }
            case 3: // Leave Queue
            {
                if (sSoloQueueMgr->RemovePlayer(player->GetGUID()))
                {
                    player->RemoveBattlegroundQueueId(BATTLEGROUND_QUEUE_3v3_SOLO);
                }
                else if (player->InBattlegroundQueueForBattlegroundQueueType(BATTLEGROUND_QUEUE_1v1_SOLO))
                {
                    if (Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_AA))
                    {
                        WorldPacket data;
                        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, player, player->GetBattlegroundQueueIndex(BATTLEGROUND_QUEUE_1v1_SOLO), STATUS_NONE, player->GetBattlegroundQueueJoinTime(BATTLEGROUND_AA), 0, ARENA_TYPE_1v1_SOLO);
                        player->GetSession()->SendPacket(&data);
                    }

                    BattlegroundQueue& queue = sBattlegroundMgr->GetBattlegroundQueue(BATTLEGROUND_QUEUE_1v1_SOLO);
                    queue.RemovePlayer(player->GetGUID(), false);
                    player->RemoveBattlegroundQueueId(BATTLEGROUND_QUEUE_1v1_SOLO);
                }
                else if (player->InBattlegroundQueueForBattlegroundQueueType(BATTLEGROUND_QUEUE_3v3_SOLO))
                {
                    if (Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_AA))
                    {
                        WorldPacket data;
                        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, player, player->GetBattlegroundQueueIndex(BATTLEGROUND_QUEUE_3v3_SOLO), STATUS_NONE, player->GetBattlegroundQueueJoinTime(BATTLEGROUND_AA), 0, ARENA_TYPE_3v3_SOLO);
                        player->GetSession()->SendPacket(&data);
                    }

                    BattlegroundQueue& queue = sBattlegroundMgr->GetBattlegroundQueue(BATTLEGROUND_QUEUE_3v3_SOLO);
                    queue.RemovePlayer(player->GetGUID(), false);
                    player->RemoveBattlegroundQueueId(BATTLEGROUND_QUEUE_3v3_SOLO);
                }
                ChatHandler(player->GetSession()).SendSysMessage("You are no longer listed for solo queue!");
                break;
            }
            case 20:
            {
                if (sSoloQueueMgr->CheckRequirements(player))
                    Join1v1Queue(player);
                break;
            }
            case 21:
            {
                player->CLOSE_GOSSIP_MENU();
                return true;
            }
        }
        OnGossipHello(player, creature);
        return true;
    }


    void Join1v1Queue(Player* player)
    {
        // ignore if we already in BG or BG queue
        if (player->InBattleground() || player->InBattlegroundQueue())
            return;

        uint64 guid = player->GetGUID();
        uint8 arenaslot = ArenaTeam::GetSlotByType(ARENA_TEAM_5v5);
        uint32 matchmakerRating = player->GetMMR(arenaslot);

        if (Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_AA))
        {
            BattlegroundTypeId bgTypeId = bg->GetTypeID();
            BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, ARENA_TYPE_1v1_SOLO);
            PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), player->getLevel());
            if (!bracketEntry)
                return;

            // check if already in queue
            if (player->GetBattlegroundQueueIndex(bgQueueTypeId) < PLAYER_MAX_BATTLEGROUND_QUEUES)
                return;

            // check if has free queue slots
            if (!player->HasFreeBattlegroundQueueId())
                return;

            uint32 ateamId = 0;
            BattlegroundQueue &bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);
            bg->SetRated(false);

            GroupQueueInfo* ginfo = bgQueue.AddGroup(player, NULL, bgTypeId, bracketEntry, ARENA_TYPE_1v1_SOLO, false, false, 0, matchmakerRating, ateamId);
            player->AddBattlegroundQueueJoinTime(bgTypeId, ginfo->JoinTime);
            uint32 avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->GetBracketId());
            uint32 queueSlot = player->AddBattlegroundQueueId(bgQueueTypeId);

            WorldPacket data;
            // send status packet (in queue)
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, player, queueSlot, STATUS_WAIT_QUEUE, avgTime, ginfo->JoinTime, ARENA_TYPE_1v1_SOLO);
            player->GetSession()->SendPacket(&data);
            sBattlegroundMgr->ScheduleQueueUpdate(0, ARENA_TYPE_1v1_SOLO, bgQueueTypeId, bgTypeId, bracketEntry->GetBracketId());
        }
    }

    void fetchQueueList()
    {
        if (GetMSTimeDiffToNow(lastFetchQueueList) < 1000)
            return;
        lastFetchQueueList = getMSTime();
        cache3v3Queue[TALENT_CAT_MELEE] = sSoloQueueMgr->GetPlayerCountInQueue(TALENT_CAT_MELEE);
        cache3v3Queue[TALENT_CAT_RANGE] = sSoloQueueMgr->GetPlayerCountInQueue(TALENT_CAT_RANGE);
        cache3v3Queue[TALENT_CAT_HEALER] = sSoloQueueMgr->GetPlayerCountInQueue(TALENT_CAT_HEALER);
        BattlegroundQueue const& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(BATTLEGROUND_QUEUE_3v3_SOLO);
        cache3v3Queue[TALENT_CAT_UNKNOWN] = bgQueue.GetQueuedGroups();
    }

    private:
        int cache3v3Queue[TALENT_CAT_MELEE];
        uint32 lastFetchQueueList;
};

class RemovePlayerFromQueue : public PlayerScript
{
public:
    RemovePlayerFromQueue() : PlayerScript("RemovePlayerFromQueue") { }

    void OnLogout(Player* player)
    {
        sSoloQueueMgr->RemovePlayer(player->GetGUID());
    }
};

void AddSC_solo_queue()
{
    new npc_solo_queue();
    new RemovePlayerFromQueue();
}