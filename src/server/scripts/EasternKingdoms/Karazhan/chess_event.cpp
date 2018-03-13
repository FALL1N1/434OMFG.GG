/* Copyright (C) 2006 - 2013 ScriptDev2 <http://www.scriptdev2.com/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: chess_event
SD%Complete: 80
SDComment: Chess AI could use some improvements.
SDCategory: Karazhan
EndScriptData */

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Spell.h"
#include "Vehicle.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CreatureTextMgr.h"
#include "karazhan.h"

enum texts
{
    TALK_PLAYER_LOOSE_PIECE_PAWN        = 0,
    TALK_PLAYER_LOOSE_PIECE_ROCK        = 1,
    TALK_PLAYER_LOOSE_PIECE_KNIGHT      = 2,
    TALK_PLAYER_LOOSE_PIECE_BISHOP      = 3,
    TALK_PLAYER_LOOSE_PIECE_QUEEN       = 4,
    TALK_PLAYER_LOOSE_PIECE_KING        = 5,
    TALK_PLAYER_LOOSE_PIECE_CHECKMATE   = 6,
    TALK_MEDHIV_LOOSE_PIECE_PAWN        = 7,
    TALK_MEDHIV_LOOSE_PIECE_ROCK        = 8,
    TALK_MEDHIV_LOOSE_PIECE_KNIGHT      = 9,
    TALK_MEDHIV_LOOSE_PIECE_BISHOP      = 10,
    TALK_MEDHIV_LOOSE_PIECE_QUEEN       = 11,
    TALK_MEDHIV_LOOSE_PIECE_KING        = 12,
    TALK_MEDHIV_LOOSE_PIECE_CHECKMATE   = 13,
    TALK_EVENT_BEGIN                    = 14,
    TALK_CHEAT                          = 15,

    SOUND_ID_CHECK_PLAYER           = 10353,
    SOUND_ID_CHECK_MEDIVH           = 10354,
    //    SOUND_ID_WIN_PLAYER             = 10355,
    //    SOUND_ID_WIN_MEDIVH             = 10356,

    // gossip texts

    // gossip menu
    GOSSIP_MENU_ID_GRUNT            = 10425,
    GOSSIP_MENU_ID_WOLF             = 10439,
    GOSSIP_MENU_ID_WARLOCK          = 10440,
    GOSSIP_MENU_ID_NECROLYTE        = 10434,
    GOSSIP_MENU_ID_DEAMON           = 10426,
    GOSSIP_MENU_ID_BLACKHAND        = 10442,
    GOSSIP_MENU_ID_FOOTMAN          = 8952,
    GOSSIP_MENU_ID_CHARGER          = 10414,
    GOSSIP_MENU_ID_CONJURER         = 10417,
    GOSSIP_MENU_ID_CLERIC           = 10416,
    GOSSIP_MENU_ID_ELEMENTAL        = 10413,
    GOSSIP_MENU_ID_LLANE            = 10418,
    GOSSIP_MENU_ID_MEDIVH           = 10506,
    GOSSIP_MENU_ID_MEDIVH_BEATEN    = 10718,
};

#define GOSSIP_ITEM_ORC_GRUNT           "Control Orc Grunt"
#define GOSSIP_ITEM_ORC_WOLF            "Control Orc Wolf"
#define GOSSIP_ITEM_SUMMONED_DEAMON     "Control Summoned Daemon"
#define GOSSIP_ITEM_ORC_WARLOCK         "Control Orc Warlock"
#define GOSSIP_ITEM_ORC_NECROLYTE       "Control Orc Necrolyte"
#define GOSSIP_ITEM_WARCHIEF_BLACKHAND  "Control Warchief Blackhand"
#define GOSSIP_ITEM_HUMAN_FOOTMAN       "Control Human Footman"
#define GOSSIP_ITEM_HUMAN_CHARGER       "Control Human Charger"
#define GOSSIP_ITEM_WATER_ELEMENTAL     "Control Conjured Water Elemental"
#define GOSSIP_ITEM_HUMAN_CONJURER      "Control Human Conjurer"
#define GOSSIP_ITEM_HUMAN_CLERIC        "Control Human Cleric"
#define GOSSIP_ITEM_KING_LLANE          "Control King Llane"
#define GOSSIP_ITEM_RESET_BOARD         "Reset Board"

enum Spells
{
    // movement spells
    SPELL_MOVE_GENERIC              = 30012,                    // spell which sends the signal to move - handled in core
    SPELL_MOVE_1                    = 32312,                    // spell which selects AI move square (for short range pieces)
    SPELL_MOVE_2                    = 37388,                    // spell which selects AI move square (for long range pieces)
    // SPELL_MOVE_PAWN              = 37146,                    // individual move spells (used only by controlled npcs)
    // SPELL_MOVE_KNIGHT            = 37144,
    // SPELL_MOVE_QUEEN             = 37148,
    // SPELL_MOVE_ROCK              = 37151,
    // SPELL_MOVE_BISHOP            = 37152,
    // SPELL_MOVE_KING              = 37153,

    // additional movement spells
    SPELL_CHANGE_FACING             = 30284,                    // spell which sends the initial facing request - handled in core
    SPELL_FACE_SQUARE               = 30270,                    // change facing - finalize facing update

    SPELL_MOVE_TO_SQUARE            = 30253,                    // spell which sends the move response from the square to the piece
    SPELL_MOVE_COOLDOWN             = 30543,                    // add some cooldown to movement
    SPELL_MOVE_MARKER               = 32261,                    // white beam visual - used to mark the movement as complete
    SPELL_DISABLE_SQUARE            = 32745,                    // used by the White / Black triggers on the squares when a chess piece moves into place
    SPELL_IS_SQUARE_USED            = 39400,                    // cast when a chess piece moves to another square
    // SPELL_SQUARED_OCCUPIED       = 39399,                    // triggered by 39400; used to check if the square is occupied (if hits a target); Missing in 2.4.3

    // generic spells
    SPELL_IN_GAME                   = 30532,                    // teleport player near the entrance
    SPELL_CONTROL_PIECE             = 30019,                    // control a chess piece
    SPELL_RECENTLY_IN_GAME          = 30529,                    // debuff on player after chess piece uncharm

    SPELL_CHESS_AI_ATTACK_TIMER     = 32226,                    // melee action timer - triggers 32225
    SPELL_ACTION_MELEE              = 32225,                    // handle melee attacks
    SPELL_MELEE_DAMAGE              = 32247,                    // melee damage spell - used by all chess pieces
    // SPELL_AI_SNAPSHOT_TIMER      = 37440,                    // used to trigger spell 32260; purpose and usage unk
    // SPELL_DISABLE_SQUARE_SELF    = 32260,                    // used when a piece moves to another square
    // SPELL_AI_ACTION_TIMER        = 37504,                    // handle some kind of event check. Cast by npc 17459. Currently the way it works is unk
    // SPELL_DISABLE_SQUARE         = 30271,                    // not used
    // SPELL_FIND_ENEMY             = 32303,                    // not used
    // SPELL_MOVE_NEAR_UNIT         = 30417,                    // not used
    // SPELL_GET_EMPTY_SQUARE       = 30418,                    // not used
    // SPELL_FACE_NEARBY_ENEMY      = 37787,                    // not used
    // SPELL_POST_MOVE_FACING       = 38011,                    // not used

    // melee action spells
    SPELL_MELEE_FOOTMAN             = 32227,
    SPELL_MELEE_WATER_ELEM          = 37142,
    SPELL_MELEE_CHARGER             = 37143,
    SPELL_MELEE_CLERIC              = 37147,
    SPELL_MELEE_CONJURER            = 37149,
    SPELL_MELEE_KING_LLANE          = 37150,
    SPELL_MELEE_GRUNT               = 32228,
    SPELL_MELEE_DAEMON              = 37220,
    SPELL_MELEE_NECROLYTE           = 37337,
    SPELL_MELEE_WOLF                = 37339,
    SPELL_MELEE_WARLOCK             = 37345,
    SPELL_MELEE_WARCHIEF_BLACKHAND  = 37348,

    // cheat spells
    SPELL_HAND_OF_MEDIVH_HORDE      = 39338,                    // triggers 39339
    SPELL_HAND_OF_MEDIVH_ALLIANCE   = 39342,                    // triggers 39339
    SPELL_FURY_OF_MEDIVH_HORDE      = 39341,                    // triggers 39343
    SPELL_FURY_OF_MEDIVH_ALLIANCE   = 39344,                    // triggers 39345
    SPELL_FURY_OF_MEDIVH_AURA       = 39383,
    SPELL_HAND_OF_MEDIVH_TRIGGER    = 39339,
    SPELL_FULL_HEAL_HORDE           = 39334,                    // spells are not confirmed (probably removed after 2.4.3)
    SPELL_FULL_HEAL_ALLIANCE        = 39335,

    // spells used by the chess npcs
    SPELL_HEROISM                   = 37471,                    // human king
    SPELL_SWEEP                     = 37474,
    SPELL_BLOODLUST                 = 37472,                    // orc king
    SPELL_CLEAVE                    = 37476,
    SPELL_HEROIC_BLOW               = 37406,                    // human pawn
    SPELL_SHIELD_BLOCK              = 37414,
    SPELL_VICIOUS_STRIKE            = 37413,                    // orc pawn
    SPELL_WEAPON_DEFLECTION         = 37416,
    SPELL_SMASH                     = 37453,                    // human knight
    SPELL_STOMP                     = 37498,
    SPELL_BITE                      = 37454,                    // orc knight
    SPELL_HOWL                      = 37502,
    SPELL_ELEMENTAL_BLAST           = 37462,                    // human queen
    SPELL_RAIN_OF_FIRE              = 37465,
    SPELL_FIREBALL                  = 37463,                    // orc queen
    // SPELL_POISON_CLOUD           = 37469,
    SPELL_POISON_CLOUD_ACTION       = 37775,                    // triggers 37469 - acts as a target selector spell for orc queen
    SPELL_HEALING                   = 37455,                    // human bishop
    SPELL_HOLY_LANCE                = 37459,
    SPELL_SHADOW_MEND_ACTION            = 37456,                    // orc bishop
    // SPELL_SHADOW_MEND_ACTION        = 37824,                    // triggers 37456 - acts as a target selector spell for orc bishop
    SPELL_SHADOW_SPEAR              = 37461,
    SPELL_GEYSER                    = 37427,                    // human rook
    SPELL_WATER_SHIELD              = 37432,
    SPELL_HELLFIRE                  = 37428,                    // orc rook
    SPELL_FIRE_SHIELD               = 37434,

    // spells used to transform side trigger when npc dies
    SPELL_TRANSFORM_FOOTMAN         = 39350,
    SPELL_TRANSFORM_CHARGER         = 39352,
    SPELL_TRANSFORM_CLERIC          = 39353,
    SPELL_TRANSFORM_WATER_ELEM      = 39354,
    SPELL_TRANSFORM_CONJURER        = 39355,
    SPELL_TRANSFORM_KING_LLANE      = 39356,
    SPELL_TRANSFORM_GRUNT           = 39357,
    SPELL_TRANSFORM_WOLF            = 39358,
    SPELL_TRANSFORM_NECROLYTE       = 39359,
    SPELL_TRANSFORM_DAEMON          = 39360,
    SPELL_TRANSFORM_WARLOCK         = 39361,
    SPELL_TRANSFORM_BLACKHAND       = 39362,

};

enum Npcs
{
    // NPC_SQUARE_OUTSIDE_B         = 17316,                    // used to check the interior of the board
    // NPC_SQUARE_OUTSIDE_W         = 17317,                    // not used in our script; keep for reference only
    NPC_FURY_MEDIVH_VISUAL          = 22521,                    // has aura 39383
};

enum Miscss
{
    TARGET_TYPE_RANDOM              = 1,
    TARGET_TYPE_FRIENDLY            = 2,
    PIECES_HP_SUM                   = 1040000,
    FIRST_CHEAT_HP_MIN              = PIECES_HP_SUM/10,
    FIRST_CHEAT_HP_MAX              = PIECES_HP_SUM/6,
    SECOND_CHEAT_HP_MIN             = PIECES_HP_SUM/5,
    SECOND_CHEAT_HP_MAX             = PIECES_HP_SUM/2,
    THIRD_CHEAT_HP_MIN              = PIECES_HP_SUM - PIECES_HP_SUM/5,
    THIRD_CHEAT_HP_MAX              = PIECES_HP_SUM - PIECES_HP_SUM/9,

    FIRST_CHEAT_TIMER_MIN           = 60000,
    FIRST_CHEAT_TIMER_MAX           = 90000,
    SECOND_CHEAT_TIMER_MIN          = 75000,
    SECOND_CHEAT_TIMER_MAX          = 120000,
    THIRD_CHEAT_TIMER_MIN           = 120000,
    THIRD_CHEAT_TIMER_MAX           = 180000
};

struct cheat_timers
{
    uint32 minTimer;
    uint32 maxTimer;
};

cheat_timers cheatTimers[3] =
{
    { FIRST_CHEAT_TIMER_MIN, FIRST_CHEAT_TIMER_MAX },
    { SECOND_CHEAT_TIMER_MIN, SECOND_CHEAT_TIMER_MAX },
    { THIRD_CHEAT_TIMER_MIN, THIRD_CHEAT_TIMER_MIN }
};

struct cheat_damages
{
    uint32 minDamages;
    uint32 maxDamages;
};

cheat_damages cheatDamagesReq[3] =
{
    { FIRST_CHEAT_HP_MIN, FIRST_CHEAT_HP_MAX },
    { SECOND_CHEAT_HP_MIN, SECOND_CHEAT_HP_MAX },
    { THIRD_CHEAT_HP_MIN, THIRD_CHEAT_HP_MAX }
};

enum Events
{
    EVENT_CHEAT_CHECK_1 = 1,
    EVENT_CHEAT_CHECK_2,
    EVENT_CHEAT_CHECK_3,
    EVENT_COMMAND_TIMER,
    EVENT_CONTROL_PIECE_SPELL,
    EVENT_MOVE_PIECE,
};

uint32 pieceEntrys[12] =
{
    21684,
    21752,
    21683,
    21750,
    17211,
    17469,
    21160,
    21726,
    21664,
    21748,
    21682,
    21747,
};

float facingAngle[4] =
{
    0.76f, // ally
    2.28f, // entrance
    3.81f, // horde
    5.43f, // exit
};

/*######
## npc_echo_of_medivh
######*/

class npc_echo_of_medivh : public CreatureScript
{
public:
    npc_echo_of_medivh() : CreatureScript("npc_echo_of_medivh") { }

    struct npc_echo_of_medivhAI : public ScriptedAI
    {
        npc_echo_of_medivhAI(Creature* creature) : ScriptedAI(creature), instance(creature->GetInstanceScript()) { }

        void Reset()
        {
            events.Reset();
        }

        void DoAction(int32 const eventType) override
        {
            if (eventType == AI_EVENT_CUSTOM_B)
            {
                events.Reset();
                events.ScheduleEvent(EVENT_CHEAT_CHECK_1, urand(FIRST_CHEAT_TIMER_MIN, FIRST_CHEAT_TIMER_MAX));
                events.ScheduleEvent(EVENT_CHEAT_CHECK_2, urand(SECOND_CHEAT_TIMER_MIN, SECOND_CHEAT_TIMER_MAX));
                events.ScheduleEvent(EVENT_CHEAT_CHECK_3, urand(THIRD_CHEAT_TIMER_MIN, THIRD_CHEAT_TIMER_MAX));
            }
        }

        void MoveInLineOfSight(Unit* /*pWho*/) override { }
        void AttackStart(Unit* /*pWho*/) override { }

        void JustSummoned(Creature* pSummoned) override
        {
            if (pSummoned->GetEntry() == NPC_FURY_MEDIVH_VISUAL)
                pSummoned->CastSpell(pSummoned, SPELL_FURY_OF_MEDIVH_AURA, true);
        }

        void UpdateAI(const uint32 diff) override
        {
            if (!instance || instance->GetData(TYPE_CHESS) != IN_PROGRESS)
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CHEAT_CHECK_1:
                    {
                        if (urand(cheatDamagesReq[0].minDamages, cheatDamagesReq[0].maxDamages) < instance->GetData(DATA_CHESS_DAMAGE))
                        {
                            DoCast(me, instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE ? SPELL_HAND_OF_MEDIVH_HORDE : SPELL_HAND_OF_MEDIVH_ALLIANCE);
                            Talk(TALK_CHEAT, me->GetGUID());
                            events.ScheduleEvent(EVENT_CHEAT_CHECK_1, urand(cheatTimers[0].minTimer, cheatTimers[0].maxTimer));
                        }
                        else
                            events.ScheduleEvent(EVENT_CHEAT_CHECK_1, 1000);
                        break;
                    }
                    case EVENT_CHEAT_CHECK_2:
                    {
                        if (urand(cheatDamagesReq[1].minDamages, cheatDamagesReq[1].maxDamages) < instance->GetData(DATA_CHESS_DAMAGE))
                        {
                            DoCast(me, instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE ? SPELL_FURY_OF_MEDIVH_ALLIANCE : SPELL_FURY_OF_MEDIVH_HORDE);
                            Talk(TALK_CHEAT, me->GetGUID());
                            events.ScheduleEvent(EVENT_CHEAT_CHECK_2, urand(cheatTimers[1].minTimer, cheatTimers[1].maxTimer));
                        }
                        else
                            events.ScheduleEvent(EVENT_CHEAT_CHECK_2, 1000);
                        break;
                    }
                    case EVENT_CHEAT_CHECK_3:
                    {
                        if (urand(cheatDamagesReq[2].minDamages, cheatDamagesReq[2].maxDamages) < instance->GetData(DATA_CHESS_DAMAGE))
                        {
                            DoCast(me, instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE ? SPELL_FULL_HEAL_HORDE : SPELL_FULL_HEAL_ALLIANCE);
                            // heal part
                            Talk(TALK_CHEAT, me->GetGUID());
                            events.ScheduleEvent(EVENT_CHEAT_CHECK_3, urand(cheatTimers[2].minTimer, cheatTimers[2].maxTimer));
                        }
                        else
                            events.ScheduleEvent(EVENT_CHEAT_CHECK_3, 1000);
                        break;
                    }
                    default:
                        break;
                }
            }
        }

    private:
        InstanceScript * instance;
        EventMap events;
    };

    bool OnGossipHello(Player* player, Creature* creature)
    {
        InstanceScript *instance = creature->GetInstanceScript();
        if (!instance)
            return false;
        if (instance->GetData(TYPE_CHESS) == SPECIAL)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_RESET_BOARD, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(GOSSIP_MENU_ID_MEDIVH, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        InstanceScript *instance = creature->GetInstanceScript();
        if (!instance)
            return false;
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                instance->SetData(TYPE_CHESS, DONE);
                break;
            default:
                break;
        }
        player->CLOSE_GOSSIP_MENU();
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_echo_of_medivhAI(creature);
    }
};

/*######
## npc_chess_piece_generic
######*/

class PieceGen
{
public:
    PieceGen(){};
    ~PieceGen(){};

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        InstanceScript *instance = creature->GetInstanceScript();
        if (!instance)
            return false;
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                // teleport at the entrance and control the chess piece
                player->NearTeleportTo(-11107.2f, -1842.88f, 229.70f, 5.39f);
                player->CastSpell(player, SPELL_IN_GAME, true);
                player->CastSpell(creature, SPELL_CONTROL_PIECE, true);

                if (instance->GetData(TYPE_CHESS) == NOT_STARTED)
                {
                    if (Creature* pMedivh =  Unit::GetCreature(*creature, instance->GetData64(NPC_ECHO_MEDIVH)))
                        pMedivh->AI()->Talk(TALK_EVENT_BEGIN);
                    instance->SetData(TYPE_CHESS, IN_PROGRESS);
                }
                else if (instance->GetData(TYPE_CHESS) == DONE)
                    instance->SetData(TYPE_CHESS, SPECIAL);
                break;
            default:
                break;
        }

        player->CLOSE_GOSSIP_MENU();
        return true;
    }
};

struct npc_chess_piece_genericAI : public ScriptedAI
{
public:
    npc_chess_piece_genericAI(Creature* creature) : ScriptedAI(creature), instance(creature->GetInstanceScript())
    {
        m_currentSquareGuid = 0;
        m_fCurrentOrientation = me->GetOrientation();
        m_faction = me->getFaction();
        me->setRegeneratingHealth(false);
        me->SetHealth(me->GetMaxHealth());
    }

    void Reset()
    {
        events.Reset();
        m_bIsPrimarySpell = true;

        // cancel move timer for player faction npcs or for friendly games
        if (!((instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE && m_faction == FACTION_ID_CHESS_ALLIANCE) ||
            (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE && m_faction == FACTION_ID_CHESS_HORDE) ||
              instance->GetData(TYPE_CHESS) == DONE))
            events.ScheduleEvent(EVENT_COMMAND_TIMER, 1000);
        events.ScheduleEvent(EVENT_CONTROL_PIECE_SPELL, 1000);
    }

    // no default attacking or evading
    void MoveInLineOfSight(Unit* /*pWho*/) override { }
    void AttackStart(Unit* /*pWho*/) override { }
    void EnterEvadeMode() override { }
    void OnCharmed(bool /*apply*/) override { }

    void DamageTaken(Unit* pDoneBy, uint32& uiDamage) override
    {
        switch (me->GetEntry())
        {
            case NPC_ORC_GRUNT:
            case NPC_ORC_WOLF:
            case NPC_ORC_WARLOCK:
            case NPC_ORC_NECROLYTE:
            case NPC_SUMMONED_DAEMON:
            case NPC_WARCHIEF_BLACKHAND:
                if (instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                    instance->SetData(DATA_CHESS_DAMAGE, uiDamage);
                break;
            case NPC_HUMAN_FOOTMAN:
            case NPC_HUMAN_CHARGER:
            case NPC_HUMAN_CONJURER:
            case NPC_HUMAN_CLERIC:
            case NPC_CONJURED_WATER_ELEMENTAL:
            case NPC_KING_LLANE:
                if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                    instance->SetData(DATA_CHESS_DAMAGE, uiDamage);
                break;
        }
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (Creature* pSquare = Unit::GetCreature(*me, m_currentSquareGuid))
            pSquare->RemoveAllAuras();
        me->SetVisible(false);
        // ToDo: remove corpse after 10 sec
    }

    void SetGUID(uint64 guid, int32 type/* = 0 */)
    {
        m_currentSquareGuid = guid;
    }

    uint64 GetGUID(int32 /*id*/ = 0) const
    {
        return m_currentSquareGuid;
    }

    void DoAction(int32 const eventType) override
    {
        // handle move event
        if (eventType == AI_EVENT_CUSTOM_A)
        {
            // clear the current square
            if (Creature* pSquare = Unit::GetCreature(*me, m_currentSquareGuid))
                pSquare->RemoveAllAuras();

            events.ScheduleEvent(EVENT_MOVE_PIECE, 1000);
        }
        // handle encounter start event
        else if (eventType == AI_EVENT_CUSTOM_B)
        {
            // reset the variables
            Reset();

            // ToDo: enable this when the scope of the spell is clear
            //if (Creature* pStalker = instance->GetSingleCreatureFromStorage(NPC_WAITING_ROOM_STALKER))
            //    pStalker->CastSpell(pStalker, SPELL_AI_ACTION_TIMER, true);

            //DoCast(me, SPELL_AI_SNAPSHOT_TIMER, true);
            DoCast(me, SPELL_CHESS_AI_ATTACK_TIMER, true);
            if (Unit *pInvoker = Unit::GetCreature(*me, m_currentSquareGuid))
            {
                pInvoker->CastSpell(pInvoker, SPELL_DISABLE_SQUARE, true);
                pInvoker->CastSpell(pInvoker, SPELL_IS_SQUARE_USED, true);
            }
        }
    }

    void MovementInform(uint32 uiMotionType, uint32 uiPointId) override
    {
        if (uiMotionType != POINT_MOTION_TYPE || uiPointId != 1)
            return;

        for (int i = 0; i < 4; i++ )
        {
            if (facingAngle[i] >= m_fCurrentOrientation)
            {
                if (i == 0)
                    m_fCurrentOrientation = facingAngle[i];
                else
                    m_fCurrentOrientation = (abs(m_fCurrentOrientation - facingAngle[i]) < abs(m_fCurrentOrientation - facingAngle[i - 1])) ?
                        facingAngle[i] : facingAngle[i - 1];
                break;
            }
        }
        // update facing
        if (!((instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE && m_faction == FACTION_ID_CHESS_ALLIANCE) ||
              (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE && m_faction == FACTION_ID_CHESS_HORDE)))
        {
            if (Unit* target = GetTargetByType(TARGET_TYPE_RANDOM, 5.0f))
                DoCast(target, SPELL_CHANGE_FACING);
            else
                me->SetFacingTo(m_fCurrentOrientation);
        }
        else
            me->SetFacingTo(m_fCurrentOrientation);
    }

    void SpellHit(Unit* pCaster, SpellInfo const* pSpell) override
    {
        // do a soft reset when the piece is controlled
        if (pCaster->GetTypeId() == TYPEID_PLAYER && pSpell->Id == SPELL_CONTROL_PIECE)
            Reset();
    }

    // Function which returns a random target by type and range
    Unit* GetTargetByType(uint8 uiType, float fRange, float fArc = M_PI)
    {
        if (!instance)
            return NULL;

        uint32 uiTeam = me->getFaction() == FACTION_ID_CHESS_ALLIANCE ? FACTION_ID_CHESS_HORDE : FACTION_ID_CHESS_ALLIANCE;

        // get friendly list for this type
        if (uiType == TARGET_TYPE_FRIENDLY)
            uiTeam = me->getFaction();

        // Get the list of enemies
        std::list<uint64 > lTempList;
        std::vector<Creature*> vTargets;
        vTargets.reserve(lTempList.size());
        ((instance_karazhan::instance_karazhan_InstanceMapScript *)instance)->GetChessPiecesByFaction(lTempList, uiTeam);
        for (std::list<uint64 >::const_iterator itr = lTempList.begin(); itr != lTempList.end(); ++itr)
        {
            Creature* pTemp = Unit::GetCreature(*me, *itr);
            if (pTemp && pTemp->isAlive())
            {
                // check for specified range targets and angle; Note: to be checked if the angle is right
                if (fRange && !me->isInFrontInMap(pTemp, fRange, fArc))
                    continue;

                if (fRange && me->GetExactDist2d(pTemp->GetPositionX(), pTemp->GetPositionY()) > fRange)
                    continue;

                // skip friendly targets which are at full HP
                if (uiType == TARGET_TYPE_FRIENDLY && pTemp->GetHealth() == pTemp->GetMaxHealth())
                    continue;

                vTargets.push_back(pTemp);
            }
        }

        if (vTargets.empty())
            return NULL;

        return Trinity::Containers::SelectRandomContainerElement(vTargets);
    }

    // Function to get a square as close as possible to the enemy
    Unit* GetMovementSquare()
    {
        if (!instance)
            return NULL;

        // define distance based on the spell radius
        // this will replace the targeting sysmte of spells SPELL_MOVE_1 and SPELL_MOVE_2
        float fRadius = 5.0f;
        std::list<Creature*> lSquaresList;

        // some pieces have special distance
        switch (me->GetEntry())
        {
            case NPC_HUMAN_CONJURER:
            case NPC_ORC_WARLOCK:
            case NPC_HUMAN_CHARGER:
            case NPC_ORC_WOLF:
                fRadius = 10.0f;
                break;
        }

        // get all available squares for movement
        GetCreatureListWithEntryInGrid(lSquaresList, me, NPC_SQUARE_BLACK, fRadius);
        GetCreatureListWithEntryInGrid(lSquaresList, me, NPC_SQUARE_WHITE, fRadius);

        if (lSquaresList.empty())
            return NULL;

        lSquaresList.remove_if([=](WorldObject* target)
                               {
                                  return (((instance_karazhan::instance_karazhan_InstanceMapScript *)instance)->IsCaseOccupied(target->ToCreature()));
                               });

        if (lSquaresList.empty())
            return NULL;

        // Get the list of enemies
        std::list<uint64 > lTempList;
        std::list<Creature*> lEnemies;

        ((instance_karazhan::instance_karazhan_InstanceMapScript *)instance)->GetChessPiecesByFaction(lTempList, me->getFaction() == FACTION_ID_CHESS_ALLIANCE ? FACTION_ID_CHESS_HORDE : FACTION_ID_CHESS_ALLIANCE);
        for (std::list<uint64 >::const_iterator itr = lTempList.begin(); itr != lTempList.end(); ++itr)
        {
            Creature* pTemp = Unit::GetCreature(*me, *itr);
            if (pTemp && pTemp->isAlive())
                lEnemies.push_back(pTemp);
        }

        if (lEnemies.empty())
            return NULL;

        // Sort the enemies by distance and the squares compared to the distance to the closest enemy
        lEnemies.sort(Trinity::ObjectDistanceOrderPred(me));
        lSquaresList.sort(Trinity::ObjectDistanceOrderPred(lEnemies.front()));

        return lSquaresList.front();
    }

    virtual uint32 DoCastPrimarySpell() { return 5000; }
    virtual uint32 DoCastSecondarySpell() { return 5000; }

    void UpdateAI(const uint32 diff) override
    {
        if (!instance || instance->GetData(TYPE_CHESS) != IN_PROGRESS)
            return;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_COMMAND_TIMER:
                {
                    // just update facing if some enemy is near
                    if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 8.0f))
                        DoCast(pTarget, SPELL_CHANGE_FACING);
                    else
                    {
                        // the npc doesn't have a 100% chance to move; also there should be some GCD check in core for this part
                        if (roll_chance_i(15))
                        {
                            // Note: in a normal case the target would be chosen using the spells above
                            // However, because the core doesn't support special targeting, we'll provide explicit target
                            //uint32 uiMoveSpell = SPELL_MOVE_1;
                            //switch (me->GetEntry())
                            //{
                            //    case NPC_HUMAN_CONJURER:
                            //    case NPC_ORC_WARLOCK:
                            //    case NPC_HUMAN_CHARGER:
                            //    case NPC_ORC_WOLF:
                            //        uiMoveSpell = SPELL_MOVE_2;
                            //        break;
                            //}
                            //DoCast(me, uiMoveSpell, true);

                            // workaround which provides specific move target
                            if (Unit* pTarget = GetMovementSquare())
                                DoCast(pTarget, SPELL_MOVE_GENERIC, true);

                            m_fCurrentOrientation = me->GetOrientation();
                        }
                    }
                    events.ScheduleEvent(EVENT_COMMAND_TIMER, 5000);
                    break;
                }
                case EVENT_CONTROL_PIECE_SPELL:
                {
                    uint32 timer = !me->HasAura(SPELL_CONTROL_PIECE) ? (m_bIsPrimarySpell ? DoCastPrimarySpell() : DoCastSecondarySpell()) : 5000;
                    if (!timer)
                        timer = 5000;
                    events.ScheduleEvent(EVENT_CONTROL_PIECE_SPELL, timer);
                    m_bIsPrimarySpell = !m_bIsPrimarySpell;
                    break;
                }
                case EVENT_MOVE_PIECE:
                {
                    if (Creature* pSquare = Unit::GetCreature(*me, m_currentSquareGuid))
                    {
                        m_fCurrentOrientation = me->GetOrientation();
                        DoCast(pSquare, SPELL_MOVE_MARKER, true);
                        me->GetMotionMaster()->MovePoint(1, pSquare->GetPositionX(), pSquare->GetPositionY(), pSquare->GetPositionZ());
                    }
                    break;
                }
            }
        }
    };

protected:
    InstanceScript* instance;
    uint64 m_currentSquareGuid;
    bool m_bIsPrimarySpell;
    float m_fCurrentOrientation;
    EventMap events;
    uint32 m_faction;
};

/*######
## npc_king_llane
######*/

class npc_king_llane : public CreatureScript, PieceGen
{
public:
    npc_king_llane() : CreatureScript("npc_king_llane") { }

    struct npc_king_llaneAI : public npc_chess_piece_genericAI
    {
        npc_king_llaneAI(Creature* creature) : npc_chess_piece_genericAI(creature)
        {
            m_bIsAttacked = false;
        }

        bool m_bIsAttacked;

        void Reset() { npc_chess_piece_genericAI::Reset(); }
        void MoveInLineOfSight(Unit* pWho) override { npc_chess_piece_genericAI::MoveInLineOfSight(pWho); }
        void AttackStart(Unit* pWho) override { npc_chess_piece_genericAI::AttackStart(pWho); }
        void EnterEvadeMode() override {npc_chess_piece_genericAI::EnterEvadeMode(); }
        void SetGUID(uint64 guid, int32 type/* = 0 */) { npc_chess_piece_genericAI::SetGUID(guid, type); }
        uint64 GetGUID(int32 id = 0) const { return npc_chess_piece_genericAI::GetGUID(id); }
        void DoAction(int32 const eventType) override { npc_chess_piece_genericAI::DoAction(eventType); }
        void MovementInform(uint32 uiMotionType, uint32 uiPointId) override { npc_chess_piece_genericAI::MovementInform(uiMotionType, uiPointId); }
        void SpellHit(Unit* pCaster, SpellInfo const* pSpell) override { npc_chess_piece_genericAI::SpellHit(pCaster, pSpell); }
        void UpdateAI(const uint32 diff) override { npc_chess_piece_genericAI::UpdateAI(diff); }

        void DamageTaken(Unit* pDoneBy, uint32& uiDamage) override
        {
            npc_chess_piece_genericAI::DamageTaken(pDoneBy, uiDamage);

            if (!uiDamage || !m_bIsAttacked || !instance || pDoneBy->GetTypeId() != TYPEID_UNIT)
                return;

            if (Creature* pMedivh =  Unit::GetCreature(*me, instance->GetData64(NPC_ECHO_MEDIVH)))
            {
                if (instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                    DoPlaySoundToSet(pMedivh, SOUND_ID_CHECK_PLAYER);
                else
                    DoPlaySoundToSet(pMedivh, SOUND_ID_CHECK_MEDIVH);
            }

            m_bIsAttacked = true;
        }

        void JustDied(Unit* pKiller) override
        {
            npc_chess_piece_genericAI::JustDied(pKiller);

            if (!instance)
                return;

            Creature* pMedivh = Unit::GetCreature(*me, instance->GetData64(NPC_ECHO_MEDIVH));
            if (!pMedivh)
                return;

            if (instance->GetData(TYPE_CHESS) == SPECIAL)
                instance->SetData(TYPE_CHESS, DONE);
            else
            {
                if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                {
                    Talk(TALK_PLAYER_LOOSE_PIECE_CHECKMATE, pMedivh->GetGUID());
                    instance->SetData(TYPE_CHESS, DONE);
                }
                else
                {
                    Talk(TALK_MEDHIV_LOOSE_PIECE_CHECKMATE, pMedivh->GetGUID());
                    instance->SetData(TYPE_CHESS, FAIL);
                }
            }

            ((instance_karazhan::instance_karazhan_InstanceMapScript *)instance)->DoMoveChessPieceToSides(SPELL_TRANSFORM_KING_LLANE, FACTION_ID_CHESS_ALLIANCE, true);
        }

        uint32 DoCastPrimarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 20.0f))
            {
                DoCast(me, SPELL_HEROISM);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_HEROISM))
                    return pSpell->GetRecoveryTime();
            }
            return 5000;
        }

        uint32 DoCastSecondarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 10.0f))
            {
                DoCast(me, SPELL_SWEEP);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_SWEEP))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }
    };

    bool OnGossipHello(Player* player, Creature* creature)
    {
        InstanceScript *instance = creature->GetInstanceScript();
        if (!instance)
            return false;

        if (player->HasAura(SPELL_RECENTLY_IN_GAME) || creature->HasAura(SPELL_CONTROL_PIECE))
            return true;

        if ((instance->GetData(TYPE_CHESS) != DONE && player->GetTeam() == ALLIANCE) || ((instance_karazhan::instance_karazhan_InstanceMapScript *)instance)->IsFriendlyGameReady())
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KING_LLANE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(GOSSIP_MENU_ID_LLANE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        return PieceGen::OnGossipSelect(player, creature, sender, action);
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_king_llaneAI(creature);
    }
};

/*######
## npc_warchief_blackhand
######*/

class npc_warchief_blackhand : public CreatureScript, PieceGen
{
public:
    npc_warchief_blackhand() : CreatureScript("npc_warchief_blackhand") { }

    struct npc_warchief_blackhandAI : public npc_chess_piece_genericAI
    {
        npc_warchief_blackhandAI(Creature* creature) : npc_chess_piece_genericAI(creature)
        {
            m_bIsAttacked = false;
        }

        bool m_bIsAttacked;

        void Reset() { npc_chess_piece_genericAI::Reset(); }
        void MoveInLineOfSight(Unit* pWho) override { npc_chess_piece_genericAI::MoveInLineOfSight(pWho); }
        void AttackStart(Unit* pWho) override { npc_chess_piece_genericAI::AttackStart(pWho); }
        void EnterEvadeMode() override {npc_chess_piece_genericAI::EnterEvadeMode(); }
        void SetGUID(uint64 guid, int32 type/* = 0 */) { npc_chess_piece_genericAI::SetGUID(guid, type); }
        uint64 GetGUID(int32 id = 0) const { return npc_chess_piece_genericAI::GetGUID(id); }
        void DoAction(int32 const eventType) override { npc_chess_piece_genericAI::DoAction(eventType); }
        void MovementInform(uint32 uiMotionType, uint32 uiPointId) override { npc_chess_piece_genericAI::MovementInform(uiMotionType, uiPointId); }
        void SpellHit(Unit* pCaster, SpellInfo const* pSpell) override { npc_chess_piece_genericAI::SpellHit(pCaster, pSpell); }
        void UpdateAI(const uint32 diff) override { npc_chess_piece_genericAI::UpdateAI(diff); }

        void DamageTaken(Unit* pDoneBy, uint32& uiDamage) override
        {
            npc_chess_piece_genericAI::DamageTaken(pDoneBy, uiDamage);

            if (!uiDamage || !m_bIsAttacked || !instance || pDoneBy->GetTypeId() != TYPEID_UNIT)
                return;

            if (Creature* pMedivh =  Unit::GetCreature(*me, instance->GetData64(NPC_ECHO_MEDIVH)))
            {
                if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                    DoPlaySoundToSet(pMedivh, SOUND_ID_CHECK_PLAYER);
                else
                    DoPlaySoundToSet(pMedivh, SOUND_ID_CHECK_MEDIVH);
            }

            m_bIsAttacked = true;
        }

        void JustDied(Unit* pKiller) override
        {
            npc_chess_piece_genericAI::JustDied(pKiller);

            if (!instance)
                return;

            Creature* pMedivh = Unit::GetCreature(*me, instance->GetData64(NPC_ECHO_MEDIVH));
            if (!pMedivh)
                return;

            if (instance->GetData(TYPE_CHESS) == SPECIAL)
                instance->SetData(TYPE_CHESS, DONE);
            else
            {
                if (instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                {
                    Talk(TALK_PLAYER_LOOSE_PIECE_CHECKMATE, pMedivh->GetGUID());
                    instance->SetData(TYPE_CHESS, DONE);
                }
                else
                {
                    Talk(TALK_MEDHIV_LOOSE_PIECE_CHECKMATE, pMedivh->GetGUID());
                    instance->SetData(TYPE_CHESS, FAIL);
                }
            }

            ((instance_karazhan::instance_karazhan_InstanceMapScript *)instance)->DoMoveChessPieceToSides(SPELL_TRANSFORM_BLACKHAND, FACTION_ID_CHESS_HORDE, true);
        }

        uint32 DoCastPrimarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 20.0f))
            {
                DoCast(me, SPELL_BLOODLUST);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_BLOODLUST))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }

        uint32 DoCastSecondarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 10.0f))
            {
                DoCast(me, SPELL_CLEAVE);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_CLEAVE))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }
    };

    bool OnGossipHello(Player* player, Creature* creature)
    {
        InstanceScript *instance = creature->GetInstanceScript();
        if (!instance)
            return false;

        if (player->HasAura(SPELL_RECENTLY_IN_GAME) || creature->HasAura(SPELL_CONTROL_PIECE))
            return true;

        if (instance->GetData(TYPE_CHESS) != DONE && player->GetTeam() == HORDE || ((instance_karazhan::instance_karazhan_InstanceMapScript *)instance)->IsFriendlyGameReady())
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_WARCHIEF_BLACKHAND, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(GOSSIP_MENU_ID_BLACKHAND, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        return PieceGen::OnGossipSelect(player, creature, sender, action);
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_warchief_blackhandAI(creature);
    }
};

/*######
## npc_human_conjurer
######*/

class npc_human_conjurer : public CreatureScript, PieceGen
{
public:
    npc_human_conjurer() : CreatureScript("npc_human_conjurer") { }

    struct npc_human_conjurerAI : public npc_chess_piece_genericAI
    {
        npc_human_conjurerAI(Creature* creature) : npc_chess_piece_genericAI(creature) { Reset(); }

        void Reset() { npc_chess_piece_genericAI::Reset(); }
        void MoveInLineOfSight(Unit* pWho) override { npc_chess_piece_genericAI::MoveInLineOfSight(pWho); }
        void AttackStart(Unit* pWho) override { npc_chess_piece_genericAI::AttackStart(pWho); }
        void EnterEvadeMode() override {npc_chess_piece_genericAI::EnterEvadeMode(); }
        void SetGUID(uint64 guid, int32 type/* = 0 */) { npc_chess_piece_genericAI::SetGUID(guid, type); }
        uint64 GetGUID(int32 id = 0) const { return npc_chess_piece_genericAI::GetGUID(id); }
        void DoAction(int32 const eventType) override { npc_chess_piece_genericAI::DoAction(eventType); }
        void MovementInform(uint32 uiMotionType, uint32 uiPointId) override { npc_chess_piece_genericAI::MovementInform(uiMotionType, uiPointId); }
        void SpellHit(Unit* pCaster, SpellInfo const* pSpell) override { npc_chess_piece_genericAI::SpellHit(pCaster, pSpell); }
        void UpdateAI(const uint32 diff) override { npc_chess_piece_genericAI::UpdateAI(diff); }
        void DamageTaken(Unit* pDoneBy, uint32& uiDamage) override { npc_chess_piece_genericAI::DamageTaken(pDoneBy, uiDamage); }

        void JustDied(Unit* pKiller) override
        {
            npc_chess_piece_genericAI::JustDied(pKiller);

            if (!instance)
                return;

            Creature* pMedivh = Unit::GetCreature(*me, instance->GetData64(NPC_ECHO_MEDIVH));
            if (!pMedivh)
                return;

            if (instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                pMedivh->AI()->Talk(TALK_PLAYER_LOOSE_PIECE_QUEEN);
            else
                pMedivh->AI()->Talk(TALK_MEDHIV_LOOSE_PIECE_QUEEN);

            ((instance_karazhan::instance_karazhan_InstanceMapScript *)instance)->DoMoveChessPieceToSides(SPELL_TRANSFORM_CONJURER, FACTION_ID_CHESS_ALLIANCE);
        }

        uint32 DoCastPrimarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 20.0f))
            {
                DoCast(pTarget, SPELL_ELEMENTAL_BLAST);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_ELEMENTAL_BLAST))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }

        uint32 DoCastSecondarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 25.0f))
            {
                DoCast(pTarget, SPELL_RAIN_OF_FIRE);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_RAIN_OF_FIRE))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }
    };

    bool OnGossipHello(Player* player, Creature* creature)
    {
        InstanceScript *instance = creature->GetInstanceScript();
        if (!instance)
            return false;

        if (player->HasAura(SPELL_RECENTLY_IN_GAME) || creature->HasAura(SPELL_CONTROL_PIECE))
            return true;

        if ((instance->GetData(TYPE_CHESS) == IN_PROGRESS && player->GetTeam() == ALLIANCE) || instance->GetData(TYPE_CHESS) == SPECIAL)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_HUMAN_CONJURER, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(GOSSIP_MENU_ID_CONJURER, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        return PieceGen::OnGossipSelect(player, creature, sender, action);
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_human_conjurerAI(creature);
    }
};

/*######
## npc_orc_warlock
######*/

class npc_orc_warlock : public CreatureScript, PieceGen
{
public:
    npc_orc_warlock() : CreatureScript("npc_orc_warlock") { }

    struct npc_orc_warlockAI : public npc_chess_piece_genericAI
    {
        npc_orc_warlockAI(Creature* creature) : npc_chess_piece_genericAI(creature) { }

        void Reset() { npc_chess_piece_genericAI::Reset(); }
        void MoveInLineOfSight(Unit* pWho) override { npc_chess_piece_genericAI::MoveInLineOfSight(pWho); }
        void AttackStart(Unit* pWho) override { npc_chess_piece_genericAI::AttackStart(pWho); }
        void EnterEvadeMode() override {npc_chess_piece_genericAI::EnterEvadeMode(); }
        void SetGUID(uint64 guid, int32 type/* = 0 */) { npc_chess_piece_genericAI::SetGUID(guid, type); }
        uint64 GetGUID(int32 id = 0) const { return npc_chess_piece_genericAI::GetGUID(id); }
        void DoAction(int32 const eventType) override { npc_chess_piece_genericAI::DoAction(eventType); }
        void MovementInform(uint32 uiMotionType, uint32 uiPointId) override { npc_chess_piece_genericAI::MovementInform(uiMotionType, uiPointId); }
        void SpellHit(Unit* pCaster, SpellInfo const* pSpell) override { npc_chess_piece_genericAI::SpellHit(pCaster, pSpell); }
        void UpdateAI(const uint32 diff) override { npc_chess_piece_genericAI::UpdateAI(diff); }
        void DamageTaken(Unit* pDoneBy, uint32& uiDamage) override { npc_chess_piece_genericAI::DamageTaken(pDoneBy, uiDamage); }

        void JustDied(Unit* pKiller) override
        {
            npc_chess_piece_genericAI::JustDied(pKiller);

            if (!instance)
                return;

            Creature* pMedivh = Unit::GetCreature(*me, instance->GetData64(NPC_ECHO_MEDIVH));
            if (!pMedivh)
                return;

            if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                pMedivh->AI()->Talk(TALK_PLAYER_LOOSE_PIECE_QUEEN);
            else
                pMedivh->AI()->Talk(TALK_MEDHIV_LOOSE_PIECE_QUEEN);

            ((instance_karazhan::instance_karazhan_InstanceMapScript *)instance)->DoMoveChessPieceToSides(SPELL_TRANSFORM_WARLOCK, FACTION_ID_CHESS_HORDE);
        }

        uint32 DoCastPrimarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 20.0f))
            {
                DoCast(pTarget, SPELL_FIREBALL);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_FIREBALL))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }

        uint32 DoCastSecondarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 25.0f))
            {
                DoCast(pTarget, SPELL_POISON_CLOUD_ACTION);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_POISON_CLOUD_ACTION))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }
    };


    bool OnGossipHello(Player* player, Creature* creature)
    {
        InstanceScript *instance = creature->GetInstanceScript();
        if (!instance)
            return false;

        if (player->HasAura(SPELL_RECENTLY_IN_GAME) || creature->HasAura(SPELL_CONTROL_PIECE))
            return true;

        if ((instance->GetData(TYPE_CHESS) == IN_PROGRESS && player->GetTeam() == HORDE) || instance->GetData(TYPE_CHESS) == SPECIAL)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ORC_WARLOCK, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(GOSSIP_MENU_ID_WARLOCK, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        return PieceGen::OnGossipSelect(player, creature, sender, action);
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_orc_warlockAI(creature);
    }
};

/*######
## npc_human_footman
######*/

class npc_human_footman : public CreatureScript, PieceGen
{
public:
    npc_human_footman() : CreatureScript("npc_human_footman") { }

    struct npc_human_footmanAI : public npc_chess_piece_genericAI
    {
        npc_human_footmanAI(Creature* creature) : npc_chess_piece_genericAI(creature) { Reset(); }

        void Reset() { npc_chess_piece_genericAI::Reset(); }
        void MoveInLineOfSight(Unit* pWho) override { npc_chess_piece_genericAI::MoveInLineOfSight(pWho); }
        void AttackStart(Unit* pWho) override { npc_chess_piece_genericAI::AttackStart(pWho); }
        void EnterEvadeMode() override {npc_chess_piece_genericAI::EnterEvadeMode(); }
        void SetGUID(uint64 guid, int32 type/* = 0 */) { npc_chess_piece_genericAI::SetGUID(guid, type); }
        uint64 GetGUID(int32 id = 0) const { return npc_chess_piece_genericAI::GetGUID(id); }
        void DoAction(int32 const eventType) override { npc_chess_piece_genericAI::DoAction(eventType); }
        void MovementInform(uint32 uiMotionType, uint32 uiPointId) override { npc_chess_piece_genericAI::MovementInform(uiMotionType, uiPointId); }
        void SpellHit(Unit* pCaster, SpellInfo const* pSpell) override { npc_chess_piece_genericAI::SpellHit(pCaster, pSpell); }
        void UpdateAI(const uint32 diff) override { npc_chess_piece_genericAI::UpdateAI(diff); }
        void DamageTaken(Unit* pDoneBy, uint32& uiDamage) override { npc_chess_piece_genericAI::DamageTaken(pDoneBy, uiDamage); }

        void JustDied(Unit* pKiller) override
        {
            npc_chess_piece_genericAI::JustDied(pKiller);

            if (!instance)
                return;

            Creature* pMedivh = Unit::GetCreature(*me, instance->GetData64(NPC_ECHO_MEDIVH));
            if (!pMedivh)
                return;

            if (instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                pMedivh->AI()->Talk(TALK_PLAYER_LOOSE_PIECE_PAWN);
            else
                pMedivh->AI()->Talk(TALK_MEDHIV_LOOSE_PIECE_PAWN);

            ((instance_karazhan::instance_karazhan_InstanceMapScript *)instance)->DoMoveChessPieceToSides(SPELL_TRANSFORM_FOOTMAN, FACTION_ID_CHESS_ALLIANCE);
        }

        uint32 DoCastPrimarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 8.0f, M_PI / 12))
            {
                DoCast(me, SPELL_HEROIC_BLOW);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_HEROIC_BLOW))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }

        uint32 DoCastSecondarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 8.0f))
            {
                DoCast(me, SPELL_SHIELD_BLOCK);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_SHIELD_BLOCK))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }
    };

    bool OnGossipHello(Player* player, Creature* creature)
    {
        InstanceScript *instance = creature->GetInstanceScript();
        if (!instance)
            return false;

        if (player->HasAura(SPELL_RECENTLY_IN_GAME) || creature->HasAura(SPELL_CONTROL_PIECE))
            return true;

        if ((instance->GetData(TYPE_CHESS) == IN_PROGRESS && player->GetTeam() == ALLIANCE) || instance->GetData(TYPE_CHESS) == SPECIAL)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_HUMAN_FOOTMAN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(GOSSIP_MENU_ID_FOOTMAN, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        return PieceGen::OnGossipSelect(player, creature, sender, action);
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_human_footmanAI(creature);
    }

};

/*######
## npc_orc_grunt
######*/

class npc_orc_grunt : public CreatureScript, PieceGen
{
public:
    npc_orc_grunt() : CreatureScript("npc_orc_grunt") { }

    struct npc_orc_gruntAI : public npc_chess_piece_genericAI
    {
        npc_orc_gruntAI(Creature* creature) : npc_chess_piece_genericAI(creature) { }

        void Reset() { npc_chess_piece_genericAI::Reset(); }
        void MoveInLineOfSight(Unit* pWho) override { npc_chess_piece_genericAI::MoveInLineOfSight(pWho); }
        void AttackStart(Unit* pWho) override { npc_chess_piece_genericAI::AttackStart(pWho); }
        void EnterEvadeMode() override {npc_chess_piece_genericAI::EnterEvadeMode(); }
        void SetGUID(uint64 guid, int32 type/* = 0 */) { npc_chess_piece_genericAI::SetGUID(guid, type); }
        uint64 GetGUID(int32 id = 0) const { return npc_chess_piece_genericAI::GetGUID(id); }
        void DoAction(int32 const eventType) override { npc_chess_piece_genericAI::DoAction(eventType); }
        void MovementInform(uint32 uiMotionType, uint32 uiPointId) override { npc_chess_piece_genericAI::MovementInform(uiMotionType, uiPointId); }
        void SpellHit(Unit* pCaster, SpellInfo const* pSpell) override { npc_chess_piece_genericAI::SpellHit(pCaster, pSpell); }
        void UpdateAI(const uint32 diff) override { npc_chess_piece_genericAI::UpdateAI(diff); }
        void DamageTaken(Unit* pDoneBy, uint32& uiDamage) override { npc_chess_piece_genericAI::DamageTaken(pDoneBy, uiDamage); }

        void JustDied(Unit* pKiller) override
        {
            npc_chess_piece_genericAI::JustDied(pKiller);

            if (!instance)
                return;

            Creature* pMedivh = Unit::GetCreature(*me, instance->GetData64(NPC_ECHO_MEDIVH));
            if (!pMedivh)
                return;

            if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                pMedivh->AI()->Talk(TALK_PLAYER_LOOSE_PIECE_PAWN);
            else
                pMedivh->AI()->Talk(TALK_MEDHIV_LOOSE_PIECE_PAWN);

            ((instance_karazhan::instance_karazhan_InstanceMapScript *)instance)->DoMoveChessPieceToSides(SPELL_TRANSFORM_GRUNT, FACTION_ID_CHESS_HORDE);
        }

        uint32 DoCastPrimarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 8.0f, M_PI / 12))
            {
                DoCast(me, SPELL_VICIOUS_STRIKE);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_VICIOUS_STRIKE))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }

        uint32 DoCastSecondarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 8.0f))
            {
                DoCast(me, SPELL_WEAPON_DEFLECTION);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_WEAPON_DEFLECTION))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }
    };

    bool OnGossipHello(Player* player, Creature* creature)
    {
        InstanceScript *instance = creature->GetInstanceScript();
        if (!instance)
            return false;

        if (player->HasAura(SPELL_RECENTLY_IN_GAME) || creature->HasAura(SPELL_CONTROL_PIECE))
            return true;

        if ((instance->GetData(TYPE_CHESS) == IN_PROGRESS && player->GetTeam() == HORDE) || instance->GetData(TYPE_CHESS) == SPECIAL)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ORC_GRUNT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(GOSSIP_MENU_ID_GRUNT, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        return PieceGen::OnGossipSelect(player, creature, sender, action);
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_orc_gruntAI(creature);
    }
};

/*######
## npc_water_elemental
######*/

class npc_water_elemental : public CreatureScript, PieceGen
{
public:
    npc_water_elemental() : CreatureScript("npc_water_elemental") { }

    struct npc_water_elementalAI : public npc_chess_piece_genericAI
    {
        npc_water_elementalAI(Creature* creature) : npc_chess_piece_genericAI(creature) {  }

        void Reset() { npc_chess_piece_genericAI::Reset(); }
        void MoveInLineOfSight(Unit* pWho) override { npc_chess_piece_genericAI::MoveInLineOfSight(pWho); }
        void AttackStart(Unit* pWho) override { npc_chess_piece_genericAI::AttackStart(pWho); }
        void EnterEvadeMode() override {npc_chess_piece_genericAI::EnterEvadeMode(); }
        void SetGUID(uint64 guid, int32 type/* = 0 */) { npc_chess_piece_genericAI::SetGUID(guid, type); }
        uint64 GetGUID(int32 id = 0) const { return npc_chess_piece_genericAI::GetGUID(id); }
        void DoAction(int32 const eventType) override { npc_chess_piece_genericAI::DoAction(eventType); }
        void MovementInform(uint32 uiMotionType, uint32 uiPointId) override { npc_chess_piece_genericAI::MovementInform(uiMotionType, uiPointId); }
        void SpellHit(Unit* pCaster, SpellInfo const* pSpell) override { npc_chess_piece_genericAI::SpellHit(pCaster, pSpell); }
        void UpdateAI(const uint32 diff) override { npc_chess_piece_genericAI::UpdateAI(diff); }
        void DamageTaken(Unit* pDoneBy, uint32& uiDamage) override { npc_chess_piece_genericAI::DamageTaken(pDoneBy, uiDamage); }

        void JustDied(Unit* pKiller) override
        {
            npc_chess_piece_genericAI::JustDied(pKiller);

            if (!instance)
                return;

            Creature* pMedivh = Unit::GetCreature(*me, instance->GetData64(NPC_ECHO_MEDIVH));
            if (!pMedivh)
                return;

            if (instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                pMedivh->AI()->Talk(TALK_PLAYER_LOOSE_PIECE_ROCK);
            else
                pMedivh->AI()->Talk(TALK_MEDHIV_LOOSE_PIECE_ROCK);

            ((instance_karazhan::instance_karazhan_InstanceMapScript *)instance)->DoMoveChessPieceToSides(SPELL_TRANSFORM_WATER_ELEM, FACTION_ID_CHESS_ALLIANCE);
        }

        uint32 DoCastPrimarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 9.0f))
            {
                DoCast(me, SPELL_GEYSER);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_GEYSER))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }

        uint32 DoCastSecondarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 9.0f))
            {
                DoCast(me, SPELL_WATER_SHIELD);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_WATER_SHIELD))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }
    };

    bool OnGossipHello(Player* player, Creature* creature)
    {
        InstanceScript *instance = creature->GetInstanceScript();
        if (!instance)
            return false;

        if (player->HasAura(SPELL_RECENTLY_IN_GAME) || creature->HasAura(SPELL_CONTROL_PIECE))
            return true;

        if ((instance->GetData(TYPE_CHESS) == IN_PROGRESS && player->GetTeam() == ALLIANCE) || instance->GetData(TYPE_CHESS) == SPECIAL)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_WATER_ELEMENTAL, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(GOSSIP_MENU_ID_ELEMENTAL, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        return PieceGen::OnGossipSelect(player, creature, sender, action);
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_water_elementalAI(creature);
    }
};

/*######
## npc_summoned_daemon
######*/

class npc_summoned_daemon : public CreatureScript, PieceGen
{
public:
    npc_summoned_daemon() : CreatureScript("npc_summoned_daemon") { }

    struct npc_summoned_daemonAI : public npc_chess_piece_genericAI
    {
        npc_summoned_daemonAI(Creature* creature) : npc_chess_piece_genericAI(creature) {  }

        void Reset() { npc_chess_piece_genericAI::Reset(); }
        void MoveInLineOfSight(Unit* pWho) override { npc_chess_piece_genericAI::MoveInLineOfSight(pWho); }
        void AttackStart(Unit* pWho) override { npc_chess_piece_genericAI::AttackStart(pWho); }
        void EnterEvadeMode() override {npc_chess_piece_genericAI::EnterEvadeMode(); }
        void SetGUID(uint64 guid, int32 type/* = 0 */) { npc_chess_piece_genericAI::SetGUID(guid, type); }
        uint64 GetGUID(int32 id = 0) const { return npc_chess_piece_genericAI::GetGUID(id); }
        void DoAction(int32 const eventType) override { npc_chess_piece_genericAI::DoAction(eventType); }
        void MovementInform(uint32 uiMotionType, uint32 uiPointId) override { npc_chess_piece_genericAI::MovementInform(uiMotionType, uiPointId); }
        void SpellHit(Unit* pCaster, SpellInfo const* pSpell) override { npc_chess_piece_genericAI::SpellHit(pCaster, pSpell); }
        void UpdateAI(const uint32 diff) override { npc_chess_piece_genericAI::UpdateAI(diff); }
        void DamageTaken(Unit* pDoneBy, uint32& uiDamage) override { npc_chess_piece_genericAI::DamageTaken(pDoneBy, uiDamage); }

        void JustDied(Unit* pKiller) override
        {
            npc_chess_piece_genericAI::JustDied(pKiller);

            if (!instance)
                return;

            Creature* pMedivh = Unit::GetCreature(*me, instance->GetData64(NPC_ECHO_MEDIVH));
            if (!pMedivh)
                return;

            if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                pMedivh->AI()->Talk(TALK_PLAYER_LOOSE_PIECE_ROCK);
            else
                pMedivh->AI()->Talk(TALK_MEDHIV_LOOSE_PIECE_ROCK);

            ((instance_karazhan::instance_karazhan_InstanceMapScript *)instance)->DoMoveChessPieceToSides(SPELL_TRANSFORM_DAEMON, FACTION_ID_CHESS_HORDE);
        }

        uint32 DoCastPrimarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 9.0f))
            {
                DoCast(me, SPELL_HELLFIRE);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_HELLFIRE))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }

        uint32 DoCastSecondarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 9.0f))
            {
                DoCast(me, SPELL_FIRE_SHIELD);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_FIRE_SHIELD))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }
    };

    bool OnGossipHello(Player* player, Creature* creature)
    {
        InstanceScript *instance = creature->GetInstanceScript();
        if (!instance)
            return false;

        if (player->HasAura(SPELL_RECENTLY_IN_GAME) || creature->HasAura(SPELL_CONTROL_PIECE))
            return true;

        if ((instance->GetData(TYPE_CHESS) == IN_PROGRESS && player->GetTeam() == HORDE) || instance->GetData(TYPE_CHESS) == SPECIAL)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_SUMMONED_DEAMON, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(GOSSIP_MENU_ID_DEAMON, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        return PieceGen::OnGossipSelect(player, creature, sender, action);
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_summoned_daemonAI(creature);
    }
};

/*######
## npc_human_charger
######*/

class npc_human_charger : public CreatureScript, PieceGen
{
public:
    npc_human_charger() : CreatureScript("npc_human_charger") { }

    struct npc_human_chargerAI : public npc_chess_piece_genericAI
    {
        npc_human_chargerAI(Creature* creature) : npc_chess_piece_genericAI(creature) {  }

        void Reset() { npc_chess_piece_genericAI::Reset(); }
        void MoveInLineOfSight(Unit* pWho) override { npc_chess_piece_genericAI::MoveInLineOfSight(pWho); }
        void AttackStart(Unit* pWho) override { npc_chess_piece_genericAI::AttackStart(pWho); }
        void EnterEvadeMode() override {npc_chess_piece_genericAI::EnterEvadeMode(); }
        void SetGUID(uint64 guid, int32 type/* = 0 */) { npc_chess_piece_genericAI::SetGUID(guid, type); }
        uint64 GetGUID(int32 id = 0) const { return npc_chess_piece_genericAI::GetGUID(id); }
        void DoAction(int32 const eventType) override { npc_chess_piece_genericAI::DoAction(eventType); }
        void MovementInform(uint32 uiMotionType, uint32 uiPointId) override { npc_chess_piece_genericAI::MovementInform(uiMotionType, uiPointId); }
        void SpellHit(Unit* pCaster, SpellInfo const* pSpell) override { npc_chess_piece_genericAI::SpellHit(pCaster, pSpell); }
        void UpdateAI(const uint32 diff) override { npc_chess_piece_genericAI::UpdateAI(diff); }
        void DamageTaken(Unit* pDoneBy, uint32& uiDamage) override { npc_chess_piece_genericAI::DamageTaken(pDoneBy, uiDamage); }

        void JustDied(Unit* pKiller) override
        {
            npc_chess_piece_genericAI::JustDied(pKiller);

            if (!instance)
                return;

            Creature* pMedivh = Unit::GetCreature(*me, instance->GetData64(NPC_ECHO_MEDIVH));
            if (!pMedivh)
                return;

            if (instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                pMedivh->AI()->Talk(TALK_PLAYER_LOOSE_PIECE_KNIGHT);
            else
                pMedivh->AI()->Talk(TALK_MEDHIV_LOOSE_PIECE_KNIGHT);

            ((instance_karazhan::instance_karazhan_InstanceMapScript *)instance)->DoMoveChessPieceToSides(SPELL_TRANSFORM_CHARGER, FACTION_ID_CHESS_ALLIANCE);
        }

        uint32 DoCastPrimarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 8.0f, M_PI / 12))
            {
                DoCast(me, SPELL_SMASH);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_SMASH))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }

        uint32 DoCastSecondarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 10.0f, M_PI / 12))
            {
                DoCast(me, SPELL_STOMP);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_STOMP))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }
    };

    bool OnGossipHello(Player* player, Creature* creature)
    {
        InstanceScript *instance = creature->GetInstanceScript();
        if (!instance)
            return false;

        if (player->HasAura(SPELL_RECENTLY_IN_GAME) || creature->HasAura(SPELL_CONTROL_PIECE))
            return true;

        if ((instance->GetData(TYPE_CHESS) == IN_PROGRESS && player->GetTeam() == ALLIANCE) || instance->GetData(TYPE_CHESS) == SPECIAL)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_HUMAN_CHARGER, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(GOSSIP_MENU_ID_CHARGER, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        return PieceGen::OnGossipSelect(player, creature, sender, action);
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_human_chargerAI(creature);
    }
};

/*######
## npc_orc_wolf
######*/

class npc_orc_wolf : public CreatureScript, PieceGen
{
public:
    npc_orc_wolf() : CreatureScript("npc_orc_wolf") { }

    struct npc_orc_wolfAI : public npc_chess_piece_genericAI
    {
        npc_orc_wolfAI(Creature* creature) : npc_chess_piece_genericAI(creature) {  }

        void Reset() { npc_chess_piece_genericAI::Reset(); }
        void MoveInLineOfSight(Unit* pWho) override { npc_chess_piece_genericAI::MoveInLineOfSight(pWho); }
        void AttackStart(Unit* pWho) override { npc_chess_piece_genericAI::AttackStart(pWho); }
        void EnterEvadeMode() override {npc_chess_piece_genericAI::EnterEvadeMode(); }
        void SetGUID(uint64 guid, int32 type/* = 0 */) { npc_chess_piece_genericAI::SetGUID(guid, type); }
        uint64 GetGUID(int32 id = 0) const { return npc_chess_piece_genericAI::GetGUID(id); }
        void DoAction(int32 const eventType) override { npc_chess_piece_genericAI::DoAction(eventType); }
        void MovementInform(uint32 uiMotionType, uint32 uiPointId) override { npc_chess_piece_genericAI::MovementInform(uiMotionType, uiPointId); }
        void SpellHit(Unit* pCaster, SpellInfo const* pSpell) override { npc_chess_piece_genericAI::SpellHit(pCaster, pSpell); }
        void UpdateAI(const uint32 diff) override { npc_chess_piece_genericAI::UpdateAI(diff); }
        void DamageTaken(Unit* pDoneBy, uint32& uiDamage) override { npc_chess_piece_genericAI::DamageTaken(pDoneBy, uiDamage); }

        void JustDied(Unit* pKiller) override
        {
            npc_chess_piece_genericAI::JustDied(pKiller);

            if (!instance)
                return;

            Creature* pMedivh = Unit::GetCreature(*me, instance->GetData64(NPC_ECHO_MEDIVH));
            if (!pMedivh)
                return;

            if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                pMedivh->AI()->Talk(TALK_PLAYER_LOOSE_PIECE_KNIGHT);
            else
                pMedivh->AI()->Talk(TALK_MEDHIV_LOOSE_PIECE_KNIGHT);

            ((instance_karazhan::instance_karazhan_InstanceMapScript *)instance)->DoMoveChessPieceToSides(SPELL_TRANSFORM_WOLF, FACTION_ID_CHESS_HORDE);
        }

        uint32 DoCastPrimarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 8.0f, M_PI / 12))
            {
                DoCast(me, SPELL_BITE);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_BITE))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }

        uint32 DoCastSecondarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 10.0f, M_PI / 12))
            {
                DoCast(me, SPELL_HOWL);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_HOWL))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }
    };

    bool OnGossipHello(Player* player, Creature* creature)
    {
        InstanceScript *instance = creature->GetInstanceScript();
        if (!instance)
            return false;

        if (player->HasAura(SPELL_RECENTLY_IN_GAME) || creature->HasAura(SPELL_CONTROL_PIECE))
            return true;

        if ((instance->GetData(TYPE_CHESS) == IN_PROGRESS && player->GetTeam() == HORDE) || instance->GetData(TYPE_CHESS) == SPECIAL)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ORC_WOLF, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(GOSSIP_MENU_ID_WOLF, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        return PieceGen::OnGossipSelect(player, creature, sender, action);
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_orc_wolfAI(creature);
    }
};

/*######
## npc_human_cleric
######*/

class npc_human_cleric : public CreatureScript, PieceGen
{
public:
    npc_human_cleric() : CreatureScript("npc_human_cleric") { }

    struct npc_human_clericAI : public npc_chess_piece_genericAI
    {
        npc_human_clericAI(Creature* creature) : npc_chess_piece_genericAI(creature) {  }

        void Reset() { npc_chess_piece_genericAI::Reset(); }
        void MoveInLineOfSight(Unit* pWho) override { npc_chess_piece_genericAI::MoveInLineOfSight(pWho); }
        void AttackStart(Unit* pWho) override { npc_chess_piece_genericAI::AttackStart(pWho); }
        void EnterEvadeMode() override {npc_chess_piece_genericAI::EnterEvadeMode(); }
        void SetGUID(uint64 guid, int32 type/* = 0 */) { npc_chess_piece_genericAI::SetGUID(guid, type); }
        uint64 GetGUID(int32 id = 0) const { return npc_chess_piece_genericAI::GetGUID(id); }
        void DoAction(int32 const eventType) override { npc_chess_piece_genericAI::DoAction(eventType); }
        void MovementInform(uint32 uiMotionType, uint32 uiPointId) override { npc_chess_piece_genericAI::MovementInform(uiMotionType, uiPointId); }
        void SpellHit(Unit* pCaster, SpellInfo const* pSpell) override { npc_chess_piece_genericAI::SpellHit(pCaster, pSpell); }
        void UpdateAI(const uint32 diff) override { npc_chess_piece_genericAI::UpdateAI(diff); }
        void DamageTaken(Unit* pDoneBy, uint32& uiDamage) override { npc_chess_piece_genericAI::DamageTaken(pDoneBy, uiDamage); }

        void JustDied(Unit* pKiller) override
        {
            npc_chess_piece_genericAI::JustDied(pKiller);

            if (!instance)
                return;

            Creature* pMedivh = Unit::GetCreature(*me, instance->GetData64(NPC_ECHO_MEDIVH));
            if (!pMedivh)
                return;

            if (instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                pMedivh->AI()->Talk(TALK_PLAYER_LOOSE_PIECE_BISHOP);
            else
                pMedivh->AI()->Talk(TALK_MEDHIV_LOOSE_PIECE_BISHOP);

            ((instance_karazhan::instance_karazhan_InstanceMapScript *)instance)->DoMoveChessPieceToSides(SPELL_TRANSFORM_CLERIC, FACTION_ID_CHESS_ALLIANCE);
        }

        uint32 DoCastPrimarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_FRIENDLY, 25.0f))
            {
                DoCast(pTarget, SPELL_HEALING);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_HEALING))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }

        uint32 DoCastSecondarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 18.0f, M_PI / 12))
            {
                DoCast(me, SPELL_HOLY_LANCE);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_HOLY_LANCE))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }
    };

    bool OnGossipHello(Player* player, Creature* creature)
    {
        InstanceScript *instance = creature->GetInstanceScript();
        if (!instance)
            return false;

        if (player->HasAura(SPELL_RECENTLY_IN_GAME) || creature->HasAura(SPELL_CONTROL_PIECE))
            return true;

        if ((instance->GetData(TYPE_CHESS) == IN_PROGRESS && player->GetTeam() == ALLIANCE) || instance->GetData(TYPE_CHESS) == SPECIAL)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_HUMAN_CLERIC, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(GOSSIP_MENU_ID_CLERIC, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        return PieceGen::OnGossipSelect(player, creature, sender, action);
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_human_clericAI(creature);
    }
};

/*######
## npc_orc_necrolyte
######*/

class npc_orc_necrolyte : public CreatureScript, PieceGen
{
public:
    npc_orc_necrolyte() : CreatureScript("npc_orc_necrolyte") { }

    struct npc_orc_necrolyteAI : public npc_chess_piece_genericAI
    {
        npc_orc_necrolyteAI(Creature* creature) : npc_chess_piece_genericAI(creature) { }

        void Reset() { npc_chess_piece_genericAI::Reset(); }
        void MoveInLineOfSight(Unit* pWho) override { npc_chess_piece_genericAI::MoveInLineOfSight(pWho); }
        void AttackStart(Unit* pWho) override { npc_chess_piece_genericAI::AttackStart(pWho); }
        void EnterEvadeMode() override {npc_chess_piece_genericAI::EnterEvadeMode(); }
        void SetGUID(uint64 guid, int32 type/* = 0 */) { npc_chess_piece_genericAI::SetGUID(guid, type); }
        uint64 GetGUID(int32 id = 0) const { return npc_chess_piece_genericAI::GetGUID(id); }
        void DoAction(int32 const eventType) override { npc_chess_piece_genericAI::DoAction(eventType); }
        void MovementInform(uint32 uiMotionType, uint32 uiPointId) override { npc_chess_piece_genericAI::MovementInform(uiMotionType, uiPointId); }
        void SpellHit(Unit* pCaster, SpellInfo const* pSpell) override { npc_chess_piece_genericAI::SpellHit(pCaster, pSpell); }
        void UpdateAI(const uint32 diff) override { npc_chess_piece_genericAI::UpdateAI(diff); }
        void DamageTaken(Unit* pDoneBy, uint32& uiDamage) override { npc_chess_piece_genericAI::DamageTaken(pDoneBy, uiDamage); }

        void JustDied(Unit* pKiller) override
        {
            npc_chess_piece_genericAI::JustDied(pKiller);

            if (!instance)
                return;

            Creature* pMedivh = Unit::GetCreature(*me, instance->GetData64(NPC_ECHO_MEDIVH));
            if (!pMedivh)
                return;

            if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                pMedivh->AI()->Talk(TALK_PLAYER_LOOSE_PIECE_BISHOP);
            else
                pMedivh->AI()->Talk(TALK_MEDHIV_LOOSE_PIECE_BISHOP);

            ((instance_karazhan::instance_karazhan_InstanceMapScript *)instance)->DoMoveChessPieceToSides(SPELL_TRANSFORM_NECROLYTE, FACTION_ID_CHESS_HORDE);
        }

        uint32 DoCastPrimarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_FRIENDLY, 25.0f))
            {
                DoCast(pTarget, SPELL_SHADOW_MEND_ACTION);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_SHADOW_MEND_ACTION))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }

        uint32 DoCastSecondarySpell() override
        {
            if (Unit* pTarget = GetTargetByType(TARGET_TYPE_RANDOM, 18.0f, M_PI / 12))
            {
                DoCast(me, SPELL_SHADOW_SPEAR);

                // reset timer based on spell values
                if (SpellInfo const* pSpell = sSpellMgr->GetSpellInfo(SPELL_SHADOW_SPEAR))
                    return pSpell->GetRecoveryTime();
            }

            return 5000;
        }
    };

    bool OnGossipHello(Player* player, Creature* creature)
    {
        InstanceScript *instance = creature->GetInstanceScript();
        if (!instance)
            return false;

        if (player->HasAura(SPELL_RECENTLY_IN_GAME) || creature->HasAura(SPELL_CONTROL_PIECE))
            return true;

        if ((instance->GetData(TYPE_CHESS) == IN_PROGRESS && player->GetTeam() == HORDE) || instance->GetData(TYPE_CHESS) == SPECIAL)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ORC_NECROLYTE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(GOSSIP_MENU_ID_NECROLYTE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        return PieceGen::OnGossipSelect(player, creature, sender, action);
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_orc_necrolyteAI(creature);
    }

};

class spell_move_piece : public SpellScriptLoader
{
public:
    spell_move_piece() : SpellScriptLoader("spell_move_piece") { }

    class spell_move_piece_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_move_piece_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if([](WorldObject* target)
                              {
                                  if (target->GetTypeId() != TYPEID_UNIT)
                                      return true;
                                  bool removed = target->GetEntry() != NPC_SQUARE_BLACK && target->GetEntry() != NPC_SQUARE_WHITE;
                                  if (!removed)
                                      if (InstanceScript *instance = target->GetInstanceScript())
                                          if (((instance_karazhan::instance_karazhan_InstanceMapScript *)instance)->IsCaseOccupied(target->ToCreature()))
                                              return true;
                                  return removed;
                              });
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit *caster = GetCaster())
                if (Unit *target = GetHitUnit())
                {
                    target->CastSpell(target, SPELL_DISABLE_SQUARE, true);
                    target->CastSpell(target, SPELL_IS_SQUARE_USED, true);
                    caster->CastSpell(caster, SPELL_MOVE_COOLDOWN, true);
                    if (caster->GetTypeId() == TYPEID_UNIT)
                    {
                        caster->ToCreature()->AI()->SetGUID(target->GetGUID());
                        caster->ToCreature()->AI()->DoAction(AI_EVENT_CUSTOM_A);
                    }
                }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_move_piece_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_move_piece_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_move_piece_SpellScript();
    }
};

class spell_control_piece : public SpellScriptLoader
{
public:
    spell_control_piece() : SpellScriptLoader("spell_control_piece") { }

    class spell_control_piece_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_control_piece_AuraScript);

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* target = GetUnitOwner())
            {
                target->RemoveAurasDueToSpell(SPELL_IN_GAME);
                target->CastSpell(target, SPELL_RECENTLY_IN_GAME, true);
            }
        }

        void Register() override
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_control_piece_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_control_piece_AuraScript();
    }
};

class npc_chess_square : public CreatureScript
{
public:
    npc_chess_square() : CreatureScript("npc_chess_square") { }

    struct npc_chess_squareAI : public ScriptedAI
    {
        npc_chess_squareAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void Reset() { }

        void UpdateAI(const uint32 diff) override { }

        void DamageTaken(Unit* , uint32& uiDamage) override
        {
            uiDamage = 0;
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_chess_squareAI(creature);
    }
};

class spell_change_piece_facing : public SpellScriptLoader
{
public:
    spell_change_piece_facing() : SpellScriptLoader("spell_change_piece_facing") { }

    class spell_change_piece_facing_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_change_piece_facing_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if([](WorldObject* target)
                              {
                                  if (target->GetTypeId() != TYPEID_UNIT)
                                      return true;
                                  return (target->GetEntry() != NPC_SQUARE_BLACK && target->GetEntry() != NPC_SQUARE_WHITE);
                              });
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit *caster = GetCaster())
                if (Unit *target = GetHitUnit())
                {
                    float newOrientation = caster->GetAngle(target);
                    for (int i = 0; i < 4; i++ )
                    {
                        if (facingAngle[i] >= newOrientation)
                        {
                            if (i == 0)
                                newOrientation = facingAngle[i];
                            else
                                newOrientation = (abs(newOrientation - facingAngle[i]) < abs(newOrientation - facingAngle[i - 1])) ?
                                    facingAngle[i] : facingAngle[i - 1];
                            break;
                        }
                    }
                    caster->SetFacingTo(newOrientation);
                }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_change_piece_facing_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_change_piece_facing_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_change_piece_facing_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_change_piece_facing_SpellScript();
    }
};

uint32 hordePieceEntrys[6] = {
    NPC_ORC_GRUNT,
    NPC_ORC_WOLF,
    NPC_ORC_WARLOCK,
    NPC_ORC_NECROLYTE,
    NPC_SUMMONED_DAEMON,
    NPC_WARCHIEF_BLACKHAND,
};

uint32 alliancePieceEntrys[6] = {
    NPC_HUMAN_FOOTMAN,
    NPC_HUMAN_CHARGER,
    NPC_HUMAN_CONJURER,
    NPC_HUMAN_CLERIC,
    NPC_CONJURED_WATER_ELEMENTAL,
    NPC_KING_LLANE,
};

class spell_piece_damage_filters_alliance_01 : public SpellScriptLoader
{
public:
    spell_piece_damage_filters_alliance_01() : SpellScriptLoader("spell_piece_damage_filters_alliance_01") { }

    class spell_piece_damage_filters_alliance_01_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_piece_damage_filters_alliance_01_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            Unit *caster = GetCaster();
            uint32 spellId = GetSpellInfo()->Id;
            targets.remove_if([caster, spellId](WorldObject* target)
                              {
                                  if (target->GetTypeId() != TYPEID_UNIT)
                                      return true;
                                  for (uint8 i = 0; i < 6; i++)
                                      if (target->GetEntry() == alliancePieceEntrys[i])
                                          return false;
                                  switch (spellId)
                                  {
                                      case SPELL_CLEAVE:
                                          if (!caster->isInFrontInMap(target->ToUnit(), 50.0f))
                                              return true;
                                          break;
                                  }
                                  return true;
                              });
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_piece_damage_filters_alliance_01_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_piece_damage_filters_alliance_01_SpellScript();
    }
};

class spell_piece_damage_filters_alliance_02 : public SpellScriptLoader
{
public:
    spell_piece_damage_filters_alliance_02() : SpellScriptLoader("spell_piece_damage_filters_alliance_02") { }

    class spell_piece_damage_filters_alliance_02_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_piece_damage_filters_alliance_02_SpellScript);

        // 37413 alliance TARGET_UNIT_CONE_ENTRY
        // 37454 alliance TARGET_UNIT_CONE_ENTRY
        // 37502 alliance TARGET_UNIT_CONE_ENTRY
        // 37461 alliance TARGET_UNIT_CONE_ENTRY

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if([](WorldObject* target)
                              {
                                  for (uint8 i = 0; i < 6; i++)
                                      if (target->GetEntry() == alliancePieceEntrys[i])
                                          return false;
                                  return true;
                              });
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_piece_damage_filters_alliance_02_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_piece_damage_filters_alliance_02_SpellScript();
    }
};

class spell_piece_damage_filters_alliance_03 : public SpellScriptLoader
{
public:
    spell_piece_damage_filters_alliance_03() : SpellScriptLoader("spell_piece_damage_filters_alliance_03") { }

    class spell_piece_damage_filters_alliance_03_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_piece_damage_filters_alliance_03_SpellScript);

        // 37469 alliance TARGET_UNIT_NEARBY_ENTRY
        // 37775 alliance TARGET_UNIT_NEARBY_ENTRY

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if([](WorldObject* target)
                              {
                                  for (uint8 i = 0; i < 6; i++)
                                      if (target->GetEntry() == alliancePieceEntrys[i])
                                          return false;
                                  return true;
                              });
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_piece_damage_filters_alliance_03_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_NEARBY_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_piece_damage_filters_alliance_03_SpellScript();
    }
};

class spell_piece_damage_filters_horde_01 : public SpellScriptLoader
{
public:
    spell_piece_damage_filters_horde_01() : SpellScriptLoader("spell_piece_damage_filters_horde_01") { }

    class spell_piece_damage_filters_horde_01_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_piece_damage_filters_horde_01_SpellScript);

        // 37474 horde
        // 37472 horde
        // 37406 horde
        // 37824 horde 1 target
        // 37427 horde

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            // 37474
            Unit *caster = GetCaster();
            uint32 spellId = GetSpellInfo()->Id;
            targets.remove_if([caster, spellId](WorldObject* target)
                              {
                                  if (target->GetTypeId() != TYPEID_UNIT)
                                      return true;
                                  for (uint8 i = 0; i < 6; i++)
                                      if (target->GetEntry() == hordePieceEntrys[i])
                                          return false;
                                  switch (spellId)
                                  {
                                      case SPELL_SWEEP:
                                          if (!caster->isInFrontInMap(target->ToUnit(), 50.0f))
                                              return true;
                                          break;

                                  }
                                  return true;
                              });
        }

        void Register()
        {
            if (m_scriptSpellId == 1234)
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_piece_damage_filters_horde_01_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENTRY);
            else
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_piece_damage_filters_horde_01_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_piece_damage_filters_horde_01_SpellScript();
    }
};

class spell_piece_damage_filters_horde_02 : public SpellScriptLoader
{
public:
    spell_piece_damage_filters_horde_02() : SpellScriptLoader("spell_piece_damage_filters_horde_02") { }

    class spell_piece_damage_filters_horde_02_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_piece_damage_filters_horde_02_SpellScript);

        // 37453 horde TARGET_UNIT_CONE_ENTRY
        // 37498 horde TARGET_UNIT_CONE_ENTRY
        // 37459 horde TARGET_UNIT_CONE_ENTRY

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if([](WorldObject* target)
                              {
                                  for (uint8 i = 0; i < 6; i++)
                                      if (target->GetEntry() == hordePieceEntrys[i])
                                          return false;
                                  return true;
                              });
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_piece_damage_filters_horde_02_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_piece_damage_filters_horde_02_SpellScript();
    }
};

class spell_piece_damage_filters_horde_03 : public SpellScriptLoader
{
public:
    spell_piece_damage_filters_horde_03() : SpellScriptLoader("spell_piece_damage_filters_horde_03") { }

    class spell_piece_damage_filters_horde_03_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_piece_damage_filters_horde_03_SpellScript);

        // 37465 horde TARGET_UNIT_NEARBY_ENTRY

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if([](WorldObject* target)
                              {
                                  for (uint8 i = 0; i < 6; i++)
                                      if (target->GetEntry() == hordePieceEntrys[i])
                                          return false;
                                  return true;
                              });
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_piece_damage_filters_horde_03_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_NEARBY_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_piece_damage_filters_horde_03_SpellScript();
    }
};

class spell_piece_auto_melee_attack : public SpellScriptLoader
{
public:
    spell_piece_auto_melee_attack() : SpellScriptLoader("spell_piece_auto_melee_attack") { }

    class spell_piece_auto_melee_attack_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_piece_auto_melee_attack_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit *caster = GetCaster())
                if (Unit *target = GetHitUnit())
                {
                    uint32 uiMeleeSpell = 0;

                    switch (target->GetEntry())
                    {
                        case NPC_KING_LLANE:
                            uiMeleeSpell = SPELL_MELEE_KING_LLANE;
                            break;
                        case NPC_HUMAN_CHARGER:
                            uiMeleeSpell = SPELL_MELEE_CHARGER;
                            break;
                        case NPC_HUMAN_CLERIC:
                            uiMeleeSpell = SPELL_MELEE_CLERIC;
                            break;
                        case NPC_HUMAN_CONJURER:
                            uiMeleeSpell = SPELL_MELEE_CONJURER;
                            break;
                        case NPC_HUMAN_FOOTMAN:
                            uiMeleeSpell = SPELL_MELEE_FOOTMAN;
                            break;
                        case NPC_CONJURED_WATER_ELEMENTAL:
                            uiMeleeSpell = SPELL_MELEE_WATER_ELEM;
                            break;
                        case NPC_WARCHIEF_BLACKHAND:
                            uiMeleeSpell = SPELL_MELEE_WARCHIEF_BLACKHAND;
                            break;
                        case NPC_ORC_GRUNT:
                            uiMeleeSpell = SPELL_MELEE_GRUNT;
                            break;
                        case NPC_ORC_NECROLYTE:
                            uiMeleeSpell = SPELL_MELEE_NECROLYTE;
                            break;
                        case NPC_ORC_WARLOCK:
                            uiMeleeSpell = SPELL_MELEE_WARLOCK;
                            break;
                        case NPC_ORC_WOLF:
                            uiMeleeSpell = SPELL_MELEE_WOLF;
                            break;
                        case NPC_SUMMONED_DAEMON:
                            uiMeleeSpell = SPELL_MELEE_DAEMON;
                            break;
                    }

                    target->CastSpell(target, uiMeleeSpell, true);
                }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_piece_auto_melee_attack_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_piece_auto_melee_attack_SpellScript();
    }
};

class spell_piece_melee_attack : public SpellScriptLoader
{
public:
    spell_piece_melee_attack() : SpellScriptLoader("spell_piece_melee_attack") { }

    class spell_piece_melee_attack_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_piece_melee_attack_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            uint32 spellId = GetSpellInfo()->Id;
            Unit *caster = GetCaster();
            targets.remove_if([caster, spellId](WorldObject* target)
                              {
                                  if (target->GetTypeId() != TYPEID_UNIT)
                                      return true;
                                  if (!target->ToUnit()->isInFrontInMap(caster, 8.0f, M_PI / 12.0f))
                                      return true;
                                  if (caster->GetExactDist2d(target->GetPositionX(), target->GetPositionY()) > 8.0f)
                                      return true;
                                  switch (spellId)
                                  {
                                      case SPELL_MELEE_FOOTMAN:
                                      case SPELL_MELEE_WATER_ELEM:
                                      case SPELL_MELEE_CHARGER:
                                      case SPELL_MELEE_CLERIC:
                                      case SPELL_MELEE_CONJURER:
                                      case SPELL_MELEE_KING_LLANE:
                                          for (uint8 i = 0; i < 6; i++)
                                              if (target->GetEntry() == hordePieceEntrys[i])
                                                  return false;
                                          break;
                                      case SPELL_MELEE_GRUNT:
                                      case SPELL_MELEE_DAEMON:
                                      case SPELL_MELEE_NECROLYTE:
                                      case SPELL_MELEE_WOLF:
                                      case SPELL_MELEE_WARLOCK:
                                      case SPELL_MELEE_WARCHIEF_BLACKHAND:
                                          for (uint8 i = 0; i < 6; i++)
                                              if (target->GetEntry() == alliancePieceEntrys[i])
                                                  return false;
                                          break;
                                  }
                                  return true;
                              });
        }

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (Unit *caster = GetCaster())
                if (Unit *target = GetHitUnit())
                    caster->CastSpell(target, SPELL_MELEE_DAMAGE, true);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_piece_melee_attack_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENTRY);
            OnEffectHitTarget += SpellEffectFn(spell_piece_melee_attack_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_piece_melee_attack_SpellScript();
    }
};

// SPELL_MELEE_DAMAGE
class spell_medivh_cheat : public SpellScriptLoader
{
public:
    spell_medivh_cheat() : SpellScriptLoader("spell_medivh_cheat") { }

    class spell_medivh_cheat_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_medivh_cheat_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            uint32 spellId = GetSpellInfo()->Id;
            targets.remove_if([spellId](WorldObject* target)
                              {
                                  if (target->GetTypeId() != TYPEID_UNIT)
                                      return true;
                                  switch (spellId)
                                  {
                                      case SPELL_HAND_OF_MEDIVH_HORDE:
                                      case SPELL_FURY_OF_MEDIVH_HORDE:
                                          for (uint8 i = 0; i < 6; i++)
                                              if (target->GetEntry() == hordePieceEntrys[i])
                                                  return false;
                                          break;
                                      case SPELL_HAND_OF_MEDIVH_ALLIANCE:
                                      case SPELL_FURY_OF_MEDIVH_ALLIANCE:
                                          for (uint8 i = 0; i < 6; i++)
                                              if (target->GetEntry() == alliancePieceEntrys[i])
                                                  return false;
                                          break;
                                      default:
                                          break;
                                  }
                                  return true;
                              });
            if (targets.size() > 3)
                Trinity::Containers::RandomResizeList(targets, 3);
        }

        void HandleScript(SpellEffIndex effIndex)
        {
            if (Unit *caster = GetCaster())
                if (Unit *target = GetHitUnit())
                    switch (GetSpellInfo()->Id)
                    {
                        case SPELL_HAND_OF_MEDIVH_HORDE:
                        case SPELL_HAND_OF_MEDIVH_ALLIANCE:
                            target->CastSpell(target, SPELL_HAND_OF_MEDIVH_TRIGGER, true);
                            break;
                        case SPELL_FURY_OF_MEDIVH_ALLIANCE:
                        case SPELL_FURY_OF_MEDIVH_HORDE:
                        {
                            uint32 spellId = GetSpellInfo()->Effects[effIndex].CalcValue();
                            caster->CastSpell(target, spellId, true);
                            break;
                        }
                    }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_medivh_cheat_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnEffectHitTarget += SpellEffectFn(spell_medivh_cheat_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_medivh_cheat_SpellScript();
    }
};

// 39384
class spell_medivh_cheat_flame_damage : public SpellScriptLoader
{
public:
    spell_medivh_cheat_flame_damage() : SpellScriptLoader("spell_medivh_cheat_flame_damage") { }

    class spell_medivh_cheat_flame_damage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_medivh_cheat_flame_damage_SpellScript);

        void FilterTargetsAlly(std::list<WorldObject*>& targets)
        {
            targets.remove_if([](WorldObject* target)
                              {
                                  if (target->GetTypeId() != TYPEID_UNIT)
                                      return true;
                                  for (uint8 i = 0; i < 6; i++)
                                      if (target->GetEntry() == alliancePieceEntrys[i])
                                          return false;
                                  return true;
                              });
        }

        void FilterTargetsHorde(std::list<WorldObject*>& targets)
        {
            targets.remove_if([](WorldObject* target)
                              {
                                  if (target->GetTypeId() != TYPEID_UNIT)
                                      return true;
                                  for (uint8 i = 0; i < 6; i++)
                                      if (target->GetEntry() == hordePieceEntrys[i])
                                          return false;
                                  return true;
                              });
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_medivh_cheat_flame_damage_SpellScript::FilterTargetsAlly, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_medivh_cheat_flame_damage_SpellScript::FilterTargetsHorde, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_medivh_cheat_flame_damage_SpellScript();
    }
};

class spell_medivh_cheat_heal : public SpellScriptLoader
{
public:
    spell_medivh_cheat_heal() : SpellScriptLoader("spell_medivh_cheat_heal") { }

    class spell_medivh_cheat_heal_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_medivh_cheat_heal_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            uint32 spellId = GetSpellInfo()->Id;
            targets.remove_if([spellId](WorldObject* target)
                              {
                                  if (target->GetTypeId() != TYPEID_UNIT)
                                      return true;
                                  switch (spellId)
                                  {
                                      case SPELL_FULL_HEAL_HORDE:
                                          for (uint8 i = 0; i < 6; i++)
                                              if (target->GetEntry() == hordePieceEntrys[i])
                                                  return false;
                                          break;
                                      case SPELL_FULL_HEAL_ALLIANCE:
                                          for (uint8 i = 0; i < 6; i++)
                                              if (target->GetEntry() == alliancePieceEntrys[i])
                                                  return false;
                                          break;
                                      default:
                                          break;
                                  }
                                  return true;
                              });
            if (targets.size() > 3)
                Trinity::Containers::RandomResizeList(targets, 3);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_medivh_cheat_heal_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_medivh_cheat_heal_SpellScript();
    }
};

void AddSC_chess_event()
{
    new npc_echo_of_medivh();
    new npc_king_llane();
    new npc_warchief_blackhand();
    new npc_human_conjurer();
    new npc_orc_warlock();
    new npc_human_footman();
    new npc_orc_grunt();
    new npc_water_elemental();
    new npc_summoned_daemon();
    new npc_human_charger();
    new npc_orc_wolf();
    new npc_human_cleric();
    new npc_orc_necrolyte();
    new spell_move_piece();
    new spell_control_piece();
    new npc_chess_square();
    new spell_change_piece_facing();
    new spell_piece_damage_filters_alliance_01();
    new spell_piece_damage_filters_alliance_02();
    new spell_piece_damage_filters_alliance_03();
    new spell_piece_damage_filters_horde_01();
    new spell_piece_damage_filters_horde_02();
    new spell_piece_damage_filters_horde_03();
    new spell_piece_auto_melee_attack();
    new spell_piece_melee_attack();
    new spell_medivh_cheat();
    new spell_medivh_cheat_flame_damage();
    new spell_medivh_cheat_heal();
}
