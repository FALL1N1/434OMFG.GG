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

#include "ScriptMgr.h"
#include "CombatAI.h"
#include "CreatureTextMgr.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "Log.h"
#include "MotionMaster.h"
#include "SharedDefines.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedEscortAI.h"
#include "ScriptedGossip.h"
#include "SpellInfo.h"
#include "Vehicle.h"

/*######
##Quest 12848
######*/

#define GCD_CAST    1

enum UnworthyInitiate
{
    SPELL_SOUL_PRISON_CHAIN_SELF = 54612,
    SPELL_SOUL_PRISON_CHAIN = 54613,
    SPELL_DK_INITIATE_VISUAL = 51519,

    SPELL_ICY_TOUCH = 52372,
    SPELL_PLAGUE_STRIKE = 52373,
    SPELL_BLOOD_STRIKE = 52374,
    SPELL_DEATH_COIL = 52375,

    SAY_EVENT_START = 0,
    SAY_EVENT_ATTACK = 1,

    EVENT_ICY_TOUCH = 1,
    EVENT_PLAGUE_STRIKE = 2,
    EVENT_BLOOD_STRIKE = 3,
    EVENT_DEATH_COIL = 4
};

enum UnworthyInitiatePhase
{
    PHASE_CHAINED,
    PHASE_TO_EQUIP,
    PHASE_EQUIPING,
    PHASE_TO_ATTACK,
    PHASE_ATTACKING,
};

uint32 acherus_soul_prison[12] =
{
    191577,
    191580,
    191581,
    191582,
    191583,
    191584,
    191585,
    191586,
    191587,
    191588,
    191589,
    191590
};

uint32 acherus_unworthy_initiate[5] =
{
    29519,
    29520,
    29565,
    29566,
    29567
};

class npc_unworthy_initiate : public CreatureScript
{
public:
    npc_unworthy_initiate() : CreatureScript("npc_unworthy_initiate") { }

    struct npc_unworthy_initiateAI : public ScriptedAI
    {
        npc_unworthy_initiateAI(Creature* creature) : ScriptedAI(creature)
        {
            Initialize();
            me->SetReactState(REACT_PASSIVE);
            if (!me->GetEquipmentId())
                if (const CreatureTemplate* info = sObjectMgr->GetCreatureTemplate(28406))
                    if (info->equipmentId)
                        const_cast<CreatureTemplate*>(me->GetCreatureTemplate())->equipmentId = info->equipmentId;

            wait_timer = 0;
            anchorX = 0.f;
            anchorY = 0.f;
        }

        void Initialize()
        {
            anchorGUID = 0;
            phase = PHASE_CHAINED;
        }

        uint64 playerGUID;
        UnworthyInitiatePhase phase;
        uint32 wait_timer;
        float anchorX, anchorY;
        uint64 anchorGUID;

        EventMap events;

        void Reset() override
        {
            Initialize();
            events.Reset();
            me->setFaction(7);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            me->SetStandState(UNIT_STAND_STATE_KNEEL);
            me->LoadEquipment(0, true);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.ScheduleEvent(EVENT_ICY_TOUCH, 1000, GCD_CAST);
            events.ScheduleEvent(EVENT_PLAGUE_STRIKE, 3000, GCD_CAST);
            events.ScheduleEvent(EVENT_BLOOD_STRIKE, 2000, GCD_CAST);
            events.ScheduleEvent(EVENT_DEATH_COIL, 5000, GCD_CAST);
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != POINT_MOTION_TYPE)
                return;

            if (id == 1)
            {
                wait_timer = 5000;
                me->LoadEquipment(1);
                me->CastSpell(me, SPELL_DK_INITIATE_VISUAL, true);

                if (Player* starter = ObjectAccessor::GetPlayer(*me, playerGUID))
                    Talk(SAY_EVENT_ATTACK, starter->GetGUID());

                phase = PHASE_TO_ATTACK;
            }
        }

        void EventStart(Creature* anchor, Player* target)
        {
            wait_timer = 5000;
            phase = PHASE_TO_EQUIP;

            me->SetStandState(UNIT_STAND_STATE_STAND);
            me->RemoveAurasDueToSpell(SPELL_SOUL_PRISON_CHAIN_SELF);
            me->RemoveAurasDueToSpell(SPELL_SOUL_PRISON_CHAIN);

            float z;
            anchor->GetContactPoint(me, anchorX, anchorY, z, 1.0f);

            playerGUID = target->GetGUID();
            Talk(SAY_EVENT_START, playerGUID);
        }

        void UpdateAI(uint32 diff) override
        {
            switch (phase)
            {
            case PHASE_CHAINED:
                if (!anchorGUID)
                {
                    if (Creature* anchor = me->FindNearestCreature(29521, 30))
                    {
                        anchor->AI()->SetGUID(me->GetGUID());
                        anchor->CastSpell(me, SPELL_SOUL_PRISON_CHAIN, true);
                        anchorGUID = anchor->GetGUID();
                    }
                    else
                        TC_LOG_ERROR("scripts", "npc_unworthy_initiateAI: unable to find anchor!");

                    float dist = 99.0f;
                    GameObject* prison = nullptr;

                    for (uint8 i = 0; i < 12; ++i)
                    {
                        if (GameObject* temp_prison = me->FindNearestGameObject(acherus_soul_prison[i], 30))
                        {
                            if (me->IsWithinDist(temp_prison, dist, false))
                            {
                                dist = me->GetDistance2d(temp_prison);
                                prison = temp_prison;
                            }
                        }
                    }

                    if (prison)
                        prison->ResetDoorOrButton();
                    else
                        TC_LOG_ERROR("scripts", "npc_unworthy_initiateAI: unable to find prison!");
                }
                break;
            case PHASE_TO_EQUIP:
                if (wait_timer)
                {
                    if (wait_timer > diff)
                        wait_timer -= diff;
                    else
                    {
                        me->GetMotionMaster()->MovePoint(1, anchorX, anchorY, me->GetPositionZ());
                        //TC_LOG_DEBUG("scripts", "npc_unworthy_initiateAI: move to %f %f %f", anchorX, anchorY, me->GetPositionZ());
                        phase = PHASE_EQUIPING;
                        wait_timer = 0;
                    }
                }
                break;
            case PHASE_TO_ATTACK:
                if (wait_timer)
                {
                    if (wait_timer > diff)
                        wait_timer -= diff;
                    else
                    {
                        me->setFaction(14);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                        phase = PHASE_ATTACKING;

                        if (Player* target = ObjectAccessor::GetPlayer(*me, playerGUID))
                            AttackStart(target);
                        wait_timer = 0;
                    }
                }
                break;
            case PHASE_ATTACKING:
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_ICY_TOUCH:
                        DoCastVictim(SPELL_ICY_TOUCH);
                        events.DelayEvents(1000, GCD_CAST);
                        events.ScheduleEvent(EVENT_ICY_TOUCH, 5000, GCD_CAST);
                        break;
                    case EVENT_PLAGUE_STRIKE:
                        DoCastVictim(SPELL_PLAGUE_STRIKE);
                        events.DelayEvents(1000, GCD_CAST);
                        events.ScheduleEvent(EVENT_PLAGUE_STRIKE, 5000, GCD_CAST);
                        break;
                    case EVENT_BLOOD_STRIKE:
                        DoCastVictim(SPELL_BLOOD_STRIKE);
                        events.DelayEvents(1000, GCD_CAST);
                        events.ScheduleEvent(EVENT_BLOOD_STRIKE, 5000, GCD_CAST);
                        break;
                    case EVENT_DEATH_COIL:
                        DoCastVictim(SPELL_DEATH_COIL);
                        events.DelayEvents(1000, GCD_CAST);
                        events.ScheduleEvent(EVENT_DEATH_COIL, 5000, GCD_CAST);
                        break;
                    }
                }

                DoMeleeAttackIfReady();
                break;
            default:
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_unworthy_initiateAI(creature);
    }
};

class npc_unworthy_initiate_anchor : public CreatureScript
{
public:
    npc_unworthy_initiate_anchor() : CreatureScript("npc_unworthy_initiate_anchor") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_unworthy_initiate_anchorAI(creature);
    }

    struct npc_unworthy_initiate_anchorAI : public PassiveAI
    {
        npc_unworthy_initiate_anchorAI(Creature* creature) : PassiveAI(creature), prisonerGUID(0) { }

        uint64 prisonerGUID;

        void SetGUID(uint64 guid, int32 /*id*/)
        {
            if (!prisonerGUID)
                prisonerGUID = guid;
        }

        uint64 GetGUID(int32 /*id*/) const
        {
            return prisonerGUID;
        }
    };
};

class go_acherus_soul_prison : public GameObjectScript
{
public:
    go_acherus_soul_prison() : GameObjectScript("go_acherus_soul_prison") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (Creature* anchor = go->FindNearestCreature(29521, 15))
            if (uint64 prisonerGUID = anchor->AI()->GetGUID())
                if (Creature* prisoner = ObjectAccessor::GetCreature(*player, prisonerGUID))
                    ENSURE_AI(npc_unworthy_initiate::npc_unworthy_initiateAI, prisoner->AI())->EventStart(anchor, player);
        return false;
    }
};


/*######
## npc_death_knight_initiate
######*/

#define GOSSIP_ACCEPT_DUEL      "I challenge you, death knight!"
#define ATTACK_CHAT_1			"No potions!"
#define ATTACK_CHAT_2			"You're going down!"
#define ATTACK_CHAT_3			"Here come the tears..."
#define ATTACK_CHAT_4			"I'm going to tear your heart out, cupcake! "
#define ATTACK_CHAT_5			"You don't stand a chance!"

enum Spells_DKI
{
    SPELL_DUEL = 52996,
    //SPELL_DUEL_TRIGGERED        = 52990,
    SPELL_DUEL_VICTORY = 52994,
    SPELL_DUEL_FLAG = 52991,
};

enum Says_VBM
{
    SAY_DUEL = 0,
};

enum Misc_VBN
{
    QUEST_DEATH_CHALLENGE = 12733,
    FACTION_HOSTILE = 2068
};

class npc_death_knight_initiate : public CreatureScript
{
public:
    npc_death_knight_initiate() : CreatureScript("npc_death_knight_initiate") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF)
        {
            player->CLOSE_GOSSIP_MENU();

            if (player->isInCombat() || creature->isInCombat())
                return true;

            if (npc_death_knight_initiateAI* pInitiateAI = CAST_AI(npc_death_knight_initiate::npc_death_knight_initiateAI, creature->AI()))
            {
                if (pInitiateAI->m_bIsDuelInProgress)
                    return true;
            }

            creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_15);

            player->CastSpell(creature, SPELL_DUEL, false);
            player->CastSpell(player, SPELL_DUEL_FLAG, true);

            uint32 random = urand(0, 4);
            switch (random)
            {
            case 0:
                creature->MonsterSay(ATTACK_CHAT_1, LANG_UNIVERSAL, 0);
                break;
            case 1:
                creature->MonsterSay(ATTACK_CHAT_2, LANG_UNIVERSAL, 0);
                break;
            case 2:
                creature->MonsterSay(ATTACK_CHAT_3, LANG_UNIVERSAL, 0);
                break;
            case 3:
                creature->MonsterSay(ATTACK_CHAT_4, LANG_UNIVERSAL, 0);
                break;
            case 4:
                creature->MonsterSay(ATTACK_CHAT_5, LANG_UNIVERSAL, 0);
                break;
            }
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (player->GetQuestStatus(QUEST_DEATH_CHALLENGE) == QUEST_STATUS_INCOMPLETE && creature->IsFullHealth())
        {
            if (player->HealthBelowPct(10))
                return true;

            if (player->isInCombat() || creature->isInCombat())
                return true;

            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ACCEPT_DUEL, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_death_knight_initiateAI(creature);
    }

    struct npc_death_knight_initiateAI : public CombatAI
    {
        npc_death_knight_initiateAI(Creature* creature) : CombatAI(creature)
        {
            m_bIsDuelInProgress = false;
        }

        bool lose;
        uint64 m_uiDuelerGUID;
        uint32 m_uiDuelTimer;
        bool m_bIsDuelInProgress;

        void Reset()
        {
            lose = false;
            me->RestoreFaction();
            CombatAI::Reset();

            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_15);

            m_uiDuelerGUID = 0;
            m_uiDuelTimer = 5000;
            m_bIsDuelInProgress = false;
        }

        void SpellHit(Unit* pCaster, const SpellInfo* pSpell)
        {
            if (!m_bIsDuelInProgress && pSpell->Id == SPELL_DUEL)
            {
                m_uiDuelerGUID = pCaster->GetGUID();
                m_bIsDuelInProgress = true;
            }
        }

        void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
        {
            if (m_bIsDuelInProgress && pDoneBy->IsControlledByPlayer())
            {
                if (pDoneBy->GetGUID() != m_uiDuelerGUID && pDoneBy->GetOwnerGUID() != m_uiDuelerGUID) // other players cannot help
                    uiDamage = 0;
                else if (uiDamage >= me->GetHealth())
                {
                    uiDamage = 0;

                    if (!lose)
                    {
                        pDoneBy->RemoveGameObject(SPELL_DUEL_FLAG, true);
                        pDoneBy->AttackStop();
                        me->CastSpell(pDoneBy, SPELL_DUEL_VICTORY, true);
                        lose = true;
                        me->CastSpell(me, 7267, true);
                        me->RestoreFaction();
                    }
                }
            }
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim())
            {
                if (m_bIsDuelInProgress)
                {
                    if (m_uiDuelTimer <= uiDiff)
                    {
                        me->setFaction(FACTION_HOSTILE);

                        if (Unit* unit = Unit::GetUnit(*me, m_uiDuelerGUID))
                            AttackStart(unit);
                    }
                    else
                        m_uiDuelTimer -= uiDiff;
                }
                return;
            }

            if (m_bIsDuelInProgress)
            {
                if (lose)
                {
                    if (!me->HasAura(7267))
                        EnterEvadeMode();
                    return;
                }
                else if (me->getVictim()->GetTypeId() == TYPEID_PLAYER && me->getVictim()->HealthBelowPct(10))
                {
                    me->getVictim()->CastSpell(me->getVictim(), 7267, true); // beg
                    me->getVictim()->RemoveGameObject(SPELL_DUEL_FLAG, true);
                    EnterEvadeMode();
                    return;
                }
            }

            // TODO: spells

            CombatAI::UpdateAI(uiDiff);
        }
    };

};

/*######
## npc_dark_rider_of_acherus
######*/

enum Spells_DR
{
    SPELL_DESPAWN_HORSE = 51918
};

enum Says_DR
{
    SAY_DARK_RIDER = 0
};

class npc_dark_rider_of_acherus : public CreatureScript
{
public:
    npc_dark_rider_of_acherus() : CreatureScript("npc_dark_rider_of_acherus") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_dark_rider_of_acherusAI(creature);
    }

    struct npc_dark_rider_of_acherusAI : public ScriptedAI
    {
        npc_dark_rider_of_acherusAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 PhaseTimer;
        uint32 Phase;
        bool Intro;
        uint64 TargetGUID;

        void Reset()
        {
            PhaseTimer = 4000;
            Phase = 0;
            Intro = false;
            TargetGUID = 0;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!Intro || !TargetGUID)
                return;

            if (PhaseTimer <= diff)
            {
                switch (Phase)
                {
                case 0:
                    me->MonsterSay(SAY_DARK_RIDER, LANG_UNIVERSAL, 0);
                    PhaseTimer = 5000;
                    Phase = 1;
                    break;
                case 1:
                    if (Unit* target = Unit::GetUnit(*me, TargetGUID))
                        DoCast(target, SPELL_DESPAWN_HORSE, true);
                    PhaseTimer = 3000;
                    Phase = 2;
                    break;
                case 2:
                    me->SetVisible(false);
                    PhaseTimer = 2000;
                    Phase = 3;
                    break;
                case 3:
                    me->DespawnOrUnsummon();
                    break;
                default:
                    break;
                }
            }
            else PhaseTimer -= diff;

        }

        void InitDespawnHorse(Unit* who)
        {
            if (!who)
                return;

            TargetGUID = who->GetGUID();
            me->SetWalk(true);
            me->SetSpeed(MOVE_RUN, 0.4f);
            me->GetMotionMaster()->MoveChase(who);
            me->SetTarget(TargetGUID);
            Intro = true;
        }

    };

};

/*######
## npc_ros_dark_rider
######*/

class npc_ros_dark_rider : public CreatureScript
{
public:
    npc_ros_dark_rider() : CreatureScript("npc_ros_dark_rider") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ros_dark_riderAI(creature);
    }

    struct npc_ros_dark_riderAI : public ScriptedAI
    {
        npc_ros_dark_riderAI(Creature* creature) : ScriptedAI(creature) {}

        void EnterCombat(Unit* /*who*/)
        {
            me->ExitVehicle();
        }

        void Reset()
        {
            Creature* deathcharger = me->FindNearestCreature(28782, 30);
            if (!deathcharger)
                return;

            deathcharger->RestoreFaction();
            deathcharger->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            deathcharger->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            if (!me->GetVehicle() && deathcharger->IsVehicle() && deathcharger->GetVehicleKit()->HasEmptySeat(0))
                me->EnterVehicle(deathcharger);
        }

        void JustDied(Unit* killer)
        {
            Creature* deathcharger = me->FindNearestCreature(28782, 30);
            if (!deathcharger)
                return;

            if (killer->GetTypeId() == TYPEID_PLAYER && deathcharger->GetTypeId() == TYPEID_UNIT && deathcharger->IsVehicle())
            {
                deathcharger->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                deathcharger->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                deathcharger->setFaction(2096);
            }
        }
    };

};

enum TheGiftThatKeepsOnGiving
{
    EVENT_GHOUL_RESTORE_STATE = 1,
    EVENT_GHOUL_CHECK_COMBAT = 2,
    EVENT_GHOUL_EMOTE = 3,
    EVENT_GHOUL_MOVE_TO_PIT = 4,

    NPC_GHOUL = 28845,
    MAX_GHOULS = 5,

    SPELL_GHOUL_EMERGE = 50142,
    SPELL_GHOUL_SUBMERGE = 26234,
    SPELL_GHOULZAP = 52521,

    SAY_GOTHIK_PIT = 0,
    SAY_SCARLET_GHOUL = 0
};


class npc_dkc1_gothik : public CreatureScript
{
public:
    npc_dkc1_gothik() : CreatureScript("npc_dkc1_gothik") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_dkc1_gothikAI(creature);
    }

    struct npc_dkc1_gothikAI : public ScriptedAI
    {
        npc_dkc1_gothikAI(Creature* creature) : ScriptedAI(creature) { spoken = 0; }

        int32 spoken;

        void DoAction(int32 action)
        {
            if (action == SAY_GOTHIK_PIT && spoken <= 0)
            {
                spoken = 5000;
                Talk(SAY_GOTHIK_PIT);
            }
        }

        void MoveInLineOfSight(Unit* who)
        {
            ScriptedAI::MoveInLineOfSight(who);

            if (!who->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC) && who->GetEntry() == NPC_GHOUL && me->IsWithinDistInMap(who, 10.0f))
                if (Unit* owner = who->GetOwner())
                    if (Player* player = owner->ToPlayer())
                    {
                        Creature* creature = who->ToCreature();
                        if (player->GetQuestStatus(12698) == QUEST_STATUS_INCOMPLETE)
                        {
                            me->CastSpell(who, SPELL_GHOULZAP, true);
                            creature->CastSpell(owner, 52517, true);
                        }


                        creature->AI()->SetGUID(me->GetGUID());
                        creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                    }
        }

        void UpdateAI(uint32 diff)
        {
            if (spoken > 0)
                spoken -= diff;

            ScriptedAI::UpdateAI(diff);
        }
    };
};


class npc_scarlet_ghoul : public CreatureScript
{
public:
    npc_scarlet_ghoul() : CreatureScript("npc_scarlet_ghoul") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_scarlet_ghoulAI(creature);
    }

    struct npc_scarlet_ghoulAI : public ScriptedAI
    {
        npc_scarlet_ghoulAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        EventMap events;
        uint64 gothikGUID;

        void InitializeAI()
        {
            gothikGUID = 0;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            ScriptedAI::InitializeAI();
            me->SetReactState(REACT_PASSIVE);

            events.ScheduleEvent(EVENT_GHOUL_EMOTE, 1);
            events.ScheduleEvent(EVENT_GHOUL_RESTORE_STATE, 3500);
        }

        void OwnerAttackedBy(Unit* attacker)
        {
            if (!me->isInCombat() && me->GetReactState() == REACT_DEFENSIVE)
                AttackStart(attacker);
        }

        void SetGUID(uint64 guid, int32)
        {
            gothikGUID = guid;
            events.ScheduleEvent(EVENT_GHOUL_MOVE_TO_PIT, 3000);
            me->GetMotionMaster()->Clear(false);
        }

        void MovementInform(uint32 type, uint32 point)
        {
            if (type == POINT_MOTION_TYPE && point == 1)
            {
                me->DespawnOrUnsummon(1500);
                me->CastSpell(me, SPELL_GHOUL_SUBMERGE, true);
            }
        }

        void JustDied(Unit* killer)
        {
            me->DespawnOrUnsummon();
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);
            switch (events.GetEvent())
            {
            case EVENT_GHOUL_MOVE_TO_PIT:
                me->GetMotionMaster()->MovePoint(1, 2364.77f, -5776.14f, 151.36f);
                if (Creature* gothik = ObjectAccessor::GetCreature(*me, gothikGUID))
                    gothik->AI()->DoAction(SAY_GOTHIK_PIT);
                events.PopEvent();
                break;
            case EVENT_GHOUL_EMOTE:
                me->CastSpell(me, SPELL_GHOUL_EMERGE, true);
                Talk(SAY_SCARLET_GHOUL);
                events.PopEvent();
                break;
            case EVENT_GHOUL_RESTORE_STATE:
                me->SetReactState(REACT_DEFENSIVE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                if (Player* owner = me->GetCharmerOrOwnerPlayerOrPlayerItself())
                    me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, frand(0.0f, 2 * M_PI));
                events.PopEvent();
                events.ScheduleEvent(EVENT_GHOUL_CHECK_COMBAT, 1000);
                return;
            case EVENT_GHOUL_CHECK_COMBAT:
                if (!me->isInCombat())
                    if (Player* owner = me->GetCharmerOrOwnerPlayerOrPlayerItself())
                        if (owner->getVictim())
                            AttackStart(owner->getVictim());

                events.RepeatEvent(1000);
                return;
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };
};

/*####
## npc_scarlet_cannon
####*/

class npc_scarlet_cannon : public CreatureScript
{
public:
    npc_scarlet_cannon() : CreatureScript("npc_scarlet_cannon") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_scarlet_cannonAI(creature);
    }

    struct npc_scarlet_cannonAI : public PassiveAI
    {
        npc_scarlet_cannonAI(Creature* creature) : PassiveAI(creature) {}

        void OnCharmed(bool apply) override
        {
            me->SetControlled(false, UNIT_STATE_ROOT);
            me->SetControlled(true, UNIT_STATE_ROOT);
        }
    };
};

/*####
## npc_scarlet_miner_cart
####*/

enum Spells_SM
{
    SPELL_CART_CHECK = 54173,
    SPELL_SUMMON_CART = 52463,
    SPELL_SUMMON_MINER = 52464,
    SPELL_CART_DRAG = 52465,

    NPC_MINER = 28841
};

class npc_scarlet_miner_cart : public CreatureScript
{
public:
    npc_scarlet_miner_cart() : CreatureScript("npc_scarlet_miner_cart") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_scarlet_miner_cartAI(creature);
    }

    struct npc_scarlet_miner_cartAI : public PassiveAI
    {
        npc_scarlet_miner_cartAI(Creature* creature) : PassiveAI(creature), minerGUID(0)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            me->SetDisplayId(me->GetCreatureTemplate()->Modelid1); // Modelid2 is a horse.
            playerGUID = 0;
            minerGUID = 0;
        }

        uint64 minerGUID;
        uint64 playerGUID;

        void SetGUID(uint64 guid, int32 /*id*/)
        {
            minerGUID = guid;
        }

        void JustSummoned(Creature* summon) override
        {
            if (summon->GetEntry() == NPC_MINER)
            {
                minerGUID = summon->GetGUID();
                summon->AI()->SetGUID(playerGUID);
            }
        }

        void SummonedCreatureDespawn(Creature* summon) override
        {
            if (summon->GetEntry() == NPC_MINER)
                minerGUID = 0;
        }

        void DoAction(const int32 /*param*/)
        {
            if (Creature* miner = Unit::GetCreature(*me, minerGUID))
            {
                me->SetWalk(false);

                //Not 100% correct, but movement is smooth. Sometimes miner walks faster
                //than normal, this speed is fast enough to keep up at those times.
                me->SetSpeed(MOVE_RUN, 1.25f);

                me->GetMotionMaster()->MoveFollow(miner, 1.0f, 0);
                me->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                me->setFaction(35);
            }
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (apply)
            {
                playerGUID = who->GetGUID();
                if (Player* player = me->GetPlayer(*me, playerGUID))
                {
                    me->CastSpell(player, SPELL_SUMMON_MINER, true);
                    player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                    player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
                }


            }
            else
            {
                if (who)
                {
                    who->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    who->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                    who->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
                }
                playerGUID = 0;
                if (Creature* miner = ObjectAccessor::GetCreature(*me, minerGUID))
                    miner->DespawnOrUnsummon();

            }
        }
    };

};

/*####
## npc_scarlet_miner
####*/

enum Says_SM
{
    SAY_SCARLET_MINER_0 = 0,
    SAY_SCARLET_MINER_1 = 1
};

class npc_scarlet_miner : public CreatureScript
{
public:
    npc_scarlet_miner() : CreatureScript("npc_scarlet_miner") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_scarlet_minerAI(creature);
    }

    struct npc_scarlet_minerAI : public npc_escortAI
    {
        npc_scarlet_minerAI(Creature* creature) : npc_escortAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            carGUID = 0;
            IntroTimer = 0;
            IntroPhase = 0;
        }

        uint32 IntroTimer;
        uint32 IntroPhase;
        uint64 carGUID;

        void Reset()
        {
            carGUID = 0;
            IntroTimer = 0;
            IntroPhase = 0;
        }

        void IsSummonedBy(Unit* summoner) override
        {
            carGUID = summoner->GetGUID();
        }

        void InitWaypoint()
        {
            AddWaypoint(1, 2389.03f, -5902.74f, 109.014f, 5000);
            AddWaypoint(2, 2341.812012f, -5900.484863f, 102.619743f);
            AddWaypoint(3, 2306.561279f, -5901.738281f, 91.792419f);
            AddWaypoint(4, 2300.098389f, -5912.618652f, 86.014885f);
            AddWaypoint(5, 2294.142090f, -5927.274414f, 75.316849f);
            AddWaypoint(6, 2286.984375f, -5944.955566f, 63.714966f);
            AddWaypoint(7, 2280.001709f, -5961.186035f, 54.228283f);
            AddWaypoint(8, 2259.389648f, -5974.197754f, 42.359348f);
            AddWaypoint(9, 2242.882812f, -5984.642578f, 32.827850f);
            AddWaypoint(10, 2217.265625f, -6028.959473f, 7.675705f);
            AddWaypoint(11, 2202.595947f, -6061.325684f, 5.882018f);
            AddWaypoint(12, 2188.974609f, -6080.866699f, 3.370027f);

            if (urand(0, 1))
            {
                AddWaypoint(13, 2176.483887f, -6110.407227f, 1.855181f);
                AddWaypoint(14, 2172.516602f, -6146.752441f, 1.074235f);
                AddWaypoint(15, 2138.918457f, -6158.920898f, 1.342926f);
                AddWaypoint(16, 2129.866699f, -6174.107910f, 4.380779f);
                AddWaypoint(17, 2117.709473f, -6193.830078f, 13.3542f, 10000);
            }
            else
            {
                AddWaypoint(13, 2184.190186f, -6166.447266f, 0.968877f);
                AddWaypoint(14, 2234.265625f, -6163.741211f, 0.916021f);
                AddWaypoint(15, 2268.071777f, -6158.750977f, 1.822252f);
                AddWaypoint(16, 2270.028320f, -6176.505859f, 6.340538f);
                AddWaypoint(17, 2271.739014f, -6195.401855f, 13.3542f, 10000);
            }
        }

        void SetGUID(uint64 guid, int32 /*id = 0*/) override
        {
            InitWaypoint();
            Start(false, false, guid);
            SetDespawnAtFar(false);
        }

        void WaypointReached(uint32 waypointId) override
        {
            switch (waypointId)
            {
            case 1:
                if (Unit* car = ObjectAccessor::GetCreature(*me, carGUID))
                {
                    me->SetFacingToObject(car);
                    car->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                    car->setFaction(35);
                }
                Talk(SAY_SCARLET_MINER_0);
                SetRun(true);
                IntroTimer = 4000;
                IntroPhase = 1;
                break;
            case 17:
                if (Unit* car = ObjectAccessor::GetCreature(*me, carGUID))
                {
                    me->SetFacingToObject(car);
                    car->RemoveAura(SPELL_CART_DRAG);
                }
                Talk(SAY_SCARLET_MINER_1);
                break;
            default:
                break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (IntroPhase)
            {
                if (IntroTimer <= diff)
                {
                    if (IntroPhase == 1)
                    {
                        if (Creature* car = Unit::GetCreature(*me, carGUID))
                            DoCast(car, SPELL_CART_DRAG);
                        IntroTimer = 800;
                        IntroPhase = 2;
                    }
                    else
                    {
                        if (Creature* car = Unit::GetCreature(*me, carGUID))
                            car->AI()->DoAction(0);
                        IntroPhase = 0;
                    }
                }
                else IntroTimer -= diff;
            }
            npc_escortAI::UpdateAI(diff);
        }
    };

};

// npc 28912 quest 17217 boss 29001 mob 29007 go 191092

void AddSC_the_scarlet_enclave_c1()
{
    new npc_unworthy_initiate();
    new npc_unworthy_initiate_anchor();
    new go_acherus_soul_prison();
    new npc_death_knight_initiate();
    new npc_dark_rider_of_acherus();
    new npc_ros_dark_rider();
    new npc_dkc1_gothik();
    new npc_scarlet_ghoul();
    new npc_scarlet_cannon();
    new npc_scarlet_miner();
    new npc_scarlet_miner_cart();
}