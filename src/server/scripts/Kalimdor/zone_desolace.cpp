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
SDName: Desolace
SD%Complete: 100
SDComment: Quest support: 5561, 5581
SDCategory: Desolace
EndScriptData */

/* ContentData
npc_aged_dying_ancient_kodo
go_demon_portal
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"
#include "Player.h"
#include "SpellInfo.h"

enum DyingKodo
{
    // signed for 9999
    SAY_SMEED_HOME                  = 0,

    QUEST_KODO                      = 5561,

    NPC_SMEED                       = 11596,
    NPC_AGED_KODO                   = 4700,
    NPC_DYING_KODO                  = 4701,
    NPC_ANCIENT_KODO                = 4702,
    NPC_TAMED_KODO                  = 11627,

    SPELL_KODO_KOMBO_ITEM           = 18153,
    SPELL_KODO_KOMBO_PLAYER_BUFF    = 18172,                //spells here have unclear function, but using them at least for visual parts and checks
    SPELL_KODO_KOMBO_DESPAWN_BUFF   = 18377,
    SPELL_KODO_KOMBO_GOSSIP         = 18362

};

class npc_aged_dying_ancient_kodo : public CreatureScript
{
public:
    npc_aged_dying_ancient_kodo() : CreatureScript("npc_aged_dying_ancient_kodo") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (player->HasAura(SPELL_KODO_KOMBO_PLAYER_BUFF) && creature->HasAura(SPELL_KODO_KOMBO_DESPAWN_BUFF))
        {
            //the expected quest objective
            player->KilledMonsterCredit(NPC_TAMED_KODO, 0);

            player->RemoveAurasDueToSpell(SPELL_KODO_KOMBO_PLAYER_BUFF);
            creature->GetMotionMaster()->MoveIdle();
        }

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnDummyEffect(Unit* caster, uint32 spellId, SpellEffIndex effIndex, Creature* target)
    {
        //always check spellid and effectindex
        if (spellId == SPELL_KODO_KOMBO_ITEM && effIndex == 0)
        {
            //no effect if player/creature already have aura from spells
            if (caster->HasAura(SPELL_KODO_KOMBO_PLAYER_BUFF) || target->HasAura(SPELL_KODO_KOMBO_DESPAWN_BUFF))
                return true;

            if (target->GetEntry() == NPC_AGED_KODO ||
                target->GetEntry() == NPC_DYING_KODO ||
                target->GetEntry() == NPC_ANCIENT_KODO)
            {
                caster->CastSpell(caster, SPELL_KODO_KOMBO_PLAYER_BUFF, true);

                target->UpdateEntry(NPC_TAMED_KODO);
                target->CastSpell(target, SPELL_KODO_KOMBO_DESPAWN_BUFF, false);

                if (target->GetMotionMaster()->GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
                    target->GetMotionMaster()->MoveIdle();

                target->GetMotionMaster()->MoveFollow(caster, PET_FOLLOW_DIST,  target->GetFollowAngle());
            }

            //always return true when we are handling this spell and effect
            return true;
        }
        return false;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_aged_dying_ancient_kodoAI(creature);
    }

    struct npc_aged_dying_ancient_kodoAI : public ScriptedAI
    {
        npc_aged_dying_ancient_kodoAI(Creature* creature) : ScriptedAI(creature) { Reset(); }

        void MoveInLineOfSight(Unit* who)
        {
            if (who->GetEntry() == NPC_SMEED)
            {
                if (me->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP))
                    return;

                if (me->IsWithinDistInMap(who, 10.0f))
                {
                    if (Creature* talker = who->ToCreature())
                        talker->AI()->Talk(SAY_SMEED_HOME);

                    //spell have no implemented effect (dummy), so useful to notify spellHit
                    DoCast(me, SPELL_KODO_KOMBO_GOSSIP, true);
                }
            }
        }

        void SpellHit(Unit* /*pCaster*/, SpellInfo const* pSpell)
        {
            if (pSpell->Id == SPELL_KODO_KOMBO_GOSSIP)
            {
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                me->DespawnOrUnsummon(10000);
            }
        }
    };

};

/*######
## go_demon_portal
######*/

enum DemonPortal
{
    NPC_DEMON_GUARDIAN          = 11937,

    QUEST_PORTAL_OF_THE_LEGION  = 5581,
};

class go_demon_portal : public GameObjectScript
{
    public:
        go_demon_portal() : GameObjectScript("go_demon_portal") { }

        bool OnGossipHello(Player* player, GameObject* go)
        {
            if (player->GetQuestStatus(QUEST_PORTAL_OF_THE_LEGION) == QUEST_STATUS_INCOMPLETE && !go->FindNearestCreature(NPC_DEMON_GUARDIAN, 5.0f, true))
            {
                if (Creature* guardian = player->SummonCreature(NPC_DEMON_GUARDIAN, go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(), 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0))
                    guardian->AI()->AttackStart(player);
            }

            return true;
        }
};

void AddSC_desolace()
{
    new npc_aged_dying_ancient_kodo();
    new go_demon_portal();
}
