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

/* ScriptData
SDName: Stranglethorn_Vale
SD%Complete: 100
SDComment: Quest support: 592
SDCategory: Stranglethorn Vale
EndScriptData */

/* ContentData
mob_yenniku
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "Player.h"
#include "SpellInfo.h"

/*######
## mob_yenniku
######*/
/*
class mob_yenniku : public CreatureScript
{
public:
    mob_yenniku() : CreatureScript("mob_yenniku") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_yennikuAI (creature);
    }

    struct mob_yennikuAI : public ScriptedAI
    {
        mob_yennikuAI(Creature* creature) : ScriptedAI(creature) { }

        bool bReset;

        void Reset()
        {
            bReset = false;
            me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_NONE);
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (caster->GetTypeId() == TYPEID_PLAYER)
            {                                                                //Yenniku's Release
                if (!bReset && spell->Id == 3607 && caster->ToPlayer()->GetQuestStatus(26305) == QUEST_STATUS_INCOMPLETE)
                {
                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STUN);
                    me->CombatStop();                   //stop combat
                    me->DeleteThreatList();             //unsure of this
                    me->setFaction(83);                 //horde generic
                    caster->ToPlayer()->AddItem(3913, 1);
                    me->DespawnOrUnsummon(5000);
                    bReset = true;
                }
            }
        }
    };
};*/

/*######
## Quest: Spared from Madness - Entire hack. Whole the event needs scripting. @ToDo.
######*/

class go_kurzen_cage : public GameObjectScript 
{
public:
    go_kurzen_cage() : GameObjectScript("go_kurzen_cage") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        uint32 prisoners [] = {43921, 43920, 43919};
        go->UseDoorOrButton();
        for (uint8 i = 0; i < 3; ++i)
        {
            if (Creature* prisoner = go->FindNearestCreature(prisoners[i], 5.0f, true))
            {
                prisoner->DespawnOrUnsummon(2500);
                player->KilledMonsterCredit(prisoners[i], 0);
            }
        }
        return true;
    }
};

enum bwembaEvents
{
    EVENT_TALK = 1,
    EVENT_CHECK_SUMMONER,
};

// 96591
class mob_bwemba : public CreatureScript
{
public:
    mob_bwemba() : CreatureScript("mob_bwemba") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_bwembaAI (creature);
    }

    struct mob_bwembaAI : public ScriptedAI
    {
        mob_bwembaAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
        }

        void IsSummonedBy(Unit* summoner)
        {
            playerGUID = summoner->GetGUID();
            me->SetReactState(REACT_PASSIVE);
            events.ScheduleEvent(EVENT_CHECK_SUMMONER, 0);
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            switch (spell->Id)
            {
                case 54694:
                case 88529:
                case 56685:
                    if (talkId < 6)
                        events.ScheduleEvent(EVENT_TALK, 0);
                    break;
            }
        }

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_TALK:
                        Talk(talkId);
                        talkId++;
                        break;
                    case EVENT_CHECK_SUMMONER:
                    {
                        bool haveToDespawn = true;
                        if (Player *player = Unit::GetPlayer(*me, playerGUID))
                        {
                            uint32 questId = player->GetTeam() == HORDE ? 29250 : 29152;
                            if (player->IsInWorld() && player->GetMapId() == me->GetMapId() &&
                                player->GetQuestStatus(questId) != QUEST_STATUS_COMPLETE &&
                                player->GetQuestStatus(questId) != QUEST_STATUS_REWARDED)
                            {
                                haveToDespawn = false;
                                me->GetMotionMaster()->MoveFollow(player, 0.0f, 0.0f);
                            }
                        }
                        if (haveToDespawn)
                            me->DespawnOrUnsummon(1000);
                        else
                            events.ScheduleEvent(EVENT_CHECK_SUMMONER, 1000);
                        break;
                    }
                }
            }
        }
    private:
        EventMap events;
        uint8 talkId;
        uint64 playerGUID;
    };
};

void AddSC_stranglethorn_vale()
{
    // new mob_yenniku(); // Handled through SAI
    new go_kurzen_cage();
    new mob_bwemba();
}
