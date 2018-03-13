#include "ScriptMgr.h"
#include "ArenaTeamMgr.h"
#include "Common.h"
#include "DisableMgr.h"
#include "BattlegroundMgr.h"
#include "Battleground.h"
#include "ArenaTeam.h"
#include "Language.h"
#include "BattlegroundQueue.h"
#include "Player.h"
#include "SoloQueue.h"
#include "Chat.h"

SoloQueue::SoloQueue()
{
    queuedHealers.clear();
    queuedMelees.clear();
    queuedRanges.clear();
    allPlayersInQueue.clear();
    lastUpdateTime = getMSTime();
}

SoloQueue::~SoloQueue()
{

}

SoloQueue* SoloQueue::instance()
{
    static SoloQueue instance;
    return &instance;
}

void SoloQueue::AddPlayer(Player* player)
{
    if (CheckRequirements(player))
    {
        if (ArenaTeam* at = sArenaTeamMgr->GetArenaTeamById(player->GetArenaTeamId(ArenaTeam::GetSlotByType(ARENA_TEAM_5v5))))
        {
            SoloQueueInfo* soloInfo = new SoloQueueInfo;
            soloInfo->playerGuid = player->GetGUID();
            soloInfo->rating = at->GetRating();
            soloInfo->ArenaMatchmakerRating = player->GetMMR(ArenaTeam::GetSlotByType(ARENA_TEAM_5v5));
            soloInfo->teamId = at->GetId();
            soloInfo->team = player->GetOTeam();
            player->soloQueueSpec = player->GetActiveSpec();
            player->AddBattlegroundQueueId(BATTLEGROUND_QUEUE_3v3_SOLO);

            switch (GetPlayerTalentCategory(player))
            {
                case TALENT_CAT_HEALER:
                    queuedHealers.emplace(soloInfo->ArenaMatchmakerRating, soloInfo);
                    allPlayersInQueue.emplace(player->GetGUID(), soloInfo);
                    break;
                case TALENT_CAT_MELEE:
                    queuedMelees.emplace(soloInfo->ArenaMatchmakerRating, soloInfo);
                    allPlayersInQueue.emplace(player->GetGUID(), soloInfo);
                    break;
                case TALENT_CAT_RANGE:
                    queuedRanges.emplace(soloInfo->ArenaMatchmakerRating, soloInfo);
                    allPlayersInQueue.emplace(player->GetGUID(), soloInfo);
                    break;
                default:
                    break;
            }
            ChatHandler(player->GetSession()).PSendSysMessage("You are now listed for random solo arena! The solo queue system will now try to find group members based on your MMR!");
        }
    }
}

bool SoloQueue::RemovePlayer(uint64 playerGuid)
{
    allPlayersInQueue.erase(playerGuid);

    for (std::multimap<uint32, SoloQueueInfo*>::iterator itr = queuedRanges.begin(); itr != queuedRanges.end(); ++itr)
        if (itr->second->playerGuid == playerGuid)
        {
            delete itr->second;
            queuedRanges.erase(itr);
            return true;
        }

    for (std::multimap<uint32, SoloQueueInfo*>::iterator itr = queuedHealers.begin(); itr != queuedHealers.end(); ++itr)
        if (itr->second->playerGuid == playerGuid)
        {
            delete itr->second;
            queuedHealers.erase(itr);
            return true;
        }

    for (std::multimap<uint32, SoloQueueInfo*>::iterator itr = queuedMelees.begin(); itr != queuedMelees.end(); ++itr)
        if (itr->second->playerGuid == playerGuid)
        {
            delete itr->second;
            queuedMelees.erase(itr);
            return true;
        }
    return false;
}

void SoloQueue::Update(const uint32 diff)
{
    if (GetMSTimeDiffToNow(lastUpdateTime) < 1000)
        return;

    lastUpdateTime = getMSTime();

    // Update MMR range for all players
    for (auto& allPlayer : allPlayersInQueue)
    {
        if (allPlayer.second->ratingRangeIncreaseCounter > 19)
            continue;

        if (getMSTimeDiff(allPlayer.second->lastMmrUpdate, getMSTime()) >= (45 * IN_MILLISECONDS))
            IncreasePlayerMMrRange(allPlayer.second);
    }

    uint32 maxRatingDiff = sBattlegroundMgr->GetMaxRatingDifference();
    for (std::multimap<uint32, SoloQueueInfo*>::iterator itr = queuedHealers.begin(); itr != queuedHealers.end();)
    {
        std::list<SoloQueueInfo*> playerList;
        playerList.push_back(itr->second);

        uint32 completeRange = maxRatingDiff + itr->second->ratingRange;
        uint32 minMMRHeal = std::max(0, int32(itr->second->ArenaMatchmakerRating - completeRange));
        uint32 maxMMRHeal = std::min(int32(itr->second->ArenaMatchmakerRating + completeRange), 4000);

        for (auto& ranges : queuedRanges)
        {
            uint32 completeRange = maxRatingDiff + ranges.second->ratingRange;
            uint32 minMMRRange = std::max(0, int32(ranges.second->ArenaMatchmakerRating - completeRange));
            uint32 maxMMRRange = std::min(int32(ranges.second->ArenaMatchmakerRating + completeRange), 4000);

            if (IsInMmrRange(minMMRRange, maxMMRRange, itr->second->ArenaMatchmakerRating) 
                || IsInMmrRange(minMMRHeal, maxMMRHeal, ranges.second->ArenaMatchmakerRating))
            {
                playerList.push_back(ranges.second);
                break;
            }
        }

        for (auto& melees : queuedMelees)
        {
            uint32 completeRange = maxRatingDiff + melees.second->ratingRange;
            uint32 minMMRMelee = std::max(0, int32(melees.second->ArenaMatchmakerRating - completeRange));
            uint32 maxMMRMelee = std::min(int32(melees.second->ArenaMatchmakerRating + completeRange), 4000);

            if (IsInMmrRange(minMMRMelee, maxMMRMelee, itr->second->ArenaMatchmakerRating) 
                || IsInMmrRange(minMMRHeal, maxMMRHeal, melees.second->ArenaMatchmakerRating))
            {
                playerList.push_back(melees.second);
                break;
            }
        }

        if (playerList.size() == 3)
        {
            if (Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_AA))
            {
                bg->SetRated(true);
                BattlegroundTypeId bgTypeId = bg->GetTypeID();
                BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, ARENA_TYPE_3v3_SOLO);
                if (PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), 85))
                {
                    BattlegroundQueue &bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);
                    GroupQueueInfo* ginfo = bgQueue.AddSoloQueueGroup(playerList, bracketEntry->GetBracketId(), ALLIANCE);

                    for (SoloQueueInfo* info : playerList)
                    {
                        if (Player* player = ObjectAccessor::FindPlayer(info->playerGuid))
                        {
                            player->AddBattlegroundQueueJoinTime(bgTypeId, ginfo->JoinTime);
                            uint32 avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->GetBracketId());
                            uint32 queueSlot = player->GetBattlegroundQueueIndex(BATTLEGROUND_QUEUE_3v3_SOLO);
                            WorldPacket data;
                            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, player, queueSlot, STATUS_WAIT_QUEUE, avgTime, ginfo->JoinTime, ARENA_TYPE_3v3_SOLO);
                            player->GetSession()->SendPacket(&data);
                        }
                    }
                    sBattlegroundMgr->ScheduleQueueUpdate(ginfo->ArenaMatchmakerRating, ARENA_TYPE_3v3_SOLO, bgQueueTypeId, bgTypeId, bracketEntry->GetBracketId());

                    // everything is done - remove players from solo queue 
                    for (SoloQueueInfo* soloInfo : playerList)
                    {
                        if (soloInfo->playerGuid == itr->second->playerGuid)
                            continue;
                        RemovePlayer(soloInfo->playerGuid);
                    }
                    // we cant remove the heal with RemovePlayer because we still using the itr here
                    allPlayersInQueue.erase(itr->second->playerGuid);
                    delete itr->second;
                    itr = queuedHealers.erase(itr);
                    continue;
                }
            }
        }
        itr++;
    }
}

uint32 SoloQueue::GetPlayerCountInQueue(SoloQueueTalentCategory talentCategory, bool allPlayers)
{
    if (allPlayers)
        return allPlayersInQueue.size();

    switch (talentCategory)
    {
        case TALENT_CAT_HEALER:
            return queuedHealers.size();
        case TALENT_CAT_MELEE:
            return queuedMelees.size();
        case TALENT_CAT_RANGE:
            return queuedRanges.size();
        default:
            break;
    }
    return 0;
}

uint8 SoloQueue::GetPlayerTalentCategory(Player* player)
{
    if (player->HasHealSpec())
        return TALENT_CAT_HEALER;
    else if (player->HasMeleeSpec(true))
        return TALENT_CAT_MELEE;
    else if (player->HasCasterSpec(true))
        return TALENT_CAT_RANGE;

    return TALENT_CAT_UNKNOWN;
}

bool SoloQueue::CheckRequirements(Player* player)
{
    if (sBattlegroundMgr->isTesting())
        return true;

    if (player->HasAura(SPELL_DESERTER_BUFF))
    {
        WorldPacket data;
        sBattlegroundMgr->BuildPlayerJoinedBattlegroundPacket(&data, ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS);
        player->GetSession()->SendPacket(&data);
        return false;
    }

    if (!player->HasFreeBattlegroundQueueId())
    {
        ChatHandler(player->GetSession()).SendSysMessage("You cant join more than 2 battleground queues!");
        return false;
    }

    if (player->InBattleground())
    {
        ChatHandler(player->GetSession()).SendSysMessage("You can't use solo queue while you are in a battleground");
        return false;
    }

    if (player->GetFreeTalentPoints() > 0)
    {
        ChatHandler(player->GetSession()).PSendSysMessage("You have currently %u free talent points. Please spend all your talent points before queueing arena.", player->GetFreeTalentPoints());
        return false;
    }

    if (player->HasTankSpec())
    {
        ChatHandler(player->GetSession()).SendSysMessage("You can't join solo queue with tank spec!");
        return false;
    }

    if (player->isUsingLfg())
    {
        ChatHandler(player->GetSession()).SendSysMessage("You can't join solo queue while you are using the dungeon finder!");
        return false;
    }

    if (player->InBattlegroundQueueForBattlegroundQueueType(BATTLEGROUND_QUEUE_2v2) ||
        player->InBattlegroundQueueForBattlegroundQueueType(BATTLEGROUND_QUEUE_3v3) ||
        player->InBattlegroundQueueForBattlegroundQueueType(BATTLEGROUND_QUEUE_5v5))
    {
        ChatHandler(player->GetSession()).SendSysMessage("You are already listed for another rated arena match!");
        return false;
    }

    if (IsPlayerInSoloQueue(player))
    {
        ChatHandler(player->GetSession()).SendSysMessage("You cant join solo queue multiple times!");
        return false;
    }

    Item* newItem = NULL;
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_OFFHAND || slot == EQUIPMENT_SLOT_RANGED || slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
            continue;

        newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (newItem == NULL)
        {
            ChatHandler(player->GetSession()).SendSysMessage("Your character is not fully equipped!");
            return false;
        }
    }

    if (player->GetAverageItemLevel() < 370)
    {
        ChatHandler(player->GetSession()).PSendSysMessage("Your current average item level is: %u! You should have at least an average item level of 370!", player->GetAverageItemLevel());
        return false;
    }

    return true;
}

bool SoloQueue::IsPlayerInSoloQueue(Player* player)
{
    if (allPlayersInQueue.find(player->GetGUID()) != allPlayersInQueue.end()
        || player->InBattlegroundQueueForBattlegroundQueueType(BATTLEGROUND_QUEUE_1v1_SOLO)
        || player->InBattlegroundQueueForBattlegroundQueueType(BATTLEGROUND_QUEUE_3v3_SOLO))
        return true;
    return false;
}

void SoloQueue::IncreasePlayerMMrRange(SoloQueueInfo* playerInfo)
{
    if (playerInfo->ratingRangeIncreaseCounter > 0)
    {
        playerInfo->ratingRange += 100;
        uint32 completeRange = sBattlegroundMgr->GetMaxRatingDifference() + playerInfo->ratingRange;
        uint32 minRating = std::max(0, int32(playerInfo->ArenaMatchmakerRating - completeRange));
        uint32 maxRating = std::min(int32(playerInfo->ArenaMatchmakerRating + completeRange), 4000);
        if (Player* player = sObjectAccessor->FindPlayer(playerInfo->playerGuid))
            ChatHandler(player->GetSession()).PSendSysMessage("The solo queue system will now try to find group members between %i and %i Matchmaking Rating", minRating, maxRating);
    }
    playerInfo->ratingRangeIncreaseCounter += 1;
    playerInfo->lastMmrUpdate = getMSTime();
}