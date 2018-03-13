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
SDName: The_Barrens
SD%Complete: 90
SDComment: Quest support: 863
SDCategory: Barrens
EndScriptData */

/* ContentData
npc_wizzlecrank_shredder
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"
#include "Player.h"
#include "SpellInfo.h"

/*######
## npc_beaten_corpse
######*/

#define GOSSIP_CORPSE "Examine corpse in detail..."

enum BeatenCorpse
{
    QUEST_LOST_IN_BATTLE    = 4921
};

class npc_beaten_corpse : public CreatureScript
{
public:
    npc_beaten_corpse() : CreatureScript("npc_beaten_corpse") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF +1)
        {
            player->SEND_GOSSIP_MENU(3558, creature->GetGUID());
            player->TalkedToCreature(creature->GetEntry(), creature->GetGUID());
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (player->GetQuestStatus(QUEST_LOST_IN_BATTLE) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(QUEST_LOST_IN_BATTLE) == QUEST_STATUS_COMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_CORPSE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        player->SEND_GOSSIP_MENU(3557, creature->GetGUID());
        return true;
    }

};

/*######
# npc_gilthares
######*/

enum Gilthares
{
    SAY_GIL_START               = 0,
    SAY_GIL_AT_LAST             = 1,
    SAY_GIL_PROCEED             = 2,
    SAY_GIL_FREEBOOTERS         = 3,
    SAY_GIL_AGGRO               = 4,
    SAY_GIL_ALMOST              = 5,
    SAY_GIL_SWEET               = 6,
    SAY_GIL_FREED               = 7,

    QUEST_FREE_FROM_HOLD        = 898,
    AREA_MERCHANT_COAST         = 391,
    FACTION_ESCORTEE            = 232                       //guessed, possible not needed for this quest
};

class npc_gilthares : public CreatureScript
{
public:
    npc_gilthares() : CreatureScript("npc_gilthares") { }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_FREE_FROM_HOLD)
        {
            creature->setFaction(FACTION_ESCORTEE);
            creature->SetStandState(UNIT_STAND_STATE_STAND);

            creature->AI()->Talk(SAY_GIL_START, player->GetGUID());

            if (npc_giltharesAI* pEscortAI = CAST_AI(npc_gilthares::npc_giltharesAI, creature->AI()))
                pEscortAI->Start(false, false, player->GetGUID(), quest);
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_giltharesAI(creature);
    }

    struct npc_giltharesAI : public npc_escortAI
    {
        npc_giltharesAI(Creature* creature) : npc_escortAI(creature) { }

        void Reset() { }

        void WaypointReached(uint32 waypointId)
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (waypointId)
            {
                case 16:
                    Talk(SAY_GIL_AT_LAST, player->GetGUID());
                    break;
                case 17:
                    Talk(SAY_GIL_PROCEED, player->GetGUID());
                    break;
                case 18:
                    Talk(SAY_GIL_FREEBOOTERS, player->GetGUID());
                    break;
                case 37:
                    Talk(SAY_GIL_ALMOST, player->GetGUID());
                    break;
                case 47:
                    Talk(SAY_GIL_SWEET, player->GetGUID());
                    break;
                case 53:
                    Talk(SAY_GIL_FREED, player->GetGUID());
                    player->GroupEventHappens(QUEST_FREE_FROM_HOLD, me);
                    break;
            }
        }

        void EnterCombat(Unit* who)
        {
            //not always use
            if (rand()%4)
                return;

            //only aggro text if not player and only in this area
            if (who->GetTypeId() != TYPEID_PLAYER && me->GetAreaId() == AREA_MERCHANT_COAST)
            {
                //appears to be pretty much random (possible only if escorter not in combat with who yet?)
                Talk(SAY_GIL_AGGRO, who->GetGUID());
            }
        }
    };

};

/*######
## npc_sputtervalve
######*/

#define GOSSIP_SPUTTERVALVE "Can you tell me about this shard?"

class npc_sputtervalve : public CreatureScript
{
public:
    npc_sputtervalve() : CreatureScript("npc_sputtervalve") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF)
        {
            player->SEND_GOSSIP_MENU(2013, creature->GetGUID());
            player->AreaExploredOrEventHappens(6981);
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(6981) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SPUTTERVALVE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

};

/*######
## npc_taskmaster_fizzule
######*/

enum TaskmasterFizzule
{
    FACTION_FRIENDLY_F  = 35,
    SPELL_FLARE         = 10113,
    SPELL_FOLLY         = 10137,
};

class npc_taskmaster_fizzule : public CreatureScript
{
public:
    npc_taskmaster_fizzule() : CreatureScript("npc_taskmaster_fizzule") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_taskmaster_fizzuleAI(creature);
    }

    struct npc_taskmaster_fizzuleAI : public ScriptedAI
    {
        npc_taskmaster_fizzuleAI(Creature* creature) : ScriptedAI(creature)
        {
            factionNorm = creature->getFaction();
        }

        uint32 factionNorm;
        bool IsFriend;
        uint32 ResetTimer;
        uint8 FlareCount;

        void Reset()
        {
            IsFriend = false;
            ResetTimer = 120000;
            FlareCount = 0;
            me->setFaction(factionNorm);
        }

        void DoFriend()
        {
            me->RemoveAllAuras();
            me->DeleteThreatList();
            me->CombatStop(true);

            me->StopMoving();
            me->GetMotionMaster()->MoveIdle();

            me->setFaction(FACTION_FRIENDLY_F);
            me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
        }

        void SpellHit(Unit* /*caster*/, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_FLARE || spell->Id == SPELL_FOLLY)
            {
                ++FlareCount;

                if (FlareCount >= 2)
                    IsFriend = true;
            }
        }

        void EnterCombat(Unit* /*who*/) {}

        void UpdateAI(const uint32 diff)
        {
            if (IsFriend)
            {
                if (ResetTimer <= diff)
                {
                    EnterEvadeMode();
                    return;
                } else ResetTimer -= diff;
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

        void ReceiveEmote(Player* /*player*/, uint32 emote)
        {
            if (emote == TEXT_EMOTE_SALUTE)
            {
                if (FlareCount >= 2)
                {
                    if (me->getFaction() == FACTION_FRIENDLY_F)
                        return;

                    DoFriend();
                }
            }
        }
    };

};

/*#####
## npc_twiggy_flathead
#####*/

enum TwiggyFlathead
{
    NPC_BIG_WILL                = 6238,
    NPC_AFFRAY_CHALLENGER       = 6240,

    SAY_BIG_WILL_READY          = 0,
    SAY_TWIGGY_FLATHEAD_BEGIN   = 0,
    SAY_TWIGGY_FLATHEAD_FRAY    = 1,
    SAY_TWIGGY_FLATHEAD_DOWN    = 2,
    SAY_TWIGGY_FLATHEAD_OVER    = 3
};

Position const AffrayChallengerLoc[6] =
{
    {-1683.0f, -4326.0f, 2.79f, 0.0f},
    {-1682.0f, -4329.0f, 2.79f, 0.0f},
    {-1683.0f, -4330.0f, 2.79f, 0.0f},
    {-1680.0f, -4334.0f, 2.79f, 1.49f},
    {-1674.0f, -4326.0f, 2.79f, 3.49f},
    {-1677.0f, -4334.0f, 2.79f, 1.66f}
};

class npc_twiggy_flathead : public CreatureScript
{
public:
    npc_twiggy_flathead() : CreatureScript("npc_twiggy_flathead") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_twiggy_flatheadAI (creature);
    }

    struct npc_twiggy_flatheadAI : public ScriptedAI
    {
        npc_twiggy_flatheadAI(Creature* creature) : ScriptedAI(creature) {}

        bool EventInProgress;
        bool EventGrate;
        bool EventBigWill;
        bool ChallengerDown[6];
        uint8 Wave;
        uint32 WaveTimer;
        uint32 ChallengerChecker;
        uint64 PlayerGUID;
        uint64 AffrayChallenger[6];
        uint64 BigWill;

        void Reset()
        {
            EventInProgress = false;
            EventGrate = false;
            EventBigWill = false;
            WaveTimer = 600000;
            ChallengerChecker = 0;
            Wave = 0;
            PlayerGUID = 0;

            for (uint8 i = 0; i < 6; ++i)
            {
                AffrayChallenger[i] = 0;
                ChallengerDown[i] = false;
            }
            BigWill = 0;
        }

        void EnterCombat(Unit* /*who*/) { }

        void MoveInLineOfSight(Unit* who)
        {
            if (!who || (!who->isAlive()))
                return;

            if (me->IsWithinDistInMap(who, 10.0f) && (who->GetTypeId() == TYPEID_PLAYER) && CAST_PLR(who)->GetQuestStatus(1719) == QUEST_STATUS_INCOMPLETE && !EventInProgress)
            {
                PlayerGUID = who->GetGUID();
                EventInProgress = true;
            }
        }

        void KilledUnit(Unit* /*victim*/) { }

        void UpdateAI(const uint32 diff)
        {
            if (EventInProgress) {
                Player* pWarrior = NULL;

                if (PlayerGUID)
                    pWarrior = Unit::GetPlayer(*me, PlayerGUID);

                if (!pWarrior)
                    return;

                if (!pWarrior->isAlive() && pWarrior->GetQuestStatus(1719) == QUEST_STATUS_INCOMPLETE) {
                    Talk(SAY_TWIGGY_FLATHEAD_DOWN);
                    pWarrior->FailQuest(1719);

                    for (uint8 i = 0; i < 6; ++i) // unsummon challengers
                    {
                        if (AffrayChallenger[i])
                        {
                            Creature* creature = Unit::GetCreature((*me), AffrayChallenger[i]);
                            if (creature && creature->isAlive())
                                creature->DisappearAndDie();
                        }
                    }

                    if (BigWill) // unsummon bigWill
                    {
                        Creature* creature = Unit::GetCreature((*me), BigWill);
                        if (creature && creature->isAlive())
                            creature->DisappearAndDie();
                    }
                    Reset();
                }

                if (!EventGrate && EventInProgress)
                {
                    float x, y, z;
                    pWarrior->GetPosition(x, y, z);

                    if (x >= -1684 && x <= -1674 && y >= -4334 && y <= -4324) {
                        pWarrior->AreaExploredOrEventHappens(1719);
                        Talk(SAY_TWIGGY_FLATHEAD_BEGIN, pWarrior->GetGUID());

                        for (uint8 i = 0; i < 6; ++i)
                        {
                            Creature* creature = me->SummonCreature(NPC_AFFRAY_CHALLENGER, AffrayChallengerLoc[i], TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 600000);
                            if (!creature)
                                continue;
                            creature->setFaction(35);
                            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            creature->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
                            AffrayChallenger[i] = creature->GetGUID();
                        }
                        WaveTimer = 5000;
                        ChallengerChecker = 1000;
                        EventGrate = true;
                    }
                }
                else if (EventInProgress)
                {
                    if (ChallengerChecker <= diff)
                    {
                        for (uint8 i = 0; i < 6; ++i)
                        {
                            if (AffrayChallenger[i])
                            {
                                Creature* creature = Unit::GetCreature((*me), AffrayChallenger[i]);
                                if ((!creature || (!creature->isAlive())) && !ChallengerDown[i])
                                {
                                    Talk(SAY_TWIGGY_FLATHEAD_DOWN);
                                    ChallengerDown[i] = true;
                                }
                            }
                        }
                        ChallengerChecker = 1000;
                    } else ChallengerChecker -= diff;

                    if (WaveTimer <= diff)
                    {
                        if (Wave < 6 && AffrayChallenger[Wave] && !EventBigWill)
                        {
                            Talk(SAY_TWIGGY_FLATHEAD_FRAY);
                            Creature* creature = Unit::GetCreature((*me), AffrayChallenger[Wave]);
                            if (creature && (creature->isAlive()))
                            {
                                creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                                creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                                creature->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
                                creature->setFaction(14);
                                creature->AI()->AttackStart(pWarrior);
                                ++Wave;
                                WaveTimer = 20000;
                            }
                        }
                        else if (Wave >= 6 && !EventBigWill) {
                            if (Creature* creature = me->SummonCreature(NPC_BIG_WILL, -1722, -4341, 6.12f, 6.26f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 480000))
                            {
                                BigWill = creature->GetGUID();
                                //creature->GetMotionMaster()->MovePoint(0, -1693, -4343, 4.32f);
                                //creature->GetMotionMaster()->MovePoint(1, -1684, -4333, 2.78f);
                                creature->GetMotionMaster()->MovePoint(2, -1682, -4329, 2.79f);
                                creature->HandleEmoteCommand(EMOTE_STATE_READY_UNARMED);
                                EventBigWill = true;
                                WaveTimer = 1000;
                            }
                        }
                        else if (Wave >= 6 && EventBigWill && BigWill)
                        {
                            Creature* creature = Unit::GetCreature((*me), BigWill);
                            if (!creature || !creature->isAlive())
                            {
                                Talk(SAY_TWIGGY_FLATHEAD_OVER);
                                Reset();
                            }
                        }
                    } else WaveTimer -= diff;
                }
            }
        }
    };

};

/*#####
## npc_wizzlecrank_shredder
#####*/

enum Wizzlecrank
{
    SAY_MERCENARY       = 0,
    SAY_START           = 0,
    SAY_STARTUP1        = 1,
    SAY_STARTUP2        = 2,
    SAY_PROGRESS_1      = 3,
    SAY_PROGRESS_2      = 4,
    SAY_PROGRESS_3      = 5,
    SAY_END             = 6,

    QUEST_ESCAPE        = 863,
    FACTION_RATCHET     = 637,
    NPC_PILOT_WIZZ      = 3451,
    NPC_MERCENARY       = 3282,
};

class npc_wizzlecrank_shredder : public CreatureScript
{
public:
    npc_wizzlecrank_shredder() : CreatureScript("npc_wizzlecrank_shredder") { }

    struct npc_wizzlecrank_shredderAI : public npc_escortAI
    {
        npc_wizzlecrank_shredderAI(Creature* creature) : npc_escortAI(creature)
        {
            IsPostEvent = false;
            PostEventTimer = 1000;
            PostEventCount = 0;
        }

        bool IsPostEvent;
        uint32 PostEventTimer;
        uint32 PostEventCount;

        void Reset()
        {
            if (!HasEscortState(STATE_ESCORT_ESCORTING))
            {
                if (me->getStandState() == UNIT_STAND_STATE_DEAD)
                     me->SetStandState(UNIT_STAND_STATE_STAND);

                IsPostEvent = false;
                PostEventTimer = 1000;
                PostEventCount = 0;
            }
        }

        void WaypointReached(uint32 waypointId)
        {
            switch (waypointId)
            {
                case 0:
                    Talk(SAY_STARTUP1);
                    break;
                case 9:
                    SetRun(false);
                    break;
                case 17:
                    if (Creature* temp = me->SummonCreature(NPC_MERCENARY, 1128.489f, -3037.611f, 92.701f, 1.472f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000))
                    {
                        temp->AI()->Talk(SAY_MERCENARY);
                        me->SummonCreature(NPC_MERCENARY, 1160.172f, -2980.168f, 97.313f, 3.690f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                    }
                    break;
                case 24:
                    IsPostEvent = true;
                    break;
            }
        }

        void WaypointStart(uint32 PointId)
        {
            Player* player = GetPlayerForEscort();

            if (!player)
                return;

            switch (PointId)
            {
                case 9:
                    Talk(SAY_STARTUP2, player->GetGUID());
                    break;
                case 18:
                    Talk(SAY_PROGRESS_1, player->GetGUID());
                    SetRun();
                    break;
            }
        }

        void JustSummoned(Creature* summoned)
        {
            if (summoned->GetEntry() == NPC_PILOT_WIZZ)
                me->SetStandState(UNIT_STAND_STATE_DEAD);

            if (summoned->GetEntry() == NPC_MERCENARY)
                summoned->AI()->AttackStart(me);
        }

        void UpdateEscortAI(const uint32 Diff)
        {
            if (!UpdateVictim())
            {
                if (IsPostEvent)
                {
                    if (PostEventTimer <= Diff)
                    {
                        switch (PostEventCount)
                        {
                            case 0:
                                Talk(SAY_PROGRESS_2);
                                break;
                            case 1:
                                Talk(SAY_PROGRESS_3);
                                break;
                            case 2:
                                Talk(SAY_END);
                                break;
                            case 3:
                                if (Player* player = GetPlayerForEscort())
                                {
                                    player->GroupEventHappens(QUEST_ESCAPE, me);
                                    me->SummonCreature(NPC_PILOT_WIZZ, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 180000);
                                }
                                break;
                        }

                        ++PostEventCount;
                        PostEventTimer = 5000;
                    }
                    else
                        PostEventTimer -= Diff;
                }

                return;
            }

            DoMeleeAttackIfReady();
        }
    };

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_ESCAPE)
        {
            creature->setFaction(FACTION_RATCHET);
            if (npc_escortAI* pEscortAI = CAST_AI(npc_wizzlecrank_shredder::npc_wizzlecrank_shredderAI, creature->AI()))
                pEscortAI->Start(true, false, player->GetGUID());
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_wizzlecrank_shredderAI(creature);
    }

};
/*
class npc_stonetalon_prisoner : public CreatureScript
{
public:
    npc_stonetalon_prisoner() : CreatureScript("npc_stonetalon_prisoner") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        if (player->GetQuestStatus(24512) == QUEST_STATUS_INCOMPLETE)
            if (player->HasItemCount(49769, 1))
            {
                player->CLOSE_GOSSIP_MENU();
                player->DestroyItemCount(49769, 1, true);
                player->KilledMonsterCredit(37176, 0);
                creature->DespawnOrUnsummon();
                return true;
            }
        return false;
    }
};*/

enum BulletQuest
{
    SPELL_RIDE_VEHICLE_HARDCODED    = 46598,
    SPELL_ROCKET_TRAIL              = 96840,
    SPELL_ROCKET_LOOP_SOUND         = 97203,
    SPELL_COSMETIC_EXPLOSION        = 60081,
    SPELL_SMOKE_CLOUD               = 78608,
    SPELL_IMPACT_EXPLOSION          = 97206,
    SPELL_QUEST_COMPLETE            = 96846,
    SPELL_EJECT_ALL_PASSENGERS      = 79737,

    EVENT_TIMER_3                   = 1,
    EVENT_TIMER_2                   = 2,
    EVENT_TIMER_1                   = 3,
    EVENT_START_PATH                = 4,

    BROADCAST_START_IN_3            = -51250,
    BROADCAST_START_IN_2            = -51251,
    BROADCAST_START_IN_1            = -51252,
    BROADCAST_GO                    = -51253,

    TALK_GYROTILT_WOW_CANT          = -51020,
    TALK_GYROTILT_MAN_OH            = -51021,
    TALK_GYROTILT_MY_DREAM          = -51023,
    TALK_GYROTILT_WELL_HERE         = -51031,
    TALK_GYROTILT_HUH_HOLD          = -51024,
    TALK_GYROTILT_WHOA_WHAT         = -51025,
    TALK_GYROTILT_NO_NO             = -51026,
    TALK_GYROTILT_AUGH_LOOKS        = -51027,
    TALK_GYROTILT_MAYBE_IF          = -51028,

    NPC_GYROTILT                    = 52204,
    NPC_SILVER_BULLET_VEHICLE       = 52201,

    QUEST_THE_SHORT_WAY_HOME        = 29094,
};

uint32 const rocketPathSize = 45;
const G3D::Vector3 rocketPath[rocketPathSize] =
{
    { 1137.208f, -3358.285f, 94.94839f },
    { 1125.748f, -3337.571f, 110.3914f },
    { 1094.142f, -3289.276f, 124.2637f },
    { 961.8195f, -3097.875f, 145.8200f },
    { 896.1893f, -3013.163f, 142.6455f },
    { 859.5625f, -3003.630f, 137.0063f },
    { 764.2969f, -3013.049f, 137.0063f },
    { 390.8420f, -3015.238f, 137.0063f },
    { 301.5608f, -3008.071f, 137.0063f },
    { 220.5521f, -2933.229f, 137.0063f },
    { 162.9410f, -2891.238f, 137.0063f },
    { 23.96007f, -2733.054f, 137.0063f },
    { -95.98959f, -2708.877f, 137.0063f },
    { -331.9705f, -2690.038f, 137.0063f },
    { -408.6945f, -2680.292f, 137.0063f },
    { -405.2691f, -2649.764f, 183.4564f },
    { -405.9306f, -2644.854f, 176.9625f },
    { -411.1615f, -2646.877f, 174.9634f },
    { -404.1632f, -2652.825f, 173.2998f },
    { -395.7587f, -2645.983f, 173.2998f },
    { -410.7969f, -2632.696f, 171.2329f },
    { -423.0903f, -2644.964f, 169.6835f },
    { -415.5434f, -2655.818f, 170.1792f },
    { -408.5486f, -2657.453f, 170.4341f },
    { -391.8524f, -2651.392f, 169.7570f },
    { -396.2899f, -2636.481f, 166.9650f },
    { -408.6458f, -2628.502f, 163.3347f },
    { -419.1701f, -2616.116f, 135.5631f },
    { -426.4549f, -2635.628f, 135.5631f },
    { -408.3993f, -2672.628f, 135.5631f },
    { -367.2396f, -2657.358f, 141.6551f },
    { -393.0556f, -2612.667f, 135.5631f },
    { -436.3438f, -2625.976f, 135.5631f },
    { -422.7344f, -2670.457f, 120.9478f },
    { -382.0764f, -2672.203f, 118.2693f },
    { -369.2483f, -2639.075f, 118.2693f },
    { -359.3403f, -2548.865f, 109.4158f },
    { -339.1858f, -2505.839f, 101.9843f },
    { -255.2500f, -2350.267f, 104.9377f },
    { -35.55035f, -1998.759f, 101.9843f },
    { 28.78472f, -1775.781f, 187.6706f },
    { 42.83681f, -1698.201f, 225.9422f },
    { 47.69271f, -1662.769f, 230.4621f },
    { 47.82465f, -1612.547f, 165.4156f },
    { 43.26389f, -1566.889f, 92.21588f }
};

class npc_silver_bullet_x_831_vehicle : public CreatureScript
{
public:
    npc_silver_bullet_x_831_vehicle() : CreatureScript("npc_silver_bullet_x_831_vehicle") { }

    struct npc_silver_bullet_x_831_vehicleAI : public PassiveAI
    {
        npc_silver_bullet_x_831_vehicleAI(Creature* creature) : PassiveAI(creature) {}

        void IsSummonedBy(Unit* summoner) override
        {
            Position pos(*me);
            if (Creature* gyrotilt = me->SummonCreature(NPC_GYROTILT, pos, TEMPSUMMON_TIMED_DESPAWN, 300000))
                gyrotilt->EnterVehicle(me, 0);

            summoner->EnterVehicle(me, 1);
            me->SetDisableGravity(true);
            me->SetCanFly(true);
        }

        void PassengerBoarded(Unit* passenger, int8 seatId, bool apply) override
        {
            if (apply && passenger->GetTypeId() == TYPEID_PLAYER)
                events.ScheduleEvent(EVENT_TIMER_3, 500);
        }

        void MovementInform(uint32 type, uint32 point) override
        {
            if (type != SPLINE_MOTION_TYPE)
                return;

            if (Unit* passenger = me->GetVehicleKit()->GetPassenger(0)){
                switch (point)
                {
                case 1:
                    passenger->ToCreature()->AI()->Talk(TALK_GYROTILT_WOW_CANT, passenger->GetGUID(), CHAT_MSG_MONSTER_SAY);
                    break;
                case 5:
                    passenger->ToCreature()->AI()->Talk(TALK_GYROTILT_MAN_OH, passenger->GetGUID(), CHAT_MSG_MONSTER_SAY);
                    break;
                case 11:
                    passenger->ToCreature()->AI()->Talk(TALK_GYROTILT_MY_DREAM, passenger->GetGUID(), CHAT_MSG_MONSTER_SAY);
                    break;
                case 13:
                    passenger->ToCreature()->AI()->Talk(TALK_GYROTILT_WELL_HERE, passenger->GetGUID(), CHAT_MSG_MONSTER_SAY);
                    break;
                case 16:
                    DoCast(me, SPELL_COSMETIC_EXPLOSION, false);
                    break;
                case 17:
                    DoCast(me, SPELL_COSMETIC_EXPLOSION, false);
                    passenger->ToCreature()->AI()->Talk(TALK_GYROTILT_HUH_HOLD, passenger->GetGUID(), CHAT_MSG_MONSTER_SAY);
                    break;
                case 24:
                    DoCast(me, SPELL_COSMETIC_EXPLOSION, false);
                    passenger->ToCreature()->AI()->Talk(TALK_GYROTILT_WHOA_WHAT, passenger->GetGUID(), CHAT_MSG_MONSTER_SAY);
                    break;
                case 26:
                    DoCast(me, SPELL_COSMETIC_EXPLOSION, false);
                    break;
                case 30:
                    passenger->ToCreature()->AI()->Talk(TALK_GYROTILT_NO_NO, passenger->GetGUID(), CHAT_MSG_MONSTER_SAY);
                    break;
                case 35:
                    passenger->ToCreature()->AI()->Talk(TALK_GYROTILT_AUGH_LOOKS, passenger->GetGUID(), CHAT_MSG_MONSTER_SAY);
                    break;
                case 38:
                    passenger->ToCreature()->AI()->Talk(TALK_GYROTILT_MAYBE_IF, passenger->GetGUID(), CHAT_MSG_MONSTER_SAY);
                    break;
                case 44:
                    DoCast(me, SPELL_SMOKE_CLOUD, false);
                    DoCast(me, SPELL_IMPACT_EXPLOSION, false);
                    DoCast(me, SPELL_QUEST_COMPLETE, false);
                    DoCast(me, SPELL_EJECT_ALL_PASSENGERS, false);
                    passenger->ToCreature()->DespawnOrUnsummon(3000);
                    me->DespawnOrUnsummon(3000);
                    break;
                default:
                    break;
                }
            }
        }

        void UpdateAI(const uint32 diff) override
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_TIMER_3:
                        if (Unit* passenger = me->GetVehicleKit()->GetPassenger(1))
                            Talk(BROADCAST_START_IN_3, passenger->GetGUID(), CHAT_MSG_RAID_BOSS_EMOTE);
                        events.ScheduleEvent(EVENT_TIMER_2, 1000);
                        break;
                    case EVENT_TIMER_2:
                        if (Unit* passenger = me->GetVehicleKit()->GetPassenger(1))
                            Talk(BROADCAST_START_IN_2, passenger->GetGUID(), CHAT_MSG_RAID_BOSS_EMOTE);
                        events.ScheduleEvent(EVENT_TIMER_1, 1000);
                        break;
                    case EVENT_TIMER_1:
                        if (Unit* passenger = me->GetVehicleKit()->GetPassenger(1))
                            Talk(BROADCAST_START_IN_1, passenger->GetGUID(), CHAT_MSG_RAID_BOSS_EMOTE);
                        events.ScheduleEvent(EVENT_START_PATH, 1000);
                        break;
                    case EVENT_START_PATH:
                        if (Unit* passenger = me->GetVehicleKit()->GetPassenger(1))
                            Talk(BROADCAST_GO, passenger->GetGUID(), CHAT_MSG_RAID_BOSS_EMOTE);
                        DoCast(me, SPELL_ROCKET_TRAIL, true);
                        DoCast(me, SPELL_ROCKET_LOOP_SOUND, true);
                        me->GetMotionMaster()->MoveSmoothPath(rocketPath, rocketPathSize);
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_silver_bullet_x_831_vehicleAI(creature);
    }
};

class npc_silver_bullet_x_831 : public CreatureScript
{
public:
    npc_silver_bullet_x_831() : CreatureScript("npc_silver_bullet_x_831") { }

    struct npc_silver_bullet_x_831AI : public ScriptedAI
    {
        npc_silver_bullet_x_831AI(Creature* creature) : ScriptedAI(creature) 
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }

        void OnSpellClick(Unit* clicker, bool& /*result*/) override
        {
            if (Player* player = clicker->ToPlayer())
            {
                if (player->GetQuestStatus(QUEST_THE_SHORT_WAY_HOME) == QUEST_STATUS_INCOMPLETE)
                {
                    if (!me->FindNearestCreature(NPC_SILVER_BULLET_VEHICLE, 5.00f))
                    {
                        Position pos(*me);
                        player->SummonCreature(NPC_SILVER_BULLET_VEHICLE, pos);
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_silver_bullet_x_831AI(creature);
    }
};

void AddSC_the_barrens()
{
    new npc_wizzlecrank_shredder();
    // new npc_stonetalon_prisoner(); // Handled through SAI
    new npc_silver_bullet_x_831_vehicle();
    new npc_silver_bullet_x_831();
}
