/*
 * Copyright (C) 2013-2014 OMFG.GG Network <http://www.omfg.gg/>
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

#include "VerificationMgr.h"
#include "AccountMgr.h"
#include "Player.h"
#include "SpellInfo.h"
#include "World.h"

bool VerificationMgr::IsValidPlayerSpell(Player* player, uint32 spellId)
{
    if (!sWorld->getBoolConfig(CONFIG_OMFG_VERIFICATIONMGR_ENABLE))
        return true;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
    {
        TC_LOG_ERROR("entities.player.skills", "VerificationMgr::PlayerSpell: Player %s (Guid: %u) failed check (unknown spell: %u)", player->GetName().c_str(), player->GetGUIDLow(), spellId);
        return false;
    }

    bool valid = true;

    //bool valid = IsSpellFitBySkillLine(spellId, player->getRaceMask(), player->getClassMask(), AccountMgr::IsPlayerAccount(player->GetSession()->GetSecurity()));

    //if (!valid)
    //    TC_LOG_ERROR("entities.player.skills", "VerificationMgr::PlayerSpell::ClassAndRace: Player %s (Guid: %u) failed check (spell: %u)", player->GetName().c_str(), player->GetGUIDLow(), spellId);

    if ((spellInfo->AttributesEx7 & SPELL_ATTR7_HORDE_ONLY) && player->GetTeam() != HORDE)
    {
        valid &= false;
        TC_LOG_ERROR("entities.player.skills", "VerificationMgr::PlayerSpell::SPELL_ATTR7_HORDE_ONLY: Player %s (Guid: %u) failed check (spell: %u)",
            player->GetName().c_str(), player->GetGUIDLow(), spellId);
    }

    if ((spellInfo->AttributesEx7 & SPELL_ATTR7_ALLIANCE_ONLY) && player->GetTeam() != ALLIANCE)
    {
        valid &= false;
        TC_LOG_ERROR("entities.player.skills", "VerificationMgr::PlayerSpell::SPELL_ATTR7_ALLIANCE_ONLY: Player %s (Guid: %u) failed check (spell: %u)",
            player->GetName().c_str(), player->GetGUIDLow(), spellId);
    }

    return valid;
}

bool VerificationMgr::CheckAllPlayerSpells()
{
    QueryResult result = CharacterDatabase.Query("SELECT c.guid, c.name, c.class, c.race, cs.spell FROM character_spell cs INNER JOIN characters c ON (cs.guid = c.guid)");
    if (!result)
        return false;

    do
    {
        Field* fields     = result->Fetch();
        uint32 guid       = fields[0].GetUInt32();
        std::string name  = fields[1].GetString();
        uint8 playerClass = fields[2].GetUInt8();
        uint8 race        = fields[3].GetUInt8();
        uint32 spellId    = fields[4].GetUInt32();

        bool valid = IsSpellFitBySkillLine(spellId, 1 << (race - 1), 1 << (playerClass - 1));

        if (!valid)
            TC_LOG_INFO("entities.player.skills", "VerificationMgr::CheckAllPlayerSpells: Player %s (Guid: %u) failed check (spell: %u)", name.c_str(), guid, spellId);
    }
    while (result->NextRow());

    return true;
}

bool VerificationMgr::IsSpellFitBySkillLine(uint32 spellId, uint32 raceMask, uint32 classMask, bool isPlayerAccount /*= true*/)
{
    if (!isPlayerAccount)
        return true;

    SkillLineAbilityMapBounds bounds = sSpellMgr->GetSkillLineAbilityMapBounds(spellId);

    // no skill
    if (bounds.first == bounds.second)
        return false;

    for (SkillLineAbilityMap::const_iterator spellItr = bounds.first; spellItr != bounds.second; ++spellItr)
    {
        // Blizzard Spells
        if (spellItr->second->skillId == SKILL_INTERNAL)
            continue;

        // skip wrong race skills
        if (spellItr->second->racemask && (spellItr->second->racemask & raceMask) == 0)
            continue;

        // skip wrong class skills
        if (spellItr->second->classmask && (spellItr->second->classmask & classMask) == 0)
            continue;

        return true;
    }

    return false;
}
