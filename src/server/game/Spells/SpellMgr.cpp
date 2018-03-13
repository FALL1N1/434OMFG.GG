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

#include "SpellMgr.h"
#include "SpellInfo.h"
#include "ObjectMgr.h"
#include "SpellAuras.h"
#include "SpellAuraDefines.h"
#include "SharedDefines.h"
#include "DBCStores.h"
#include "World.h"
#include "Chat.h"
#include "Spell.h"
#include "BattlegroundMgr.h"
#include "CreatureAI.h"
#include "MapManager.h"
#include "BattlegroundIC.h"
#include "BattlefieldWG.h"
#include "BattlefieldMgr.h"
#include "Player.h"

bool IsPrimaryProfessionSkill(uint32 skill)
{
    SkillLineEntry const* pSkill = sSkillLineStore.LookupEntry(skill);
    return pSkill && pSkill->categoryId == SKILL_CATEGORY_PROFESSION;
}

bool IsWeaponSkill(uint32 skill)
{
    SkillLineEntry const* pSkill = sSkillLineStore.LookupEntry(skill);
    return pSkill && pSkill->categoryId == SKILL_CATEGORY_WEAPON;
}

bool IsPartOfSkillLine(uint32 skillId, uint32 spellId)
{
    SkillLineAbilityMapBounds skillBounds = sSpellMgr->GetSkillLineAbilityMapBounds(spellId);
    for (SkillLineAbilityMap::const_iterator itr = skillBounds.first; itr != skillBounds.second; ++itr)
        if (itr->second->skillId == skillId)
            return true;

    return false;
}

std::list<DiminishingGroup> GetDiminishingReturnsGroupForSpell(SpellInfo const* spellproto, bool triggered)
{
    std::list<DiminishingGroup> temp;
    if (spellproto->IsPositive())
        temp.push_back(DIMINISHING_NONE);

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (spellproto->Effects[i].ApplyAuraName == SPELL_AURA_MOD_TAUNT)
            temp.push_back(DIMINISHING_TAUNT);
    }

    // Explicit Diminishing Groups
    switch (spellproto->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            // Pet charge effects (Infernal Awakening, Demon Charge)
            if (spellproto->SpellVisual[0] == 2816 && spellproto->SpellIconID == 15)
                temp.push_back(DIMINISHING_CONTROLLED_STUN);
            // Frost Tomb
            else if (spellproto->Id == 48400)
                temp.push_back(DIMINISHING_NONE);
            // Gnaw
            else if (spellproto->Id == 47481)
                temp.push_back(DIMINISHING_CONTROLLED_STUN);
            // ToC Icehowl Arctic Breath
            else if (spellproto->SpellVisual[0] == 14153)
                temp.push_back(DIMINISHING_NONE);
            // Pet - Intimidation
            else if (spellproto->Id == 24394)
                temp.push_back(DIMINISHING_CONTROLLED_STUN);
            // Reckless Leap
            else if (spellproto->Id == 99646 || spellproto->Id == 100104)
                temp.push_back(DIMINISHING_NONE);
            break;
        }
        // Event spells
        case SPELLFAMILY_UNK1:
            temp.push_back(DIMINISHING_NONE);
            break;
        case SPELLFAMILY_MAGE:
        {
            // Frostbite
            if (spellproto->SpellFamilyFlags[1] & 0x80000000)
                temp.push_back(DIMINISHING_ROOT);
            // Shattered Barrier
            else if (spellproto->SpellVisual[0] == 12297)
                temp.push_back(DIMINISHING_ROOT);
            // Deep Freeze
            else if (spellproto->SpellIconID == 2939 && spellproto->SpellVisual[0] == 9963)
            {
                temp.push_back(DIMINISHING_CONTROLLED_STUN);
                temp.push_back(DIMINISHING_RING_OF_FROST);
            }
            // Ring of Frost
            else if (spellproto->Id == 82691)
            {
                temp.push_back(DIMINISHING_DISORIENT);
                temp.push_back(DIMINISHING_RING_OF_FROST);
            }
            // Frost Nova / Freeze (Water Elemental)
            else if (spellproto->SpellIconID == 193 || spellproto->Id == 122)
                temp.push_back(DIMINISHING_CONTROLLED_ROOT);
            // Dragon's Breath
            else if (spellproto->SpellFamilyFlags[0] & 0x800000)
                temp.push_back(DIMINISHING_DRAGONS_BREATH);
            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            // Hamstring - limit duration to 10s in PvP
            if (spellproto->SpellFamilyFlags[0] & 0x2)
                temp.push_back(DIMINISHING_LIMITONLY);
            // Patch 4.1.0 (26-Apr-2011): Charge and Intercept no longer have diminishing returns on their stun effects.
            // charge
            if (spellproto->SpellFamilyFlags[0] & 0x01000000)
                temp.push_back(DIMINISHING_NONE);
            // intercept
            if (spellproto->Id == 20253)
                temp.push_back(DIMINISHING_NONE);
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // Curses/etc
            if ((spellproto->SpellFamilyFlags[0] & 0x80000000) || (spellproto->SpellFamilyFlags[1] & 0x200))
                temp.push_back(DIMINISHING_LIMITONLY);
            // Seduction
            else if (spellproto->SpellFamilyFlags[1] & 0x10000000)
                temp.push_back(DIMINISHING_FEAR);
            // Unstable affliction
            else if (spellproto->Id == 31117)
                temp.push_back(DIMINISHING_LIMITONLY);
            // Sin and Punishment - Has Warlock spellfamily
            else if (spellproto->Id == 87204)
                temp.push_back(DIMINISHING_LIMITONLY);
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            // Cyclone
            if (spellproto->SpellFamilyFlags[1] & 0x20)
                temp.push_back(DIMINISHING_CYCLONE);
            // Entangling Roots
            // Nature's Grasp
            else if (spellproto->SpellFamilyFlags[0] & 0x00000200)
                temp.push_back(DIMINISHING_CONTROLLED_ROOT);
            // Faerie Fire
            else if (spellproto->SpellFamilyFlags[0] & 0x400)
                temp.push_back(DIMINISHING_LIMITONLY);
            // Solar beam
            else if (spellproto->Id == 81261)
                temp.push_back(DIMINISHING_LIMITONLY);
            // Feral charge - Bear
            else if (spellproto->Id == 45334)
                temp.push_back(DIMINISHING_NONE);
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            // Gouge
            if (spellproto->SpellFamilyFlags[0] & 0x8)
                temp.push_back(DIMINISHING_DISORIENT);
            // Blind
            else if (spellproto->SpellFamilyFlags[0] & 0x1000000)
                temp.push_back(DIMINISHING_FEAR);
            // Crippling poison - Limit to 10 seconds in PvP (No SpellFamilyFlags)
            else if (spellproto->SpellIconID == 163)
                temp.push_back(DIMINISHING_LIMITONLY);
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // Hunter's Mark
            if ((spellproto->SpellFamilyFlags[0] & 0x400) && spellproto->SpellIconID == 538)
                temp.push_back(DIMINISHING_LIMITONLY);
            // Scatter Shot (own diminishing)
            else if ((spellproto->SpellFamilyFlags[0] & 0x40000) && spellproto->SpellIconID == 132)
                temp.push_back(DIMINISHING_SCATTER_SHOT);
            // Entrapment (own diminishing)
            else if (spellproto->SpellVisual[0] == 7484 && spellproto->SpellIconID == 20)
                temp.push_back(DIMINISHING_ENTRAPMENT);
            // Wyvern Sting mechanic is MECHANIC_SLEEP but the diminishing is DIMINISHING_DISORIENT
            else if ((spellproto->SpellFamilyFlags[1] & 0x1000) && spellproto->SpellIconID == 1721)
                temp.push_back(DIMINISHING_DISORIENT);
            // Freezing Arrow
            else if (spellproto->SpellFamilyFlags[0] & 0x8)
                temp.push_back(DIMINISHING_DISORIENT);
            // Pet - Bad Manner
            else if (spellproto->Id == 90337)
                temp.push_back(DIMINISHING_CONTROLLED_STUN);
            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            // Judgement of Justice - limit duration to 10s in PvP
            if (spellproto->SpellFamilyFlags[0] & 0x100000)
                temp.push_back(DIMINISHING_LIMITONLY);
            // Turn Evil
            else if ((spellproto->SpellFamilyFlags[1] & 0x804000) && spellproto->SpellIconID == 309)
                temp.push_back(DIMINISHING_FEAR);
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            // Storm, Earth and Fire - Earthgrab
            if (spellproto->SpellFamilyFlags[2] & 0x4000)
                temp.push_back(DIMINISHING_NONE);
            break;
        }
        case SPELLFAMILY_DEATHKNIGHT:
        {
            // Hungering Cold (no flags)
            if (spellproto->SpellIconID == 2797)
                temp.push_back(DIMINISHING_DISORIENT);
            // Mark of Blood
            else if ((spellproto->SpellFamilyFlags[0] & 0x10000000) && spellproto->SpellIconID == 2285)
                temp.push_back(DIMINISHING_LIMITONLY);
            break;
        }
        default:
            break;
    }

    if (temp.size())
        return temp;

    // Lastly - Set diminishing depending on mechanic
    uint32 mechanic = spellproto->GetAllEffectsMechanicMask();
    if (mechanic & (1 << MECHANIC_CHARM))
        temp.push_back(DIMINISHING_MIND_CONTROL);
    else if (mechanic & (1 << MECHANIC_SILENCE))
        temp.push_back(DIMINISHING_SILENCE);
    else if (mechanic & (1 << MECHANIC_SLEEP))
        temp.push_back(DIMINISHING_SLEEP);
    else if (mechanic & ((1 << MECHANIC_SAPPED) | (1 << MECHANIC_POLYMORPH) | (1 << MECHANIC_SHACKLE)))
        temp.push_back(DIMINISHING_DISORIENT);
    // Mechanic Knockout, except Blast Wave
    else if (mechanic & (1 << MECHANIC_KNOCKOUT) && spellproto->SpellIconID != 292)
        temp.push_back(DIMINISHING_DISORIENT);
    else if (mechanic & (1 << MECHANIC_DISARM))
        temp.push_back(DIMINISHING_DISARM);
    else if (mechanic & (1 << MECHANIC_FEAR))
        temp.push_back(DIMINISHING_FEAR);
    else if (mechanic & (1 << MECHANIC_STUN))
        triggered ? temp.push_back(DIMINISHING_STUN) : temp.push_back(DIMINISHING_CONTROLLED_STUN);
    else if (mechanic & (1 << MECHANIC_BANISH))
        temp.push_back(DIMINISHING_BANISH);
    else if (mechanic & (1 << MECHANIC_ROOT))
        triggered ? temp.push_back(DIMINISHING_ROOT) : temp.push_back(DIMINISHING_CONTROLLED_ROOT);
    else if (mechanic & (1 << MECHANIC_HORROR))
        temp.push_back(DIMINISHING_HORROR);

    if (!temp.size())
        // All negative slows should be limited to 8 seconds in pvp
        if (spellproto->HasAura(SPELL_AURA_MOD_DECREASE_SPEED))
            temp.push_back(DIMINISHING_LIMITONLY);

    temp.push_back(DIMINISHING_NONE);
    return temp;
}

DiminishingReturnsType GetDiminishingReturnsGroupType(DiminishingGroup group)
{
    switch (group)
    {
        case DIMINISHING_TAUNT:
        case DIMINISHING_CONTROLLED_STUN:
        case DIMINISHING_STUN:
        case DIMINISHING_OPENING_STUN:
        case DIMINISHING_CYCLONE:
        case DIMINISHING_CHARGE:
            return DRTYPE_ALL;
        case DIMINISHING_LIMITONLY:
        case DIMINISHING_NONE:
            return DRTYPE_NONE;
        default:
            return DRTYPE_PLAYER;
    }
}

DiminishingLevels GetDiminishingReturnsMaxLevel(DiminishingGroup group)
{
    switch (group)
    {
        case DIMINISHING_TAUNT:
            return DIMINISHING_LEVEL_TAUNT_IMMUNE;
        default:
            return DIMINISHING_LEVEL_IMMUNE;
    }
}

int32 GetDiminishingReturnsLimitDuration(DiminishingGroup group, SpellInfo const* spellproto)
{
    if (!IsDiminishingReturnsGroupDurationLimited(group))
        return 0;

    // Explicit diminishing duration
    switch (spellproto->SpellFamilyName)
    {
        case SPELLFAMILY_DRUID:
        {
            // Faerie Fire - limit to 40 seconds in PvP (3.1)
            if (spellproto->SpellFamilyFlags[0] & 0x400)
                return 40 * IN_MILLISECONDS;
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // Wyvern Sting
            if (spellproto->SpellFamilyFlags[1] & 0x1000)
                return 6 * IN_MILLISECONDS;
            // Hunter's Mark
            if (spellproto->SpellFamilyFlags[0] & 0x400)
                return 30 * IN_MILLISECONDS;
            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            // Repentance - limit to 6 seconds in PvP
            if (spellproto->SpellFamilyFlags[0] & 0x4)
                return 6 * IN_MILLISECONDS;
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // Banish - limit to 6 seconds in PvP
            if (spellproto->SpellFamilyFlags[1] & 0x8000000)
                return 6 * IN_MILLISECONDS;
            // Curse of Tongues - limit to 12 seconds in PvP
            else if (spellproto->SpellFamilyFlags[2] & 0x800)
                return 12 * IN_MILLISECONDS;
            // Curse of Elements - limit to 120 seconds in PvP
            else if (spellproto->SpellFamilyFlags[1] & 0x200)
               return 120 * IN_MILLISECONDS;
            break;
        }
        default:
            break;
    }

    return 8 * IN_MILLISECONDS;
}

bool IsDiminishingReturnsGroupDurationLimited(DiminishingGroup group)
{
    switch (group)
    {
        case DIMINISHING_BANISH:
        case DIMINISHING_CONTROLLED_STUN:
        case DIMINISHING_CONTROLLED_ROOT:
        case DIMINISHING_CYCLONE:
        case DIMINISHING_DISORIENT:
        case DIMINISHING_ENTRAPMENT:
        case DIMINISHING_FEAR:
        case DIMINISHING_HORROR:
        case DIMINISHING_MIND_CONTROL:
        case DIMINISHING_OPENING_STUN:
        case DIMINISHING_ROOT:
        case DIMINISHING_STUN:
        case DIMINISHING_SLEEP:
        case DIMINISHING_LIMITONLY:
        case DIMINISHING_RING_OF_FROST:
            return true;
        default:
            return false;
    }
}

SpellMgr::SpellMgr()
{
}

SpellMgr::~SpellMgr()
{
    UnloadSpellInfoStore();
}

/// Some checks for spells, to prevent adding deprecated/broken spells for trainers, spell book, etc
bool SpellMgr::IsSpellValid(SpellInfo const* spellInfo, Player* player, bool msg)
{
    // not exist
    if (!spellInfo)
        return false;

    bool need_check_reagents = false;

    // check effects
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        switch (spellInfo->Effects[i].Effect)
        {
            case 0:
                continue;

            // craft spell for crafting non-existed item (break client recipes list show)
            case SPELL_EFFECT_CREATE_ITEM:
            case SPELL_EFFECT_CREATE_ITEM_2:
            {
                if (spellInfo->Effects[i].ItemType == 0)
                {
                    // skip auto-loot crafting spells, its not need explicit item info (but have special fake items sometime)
                    if (!spellInfo->IsLootCrafting())
                    {
                        if (msg)
                        {
                            if (player)
                                ChatHandler(player->GetSession()).PSendSysMessage("Craft spell %u not have create item entry.", spellInfo->Id);
                            else
                                TC_LOG_ERROR("sql.sql", "Craft spell %u not have create item entry.", spellInfo->Id);
                        }
                        return false;
                    }

                }
                // also possible IsLootCrafting case but fake item must exist anyway
                else if (!sObjectMgr->GetItemTemplate(spellInfo->Effects[i].ItemType))
                {
                    if (msg)
                    {
                        if (player)
                            ChatHandler(player->GetSession()).PSendSysMessage("Craft spell %u create not-exist in DB item (Entry: %u) and then...", spellInfo->Id, spellInfo->Effects[i].ItemType);
                        else
                            TC_LOG_ERROR("sql.sql", "Craft spell %u create not-exist in DB item (Entry: %u) and then...", spellInfo->Id, spellInfo->Effects[i].ItemType);
                    }
                    return false;
                }

                need_check_reagents = true;
                break;
            }
            case SPELL_EFFECT_LEARN_SPELL:
            {
                SpellInfo const* spellInfo2 = sSpellMgr->GetSpellInfo(spellInfo->Effects[i].TriggerSpell);
                if (!IsSpellValid(spellInfo2, player, msg))
                {
                    if (msg)
                    {
                        if (player)
                            ChatHandler(player->GetSession()).PSendSysMessage("Spell %u learn to broken spell %u, and then...", spellInfo->Id, spellInfo->Effects[i].TriggerSpell);
                        else
                            TC_LOG_ERROR("sql.sql", "Spell %u learn to invalid spell %u, and then...", spellInfo->Id, spellInfo->Effects[i].TriggerSpell);
                    }
                    return false;
                }
                break;
            }
        }
    }

    if (need_check_reagents)
    {
        for (uint8 j = 0; j < MAX_SPELL_REAGENTS; ++j)
        {
            if (spellInfo->Reagent[j] > 0 && !sObjectMgr->GetItemTemplate(spellInfo->Reagent[j]))
            {
                if (msg)
                {
                    if (player)
                        ChatHandler(player->GetSession()).PSendSysMessage("Craft spell %u have not-exist reagent in DB item (Entry: %u) and then...", spellInfo->Id, spellInfo->Reagent[j]);
                    else
                        TC_LOG_ERROR("sql.sql", "Craft spell %u have not-exist reagent in DB item (Entry: %u) and then...", spellInfo->Id, spellInfo->Reagent[j]);
                }
                return false;
            }
        }
    }

    return true;
}

uint32 SpellMgr::GetSpellDifficultyId(uint32 spellId) const
{
    SpellDifficultySearcherMap::const_iterator i = mSpellDifficultySearcherMap.find(spellId);
    return i == mSpellDifficultySearcherMap.end() ? 0 : (*i).second;
}

void SpellMgr::SetSpellDifficultyId(uint32 spellId, uint32 id)
{
    mSpellDifficultySearcherMap[spellId] = id;
}

uint32 SpellMgr::GetSpellIdForDifficulty(uint32 spellId, uint32 spawnMode) const
{
    if (!GetSpellInfo(spellId))
        return spellId;

    if (spawnMode >= MAX_DIFFICULTY)
    {
        TC_LOG_ERROR("spells", "SpellMgr::GetSpellIdForDifficulty: Incorrect Difficulty for spell %u.", spellId);
        return spellId; //return source spell
    }

    uint32 difficultyId = GetSpellDifficultyId(spellId);
    if (!difficultyId)
        return spellId; //return source spell, it has only REGULAR_DIFFICULTY

    SpellDifficultyEntry const* difficultyEntry = sSpellDifficultyStore.LookupEntry(difficultyId);
    if (!difficultyEntry)
    {
        TC_LOG_DEBUG("spells", "SpellMgr::GetSpellIdForDifficulty: SpellDifficultyEntry not found for spell %u. This should never happen.", spellId);
        return spellId; //return source spell
    }

    if (difficultyEntry->SpellID[spawnMode] <= 0 && spawnMode > DUNGEON_DIFFICULTY_HEROIC)
    {
        TC_LOG_DEBUG("spells", "SpellMgr::GetSpellIdForDifficulty: spell %u mode %u spell is NULL, using mode %u", spellId, spawnMode, spawnMode - 2);
        spawnMode -= 2;
    }

    if (difficultyEntry->SpellID[spawnMode] <= 0)
    {
        TC_LOG_ERROR("sql.sql", "SpellMgr::GetSpellIdForDifficulty: spell %u mode %u spell is 0. Check spelldifficulty_dbc!", spellId, spawnMode);
        return spellId;
    }

    TC_LOG_DEBUG("spells", "SpellMgr::GetSpellIdForDifficulty: spellid for spell %u in mode %u is %d", spellId, spawnMode, difficultyEntry->SpellID[spawnMode]);
    return uint32(difficultyEntry->SpellID[spawnMode]);
}

uint32 SpellMgr::GetSpellIdForDifficulty(uint32 spellId, Unit const* caster) const
{
    if (!caster || !caster->GetMap() || !caster->GetMap()->IsDungeon())
        return spellId;

    return GetSpellIdForDifficulty(spellId, caster->GetMap()->GetSpawnMode());
}

SpellInfo const* SpellMgr::GetSpellForDifficultyFromSpell(SpellInfo const* spell, Unit const* caster) const
{
    uint32 newSpellId = GetSpellIdForDifficulty(spell->Id, caster);
    SpellInfo const* newSpell = GetSpellInfo(newSpellId);
    if (!newSpell || newSpellId == 67200) // hack fix for IOC vehicle
    {
        TC_LOG_DEBUG("spells", "SpellMgr::GetSpellForDifficultyFromSpell: spell %u not found. Check spelldifficulty_dbc!", newSpellId);
        return spell;
    }

    TC_LOG_DEBUG("spells", "SpellMgr::GetSpellForDifficultyFromSpell: Spell id for instance mode is %u (original %u)", newSpell->Id, spell->Id);
    return newSpell;
}

SpellChainNode const* SpellMgr::GetSpellChainNode(uint32 spell_id) const
{
    SpellChainMap::const_iterator itr = mSpellChains.find(spell_id);
    if (itr == mSpellChains.end())
        return NULL;

    return &itr->second;
}

uint32 SpellMgr::GetFirstSpellInChain(uint32 spell_id) const
{
    if (SpellChainNode const* node = GetSpellChainNode(spell_id))
        return node->first->Id;

    return spell_id;
}

uint32 SpellMgr::GetLastSpellInChain(uint32 spell_id) const
{
    if (SpellChainNode const* node = GetSpellChainNode(spell_id))
        return node->last->Id;

    return spell_id;
}

uint32 SpellMgr::GetNextSpellInChain(uint32 spell_id) const
{
    if (SpellChainNode const* node = GetSpellChainNode(spell_id))
        if (node->next)
            return node->next->Id;

    return 0;
}

uint32 SpellMgr::GetPrevSpellInChain(uint32 spell_id) const
{
    if (SpellChainNode const* node = GetSpellChainNode(spell_id))
        if (node->prev)
            return node->prev->Id;

    return 0;
}

uint8 SpellMgr::GetSpellRank(uint32 spell_id) const
{
    if (SpellChainNode const* node = GetSpellChainNode(spell_id))
        return node->rank;

    return 0;
}

uint32 SpellMgr::GetSpellWithRank(uint32 spell_id, uint32 rank, bool strict) const
{
    if (SpellChainNode const* node = GetSpellChainNode(spell_id))
    {
        if (rank != node->rank)
            return GetSpellWithRank(node->rank < rank ? node->next->Id : node->prev->Id, rank, strict);
    }
    else if (strict && rank > 1)
        return 0;
    return spell_id;
}

SpellRequiredMapBounds SpellMgr::GetSpellsRequiredForSpellBounds(uint32 spell_id) const
{
    return mSpellReq.equal_range(spell_id);
}

SpellsRequiringSpellMapBounds SpellMgr::GetSpellsRequiringSpellBounds(uint32 spell_id) const
{
    return mSpellsReqSpell.equal_range(spell_id);
}

bool SpellMgr::IsSpellRequiringSpell(uint32 spellid, uint32 req_spellid) const
{
    SpellsRequiringSpellMapBounds spellsRequiringSpell = GetSpellsRequiringSpellBounds(req_spellid);
    for (SpellsRequiringSpellMap::const_iterator itr = spellsRequiringSpell.first; itr != spellsRequiringSpell.second; ++itr)
    {
        if (itr->second == spellid)
            return true;
    }
    return false;
}

const SpellsRequiringSpellMap SpellMgr::GetSpellsRequiringSpell()
{
    return this->mSpellsReqSpell;
}

uint32 SpellMgr::GetSpellRequired(uint32 spell_id) const
{
    SpellRequiredMap::const_iterator itr = mSpellReq.find(spell_id);

    if (itr == mSpellReq.end())
        return 0;

    return itr->second;
}

SpellLearnSkillNode const* SpellMgr::GetSpellLearnSkill(uint32 spell_id) const
{
    SpellLearnSkillMap::const_iterator itr = mSpellLearnSkills.find(spell_id);
    if (itr != mSpellLearnSkills.end())
        return &itr->second;
    else
        return NULL;
}

SpellLearnSpellMapBounds SpellMgr::GetSpellLearnSpellMapBounds(uint32 spell_id) const
{
    return mSpellLearnSpells.equal_range(spell_id);
}

bool SpellMgr::IsSpellLearnSpell(uint32 spell_id) const
{
    return mSpellLearnSpells.find(spell_id) != mSpellLearnSpells.end();
}

bool SpellMgr::IsSpellLearnToSpell(uint32 spell_id1, uint32 spell_id2) const
{
    SpellLearnSpellMapBounds bounds = GetSpellLearnSpellMapBounds(spell_id1);
    for (SpellLearnSpellMap::const_iterator i = bounds.first; i != bounds.second; ++i)
        if (i->second.spell == spell_id2)
            return true;
    return false;
}

SpellTargetPosition const* SpellMgr::GetSpellTargetPosition(uint32 spell_id) const
{
    SpellTargetPositionMap::const_iterator itr = mSpellTargetPositions.find(spell_id);
    if (itr != mSpellTargetPositions.end())
        return &itr->second;
    return NULL;
}

SpellSpellGroupMapBounds SpellMgr::GetSpellSpellGroupMapBounds(uint32 spell_id) const
{
    spell_id = GetFirstSpellInChain(spell_id);
    return mSpellSpellGroup.equal_range(spell_id);
}

bool SpellMgr::IsSpellMemberOfSpellGroup(uint32 spellid, SpellGroup groupid) const
{
    SpellSpellGroupMapBounds spellGroup = GetSpellSpellGroupMapBounds(spellid);
    for (SpellSpellGroupMap::const_iterator itr = spellGroup.first; itr != spellGroup.second; ++itr)
    {
        if (itr->second == groupid)
            return true;
    }
    return false;
}

SpellGroupSpellMapBounds SpellMgr::GetSpellGroupSpellMapBounds(SpellGroup group_id) const
{
    return mSpellGroupSpell.equal_range(group_id);
}

void SpellMgr::GetSetOfSpellsInSpellGroup(SpellGroup group_id, std::set<uint32>& foundSpells) const
{
    std::set<SpellGroup> usedGroups;
    GetSetOfSpellsInSpellGroup(group_id, foundSpells, usedGroups);
}

void SpellMgr::GetSetOfSpellsInSpellGroup(SpellGroup group_id, std::set<uint32>& foundSpells, std::set<SpellGroup>& usedGroups) const
{
    if (usedGroups.find(group_id) != usedGroups.end())
        return;
    usedGroups.insert(group_id);

    SpellGroupSpellMapBounds groupSpell = GetSpellGroupSpellMapBounds(group_id);
    for (SpellGroupSpellMap::const_iterator itr = groupSpell.first; itr != groupSpell.second; ++itr)
    {
        if (itr->second < 0)
        {
            SpellGroup currGroup = (SpellGroup)abs(itr->second);
            GetSetOfSpellsInSpellGroup(currGroup, foundSpells, usedGroups);
        }
        else
        {
            foundSpells.insert(itr->second);
        }
    }
}

bool SpellMgr::AddSameEffectStackRuleSpellGroups(SpellInfo const* spellInfo, int32 amount, std::map<SpellGroup, int32>& groups) const
{
    uint32 spellId = spellInfo->GetFirstRankSpell()->Id;
    SpellSpellGroupMapBounds spellGroup = GetSpellSpellGroupMapBounds(spellId);
    // Find group with SPELL_GROUP_STACK_RULE_EXCLUSIVE_SAME_EFFECT if it belongs to one
    for (SpellSpellGroupMap::const_iterator itr = spellGroup.first; itr != spellGroup.second; ++itr)
    {
        SpellGroup group = itr->second;
        SpellGroupStackMap::const_iterator found = mSpellGroupStack.find(group);
        if (found != mSpellGroupStack.end())
        {
            if (found->second == SPELL_GROUP_STACK_RULE_EXCLUSIVE_SAME_EFFECT)
            {
                // Put the highest amount in the map
                if (groups.find(group) == groups.end())
                    groups[group] = amount;
                else
                {
                    int32 curr_amount = groups[group];
                    // Take absolute value because this also counts for the highest negative aura
                    if (abs(curr_amount) < abs(amount))
                        groups[group] = amount;
                }
                // return because a spell should be in only one SPELL_GROUP_STACK_RULE_EXCLUSIVE_SAME_EFFECT group
                return true;
            }
        }
    }
    // Not in a SPELL_GROUP_STACK_RULE_EXCLUSIVE_SAME_EFFECT group, so return false
    return false;
}

SpellGroupStackRule SpellMgr::CheckSpellGroupStackRules(SpellInfo const* spellInfo1, SpellInfo const* spellInfo2, bool checkSelf) const
{
    uint32 spellid_1 = spellInfo1->GetFirstRankSpell()->Id;
    uint32 spellid_2 = spellInfo2->GetFirstRankSpell()->Id;
    if (spellid_1 == spellid_2 && !checkSelf)
        return SPELL_GROUP_STACK_RULE_DEFAULT;
    // find SpellGroups which are common for both spells
    SpellSpellGroupMapBounds spellGroup1 = GetSpellSpellGroupMapBounds(spellid_1);
    std::set<SpellGroup> groups;
    for (SpellSpellGroupMap::const_iterator itr = spellGroup1.first; itr != spellGroup1.second; ++itr)
    {
        if (IsSpellMemberOfSpellGroup(spellid_2, itr->second))
        {
            bool add = true;
            SpellGroupSpellMapBounds groupSpell = GetSpellGroupSpellMapBounds(itr->second);
            for (SpellGroupSpellMap::const_iterator itr2 = groupSpell.first; itr2 != groupSpell.second; ++itr2)
            {
                if (itr2->second < 0)
                {
                    SpellGroup currGroup = (SpellGroup)abs(itr2->second);
                    if (IsSpellMemberOfSpellGroup(spellid_1, currGroup) && IsSpellMemberOfSpellGroup(spellid_2, currGroup))
                    {
                        add = false;
                        break;
                    }
                }
            }
            if (add)
                groups.insert(itr->second);
        }
    }

    SpellGroupStackRule rule = SPELL_GROUP_STACK_RULE_DEFAULT;

    for (std::set<SpellGroup>::iterator itr = groups.begin(); itr!= groups.end(); ++itr)
    {
        SpellGroupStackMap::const_iterator found = mSpellGroupStack.find(*itr);
        if (found != mSpellGroupStack.end())
            rule = found->second;
        if (rule)
            break;
    }
    return rule;
}

SpellProcEventEntry const* SpellMgr::GetSpellProcEvent(uint32 spellId) const
{
    SpellProcEventMap::const_iterator itr = mSpellProcEventMap.find(spellId);
    if (itr != mSpellProcEventMap.end())
        return &itr->second;
    return NULL;
}

bool SpellMgr::IsSpellProcEventCanTriggeredBy(SpellInfo const* spellProto, SpellProcEventEntry const* spellProcEvent, uint32 EventProcFlag, SpellInfo const* procSpell, uint32 procFlags, uint32 procExtra, bool active) const
{
    // No extra req need
    uint32 procEvent_procEx = PROC_EX_NONE;

    // check prockFlags for condition
    if ((procFlags & EventProcFlag) == 0)
        return false;

    bool hasFamilyMask = false;

    /**

    * @brief Check auras procced by periodics

    *Only damaging Dots can proc auras with PROC_FLAG_TAKEN_DAMAGE

    *Only Dots can proc if ONLY has PROC_FLAG_DONE_PERIODIC or PROC_FLAG_TAKEN_PERIODIC.

    *Hots can proc if ONLY has PROC_FLAG_DONE_PERIODIC and spellfamily != 0

    *Only Dots can proc auras with PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG or PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG

    *Only Hots can proc auras with PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS or PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS

    *Only Dots can proc auras with PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG or PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_NEG

    *Only Hots can proc auras with PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_POS or PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_POS

    * @param procSpell the spell proccing the aura
    * @param procFlags proc_flags of spellProc
    * @param procExtra proc_EX of procSpell
    * @param EventProcFlag proc_flags of aura to be procced
    * @param spellProto SpellInfo of aura to be procced

    */

    /// Quick Check - If PROC_FLAG_TAKEN_DAMAGE is set for aura and procSpell dealt damage, proc no matter what kind of spell that deals the damage.
    if (procFlags & PROC_FLAG_TAKEN_DAMAGE && EventProcFlag & PROC_FLAG_TAKEN_DAMAGE)
        return true;

    if (procFlags & PROC_FLAG_DONE_PERIODIC && EventProcFlag & PROC_FLAG_DONE_PERIODIC)
    {
        if (procExtra & PROC_EX_INTERNAL_HOT)
        {
            if (EventProcFlag == PROC_FLAG_DONE_PERIODIC)
            {
                /// no aura with only PROC_FLAG_DONE_PERIODIC and spellFamilyName == 0 can proc from a HOT.
                if (spellProto && !spellProto->SpellFamilyName)
                    return false;
            }
            /// Aura must have positive procflags for a HOT to proc
            else if (!(EventProcFlag & (PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS | PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS)))
                return false;
        }
        /// Aura must have negative or neutral(PROC_FLAG_DONE_PERIODIC only) procflags for a DOT to proc
        else if (EventProcFlag != PROC_FLAG_DONE_PERIODIC)
            if (!(EventProcFlag & (PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG | PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG)))
                return false;
    }

    if (procFlags & PROC_FLAG_TAKEN_PERIODIC && EventProcFlag & PROC_FLAG_TAKEN_PERIODIC)
    {
        if (procExtra & PROC_EX_INTERNAL_HOT)
        {
            /// No aura that only has PROC_FLAG_TAKEN_PERIODIC can proc from a HOT.
            if (EventProcFlag == PROC_FLAG_TAKEN_PERIODIC)
                return false;
            /// Aura must have positive procflags for a HOT to proc
            if (!(EventProcFlag & (PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_POS | PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_POS)))
                return false;
        }
        /// Aura must have negative or neutral(PROC_FLAG_TAKEN_PERIODIC only) procflags for a DOT to proc
        else if (EventProcFlag != PROC_FLAG_TAKEN_PERIODIC)
            if (!(EventProcFlag & (PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG | PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_NEG)))
                return false;
    }
    // Trap casts are active by default
    if (procFlags & PROC_FLAG_DONE_TRAP_ACTIVATION)
        active = true;

    // Always trigger for this
    if (procFlags & (PROC_FLAG_KILLED | PROC_FLAG_KILL | PROC_FLAG_DEATH))
        return true;

    if (spellProcEvent)     // Exist event data
    {
        // Store extra req
        procEvent_procEx = spellProcEvent->procEx;

        // For melee triggers
        if (procSpell == NULL)
        {
            // Check (if set) for school (melee attack have Normal school)
            if (spellProcEvent->schoolMask && (spellProcEvent->schoolMask & SPELL_SCHOOL_MASK_NORMAL) == 0)
                return false;
        }
        else // For spells need check school/spell family/family mask
        {
            // Check (if set) for school
            if (spellProcEvent->schoolMask && (spellProcEvent->schoolMask & procSpell->SchoolMask) == 0)
                return false;

            // Check (if set) for spellFamilyName
            if (spellProcEvent->spellFamilyName && (spellProcEvent->spellFamilyName != procSpell->SpellFamilyName))
                return false;

            // spellFamilyName is Ok need check for spellFamilyMask if present
            if (spellProcEvent->spellFamilyMask)
            {
                if (!(spellProcEvent->spellFamilyMask & procSpell->SpellFamilyFlags))
                    return false;
                hasFamilyMask = true;
                // Some spells are not considered as active even with have spellfamilyflags
                if (!(procEvent_procEx & PROC_EX_ONLY_ACTIVE_SPELL))
                    active = true;
            }
        }
    }

    if (procFlags & PROC_FLAG_ON_CAST && EventProcFlag & PROC_FLAG_ON_CAST)
        return true;

    if (procExtra & (PROC_EX_INTERNAL_REQ_FAMILY))
    {
        if (!hasFamilyMask)
            return false;
    }

    // Check for extra req (if none) and hit/crit
    if (procEvent_procEx == PROC_EX_NONE)
    {
        // No extra req, so can trigger only for hit/crit - spell has to be active
        if ((procExtra & (PROC_EX_NORMAL_HIT | PROC_EX_CRITICAL_HIT)) && active)
            return true;
    }
    else // Passive spells hits here only if resist/reflect/immune/evade
    {
        if (procExtra & AURA_SPELL_PROC_EX_MASK)
        {
            // if spell marked as procing only from not active spells
            if (active && procEvent_procEx & PROC_EX_NOT_ACTIVE_SPELL)
                return false;
            // if spell marked as procing only from active spells
            if (!active && procEvent_procEx & PROC_EX_ONLY_ACTIVE_SPELL)
                return false;
            // Exist req for PROC_EX_EX_TRIGGER_ALWAYS
            if (procEvent_procEx & PROC_EX_EX_TRIGGER_ALWAYS)
                return true;
            // PROC_EX_NOT_ACTIVE_SPELL and PROC_EX_ONLY_ACTIVE_SPELL flags handle: if passed checks before
            if ((procExtra & (PROC_EX_NORMAL_HIT | PROC_EX_CRITICAL_HIT)) && ((procEvent_procEx & (AURA_SPELL_PROC_EX_MASK)) == 0))
                return true;
        }
        // Check Extra Requirement like (hit/crit/miss/resist/parry/dodge/block/immune/reflect/absorb and other)
        if (procEvent_procEx & procExtra)
            return true;
    }
    return false;
}

SpellProcEntry const* SpellMgr::GetSpellProcEntry(uint32 spellId) const
{
    SpellProcMap::const_iterator itr = mSpellProcMap.find(spellId);
    if (itr != mSpellProcMap.end())
        return &itr->second;
    return NULL;
}

bool SpellMgr::CanSpellTriggerProcOnEvent(SpellProcEntry const& procEntry, ProcEventInfo& eventInfo)
{
    // proc type doesn't match
    if (!(eventInfo.GetTypeMask() & procEntry.typeMask))
        return false;

    if (eventInfo.GetTypeMask() & PROC_FLAG_ON_CAST)
        return true;

    // check XP or honor target requirement
    if (procEntry.attributesMask & PROC_ATTR_REQ_EXP_OR_HONOR)
        if (Player* actor = eventInfo.GetActor()->ToPlayer())
            if (eventInfo.GetActionTarget() && !actor->isHonorOrXPTarget(eventInfo.GetActionTarget()))
                return false;

    // always trigger for these types
    if (eventInfo.GetTypeMask() & (PROC_FLAG_KILLED | PROC_FLAG_KILL | PROC_FLAG_DEATH))
        return true;

    // check school mask (if set) for other trigger types
    if (procEntry.schoolMask && !(eventInfo.GetSchoolMask() & procEntry.schoolMask))
        return false;

    // check spell family name/flags (if set) for spells
    if (eventInfo.GetTypeMask() & (PERIODIC_PROC_FLAG_MASK | SPELL_PROC_FLAG_MASK | PROC_FLAG_DONE_TRAP_ACTIVATION))
    {
        if (procEntry.spellFamilyName && (procEntry.spellFamilyName != eventInfo.GetSpellInfo()->SpellFamilyName))
            return false;

        if (procEntry.spellFamilyMask && !(procEntry.spellFamilyMask & eventInfo.GetSpellInfo()->SpellFamilyFlags))
            return false;
    }

    // check spell type mask (if set)
    if (eventInfo.GetTypeMask() & (SPELL_PROC_FLAG_MASK | PERIODIC_PROC_FLAG_MASK))
    {
        if (procEntry.spellTypeMask && !(eventInfo.GetSpellTypeMask() & procEntry.spellTypeMask))
            return false;
    }

    // check spell phase mask
    if (eventInfo.GetTypeMask() & REQ_SPELL_PHASE_PROC_FLAG_MASK)
    {
        if (!(eventInfo.GetSpellPhaseMask() & procEntry.spellPhaseMask))
            return false;
    }

    // check hit mask (on taken hit or on done hit, but not on spell cast phase)
    if ((eventInfo.GetTypeMask() & TAKEN_HIT_PROC_FLAG_MASK) || ((eventInfo.GetTypeMask() & DONE_HIT_PROC_FLAG_MASK) && !(eventInfo.GetSpellPhaseMask() & PROC_SPELL_PHASE_CAST)))
    {
        uint32 hitMask = procEntry.hitMask;
        // get default values if hit mask not set
        if (!hitMask)
        {
            // for taken procs allow normal + critical hits by default
            if (eventInfo.GetTypeMask() & TAKEN_HIT_PROC_FLAG_MASK)
                hitMask |= PROC_HIT_NORMAL | PROC_HIT_CRITICAL;
            // for done procs allow normal + critical + absorbs by default
            else
                hitMask |= PROC_HIT_NORMAL | PROC_HIT_CRITICAL | PROC_HIT_ABSORB;
        }
        if (!(eventInfo.GetHitMask() & hitMask))
            return false;
    }

    return true;
}

SpellBonusEntry const* SpellMgr::GetSpellBonusData(uint32 spellId) const
{
    // Lookup data
    SpellBonusMap::const_iterator itr = mSpellBonusMap.find(spellId);
    if (itr != mSpellBonusMap.end())
        return &itr->second;
    // Not found, try lookup for 1 spell rank if exist
    if (uint32 rank_1 = GetFirstSpellInChain(spellId))
    {
        SpellBonusMap::const_iterator itr2 = mSpellBonusMap.find(rank_1);
        if (itr2 != mSpellBonusMap.end())
            return &itr2->second;
    }
    return NULL;
}

SpellThreatEntry const* SpellMgr::GetSpellThreatEntry(uint32 spellID) const
{
    SpellThreatMap::const_iterator itr = mSpellThreatMap.find(spellID);
    if (itr != mSpellThreatMap.end())
        return &itr->second;
    else
    {
        uint32 firstSpell = GetFirstSpellInChain(spellID);
        itr = mSpellThreatMap.find(firstSpell);
        if (itr != mSpellThreatMap.end())
            return &itr->second;
    }
    return NULL;
}

SkillLineAbilityMapBounds SpellMgr::GetSkillLineAbilityMapBounds(uint32 spell_id) const
{
    return mSkillLineAbilityMap.equal_range(spell_id);
}

PetAura const* SpellMgr::GetPetAura(uint32 spell_id, uint8 eff)
{
    SpellPetAuraMap::const_iterator itr = mSpellPetAuraMap.find((spell_id<<8) + eff);
    if (itr != mSpellPetAuraMap.end())
        return &itr->second;
    else
        return NULL;
}

SpellEnchantProcEntry const* SpellMgr::GetSpellEnchantProcEvent(uint32 enchId) const
{
    SpellEnchantProcEventMap::const_iterator itr = mSpellEnchantProcEventMap.find(enchId);
    if (itr != mSpellEnchantProcEventMap.end())
        return &itr->second;
    return NULL;
}

bool SpellMgr::IsArenaAllowedEnchancment(uint32 ench_id) const
{
    return mEnchantCustomAttr[ench_id];
}

const std::vector<int32>* SpellMgr::GetSpellLinked(int32 spell_id) const
{
    SpellLinkedMap::const_iterator itr = mSpellLinkedMap.find(spell_id);
    return itr != mSpellLinkedMap.end() ? &(itr->second) : NULL;
}

PetLevelupSpellSet const* SpellMgr::GetPetLevelupSpellList(uint32 petFamily) const
{
    PetLevelupSpellMap::const_iterator itr = mPetLevelupSpellMap.find(petFamily);
    if (itr != mPetLevelupSpellMap.end())
        return &itr->second;
    else
        return NULL;
}

PetDefaultSpellsEntry const* SpellMgr::GetPetDefaultSpellsEntry(int32 id) const
{
    PetDefaultSpellsMap::const_iterator itr = mPetDefaultSpellsMap.find(id);
    if (itr != mPetDefaultSpellsMap.end())
        return &itr->second;
    return NULL;
}

SpellAreaMapBounds SpellMgr::GetSpellAreaMapBounds(uint32 spell_id) const
{
    return mSpellAreaMap.equal_range(spell_id);
}

SpellAreaForQuestMapBounds SpellMgr::GetSpellAreaForQuestMapBounds(uint32 quest_id) const
{
    return mSpellAreaForQuestMap.equal_range(quest_id);
}

SpellAreaForQuestMapBounds SpellMgr::GetSpellAreaForQuestEndMapBounds(uint32 quest_id) const
{
    return mSpellAreaForQuestEndMap.equal_range(quest_id);
}

SpellAreaForAuraMapBounds SpellMgr::GetSpellAreaForAuraMapBounds(uint32 spell_id) const
{
    return mSpellAreaForAuraMap.equal_range(spell_id);
}

SpellAreaForAreaMapBounds SpellMgr::GetSpellAreaForAreaMapBounds(uint32 area_id) const
{
    return mSpellAreaForAreaMap.equal_range(area_id);
}

bool SpellArea::IsFitToRequirements(Player const* player, uint32 newZone, uint32 newArea) const
{
    if (gender != GENDER_NONE)                   // not in expected gender
        if (!player || gender != player->getGender())
            return false;

    if (raceMask)                                // not in expected race
        if (!player || !(raceMask & player->getRaceMask()))
            return false;

    if (areaId)                                  // not in expected zone
        if (newZone != areaId && newArea != areaId)
            return false;

    if (questStart)                              // not in expected required quest state
        if (!player || (((1 << player->GetQuestStatus(questStart)) & questStartStatus) == 0))
            return false;

    if (questEnd)                                // not in expected forbidden quest state
        if (!player || (((1 << player->GetQuestStatus(questEnd)) & questEndStatus) == 0))
            return false;

    if (auraSpell)                               // not have expected aura
        if (!player || (auraSpell > 0 && !player->HasAura(auraSpell)) || (auraSpell < 0 && player->HasAura(-auraSpell)))
            return false;

    // Extra conditions -- leaving the possibility add extra conditions...
    switch (spellId)
    {
        case 91604: // No fly Zone - Wintergrasp
        {
            if (!player)
                return false;

            Battlefield* Bf = sBattlefieldMgr->GetBattlefieldToZoneId(player->GetZoneId());
            if (!Bf || Bf->CanFlyIn() || (!player->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) && !player->HasAuraType(SPELL_AURA_FLY)))
                return false;
            break;
        }
        case 68719: // Oil Refinery - Isle of Conquest.
        case 68720: // Quarry - Isle of Conquest.
        {
            if (!player || player->GetBattlegroundTypeId() != BATTLEGROUND_IC || !player->GetBattleground())
                return false;

            uint8 nodeType = spellId == 68719 ? NODE_TYPE_REFINERY : NODE_TYPE_QUARRY;
            uint8 nodeState = player->GetTeamId() == TEAM_ALLIANCE ? NODE_STATE_CONTROLLED_A : NODE_STATE_CONTROLLED_H;

            BattlegroundIC* pIC = static_cast<BattlegroundIC*>(player->GetBattleground());
            if (pIC->GetNodeState(nodeType) == nodeState)
                return true;

            return false;
        }
        case 56618: // Horde Controls Factory Phase Shift
        case 56617: // Alliance Controls Factory Phase Shift
        {
            if (!player)
                return false;

            Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(player->GetZoneId());

            if (!bf || bf->GetTypeId() != BATTLEFIELD_WG)
                return false;

            // team that controls the workshop in the specified area
            uint32 team = bf->GetData(newArea);

            if (team == TEAM_HORDE)
                return spellId == 56618;
            else if (team == TEAM_ALLIANCE)
                return spellId == 56617;
            break;
        }
        case 57940: // Essence of Wintergrasp - Northrend
        case 58045: // Essence of Wintergrasp - Wintergrasp
        {
            if (!player)
                return false;

            if (Battlefield* battlefieldWG = sBattlefieldMgr->GetBattlefieldByBattleId(BATTLEFIELD_BATTLEID_WG))
                return battlefieldWG->IsEnabled() && (player->GetTeamId() == battlefieldWG->GetDefenderTeam()) && !battlefieldWG->IsWarTime();
            break;
        }
        case 74411: // Battleground - Dampening
        {
            if (!player)
                return false;

            if (Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(player->GetZoneId()))
                return bf->IsWarTime();
            break;
        }

    }

    return true;
}

void SpellMgr::LoadSpellRanks()
{
    uint32 oldMSTime = getMSTime();

    // cleanup core data before reload - remove reference to ChainNode from SpellInfo
    for (SpellChainMap::iterator itr = mSpellChains.begin(); itr != mSpellChains.end(); ++itr)
    {
        mSpellInfoMap[itr->first]->ChainEntry = NULL;
    }
    mSpellChains.clear();
    //                                                     0             1      2
    QueryResult result = WorldDatabase.Query("SELECT first_spell_id, spell_id, rank from spell_ranks ORDER BY first_spell_id, rank");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell rank records. DB table `spell_ranks` is empty.");

        return;
    }

    uint32 count = 0;
    bool finished = false;

    do
    {
                        // spellid, rank
        std::list < std::pair < int32, int32 > > rankChain;
        int32 currentSpell = -1;
        int32 lastSpell = -1;

        // fill one chain
        while (currentSpell == lastSpell && !finished)
        {
            Field* fields = result->Fetch();

            currentSpell = fields[0].GetUInt32();
            if (lastSpell == -1)
                lastSpell = currentSpell;
            uint32 spell_id = fields[1].GetUInt32();
            uint32 rank = fields[2].GetUInt8();

            // don't drop the row if we're moving to the next rank
            if (currentSpell == lastSpell)
            {
                rankChain.push_back(std::make_pair(spell_id, rank));
                if (!result->NextRow())
                    finished = true;
            }
            else
                break;
        }
        // check if chain is made with valid first spell
        SpellInfo const* first = GetSpellInfo(lastSpell);
        if (!first)
        {
            TC_LOG_ERROR("sql.sql", "Spell rank identifier(first_spell_id) %u listed in `spell_ranks` does not exist!", lastSpell);
            continue;
        }
        // check if chain is long enough
        if (rankChain.size() < 2)
        {
            TC_LOG_ERROR("sql.sql", "There is only 1 spell rank for identifier(first_spell_id) %u in `spell_ranks`, entry is not needed!", lastSpell);
            continue;
        }
        int32 curRank = 0;
        bool valid = true;
        // check spells in chain
        for (std::list<std::pair<int32, int32> >::iterator itr = rankChain.begin(); itr!= rankChain.end(); ++itr)
        {
            SpellInfo const* spell = GetSpellInfo(itr->first);
            if (!spell)
            {
                TC_LOG_ERROR("sql.sql", "Spell %u (rank %u) listed in `spell_ranks` for chain %u does not exist!", itr->first, itr->second, lastSpell);
                valid = false;
                break;
            }
            ++curRank;
            if (itr->second != curRank)
            {
                TC_LOG_ERROR("sql.sql", "Spell %u (rank %u) listed in `spell_ranks` for chain %u does not have proper rank value(should be %u)!", itr->first, itr->second, lastSpell, curRank);
                valid = false;
                break;
            }
        }
        if (!valid)
            continue;
        int32 prevRank = 0;
        // insert the chain
        std::list<std::pair<int32, int32> >::iterator itr = rankChain.begin();
        do
        {
            ++count;
            int32 addedSpell = itr->first;
            mSpellChains[addedSpell].first = GetSpellInfo(lastSpell);
            mSpellChains[addedSpell].last = GetSpellInfo(rankChain.back().first);
            mSpellChains[addedSpell].rank = itr->second;
            mSpellChains[addedSpell].prev = GetSpellInfo(prevRank);
            mSpellInfoMap[addedSpell]->ChainEntry = &mSpellChains[addedSpell];
            prevRank = addedSpell;
            ++itr;

            if (itr == rankChain.end())
            {
                mSpellChains[addedSpell].next = NULL;
                break;
            }
            else
                mSpellChains[addedSpell].next = GetSpellInfo(itr->first);
        }
        while (true);
    } while (!finished);

    TC_LOG_INFO("server.loading", ">> Loaded %u spell rank records in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

}

void SpellMgr::LoadSpellRequired()
{
    uint32 oldMSTime = getMSTime();

    mSpellsReqSpell.clear();                                   // need for reload case
    mSpellReq.clear();                                         // need for reload case

    //                                                   0        1
    QueryResult result = WorldDatabase.Query("SELECT spell_id, req_spell from spell_required");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell required records. DB table `spell_required` is empty.");

        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 spell_id = fields[0].GetUInt32();
        uint32 spell_req = fields[1].GetUInt32();

        // check if chain is made with valid first spell
        SpellInfo const* spell = GetSpellInfo(spell_id);
        if (!spell)
        {
            TC_LOG_ERROR("sql.sql", "spell_id %u in `spell_required` table is not found in dbcs, skipped", spell_id);
            continue;
        }

        SpellInfo const* req_spell = GetSpellInfo(spell_req);
        if (!req_spell)
        {
            TC_LOG_ERROR("sql.sql", "req_spell %u in `spell_required` table is not found in dbcs, skipped", spell_req);
            continue;
        }

        if (GetFirstSpellInChain(spell_id) == GetFirstSpellInChain(spell_req))
        {
            TC_LOG_ERROR("sql.sql", "req_spell %u and spell_id %u in `spell_required` table are ranks of the same spell, entry not needed, skipped", spell_req, spell_id);
            continue;
        }

        if (IsSpellRequiringSpell(spell_id, spell_req))
        {
            TC_LOG_ERROR("sql.sql", "duplicated entry of req_spell %u and spell_id %u in `spell_required`, skipped", spell_req, spell_id);
            continue;
        }

        mSpellReq.insert (std::pair<uint32, uint32>(spell_id, spell_req));
        mSpellsReqSpell.insert (std::pair<uint32, uint32>(spell_req, spell_id));
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u spell required records in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

}

void SpellMgr::LoadSpellLearnSkills()
{
    uint32 oldMSTime = getMSTime();

    mSpellLearnSkills.clear();                              // need for reload case

    // search auto-learned skills and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    for (uint32 spell = 0; spell < GetSpellInfoStoreSize(); ++spell)
    {
        SpellInfo const* entry = GetSpellInfo(spell);

        if (!entry)
            continue;

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (entry->Effects[i].Effect == SPELL_EFFECT_SKILL)
            {
                SpellLearnSkillNode dbc_node;
                dbc_node.skill = entry->Effects[i].MiscValue;
                dbc_node.step  = entry->Effects[i].CalcValue();
                if (dbc_node.skill != SKILL_RIDING)
                    dbc_node.value = 1;
                else
                    dbc_node.value = dbc_node.step * 75;
                dbc_node.maxvalue = dbc_node.step * 75;
                mSpellLearnSkills[spell] = dbc_node;
                ++dbc_count;
                break;
            }
        }
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u Spell Learn Skills from DBC in %u ms", dbc_count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellLearnSpells()
{
    uint32 oldMSTime = getMSTime();

    mSpellLearnSpells.clear();                              // need for reload case

    //                                                  0      1        2
    QueryResult result = WorldDatabase.Query("SELECT entry, SpellID, Active FROM spell_learn_spell");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell learn spells. DB table `spell_learn_spell` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 spell_id = fields[0].GetUInt32();

        SpellLearnSpellNode node;
        node.spell       = fields[1].GetUInt32();
        node.active      = fields[2].GetBool();
        node.autoLearned = false;

        if (!GetSpellInfo(spell_id))
        {
            TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_learn_spell` does not exist", spell_id);
            continue;
        }

        if (!GetSpellInfo(node.spell))
        {
            TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_learn_spell` learning not existed spell %u", spell_id, node.spell);
            continue;
        }

        if (GetTalentSpellCost(node.spell))
        {
            TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_learn_spell` attempt learning talent spell %u, skipped", spell_id, node.spell);
            continue;
        }

        mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell_id, node));

        ++count;
    } while (result->NextRow());

    // search auto-learned spells and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    for (uint32 spell = 0; spell < GetSpellInfoStoreSize(); ++spell)
    {
        SpellInfo const* entry = GetSpellInfo(spell);

        if (!entry)
            continue;

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (entry->Effects[i].Effect == SPELL_EFFECT_LEARN_SPELL)
            {
                SpellLearnSpellNode dbc_node;
                dbc_node.spell = entry->Effects[i].TriggerSpell;
                dbc_node.active = true;                     // all dbc based learned spells is active (show in spell book or hide by client itself)

                // ignore learning not existed spells (broken/outdated/or generic learnig spell 483
                if (!GetSpellInfo(dbc_node.spell))
                    continue;

                // talent or passive spells or skill-step spells auto-casted and not need dependent learning,
                // pet teaching spells must not be dependent learning (casted)
                // other required explicit dependent learning
                dbc_node.autoLearned = entry->Effects[i].TargetA.GetTarget() == TARGET_UNIT_PET || GetTalentSpellCost(spell) > 0 || entry->IsPassive() || entry->HasEffect(SPELL_EFFECT_SKILL_STEP);

                SpellLearnSpellMapBounds db_node_bounds = GetSpellLearnSpellMapBounds(spell);

                bool found = false;
                for (SpellLearnSpellMap::const_iterator itr = db_node_bounds.first; itr != db_node_bounds.second; ++itr)
                {
                    if (itr->second.spell == dbc_node.spell)
                    {
                        TC_LOG_ERROR("sql.sql", "Spell %u auto-learn spell %u in spell.dbc then the record in `spell_learn_spell` is redundant, please fix DB.",
                            spell, dbc_node.spell);
                        found = true;
                        break;
                    }
                }

                if (!found)                                  // add new spell-spell pair if not found
                {
                    mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell, dbc_node));
                    ++dbc_count;
                }
            }
        }
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u spell learn spells + %u found in DBC in %u ms", count, dbc_count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellTargetPositions()
{
    uint32 oldMSTime = getMSTime();

    mSpellTargetPositions.clear();                                // need for reload case

    //                                                0      1              2                  3                  4                  5
    QueryResult result = WorldDatabase.Query("SELECT id, target_map, target_position_x, target_position_y, target_position_z, target_orientation FROM spell_target_position");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell target coordinates. DB table `spell_target_position` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 Spell_ID = fields[0].GetUInt32();

        SpellTargetPosition st;

        st.target_mapId       = fields[1].GetUInt16();
        st.target_X           = fields[2].GetFloat();
        st.target_Y           = fields[3].GetFloat();
        st.target_Z           = fields[4].GetFloat();
        st.target_Orientation = fields[5].GetFloat();

        MapEntry const* mapEntry = sMapStore.LookupEntry(st.target_mapId);
        if (!mapEntry)
        {
            TC_LOG_ERROR("sql.sql", "Spell (ID:%u) target map (ID: %u) does not exist in `Map.dbc`.", Spell_ID, st.target_mapId);
            continue;
        }

        if (st.target_X==0 && st.target_Y==0 && st.target_Z==0)
        {
            TC_LOG_ERROR("sql.sql", "Spell (ID:%u) target coordinates not provided.", Spell_ID);
            continue;
        }

        SpellInfo const* spellInfo = GetSpellInfo(Spell_ID);
        if (!spellInfo)
        {
            TC_LOG_ERROR("sql.sql", "Spell (ID:%u) listed in `spell_target_position` does not exist.", Spell_ID);
            continue;
        }

        bool found = false;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (spellInfo->Effects[i].TargetA.GetTarget() == TARGET_DEST_DB || spellInfo->Effects[i].TargetB.GetTarget() == TARGET_DEST_DB)
            {
                // additional requirements
                if (spellInfo->Effects[i].Effect == SPELL_EFFECT_BIND && spellInfo->Effects[i].MiscValue)
                {
                    uint32 area_id = sMapMgr->GetAreaId(st.target_mapId, st.target_X, st.target_Y, st.target_Z);
                    if (area_id != uint32(spellInfo->Effects[i].MiscValue))
                    {
                        TC_LOG_ERROR("sql.sql", "Spell (Id: %u) listed in `spell_target_position` expected point to zone %u bit point to zone %u.", Spell_ID, spellInfo->Effects[i].MiscValue, area_id);
                        break;
                    }
                }

                found = true;
                break;
            }
        }
        if (!found)
        {
            TC_LOG_ERROR("sql.sql", "Spell (Id: %u) listed in `spell_target_position` does not have target TARGET_DEST_DB (17).", Spell_ID);
            continue;
        }

        mSpellTargetPositions[Spell_ID] = st;
        ++count;

    } while (result->NextRow());

    /*
    // Check all spells
    for (uint32 i = 1; i < GetSpellInfoStoreSize; ++i)
    {
        SpellInfo const* spellInfo = GetSpellInfo(i);
        if (!spellInfo)
            continue;

        bool found = false;
        for (int j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            switch (spellInfo->Effects[j].TargetA)
            {
                case TARGET_DEST_DB:
                    found = true;
                    break;
            }
            if (found)
                break;
            switch (spellInfo->Effects[j].TargetB)
            {
                case TARGET_DEST_DB:
                    found = true;
                    break;
            }
            if (found)
                break;
        }
        if (found)
        {
            if (!sSpellMgr->GetSpellTargetPosition(i))
                TC_LOG_DEBUG("spells", "Spell (ID: %u) does not have record in `spell_target_position`", i);
        }
    }*/

    TC_LOG_INFO("server.loading", ">> Loaded %u spell teleport coordinates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellGroups()
{
    uint32 oldMSTime = getMSTime();

    mSpellSpellGroup.clear();                                  // need for reload case
    mSpellGroupSpell.clear();

    //                                                0     1
    QueryResult result = WorldDatabase.Query("SELECT id, spell_id FROM spell_group");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell group definitions. DB table `spell_group` is empty.");
        return;
    }

    std::set<uint32> groups;
    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 group_id = fields[0].GetUInt32();
        if (group_id <= SPELL_GROUP_DB_RANGE_MIN && group_id >= SPELL_GROUP_CORE_RANGE_MAX)
        {
            TC_LOG_ERROR("sql.sql", "SpellGroup id %u listed in `spell_group` is in core range, but is not defined in core!", group_id);
            continue;
        }
        int32 spell_id = fields[1].GetInt32();

        groups.insert(std::set<uint32>::value_type(group_id));
        mSpellGroupSpell.insert(SpellGroupSpellMap::value_type((SpellGroup)group_id, spell_id));

    } while (result->NextRow());

    for (SpellGroupSpellMap::iterator itr = mSpellGroupSpell.begin(); itr!= mSpellGroupSpell.end();)
    {
        if (itr->second < 0)
        {
            if (groups.find(abs(itr->second)) == groups.end())
            {
                TC_LOG_ERROR("sql.sql", "SpellGroup id %u listed in `spell_group` does not exist", abs(itr->second));
                mSpellGroupSpell.erase(itr++);
            }
            else
                ++itr;
        }
        else
        {
            SpellInfo const* spellInfo = GetSpellInfo(itr->second);

            if (!spellInfo)
            {
                TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_group` does not exist", itr->second);
                mSpellGroupSpell.erase(itr++);
            }
            else if (spellInfo->GetRank() > 1)
            {
                TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_group` is not first rank of spell", itr->second);
                mSpellGroupSpell.erase(itr++);
            }
            else
                ++itr;
        }
    }

    for (std::set<uint32>::iterator groupItr = groups.begin(); groupItr != groups.end(); ++groupItr)
    {
        std::set<uint32> spells;
        GetSetOfSpellsInSpellGroup(SpellGroup(*groupItr), spells);

        for (std::set<uint32>::iterator spellItr = spells.begin(); spellItr != spells.end(); ++spellItr)
        {
            ++count;
            mSpellSpellGroup.insert(SpellSpellGroupMap::value_type(*spellItr, SpellGroup(*groupItr)));
        }
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u spell group definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellGroupStackRules()
{
    uint32 oldMSTime = getMSTime();

    mSpellGroupStack.clear();                                  // need for reload case

    //                                                       0         1
    QueryResult result = WorldDatabase.Query("SELECT group_id, stack_rule FROM spell_group_stack_rules");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell group stack rules. DB table `spell_group_stack_rules` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 group_id = fields[0].GetUInt32();
        uint8 stack_rule = fields[1].GetInt8();
        if (stack_rule >= SPELL_GROUP_STACK_RULE_MAX)
        {
            TC_LOG_ERROR("sql.sql", "SpellGroupStackRule %u listed in `spell_group_stack_rules` does not exist", stack_rule);
            continue;
        }

        SpellGroupSpellMapBounds spellGroup = GetSpellGroupSpellMapBounds((SpellGroup)group_id);

        if (spellGroup.first == spellGroup.second)
        {
            TC_LOG_ERROR("sql.sql", "SpellGroup id %u listed in `spell_group_stack_rules` does not exist", group_id);
            continue;
        }

        mSpellGroupStack[(SpellGroup)group_id] = (SpellGroupStackRule)stack_rule;

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u spell group stack rules in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellProcEvents()
{
    uint32 oldMSTime = getMSTime();

    mSpellProcEventMap.clear();                             // need for reload case

    //                                                0      1           2                3                 4                 5                 6          7       8        9             10
    QueryResult result = WorldDatabase.Query("SELECT entry, SchoolMask, SpellFamilyName, SpellFamilyMask0, SpellFamilyMask1, SpellFamilyMask2, procFlags, procEx, ppmRate, CustomChance, Cooldown FROM spell_proc_event");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell proc event conditions. DB table `spell_proc_event` is empty.");
        return;
    }

    uint32 count = 0;
    uint32 customProc = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        SpellInfo const* spell = GetSpellInfo(entry);
        if (!spell)
        {
            TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_proc_event` does not exist", entry);
            continue;
        }

        SpellProcEventEntry spe;

        spe.schoolMask      = fields[1].GetInt8();
        spe.spellFamilyName = fields[2].GetUInt16();
        spe.spellFamilyMask[0] = fields[3].GetUInt32();
        spe.spellFamilyMask[1] = fields[4].GetUInt32();
        spe.spellFamilyMask[2] = fields[5].GetUInt32();
        spe.procFlags       = fields[6].GetUInt32();
        spe.procEx          = fields[7].GetUInt32();
        spe.ppmRate         = fields[8].GetFloat();
        spe.customChance    = fields[9].GetFloat();
        spe.cooldown        = fields[10].GetUInt32();

        mSpellProcEventMap[entry] = spe;

        if (spell->ProcFlags == 0)
        {
            if (spe.procFlags == 0)
            {
                TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_proc_event` probally not triggered spell", entry);
                continue;
            }
            customProc++;
        }
        ++count;
    } while (result->NextRow());

    if (customProc)
        TC_LOG_INFO("server.loading", ">> Loaded %u extra and %u custom spell proc event conditions in %u ms",  count, customProc, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_INFO("server.loading", ">> Loaded %u extra spell proc event conditions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

}

void SpellMgr::LoadSpellProcs()
{
    uint32 oldMSTime = getMSTime();

    mSpellProcMap.clear();                             // need for reload case

    //                                                 0        1           2                3                 4                 5                 6         7              8               9        10              11             12      13        14
    QueryResult result = WorldDatabase.Query("SELECT spellId, schoolMask, spellFamilyName, spellFamilyMask0, spellFamilyMask1, spellFamilyMask2, typeMask, spellTypeMask, spellPhaseMask, hitMask, attributesMask, ratePerMinute, chance, cooldown, charges FROM spell_proc");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell proc conditions and data. DB table `spell_proc` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        int32 spellId = fields[0].GetInt32();

        bool allRanks = false;
        if (spellId < 0)
        {
            allRanks = true;
            spellId = -spellId;
        }

        SpellInfo const* spellInfo = GetSpellInfo(spellId);
        if (!spellInfo)
        {
            TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_proc` does not exist", spellId);
            continue;
        }

        if (allRanks)
        {
            if (spellInfo->GetFirstRankSpell()->Id != uint32(spellId))
            {
                TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_proc` is not first rank of spell.", spellId);
                continue;
            }
        }

        SpellProcEntry baseProcEntry;

        baseProcEntry.schoolMask      = fields[1].GetInt8();
        baseProcEntry.spellFamilyName = fields[2].GetUInt16();
        baseProcEntry.spellFamilyMask[0] = fields[3].GetUInt32();
        baseProcEntry.spellFamilyMask[1] = fields[4].GetUInt32();
        baseProcEntry.spellFamilyMask[2] = fields[5].GetUInt32();
        baseProcEntry.typeMask        = fields[6].GetUInt32();
        baseProcEntry.spellTypeMask   = fields[7].GetUInt32();
        baseProcEntry.spellPhaseMask  = fields[8].GetUInt32();
        baseProcEntry.hitMask         = fields[9].GetUInt32();
        baseProcEntry.attributesMask  = fields[10].GetUInt32();
        baseProcEntry.ratePerMinute   = fields[11].GetFloat();
        baseProcEntry.chance          = fields[12].GetFloat();
        float cooldown                = fields[13].GetFloat();
        baseProcEntry.cooldown        = uint32(cooldown);
        baseProcEntry.charges         = fields[14].GetUInt32();

        while (spellInfo)
        {
            if (mSpellProcMap.find(spellInfo->Id) != mSpellProcMap.end())
            {
                TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_proc` has duplicate entry in the table", spellInfo->Id);
                break;
            }
            SpellProcEntry procEntry = SpellProcEntry(baseProcEntry);

            // take defaults from dbcs
            if (!procEntry.typeMask)
                procEntry.typeMask = spellInfo->ProcFlags;
            if (!procEntry.charges)
                procEntry.charges = spellInfo->ProcCharges;
            if (!procEntry.chance && !procEntry.ratePerMinute)
                procEntry.chance = float(spellInfo->ProcChance);

            // validate data
            if (procEntry.schoolMask & ~SPELL_SCHOOL_MASK_ALL)
                TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u has wrong `schoolMask` set: %u", spellInfo->Id, procEntry.schoolMask);
            if (procEntry.spellFamilyName && (procEntry.spellFamilyName < 3 || procEntry.spellFamilyName > 17 || procEntry.spellFamilyName == 14 || procEntry.spellFamilyName == 16))
                TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u has wrong `spellFamilyName` set: %u", spellInfo->Id, procEntry.spellFamilyName);
            if (procEntry.chance < 0)
            {
                TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u has negative value in `chance` field", spellInfo->Id);
                procEntry.chance = 0;
            }
            if (procEntry.ratePerMinute < 0)
            {
                TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u has negative value in `ratePerMinute` field", spellInfo->Id);
                procEntry.ratePerMinute = 0;
            }
            if (cooldown < 0)
            {
                TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u has negative value in `cooldown` field", spellInfo->Id);
                procEntry.cooldown = 0;
            }
            if (procEntry.chance == 0 && procEntry.ratePerMinute == 0)
                TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u doesn't have `chance` and `ratePerMinute` values defined, proc will not be triggered", spellInfo->Id);
            if (procEntry.charges > 99)
            {
                TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u has too big value in `charges` field", spellInfo->Id);
                procEntry.charges = 99;
            }
            if (!procEntry.typeMask)
                TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u doesn't have `typeMask` value defined, proc will not be triggered", spellInfo->Id);
            if (procEntry.spellTypeMask & ~PROC_SPELL_TYPE_MASK_ALL)
                TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u has wrong `spellTypeMask` set: %u", spellInfo->Id, procEntry.spellTypeMask);
            if (procEntry.spellTypeMask && !(procEntry.typeMask & (SPELL_PROC_FLAG_MASK | PERIODIC_PROC_FLAG_MASK)))
                TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u has `spellTypeMask` value defined, but it won't be used for defined `typeMask` value", spellInfo->Id);
            if (!procEntry.spellPhaseMask && procEntry.typeMask & REQ_SPELL_PHASE_PROC_FLAG_MASK)
                TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u doesn't have `spellPhaseMask` value defined, but it's required for defined `typeMask` value, proc will not be triggered", spellInfo->Id);
            if (procEntry.spellPhaseMask & ~PROC_SPELL_PHASE_MASK_ALL)
                TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u has wrong `spellPhaseMask` set: %u", spellInfo->Id, procEntry.spellPhaseMask);
            if (procEntry.spellPhaseMask && !(procEntry.typeMask & REQ_SPELL_PHASE_PROC_FLAG_MASK))
                TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u has `spellPhaseMask` value defined, but it won't be used for defined `typeMask` value", spellInfo->Id);
            if (procEntry.hitMask & ~PROC_HIT_MASK_ALL)
                TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u has wrong `hitMask` set: %u", spellInfo->Id, procEntry.hitMask);
            if (procEntry.hitMask && !(procEntry.typeMask & TAKEN_HIT_PROC_FLAG_MASK || (procEntry.typeMask & DONE_HIT_PROC_FLAG_MASK && (!procEntry.spellPhaseMask || procEntry.spellPhaseMask & (PROC_SPELL_PHASE_HIT | PROC_SPELL_PHASE_FINISH)))))
                TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u has `hitMask` value defined, but it won't be used for defined `typeMask` and `spellPhaseMask` values", spellInfo->Id);

            mSpellProcMap[spellInfo->Id] = procEntry;

            if (allRanks)
                spellInfo = spellInfo->GetNextRankSpell();
            else
                break;
        }
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u spell proc conditions and data in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellBonusess()
{
    uint32 oldMSTime = getMSTime();

    mSpellBonusMap.clear();                             // need for reload case

    //                                                0      1             2          3         4
    QueryResult result = WorldDatabase.Query("SELECT entry, direct_bonus, dot_bonus, ap_bonus, ap_dot_bonus FROM spell_bonus_data");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell bonus data. DB table `spell_bonus_data` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].GetUInt32();

        SpellInfo const* spell = GetSpellInfo(entry);
        if (!spell)
        {
            TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_bonus_data` does not exist", entry);
            continue;
        }

        SpellBonusEntry& sbe = mSpellBonusMap[entry];
        sbe.direct_damage = fields[1].GetFloat();
        sbe.dot_damage    = fields[2].GetFloat();
        sbe.ap_bonus      = fields[3].GetFloat();
        sbe.ap_dot_bonus   = fields[4].GetFloat();

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u extra spell bonus data in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellThreats()
{
    uint32 oldMSTime = getMSTime();

    mSpellThreatMap.clear();                                // need for reload case

    //                                                0      1        2       3
    QueryResult result = WorldDatabase.Query("SELECT entry, flatMod, pctMod, apPctMod FROM spell_threat");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 aggro generating spells. DB table `spell_threat` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        if (!GetSpellInfo(entry))
        {
            TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_threat` does not exist", entry);
            continue;
        }

        SpellThreatEntry ste;
        ste.flatMod  = fields[1].GetInt32();
        ste.pctMod   = fields[2].GetFloat();
        ste.apPctMod = fields[3].GetFloat();

        mSpellThreatMap[entry] = ste;
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u SpellThreatEntries in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSkillLineAbilityMap()
{
    uint32 oldMSTime = getMSTime();

    mSkillLineAbilityMap.clear();

    uint32 count = 0;

    for (uint32 i = 0; i < sSkillLineAbilityStore.GetNumRows(); ++i)
    {
        SkillLineAbilityEntry const* SkillInfo = sSkillLineAbilityStore.LookupEntry(i);
        if (!SkillInfo)
            continue;

        mSkillLineAbilityMap.insert(SkillLineAbilityMap::value_type(SkillInfo->spellId, SkillInfo));
        ++count;
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u SkillLineAbility MultiMap Data in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellPetAuras()
{
    uint32 oldMSTime = getMSTime();

    mSpellPetAuraMap.clear();                                  // need for reload case

    //                                                  0       1       2    3
    QueryResult result = WorldDatabase.Query("SELECT spell, effectId, pet, aura FROM spell_pet_auras");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell pet auras. DB table `spell_pet_auras` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 spell = fields[0].GetUInt32();
        uint8 eff = fields[1].GetUInt8();
        uint32 pet = fields[2].GetUInt32();
        uint32 aura = fields[3].GetUInt32();

        SpellPetAuraMap::iterator itr = mSpellPetAuraMap.find((spell<<8) + eff);
        if (itr != mSpellPetAuraMap.end())
            itr->second.AddAura(pet, aura);
        else
        {
            SpellInfo const* spellInfo = GetSpellInfo(spell);
            if (!spellInfo)
            {
                TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_pet_auras` does not exist", spell);
                continue;
            }
            if (spellInfo->Effects[eff].Effect != SPELL_EFFECT_DUMMY &&
               (spellInfo->Effects[eff].Effect != SPELL_EFFECT_APPLY_AURA ||
                spellInfo->Effects[eff].ApplyAuraName != SPELL_AURA_DUMMY))
            {
                TC_LOG_ERROR("spells", "Spell %u listed in `spell_pet_auras` does not have dummy aura or dummy effect", spell);
                continue;
            }

            SpellInfo const* spellInfo2 = GetSpellInfo(aura);
            if (!spellInfo2)
            {
                TC_LOG_ERROR("sql.sql", "Aura %u listed in `spell_pet_auras` does not exist", aura);
                continue;
            }

            PetAura pa(pet, aura, spellInfo->Effects[eff].TargetA.GetTarget() == TARGET_UNIT_PET, spellInfo->Effects[eff].CalcValue());
            mSpellPetAuraMap[(spell<<8) + eff] = pa;
        }

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u spell pet auras in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

// Fill custom data about enchancments
void SpellMgr::LoadEnchantCustomAttr()
{
    uint32 oldMSTime = getMSTime();

    uint32 size = sSpellItemEnchantmentStore.GetNumRows();
    mEnchantCustomAttr.resize(size);

    for (uint32 i = 0; i < size; ++i)
       mEnchantCustomAttr[i] = 0;

    uint32 count = 0;
    for (uint32 i = 0; i < GetSpellInfoStoreSize(); ++i)
    {
        SpellInfo const* spellInfo = GetSpellInfo(i);
        if (!spellInfo)
            continue;

        // TODO: find a better check
        if (!(spellInfo->AttributesEx2 & SPELL_ATTR2_PRESERVE_ENCHANT_IN_ARENA) && !(spellInfo->Attributes & SPELL_ATTR0_NOT_SHAPESHIFT))
            continue;

        for (uint32 j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            if (spellInfo->Effects[j].Effect == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY)
            {
                uint32 enchId = spellInfo->Effects[j].MiscValue;
                SpellItemEnchantmentEntry const* ench = sSpellItemEnchantmentStore.LookupEntry(enchId);
                if (!ench)
                    continue;
                mEnchantCustomAttr[enchId] = true;
                ++count;
                break;
            }
        }
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u custom enchant attributes in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellEnchantProcData()
{
    uint32 oldMSTime = getMSTime();

    mSpellEnchantProcEventMap.clear();                             // need for reload case

    //                                                  0         1           2         3
    QueryResult result = WorldDatabase.Query("SELECT entry, customChance, PPMChance, procEx FROM spell_enchant_proc_data");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell enchant proc event conditions. DB table `spell_enchant_proc_data` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 enchantId = fields[0].GetUInt32();

        SpellItemEnchantmentEntry const* ench = sSpellItemEnchantmentStore.LookupEntry(enchantId);
        if (!ench)
        {
            TC_LOG_ERROR("sql.sql", "Enchancment %u listed in `spell_enchant_proc_data` does not exist", enchantId);
            continue;
        }

        SpellEnchantProcEntry spe;

        spe.customChance = fields[1].GetUInt32();
        spe.PPMChance = fields[2].GetFloat();
        spe.procEx = fields[3].GetUInt32();

        mSpellEnchantProcEventMap[enchantId] = spe;

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u enchant proc data definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellLinked()
{
    uint32 oldMSTime = getMSTime();

    mSpellLinkedMap.clear();    // need for reload case

    //                                                0              1             2
    QueryResult result = WorldDatabase.Query("SELECT spell_trigger, spell_effect, type FROM spell_linked_spell");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 linked spells. DB table `spell_linked_spell` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        int32 trigger = fields[0].GetInt32();
        int32 effect = fields[1].GetInt32();
        int32 type = fields[2].GetUInt8();

        SpellInfo const* spellInfo = GetSpellInfo(abs(trigger));
        if (!spellInfo)
        {
            TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_linked_spell` does not exist", abs(trigger));
            continue;
        }
        spellInfo = GetSpellInfo(abs(effect));
        if (!spellInfo)
        {
            TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_linked_spell` does not exist", abs(effect));
            continue;
        }

        if (type) //we will find a better way when more types are needed
        {
            if (trigger > 0)
                trigger += SPELL_LINKED_MAX_SPELLS * type;
            else
                trigger -= SPELL_LINKED_MAX_SPELLS * type;
        }
        mSpellLinkedMap[trigger].push_back(effect);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u linked spells in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadPetLevelupSpellMap()
{
    uint32 oldMSTime = getMSTime();

    mPetLevelupSpellMap.clear();                                   // need for reload case

    uint32 count = 0;
    uint32 family_count = 0;

    for (uint32 i = 0; i < sCreatureFamilyStore.GetNumRows(); ++i)
    {
        CreatureFamilyEntry const* creatureFamily = sCreatureFamilyStore.LookupEntry(i);
        if (!creatureFamily)                                     // not exist
            continue;

        for (uint8 j = 0; j < 2; ++j)
        {
            if (!creatureFamily->skillLine[j])
                continue;

            for (uint32 k = 0; k < sSkillLineAbilityStore.GetNumRows(); ++k)
            {
                SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(k);
                if (!skillLine)
                    continue;

                //if (skillLine->skillId != creatureFamily->skillLine[0] &&
                //    (!creatureFamily->skillLine[1] || skillLine->skillId != creatureFamily->skillLine[1]))
                //    continue;

                if (skillLine->skillId != creatureFamily->skillLine[j])
                    continue;

                if (skillLine->learnOnGetSkill != ABILITY_LEARNED_ON_GET_RACE_OR_CLASS_SKILL)
                    continue;

                SpellInfo const* spell = GetSpellInfo(skillLine->spellId);
                if (!spell) // not exist or triggered or talent
                    continue;

                if (!spell->SpellLevel)
                    continue;

                PetLevelupSpellSet& spellSet = mPetLevelupSpellMap[creatureFamily->ID];
                if (spellSet.empty())
                    ++family_count;

                spellSet.insert(PetLevelupSpellSet::value_type(spell->SpellLevel, spell->Id));
                ++count;
            }
        }
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u pet levelup and default spells for %u families in %u ms", count, family_count, GetMSTimeDiffToNow(oldMSTime));
}

bool LoadPetDefaultSpells_helper(CreatureTemplate const* cInfo, PetDefaultSpellsEntry& petDefSpells)
{
    // skip empty list;
    bool have_spell = false;
    for (uint8 j = 0; j < MAX_CREATURE_SPELL_DATA_SLOT; ++j)
    {
        if (petDefSpells.spellid[j])
        {
            have_spell = true;
            break;
        }
    }
    if (!have_spell)
        return false;

    // remove duplicates with levelupSpells if any
    if (PetLevelupSpellSet const* levelupSpells = cInfo->family ? sSpellMgr->GetPetLevelupSpellList(cInfo->family) : NULL)
    {
        for (uint8 j = 0; j < MAX_CREATURE_SPELL_DATA_SLOT; ++j)
        {
            if (!petDefSpells.spellid[j])
                continue;

            for (PetLevelupSpellSet::const_iterator itr = levelupSpells->begin(); itr != levelupSpells->end(); ++itr)
            {
                if (itr->second == petDefSpells.spellid[j])
                {
                    petDefSpells.spellid[j] = 0;
                    break;
                }
            }
        }
    }

    // skip empty list;
    have_spell = false;
    for (uint8 j = 0; j < MAX_CREATURE_SPELL_DATA_SLOT; ++j)
    {
        if (petDefSpells.spellid[j])
        {
            have_spell = true;
            break;
        }
    }

    return have_spell;
}

void SpellMgr::LoadPetDefaultSpells()
{
    uint32 oldMSTime = getMSTime();

    mPetDefaultSpellsMap.clear();

    uint32 countCreature = 0;
    uint32 countData = 0;

    CreatureTemplateContainer const* ctc = sObjectMgr->GetCreatureTemplates();
    for (CreatureTemplateContainer::const_iterator itr = ctc->begin(); itr != ctc->end(); ++itr)
    {

        if (!itr->second.PetSpellDataId)
            continue;

        // for creature with PetSpellDataId get default pet spells from dbc
        CreatureSpellDataEntry const* spellDataEntry = sCreatureSpellDataStore.LookupEntry(itr->second.PetSpellDataId);
        if (!spellDataEntry)
            continue;

        int32 petSpellsId = -int32(itr->second.PetSpellDataId);
        PetDefaultSpellsEntry petDefSpells;
        for (uint8 j = 0; j < MAX_CREATURE_SPELL_DATA_SLOT; ++j)
            petDefSpells.spellid[j] = spellDataEntry->spellId[j];

        if (LoadPetDefaultSpells_helper(&itr->second, petDefSpells))
        {
            mPetDefaultSpellsMap[petSpellsId] = petDefSpells;
            ++countData;
        }
    }

    TC_LOG_INFO("server.loading", ">> Loaded addition spells for %u pet spell data entries in %u ms", countData, GetMSTimeDiffToNow(oldMSTime));

    TC_LOG_INFO("server.loading", "Loading summonable creature templates...");
    oldMSTime = getMSTime();

    // different summon spells
    for (uint32 i = 0; i < GetSpellInfoStoreSize(); ++i)
    {
        SpellInfo const* spellEntry = GetSpellInfo(i);
        if (!spellEntry)
            continue;

        for (uint8 k = 0; k < MAX_SPELL_EFFECTS; ++k)
        {
            if (spellEntry->Effects[k].Effect == SPELL_EFFECT_SUMMON || spellEntry->Effects[k].Effect == SPELL_EFFECT_SUMMON_PET)
            {
                uint32 creature_id = spellEntry->Effects[k].MiscValue;
                CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(creature_id);
                if (!cInfo)
                    continue;

                // already loaded
                if (cInfo->PetSpellDataId)
                    continue;

                // for creature without PetSpellDataId get default pet spells from creature_template
                int32 petSpellsId = cInfo->Entry;
                if (mPetDefaultSpellsMap.find(cInfo->Entry) != mPetDefaultSpellsMap.end())
                    continue;

                PetDefaultSpellsEntry petDefSpells;
                for (uint8 j = 0; j < MAX_CREATURE_SPELL_DATA_SLOT; ++j)
                    petDefSpells.spellid[j] = cInfo->spells[j];

                if (LoadPetDefaultSpells_helper(cInfo, petDefSpells))
                {
                    mPetDefaultSpellsMap[petSpellsId] = petDefSpells;
                    ++countCreature;
                }
            }
        }
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u summonable creature templates in %u ms", countCreature, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellAreas()
{
    uint32 oldMSTime = getMSTime();

    mSpellAreaMap.clear();                                  // need for reload case
    mSpellAreaForQuestMap.clear();
    mSpellAreaForQuestEndMap.clear();
    mSpellAreaForAuraMap.clear();

    //                                                  0     1         2              3               4                 5          6          7       8         9
    QueryResult result = WorldDatabase.Query("SELECT spell, area, quest_start, quest_start_status, quest_end_status, quest_end, aura_spell, racemask, gender, autocast FROM spell_area");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell area requirements. DB table `spell_area` is empty.");

        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 spell = fields[0].GetUInt32();
        SpellArea spellArea;
        spellArea.spellId             = spell;
        spellArea.areaId              = fields[1].GetUInt32();
        spellArea.questStart          = fields[2].GetUInt32();
        spellArea.questStartStatus    = fields[3].GetUInt32();
        spellArea.questEndStatus      = fields[4].GetUInt32();
        spellArea.questEnd            = fields[5].GetUInt32();
        spellArea.auraSpell           = fields[6].GetInt32();
        spellArea.raceMask            = fields[7].GetUInt32();
        spellArea.gender              = Gender(fields[8].GetUInt8());
        spellArea.autocast            = fields[9].GetBool();

        if (SpellInfo const* spellInfo = GetSpellInfo(spell))
        {
            if (spellArea.autocast)
                const_cast<SpellInfo*>(spellInfo)->Attributes |= SPELL_ATTR0_CANT_CANCEL;
        }
        else
        {
            TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_area` does not exist", spell);
            continue;
        }

        {
            bool ok = true;
            SpellAreaMapBounds sa_bounds = GetSpellAreaMapBounds(spellArea.spellId);
            for (SpellAreaMap::const_iterator itr = sa_bounds.first; itr != sa_bounds.second; ++itr)
            {
                if (spellArea.spellId != itr->second.spellId)
                    continue;
                if (spellArea.areaId != itr->second.areaId)
                    continue;
                if (spellArea.questStart != itr->second.questStart)
                    continue;
                if (spellArea.auraSpell != itr->second.auraSpell)
                    continue;
                if ((spellArea.raceMask & itr->second.raceMask) == 0)
                    continue;
                if (spellArea.gender != itr->second.gender)
                    continue;

                // duplicate by requirements
                ok = false;
                break;
            }

            if (!ok)
            {
                TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_area` already listed with similar requirements.", spell);
                continue;
            }
        }

        if (spellArea.areaId && !GetAreaEntryByAreaID(spellArea.areaId))
        {
            TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_area` have wrong area (%u) requirement", spell, spellArea.areaId);
            continue;
        }

        if (spellArea.questStart && !sObjectMgr->GetQuestTemplate(spellArea.questStart))
        {
            TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_area` have wrong start quest (%u) requirement", spell, spellArea.questStart);
            continue;
        }

        if (spellArea.questEnd)
        {
            if (!sObjectMgr->GetQuestTemplate(spellArea.questEnd))
            {
                TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_area` have wrong end quest (%u) requirement", spell, spellArea.questEnd);
                continue;
            }
        }

        if (spellArea.auraSpell)
        {
            SpellInfo const* spellInfo = GetSpellInfo(abs(spellArea.auraSpell));
            if (!spellInfo)
            {
                TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_area` have wrong aura spell (%u) requirement", spell, abs(spellArea.auraSpell));
                continue;
            }

            if (uint32(abs(spellArea.auraSpell)) == spellArea.spellId)
            {
                TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_area` have aura spell (%u) requirement for itself", spell, abs(spellArea.auraSpell));
                continue;
            }

            // not allow autocast chains by auraSpell field (but allow use as alternative if not present)
            if (spellArea.autocast && spellArea.auraSpell > 0)
            {
                bool chain = false;
                SpellAreaForAuraMapBounds saBound = GetSpellAreaForAuraMapBounds(spellArea.spellId);
                for (SpellAreaForAuraMap::const_iterator itr = saBound.first; itr != saBound.second; ++itr)
                {
                    if (itr->second->autocast && itr->second->auraSpell > 0)
                    {
                        chain = true;
                        break;
                    }
                }

                if (chain)
                {
                    TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_area` have aura spell (%u) requirement that itself autocast from aura", spell, spellArea.auraSpell);
                    continue;
                }

                SpellAreaMapBounds saBound2 = GetSpellAreaMapBounds(spellArea.auraSpell);
                for (SpellAreaMap::const_iterator itr2 = saBound2.first; itr2 != saBound2.second; ++itr2)
                {
                    if (itr2->second.autocast && itr2->second.auraSpell > 0)
                    {
                        chain = true;
                        break;
                    }
                }

                if (chain)
                {
                    TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_area` have aura spell (%u) requirement that itself autocast from aura", spell, spellArea.auraSpell);
                    continue;
                }
            }
        }

        if (spellArea.raceMask && (spellArea.raceMask & RACEMASK_ALL_PLAYABLE) == 0)
        {
            TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_area` have wrong race mask (%u) requirement", spell, spellArea.raceMask);
            continue;
        }

        if (spellArea.gender != GENDER_NONE && spellArea.gender != GENDER_FEMALE && spellArea.gender != GENDER_MALE)
        {
            TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_area` have wrong gender (%u) requirement", spell, spellArea.gender);
            continue;
        }

        SpellArea const* sa = &mSpellAreaMap.insert(SpellAreaMap::value_type(spell, spellArea))->second;

        // for search by current zone/subzone at zone/subzone change
        if (spellArea.areaId)
            mSpellAreaForAreaMap.insert(SpellAreaForAreaMap::value_type(spellArea.areaId, sa));

        // for search at quest start/reward
        if (spellArea.questStart)
            mSpellAreaForQuestMap.insert(SpellAreaForQuestMap::value_type(spellArea.questStart, sa));

        // for search at quest start/reward
        if (spellArea.questEnd)
            mSpellAreaForQuestEndMap.insert(SpellAreaForQuestMap::value_type(spellArea.questEnd, sa));

        // for search at aura apply
        if (spellArea.auraSpell)
            mSpellAreaForAuraMap.insert(SpellAreaForAuraMap::value_type(abs(spellArea.auraSpell), sa));

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u spell area requirements in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

// Temporary structure to hold spell effect entries for faster loading
struct SpellEffectArray
{
    SpellEffectArray()
    {
        effects[0] = NULL;
        effects[1] = NULL;
        effects[2] = NULL;
    }

    SpellEffectEntry const* effects[MAX_SPELL_EFFECTS];
};

void SpellMgr::LoadSpellInfoStore()
{
    uint32 oldMSTime = getMSTime();

    UnloadSpellInfoStore();
    mSpellInfoMap.resize(sSpellStore.GetNumRows(), NULL);

    std::map<uint32, SpellEffectArray> effectsBySpell;

    for (uint32 i = 0; i < sSpellEffectStore.GetNumRows(); ++i)
    {
        SpellEffectEntry const* effect = sSpellEffectStore.LookupEntry(i);
        if (!effect)
            continue;

        effectsBySpell[effect->EffectSpellId].effects[effect->EffectIndex] = effect;
    }

    for (uint32 i = 0; i < sSpellStore.GetNumRows(); ++i)
        if (SpellEntry const* spellEntry = sSpellStore.LookupEntry(i))
            mSpellInfoMap[i] = new SpellInfo(spellEntry, effectsBySpell[i].effects);

    TC_LOG_INFO("server.loading", ">> Loaded spell info store in %u ms", GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::UnloadSpellInfoStore()
{
    for (uint32 i = 0; i < mSpellInfoMap.size(); ++i)
        delete mSpellInfoMap[i];

    mSpellInfoMap.clear();
}

void SpellMgr::UnloadSpellInfoImplicitTargetConditionLists()
{
    for (uint32 i = 0; i < mSpellInfoMap.size(); ++i)
        if (mSpellInfoMap[i])
            mSpellInfoMap[i]->_UnloadImplicitTargetConditionLists();
}

void SpellMgr::LoadSpellCustomAttr()
{
    uint32 oldMSTime = getMSTime();

    SpellInfo* spellInfo = NULL;
    for (uint32 i = 0; i < GetSpellInfoStoreSize(); ++i)
    {
        spellInfo = mSpellInfoMap[i];
        if (!spellInfo)
            continue;

        for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            switch (spellInfo->Effects[j].ApplyAuraName)
            {
                case SPELL_AURA_MOD_POSSESS:
                case SPELL_AURA_MOD_CONFUSE:
                case SPELL_AURA_MOD_CHARM:
                case SPELL_AURA_AOE_CHARM:
                case SPELL_AURA_MOD_FEAR:
                case SPELL_AURA_MOD_STUN:
                case SPELL_AURA_MOD_ROOT:
                    spellInfo->AttributesCu |= SPELL_ATTR0_CU_AURA_CC;
                    break;
                case SPELL_AURA_PERIODIC_HEAL:
                case SPELL_AURA_PERIODIC_DAMAGE:
                case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
                case SPELL_AURA_PERIODIC_LEECH:
                case SPELL_AURA_PERIODIC_MANA_LEECH:
                case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
                case SPELL_AURA_PERIODIC_ENERGIZE:
                case SPELL_AURA_OBS_MOD_HEALTH:
                case SPELL_AURA_OBS_MOD_POWER:
                case SPELL_AURA_POWER_BURN:
                    spellInfo->AttributesCu |= SPELL_ATTR0_CU_NO_INITIAL_THREAT;
                    break;
            }

            switch (spellInfo->Effects[j].Effect)
            {
                case SPELL_EFFECT_SCHOOL_DAMAGE:
                case SPELL_EFFECT_WEAPON_DAMAGE:
                case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
                case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
                case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                case SPELL_EFFECT_HEAL:
                    spellInfo->AttributesCu |= SPELL_ATTR0_CU_DIRECT_DAMAGE;
                    break;
                case SPELL_EFFECT_POWER_DRAIN:
                case SPELL_EFFECT_POWER_BURN:
                case SPELL_EFFECT_HEAL_MAX_HEALTH:
                case SPELL_EFFECT_HEALTH_LEECH:
                case SPELL_EFFECT_HEAL_PCT:
                case SPELL_EFFECT_ENERGIZE_PCT:
                case SPELL_EFFECT_ENERGIZE:
                case SPELL_EFFECT_HEAL_MECHANICAL:
                    spellInfo->AttributesCu |= SPELL_ATTR0_CU_NO_INITIAL_THREAT;
                    break;
                case SPELL_EFFECT_CHARGE:
                case SPELL_EFFECT_CHARGE_DEST:
                case SPELL_EFFECT_JUMP:
                case SPELL_EFFECT_JUMP_DEST:
                case SPELL_EFFECT_LEAP_BACK:
                    spellInfo->AttributesCu |= SPELL_ATTR0_CU_CHARGE;
                    break;
                case SPELL_EFFECT_PICKPOCKET:
                    spellInfo->AttributesCu |= SPELL_ATTR0_CU_PICKPOCKET;
                    break;
                case SPELL_EFFECT_ENCHANT_ITEM:
                case SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY:
                case SPELL_EFFECT_ENCHANT_ITEM_PRISMATIC:
                case SPELL_EFFECT_ENCHANT_HELD_ITEM:
                {
                    // only enchanting profession enchantments procs can stack
                    if (IsPartOfSkillLine(SKILL_ENCHANTING, i))
                    {
                        uint32 enchantId = spellInfo->Effects[j].MiscValue;
                        SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(enchantId);
                        for (uint8 s = 0; s < MAX_ITEM_ENCHANTMENT_EFFECTS; ++s)
                        {
                            if (enchant->type[s] != ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL)
                                continue;

                            SpellInfo* procInfo = (SpellInfo*)GetSpellInfo(enchant->spellid[s]);
                            if (!procInfo)
                                continue;

                            // if proced directly from enchantment, not via proc aura
                            // NOTE: Enchant Weapon - Blade Ward also has proc aura spell and is proced directly
                            // however its not expected to stack so this check is good
                            if (procInfo->HasAura(SPELL_AURA_PROC_TRIGGER_SPELL))
                                continue;

                            procInfo->AttributesCu |= SPELL_ATTR0_CU_ENCHANT_PROC;
                        }
                    }
                    break;
                }
            }

            switch (spellInfo->Effects[j].Mechanic)
            {
                case MECHANIC_ROOT:
                case MECHANIC_INTERRUPT:
                case MECHANIC_SILENCE:
                case MECHANIC_HORROR:
                    spellInfo->AttributesCu |= SPELL_ATTR0_CU_BINARY;
                    break;
                default:
                    break;
            }
        }

        switch (spellInfo->Mechanic)
        {
            case MECHANIC_FEAR:
            case MECHANIC_CHARM:
            case MECHANIC_FREEZE:
            case MECHANIC_BANISH:
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_BINARY;
                break;
        }

        if (!spellInfo->_IsPositiveEffect(EFFECT_0, false))
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE_EFF0;

        if (!spellInfo->_IsPositiveEffect(EFFECT_1, false))
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE_EFF1;

        if (!spellInfo->_IsPositiveEffect(EFFECT_2, false))
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE_EFF2;

        if (spellInfo->SpellVisual[0] == 3879)
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_CONE_BACK;

        switch (spellInfo->Id)
        {
            case 1776: // Gouge
            case 1777:
            case 8629:
            case 11285:
            case 11286:
            case 12540:
            case 13579:
            case 24698:
            case 28456:
            case 29425:
            case 34940:
            case 36862:
            case 38764:
            case 38863:
            case 52743: // Head Smack
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_REQ_TARGET_FACING_CASTER;
                break;
            case 53: // Backstab
            case 2589:
            case 2590:
            case 2591:
            case 8721:
            case 11279:
            case 11280:
            case 11281:
            case 25300:
            case 26863:
            case 48656:
            case 48657:
            case 703: // Garrote
            case 8631:
            case 8632:
            case 8633:
            case 11289:
            case 11290:
            case 26839:
            case 26884:
            case 48675:
            case 48676:
            case 5221: // Shred
            case 6800:
            case 8992:
            case 9829:
            case 9830:
            case 27001:
            case 27002:
            case 48571:
            case 48572:
            case 8676: // Ambush
            case 8724:
            case 8725:
            case 11267:
            case 11268:
            case 11269:
            case 27441:
            case 48689:
            case 48690:
            case 48691:
            case 6785: // Ravage
            case 6787:
            case 9866:
            case 9867:
            case 27005:
            case 48578:
            case 48579:
            case 21987: // Lash of Pain
            case 23959: // Test Stab R50
            case 24825: // Test Backstab
            case 58563: // Assassinate Restless Lookout
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_REQ_CASTER_BEHIND_TARGET;
                break;
            case 26029: // Dark Glare
            case 37433: // Spout
            case 43140: // Flame Breath
            case 43215: // Flame Breath
            case 70461: // Coldflame Trap
            case 72133: // Pain and Suffering
            case 73788: // Pain and Suffering
            case 73789: // Pain and Suffering
            case 73790: // Pain and Suffering
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_CONE_LINE;
                break;
            case 24340: // Meteor
            case 26558: // Meteor
            case 28884: // Meteor
            case 36837: // Meteor
            case 38903: // Meteor
            case 41276: // Meteor
            case 57467: // Meteor
            case 26789: // Shard of the Fallen Star
            case 31436: // Malevolent Cleave
            case 35181: // Dive Bomb
            case 40810: // Saber Lash
            case 43267: // Saber Lash
            case 43268: // Saber Lash
            case 45150: // Meteor Slash
            case 64688: // Sonic Screech
            case 72373: // Shared Suffering
            case 71904: // Chaos Bane
            case 70492: // Ooze Eruption
            case 72505: // Ooze Eruption
            case 72624: // Ooze Eruption
            case 72625: // Ooze Eruption
            case 76717: // Meteor
            case 93668: // Meteor
            case 84032: // Meteor
            case 90023: // Meteor
            case 88942: // Meteor Slash
            case 95172: // Meteor Slash
            case 89348: // Demon Repellent Ray
            case 95178: // Demon Repellent Ray
            case 92864: // twilight meteorit
            case 92865: // twilight meteorit
            case 86367: // Sleet Storm
            case 93135: // Sleet Storm
            case 93136: // Sleet Storm
            case 93137: // Sleet Storm
            case 100431: // Flaming Cleave
            case 103414: //Morchok Stomp
            case 108571:
            case 103527: //Zonozz Void Diffusion
            case 104605:
            case 108345:
            case 108346:
                // ONLY SPELLS WITH SPELLFAMILY_GENERIC and EFFECT_SCHOOL_DAMAGE
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_SHARE_DAMAGE;
                break;
            case 18500: // Wing Buffet
            case 33086: // Wild Bite
            case 49749: // Piercing Blow
            case 52890: // Penetrating Strike
            case 53454: // Impale
            case 59446: // Impale
            case 62383: // Shatter
            case 64777: // Machine Gun
            case 65239: // Machine Gun
            case 65919: // Impale
            case 67858: // Impale
            case 67859: // Impale
            case 67860: // Impale
            case 69293: // Wing Buffet
            case 74439: // Machine Gun
            case 63278: // Mark of the Faceless (General Vezax)
            case 62544: // Thrust (Argent Tournament)
            case 64588: // Thrust (Argent Tournament)
            case 66479: // Thrust (Argent Tournament)
            case 68505: // Thrust (Argent Tournament)
            case 62709: // Counterattack! (Argent Tournament)
            case 62626: // Break-Shield (Argent Tournament, Player)
            case 64590: // Break-Shield (Argent Tournament, Player)
            case 64342: // Break-Shield (Argent Tournament, NPC)
            case 64686: // Break-Shield (Argent Tournament, NPC)
            case 65147: // Break-Shield (Argent Tournament, NPC)
            case 68504: // Break-Shield (Argent Tournament, NPC)
            case 62874: // Charge (Argent Tournament, Player)
            case 68498: // Charge (Argent Tournament, Player)
            case 64591: // Charge (Argent Tournament, Player)
            case 63003: // Charge (Argent Tournament, NPC)
            case 63010: // Charge (Argent Tournament, NPC)
            case 68321: // Charge (Argent Tournament, NPC)
            case 72255: // Mark of the Fallen Champion (Deathbringer Saurfang)
            case 72444: // Mark of the Fallen Champion (Deathbringer Saurfang)
            case 72445: // Mark of the Fallen Champion (Deathbringer Saurfang)
            case 72446: // Mark of the Fallen Champion (Deathbringer Saurfang)
            case 93424: // Asphyxiate
            case 22482: // blade flurry extra attack
            case 12723: // Sweeping Strikes extra attack
            case 42384: // Brutal Strike
            case 28375: // Decimate - Gluth (Naxxramas)
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
                break;
                // Black Window kiss
            case 99506:
                spellInfo->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_DUMMY;
                break;
            case 99476:
                spellInfo->Effects[EFFECT_0].TriggerSpell = 99506;
                break;
            case 64422: // Sonic Screech (Auriaya)
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_SHARE_DAMAGE;
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
                break;
            case 72293: // Mark of the Fallen Champion (Deathbringer Saurfang)
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE_EFF0;
                break;
            case 88667: // Holy Word: Sanctuary 3yd Dummy
                spellInfo->SpellFamilyName = SPELLFAMILY_PRIEST;
                spellInfo->Effects[0].RadiusEntry = sSpellRadiusStore.LookupEntry(15);
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_18_SECONDS);
                break;
            case 88668: // Holy Word: Sanctuary 3yd Heal
                spellInfo->SpellFamilyName = SPELLFAMILY_PRIEST;
                spellInfo->Effects[0].RadiusEntry = sSpellRadiusStore.LookupEntry(15);
                break;
            case 88685: // Holy Word: Sanctuary 6yd Dummy
            case 88686: // Holy Word: Sanctuary 6yd Heal
                spellInfo->Effects[0].RadiusEntry = sSpellRadiusStore.LookupEntry(29);
				break;
				/*Druid*/
			case 48517: // Eclipse markers
			case 48518:
				spellInfo->AuraInterruptFlags = 0;
				spellInfo->Attributes |= SPELL_ATTR0_CANT_CANCEL;
				break;
			case 16870: // Clearcasting
				spellInfo->Effects[EFFECT_0].SpellClassMask[0] &= ~0x10000; // remove Ravage
				break;
			case 1953: // Blink(made it so the de-buff gets removed at 250ms instead of 1sec)
				spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(328); // 250ms
				break;
            case 84839: // Vengeance
            case 84840: // Vengeance
            case 93098: // Vengeance
            case 93099: // Vengeance
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_PROC_ON_FULL_ABSORB;
                break;
            case 93612: // Black Breath
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_6_YARDS);
                break;
            case 98474: // Flame Scythe
            case 100212: // Flame Scythe
            case 100213: // Flame Scythe
            case 100214:  // Flame Scythe
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_SHARE_DAMAGE;
                spellInfo->AttributesEx3 |= SPELL_ATTR3_ONLY_TARGET_PLAYERS;
                spellInfo->Mechanic = MECHANIC_DISTRACT;
                break;

            // Leaping Flames
            case 98535:
                spellInfo->Mechanic = MECHANIC_DISTRACT;
                break;
                // Deathwing Cutscenes
            case 104574: //deathwing spine movie 3
                spellInfo->Effects[EFFECT_0].Effect = SPELL_EFFECT_PLAY_MOVIE;
                spellInfo->Effects[EFFECT_0].MiscValue = 75;
                break;
            case 106085: //deathwing spine movie 2
                spellInfo->Effects[EFFECT_0].Effect = SPELL_EFFECT_PLAY_MOVIE;
                spellInfo->Effects[EFFECT_0].MiscValue = 74;
                break;
            case 106087: //deathwing spine movie 1
                spellInfo->Effects[EFFECT_0].Effect = SPELL_EFFECT_PLAY_MOVIE;
                spellInfo->Effects[EFFECT_0].MiscValue = 73;
                break;
                // Deep Corruption
            case 108220:
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE;
                break;
                // Safe
            case 103541:
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE;
                break;
                // Yoe'sahj The Unsleeping
            case 105420: // Color Combine 1
            case 105435: // Color Combine 2
            case 105436: // Color Combine 3
            case 105437: // Color Combine 4
            case 105439: // Color Combine 5
            case 105440: // Color Combine 6
                spellInfo->Effects[EFFECT_0].TargetB = TARGET_UNIT_DEST_AREA_ENTRY;
                break;
            case 105636: // Corrupted Minions aura
            case 109558:
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_SRC_CASTER;
                spellInfo->Effects[EFFECT_0].TargetB = TARGET_UNIT_SRC_AREA_ENTRY;
                break;
            case 105534: // Mana Void dummy
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_UNIT_CASTER;
                break;
                // Morchok
            case 103821: // Earthen Vortex
            case 110047:
            case 110046:
            case 110045:
                spellInfo->Effects[EFFECT_0].Effect = SPELL_EFFECT_APPLY_AURA;
                spellInfo->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_MOD_STUN;
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_60_YARDS);
                spellInfo->Effects[EFFECT_1].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_60_YARDS);
                spellInfo->Effects[EFFECT_2].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_60_YARDS);
                break;
            case 103178: // Earths Vengeance dmg
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(618);
                break;
            case 103534: // Danger
            case 103536: // Warning
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE;
                break;
            case 103687: // Crush Armor
                spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
                break;
            case 103785: // Black Blood of the Earth dmg
            case 108570:
            case 110288:
            case 110287:
                spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
                break;
            case 109033: // Stomp (heroic)
            case 109034:
                spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_SHARE_DAMAGE;
                break;
            case 102726: // Mark of Silence
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(21);
                break;
            case 103597: // Throw Knife
                spellInfo->MaxAffectedTargets = 1;
                break;
                // Warlord Zon'ozz
            case 103571: // Void of the Unmaking
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_30_YARDS);
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_DEST_CASTER_FRONT);
                spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo();
                break;
            case 103434: // Disrupting shadows 10n/10h
            case 104600:
                spellInfo->MaxAffectedTargets = 3;
                break;
            case 104599: // Disrupting shadows 25n/25h
            case 104601:
                spellInfo->MaxAffectedTargets = urand(7, 8);
                break;
                // Hagara the stormbinder
            case 107851: // Focused Assault
            case 110898:
            case 110899:
            case 110900:
                spellInfo->Effects[EFFECT_1].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_TARGET_ENEMY);
                spellInfo->Effects[EFFECT_1].TargetB = SpellImplicitTargetInfo();
                break;
            case 92202: // Icicle
                spellInfo->SpellVisual[0] = 13165;
                break;
            case 105367: // Lightning Conduit
            case 105369:
            case 105377:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_15_YARDS);
                spellInfo->Effects[EFFECT_1].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_15_YARDS);
                spellInfo->Effects[EFFECT_2].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_15_YARDS);
                break;
            case 108569:
            case 109201:
            case 109202:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_10_YARDS);
                break;
            case 92203:
                spellInfo->Effects[EFFECT_0].BasePoints = 100000;
                break;
                // Ultraxion
            case 106174: // Hour of Twilight
                spellInfo->AttributesEx3 |= SPELL_ATTR3_ONLY_TARGET_PLAYERS;
                break;
            case 106375: // Twilight Instability
            case 109182:
            case 109183:
            case 109184:
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_SHARE_DAMAGE;
                break;
            case 105900: // Essence of Dreams (Ultraxion)
                spellInfo->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_DUMMY;
                break;
            case 105996:
                spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo(TARGET_UNIT_DEST_AREA_ALLY);
                break;
            case 109075: // Fading Light
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 110080:
                spellInfo->MaxAffectedTargets = 3;
                break;
            case 110079:
                spellInfo->MaxAffectedTargets = 2;
                break;
            case 110078:
                spellInfo->MaxAffectedTargets = 6;
                break;
            case 106108: // Heroic Will
            case 106175:
            case 105554:
                // Prevent client crash on pressing ESC
                spellInfo->AttributesEx4 &= ~SPELL_ATTR4_TRIGGERED;
                break;
            case 99245:
                spellInfo->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_TRANSFORM;
                break;
                // Warmaster Blackhorn
            case 107518: // Detonate
                spellInfo->SpellVisual[0] = 22480;
                break;
            case 107588: // Twilight Onslaught
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_DEST_AREA_ENTRY);
                break;
            case 108038: // Harpoon
                spellInfo->RangeEntry = sSpellRangeStore.LookupEntry(36);
                break;
            case 107439: // Twilight Barrage
            case 109203:
            case 109204:
            case 109205:
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_SHARE_DAMAGE;
                break;
            case 108076: // Twilight Flames
            case 109222:
            case 109223:
            case 109224:
                spellInfo->AttributesEx3 |= SPELL_ATTR3_IGNORE_HIT_RESULT;
                break;
            case 107567: // Brutal Strike
            case 109209:
            case 109210:
            case 109211:
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
                break;
            case 107595:
            case 109013:
            case 109014:
            case 109015:
                spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo(TARGET_UNIT_CONE_ENEMY_24);
                break;
                // Spine of Deathwing
            case 105845:
                spellInfo->AttributesEx3 |= SPELL_ATTR3_IGNORE_HIT_RESULT;
                break;
            case 105479: // Searing Plasma
            case 109362:
            case 109363:
            case 109364:
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE;
                break;
            case 105219: // Burst
            case 109371:
            case 109372:
            case 109373:
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
                break;
                // Madness of Deathwing
            case 106663: // Carrying winds
            case 106668:
            case 106670:
            case 106672:
            case 106674:
            case 106676:
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(39);
                break;
            case 106466: // Dream
            case 109634:
            case 109635:
            case 109636:
                // Prevent client crash on pressing ESC
                spellInfo->AttributesEx4 &= ~SPELL_ATTR4_TRIGGERED;
                break;
            case 106444: // Impale
            case 109631:
            case 109632:
            case 109633:
            case 106791: // Shrapnel
            case 109597:
            case 109598:
            case 109599:
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
                break;
            default:
                break;
        }

        spellInfo->_InitializeExplicitTargetMask();
    }

    CreatureAI::FillAISpellInfo();

    TC_LOG_INFO("server.loading", ">> Loaded spell custom attributes in %u ms", GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellInfoCorrections()
{
    uint32 oldMSTime = getMSTime();

    SpellInfo* spellInfo = NULL;
    for (uint32 i = 0; i < mSpellInfoMap.size(); ++i)
    {
        spellInfo = mSpellInfoMap[i];
        if (!spellInfo)
            continue;

        for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            switch (spellInfo->Effects[j].Effect)
            {
                case SPELL_EFFECT_CHARGE:
                case SPELL_EFFECT_CHARGE_DEST:
                case SPELL_EFFECT_JUMP:
                case SPELL_EFFECT_JUMP_DEST:
                case SPELL_EFFECT_LEAP_BACK:
                    if (!spellInfo->Speed && !spellInfo->SpellFamilyName)
                        spellInfo->Speed = SPEED_CHARGE;
                    break;
            }
        }

        if (spellInfo->ActiveIconID == 2158)  // flight
            spellInfo->Attributes |= SPELL_ATTR0_PASSIVE;

        switch (spellInfo->Id)
        {
             case 69123: // Curse of the Worgen, hackfix
                spellInfo->Effects[EFFECT_0].MiscValue = 0;
                break;
             case 68591: // Fiery Boulder
                 spellInfo->MaxAffectedTargets = 1;
                 break;
            case 47753: // Divine Aegis
            case 77613:
                spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
                break;
            case 47930: // Grace
                spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
                spellInfo->RangeEntry = sSpellRangeStore.LookupEntry(13);
                break;
            case 6770: // Sap
                spellInfo->AttributesEx6 |= SPELL_ATTR6_CAN_TARGET_INVISIBLE;
                break;
            case 379: // Earth Shield Direct Heal Cast
                spellInfo->AttributesEx3 |= SPELL_ATTR3_NO_DONE_BONUS;
                break;
            case 61391: // Typhoon
                spellInfo->Speed = 0.0f; // This spell's summon should happen instantly
                break;
                // Owlkin Frenzy
            case 48391:
                spellInfo->Attributes |= SPELL_ATTR0_NOT_SHAPESHIFT;
                break;
            case 107818: // Summon Tentacle of the Old Ones
            case 109838: // should be useable in Arena and rated Bgs
            case 109840:
                spellInfo->AttributesEx4 &= ~SPELL_ATTR4_NOT_USABLE_IN_ARENA_OR_RATED_BG;
                break;
            case 46946: // Safeguard (Rank 1)
             case 46947: // Safeguard (Rank 2)
                 spellInfo->RangeEntry = sSpellRangeStore.LookupEntry(34); // Twenty-Five yards
                break;
            case 34490: // Silencing Shot
                spellInfo->Speed = 0.0f;
                spellInfo->AttributesEx3 |= SPELL_ATTR3_IGNORE_HIT_RESULT;
                break;
            case 6562: // Heroic Presence
            case 28878:
                spellInfo->Effects[EFFECT_0].Effect = SPELL_EFFECT_APPLY_AURA;
                spellInfo->Effects[EFFECT_1].Effect = SPELL_EFFECT_APPLY_AURA;
                break;
            case 80451:
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_DEST_CASTER;
                break;
            case 86150: // Guardioan of Ancient kings
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_UNIT_CASTER;
                break;
            case 65142: // Ebon Plague
                spellInfo->Effects[EFFECT_1].Effect = SPELL_EFFECT_APPLY_AURA;
                spellInfo->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN;
                spellInfo->Effects[EFFECT_1].MiscValue = 126;
                spellInfo->Effects[EFFECT_1].TriggerSpell = 0;
                break;
            case 49224: // Magic Suppression
            case 49611:
                spellInfo->ProcCharges = 0;
                break;
            case 42490: // Energized!
            case 42492: // Cast Energized
                spellInfo->AttributesEx |= SPELL_ATTR1_NO_THREAT;
                break;
            case 53096: // Quetz'lun's Judgment
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 42730:
                spellInfo->Effects[EFFECT_1].TriggerSpell = 42739;
                break;
            case 59735:
                spellInfo->Effects[EFFECT_1].TriggerSpell = 59736;
                break;
            case 52611: // Summon Skeletons
            case 52612: // Summon Skeletons
                spellInfo->Effects[EFFECT_0].MiscValueB = 64;
                break;
            case 40244: // Simon Game Visual
            case 40245: // Simon Game Visual
            case 40246: // Simon Game Visual
            case 40247: // Simon Game Visual
            case 42835: // Spout, remove damage effect, only anim is needed
                spellInfo->Effects[EFFECT_0].Effect = 0;
                break;
            case 30657: // Quake
                spellInfo->Effects[EFFECT_0].TriggerSpell = 30571;
                break;
            case 30541: // Blaze (needs conditions entry)
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_TARGET_ENEMY);
                spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo();
                break;
            case 63665: // Charge (Argent Tournament emote on riders)
            case 31298: // Sleep (needs target selection script)
            case 51904: // Summon Ghouls On Scarlet Crusade (this should use conditions table, script for this spell needs to be fixed)
            case 2895:  // Wrath of Air Totem rank 1 (Aura)
            case 68933: // Wrath of Air Totem rank 2 (Aura)
            case 29200: // Purify Helboar Meat
            case 10872: // Abolish Disease Effect
            case 3137:  // Abolish Poison Effect
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
                spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo();
                break;
            case 31344: // Howl of Azgalor
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_100_YARDS); // 100yards instead of 50000?!
                break;
            case 42818: // Headless Horseman - Wisp Flight Port
            case 42821: // Headless Horseman - Wisp Flight Missile
                spellInfo->RangeEntry = sSpellRangeStore.LookupEntry(6); // 100 yards
                break;
            case 36350: //They Must Burn Bomb Aura (self)
                spellInfo->Effects[EFFECT_0].TriggerSpell = 36325; // They Must Burn Bomb Drop (DND)
                break;
            case 61407: // Energize Cores
            case 62136: // Energize Cores
            case 54069: // Energize Cores
            case 56251: // Energize Cores
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_SRC_AREA_ENTRY);
                break;
            case 50785: // Energize Cores
            case 59372: // Energize Cores
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_SRC_AREA_ENEMY);
                break;
            case 8494: // Mana Shield (rank 2)
                // because of bug in dbc
                spellInfo->ProcChance = 0;
                break;
            case 20335: // Heart of the Crusader
            case 20336:
            case 20337:
            case 63320: // Glyph of Life Tap
            // Entries were not updated after spell effect change, we have to do that manually :/
                spellInfo->AttributesEx3 |= SPELL_ATTR3_CAN_PROC_WITH_TRIGGERED;
                break;
            case 59725: // Improved Spell Reflection - aoe aura
                // Target entry seems to be wrong for this spell :/
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER_AREA_PARTY);
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_10_YARDS_2);
                break;
            case 44978: // Wild Magic
            case 45001:
            case 45002:
            case 45004:
            case 45006:
            case 45010:
            case 31347: // Doom
            case 41635: // Prayer of Mending
            case 44869: // Spectral Blast
            case 45027: // Revitalize
            case 45976: // Muru Portal Channel
            case 39365: // Thundering Storm
            case 41071: // Raise Dead (HACK)
            case 52124: // Sky Darkener Assault
            case 42442: // Vengeance Landing Cannonfire
            case 45863: // Cosmetic - Incinerate to Random Target
            case 25425: // Shoot
            case 45761: // Shoot
            case 42611: // Shoot
            case 61588: // Blazing Harpoon
            case 52479: // Gift of the Harvester
            case 48246: // Ball of Flame
            case 75115: // Burning Light
            case 76674: // Chaos Blast
            case 102726: // asira silence marked
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 36384: // Skartax Purple Beam
                spellInfo->MaxAffectedTargets = 2;
                break;
            case 41376: // Spite
            case 39992: // Needle Spine
            case 29576: // Multi-Shot
            case 40816: // Saber Lash
            case 37790: // Spread Shot
            case 46771: // Flame Sear
            case 45248: // Shadow Blades
            case 41303: // Soul Drain
            case 54172: // Divine Storm (heal)
            case 29213: // Curse of the Plaguebringer - Noth
            case 28542: // Life Drain - Sapphiron
            case 66588: // Flaming Spear
            case 54171: // Divine Storm
                spellInfo->MaxAffectedTargets = 3;
                break;
            case 38310: // Multi-Shot
                spellInfo->MaxAffectedTargets = 4;
                break;
            case 42005: // Bloodboil
            case 38296: // Spitfire Totem
            case 37676: // Insidious Whisper
            case 46008: // Negative Energy
            case 45641: // Fire Bloom
            case 55665: // Life Drain - Sapphiron (H)
            case 28796: // Poison Bolt Volly - Faerlina
                spellInfo->MaxAffectedTargets = 5;
                break;
            case 40827: // Sinful Beam
            case 40859: // Sinister Beam
            case 40860: // Vile Beam
            case 40861: // Wicked Beam
            case 54835: // Curse of the Plaguebringer - Noth (H)
            case 54098: // Poison Bolt Volly - Faerlina (H)
                spellInfo->MaxAffectedTargets = 10;
                break;
            case 50312: // Unholy Frenzy
                spellInfo->MaxAffectedTargets = 15;
                break;
            case 33711: // Murmur's Touch
            case 38794:
                spellInfo->MaxAffectedTargets = 1;
                spellInfo->Effects[EFFECT_0].TriggerSpell = 33760;
                break;
            case 17941: // Shadow Trance
            case 22008: // Netherwind Focus
            case 31834: // Light's Grace
            case 34754: // Clearcasting
            case 34936: // Backlash
            case 48108: // Hot Streak
            case 51124: // Killing Machine
            case 54741: // Firestarter
            case 57761: // Fireball!
            case 39805: // Lightning Overload
            case 64823: // Item - Druid T8 Balance 4P Bonus
            case 34477: // Misdirection
            case 44401: // Missile Barrage
            case 93400: // Shooting Stars
            case 88819: // Daybreak
            case 81093: // Fury of stormrage
                spellInfo->ProcCharges = 1;
                break;
            case 53257: // Cobra strikes
                spellInfo->StackAmount = 0;
                spellInfo->ProcCharges = 2;
                break;
            case 44544: // Fingers of Frost
                spellInfo->Effects[0].SpellClassMask[0] |= 0x00020000;
                break;
            case 57470: // Renewed Hope
            case 57472:
                spellInfo->Effects[0].SpellClassMask[0] |= 0x00000800;
                break;
            case 28200: // Ascendance (Talisman of Ascendance trinket)
                spellInfo->ProcCharges = 6;
                break;
            case 47201: // Everlasting Affliction
            case 47202:
            case 47203:
            case 47204:
            case 47205:
                // add corruption to affected spells
                spellInfo->Effects[1].SpellClassMask[0] |= 2;
                break;
            case 51852: // The Eye of Acherus (no spawn in phase 2 in db)
                spellInfo->Effects[0].MiscValue |= 1;
                break;
            case 51912: // Crafty's Ultra-Advanced Proto-Typical Shortening Blaster
                spellInfo->Effects[0].Amplitude = 3000;
                break;
            case 29809: // Desecration Arm - 36 instead of 37 - typo? :/
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_7_YARDS);
                break;
            // Master Shapeshifter: missing stance data for forms other than bear - bear version has correct data
            // To prevent aura staying on target after talent unlearned
            case 48420:
                spellInfo->Stances = 1 << (FORM_CAT - 1);
                break;
            case 48421:
                spellInfo->Stances = 1 << (FORM_MOONKIN - 1);
                break;
            // Tree of Life passives
            case 5420:
                spellInfo->Stances = 1 << (FORM_TREE - 1);
                break;
            case 81097:
                spellInfo->Stances = 1 << (FORM_TREE - 1);
                break;
                // Heart of the Wild
			case 17003:
				spellInfo->SpellFamilyName = SPELLFAMILY_DRUID;

				spellInfo->Effects[EFFECT_1].Effect = SPELL_EFFECT_APPLY_AURA;
				spellInfo->Effects[EFFECT_1].BasePoints = 0;
				spellInfo->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_MOD_ATTACK_POWER_PCT;
				spellInfo->Effects[EFFECT_1].MiscValueB = 3;

				spellInfo->Effects[EFFECT_2].Effect = SPELL_EFFECT_APPLY_AURA;
				spellInfo->Effects[EFFECT_2].BasePoints = 0;
				spellInfo->Effects[EFFECT_2].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
				spellInfo->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_MOD_PERCENT_STAT;
				spellInfo->Effects[EFFECT_2].MiscValue = UNIT_MOD_STAT_STAMINA;
				spellInfo->Effects[EFFECT_2].MiscValueB = 2;
				break;
			case 17004:
				spellInfo->SpellFamilyName = SPELLFAMILY_DRUID;

				spellInfo->Effects[EFFECT_1].Effect = SPELL_EFFECT_APPLY_AURA;
				spellInfo->Effects[EFFECT_1].BasePoints = 0;
				spellInfo->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_MOD_ATTACK_POWER_PCT;
				spellInfo->Effects[EFFECT_1].MiscValueB = 7;

				spellInfo->Effects[EFFECT_2].Effect = SPELL_EFFECT_APPLY_AURA;
				spellInfo->Effects[EFFECT_2].BasePoints = 0;
				spellInfo->Effects[EFFECT_2].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
				spellInfo->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_MOD_PERCENT_STAT;
				spellInfo->Effects[EFFECT_2].MiscValue = UNIT_MOD_STAT_STAMINA;
				spellInfo->Effects[EFFECT_2].MiscValueB = 4;
				break;
			case 17005:
				spellInfo->SpellFamilyName = SPELLFAMILY_DRUID;
				spellInfo->Effects[EFFECT_1].Effect = SPELL_EFFECT_APPLY_AURA;
				spellInfo->Effects[EFFECT_1].BasePoints = 0;
				spellInfo->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_MOD_ATTACK_POWER_PCT;
				spellInfo->Effects[EFFECT_1].MiscValueB = 10;

				spellInfo->Effects[EFFECT_2].Effect = SPELL_EFFECT_APPLY_AURA;
				spellInfo->Effects[EFFECT_2].BasePoints = 0;
				spellInfo->Effects[EFFECT_2].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
				spellInfo->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_MOD_PERCENT_STAT;
				spellInfo->Effects[EFFECT_2].MiscValue = UNIT_MOD_STAT_STAMINA;
				spellInfo->Effects[EFFECT_2].MiscValueB = 6;
				break;
            case 51466: // Elemental Oath (Rank 1)
            case 51470: // Elemental Oath (Rank 2)
                spellInfo->Effects[EFFECT_1].Effect = SPELL_EFFECT_APPLY_AURA;
                spellInfo->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_ADD_FLAT_MODIFIER;
                spellInfo->Effects[EFFECT_1].MiscValue = SPELLMOD_EFFECT2;
                spellInfo->Effects[EFFECT_1].SpellClassMask = flag96(0x00000000, 0x00004000, 0x00000000);
                break;
            case 47569: // Improved Shadowform (Rank 1)
                // with this spell atrribute aura can be stacked several times
                spellInfo->Attributes &= ~SPELL_ATTR0_NOT_SHAPESHIFT;
                break;
            case 64904: // Hymn of Hope
                spellInfo->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT;
                spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
                break;
            case 30421: // Nether Portal - Perseverence
                spellInfo->Effects[2].BasePoints += 30000;
                break;
            case 41913: // Parasitic Shadowfiend Passive
                spellInfo->Effects[0].ApplyAuraName = SPELL_AURA_DUMMY; // proc debuff, and summon infinite fiends
                break;
            case 27892: // To Anchor 1
            case 27928: // To Anchor 1
            case 27935: // To Anchor 1
            case 27915: // Anchor to Skulls
            case 27931: // Anchor to Skulls
            case 27937: // Anchor to Skulls
            case 48714: // Compelled
            case 7853:  // The Art of Being a Water Terror: Force Cast on Player
			case 16177: // Ancestral Fortitude (Rank 1)
            case 16236: // Ancestral Fortitude (Rank 2)
            case 16237: // Ancestral Fortitude (Rank 3)
                spellInfo->RangeEntry = sSpellRangeStore.LookupEntry(13);
                break;
            // target allys instead of enemies, target A is src_caster, spells with effect like that have ally target
            // this is the only known exception, probably just wrong data
            case 29214: // Wrath of the Plaguebringer
            case 54836: // Wrath of the Plaguebringer
                spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo(TARGET_UNIT_SRC_AREA_ALLY);
                spellInfo->Effects[EFFECT_1].TargetB = SpellImplicitTargetInfo(TARGET_UNIT_SRC_AREA_ALLY);
                break;
            case 63675: // Improved Devouring Plague
                spellInfo->AttributesEx3 |= SPELL_ATTR3_NO_DONE_BONUS;
                break;
            case 8145: // Tremor Totem (instant pulse)
            case 6474: // Earthbind Totem (instant pulse)
                spellInfo->AttributesEx5 |= SPELL_ATTR5_START_PERIODIC_AT_APPLY;
                break;
            case 53241: // Marked for Death (Rank 1)
            case 53243: // Marked for Death (Rank 2)
            case 53244: // Marked for Death (Rank 3)
            case 53245: // Marked for Death (Rank 4)
            case 53246: // Marked for Death (Rank 5)
                spellInfo->Effects[EFFECT_0].SpellClassMask = flag96(0x00067801, 0x10820001, 0x00000801);
                break;
            case 70728: // Exploit Weakness (needs target selection script)
            case 70840: // Devious Minds (needs target selection script)
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
                spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo(TARGET_UNIT_PET);
                break;
            case 70893: // Culling The Herd (needs target selection script)
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
                spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo(TARGET_UNIT_MASTER);
                break;
            case 54800: // Sigil of the Frozen Conscience - change class mask to custom extended flags of Icy Touch
                        // this is done because another spell also uses the same SpellFamilyFlags as Icy Touch
                        // SpellFamilyFlags[0] & 0x00000040 in SPELLFAMILY_DEATHKNIGHT is currently unused (3.3.5a)
                        // this needs research on modifier applying rules, does not seem to be in Attributes fields
                spellInfo->Effects[EFFECT_0].SpellClassMask = flag96(0x00000040, 0x00000000, 0x00000000);
                break;
            case 64949: // Idol of the Flourishing Life
                spellInfo->Effects[EFFECT_0].SpellClassMask = flag96(0x00000000, 0x02000000, 0x00000000);
                spellInfo->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_ADD_FLAT_MODIFIER;
                break;
            case 34231: // Libram of the Lightbringer
            case 60792: // Libram of Tolerance
            case 64956: // Libram of the Resolute
                spellInfo->Effects[EFFECT_0].SpellClassMask = flag96(0x80000000, 0x00000000, 0x00000000);
                spellInfo->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_ADD_FLAT_MODIFIER;
                break;
            case 28851: // Libram of Light
            case 28853: // Libram of Divinity
            case 32403: // Blessed Book of Nagrand
                spellInfo->Effects[EFFECT_0].SpellClassMask = flag96(0x40000000, 0x00000000, 0x00000000);
                spellInfo->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_ADD_FLAT_MODIFIER;
                break;
            case 45602: // Ride Carpet
                spellInfo->Effects[EFFECT_0].BasePoints = 0; // force seat 0, vehicle doesn't have the required seat flags for "no seat specified (-1)"
                break;
            case 64745: // Item - Death Knight T8 Tank 4P Bonus
            case 64936: // Item - Warrior T8 Protection 4P Bonus
                spellInfo->Effects[0].BasePoints = 100; // 100% chance of procc'ing, not -10% (chance calculated in PrepareTriggersExecutedOnHit)
                break;
            case 59414: // Pulsing Shockwave Aura (Loken)
                // this flag breaks movement, remove it
                spellInfo->AttributesEx &= ~SPELL_ATTR1_CHANNELED_1;
                break;
            case 61719: // Easter Lay Noblegarden Egg Aura - Interrupt flags copied from aura which this aura is linked with
                spellInfo->AuraInterruptFlags = AURA_INTERRUPT_FLAG_HITBYSPELL | AURA_INTERRUPT_FLAG_TAKE_DAMAGE;
                break;
            case 70650: // Death Knight T10 Tank 2P Bonus
                spellInfo->Effects[0].ApplyAuraName = SPELL_AURA_ADD_PCT_MODIFIER;
                break;
            case 71838: // Drain Life - Bryntroll Normal
            case 71839: // Drain Life - Bryntroll Heroic
                spellInfo->AttributesEx2 |= SPELL_ATTR2_CANT_CRIT;
                break;
            case 34471: // The Beast Within
                spellInfo->AttributesEx5 |= SPELL_ATTR5_USABLE_WHILE_CONFUSED | SPELL_ATTR5_USABLE_WHILE_FEARED | SPELL_ATTR5_USABLE_WHILE_STUNNED;
                break;
            case 68182: // Shatter Rubble - For Quest: Deep Impact, now targets gameobject and not self / npc
                spellInfo->AttributesEx |= SPELL_ATTR1_CANT_TARGET_SELF;
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_GAMEOBJECT_NEARBY_ENTRY;
                break;
                // Burn Hay, Braizer Torch
            case 88646:
                spellInfo->Effects[EFFECT_1].Effect = 0;
                spellInfo->Effects[EFFECT_1].TargetA = 0;
                spellInfo->Effects[EFFECT_1].TargetB = 0;
                break;
            // ULDUAR SPELLS
            //
            case 62374: // Pursued (Flame Leviathan)
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50000_YARDS);   // 50000yd
                break;
            case 63342: // Focused Eyebeam Summon Trigger (Kologarn)
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 77303: // Highborne Prison, very weird spell. Has two triggers, so adding them all. The spell kills everything around it, it ignores all conditions, some DBC mess-up?? 
            case 77306: // Highborne Prison, very weird spell. Has two triggers, so adding them all. The spell kills everything around it, it ignores all conditions, some DBC mess-up?? 
            case 77307: // Highborne Prison, very weird spell. Has two triggers, so adding them all. The spell kills everything around it, it ignores all conditions, some DBC mess-up?? 
                spellInfo->MaxAffectedTargets = 1;
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_0_5_YARDS);
                spellInfo->Effects[EFFECT_1].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_0_5_YARDS);
                spellInfo->Effects[EFFECT_2].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_0_5_YARDS);
                break;
            case 62716: // Growth of Nature (Freya)
            case 65584: // Growth of Nature (Freya)
            case 64381: // Strength of the Pack (Auriaya)
                spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
                break;
            case 63018: // Searing Light (XT-002)
            case 65121: // Searing Light (25m) (XT-002)
            case 63024: // Gravity Bomb (XT-002)
            case 64234: // Gravity Bomb (25m) (XT-002)
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 62834: // Boom (XT-002)
            // This hack is here because we suspect our implementation of spell effect execution on targets
            // is done in the wrong order. We suspect that EFFECT_0 needs to be applied on all targets,
            // then EFFECT_1, etc - instead of applying each effect on target1, then target2, etc.
            // The above situation causes the visual for this spell to be bugged, so we remove the instakill
            // effect and implement a script hack for that.
                spellInfo->Effects[EFFECT_1].Effect = 0;
                break;
            case 64386: // Terrifying Screech (Auriaya)
            case 64389: // Sentinel Blast (Auriaya)
            case 64678: // Sentinel Blast (Auriaya)
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_5_SECONDS);
                break;
            case 64321: // Potent Pheromones (Freya)
                // spell should dispel area aura, but doesn't have the attribute
                // may be db data bug, or blizz may keep reapplying area auras every update with checking immunity
                // that will be clear if we get more spells with problem like this
                spellInfo->AttributesEx |= SPELL_ATTR1_DISPEL_AURAS_ON_IMMUNITY;
                break;
            case 62301: // Cosmic Smash (Algalon the Observer)
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 64598: // Cosmic Smash (Algalon the Observer)
                spellInfo->MaxAffectedTargets = 3;
                break;
            case 62293: // Cosmic Smash (Algalon the Observer)
                spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo(TARGET_DEST_CASTER);
                break;
            case 62311: // Cosmic Smash (Algalon the Observer)
            case 64596: // Cosmic Smash (Algalon the Observer)
                spellInfo->RangeEntry = sSpellRangeStore.LookupEntry(6);  // 100yd
                break;
            // ENDOF ULDUAR SPELLS
            //
            // TRIAL OF THE CRUSADER SPELLS
            //
            case 66258: // Infernal Eruption (10N)
            case 67901: // Infernal Eruption (25N)
                // increase duration from 15 to 18 seconds because caster is already
                // unsummoned when spell missile hits the ground so nothing happen in result
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_18_SECONDS);
                break;
            // ENDOF TRIAL OF THE CRUSADER SPELLS
            //
            // ICECROWN CITADEL SPELLS
            //
            // THESE SPELLS ARE WORKING CORRECTLY EVEN WITHOUT THIS HACK
            // THE ONLY REASON ITS HERE IS THAT CURRENT GRID SYSTEM
            // DOES NOT ALLOW FAR OBJECT SELECTION (dist > 333)
            case 70781: // Light's Hammer Teleport
            case 70856: // Oratory of the Damned Teleport
            case 70857: // Rampart of Skulls Teleport
            case 70858: // Deathbringer's Rise Teleport
            case 70859: // Upper Spire Teleport
            case 70860: // Frozen Throne Teleport
            case 70861: // Sindragosa's Lair Teleport
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_DEST_DB);
                break;
            case 69055: // Saber Lash (Lord Marrowgar)
            case 70814: // Saber Lash (Lord Marrowgar)
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_5_YARDS); // 5yd
                break;
            case 69075: // Bone Storm (Lord Marrowgar)
            case 70834: // Bone Storm (Lord Marrowgar)
            case 70835: // Bone Storm (Lord Marrowgar)
            case 70836: // Bone Storm (Lord Marrowgar)
            case 72864: // Death Plague (Rotting Frost Giant)
            case 71160: // Plague Stench (Stinky)
            case 71161: // Plague Stench (Stinky)
            case 71123: // Decimate (Stinky & Precious)
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_100_YARDS); // 100yd
                break;
            case 71169: // Shadow's Fate
                spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
                break;
            case 72378: // Blood Nova (Deathbringer Saurfang)
            case 73058: // Blood Nova (Deathbringer Saurfang)
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_200_YARDS);
                spellInfo->Effects[EFFECT_1].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_200_YARDS);
                break;
            case 72769: // Scent of Blood (Deathbringer Saurfang)
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_200_YARDS);
                // no break
            case 72771: // Scent of Blood (Deathbringer Saurfang)
                spellInfo->Effects[EFFECT_1].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_200_YARDS);
                break;
            case 72723: // Resistant Skin (Deathbringer Saurfang adds)
                // this spell initially granted Shadow damage immunity, however it was removed but the data was left in client
                spellInfo->Effects[EFFECT_2].Effect = 0;
                break;
            case 70460: // Coldflame Jets (Traps after Saurfang)
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_10_SECONDS);
                break;
            case 71412: // Green Ooze Summon (Professor Putricide)
            case 71415: // Orange Ooze Summon (Professor Putricide)
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_TARGET_ANY);
                break;
            case 71159: // Awaken Plagued Zombies
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_INFINITE);
                break;
            case 70530: // Volatile Ooze Beam Protection (Professor Putricide)
                spellInfo->Effects[EFFECT_0].Effect = SPELL_EFFECT_APPLY_AURA; // for an unknown reason this was SPELL_EFFECT_APPLY_AREA_AURA_RAID
                break;
            // THIS IS HERE BECAUSE COOLDOWN ON CREATURE PROCS IS NOT IMPLEMENTED
            case 71604: // Mutated Strength (Professor Putricide)
            case 72673: // Mutated Strength (Professor Putricide)
            case 72674: // Mutated Strength (Professor Putricide)
            case 72675: // Mutated Strength (Professor Putricide)
                spellInfo->Effects[EFFECT_1].Effect = 0;
                break;
            case 72454: // Mutated Plague (Professor Putricide)
            case 72464: // Mutated Plague (Professor Putricide)
            case 72506: // Mutated Plague (Professor Putricide)
            case 72507: // Mutated Plague (Professor Putricide)
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50000_YARDS); // 50000yd
                break;
            case 70911: // Unbound Plague (Professor Putricide) (needs target selection script)
            case 72854: // Unbound Plague (Professor Putricide) (needs target selection script)
            case 72855: // Unbound Plague (Professor Putricide) (needs target selection script)
            case 72856: // Unbound Plague (Professor Putricide) (needs target selection script)
                spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo(TARGET_UNIT_TARGET_ENEMY);
                break;
            case 71518: // Unholy Infusion Quest Credit (Professor Putricide)
            case 72934: // Blood Infusion Quest Credit (Blood-Queen Lana'thel)
            case 72289: // Frost Infusion Quest Credit (Sindragosa)
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50000_YARDS); // another missing radius
                break;
            case 71708: // Empowered Flare (Blood Prince Council)
            case 72785: // Empowered Flare (Blood Prince Council)
            case 72786: // Empowered Flare (Blood Prince Council)
            case 72787: // Empowered Flare (Blood Prince Council)
                spellInfo->AttributesEx3 |= SPELL_ATTR3_NO_DONE_BONUS;
                break;
            case 71266: // Swarming Shadows
            case 72890: // Swarming Shadows
                spellInfo->AreaGroupId = 0; // originally, these require area 4522, which is... outside of Icecrown Citadel
                break;
            case 79625:
                spellInfo->AttributesEx4 |= SPELL_ATTR4_FIXED_DAMAGE;
                break;
            case 70602: // Corruption
            case 48278: // Paralyze
            case 22959: // Scorch
            case 58567: // sunder armor
            case 12579: // Winterchill
            case 17364: // Stormstrike
            case 50328: // Leeching Poison
            case 40520: // Shade Soul Channel
                spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
                break;
            case 47476: // Deathknight - Strangulate
            case 15487: // Priest - Silence
            case 5211:  // Druid - Bash
                spellInfo->AttributesEx7 |= SPELL_ATTR7_INTERRUPT_ONLY_NONPLAYER;
                break;
            case 70715: // Column of Frost (visual marker)
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_6_SECONDS);
                break;
            case 71085: // Mana Void (periodic aura)
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_30_SECONDS);
                break;
				case 68659: // Launch
 				spellInfo->Effects[1].TriggerSpell = 4336;
 				break;
            case 72015: // Frostbolt Volley (only heroic)
            case 72016: // Frostbolt Volley (only heroic)
                spellInfo->Effects[EFFECT_2].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_40_YARDS);
                break;
            case 70936: // Summon Suppressor (needs target selection script)
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_TARGET_ANY);
                spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo();
                break;
            case 72706: // Achievement Check (Valithria Dreamwalker)
            case 71357: // Order Whelp
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_200_YARDS);   // 200yd
                break;
            case 70598: // Sindragosa's Fury
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_DEST_DEST);
                break;
            case 69846: // Frost Bomb
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_UNIT_TARGET_ANY;
                spellInfo->Effects[EFFECT_1].TargetA = TARGET_UNIT_TARGET_ANY;
                spellInfo->Effects[EFFECT_1].Effect = 0;
                break;
            case 71614: // Ice Lock
                spellInfo->Mechanic = MECHANIC_STUN;
                break;
            case 72762: // Defile
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_53_SECONDS);
                break;
            case 72743: // Defile
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_45_SECONDS);
                break;
            case 72754: // Defile
            case 73708: // Defile
            case 73709: // Defile
            case 73710: // Defile
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_200_YARDS); // 200yd
                spellInfo->Effects[EFFECT_1].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_200_YARDS); // 200yd
                break;
            case 69030: // Val'kyr Target Search
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_200_YARDS); // 200yd
                spellInfo->Effects[EFFECT_1].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_200_YARDS); // 200yd
                break;
            case 69198: // Raging Spirit Visual
                spellInfo->RangeEntry = sSpellRangeStore.LookupEntry(13); // 50000yd
                break;
            case 73654: // Harvest Souls
            case 74295: // Harvest Souls
            case 74296: // Harvest Souls
            case 74297: // Harvest Souls
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50000_YARDS); // 50000yd
                spellInfo->Effects[EFFECT_1].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50000_YARDS); // 50000yd
                spellInfo->Effects[EFFECT_2].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50000_YARDS); // 50000yd
                break;
            case 73655: // Harvest Soul
                spellInfo->AttributesEx3 |= SPELL_ATTR3_NO_DONE_BONUS;
                break;
            case 73540: // Summon Shadow Trap
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_90_SECONDS);
                break;
            case 73530: // Shadow Trap (visual)
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_5_SECONDS);
                break;
            case 73529: // Shadow Trap
                spellInfo->Effects[EFFECT_1].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_10_YARDS); // 10yd
                break;
            case 74282: // Shadow Trap (searcher)
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_3_YARDS); // 3yd
                break;
            case 72595: // Restore Soul
            case 73650: // Restore Soul
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_200_YARDS); // 200yd
                break;
            case 74086: // Destroy Soul
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_200_YARDS); // 200yd
                break;
            case 74302: // Summon Spirit Bomb
            case 74342: // Summon Spirit Bomb
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_200_YARDS); // 200yd
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 74341: // Summon Spirit Bomb
            case 74343: // Summon Spirit Bomb
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_200_YARDS); // 200yd
                spellInfo->MaxAffectedTargets = 3;
                break;
            case 73579: // Summon Spirit Bomb
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_25_YARDS); // 25yd
                break;
            case 72350: // Fury of Frostmourne
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50000_YARDS); // 50000yd
                spellInfo->Effects[EFFECT_1].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50000_YARDS); // 50000yd
                break;
            case 75127: // Kill Frostmourne Players
            case 72351: // Fury of Frostmourne
            case 72431: // Jump (removes Fury of Frostmourne debuff)
            case 72429: // Mass Resurrection
            case 73159: // Play Movie
            case 73582: // Trigger Vile Spirit (Inside, Heroic)
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50000_YARDS); // 50000yd
                break;
            case 72376: // Raise Dead
                spellInfo->MaxAffectedTargets = 3;
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50000_YARDS); // 50000yd
                break;
            case 71809: // Jump
                spellInfo->RangeEntry = sSpellRangeStore.LookupEntry(3); // 20yd
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_25_YARDS); // 25yd
                break;
            case 72405: // Broken Frostmourne
                spellInfo->Effects[EFFECT_1].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_200_YARDS); // 200yd
                break;
            // ENDOF ICECROWN CITADEL SPELLS
            //
            // RUBY SANCTUM SPELLS
            //
            case 74769: // Twilight Cutter
            case 77844: // Twilight Cutter
            case 77845: // Twilight Cutter
            case 77846: // Twilight Cutter
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_100_YARDS); // 100yd
                break;
            case 75509: // Twilight Mending
                spellInfo->AttributesEx6 |= SPELL_ATTR6_CAN_TARGET_INVISIBLE;
                spellInfo->AttributesEx2 |= SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS;
                break;
            case 102975:
                spellInfo->AttributesEx2 &= ~SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS;
                break;
            case 75888: // Awaken Flames
            case 75889: // Awaken Flames
                spellInfo->AttributesEx |= SPELL_ATTR1_CANT_TARGET_SELF;
                break;
            // ENDOF RUBY SANCTUM SPELLS
            //
            case 40055: // Introspection
            case 40165: // Introspection
            case 40166: // Introspection
            case 40167: // Introspection
                spellInfo->Attributes |= SPELL_ATTR0_NEGATIVE_1;
                break;
            case 2378: // Minor Fortitude
                spellInfo->ManaCost = 0;
                spellInfo->ManaPerSecond = 0;
                break;
            case 62012: // Turkey Caller
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_0_YARDS); // 0yd
                break;
            // OCULUS SPELLS
            // The spells below are here, because their effect 1 is giving warning, because the triggered spell is not found in dbc and is missing from encounter sniff.
            case 49462: // Call Ruby Drake
            case 49461: // Call Amber Drake
            case 49345: // Call Emerald Drake
                spellInfo->Effects[EFFECT_1].Effect = 0;
                break;
            // Halls Of Origination spells
            // Temple Guardian Anhuur
            case 76606: // Disable Beacon Beams L
            case 76608: // Disable Beacon Beams R
                // Little hack, Increase the radius so it can hit the Cave In Stalkers in the platform.
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_45_YARDS);
                break;
            case 75323: // Reverberating Hymn
                // Aura is refreshed at 3 seconds, and the tick should happen at the fourth.
                spellInfo->AttributesEx8 |= SPELL_ATTR8_DONT_RESET_PERIODIC_TIMER;
                break;
            case 24314: // Threatening Gaze
                spellInfo->AuraInterruptFlags |= AURA_INTERRUPT_FLAG_CAST | AURA_INTERRUPT_FLAG_MOVE | AURA_INTERRUPT_FLAG_JUMP;
                break;
                break;
            case 45257: // Using Steam Tonk Controller
            case 45440: // Steam Tonk Controller
            case 60256: // Collect Sample
                // Crashes client on pressing ESC
                spellInfo->AttributesEx4 &= ~SPELL_ATTR4_TRIGGERED;
                break;
            case 67919:
                spellInfo->Effects[EFFECT_0].MiscValue = 750;
                spellInfo->CastTimeMin = 0;
                spellInfo->CastTimeMax = 0;
                spellInfo->CastTimeMaxLevel = 0;
                break;
            case 69992: // launch the bomball
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_100_YARDS);
                break;
            case 76085: // fissure
            case 91375:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_10_YARDS);
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = spellInfo->Effects[EFFECT_0].RadiusEntry;
                break;
            case 88448: // Demonic Rebirth
                spellInfo->RecoveryTime = 0;
                break;
            case 85107: // Impending Doom
            case 85108:
                spellInfo->Effects[EFFECT_1].Effect = SPELL_EFFECT_APPLY_AURA;
                spellInfo->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_DUMMY;
                spellInfo->Effects[EFFECT_1].MiscValue = 15;
                break;
            case 93975: // Aura of Foreboding
            case 93986:
            case 93987:
            case 93974:
                spellInfo->AttributesEx8 |= SPELL_ATTR8_CANT_MISS;
                break;
            case 79268: // soul harvest
            case 101976:
                spellInfo->Effects[EFFECT_0].Amplitude = 3000;
                break;
            // Tol'vir spells
            case 82215: // Soul Sever summon
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_DEST_CASTER_SUMMON);
                break;
            case 82195: // wail of the darkness summon radius
            case 82203: // wail of the darkness summon radius
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_15_YARDS);
                break;
            case 82430: // repentance pull towards - sometimes i don't understand blizzards logic.....
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_SRC_CASTER);
                spellInfo->Effects[EFFECT_1].TargetA = SpellImplicitTargetInfo(TARGET_SRC_CASTER);
                spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo(TARGET_UNIT_SRC_AREA_ENEMY);
                spellInfo->Effects[EFFECT_1].TargetB = SpellImplicitTargetInfo(TARGET_UNIT_SRC_AREA_ENEMY);
                spellInfo->Effects[EFFECT_0].MiscValue = 150;
                break;
            case 83131: //summon shockwave N base research on http://www.youtube.com/watch?v=CtTepsVPpRQ
            case 83132: //summon shockwave S,E,W no radisu entry defined in dbc for those 3 spells
            case 83133:
            case 83134:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50_YARDS);
                break;
            case 85016: //slipstream
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
                spellInfo->Effects[EFFECT_1].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
                break;
                // Ammunae
            case 75657: // Ammunaes Buff should only target himself
            case 94970:
                spellInfo->Effects[EFFECT_0].TargetB = 0;
                spellInfo->Effects[EFFECT_1].TargetB = 0;
                break;
            case 89124: // Ammunae Heal
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_UNIT_NEARBY_ENTRY;
                spellInfo->Effects[EFFECT_0].TargetB = 0;
                break;
            case 85552: // Twilight highland quest spells 
            case 85631:
            case 85902:
            case 85903:
            case 85916:
            case 85917:
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_DEST_CASTER_SUMMON);
                break;
                // Setesh
            case 76681: // Setesh Chaos Blast Aura Radius
            case 89875:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(32);
                break;
            case 76684: // Chaos Burn Triggger
            case 89874:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(13);
                break;
                // Rajh
            case 74108: // Solar Fire Damage Aura
            case 89130:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_4_YARDS);
                spellInfo->Effects[EFFECT_1].RadiusEntry = sSpellRadiusStore.LookupEntry(26);
                spellInfo->Effects[EFFECT_1].Effect = SPELL_EFFECT_KNOCK_BACK_DEST;
                break;
            case 89133:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_2_YARDS);
                break;
            case 83607: // Blight of Ozumat
                spellInfo->Effects[EFFECT_0].Amplitude = 1100;
                break;
            case 89821: // Hercular's Rod
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_UNIT_TARGET_ANY;
                break;
            case 75192: // Weakening, Flameseer's Staff
                spellInfo->StackAmount = 20;
                break;
            case 25281: // Turkey Vengeance
                spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
                break;
            case 76133: // vague des maree (throne of tides)
                spellInfo->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_MOD_DAMAGE_PERCENT_DONE;
                spellInfo->Effects[EFFECT_2].MiscValue = 127;
                spellInfo->Effects[EFFECT_2].BasePoints = 2000;
                break;
            case 75610:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50000_YARDS);
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 66532: // Fel Fireball
            case 66963:
            case 66964:
            case 66965:
                spellInfo->InterruptFlags |= SPELL_INTERRUPT_FLAG_INTERRUPT;
                break;
            case 83066: //Wailing Winds - fixed scriptside
                spellInfo->Effects[EFFECT_0].Amplitude = 3000;
                break;
            case 93520: // pistol barrage
                spellInfo->Effects[EFFECT_0].Amplitude = 1000;
                break;
            case 75540:
                spellInfo->MaxAffectedTargets = 2;
                break;
            case 89881:
                spellInfo->MaxAffectedTargets = 3;
                break;
            case 80895:
                spellInfo->Effects[EFFECT_1].Effect = 0;
                break;
            case 88314:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_3_YARDS);
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_3_YARDS);
                spellInfo->Effects[EFFECT_1].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_3_YARDS);
                spellInfo->Effects[EFFECT_1].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_3_YARDS);
                spellInfo->Effects[EFFECT_2].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_3_YARDS);
                spellInfo->Effects[EFFECT_2].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_3_YARDS);
                break;
            case 91180:
            case 81643:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_3_YARDS);
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_3_YARDS);
                break;
            case 91181:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_12_YARDS);
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_12_YARDS);
                break;
            case 83646:
            case 91253:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_10_YARDS);
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_10_YARDS);
                break;
            case 80340:
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_UNIT_NEARBY_ENEMY;
                break;
            case 91196:
            case 95249:
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_3_YARDS);
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_3_YARDS);
                break;
            case 89396: // Quicksand
                spellInfo->Effects[EFFECT_1].Effect = 0;
                break;
            case 88286:
            case 88282:
                spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo();
                spellInfo->Effects[EFFECT_1].TargetB = SpellImplicitTargetInfo();
                break;
            case 74976: // Disorented roar
            case 90737:
                spellInfo->ProcCharges = 1;
                break;
            case 88814: // terre benie
            case 90010:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_13_YARDS);
                break;
            case 92393: // rupture
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_25_SECONDS);
                break;
            case 78218: // atramedes Building Speed Effect
                spellInfo->Effects[EFFECT_0].BasePoints = 4;
                break;
            case 92463: // atramedes Building Speed Effect
                spellInfo->Effects[EFFECT_0].BasePoints = 6;
                break;
            case 92464: // atramedes Building Speed Effect
                spellInfo->Effects[EFFECT_0].BasePoints = 6;
                break;
            case 92465: // atramedes Building Speed Effect
                spellInfo->Effects[EFFECT_0].BasePoints = 8;
                break;
            case 51698: // Honor Among Thieves
                spellInfo->SpellFamilyName = SPELLFAMILY_ROGUE;
                break;
            case 3286:
                spellInfo->Effects[EFFECT_0].TargetA =  SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
                spellInfo->Effects[EFFECT_1].TargetA =  SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
                break;
            case 74087:
                spellInfo->Effects[EFFECT_0].TargetA =  SpellImplicitTargetInfo(TARGET_UNIT_NEARBY_ENTRY);
                break;
				/* Warlock corrections*/
			case 50589: // Immolation Aura - Metamoprhosis
			case 1949:  // Hellfire
				spellInfo->AttributesCu &= ~SPELL_ATTR0_CU_NEGATIVE;
				break;
			case 86041: // Hand of guldan
				spellInfo->AttributesEx5 |= SPELL_ATTR5_HIDE_DURATION;
				/* End */
				/* Hunter corrections*/
			case 94528: // Flare Trigger
				spellInfo->AttributesEx5 |= SPELL_ATTR5_HIDE_DURATION;
				spellInfo->AttributesEx2 |= SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS;
				break;
				/* End*/
            case 88954: // Consuming Darkness 10
            case 95173: // Consuming Darkness 25
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 82881: // chimaeron's break
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE_EFF0;
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE_EFF1;
                break;
            case 101441: // SPELL_GROWTH_CATALYST
            case 101442:
                spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
                break;
            case 97693: // flash freeze target selector
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 92970: // SPELL_SCORCHING_BLAST 25 HC
            case 82935: // Caustic Slime 10
            case 88915: // Caustic Slime 10 HC
            case 88916: // Caustic Slime 25
            case 88917: // Caustic Slime 25 HC
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_SHARE_DAMAGE;
                break;
            case 86058:
            case 83862:
            case 86607:
            case 92912:
            case 92913:
            case 92914:
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 91927: // Magma Spit 10hc
                spellInfo->MaxAffectedTargets = 3;
                break;
            case 91928: // Magma Spit 25hc
                spellInfo->MaxAffectedTargets = 8;
                break;
            case 82856:
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_DEST_DEST;
                spellInfo->Effects[EFFECT_0].MiscValue = 10;
                break;
            case 84948:
            case 92487:
                spellInfo->MaxAffectedTargets = 1;
                spellInfo->AttributesEx3 |= SPELL_ATTR3_UNINTERRUPTIBLE;
                break;
            case 92486:
            case 92488:
                spellInfo->MaxAffectedTargets = 3;
                spellInfo->AttributesEx3 |= SPELL_ATTR3_UNINTERRUPTIBLE;
                break;
            case 84912:
            case 92491:
            case 92492:
            case 92493:
                spellInfo->Effects[EFFECT_0].TargetB =  SpellImplicitTargetInfo(TARGET_UNIT_SRC_AREA_ENEMY);
                break;
            case 84644:
            case 84643:
                spellInfo->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_DUMMY;
                spellInfo->AttributesEx &= ~SPELL_ATTR1_CANT_TARGET_SELF;
                break;
            case 84638:
                spellInfo->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_DUMMY;
                spellInfo->AttributesEx &= ~SPELL_ATTR1_CANT_TARGET_SELF;
                break;
                case 98379: // Scorpion Form
            case 98374: // Cat Form
                spellInfo->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_DUMMY;
                break;
            // Adrenaline
            case 97238:
                spellInfo->Effects[EFFECT_1].BasePoints = 20;
                break;
            // Burning Orbs
            case 98565:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_40_YARDS);
                break;
            case 71041: // Dungeon Deserter
            case 86202:
            case 92889:
            case 92890:
            case 92891:
            case 88436:
            case 92892:
            case 92893:
            case 92894:
                spellInfo->SchoolMask = SPELL_SCHOOL_MASK_NORMAL;
                break;
            case 92504:
            case 92505:
                spellInfo->ProcFlags = 0;
                break;
            case 82151:
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 93193:
            case 93194:
                spellInfo->MaxAffectedTargets = 2;
                break;
            case 93195:
                spellInfo->MaxAffectedTargets = 4;
                break;
            case 95740:
                spellInfo->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_PROC_TRIGGER_SPELL;
                spellInfo->ProcFlags = 327680;
                spellInfo->ProcChance = 10;
                break;
            case 8187:
                spellInfo->AttributesEx |= SPELL_ATTR1_NO_THREAT;
                break;
            case 77486:
                spellInfo->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_DUMMY;
                break;
            case 84617:
                spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
                spellInfo->ProcCharges = -1;
                break;
				// Glyph of Totemic Recall
			case 55438:
				spellInfo->Effects[0].MiscValue = SPELLMOD_EFFECT1;
				spellInfo->Effects[0].ApplyAuraName = SPELL_AURA_ADD_PCT_MODIFIER;
				spellInfo->Effects[0].BasePoints = 200;
				break;
            case 52212:  // Death and Decay (Level 85)
                spellInfo->AttributesEx2 |= SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS;
                spellInfo->AttributesEx4 &= ~SPELL_ATTR4_TRIGGERED;
                break;
            case 62137: // dk pet aoe avoidance
                spellInfo->Effects[EFFECT_0].BasePoints = -90;
                break;
            case 6203:
                spellInfo->AttributesEx2 |= SPELL_ATTR2_CAN_TARGET_DEAD;
                break;
            case 74374: // Arcane Barrage
            case 89886: // Arcane Barrage
            case 74041: // Energy Flux
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 78684: // grele os
            case 94104:
            case 94105:
            case 94106:
                spellInfo->DurationEntry = 0;
                break;
            case 80167: // Drink - Seafood Magnifique Feast, Goblin Barbecue
            case 80166: // Drink - Goblin Barbecue Feast, Broiled Dragon Feast
                spellInfo->Effects[EFFECT_2].Effect = SPELL_EFFECT_APPLY_AURA;
                spellInfo->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_PERIODIC_TRIGGER_SPELL;
                spellInfo->Effects[EFFECT_2].Amplitude = 10000;
                spellInfo->Effects[EFFECT_2].BasePoints = 0;
                break;
            case 87604: // Food - Fortune Cookie
                spellInfo->Effects[EFFECT_2].Effect = SPELL_EFFECT_TRIGGER_SPELL;
                spellInfo->Effects[EFFECT_2].TriggerSpell = 84193;
                spellInfo->Effects[EFFECT_2].BasePoints = 0;
                spellInfo->Effects[EFFECT_1].BasePoints = 87604;
                break;
            case 82333:
                spellInfo->Effects[EFFECT_0].BasePoints = 4;
                break;
            case 91020:
            case 93258:
            case 93259:
            case 93260:
                spellInfo->Effects[EFFECT_1].BasePoints = -spellInfo->Effects[EFFECT_1].BasePoints;
                break;
            case 89668: // lighting rod
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 83968: // guild mass resurect
                spellInfo->AttributesEx2 |= SPELL_ATTR2_CAN_TARGET_DEAD;
                break;
            case 82170:
                spellInfo->Effects[EFFECT_2].BasePoints = 0;
                break;
            case 85425:
                spellInfo->AttributesEx &= ~SPELL_ATTR1_CHANNELED_2;
                break;
            case 93262:
            case 93263:
                spellInfo->InterruptFlags = 0;
                break;
            case 44543: // Fingers of Frost talent1
                spellInfo->ProcChance = 7;
                break;
            case 44545: // Fingers of Frost talent2
                spellInfo->ProcChance = 14;
                break;
            case 83074: // Fingers of Frost talent3
                spellInfo->ProcChance = 25;
                break;
            case 82857:
            case 92520:
            case 92521:
            case 92522:
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_DEST_DEST;
                spellInfo->Effects[EFFECT_1].TargetA = TARGET_DEST_DEST;
                break;
            case 82859:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50000_YARDS);
                break;
            case 83070:
            case 92454:
            case 92455:
            case 92456:
                spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo(TARGET_UNIT_TARGET_ENEMY);
                spellInfo->Effects[EFFECT_1].TargetB = SpellImplicitTargetInfo(TARGET_UNIT_TARGET_ENEMY);
                break;
            case 94731:
                spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo(TARGET_UNIT_TARGET_ENEMY);
                break;
            case 85078:
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
                break;
            case 94548:
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_10_SECONDS);
                break;
            case 13795: // v Immolation Trap
            case 22910:
            case 47784:
            case 52606:
            case 82944:
            case 13813: // v Explosive Trap (Fire)
            case 82938:
                spellInfo->Effects[EFFECT_0].Effect = SPELL_EFFECT_SUMMON_OBJECT_SLOT4;
                break;
            case 34026: // Kill Command
                spellInfo->AttributesEx |= SPELL_ATTR0_IMPOSSIBLE_DODGE_PARRY_BLOCK;
                break;
            case 90174: // Divine Purpose
                spellInfo->ProcCharges = 1;
                spellInfo->Effects[EFFECT_0].SpellClassMask = flag96(0x00000000, 0x00000000, 0x0020E000);
                break;
            case 7328:
                spellInfo->SpellFamilyName = SPELLFAMILY_PALADIN;
                break;
            case 20711: // spirit of redemption
                spellInfo->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_DUMMY;
                break;
            case 76595: // Flashburn
                spellInfo->Effects[EFFECT_0].SpellClassMask = flag96(0x08400000, 0x00021000, 0x00001010);
                break;
            case 79396: // Nefarian ombrase
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_30_SECONDS);
                break;
            case 92023:
                spellInfo->Speed = 75.0f;
                break;
            case 91849:
                spellInfo->Effects[EFFECT_0].BasePoints = 200;
                spellInfo->Effects[EFFECT_0].MiscValue = 200;
                break;
            case 91858:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_5_YARDS);
                spellInfo->Effects[EFFECT_1].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_5_YARDS);
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_5_YARDS);
                spellInfo->Effects[EFFECT_1].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_5_YARDS);
                break;
            case 92134:
            case 92196:
            case 92197:
            case 92198:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_1_YARD);
                spellInfo->Effects[EFFECT_1].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_1_YARD);
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_1_YARD);
                spellInfo->Effects[EFFECT_1].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_1_YARD);
                break;
            case 92720:
                spellInfo->ChannelInterruptFlags = 0;
                break;
            case 92272: // Item - Collecting Mana
                spellInfo->Effects[EFFECT_1].BasePoints = 0;
                break;
            case 78098:
            case 92403:
            case 92404:
            case 92405:
                spellInfo->InterruptFlags = 0;
                spellInfo->ChannelInterruptFlags = 0;
                break;
            case 90164: // Astral Alignment
                spellInfo->ProcCharges = 3;
                break;
            case 20549: // Warstomp
                spellInfo->Attributes |= SPELL_ATTR4_IGNORE_RESISTANCES; //For some reason warstomp had a chance to resist
                break;
            case 72830: // Achievement Check
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50000_YARDS); // 50000yd
                break;
            case 585: // Smite, apparantly the dbc data is wrong based on what the tooltip says. BIG HACK, must discover proper reason why it scales wrongly, eg smite says 10 mana, but yet it costs 15 mana ??
                spellInfo->ManaCost = -6;
                break;
            case 5176: // Wrath, same issue as above ^^
                spellInfo->ManaCost = -2;
                break;
            case 403: // Lightning Bolt, same issue as above ^^
                spellInfo->ManaCost = -1;
                break;
            case 133: // Fireball, same issue as above ^^
                spellInfo->ManaCost = -4;
                break;
            case 116: // Frostbolt, same issue as above ^^
                spellInfo->ManaCost = -3;
                break;
            case 686: // Shadowbolt, same issue as above ^^
                spellInfo->ManaCost = -4;
                break;
                // wrong dbc, 3 days cooldown
            case 60893: // Northrend Alchemy Research
                spellInfo->RecoveryTime = 0;
                spellInfo->CategoryRecoveryTime = 259200000;
                break;
                // wrong dbc, 7 days cooldown
            case 75141: // Dream of Skywall
            case 75142: // Dream of Deepholm
            case 75144: // Dream of Hyjal
            case 75145: // Dream of Ragnaros
            case 75146: // Dream of Azshara
                spellInfo->RecoveryTime = 0;
                spellInfo->CategoryRecoveryTime = 604800000;
                break;
                // wrong dbc, 1 day cooldown 
            case 80243: // Transmute: Truegold
            case 61288: // Minor Inscription Research
            case 61177: // Northrend Inscription Research
            case 53773: // Transmute: Eternal Life to Fire
            case 53784: // Transmute: Eternal Water to Fire
            case 53780: // Transmute: Eternal Shadow to Life
            case 53775: // Transmute: Eternal Fire to Life
            case 53776: // Transmute: Eternal Air to Water
            case 53774: // Transmute: Eternal Fire to Water
            case 53777: // Transmute: Eternal Air to Earth
            case 53779: // Transmute: Eternal Shadow to Earth
            case 53781: // Transmute: Eternal Earth to Air
            case 53783: // Transmute: Eternal Water to Air
            case 53782: // Transmute: Eternal Earth to Shadow
            case 53771: // Transmute: Eternal Life to Shadow
            case 80244: // Transmute: Pyrium Bar
            case 73478: // Fire Prism
                spellInfo->RecoveryTime = 0;
                spellInfo->CategoryRecoveryTime = 86400000;
                break;
            // All non-damaging interrupts off the global cooldown will now always hit the target.
            // This includes Pummel, Kick, Mind Freeze, Rebuke, Skull Bash, Counterspell, Wind Shear, Solar Beam, Silencing Shot, and related player pet abilities.
            case 6552:  // Pummel
            case 1766:  // kick
            case 47528: // Mind Freeze
            case 96231: // Rebuke
            case 93985: // Skull Bash
            case 93983: // Skull Bash
            case 80964: // Skull Bash
            case 80965: // Skull Bash
            case 2139:  // Counterspell
            case 57994: // Wind Shear
            case 97547: // Solar Beam
                spellInfo->AttributesEx3 |= SPELL_ATTR3_IGNORE_HIT_RESULT;
                break;
            case 77760:
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 93019:
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 93020:
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 75790: // Rampant Growth
            case 89888:
                spellInfo->Effects[EFFECT_0].Effect = 0;
                break;
            case 63093:
                spellInfo->Effects[EFFECT_0].Effect = SPELL_EFFECT_APPLY_AURA;
                spellInfo->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_DUMMY;
                break;
            case 83077: // improved serpent sting - shouldn't benefit from talents
                spellInfo->SpellFamilyFlags[0] = 0x00000000;
                break;
            case 75548: // Quicksand
                spellInfo->Effects[EFFECT_1].Effect = 0;
                break;
            case 84738: // Celestial Focus
                spellInfo->Effects[EFFECT_0].SpellClassMask = flag96(0x00400205, 0x01000020, 0x02008000);
                break;
            case 56070:
                spellInfo->Effects[EFFECT_0].MiscValueB = 367;
                break;
            case 94984: // Dark Simulacrum
            case 77616: // Dark Simulacrum
                spellInfo->AttributesEx3 |= SPELL_ATTR3_IGNORE_HIT_RESULT;
                break;
            case 45464:
                spellInfo->CastTimeEntry = sSpellCastTimesStore.LookupEntry(7);
                break;
            case 95564:
            case 92851:
                spellInfo->AttributesEx3 |= SPELL_ATTR3_UNINTERRUPTIBLE;
                break;
            case 82631:
            case 92512:
            case 92513:
            case 92514:
                spellInfo->InterruptFlags = 0;
                break;
            case 86199:
            case 92868:
            case 92869:
            case 92870:
                spellInfo->Effects[EFFECT_1].TriggerSpell = 0;
                break;
            case 81748: // Alliance RBG faction buff
                spellInfo->SpellIconID = 3375;
                spellInfo->Effects[EFFECT_0].Effect = SPELL_EFFECT_APPLY_AURA;
                spellInfo->Effects[EFFECT_1].Effect = SPELL_EFFECT_APPLY_AURA;
                spellInfo->Effects[EFFECT_2].Effect = SPELL_EFFECT_APPLY_AURA;
                break;
            case 81744: // Horde RBG faction buff
                spellInfo->SpellIconID = 3374;
                spellInfo->Effects[EFFECT_0].Effect = SPELL_EFFECT_APPLY_AURA;
                spellInfo->Effects[EFFECT_1].Effect = SPELL_EFFECT_APPLY_AURA;
                spellInfo->Effects[EFFECT_2].Effect = SPELL_EFFECT_APPLY_AURA;
                break;
            case 90944: // Devouring Flames
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_UNIT_TARGET_ENEMY;
                spellInfo->Effects[EFFECT_0].TargetB = 0;
                break;
            case 75245: // Burning Shadowbolt
            case 90915: // Burning Shadowbolt
            case 74944: // Flaming Arrow
            case 90810: // Flaming Arrow
            case 82637: // Plague of Ages
            case 82640: // Plague of Ages
            case 89996: // Plague of Ages
            case 89995: // Plague of Ages
            case 87880: // Violent Gale Storm
            case 93356: // Violent Gale Storm
            case 75520: // Twilight Corruption
            case 91049: // Twilight Corruption
            case 76194: // Twilight Blast
            case 91042: // Twilight Blast
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 89421: // Wrack
            case 92955: // Wrack
            case 89435: // Wrack
            case 92956: // Wrack
                spellInfo->AttributesEx4 |= SPELL_ATTR4_IGNORE_RESISTANCES;
                spellInfo->AttributesEx5 |= SPELL_ATTR5_START_PERIODIC_AT_APPLY;
                break;
            case 98021: // Spirit Link totem
                spellInfo->AttributesEx4 |= SPELL_ATTR4_IGNORE_RESISTANCES;
                break;
            case 77917:
            case 77941:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_15_YARDS);
                break;
            case 85422:
                spellInfo->AttributesEx3 |= SPELL_ATTR3_UNINTERRUPTIBLE;
                break;
            case 81007:
            case 94085:
            case 94086:
            case 94087:
            case 79492:
                spellInfo->AttributesEx2 |= SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS;
                break;
            case 87023: // Cauterize
                spellInfo->AttributesEx10 |= SPELL_ATTR10_UNK1;
                break;
            case 47632: // death coil damage trigger
                spellInfo->FacingCasterFlags &= ~SPELL_FACING_FLAG_INFRONT;
                break;
            case 19621:
            case 19622:
            case 19623:
                spellInfo->AttributesEx &= ~SPELL_ATTR0_HIDDEN_CLIENTSIDE;
                break;
            case 88453:
                spellInfo->Effects[EFFECT_0].TargetB = 0;
                spellInfo->Effects[EFFECT_1].TargetB = 0;
                break;
            case 58642:
                spellInfo->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_DUMMY;
                break;
            case 14751: // Chakra - remove prayer of mending proc - has extra handling
                spellInfo->Effects[EFFECT_1].SpellClassMask = flag96(0x00000200, 0x00000000, 0x00000000);
                break;
            case 8178:
                spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
                break;
            case 75: // Auto Shot should always hit (tested on retail with level 80 character on level 88 dummy)
                spellInfo->AttributesEx3 |= SPELL_ATTR3_IGNORE_HIT_RESULT;
                break;
            case 89247:
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_INFINITE);
                break;
            case 89250:
            case 91301:
            case 91302:
            case 43302:
            case 97492:
            case 97500:
            case 97499:
            case 97930:
                spellInfo->Effects[EFFECT_0].MiscValueB = 367;
                break;
            case 87897:
            case 87900:
                spellInfo->ProcChance = 30;
                break;
            case 88295:
                spellInfo->Speed = 40;
                break;
            case 88265:
            case 88264:
                spellInfo->AttributesEx2 |= SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS;
                break;
            case 75821: // Manifested Nightmare
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_15_YARDS);
                break;
            case 93120:
                spellInfo->StackAmount = 25;
                spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.20f);
                break;
            case 93121:
            case 93122:
                spellInfo->StackAmount = 25;
                break;
            case 96521:
            case 97089:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_2_YARDS);
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_2_YARDS);
                break;
            case 96724:
                spellInfo->AttributesEx3 |= SPELL_ATTR3_IGNORE_HIT_RESULT;
                break;
            /////////////////////
            // Firelands Spells//
            /////////////////////
            case 101140:
                spellInfo->Effects[EFFECT_2].TargetA = TARGET_UNIT_TARGET_ANY;
                break;
            case 98333:
            case 100385:
                spellInfo->MaxAffectedTargets = 8;
                break;
            case 98496:
            case 100386:
                spellInfo->MaxAffectedTargets = 20;
                break;
            case 99256: // Torment
            case 100230: //Torment
            case 100231: //Torment
            case 100232: //Torment
                spellInfo->Attributes |= SPELL_ATTR0_NEGATIVE_1;
                break;
            case 98535:  // Leaping Flames
            case 100206: // Leaping Flames
            case 100207: // Leaping Flames
            case 100208: // Leaping Flames
                spellInfo->Attributes |= SPELL_ATTR0_NEGATIVE_1;
                break;
            case 100048: // Fiery Web Silk
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_INFINITE);
                break;
            case 99947: // Face Rage
                spellInfo->InterruptFlags = 0;
                spellInfo->ChannelInterruptFlags = 0;
                break;
            case 99557:
                spellInfo->InterruptFlags = 0;
                spellInfo->ChannelInterruptFlags = 0;
                spellInfo->AuraInterruptFlags = 0;
                break;
            case 98471: // Burning Acid
            case 100826:
            case 100827:
            case 100828:
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 98474:  // Flame Scythe
            case 100212: // Flame Scythe
            case 100213: // Flame Scythe
            case 100214: // Flame Scythe
                spellInfo->Effects[EFFECT_1].TargetA = TARGET_UNIT_CASTER;
                spellInfo->Mechanic = MECHANIC_DISTRACT;
                break;
            case 97225: // Magma Flow
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_15_SECONDS);
                break;
            case 100158: // Molten Power
            case 100302: // Molten Power
                spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
                break;
            case 100107: // Ragnaros magma trap eruption 10h
                spellInfo->Effects[EFFECT_0].BasePoints = 55102;
                break;
            case 100108: // Ragnaros magma trap eruption 25h
                spellInfo->Effects[EFFECT_0].BasePoints = 65000;
                break;
            ////////////////////////////
            // End of Firelands Spells//
            ////////////////////////////
            ////////////////////////////
            // Dragon  Soul spells//////
            ////////////////////////////
            case 103821: // Morchok - Earthen Vortex 10
            case 110047: // Morchok - Earthen Vortex 25
            case 110046: // Morchok - Earthen Vortex 10H
            case 110045: // Morchok - Earthen Vortex 25H
                spellInfo->Effects[EFFECT_1].TargetA = TARGET_DEST_CASTER;
                spellInfo->Effects[EFFECT_1].TargetB = TARGET_UNIT_DEST_AREA_ENEMY;
                break;
            case 103414: // Morchok - Stomp 10
            case 108571: // Morchok - Stomp 25
            case 109033: // Morchok - Stomp 10H
            case 109034: // Morchok - Stomp 25H
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_SHARE_DAMAGE;
                break;
            case 103534: // Morchok - Danger
            case 103536: // Morchok - Warning
            case 103541: // Morchok - Safe
                spellInfo->Attributes |= SPELL_ATTR0_NEGATIVE_1;
                break;
            case 105287: // ice lance should not have a recovery time
                spellInfo->RecoveryTime = 0;
                break;
            case 109325: // Frostflake
                spellInfo->Effects[EFFECT_0].TargetB = TARGET_UNIT_SRC_AREA_ENEMY;
                spellInfo->Effects[EFFECT_1].TargetB = TARGET_UNIT_SRC_AREA_ENEMY;
                break;
            case 105420: // Color Combination
            case 105435: // Color Combination
            case 105436: // Color Combination
            case 105437: // Color Combination
            case 105439: // Color Combination
            case 105440: // Color Combination
                spellInfo->AttributesEx9 &= ~SPELL_ATTR9_SPECIAL_DELAY_CALCULATION;
                spellInfo->Speed = 15.0f;
                break;
            case 110166: // Ping
                spellInfo->AttributesEx3 |= SPELL_ATTR3_NO_INITIAL_AGGRO;
                break;
            case 103434: // Disrupting Shadows
            case 104600: // Disrupting Shadows
                spellInfo->MaxAffectedTargets = 3;
                break;
            case 104599: // Disrupting Shadows
            case 104601: // Disrupting Shadows
                spellInfo->MaxAffectedTargets = 9;
                break;
            case 106375: // Twilight Instability
            case 109182: // Twilight Instability
            case 109183: // Twilight Instability
            case 109184: // Twilight Instability
            case 106401: // Twilight Onslaught
            case 108862: // Twilight Onslaught
            case 109226: // Twilight Onslaught
            case 109227: // Twilight Onslaught
            case 107439: // Twilight Barrage
            case 109203: // Twilight Barrage
            case 109204: // Twilight Barrage
            case 109205: // Twilight Barrage
            case 103527: // Void Diffusion
            case 104605: // Void Diffusion
            case 108345: // Void Diffusion
            case 108346: // Void Diffusion
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_SHARE_DAMAGE;
                break;
            case 108531:
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_INFINITE);
                break;
            case 108038: // Harpoon
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_UNIT_CASTER;
                spellInfo->Effects[EFFECT_0].TargetB = TARGET_UNIT_TARGET_ANY;
                break;
            case 109445:
            case 109470:
                spellInfo->Effects[EFFECT_0].TargetB = TARGET_UNIT_DEST_AREA_ALLY;
                break;
            case 106371: // Hour of Twilight
            case 109415: // Hour of Twilight
            case 109416: // Hour of Twilight
            case 109417: // Hour of Twilight - we exchange effect 0 and effect 1 because effect 0 is aura remove but we need this aura for the effect 1 triggerspell targetfilter
                spellInfo->Effects[EFFECT_0].BasePoints = 103327;
                spellInfo->Effects[EFFECT_0].TriggerSpell = 103327;
                spellInfo->Effects[EFFECT_1].BasePoints = 106174;
                spellInfo->Effects[EFFECT_1].TriggerSpell = 106174;
                break;
            case 106368: // Twilight Shift
            case 105554: // Heroic Will
                spellInfo->AttributesEx3 |= SPELL_ATTR3_DEATH_PERSISTENT;
                spellInfo->SchoolMask = SPELL_SCHOOL_MASK_NORMAL;
                break;
            case 108046: // Shockwave
                spellInfo->Effects[EFFECT_0].Effect = 0;
                break;
            case 105479: // Searing Plasma
            case 109362:
            case 109363:
            case 109364:
                spellInfo->MaxAffectedTargets = 1;
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE;
                break;
            case 106794: // Shrapnel
            case 110141:
            case 110142:
            case 110143:
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE;
                break;
            case 106444: // Impale
            case 109631:
            case 109632:
            case 109633:
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE;
                spellInfo->Mechanic = MECHANIC_BLEED;
                break;
            case 107018: // Assault Aspects
                spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo(TARGET_UNIT_SRC_AREA_ENTRY);
                spellInfo->AttributesEx3 = 0;
                break;
            case 106548: // Agonizing Pain
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
                break;
            case 106663: // Carrying Winds
            case 106668: // Carrying Winds
            case 106670: // Carrying Winds
            case 106672: // Carrying Winds
            case 106674: // Carrying Winds
            case 106676: // Carrying Winds
                spellInfo->Effects[EFFECT_0].MiscValue = 15;
                spellInfo->ExcludeCasterAuraSpell = 0;
                break;
            case 106666: // Carrying Winds
            case 106669: // Carrying Winds
            case 106671: // Carrying Winds
            case 106673: // Carrying Winds
            case 106675: // Carrying Winds
            case 106677: // Carrying Winds
                spellInfo->ExcludeTargetAuraSpell = 0;
                break;
            case 105465: // Lightning Storm
                spellInfo->MaxAffectedTargets = 3;
                break;
            case 108568: // Lightning Storm
                spellInfo->MaxAffectedTargets = 7;
                break;
            case 106385:
            case 109628:
            case 109629:
            case 109630:
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CONE_ENEMY_24);
                break;
            case 105069: // Seething Hate
            case 108094:
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_SHARE_DAMAGE;
                break;
            case 105777: // Roll control
                spellInfo->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_DUMMY;
                break;
            case 108220:
                spellInfo->Attributes |= SPELL_ATTR0_NEGATIVE_1;
                spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
                break;
            case 105490: // Fiery Grip - 10er
            case 109458:
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 109457: // Fiery Grip - 25er
            case 109459:
                spellInfo->MaxAffectedTargets = 3;
                break;
            case 105937: // Regenerative Blood
            case 110208:
            case 110209:
            case 110210:
                spellInfo->Effects[EFFECT_0].TargetB = 0;
                break;
            case 106040: // Spellweaving
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_UNIT_CASTER;
                break;
            case 107835:
            case 109847:
            case 109849:
                spellInfo->AttributesEx3 |= SPELL_ATTR3_NO_DONE_BONUS;
                break;
            case 109089: // Congealing Blood
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_DEST_DEST_RANDOM;
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_5_YARDS);
                break;
            case 110899: // Focused Assault
            case 110898: // Focused Assault
            case 105256: // Frozen Tempest
            case 109552: // Frozen Tempest
            case 109553: // Frozen Tempest
            case 109554: // Frozen Tempest
            case 105409: // Water Shield
            case 109560: // Water Shield
            case 109561: // Water Shield
            case 109562: // Water Shield
                spellInfo->InterruptFlags = 0;
                spellInfo->ChannelInterruptFlags = 0;
                break;
            case 110215: // Consuming Shroud
                spellInfo->AttributesEx |= SPELL_ATTR0_CASTABLE_WHILE_DEAD;
                break;
            case 106199: // Blood Corruption: Death
            case 106200:
                spellInfo->Effects[EFFECT_0].TargetB = TARGET_UNIT_SRC_AREA_ENTRY;
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE;
                break;
            //////////////////////////////
            // End of Dragon Soul spells//
            //////////////////////////////
            //////////////////////////////
            //Start DS 5 Player Dungeons//
            //////////////////////////////
            case 101840:
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE;
                break;
            case 101547:
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_DEST_DEST;
                spellInfo->Effects[EFFECT_0].RadiusEntry = 0;
                break;
            case 100865:
                spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
                break;
            case 101867:
                spellInfo->AttributesEx4 |= SPELL_ATTR4_FIXED_DAMAGE;
                break;
            case 103597: // Throw Knife
                spellInfo->Effects[EFFECT_1].TargetA = TARGET_UNIT_TARGET_ANY;
                break;
            case 108465: // Night Elf Illusion
            case 108466: // Night Elf Illusion
                spellInfo->AttributesEx3 |= SPELL_ATTR3_DEATH_PERSISTENT;
                break;
            //////////////////////////////
            // End DS 5 Player Dungeons //
            //////////////////////////////
            case 96319:
            case 96316:
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_DEST_NEARBY_ENTRY;
                break;
            case 96470:
                spellInfo->MaxAffectedTargets = 4;
                break;
            case 97152:
            case 97158:
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 96658:
                spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo(TARGET_UNIT_SRC_AREA_ENEMY);
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50000_YARDS);
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50000_YARDS);
                spellInfo->MaxAffectedTargets = 5;
                break;
            case 62789:
                spellInfo->Effects[EFFECT_0].TriggerSpell = 62790;
                break;
            case 62462:
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_10_SECONDS);
            case 24131:
                spellInfo->Attributes |= SPELL_ATTR0_NEGATIVE_1;
                break;
            case 42471:
                spellInfo->MaxAffectedTargets = 3;
                break;
            case 42621:
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_6_SECONDS);
                break;
            case 97505:
                spellInfo->AttributesEx |= SPELL_ATTR1_NO_THREAT;
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_6_YARDS);
                spellInfo->Effects[EFFECT_1].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_6_YARDS);
                break;
            case 98893:
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_0_YARDS);
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_0_YARDS);
                break;
            case 61603:
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_10_YARDS);
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_10_YARDS);
                break;
            case 99262:
                spellInfo->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_DUMMY;
                spellInfo->StackAmount = 255;
                break;
                // Vital Flame
            case 99263:
                spellInfo->Effects[EFFECT_0].BasePoints = 5;
                break;
            // Countdown
            case 99519:
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(31); // 8 seconds
                break;
            case 96682:
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 96859:
            case 96860:
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 86013:
            case 92859:
            case 92860:
            case 92861:
                spellInfo->InterruptFlags = 0;
                break;
            case 99605:
            case 101658:
            case 101659:
            case 101660:
                spellInfo->Attributes |= SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY;
                break;
            case 99476:
                spellInfo->Effects[EFFECT_0].TriggerSpell = 99506;
                break;
            case 44735:
            case 42357:
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 42402:
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE_EFF2;
                break;
            case 96335:
            case 96434:
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_15_SECONDS);
                break;
            case 97001:
            case 97352:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_10_YARDS);
                break;
            case 96684:
                spellInfo->Effects[EFFECT_1].MiscValue = 20;
                spellInfo->Effects[EFFECT_1].MiscValueB = 10;
                break;
            case 99921:
            case 99922:
                spellInfo->Effects[EFFECT_0].MiscValue = 3;
                break;
            case 92958:
            case 92959:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_5_YARDS);
                break;
            case 18735:
                spellInfo->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT;
                break;
            case 98136:
            case 100392:
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_40_SECONDS);
                break;
            case 98649:
            case 101646:
            case 101647:
            case 101648:
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_UNIT_TARGET_ENEMY;
                spellInfo->Effects[EFFECT_0].TargetB = 0;
                spellInfo->Effects[EFFECT_1].TargetA = TARGET_UNIT_TARGET_ENEMY;
                break;
            case 98552:
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_INFINITE);
                break;
            case 98980:
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_UNIT_TARGET_ANY;
                spellInfo->Effects[EFFECT_1].TargetA = TARGET_UNIT_TARGET_ANY;
                break;
            case 99919: // ignition
                spellInfo->Effects[EFFECT_0].MiscValue = 3;
                break;
            case 97549:
                spellInfo->Speed = 30.0f;
                spellInfo->Effects[0].Amplitude = 2000;
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_10_SECONDS);
                break;
            case 89091: // Volcanic Destruction
            case 45055: // Shadow Bolt
            case 23687: // Lightning Strike
            case 34587: // Romulo's Poison
            case 55039: // Gnomish Lightning Generator
            case 54092: // Monster Slayer's Kit
            case 27655: // Flame Lash
            case 14537: // Six Demon Bag
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_100_YARDS);
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_100_YARDS);
                break;
           case 78068: // Magma Spit 10nh
               spellInfo->MaxAffectedTargets = 3;
               break;
           case 91917: // Magma Spit 25hc
               spellInfo->MaxAffectedTargets = 8;
               break;
           case 77679: // SPELL_SCORCHING_BLAST 10NM
           case 92968: // SPELL_SCORCHING_BLAST 25NM
           case 92969:
               spellInfo->AttributesCu |= SPELL_ATTR0_CU_SHARE_DAMAGE;
               break;
           case 79710:
           case 91540:
           case 91541:
           case 91542:
               spellInfo->MaxAffectedTargets = 1;
               break;
           case 79504:
           case 91535:
           case 91536:
           case 91537:
               spellInfo->AttributesEx2 |= SPELL_ATTR2_CANT_CRIT;
               break;
           case 83703:
           case 86166:
               spellInfo->CastTimeEntry = sSpellCastTimesStore.LookupEntry(2);
               break;
           case 86014:
           case 92863:
           case 86825:
           case 92879:
           case 92880:
           case 92881:
               spellInfo->AttributesCu |= SPELL_ATTR0_CU_SHARE_DAMAGE;
               break;
           case 91331:
           case 93206:
               spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_20_SECONDS);
               break;
           case 86307:
           case 101444:
               spellInfo->CastTimeEntry = sSpellCastTimesStore.LookupEntry(208);
               break;
           case 87904:
           case 101458:
               spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_20_SECONDS);
               break;
           case 87770:
           case 93261:
               spellInfo->InterruptFlags = 0;
               break;
           case 77987: // SPELL_GROW_CATALYST
           case 101440:
               spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
               break;
           case 96873:
               spellInfo->Effects[EFFECT_0].TargetA = TARGET_UNIT_CASTER;
               break;
           case 96920:
           case 101006:
               spellInfo->MaxAffectedTargets = 1;
               break;
           case 97202:
               spellInfo->MaxAffectedTargets = 1;
               spellInfo->InterruptFlags |= SPELL_INTERRUPT_FLAG_INTERRUPT;
               break;
           case 97151:
               spellInfo->AttributesEx3 |= SPELL_ATTR3_ONLY_TARGET_PLAYERS;
               spellInfo->AttributesEx3 &= ~SPELL_ATTR3_DEATH_PERSISTENT;
               break;
           case 70475:
               spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_10_YARDS);
               spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_10_YARDS);
               break;
           case 98557:
               spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_INFINITE);
               break;
           case 99875: // Fuse
               spellInfo->Effects[EFFECT_0].TargetA = TARGET_SRC_CASTER;
               spellInfo->Effects[EFFECT_0].TargetB = TARGET_UNIT_SRC_AREA_ENTRY;
               spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_100_YARDS);
               break;
           case 99526:
               spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo(TARGET_UNIT_TARGET_ENEMY);
               spellInfo->Effects[EFFECT_1].TargetB = SpellImplicitTargetInfo(TARGET_UNIT_TARGET_ENEMY);
               break;
           case 98619:
               spellInfo->AuraInterruptFlags = 0;
               break;
            case 68827:
                spellInfo->Effects[EFFECT_0].TriggerSpell = 67448;
                break;
            case 69515:
                spellInfo->Effects[EFFECT_0].TriggerSpell = 68829;
                break;
            case 68170:
                spellInfo->Effects[EFFECT_0].TriggerSpell = 67454;
                break;
            case 68169:
                spellInfo->Effects[EFFECT_0].TriggerSpell = 67456;
                break;
            case 69495:
                spellInfo->Effects[EFFECT_0].TriggerSpell = 67436;
                break;
            case 69502:
                spellInfo->Effects[EFFECT_1].TriggerSpell = 67815;
                break;
            case 68832:
                spellInfo->Effects[EFFECT_0].TriggerSpell = 67444;
                break;
            case 69505:
                spellInfo->Effects[EFFECT_0].TriggerSpell = 67459;
                break;
            case 68829:
                spellInfo->Effects[EFFECT_0].TriggerSpell = 67450;
                break;
            case 66551:
            case 46598:
                spellInfo->RangeEntry = sSpellRangeStore.LookupEntry(13); // 50000yd
                break;
            case 82691: // Ring of Frost
                spellInfo->AttributesEx2 |= SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS;
                break;
            case 72347: // Lock Players and Tap Chest
                spellInfo->AttributesEx3 &= ~SPELL_ATTR3_NO_INITIAL_AGGRO;
                break;
            case 73843: // Award Reputation - Boss Kill
            case 73844: // Award Reputation - Boss Kill
            case 73845: // Award Reputation - Boss Kill
            case 73846: // Award Reputation - Boss Kill
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50000_YARDS); // 50000yd
                break;
            case 70446:
                spellInfo->AttributesEx |= SPELL_ATTR0_CASTABLE_WHILE_DEAD;
                break;
            case 100823:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_2_YARDS);
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_2_YARDS);
                break;
            case 100476:
            case 100644:
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_25_YARDS);
                break;
            case 100941:
            case 100998:
            case 101110:
                spellInfo->Attributes |= SPELL_ATTR0_NEGATIVE_1;
                break;
            case 100604:
            case 100997:
                spellInfo->AttributesEx5 |= SPELL_ATTR5_USABLE_WHILE_STUNNED;
                break;
            case 99063: // Mage T12 2P Bonus proc should not interrupt mage casts...
                spellInfo->AttributesEx4 |= SPELL_ATTR4_TRIGGERED;
                break;
            case 99838:
            case 101208:
            case 101209:
            case 101210:
                spellInfo->AttributesEx |= SPELL_ATTR1_CANT_BE_REFLECTED;
                break;
            case 24907: // Moonkin Aura
                spellInfo->Attributes &= ~SPELL_ATTR0_PASSIVE;
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_INFINITE);
                break;
            case 30019:
                spellInfo->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_MOD_POSSESS;
                spellInfo->Effects[EFFECT_2].BasePoints = 99;
                spellInfo->Effects[EFFECT_2].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_200_YARDS);
                break;
            case 39350:
            case 39352:
            case 39353:
            case 39354:
            case 39355:
            case 39356:
            case 39357:
            case 39358:
            case 39359:
            case 39360:
            case 39361:
            case 39362:
                spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_TARGET_ANY);
                break;
            case 84748: // Bandit's Guile should stack for different casters - we use effect 2 as sinestra strike counter
                spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
                spellInfo->Effects[EFFECT_2].Effect = SPELL_EFFECT_APPLY_AURA;
                spellInfo->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_DUMMY;
                break;
            case 101200:
                spellInfo->Effects[EFFECT_0].MiscValueB = 367;
                break;
            case 101567:
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 101401:
                spellInfo->AttributesEx2 |= SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS;
                break;
            case 102242:
            case 102173:
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 101626:
                spellInfo->Effects[EFFECT_0].MiscValueB = 20;
                break;
            case 101810:
                spellInfo->MaxAffectedTargets = 3;
                break;
            case 101337:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_2_YARDS);
                break;
            case 97085:
            case 96755:
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_2_YARDS);
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_2_YARDS);
                break;
            case 108442:
                spellInfo->RangeEntry = sSpellRangeStore.LookupEntry(13); // 100 yards
                spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50000_YARDS);
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_50000_YARDS);
                break;
            case 32409:
                spellInfo->AttributesEx10 |= SPELL_ATTR10_UNK1;
                break;
            case 77024:
                spellInfo->CategoryRecoveryTime = 0;
                spellInfo->Effects[EFFECT_0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_15_YARDS);
                break;
            case 108374:
                spellInfo->Effects[EFFECT_0].MiscValueB = 367;
                break;
            case 109828:
            case 108022:
            case 109831:
            case 107821:
            case 109856:
            case 109858:
                spellInfo->AttributesEx3 |= SPELL_ATTR3_NO_DONE_BONUS;
                break;
            case 105737: // druid t13 4p
                spellInfo->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_PERIODIC_DUMMY;
                spellInfo->Effects[EFFECT_0].Amplitude = 1000;
                break;
            case 102214:
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 90337: // Bad Manner missing interrupt flag
                spellInfo->InterruptFlags |= SPELL_INTERRUPT_FLAG_INTERRUPT;
                break;
            case 95466: // Corrosive Spit(Special Ability) (Level 85)
                spellInfo->AttributesEx7 &= ~SPELL_ATTR7_DISPEL_CHARGES;
                break;
            case 108126:
                spellInfo->StartRecoveryCategory = 0;
                break;
            case 97779: // Lashtail Hatchling
                spellInfo->Effects[EFFECT_0].MiscValueB = 41;
                break;
                // well of eternity start
            case 103004: // shadowcloak
                spellInfo->Effects[EFFECT_1].Amplitude = 1000;
                break;
            case 105247:
                spellInfo->AttributesEx5 |= SPELL_ATTR5_START_PERIODIC_AT_APPLY;
                break;
            case 105074:
            case 105004:
                spellInfo->Effects[EFFECT_0].TargetA = TARGET_SRC_CASTER;
                spellInfo->Effects[EFFECT_0].TargetB = TARGET_UNIT_SRC_AREA_ENTRY;
                break;
            case 105544:
                spellInfo->Attributes |= SPELL_ATTR0_NEGATIVE_1;
                break;
            case 103241:
                spellInfo->Effects[EFFECT_0].MiscValue = 14;
                break;
            case 105054:
            case 105058:
            case 105059:
                spellInfo->Effects[0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_1_YARD);
                break;
            case 102453:
            case 102454:
                spellInfo->Effects[0].RadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_1_YARD);
                spellInfo->Effects[0].MaxRadiusEntry = sSpellRadiusStore.LookupEntry(EFFECT_RADIUS_1_YARD);
                break;
            case 103530:
                spellInfo->MaxAffectedTargets = 1;
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_NO_INITIAL_THREAT;
                break;
                // well of eternity end
                // hour of twilight
            case 102475:
                spellInfo->AttributesEx2 |= SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS;
                break;
                // hour of twilight end
            //Unbanish Azaloth should remove aura through immunity
            case 37834:
                spellInfo->Attributes |= SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY;
                break;
            default:
                break;
        }

		switch (spellInfo->Id)
		{
		case 19386: // Wyvern Sting
		case 2637: // Hibernate
		case 339: // Entangling Roots
		case 335: // Freezing Trap
		case 118: // Polymorph
		case 28271: // Polymorph
		case 28272: // Polymorph
		case 61721: // Polymorph 
		case 61305: // Polymorph
		case 20066: // Repentance
		case 9484: // Shackle Undead
		case 2094: // Blind
		case 51514: // Hex
		case 76780: // Shackle Elemental
		case 710: // Banish
		case 6358: // Seduction
			spellInfo->AttributesEx3 |= SPELL_ATTR3_NO_INITIAL_AGGRO;
			break;

		default:
			break;
		}

        switch (spellInfo->SpellFamilyName)
        {
            case SPELLFAMILY_PALADIN:
                // Seals of the Pure should affect Seal of Righteousness
                if (spellInfo->SpellIconID == 25 && spellInfo->Attributes & SPELL_ATTR0_PASSIVE)
                    spellInfo->Effects[EFFECT_0].SpellClassMask[1] |= 0x20000000;
                break;
            case SPELLFAMILY_DEATHKNIGHT:
                // Icy Touch - extend FamilyFlags (unused value) for Sigil of the Frozen Conscience to use
                if (spellInfo->SpellIconID == 2721 && spellInfo->SpellFamilyFlags[0] & 0x2)
                    spellInfo->SpellFamilyFlags[0] |= 0x40;
                break;
        }
    }

    SummonPropertiesEntry* properties = const_cast<SummonPropertiesEntry*>(sSummonPropertiesStore.LookupEntry(121));
    properties->Type = SUMMON_TYPE_TOTEM;
    properties = const_cast<SummonPropertiesEntry*>(sSummonPropertiesStore.LookupEntry(647)); // 52893
    properties->Type = SUMMON_TYPE_TOTEM;

    TC_LOG_INFO("server.loading", ">> Loaded SpellInfo corrections in %u ms", GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadBwDTfWBoTNerfCorrection(SpellInfo *spellInfo)
{
    switch (spellInfo->Id)
    {
        /* nefarian nerf*/
        // patch 4.1
        case 78621: // path 4.1 10 man only
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.38;
            break;
        case 94121:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.25;
            break;
        case 94122:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.11;
            break;
        case 77827:
            spellInfo->Effects[EFFECT_0].BasePoints *= 2.49;
            break;
        case 94128:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.99;
            break;
        case 94129:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.24;
            break;
            // patch 4.2
        case 81272:
        case 94088:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.42;
            break;
        case 81118:
        case 94073:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.75f);
            break;
        case 80734: // heroic is already at 1.5 sec i found no information on other spells
        case 101430:
            spellInfo->CastTimeEntry = sSpellCastTimesStore.LookupEntry(16); // 1.5 second
            break;
        case 78679:
            spellInfo->Effects[EFFECT_0].Amplitude = 4000;
            break;
        case 81077:
            spellInfo->Effects[EFFECT_0].Amplitude = 2000;
            break;
        case 101433:
            spellInfo->Effects[EFFECT_0].Amplitude = 4000;
            break;
        case 101434:
            spellInfo->Effects[EFFECT_0].Amplitude = 2000;
            break;
            // end with nerf nefarian spells
            // ATRAMEDES PRE NERF SPELLS
        case 78023:
        case 92483:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.49;
            break;
        case 77612:
        case 92451:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.25;
            break;
        case 77675:
        case 92417:
            spellInfo->Effects[EFFECT_0].BasePoints = 5;
            break;
            // end with nerf atramedes spells
            // MAGMAW PRE NERF SPELLS
        case 77690:
        case 91919:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.25;
            break;
        case 91931:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.33;
            break;
        case 91932:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.15;
            break;
        case 88287:
        case 91914:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.25;
            break;
        case 78937:
        case 91915:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.25;
            break;
        case 78068: // Magma Spit 10nh
            spellInfo->MaxAffectedTargets = 3;
            break;
        case 91917: // Magma Spit 25hc
            spellInfo->MaxAffectedTargets = 8;
            break;
        case 78362:
            spellInfo->Effects[EFFECT_0].BasePoints *= 5;
            break;
        case 77896:
            spellInfo->CastTimeEntry = 0;
            break;
                // end with laglaw pre nerf spells
                // Maloriak pre nef spells.
        case 77908:
        case 92961:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.25;
            break;
        case 77679: // SPELL_SCORCHING_BLAST 10NM
        case 92968: // SPELL_SCORCHING_BLAST 25NM
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.20f);
            break;
        case 92969:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.17;
            break;
        case 77715:
        case 95655:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.20f);
            spellInfo->Effects[EFFECT_1].BasePoints /= (1.0f - 0.20f);
            break;
        case 77699:
        case 92978:
            spellInfo->Effects[EFFECT_1].BasePoints /= (1.0f - 0.20f);
            break;
        case 77987: // SPELL_GROW_CATALYST
        case 101440:
            spellInfo->Effects[EFFECT_0].BasePoints = 20;
            break;
                // end with maliorak pre nerf spell.
                // OMNOTRON PRE NERF SPELLS
        case 91466:
        case 91467:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.22;
            break;
        case 91524:
        case 91525:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.60;
            break;
        case 79879:
        case 91465:
        case 79889:
        case 91438:
        case 79035:
        case 91523:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.20f);
            break;
        case 79710:
        case 91540:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.20f);
            spellInfo->MaxAffectedTargets = 1;
            break;
        case 91541:
        case 91542:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.31;
            spellInfo->MaxAffectedTargets = 1;
            break;
        case 79582:
        case 91516:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f + 1.0f);
            break;
        case 79504:
        case 91535:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.40f);
            break;
        case 91536:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.22;
            break;
        case 91537:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.15;
            break;
        case 91457:
        case 91458:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.33;
            break;
        case 79912:
        case 91456:
        case 80092:
        case 91498:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.50f);
            break;
                // end with omnotron pre nerf spells

                // nerf halfus
        case 83721:
        case 86151:
        case 83734:
        case 86154:
        case 83710:
        case 86169:
        case 83855:
        case 86163:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.20f);
            break;
        case 83703:
        case 86166:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.20f);
            spellInfo->CastTimeEntry = sSpellCastTimesStore.LookupEntry(2);
            break;
                // end with halfus pre nerf
                // nerf valio and theralion
        case 92873:
        case 92874:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.22f;
            break;
        case 92883:
        case 92884:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.375f;
            break;
        case 92904:
        case 92905:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.4285f;
            break;
        case 86844:
        case 92872:
        case 86305:
        case 92882:
        case 86371:
        case 92903:
        case 86505:
        case 92907:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.20f);
            break;
        case 92886:
        case 92887:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.50f;
            break;
        case 86214:
        case 92885:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.20f);
            break;
        case 92864:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.27f;
            break;
        case 92865:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.2f;
            break;
        case 86014:
        case 92863:
        case 86825:
        case 92879:
        case 92880:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.20f);
            break;
        case 92881:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.3f;
            break;
                // end with valio and theralion nerf
                // nerf ascendant council
        case 83282:
        case 92448:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.35f);
            break;
        case 82772:
        case 92503:
            spellInfo->Effects[EFFECT_1].BasePoints /= (1.0f - 0.625f);
            spellInfo->ProcFlags = 0;
            break;
        case 84529:
        case 92480:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.375f);
            break;
        case 82639:
        case 101415:
            spellInfo->Effects[EFFECT_0].BasePoints = 5;
            break;
        case 83718:
        case 92541:
        case 92542:
        case 92543:
            spellInfo->Effects[EFFECT_0].BasePoints = 100;
            break;
                // end with asc council nerf
                // pre nerf chogall
        case 81713:
        case 93175:
        case 81689:
        case 93184:
        case 81527:
        case 93226:
        case 81571:
        case 93221:
        case 82411:
        case 93132:
        case 93133:
        case 93134:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.20f);
            break;
        case 91331:
        case 93206:
            spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_20_SECONDS);
            break;
                //end with pre nerf chogall.
                // nerf t4v
                // nerf conclave of wind
        case 86282:
        case 86367:
        case 93135:
        case 86487:
        case 93144:
        case 86081:
        case 93126:
        case 86182:
        case 93056:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.20f);
            break;
        case 85483:
        case 93138:
            spellInfo->Effects[EFFECT_1].BasePoints /= (1.0f - 0.20f);
            break;
        case 86307:
        case 101444:
            spellInfo->CastTimeEntry = sSpellCastTimesStore.LookupEntry(208);
            break;
        case 86111:
        case 93129:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.20f);
            spellInfo->Effects[EFFECT_1].BasePoints = -10;
            spellInfo->StackAmount = 30;
            break;
        case 84645:
        case 93123:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.20f);
            spellInfo->Effects[EFFECT_1].BasePoints = 10;
            break;
               // end with nerf conclave of wind
               // nerf alakir
        case 88427:
            spellInfo->Effects[EFFECT_0].Amplitude = 500;
            break;
        case 87904:
        case 101458:
            spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_20_SECONDS);
            break;
        case 87770:
        case 93261:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.20f);
            spellInfo->InterruptFlags = 0;
            break;
        case 89690:
        case 101468:
            spellInfo->Effects[EFFECT_0].Amplitude = 15000;
            break;
        case 95764:
        case 95780:
        case 95781:
        case 95782:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.50f);
            break;
            // end with alakir nerf
            // end with t4v nerf
        }
}

void SpellMgr::LoadFirelandsNerfCorrection(SpellInfo *spellInfo)
{
    switch (spellInfo->Id)
    {
        // trashes
        // hell hound
        case 100057:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.25f);
            break;
        // Unstable Magma (not scrupted yet
        // The damage multiplier granted by energy to Unstable Magma has been reduced by 50%.
            // alysrazor
        case 98619:
            spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_20_SECONDS);
            break;
        case 99605:
        case 101658:
        case 101659:
        case 101660:
            spellInfo->Effects[EFFECT_0].BasePoints = 30000;
            break;
        case 100745:
        case 101664:
        case 101665:
        case 101666:
            spellInfo->Effects[EFFECT_0].BasePoints = 100000;
            break;
        case 98463:
            spellInfo->Effects[EFFECT_0].BasePoints = 100000;
            break;
        case 99794:
            spellInfo->Effects[EFFECT_0].BasePoints = 50000;
            break;
        case 101223:
        case 101294:
        case 101295:
        case 101296:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.5f);
            break;
        case 98885:
        case 100715:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.25f);
            break;
        case 100716:
        case 100717:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.15f);
            break;
        case 100024:
            spellInfo->Effects[EFFECT_0].BasePoints = 3000;
            break;
        case 100721:
            spellInfo->Effects[EFFECT_0].BasePoints = 4000;
            break;
        case 100722:
            spellInfo->Effects[EFFECT_0].BasePoints = 5000;
            break;
        case 99336:
            spellInfo->Effects[EFFECT_0].BasePoints = 40000;
            break;
        case 100725:
            spellInfo->Effects[EFFECT_0].BasePoints = 50000;
            break;
        case 100726:
            spellInfo->Effects[EFFECT_0].BasePoints = 70000;
            break;
        case 100727:
            spellInfo->Effects[EFFECT_0].BasePoints = 90000;
            break;
        case 99427:
            spellInfo->Effects[EFFECT_0].BasePoints = 50000;
            break;
        case 100729:
            spellInfo->Effects[EFFECT_0].BasePoints = 60000;
            break;
        case 100730:
            spellInfo->Effects[EFFECT_0].BasePoints = 70000;
            break;
        case 100731:
            spellInfo->Effects[EFFECT_0].BasePoints = 80000;
            break;
        case 99844:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.3f);
            break;
        case 101729:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.25f);
            break;
        case 101730:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.20f);
            break;
        case 101731:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.15f);
            break;
        case 99757:
        case 100739:
        case 100740:
        case 100741:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.25f);
            break;
        case 100640:
            spellInfo->Effects[EFFECT_0].BasePoints = 50000;
            break;
            // end alysrazor
            // Baleroc
        case 99351:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.30f);
            break;
        case 101000:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.25f);
            break;
        case 101001:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.15f);
            break;
        case 101002:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.15f);
            break;
        case 99352:
            spellInfo->Effects[EFFECT_0].BasePoints = -100;
            break;
        case 99405:
            spellInfo->Effects[EFFECT_0].BasePoints = 0;
            break;
        case 99256:
        case 100230:
            spellInfo->Effects[EFFECT_0].BasePoints = 4000;
            break;
        case 100231:
        case 100232:
            spellInfo->Effects[EFFECT_0].BasePoints = 5000;
            break;
        case 99257:
            spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_40_SECONDS);
            break;
        case 99402:
            spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_60_SECONDS);
            break;
            // end Baleroc
            // bethlac
        case 99333:
        case 101128:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.25f);
            break;
        case 101129:
        case 101130:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.20f);
            break;
        case 98471:
        case 100826:
        case 100827:
        case 100828:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.30f);
            break;
        case 99278:
        case 101133:
            spellInfo->Effects[EFFECT_1].BasePoints /= (1.0f - 0.25f);
            break;
            // end bethilac
            // rhyolith
        case 98837:
            spellInfo->Effects[EFFECT_0].BasePoints = 50;
            break;
        case 98598:
        case 100414:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.25f);
            break;
            // end rhyolith
            // majordomo
        case 97238:
            spellInfo->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_MOD_POWER_REGEN_PERCENT;
            spellInfo->Effects[EFFECT_1].BasePoints = 20;
            break;
        case 98535:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.25f);
            break;
        case 100206:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.30f);
            break;
        case 100207:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.15f);
            break;
        case 100208:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.15f);
            break;
        case 98474:
            spellInfo->Effects[EFFECT_0].BasePoints = 750000;
            break;
        case 100212:
            spellInfo->Effects[EFFECT_0].BasePoints = 2250000;
            break;
        case 100213:
            spellInfo->Effects[EFFECT_0].BasePoints = 900000;
            break;
        case 100214:
            spellInfo->Effects[EFFECT_0].BasePoints = 2700000;
            break;
        case 98620:
        case 100215:
            spellInfo->Effects[EFFECT_0].BasePoints = 60000;
            break;
        case 100216:
        case 100217:
            spellInfo->Effects[EFFECT_0].BasePoints = 75000;
            break;
        case 98584:
        case 100209:
            spellInfo->Effects[EFFECT_0].BasePoints = 5000;
            break;
        case 100210:
        case 100211:
            spellInfo->Effects[EFFECT_0].BasePoints = 9000;
            break;
        case 98934:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.31f;
            break;
        case 100648:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.18f;
            break;
        case 100834:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.19f;
            break;
        case 100835:
            spellInfo->Effects[EFFECT_0].BasePoints *= 1.07f;
            break;
            // end majordomo
            // shannox
        case 99937:
            spellInfo->Effects[EFFECT_0].BasePoints = 3000;
            break;
        case 101218:
            spellInfo->Effects[EFFECT_0].BasePoints = 6000;
            break;
        case 101219:
            spellInfo->Effects[EFFECT_0].BasePoints = 4000;
            break;
        case 101220:
            spellInfo->Effects[EFFECT_0].BasePoints = 8000;
            break;
        case 99840:
            spellInfo->Effects[EFFECT_1].BasePoints /= (1.0f - 0.15f);
            break;
        case 100167:
        case 101215:
        case 101216:
        case 101217:
            spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(DURATION_25_SECONDS);
            break;
        case 99838:
        case 101208:
        case 101209:
        case 101210:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.15f);
            spellInfo->Effects[EFFECT_1].BasePoints /= (1.0f - 0.15f);
            break;
        case 100495:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.15f);
            break;
        case 100002:
            spellInfo->Effects[EFFECT_1].BasePoints /= (1.0f - 0.15f);
            break;
            // end shannox
            // ragnaros
        case 99399:
        case 101238:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.25f);
            break;
        case 101239:
        case 101240:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.15f);
            break;
        case 98708:
        case 100256:
        case 100257:
        case 100258:
            spellInfo->Effects[EFFECT_0].BasePoints *= 5.0f;
            break;
        case 98928:
        case 100292:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.25f);
            spellInfo->Effects[EFFECT_1].BasePoints /= (1.0f - 0.25f);
            break;
        case 100293:
        case 100294:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.15f);
            spellInfo->Effects[EFFECT_1].BasePoints /= (1.0f - 0.15f);
            break;
        case 98175:
        case 100106:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.25f);
            break;
        case 100107:
        case 100108:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.15f);
            break;
        case 98237:
        case 100383:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.25f);
            break;
        case 100384:
        case 100387:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.15f);
            break;
        case 98263:
        case 100113:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.25f);
            spellInfo->Effects[EFFECT_1].MiscValue = 100;
            break;
        case 100114:
        case 100115:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.15f);
            break;
        case 99112:
        case 100259:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.25f);
            break;
        case 98498:
        case 100579:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.25f);
            break;
        case 100580:
        case 100581:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.15f);
            break;
        case 98518:
        case 100252:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.25f);
            break;
        case 100253:
        case 100254:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.15f);
            break;
        case 99144:
        case 100303:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.25f);
            break;
        case 100304:
        case 100305:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.15f);
            break;
        case 99287:
        case 100299:
            spellInfo->Effects[EFFECT_0].BasePoints *= 4.5f;
            break;
        case 100941:
        case 100998:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.15f);
            spellInfo->Effects[EFFECT_1].BasePoints = 4000;
            break;
        case 101234:
        case 101235:
            spellInfo->Effects[EFFECT_0].BasePoints /= (1.0f - 0.25f);
            break;
            // end ragnaros
        case 99529:
        case 99528:
            spellInfo->Effects[0].RadiusEntry = sSpellRadiusStore.LookupEntry(13);
            break;
        default:
            break;
    }
}

void SpellMgr::LoadDragonSoulNerfCorrection(SpellInfo *spellInfo)
{
    switch (spellInfo->Id)
    {
        // Spine of Deathwing
        case 106213: // Blood of Neltharion
            spellInfo->Effects[EFFECT_0].BasePoints = -10;
            spellInfo->StackAmount = 3;
            break;
        case 106005: // Degradation
            spellInfo->Effects[EFFECT_0].BasePoints = -6;
            break;
        default:
            break;
    }
}

void SpellMgr::LoadSpellNerfCorrections()
{
    //uint32 oldMSTime = getMSTime();

    SpellInfo* spellInfo = NULL;
    for (uint32 i = 0; i < mSpellInfoMap.size(); ++i)
    {
        spellInfo = mSpellInfoMap[i];
        if (!spellInfo)
            continue;
        if (sWorld->getBoolConfig(CONFIG_DISABLE_406_NERFS))
            LoadBwDTfWBoTNerfCorrection(spellInfo);
        if (sWorld->getBoolConfig(CONFIG_DISABLE_420_NERFS))
            LoadFirelandsNerfCorrection(spellInfo);
        if (sWorld->getBoolConfig(CONFIG_DISABLE_430_NERFS))
            LoadDragonSoulNerfCorrection(spellInfo);
    }
}

void SpellMgr::LoadSpellCategoryCooldown(uint32 spellId, int32 cooldown, uint32 category, int32 categoryCooldown)
{
    if (spellId > mSpellInfoMap.size())
        return;

    SpellInfo* spellInfo = mSpellInfoMap[spellId];
    if (!spellInfo)
        return;

    spellInfo->RecoveryTime = cooldown;
    spellInfo->Category = category;
    spellInfo->CategoryRecoveryTime = categoryCooldown;

    SpellCategoryStore::iterator i_scstore = sSpellCategoryStore.find(category);
    if (i_scstore != sSpellCategoryStore.end())
        i_scstore->second.insert(spellId);
}
