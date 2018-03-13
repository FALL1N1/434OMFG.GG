/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "Creature.h"
#include "TemporarySummon.h"
#include "Player.h"
#include "PassiveAI.h"
#include "ScriptedGossip.h"
#include "InstanceScript.h"

PassiveAI::PassiveAI(Creature* c) : CreatureAI(c) { me->SetReactState(REACT_PASSIVE); }
PossessedAI::PossessedAI(Creature* c) : CreatureAI(c) { me->SetReactState(REACT_PASSIVE); }
NullCreatureAI::NullCreatureAI(Creature* c) : CreatureAI(c) { me->SetReactState(REACT_PASSIVE); }
TeleporterAI::TeleporterAI(Creature* c) : CreatureAI(c) { me->SetReactState(REACT_PASSIVE); }

void PassiveAI::UpdateAI(const uint32)
{
    if (me->isInCombat() && me->getAttackers().empty())
        EnterEvadeMode();
}

void PossessedAI::AttackStart(Unit* target)
{
    me->Attack(target, true);
}

void PossessedAI::UpdateAI(const uint32 /*diff*/)
{
    if (me->getVictim())
    {
        if (!me->IsValidAttackTarget(me->getVictim()))
            me->AttackStop();
        else
            DoMeleeAttackIfReady();
    }
}

void PossessedAI::JustDied(Unit* /*u*/)
{
    // We died while possessed, disable our loot
    me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
}

void PossessedAI::KilledUnit(Unit* victim)
{
    // We killed a creature, disable victim's loot
    if (victim->GetTypeId() == TYPEID_UNIT)
        victim->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
}

void CritterAI::DamageTaken(Unit* /*done_by*/, uint32&)
{
    if (!me->HasUnitState(UNIT_STATE_FLEEING))
        me->SetControlled(true, UNIT_STATE_FLEEING);
}

void CritterAI::EnterEvadeMode()
{
    if (me->HasUnitState(UNIT_STATE_FLEEING))
        me->SetControlled(false, UNIT_STATE_FLEEING);
    CreatureAI::EnterEvadeMode();
}

void TriggerAI::IsSummonedBy(Unit* summoner)
{
    if (me->m_spells[0])
        me->CastSpell(me, me->m_spells[0], false, 0, 0, summoner->GetGUID());
}

void TeleporterAI::Reset()
{
    std::vector<TeleporterData> const* data = sObjectMgr->GetTeleporterGroup(me->GetEntry(), TELEPORTER_TYPE_CREATURE, 1);
    if (data)
    {
        SetDirectTeleportToLastDest((*data)[0].directTeleport);
        SetVisibleOnActiveTeleportations(!(*data)[0].invisibleBeforeActivation);
    }
    if (InstanceScript *instance = me->GetInstanceScript())
        instance->UpdateTeleportations();
    CreatureAI::Reset();
}

void TeleporterAI::UpdateAI(const uint32)
{
    if (me->isInCombat() && me->getAttackers().empty())
        EnterEvadeMode();
}


void TeleporterAI::SetData(uint32 uiType, uint32 uiData)
{
    if (!isVisibleOnActiveTeleportation && uiData)
        me->SetVisible(true);
    _teleportationStates[uiType] = (bool)uiData;
}

bool TeleporterAI::OnGossipHello(Player* player, Creature* creature)
{
    player->PlayerTalkClass->ClearMenus();

    if (isTeleportedAtLastDest)
    {
        int32 teleportId = -1;
        for (std::map<uint32, bool>::iterator itr = _teleportationStates.begin(); itr != _teleportationStates.end(); itr++)
            if (itr->second)
            {
                int tempId = itr->first;
                if (teleportId < tempId)
                    teleportId = tempId;
            }
        if (teleportId > 0)
        {
            std::vector<TeleporterData> const* data = sObjectMgr->GetTeleporterGroup(creature->GetEntry(), TELEPORTER_TYPE_CREATURE, teleportId);
            if (data)
            {
                if ((*data)[0].spellId)
                    me->CastSpell(player, (*data)[0].spellId, true);
                else
                {
                    Position pos;
                    pos.Relocate((*data)[0].pos);
                    player->TeleportTo(player->GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ, pos.m_orientation);
                }
                player->CLOSE_GOSSIP_MENU();
            }
        }
    }
    else
    {
        for (std::map<uint32, bool>::iterator itr = _teleportationStates.begin(); itr != _teleportationStates.end(); itr++)
            if (itr->second)
            {
                std::vector<TeleporterData> const* data = sObjectMgr->GetTeleporterGroup(creature->GetEntry(), TELEPORTER_TYPE_CREATURE, itr->first);
                if (data)
                {
                    std::string gossipMessage = (*data)[0].gossipMess;
                    BroadcastText const* gossipBroadcastText = sObjectMgr->GetBroadcastText((*data)[0].BroadcastTextId);
                    LocaleConstant locale = player->GetSession()->GetSessionDbLocaleIndex();
                    if (gossipBroadcastText)
                        gossipMessage = gossipBroadcastText->GetText(locale, me->getGender());
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, gossipMessage, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + itr->first);
                }
            }
        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
    }
    return true;
}

bool TeleporterAI::OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
{
    player->PlayerTalkClass->ClearMenus();

    std::vector<TeleporterData> const* data = sObjectMgr->GetTeleporterGroup(creature->GetEntry(), TELEPORTER_TYPE_CREATURE, action - GOSSIP_ACTION_INFO_DEF);
    if (data)
    {
        if ((*data)[0].spellId)
            me->CastSpell(player, (*data)[0].spellId, true);
        else
        {
            Position pos;
            pos.Relocate((*data)[0].pos);
            player->TeleportTo(player->GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ, pos.m_orientation);
        }
    }
    player->CLOSE_GOSSIP_MENU();
    return true;
}

void TeleporterAI::SetVisibleOnActiveTeleportations(bool val)
{
    isVisibleOnActiveTeleportation = val;
    me->SetVisible(val ? true : false);
}
