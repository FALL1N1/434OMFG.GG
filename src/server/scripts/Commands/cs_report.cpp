/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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
Name: quest_commandscript
%Complete: 100
Comment: All quest related commands
Category: commandscripts
EndScriptData */

#include "Chat.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ReputationMgr.h"
#include "ScriptMgr.h"

class report_commandscript : public CommandScript
{
public:
    report_commandscript() : CommandScript("report_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> reportCommandTable =
        {
            { "quest",          SEC_PLAYER,         false, &HandleReportQuest,                 "" },
            { "complete",       SEC_ADMINISTRATOR,  false, &HandleReportComplete,               "" },
            { "remove",         SEC_ADMINISTRATOR,  false, &HandleReportRemove,                 "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "report",         SEC_PLAYER,         false, NULL,                  "", reportCommandTable },
        };
        return commandTable;
    }

    static bool HandleReportQuest(ChatHandler* handler, const char* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!args)
            return false;



        // number or [name] Shift-click form |color|Hquest:quest_id:quest_level|h[name]|h|r
        char* cId = handler->extractKeyFromLink((char*)args, "Hquest");
        if (!cId)
            return false;

        uint32 entry = atol(cId);

        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);

        // If player doesn't have the quest
        if (!quest || player->GetQuestStatus(entry) == QUEST_STATUS_NONE)
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->GetQuestStatus(entry) == QUEST_STATUS_COMPLETE)
        {
            handler->SendSysMessage("quest already complete");
            return true;
        }

        uint32 status = 0;
        uint32 count = 0;
        sObjectMgr->GetReportQuestStatuCount(entry, status, count);
        if (!player->HaveReportQuest(entry))
        {
            count++;
            sObjectMgr->ModifyReportQuestData(entry, status, count);
        }

        if (player->HaveReportQuest(entry) && status == 0)
        {
            handler->SendSysMessage("you have already reported this quest");
            return true;
        }

        switch (status)
        {
            case 0:
                handler->SendSysMessage("thanks for this report but this quest is not yet verified by the staff, please open a ticket");
                break;
            case 1:
                if (!ValidQuest(quest, player, handler, entry))
                    return false;
                handler->SendSysMessage("quest complete succesfully, thanks for the report");
                break;
            case 2:
                handler->SendSysMessage("this quest is working as intended.");
                break;
            default:
                break;
        }
        player->ReportQuest(entry);
        return true;
    }

    static bool HandleReportComplete(ChatHandler* handler, const char* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hquest:quest_id:quest_level|h[name]|h|r
        char* cId = handler->extractKeyFromLink((char*)args, "Hquest");
        if (!cId)
            return false;

        uint32 entry = atol(cId);

        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);

        if (!quest)
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        sObjectMgr->ModifyReportQuestData(entry, 1, 0);
        handler->SendSysMessage("quest status change as buggy, will be autocomplete on report");
        return true;
    }

    static bool HandleReportRemove(ChatHandler* handler, const char* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hquest:quest_id:quest_level|h[name]|h|r
        char* cId = handler->extractKeyFromLink((char*)args, "Hquest");
        if (!cId)
            return false;

        uint32 entry = atol(cId);

        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);

        if (!quest)
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        sObjectMgr->ModifyReportQuestData(entry, 2, 0);
        handler->SendSysMessage("quest status change as not buggy, will never be complete on report");
        return true;
    }

    static bool ValidQuest(Quest const* quest, Player *player, ChatHandler* /*handler*/, uint32 entry)
    {
        // Add quest items for quests that require items
        for (uint8 x = 0; x < QUEST_ITEM_OBJECTIVES_COUNT; ++x)
        {
            uint32 id = quest->RequiredItemId[x];
            uint32 count = quest->RequiredItemCount[x];
            if (!id || !count)
                continue;

            uint32 curItemCount = player->GetItemCount(id, true);

            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, id, count-curItemCount);
            if (msg == EQUIP_ERR_OK)
            {
                Item* item = player->StoreNewItem(dest, id, true);
                player->SendNewItem(item, count-curItemCount, true, false);
            }
        }

        // All creature/GO slain/casted (not required, but otherwise it will display "Creature slain 0/10")
        for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        {
            int32 creature = quest->RequiredNpcOrGo[i];
            uint32 creaturecount = quest->RequiredNpcOrGoCount[i];

            if (uint32 spell_id = quest->RequiredSpellCast[i])
            {
                for (uint16 z = 0; z < creaturecount; ++z)
                    player->CastedCreatureOrGO(creature, 0, spell_id);
            }
            else if (creature > 0)
            {
                if (CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(creature))
                    for (uint16 z = 0; z < creaturecount; ++z)
                        player->KilledMonster(cInfo, 0);
            }
            else if (creature < 0)
            {
                for (uint16 z = 0; z < creaturecount; ++z)
                    player->CastedCreatureOrGO(creature, 0, 0);
            }
        }

        // If the quest requires reputation to complete
        if (uint32 repFaction = quest->GetRepObjectiveFaction())
        {
            uint32 repValue = quest->GetRepObjectiveValue();
            uint32 curRep = player->GetReputationMgr().GetReputation(repFaction);
            if (curRep < repValue)
                if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
                    player->GetReputationMgr().SetReputation(factionEntry, repValue);
        }

        // If the quest requires a SECOND reputation to complete
        if (uint32 repFaction = quest->GetRepObjectiveFaction2())
        {
            uint32 repValue2 = quest->GetRepObjectiveValue2();
            uint32 curRep = player->GetReputationMgr().GetReputation(repFaction);
            if (curRep < repValue2)
                if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
                    player->GetReputationMgr().SetReputation(factionEntry, repValue2);
        }

        // If the quest requires money
        int32 ReqOrRewMoney = quest->GetRewOrReqMoney();
        if (ReqOrRewMoney < 0)
            player->ModifyMoney(-ReqOrRewMoney);

        player->CompleteQuest(entry);
        return true;
    }
};

void AddSC_report_commandscript()
{
    new report_commandscript();
}
