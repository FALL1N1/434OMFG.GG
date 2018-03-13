/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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
SDName: Sunwell_Plateau
SD%Complete: 0
SDComment: Placeholder, Epilogue after Kil'jaeden, Captain Selana Gossips
EndScriptData */

/* ContentData
npc_prophet_velen
npc_captain_selana
EndContentData */

#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "sunwell_plateau.h"

/*######
## npc_prophet_velen
######*/

enum ProphetSpeeches
{
    PROPHET_SAY1 = -1580099,
    PROPHET_SAY2 = -1580100,
    PROPHET_SAY3 = -1580101,
    PROPHET_SAY4 = -1580102,
    PROPHET_SAY5 = -1580103,
    PROPHET_SAY6 = -1580104,
    PROPHET_SAY7 = -1580105,
    PROPHET_SAY8 = -1580106
};

enum LiadrinnSpeeches
{
    LIADRIN_SAY1 = -1580107,
    LIADRIN_SAY2 = -1580108,
    LIADRIN_SAY3 = -1580109
};

/*######
## npc_captain_selana
######*/

#define CS_GOSSIP1 "Give me a situation report, Captain."
#define CS_GOSSIP2 "What went wrong?"
#define CS_GOSSIP3 "Why did they stop?"
#define CS_GOSSIP4 "Your insight is appreciated."

enum Says
{
    SAY_QUELDELAR_1  = 0,

    // 37781
    SAY_QUELDELAR_2  = 0,
    SAY_QUELDELAR_7  = 1,

    // pTheron
    SAY_QUELDELAR_3  = 0,
    SAY_QUELDELAR_5  = 1,

    // pRommath
    SAY_QUELDELAR_4  = 0,
    SAY_QUELDELAR_6  = 1,
    SAY_QUELDELAR_10 = 2,
    SAY_QUELDELAR_11 = 3,

    // pAuric
    SAY_QUELDELAR_8  = 0,
    SAY_QUELDELAR_9  = 1,
    SAY_QUELDELAR_12 = 2,
};
class npc_queldelar_sp : public CreatureScript
{
public:
    npc_queldelar_sp() : CreatureScript("npc_queldelar_sp") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_queldelar_spAI(creature);
    }

        bool OnGossipHello(Player* player, Creature* creature)
        {
            player->PrepareGossipMenu(creature, 0);

            if (player->HasItemCount(49879, 1) || player->HasItemCount(49889, 1))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "He brought Quel'delar.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->SendPreparedGossip(creature);

            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
        {
            player->PlayerTalkClass->ClearMenus();

            switch(uiAction)
            {
                case GOSSIP_ACTION_INFO_DEF+1:
                    player->CLOSE_GOSSIP_MENU();
                    creature->AI()->SetGUID(player->GetGUID());
                    creature->AI()->DoAction(1);
                    break;
                default:
                    return false;
            }

            return true;
        }

    struct npc_queldelar_spAI : public ScriptedAI
    {
        npc_queldelar_spAI(Creature* c) : ScriptedAI(c) {}

        uint64 PlayerGUID;

        EventMap events;
        uint64 uiRommath;
        uint64 uiTheron;
        uint64 uiAuric;
        uint64 uiQuelDelar;
        uint64 uiPlayer;
        bool StartQuest;

        void Reset()
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            StartQuest = true;
            events.Reset();
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            switch(events.ExecuteEvent())
            {
                case 1:
                    if(Creature* pRommath = me->FindNearestCreature(37763, 50, true))
                    {
                        pRommath->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                        uiRommath = pRommath->GetGUID();
                    }
                    if(Creature* pTheron = me->FindNearestCreature(37764, 50, true))
                    {
                        pTheron->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                        uiTheron = pTheron->GetGUID();
                    }
                    if(Creature* pAuric = me->FindNearestCreature(37765, 50, true))
                    {
                        pAuric->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                        uiAuric = pAuric->GetGUID();
                    }
                    if(GameObject* gQuelDelar = me->SummonGameObject(201794, 1683.99f, 620.231f, 29.3599f, 0.410932f, 0, 0, 0, 0, 0))
                    {
                        uiQuelDelar = gQuelDelar->GetGUID();
                        gQuelDelar->SetFlag(GAMEOBJECT_FLAGS, 5);
                    }
                    if (Player* player = me->GetPlayer(*me, uiPlayer))
                    {
                        player->DestroyItemCount(49879, 1, true);
                        player->DestroyItemCount(49889, 1, true);
                        Talk(SAY_QUELDELAR_1, player->GetGUID());
                    }
                    events.ScheduleEvent(2, 2000);
                    break;
                case 2:
                    if(Creature* pGuard = me->FindNearestCreature(37781, 20, true))
                        pGuard->AI()->Talk(SAY_QUELDELAR_2);
                    events.ScheduleEvent(3, 1000);
                    break;
                case 3:
                    if(Creature* pTheron = me->GetCreature(*me, uiTheron))
                        pTheron->AI()->Talk(SAY_QUELDELAR_3);
                    events.ScheduleEvent(4, 4000);
                    break;
                case 4:
                    if(Creature* pRommath = me->GetCreature(*me, uiRommath))
                        pRommath->GetMotionMaster()->MovePoint(1, 1675.8f, 617.19f, 28.0504f);
                    if(Creature* pAuric = me->GetCreature(*me, uiAuric))
                        pAuric->GetMotionMaster()->MovePoint(1, 1681.77f, 612.084f, 28.4409f);
                    events.ScheduleEvent(5, 6000);
                    break;
                case 5:
                    if(Creature* pRommath = me->GetCreature(*me, uiRommath))
                    {
                        pRommath->SetOrientation(0.3308f);
                        pRommath->AI()->Talk(SAY_QUELDELAR_4);
                    }
                    if(Creature* pAuric = me->GetCreature(*me, uiAuric))
                        pAuric->SetOrientation(1.29057f);
                    if(Creature* pTheron = me->GetCreature(*me, uiTheron))
                        pTheron->GetMotionMaster()->MovePoint(1, 1677.07f, 613.122f, 28.0504f);
                    events.ScheduleEvent(6, 10000);
                    break;
                case 6:
                    if(Creature* pTheron = me->GetCreature(*me, uiTheron))
                    {
                        if(Player* player = me->GetPlayer(*me, uiPlayer))
                            pTheron->AI()->Talk(SAY_QUELDELAR_5, player->GetGUID());
                        pTheron->GetMotionMaster()->MovePoint(1, 1682.3f, 618.459f, 27.9581f);
                    }
                    events.ScheduleEvent(7, 4000);
                    break;
                case 7:
                    if(Creature* pTheron = me->GetCreature(*me, uiTheron))
                        pTheron->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                    events.ScheduleEvent(8, 800);
                    break;
                case 8:
                    if(Creature* pTheron = me->GetCreature(*me, uiTheron))
                        pTheron->CastSpell(pTheron, 70493, true);
                    events.ScheduleEvent(9, 1000);
                    break;
                case 9:
                    if(Creature* pRommath = me->GetCreature(*me, uiRommath))
                    {
                        if(Player* player = me->GetPlayer(*me, uiPlayer))
                        {
                            pRommath->AddAura(70540, player);
                        }
                        pRommath->AI()->Talk(SAY_QUELDELAR_6);
                    }
                    if(Creature* pGuard = me->FindNearestCreature(37781, 20))
                    {
                        pGuard->GetMotionMaster()->MovePoint(0, 1681.1f, 614.955f, 28.4983f);
                        pGuard->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_READY1H);
                    }
                    events.ScheduleEvent(10, 3000);
                    break;
                case 10:
                    if(Creature* pGuard = me->FindNearestCreature(37781, 20))
                        pGuard->AI()->Talk(SAY_QUELDELAR_7);
                    events.ScheduleEvent(11, 2000);
                    break;
                case 11:
                    if(Creature* pAuric = me->GetCreature(*me, uiAuric))
                        pAuric->AI()->Talk(SAY_QUELDELAR_8);
                    events.ScheduleEvent(12, 6000);
                    break;
                case 12:
                    if(Creature* pAuric = me->GetCreature(*me, uiAuric))
                        pAuric->AI()->Talk(SAY_QUELDELAR_9);
                    events.ScheduleEvent(13, 5000);
                    break;
                case 13:
                    if(Creature* pRommath = me->GetCreature(*me, uiRommath))
                        pRommath->AI()->Talk(SAY_QUELDELAR_10);
                    events.ScheduleEvent(14, 2000);
                    break;
                case 14:
                    if(Creature* pGuard = me->FindNearestCreature(37781, 20))
                    {
                        pGuard->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STAND);
                        pGuard->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                        pGuard->GetMotionMaster()->MovePoint(0, pGuard->GetHomePosition());
                    }
                    if(Creature* pRommath = me->GetCreature(*me, uiRommath))
                    {
                        pRommath->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                        if(Player* player = me->GetPlayer(*me, uiPlayer))
                            pRommath->AI()->Talk(SAY_QUELDELAR_11, player->GetGUID());
                    }
                    events.ScheduleEvent(15, 7000);
                    break;
                case 15:
                    if(Creature* pAuric = me->GetCreature(*me, uiAuric))
                    {
                        pAuric->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                        if(Player* player = me->GetPlayer(*me, uiPlayer))
                            pAuric->AI()->Talk(SAY_QUELDELAR_12, player->GetGUID());
                        if(GameObject* gQuelDelar = me->FindNearestGameObject(201794, 20))
                            gQuelDelar->RemoveFlag(GAMEOBJECT_FLAGS, 5);
                    }
                    events.ScheduleEvent(16, 2000);
                    break;
                case 16:
                     if(Creature* pAuric = me->GetCreature(*me, uiAuric))
                         pAuric->GetMotionMaster()->MovePoint(0, pAuric->GetHomePosition());
                     if(Creature* pRommath = me->GetCreature(*me, uiRommath))
                         pRommath->GetMotionMaster()->MovePoint(0, pRommath->GetHomePosition());
                     if(Creature* pTheron = me->GetCreature(*me, uiTheron))
                         pTheron->GetMotionMaster()->MovePoint(0, pTheron->GetHomePosition());
                    break;
            }
        }

        void SetGUID(uint64 guid, int32 /*id*/)
        {
            uiPlayer = guid;
        }

        void DoAction(const int32 actionId)
        {
            switch(actionId)
            {
                case 1:
                    if(StartQuest)
                    {
                        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        events.ScheduleEvent(1,0);
                        StartQuest = false;
                    }
                    break;
            }
        }
    };

};

class go_dalaran_portal : public GameObjectScript
{
    public:

        go_dalaran_portal()
            : GameObjectScript("go_dalaran_portal_sunwell")
        {
        }

        bool OnGossipHello(Player* player, GameObject* /*go*/)
        {
            player->SetPhaseMask(1, true);
            player->TeleportTo(571, 5804.15f, 624.771f, 647.767f, 1.64f);
            return false;
        }
};

class item_tainted_queldelar : public ItemScript
{
    public:

        item_tainted_queldelar()
            : ItemScript("item_tainted_queldelar")
        {
        }
        bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/)
        {
            InstanceScript *instance = player->GetInstanceScript();

            if (instance && player->FindNearestCreature(37746, 18.0f, true))
            {
                Creature *Introducer = NULL;
                Introducer = Unit::GetCreature((*player), instance->GetData64(DATA_QUELDELAR_INTRODUCER));
                Introducer->AI()->SetGUID(player->GetGUID());
                Introducer->AI()->DoAction(1);
                return true;
            }
            else
                return false;
        }
};

void AddSC_sunwell_plateau()
{
    new npc_queldelar_sp();
    new go_dalaran_portal();
    new item_tainted_queldelar();
}
