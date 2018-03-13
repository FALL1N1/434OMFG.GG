
#include "ArenaTeamMgr.h"
#include "BattlegroundMgr.h"
#include "Chat.h"
#include "DisableMgr.h"
#include "Language.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"

#include "Formulas.h"
#include "InfoMgr.h"

class Skirmish_npc : public CreatureScript
{
public:
    Skirmish_npc() : CreatureScript("Skirmish_npc") {}

    bool OnGossipHello(Player *pPlayer, Creature *pCreature)
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Queue for 2v2 Skirmish", GOSSIP_SENDER_MAIN, ARENA_TYPE_2v2);
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Queue for 3v3 Skirmish", GOSSIP_SENDER_MAIN, ARENA_TYPE_3v3);
        pPlayer->PlayerTalkClass->SendGossipMenu(1, pCreature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player *pPlayer, Creature * /*pCreature*/, uint32 /*sender*/, uint32 action)
    {
        if (action != ARENA_TYPE_2v2 && action != ARENA_TYPE_3v3)
            return true;

        if (pPlayer->InBattlegroundQueue())
            return true;

        uint32 arenaRating = 0;
        uint32 matchmakerRating = 0;

        uint8 arenatype = action;

        //check existance
        Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_AA);
        if (!bg)
        {
            TC_LOG_ERROR("network.opcode", "Battleground: template bg (all arenas) not found");
            return false;
        }

        if (DisableMgr::IsDisabledFor(DISABLE_TYPE_BATTLEGROUND, BATTLEGROUND_AA, NULL))
        {
            ChatHandler(pPlayer->GetSession()).PSendSysMessage(LANG_ARENA_DISABLED);
            return false;
        }

        BattlegroundTypeId bgTypeId = bg->GetTypeID();
        BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, arenatype);
        PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), pPlayer->getLevel());
        if (!bracketEntry)
            return false;

        BattlegroundQueue &bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);

        uint32 avgTime = 0;
        GroupQueueInfo* ginfo = bgQueue.AddGroup(pPlayer, NULL, bgTypeId, bracketEntry, arenatype, false, false, arenaRating, matchmakerRating, 0);
        avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->GetBracketId());

        // add to queue
        uint32 queueSlot = pPlayer->AddBattlegroundQueueId(bgQueueTypeId);
        // add joined time data
        pPlayer->AddBattlegroundQueueJoinTime(bgTypeId, ginfo->JoinTime);

        WorldPacket data; // send status packet (in queue)
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, pPlayer, queueSlot, STATUS_WAIT_QUEUE, avgTime, ginfo->JoinTime, arenatype);
        pPlayer->GetSession()->SendPacket(&data);

        sBattlegroundMgr->ScheduleQueueUpdate(matchmakerRating, arenatype, bgQueueTypeId, bgTypeId, bracketEntry->GetBracketId());

        pPlayer->PlayerTalkClass->ClearMenus();
        pPlayer->CLOSE_GOSSIP_MENU();
        return true;
    }
};

class SaveNewCap : public PlayerScript
{
public:
    SaveNewCap() : PlayerScript("SaveNewCap") { }

    void OnLogout(Player* player)
    {
        if (player->GetCurrency(CURRENCY_TYPE_CONQUEST_POINTS, true))
        {
            PreparedStatement* stmt = NULL;
            uint32 capArena = Trinity::Currency::ConquestRatingCalculator(player->GetMaxPersonalArenaRatingRequirement(0)) * CURRENCY_PRECISION;
            uint32 capRBG = Trinity::Currency::BgConquestRatingCalculator(player->GetRbgOrSoloQueueRatingForCapCalculation()) * CURRENCY_PRECISION;
            uint32 cap = std::max(capArena, capRBG);
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_PLAYER_CURRENCY_NEW_CAP_OVERALL);
            stmt->setUInt32(0, cap);
            stmt->setUInt64(1, player->GetGUIDLow());
            CharacterDatabase.Execute(stmt);

            if (player->GetCurrency(CURRENCY_TYPE_CONQUEST_META_ARENA, true))
            {
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_PLAYER_CURRENCY_NEW_CAP_ARENA);
                stmt->setUInt32(0, capArena);
                stmt->setUInt64(1, player->GetGUIDLow());
                CharacterDatabase.Execute(stmt);
            }

            if (player->GetCurrency(CURRENCY_TYPE_CONQUEST_META_RBG, true))
            {
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_PLAYER_CURRENCY_NEW_CAP_RBG);
                stmt->setUInt32(0, capRBG);
                stmt->setUInt64(1, player->GetGUIDLow());
                CharacterDatabase.Execute(stmt);
            }
        }
    }

};

class arena_spectator_commands : public CommandScript
{
public:
    arena_spectator_commands() : CommandScript("arena_spectator_commands") { }

    static bool HandleSpectateCommand(ChatHandler* handler, const char *args)
    {
        Player* target;
        uint64 target_guid;
        std::string target_name;
        if (!handler->extractPlayerTarget((char*)args, &target, &target_guid, &target_name))
            return false;

        Player* player = handler->GetSession()->GetPlayer();
        if (target == player || target_guid == player->GetGUID())
        {
            handler->SendSysMessage("You cant teleport to yourself");
            handler->SetSentErrorMessage(true);
            player->CLOSE_GOSSIP_MENU();
            return false;
        }

        if (player->isInCombat())
        {
            handler->SendSysMessage("Cant do that while in combat");
            handler->SetSentErrorMessage(true);
            player->CLOSE_GOSSIP_MENU();
            return false;
        }

        if (!target)
        {
            handler->SendSysMessage("The selected player does not exist or is offline");
            handler->SetSentErrorMessage(true);
            player->CLOSE_GOSSIP_MENU();
            return false;
        }

        if (player->GetPet())
        {
            handler->PSendSysMessage("You must dismiss your pet.");
            handler->SetSentErrorMessage(true);
            player->CLOSE_GOSSIP_MENU();
            return false;
        }

        if (player->GetMap()->IsBattlegroundOrArena() && !player->isSpectator())
        {
            handler->PSendSysMessage("You are already in battleground or arena.");
            handler->SetSentErrorMessage(true);
            player->CLOSE_GOSSIP_MENU();
            return false;
        }

        Map* cMap = target->GetMap();
        if (!cMap->IsBattleArena())
        {
            handler->PSendSysMessage("No player found in arena.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (Battleground* bg = target->GetBattleground())
            if ((bg->GetStatus() != STATUS_IN_PROGRESS) && !player->isGameMaster())
            {
                handler->PSendSysMessage("Battleground not started yet.");
                handler->SetSentErrorMessage(true);
                return false;
            }

        if (player->GetMap()->IsBattleground())
        {
            handler->PSendSysMessage("Cant do that while you are in battleground.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        // all's well, set bg id
        // when porting out from the bg, it will be reset to 0
        player->SetBattlegroundId(target->GetBattlegroundId(), target->GetBattlegroundTypeId());
        // remember current position as entry point for return at bg end teleportation
        if (!player->GetMap()->IsBattlegroundOrArena())
            player->SetBattlegroundEntryPoint();

        if (target->isSpectator())
        {
            handler->PSendSysMessage("Can`t do that. Your target is spectator.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (player->isInFlight())
        {
            player->GetMotionMaster()->MovementExpired();
            player->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
            player->SaveRecallPosition();

        // search for two teams
        Battleground* bGround = target->GetBattleground();
        if (!bGround)
            return false;
        if (bGround->isRated())
        {
            uint32 slot = bGround->GetArenaType() - 2;
            if (bGround->GetArenaType() > 3)
                slot = 2;
            uint32 firstTeamID = target->GetArenaTeamId(slot);
            uint32 secondTeamID = 0;
            Player *firstTeamMember = target;
            Player *secondTeamMember = NULL;
            for (Battleground::BattlegroundPlayerMap::const_iterator itr = bGround->GetPlayers().begin(); itr != bGround->GetPlayers().end(); ++itr)
                if (Player* tmpPlayer = ObjectAccessor::FindPlayer(itr->first))
                {
                    if (tmpPlayer->isSpectator())
                        continue;

                    uint32 tmpID = tmpPlayer->GetArenaTeamId(slot);
                    if (tmpID != firstTeamID && tmpID > 0)
                    {
                        secondTeamID = tmpID;
                        secondTeamMember = tmpPlayer;
                        break;
                    }
                }

                if (firstTeamID > 0 && secondTeamID > 0 && secondTeamMember)
                {
                    ArenaTeam *firstTeam = sArenaTeamMgr->GetArenaTeamById(firstTeamID);
                    ArenaTeam *secondTeam = sArenaTeamMgr->GetArenaTeamById(secondTeamID);
                    if (firstTeam && secondTeam)
                    {
                        handler->PSendSysMessage("You entered rated arena.");
                        handler->PSendSysMessage("Teams:");
                        handler->PSendSysMessage("%s - %s", firstTeam->GetName().c_str(), secondTeam->GetName().c_str());
                        handler->PSendSysMessage("%u(%u) - %u(%u)", firstTeam->GetRating(), firstTeam->GetAverageMMR(firstTeamMember->GetGroup()),
                            secondTeam->GetRating(), secondTeam->GetAverageMMR(secondTeamMember->GetGroup()));
                        handler->PSendSysMessage("Type .spec leave to leave arena");
                        handler->PSendSysMessage("Type .spec watch to spectate from the viewpoint of the targeted player");
                    }
                }
        }

        // to point to see at target with same orientation
        float x, y, z;
        target->GetContactPoint(player, x, y, z);

        player->TeleportTo(target->GetMapId(), x, y, z, player->GetAngle(target), TELE_TO_GM_MODE);
        player->SetPhaseMask(target->GetPhaseMask(), true);
        player->SetSpectate(true);

        return true;
    }

    static bool HandleSpectateCancelCommand(ChatHandler* handler, const char* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (!player->isSpectator())
        {
            handler->PSendSysMessage("You are not a spectator.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        player->TeleportToBGEntryPoint();

        return true;
    }

    static bool HandleSpectateFromCommand(ChatHandler* handler, const char *args)
    {
        Player* target;
        uint64 target_guid;
        std::string target_name;
        if (!handler->extractPlayerTarget((char*)args, &target, &target_guid, &target_name))
            return false;

        Player* player = handler->GetSession()->GetPlayer();

        if (!target)
        {
            handler->PSendSysMessage("Cant find player.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!player->isSpectator())
        {
            handler->PSendSysMessage("You are not spectator, spectate someone first.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (target->isSpectator() && target != player)
        {
            handler->PSendSysMessage("Can`t do that. Your target is spectator.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->GetMap() != target->GetMap())
        {
            handler->PSendSysMessage("Cant do that. Different arenas?");
            handler->SetSentErrorMessage(true);
            return false;
        }

        // check for arena preperation
        // if exists than battle didn`t begin
        if (target->HasAura(32728) || target->HasAura(32727))
        {
            handler->PSendSysMessage("Cant do that. Arena didn`t start.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->GetViewpoint())
            player->StopCastingBindSight();
        else
            player->CastSpell(target, 6277, true);

        return true;
    }

    static bool HandleSpectateResetCommand(ChatHandler* handler, const char *args)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (!player)
        {
            handler->PSendSysMessage("Cant find player.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!player->isSpectator() || !player->InArena())
        {
            handler->PSendSysMessage("You are not spectator!");
            handler->SetSentErrorMessage(true);
            return false;
        }

        Map::PlayerList const &PlList = player->GetMap()->GetPlayers();
        if (PlList.isEmpty())
            return true;
        for (Map::PlayerList::const_iterator i = PlList.begin(); i != PlList.end(); ++i)
        {
            if (Player* pPlayer = i->getSource())
            {
                if (pPlayer->isGameMaster() || pPlayer->isSpectator())
                    continue;

                pPlayer->m_arenaSpectatorFlags = ASPEC_ALL_FLAGS;
            }
        }

        return true;
    }

    static bool HandleForceBGCommand(ChatHandler* handler, const char *args)
    {
        if (!*args)
            return false;

        uint32 bgtypeid = atoi(args);
        sWorld->SetForcedBG(BattlegroundTypeId(bgtypeid));

        return true;
    }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> spectateCommandTable =
        {
            { "spectate", SEC_MODERATOR, true, &HandleSpectateCommand,      "" },
            { "watch",    SEC_MODERATOR, true, &HandleSpectateFromCommand,  "" },
            { "reset",    SEC_MODERATOR, true, &HandleSpectateResetCommand, "" },
            { "leave",    SEC_MODERATOR, true, &HandleSpectateCancelCommand,"" },
            { "setbg",    SEC_MODERATOR, true, &HandleForceBGCommand,       "" },
        };

        static std::vector<ChatCommand> commandTable =
        {
            { "spectator", SEC_PLAYER, false, NULL, "", spectateCommandTable },
        };
        return commandTable;
    }
};

enum NpcSpectatorActions
{
    // will be used for scrolling
    NPC_SPECTATOR_ACTION_CANCEL = 500,
    NPC_SPECTATOR_ACTION_LIST_GAMES_2v2 = 2000,
    NPC_SPECTATOR_ACTION_LIST_TOP_GAMES_2v2 = 3000,
    NPC_SPECTATOR_ACTION_LIST_GAMES_3v3 = 4000,
    NPC_SPECTATOR_ACTION_LIST_TOP_GAMES_3v3 = 5000,
    NPC_SPECTATOR_ACTION_LIST_2v2 = 1000,
    NPC_SPECTATOR_ACTION_LIST_3v3 = 1001,
    NPC_SPECTATOR_ACTION_PLAYER = 6000,
};

const uint16 TopGamesRating = 2000;
const uint8 GamesOnPage = 20;

class npc_arena_spectator : public CreatureScript
{
public:
    npc_arena_spectator() : CreatureScript("npc_arena_spectator") { }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
        pPlayer->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_CHAT, "Teleport me to the following player...", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_PLAYER, "", 0, true);
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "2vs2 with lower MMR than 2000", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_LIST_GAMES_2v2);
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "2vs2 with higher MMR than 2000", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_LIST_TOP_GAMES_2v2);
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "3vs3 with lower MMR than 2000", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_LIST_GAMES_3v3);
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "3vs3 with higher MMR than 2000", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_LIST_TOP_GAMES_3v3);
        pPlayer->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();

        if (action >= NPC_SPECTATOR_ACTION_LIST_GAMES_2v2 && action < NPC_SPECTATOR_ACTION_LIST_TOP_GAMES_2v2)
        {
            ShowPage(player, action - NPC_SPECTATOR_ACTION_LIST_GAMES_2v2, SLOT_2VS2);
            player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        }
        else if (action >= NPC_SPECTATOR_ACTION_LIST_TOP_GAMES_2v2 && action < NPC_SPECTATOR_ACTION_LIST_GAMES_3v3)
        {
            ShowPage(player, action - NPC_SPECTATOR_ACTION_LIST_TOP_GAMES_2v2, SLOT_2VS2_TOP);
            player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        }
        else if (action >= NPC_SPECTATOR_ACTION_LIST_GAMES_3v3 && action < NPC_SPECTATOR_ACTION_LIST_TOP_GAMES_3v3)
        {
            ShowPage(player, action - NPC_SPECTATOR_ACTION_LIST_GAMES_3v3, SLOT_3VS3);
            player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        }
        else if (action >= NPC_SPECTATOR_ACTION_LIST_TOP_GAMES_3v3 && action < NPC_SPECTATOR_ACTION_PLAYER)
        {
            ShowPage(player, action - NPC_SPECTATOR_ACTION_LIST_TOP_GAMES_3v3, SLOT_3VS3_TOP);
            player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        }
        else if (action == NPC_SPECTATOR_ACTION_CANCEL)
            player->CLOSE_GOSSIP_MENU();
        else
        {
            uint32 teamid = action - NPC_SPECTATOR_ACTION_PLAYER;
            if (ArenaTeam* team = sArenaTeamMgr->GetArenaTeamById(teamid))
            {
                if (Player* target = team->GetFirstMemberInArena())
                {
                    ChatHandler handler(player->GetSession());
                    arena_spectator_commands::HandleSpectateCommand(&handler, target->GetName().c_str());
                }
            }
        }
        return true;
    }

    bool OnGossipSelectCode(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction, const char* code)
    {
        player->PlayerTalkClass->ClearMenus();
        if (uiSender == GOSSIP_SENDER_MAIN)
        {
            switch (uiAction)
            {
            case NPC_SPECTATOR_ACTION_PLAYER:

                std::string name(code);
                if (!name.empty())
                    name[0] = toupper(name[0]);

                ChatHandler handler(player->GetSession());
                arena_spectator_commands::HandleSpectateCommand(&handler, name.c_str());
                return true;
            }
        }

        return false;
    }

    std::string GetClassAndTalentnameById(uint32 id)
    {
        std::string sClass = "";
        switch (id)
        {
        case TALENT_TREE_WARRIOR_ARMS: sClass = " Arms War "; break;
        case TALENT_TREE_WARRIOR_FURY: sClass = " Fury War "; break;
        case TALENT_TREE_WARRIOR_PROTECTION: sClass = " Prot War "; break;
        case TALENT_TREE_PALADIN_HOLY: sClass = " Holy Pal "; break;
        case TALENT_TREE_PALADIN_PROTECTION: sClass = " Prot Pal "; break;
        case TALENT_TREE_PALADIN_RETRIBUTION: sClass = " Ret Pal "; break;
        case TALENT_TREE_HUNTER_BEAST_MASTERY: sClass = " BM Hunt "; break;
        case TALENT_TREE_HUNTER_MARKSMANSHIP: sClass = " MM Hunt "; break;
        case TALENT_TREE_HUNTER_SURVIVAL: sClass = " SV Hunt "; break;
        case TALENT_TREE_ROGUE_ASSASSINATION: sClass = " ASS Rog "; break;
        case TALENT_TREE_ROGUE_COMBAT: sClass = " Com Rog "; break;
        case TALENT_TREE_ROGUE_SUBTLETY: sClass = " Sub Rog "; break;
        case TALENT_TREE_PRIEST_DISCIPLINE: sClass = " Disc Pri "; break;
        case TALENT_TREE_PRIEST_HOLY: sClass = " Holy Pri "; break;
        case TALENT_TREE_PRIEST_SHADOW: sClass = " Shad Pri "; break;
        case TALENT_TREE_DEATH_KNIGHT_BLOOD: sClass = " Blood DK "; break;
        case TALENT_TREE_DEATH_KNIGHT_FROST: sClass = " Frost DK "; break;
        case TALENT_TREE_DEATH_KNIGHT_UNHOLY: sClass = " Unh DK "; break;
        case TALENT_TREE_SHAMAN_ELEMENTAL: sClass = " Ele Sham "; break;
        case TALENT_TREE_SHAMAN_ENHANCEMENT: sClass = " Enh Sham "; break;
        case TALENT_TREE_SHAMAN_RESTORATION: sClass = " Rest Sham "; break;
        case TALENT_TREE_MAGE_ARCANE: sClass = " Arc Mage "; break;
        case TALENT_TREE_MAGE_FIRE: sClass = " Fire Mage "; break;
        case TALENT_TREE_MAGE_FROST: sClass = " Frost Mage "; break;
        case TALENT_TREE_WARLOCK_AFFLICTION: sClass = " Affli Warl "; break;
        case TALENT_TREE_WARLOCK_DEMONOLOGY: sClass = " Demo Warl "; break;
        case TALENT_TREE_WARLOCK_DESTRUCTION: sClass = " Destr Warl "; break;
        case TALENT_TREE_DRUID_BALANCE: sClass = " Boomkin "; break;
        case TALENT_TREE_DRUID_FERAL_COMBAT: sClass = " Feral Dru "; break;
        case TALENT_TREE_DRUID_RESTORATION: sClass = " Restr Dru "; break;
        }
        return sClass;
    }

    std::string GetGamesStringData(Player* player, uint16 mmr)
    {
        std::string teamsMember[BG_TEAMS_COUNT];
        uint32 firstTeamId = 0;
        if (Battleground* arena = player->GetBattleground())
        {
            for (Battleground::BattlegroundPlayerMap::const_iterator itr = arena->GetPlayers().begin(); itr != arena->GetPlayers().end(); ++itr)
            {
                if (Player* member = ObjectAccessor::FindPlayer(itr->first))
                {
                    if (member->isSpectator())
                        continue;

                    uint32 team = itr->second.Team;
                    if (!firstTeamId)
                        firstTeamId = team;

                    teamsMember[firstTeamId == team] += GetClassAndTalentnameById(member->GetPrimaryTalentTree(member->GetActiveSpec()));
                }
            }

            std::stringstream ss;
            ss << teamsMember[0] << " - " << mmr << " - " << teamsMember[1];
            std::string data = ss.str();

            //TC_LOG_INFO("server.loading", "%s", data.c_str());
            return data;
        }
        return "An error occured";
    }

    void ShowPage(Player* player, uint16 page, SlotStoreType slot)
    {
        uint16 highGames = 0;
        uint16 lowGames = 0;
        bool haveNextPage = false;
        player->PlayerTalkClass->ClearMenus();
        player->PlayerTalkClass->SendCloseGossip();

        std::map<uint32, InfoBGEntry> bginfomap = sInfoMgr->GetBgStore(slot);
        for (InfoMgr::BGInfoMap::iterator iter = bginfomap.begin(); iter != bginfomap.end(); ++iter)
        {
            ArenaTeam* team1 = sArenaTeamMgr->GetArenaTeamById(iter->second.team1);
            ArenaTeam* team2 = sArenaTeamMgr->GetArenaTeamById(iter->second.team2);

            if (!team1 || !team2)
                continue;

            Player* member = team1->GetFirstMemberInArena();

            if (!member)
                continue;

            uint16 highestMMR = std::max(iter->second.MMR1, iter->second.MMR2);

            switch (slot)
            {
                case SLOT_2VS2_TOP:
                case SLOT_3VS3_TOP:
                {
                    highGames++;
                    if (highGames)
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, GetGamesStringData(member, highestMMR), GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_PLAYER + iter->second.team1);
                    break;
                }
                case SLOT_2VS2:
                case SLOT_3VS3:
                {
                    lowGames++;
                    if (lowGames)
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, GetGamesStringData(member, highestMMR), GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_PLAYER + iter->second.team1);
                    break;
                }
                default:
                    continue;
            }
        }

        if (!highGames && !lowGames)
        {
            player->SendChatMessage("Currently no teams queued");
            player->PlayerTalkClass->ClearMenus();
            player->CLOSE_GOSSIP_MENU();
        }

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Cancel", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_CANCEL);

        if (page > 0)
        {
            switch (slot)
            {
            case SLOT_2VS2:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Prev...", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_LIST_GAMES_2v2 + page - 1);
                break;
            case SLOT_2VS2_TOP:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Prev...", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_LIST_TOP_GAMES_2v2 + page - 1);
                break;
            case SLOT_3VS3:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Prev...", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_LIST_GAMES_3v3 + page - 1);
                break;
            case SLOT_3VS3_TOP:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Prev...", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_LIST_TOP_GAMES_3v3 + page - 1);
                break;
            default:
                break;
            }
        }

        if (haveNextPage)
        {
            switch (slot)
            {
            case SLOT_2VS2:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Next...", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_LIST_GAMES_2v2 + page + 1);
                break;
            case SLOT_2VS2_TOP:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Next...", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_LIST_TOP_GAMES_2v2 + page + 1);
                break;
            case SLOT_3VS3:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Next...", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_LIST_GAMES_3v3 + page + 1);
                break;
            case SLOT_3VS3_TOP:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Next...", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_LIST_TOP_GAMES_3v3 + page + 1);
                break;
            default:
                break;
            }
        }
    }
};

uint32 const tabardEntry[13] = { 38314, 38309, 38312, 38310, 23705, 23709, 38313, 38311, 36941, 28788, 45983, 49086, 51534 };
// Tabard vendor
class npc_arena_vendor : public CreatureScript
{
public:
    npc_arena_vendor() : CreatureScript("npc_arena_vendor"){ }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        player->PlayerTalkClass->ClearMenus();
        uint32 rating = player->GetArenaPersonalRating(1);

        if (rating >= 2350)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Wrathful Gladiator Tabard", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 13);
        if (rating >= 2200)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Relentless Gladiator Tabard", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12);
        if (rating >= 2100)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Furious Gladiator Tabard", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
        if (rating >= 2000)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Tabard of the protector", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
        if (rating >= 1800)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Competitors Tabard", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
        if (rating >= 1700)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Tabard of Void", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 8);
        if (rating >= 1600)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Tabard of Fury", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
        if (rating >= 1500)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Tabard of Frost", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
        if (rating >= 1400)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Tabard of Flame", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
        if (rating >= 1300)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Tabard of Arcane", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
        if (rating >= 1200)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Tabard of Brilliance", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
        if (rating >= 1100)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Tabard of Nature", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        if (rating >= 1000)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Tabard of the Defender", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        CurrencyTypes currency = CURRENCY_TYPE_HONOR_POINTS;
        if (action > GOSSIP_ACTION_INFO_DEF + 9)
            currency = CURRENCY_TYPE_CONQUEST_POINTS;
        uint32 points = player->GetCurrency(currency, false);
        if (points >= 50000)
        {
            if (player->AddItem(tabardEntry[action - (GOSSIP_ACTION_INFO_DEF + 1)], 1))
                player->ModifyCurrency(currency, -50000);
            else
                creature->MonsterWhisper("You don´t have enough space in your bags!", player->GetGUID());
        }
        else
            creature->MonsterWhisper("Not enough points! You need at least 500 honor/conquest points!", player->GetGUID());

        player->CLOSE_GOSSIP_MENU();
        return true;
    }
};

void AddSC_Skirmish_npc()
{
    new Skirmish_npc();
    new SaveNewCap();
    new arena_spectator_commands();
    new npc_arena_spectator();
    new npc_arena_vendor();
}