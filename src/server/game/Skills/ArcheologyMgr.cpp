#include "SharedDefines.h"
#include "ArcheologyMgr.h"
#include "DBCStores.h"
#include "WorldPacket.h"
#include "Common.h"
#include "DBCEnums.h"
#include "ObjectMgr.h"
#include "World.h"
#include "DatabaseEnv.h"
#include "CellImpl.h"
#include "GameEventMgr.h"
#include "GridNotifiersImpl.h"
#include "Language.h"
#include "Player.h"
#include "SpellMgr.h"
#include "DisableMgr.h"
#include "ScriptMgr.h"
#include "MapManager.h"
#include "Map.h"
#include "Chat.h"

void ArcheologyMgr::GenerateResearchDigSites()
{
    uint32 skill_now = m_player->GetSkillValue(SKILL_ARCHAEOLOGY);

    if (skill_now == 0)
        return;

    uint32 slot = 0;

    // Keep saved dig sites
    if (HasSavedDigSites() > 0)
    {
        for (uint8 i = 0; i < 15; i++)
        {
            uint32 site_1 = m_digSites[i];
            uint32 site_2 = m_digSites[i+1];
            uint32 value = (site_2 << 16) | (site_1);
            m_player->SetUInt32Value(PLAYER_FIELD_RESEARCH_SITE_1 + slot, value);
            ++i;
            ++slot;
        }
        return;
    }

    // Generate first time sites
    GenerateResearchDigSitesInMap(0, slot);         // Kalimidor
    GenerateResearchDigSitesInMap(1, slot);         // Eastern Kindom

    if (m_player->getLevel() >= 58)
        GenerateResearchDigSitesInMap(530, slot);   // Outland

    if (m_player->getLevel() >= 68)
        GenerateResearchDigSitesInMap(571, slot);   // Northrend
}

void ArcheologyMgr::GenerateResearchDigSitesInMap(const uint32 &map, uint32 &slot)
{
    uint32 tempPad = slot * 2;
    for (uint8 i = 0; i < 4; i++)
    {
        uint32 rsdg = GetNewRandomSite(map);
        uint32 site_now = m_player->GetUInt32Value(PLAYER_FIELD_RESEARCH_SITE_1 + slot) & 0xFFFF;

        if (site_now != 0)
        {
            m_player->SetUInt32Value(PLAYER_FIELD_RESEARCH_SITE_1 + slot, (rsdg << 16) | (site_now));
            slot++;
        }
         else
            m_player->SetUInt32Value(PLAYER_FIELD_RESEARCH_SITE_1 + slot, (site_now << 16) | (rsdg));
        m_digSites[i + tempPad] = rsdg;
    }
}

uint8 ArcheologyMgr::HasSavedDigSites()
{
    uint8 count = 0;
    for (uint8 i = 0; i < 16; ++i)
    {
        if (m_digSites[i] != 0)
            count++;
    }
    return count;
}

void ArcheologyMgr::SetActualDigSitePosition(bool force)
{
    if (force || (m_actualDigPos.m_positionX == 0.0f || m_actualDigPos.m_positionY == 0.0f))
    {
        for (uint8 i = 0; i < HasSavedDigSites(); i++)
            if (ResearchSiteInfo const* rsInf = sObjectMgr->GetResearchSiteInfo(m_digSites[i]))
                if (rsInf->state == DIGSITE_ENABLE)
                    if (m_player->GetMapId() == rsInf->map)
                        if (m_player->GetDistance2d(rsInf->center_x, rsInf->center_y) < rsInf->ray * 2.0f)
                        {
                            Position centerPos;
                            centerPos.m_positionX = rsInf->center_x;
                            centerPos.m_positionY = rsInf->center_y;
                            centerPos.m_positionZ = m_player->GetPositionZ();
                            m_player->GetRandomPoint(centerPos, rsInf->ray, m_actualDigPos);
                            m_currentDigSite = i;
                            return;
                        }

        TC_LOG_ERROR("misc", "ARCHAEOLOGY : digsite broken zone : %u", m_player->GetZoneId());
    }
}

void ArcheologyMgr::UpdateWrongSkillDBValues(uint32 skillReq)
{
    SQLTransaction trans = WorldDatabase.BeginTransaction();
    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_ARCHAEOLOGY_REASEARCHSITE_SKILL_REQ);
    // "UPDATE research_sites SET minSkill = %u where entry = %u"
    stmt->setUInt32(0, skillReq);
    stmt->setUInt32(1, m_digSites[m_currentDigSite]);
    trans->Append(stmt);
    WorldDatabase.CommitTransaction(trans);
}

void ArcheologyMgr::SpawnArchaeologyScope()
{
    uint32 skill_now = m_player->GetSkillValue(SKILL_ARCHAEOLOGY);
    float distance = m_player->GetDistance2d(m_actualDigPos.m_positionX, m_actualDigPos.m_positionY);
    uint32 telescopeEntry = 0;
    ResearchSiteInfo const* rsInf = sObjectMgr->GetResearchSiteInfo(m_digSites[m_currentDigSite]);
    if (!rsInf || (rsInf && distance > rsInf->ray * 2.0f))
    {
        SetActualDigSitePosition(true);
        distance = m_player->GetDistance2d(m_actualDigPos.m_positionX, m_actualDigPos.m_positionY);
    }
    if (distance > ARCHAEOLOGY_DIG_SITE_FAR_DIST)
        telescopeEntry = ARCHAEOLOGY_DIG_SITE_FAR_SURVEYBOT;
    else if (distance < ARCHAEOLOGY_DIG_SITE_FAR_DIST && distance > ARCHAEOLOGY_DIG_SITE_MED_DIST)
        telescopeEntry = ARCHAEOLOGY_DIG_SITE_MEDIUM_SURVEYBOT;
    else if (distance < ARCHAEOLOGY_DIG_SITE_MED_DIST && distance > ARCHAEOLOGY_DIG_SITE_CLOSE_DIST)
        telescopeEntry = ARCHAEOLOGY_DIG_SITE_CLOSE_SURVEYBOT;
    else if (distance < ARCHAEOLOGY_DIG_SITE_CLOSE_DIST)
        telescopeEntry = ARCHAEOLOGY_DIG_SITE_REWARD_PLAYER;

    if (telescopeEntry != ARCHAEOLOGY_DIG_SITE_REWARD_PLAYER)
    {
        GameObject* pGameObj = new GameObject;
        Position summonPos;
        m_player->GetClosePoint(summonPos.m_positionX, summonPos.m_positionY, summonPos.m_positionZ, DEFAULT_WORLD_OBJECT_SIZE);
        Map *map = m_player->GetMap();

        if (!pGameObj->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_GAMEOBJECT), telescopeEntry, map, m_player->GetPhaseMask(), summonPos.m_positionX, summonPos.m_positionY, summonPos.m_positionZ, summonPos.GetAngle(m_actualDigPos.GetPositionX(), m_actualDigPos.GetPositionY()), 0.0f, 0.0f, 0.0f, 0.0f, 0, GO_STATE_READY))
        {
            delete pGameObj;
            return;
        }
        pGameObj->SetRespawnTime(6000 / IN_MILLISECONDS);
        m_player->AddGameObject(pGameObj);
        map->AddToMap(pGameObj);
    }
    else
    {
        // Summon object
        std::list<uint32> treasureEntryList;
        switch (m_player->GetZoneId())
        {
            case 4709: // Souther Barrens
                treasureEntryList.push_back(ARCHAEOLOGY_DWARF_TREASURE);
                treasureEntryList.push_back(ARCHAEOLOGY_FOSSIL_TREASURE);
                treasureEntryList.push_back(ARCHAEOLOGY_NIGHTELF_TREASURE);
                break;
            case 4922: // Twilight Hightlands
            case 3:    // Badlands
            case 51:   // Searing gorge
            case 38:   // loch modan
                treasureEntryList.push_back(ARCHAEOLOGY_DWARF_TREASURE);
                break;
            case 267:  // Hillsbard Foothills
            case 46:   // Burning Steppes
            case 11:   // Wetlands
                treasureEntryList.push_back(ARCHAEOLOGY_DWARF_TREASURE);
                treasureEntryList.push_back(ARCHAEOLOGY_FOSSIL_TREASURE);
                break;
            case 45:   // Arathi Highlands
            case 47:   // Hinterlands
                treasureEntryList.push_back(ARCHAEOLOGY_DWARF_TREASURE);
                treasureEntryList.push_back(ARCHAEOLOGY_TROLL_TREASURE);
                break;
            case 28:   // Weastern plaguelands
            case 4:    // Blasted Lands
            case 44:   // Redrige Mountains
            case 15:   // Dustwallow Marsh
                treasureEntryList.push_back(ARCHAEOLOGY_FOSSIL_TREASURE);
                break;
            case 10:   // Duskwood
            case 405:  // Desolance
            case 406:  // Stonetalon Mountains
                treasureEntryList.push_back(ARCHAEOLOGY_FOSSIL_TREASURE);
                treasureEntryList.push_back(ARCHAEOLOGY_NIGHTELF_TREASURE);
                break;
            case 139:  // Eastern Plaguelands
                treasureEntryList.push_back(ARCHAEOLOGY_FOSSIL_TREASURE);
                treasureEntryList.push_back(ARCHAEOLOGY_NIGHTELF_TREASURE);
                treasureEntryList.push_back(ARCHAEOLOGY_TROLL_TREASURE);
                if (skill_now >= 375)
                    treasureEntryList.push_back(ARCHAEOLOGY_NERUBIAN_TREASURE);
                break;
            case 33:   // Norther Stranglethorn
            case 8:    // Swamp of Sorrows
            case 490:  // Un'Goro Crater
            case 440:  // Tanaris
                treasureEntryList.push_back(ARCHAEOLOGY_FOSSIL_TREASURE);
                treasureEntryList.push_back(ARCHAEOLOGY_TROLL_TREASURE);
                break;
            case 357:  // Feralas
            case 616:  // Mount Hyjal
            case 331:  // Ashenvale
            case 361:  // Felwood
            case 618:  // Winterspring
            case 16:   // Azshara
            case 148:  // Darkshore
            case 1377: // Silithus
            case 2817: // Crystalsong forest
                treasureEntryList.push_back(ARCHAEOLOGY_NIGHTELF_TREASURE);
                break;
            case 3537: // Borean Thundra
            case 65:   // Dragonblight
                treasureEntryList.push_back(ARCHAEOLOGY_NIGHTELF_TREASURE);
                if (skill_now >= 375)
                    treasureEntryList.push_back(ARCHAEOLOGY_NERUBIAN_TREASURE);
                break;
            case 5287: // Cape of Stranglethorn
                treasureEntryList.push_back(ARCHAEOLOGY_TROLL_TREASURE);
                break;
            case 66:   // Zul'Drak
                treasureEntryList.push_back(ARCHAEOLOGY_TROLL_TREASURE);
                if (skill_now >= 375)
                    treasureEntryList.push_back(ARCHAEOLOGY_NERUBIAN_TREASURE);
                break;
            case 394:  // Grizzly hills
                treasureEntryList.push_back(ARCHAEOLOGY_TROLL_TREASURE);
                if (skill_now >= 375)
                    treasureEntryList.push_back(ARCHAEOLOGY_VRYKUL_TREASURE);
                break;
            case 3520: // Shadowmoon valley
            case 3519: // Terokkar Forest
            case 3483: // Hellfire Peninsula
            case 3518: // Nagrand
                if (skill_now >= 300)
                {
                    treasureEntryList.push_back(ARCHAEOLOGY_DRAENEI_TREASURE);
                    treasureEntryList.push_back(ARCHAEOLOGY_ORC_TREASURE);
                }
                else
                    UpdateWrongSkillDBValues(300);
                break;
            case 3523: // Netherstorm
            case 3521: // Zangarmash
                if (skill_now >= 300)
                    treasureEntryList.push_back(ARCHAEOLOGY_DRAENEI_TREASURE);
                else
                    UpdateWrongSkillDBValues(300);
                break;
            case 210:  // Icecrown
                if (skill_now >= 375)
                {
                    treasureEntryList.push_back(ARCHAEOLOGY_NERUBIAN_TREASURE);
                    treasureEntryList.push_back(ARCHAEOLOGY_VRYKUL_TREASURE);
                }
                else
                    UpdateWrongSkillDBValues(375);
                break;
            case 495:  // Howling Fjord
            case 67:   // Storm Peaks
                if (skill_now >= 375)
                    treasureEntryList.push_back(ARCHAEOLOGY_VRYKUL_TREASURE);
                else
                    UpdateWrongSkillDBValues(375);
                break;
            case 5034: // Uldum
                if (skill_now >= 450)
                    treasureEntryList.push_back(ARCHAEOLOGY_TOLVIR_TREASURE);
                else
                    UpdateWrongSkillDBValues(450);
                break;
            default:
                treasureEntryList.push_back(ARCHAEOLOGY_FOSSIL_TREASURE);
                break;
        }

        if (treasureEntryList.empty())
            return;

        std::list<uint32>::const_iterator itr = treasureEntryList.begin();
        std::advance(itr, urand(1, treasureEntryList.size() - 1));
        uint32 findEntry = (*itr);

        if (!findEntry)
            return;

        GameObject* pGameObj = new GameObject;
        Position summonPos;
        m_player->GetClosePoint(summonPos.m_positionX, summonPos.m_positionY, summonPos.m_positionZ, DEFAULT_WORLD_OBJECT_SIZE);
        Map *map = m_player->GetMap();

        if (!pGameObj->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_GAMEOBJECT), findEntry, map, m_player->GetPhaseMask(), summonPos.m_positionX, summonPos.m_positionY, summonPos.m_positionZ, m_player->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f, 0, GO_STATE_READY))
        {
            delete pGameObj;
            return;
        }

        pGameObj->SetRespawnTime(30000 / IN_MILLISECONDS);
        m_player->AddGameObject(pGameObj);
        map->AddToMap(pGameObj);

        // Skill up
        if (skill_now < 100)
            m_player->SetSkill(SKILL_ARCHAEOLOGY, m_player->GetSkillStep(SKILL_ARCHAEOLOGY), skill_now + 1, m_player->GetMaxSkillValue(SKILL_ARCHAEOLOGY));
        else if (skill_now > 525)
            m_player->SetSkill(SKILL_ARCHAEOLOGY, m_player->GetSkillStep(SKILL_ARCHAEOLOGY), 525, m_player->GetMaxSkillValue(SKILL_ARCHAEOLOGY));

        // Give currency & project
        uint32 currencyId = 0;
        uint32 branchId;
        switch (findEntry)
        {
            case ARCHAEOLOGY_DWARF_TREASURE:
                currencyId = CURRENCY_TYPE_ARCHEAOLOGY_DWARF;
                branchId = 1;
                break;
            case ARCHAEOLOGY_DRAENEI_TREASURE:
                currencyId = CURRENCY_TYPE_ARCHEAOLOGY_DRAENEI;
                branchId = 2;
                break;
            case ARCHAEOLOGY_FOSSIL_TREASURE:
                currencyId = CURRENCY_TYPE_ARCHEAOLOGY_FOSSIL;
                branchId = 3;
                break;
            case ARCHAEOLOGY_NERUBIAN_TREASURE:
                currencyId = CURRENCY_TYPE_ARCHEAOLOGY_NERUBIAN;
                branchId = 5;
                break;
            case ARCHAEOLOGY_NIGHTELF_TREASURE:
                currencyId = CURRENCY_TYPE_ARCHEAOLOGY_NIGHT_ELF;
                branchId = 4;
                break;
            case ARCHAEOLOGY_ORC_TREASURE:
                currencyId = CURRENCY_TYPE_ARCHEAOLOGY_ORC;
                branchId = 6;
                break;
            case ARCHAEOLOGY_TOLVIR_TREASURE:
                currencyId = CURRENCY_TYPE_ARCHEAOLOGY_TOLVIR;
                branchId = 7;
                break;
            case ARCHAEOLOGY_TROLL_TREASURE:
                currencyId = CURRENCY_TYPE_ARCHEAOLOGY_TROLL;
                branchId = 8;
                break;
            case ARCHAEOLOGY_VRYKUL_TREASURE:
                currencyId = CURRENCY_TYPE_ARCHEAOLOGY_VRYKUL;
                branchId = 27;
                break;
        }

        GenerateResearchProject(branchId, false, 0);
        if (skill_now < 450)
            m_player->ModifyCurrency(currencyId, urand(3, 4));
        else
            m_player->ModifyCurrency(currencyId, urand(5, 6));
        // Reset coord
        m_actualDigPos.m_positionX = 0.0f;
        m_actualDigPos.m_positionY = 0.0f;
        m_digSitesCount[m_currentDigSite]++;

        if (m_digSitesCount[m_currentDigSite] >= 3)
        {
            m_digSitesCount[m_currentDigSite] = 0;
            uint8 slot = 0;
            for (slot = 0; slot < 8; slot++)
            {
                bool site_1 = (m_player->GetUInt32Value(PLAYER_FIELD_RESEARCH_SITE_1 + slot) & 0xFFFF) == m_digSites[m_currentDigSite];
                bool site_2 = (m_player->GetUInt32Value(PLAYER_FIELD_RESEARCH_SITE_1 + slot) >> 16) == m_digSites[m_currentDigSite];
                if (site_1 || site_2)
                {
                    m_digSites[m_currentDigSite] = GetNewRandomSite(m_player->GetMapId());
                    SetDigSiteInSlot(slot, site_1 ? m_digSites[m_currentDigSite] : (m_player->GetUInt32Value(PLAYER_FIELD_RESEARCH_SITE_1 + slot) & 0xFFFF),
                                     site_2 ? m_digSites[m_currentDigSite] : (m_player->GetUInt32Value(PLAYER_FIELD_RESEARCH_SITE_1 + slot) >> 16));
                    break;
                }
            }
        }
    }
}

uint32 ArcheologyMgr::GetNewRandomSite(const uint32 &map)
{
    uint32 skill_now = m_player->GetSkillValue(SKILL_ARCHAEOLOGY);
    std::list<uint32> sitesList = sObjectMgr->GetResearchSiteList(map, skill_now, m_player->getLevel());
    std::list<uint32> tempSitesList;
    for (std::list<uint32>::const_iterator itr = sitesList.begin(); itr != sitesList.end(); ++itr)
    {
        bool isValid = true;
        for (int i = 0; i < 16; i++)
            if (m_digSites[i] == *itr)
            {
                isValid = false;
                break;
            }
        if (isValid)
            tempSitesList.push_back(*itr);
    }
    return (Trinity::Containers::SelectRandomContainerElement(tempSitesList));
}

void ArcheologyMgr::SetDigSiteInSlot(const uint32 &slot, const uint32 &site1, const uint32 &site2)
{
    uint32 new_value = (site2 << 16) | (site1);
    m_player->SetUInt32Value(PLAYER_FIELD_RESEARCH_SITE_1 + slot, new_value);
}

void ArcheologyMgr::GenerateResearchProject(const uint32 &branchId, bool force, const uint32 &excludeId)
{
    if (m_researchProject[branchId] != 0 && !force)
        return;

    // Remove old
    if (force)
    {
        uint32 reset_value = 0;
        uint8 resetSlot = 0;
        for (uint8 i = 0; i < 5; i++)
        {
            uint32 project_now_1 = m_player->GetUInt32Value(PLAYER_FIELD_RESEARCHING_1 + i) & 0xFFFF;
            uint32 project_now_2 = m_player->GetUInt32Value(PLAYER_FIELD_RESEARCHING_1 + i) >> 16;

            if (project_now_1 == m_researchProject[branchId])
            {
                resetSlot = int32(i);
                reset_value = (project_now_2 << 16) | 0;
                break;
            }
            else if (project_now_2 == m_researchProject[branchId])
            {
                resetSlot = int32(i);
                reset_value = (0 << 16) | (project_now_1);
                break;
            }
        }
        m_player->SetUInt32Value(PLAYER_FIELD_RESEARCHING_1 + resetSlot, reset_value);
        m_researchProject[branchId] = 0;
    }

    uint32 skill_now = m_player->GetSkillValue(SKILL_ARCHAEOLOGY);
    std::list<uint32> projectList;
    for(uint32 row = 39; row < sResearchProjectStore.GetNumRows(); row++)
    {
        ResearchProjectEntry *rs = sResearchProjectStore.LookupRow(row);
        if (!rs)
            continue;

        if (rs->branchId != branchId)
            continue;

        if (excludeId != 0)
            if (rs->spellId == excludeId)
                continue;

        if (rs->quality != 0) // rare and epic craft
        {
            if (m_researchHistory.find(rs->id) != m_researchHistory.end())
                continue;

            if (!roll_chance_i(skill_now * 100 / 525))
                continue;
        }

        uint32 frag_req = 75;
        if (skill_now / 3 > 75)
            frag_req = skill_now / 3;
        if (rs->req_fragments > frag_req)
            continue;

        projectList.push_back(rs->id);
    }

    uint32 selectedProject = urand (0, projectList.size());
    uint8 count = 0;
    for (std::list<uint32>::const_iterator itr = projectList.begin(); itr != projectList.end(); ++itr)
    {
        if (count == selectedProject)
        {
            int32 slot = -10;
            int32 new_value = -10;
            for (uint8 i = 0; i < 5; i++)
            {
                uint32 project_now_1 = m_player->GetUInt32Value(PLAYER_FIELD_RESEARCHING_1 + i) & 0xFFFF;
                uint32 project_now_2 = m_player->GetUInt32Value(PLAYER_FIELD_RESEARCHING_1 + i) >> 16;

                if (project_now_1 == 0)
                {
                    slot = int32(i);
                    new_value = (project_now_2 << 16) | (*itr);
                    break;
                }
                else if (project_now_2 == 0)
                {
                    slot = int32(i);
                    new_value = ((*itr) << 16) | (project_now_1);
                    break;
                }
            }

            if (slot >= 0 && new_value >= 0)
                m_player->SetUInt32Value(PLAYER_FIELD_RESEARCHING_1 + slot, new_value);

            m_researchProject[branchId] = (*itr);
            break;
        }
        count++;
    }
}

void ArcheologyMgr::GenerateSavedArtifacts()
{
    uint8 slot = 0;
    for (uint8 i = 1; i < 28; i++)
    {
        if (i == 9)
            i = 27;

        uint32 project_1 = m_researchProject[i];
        uint32 project_2 = 0;

        if (i != 27)
            project_2 = m_researchProject[i+1];

        uint32 value = (project_2 << 16) | (project_1);
        m_player->SetUInt32Value(PLAYER_FIELD_RESEARCHING_1 + slot, value);
        ++slot;
        ++i;
    }
}

void ArcheologyMgr::CompleteArtifact(const uint32 &artId, const uint32 &spellId, ByteBuffer &data)
{
    uint32 target_mask, unk1, unk2, numberOfStones;
    data >> target_mask >> unk1 >> unk2 >> numberOfStones;

    if (numberOfStones == 256 || numberOfStones == 512 || numberOfStones == 768)
        numberOfStones = numberOfStones / 256;
    else
        numberOfStones = 0;

    if (ResearchProjectEntry* rp = sResearchProjectStore.LookupRow(artId))
    {
        uint32 currencyId = 0;
        uint32 stone = 0;
        switch (rp->branchId)
        {
            case 1: currencyId = CURRENCY_TYPE_ARCHEAOLOGY_DWARF;
                stone = 52843;
                break;
            case 2:
                currencyId = CURRENCY_TYPE_ARCHEAOLOGY_DRAENEI;
                stone = 64394;
                break;
            case 3:
                currencyId = CURRENCY_TYPE_ARCHEAOLOGY_FOSSIL;
                stone = 0;
                break;
            case 5:
                currencyId = CURRENCY_TYPE_ARCHEAOLOGY_NERUBIAN;
                stone = 64396;
                break;
            case 4:
                currencyId = CURRENCY_TYPE_ARCHEAOLOGY_NIGHT_ELF;
                stone = 63127;
                break;
            case 6:
                currencyId = CURRENCY_TYPE_ARCHEAOLOGY_ORC;
                stone = 64392;
                break;
            case 7:
                currencyId = CURRENCY_TYPE_ARCHEAOLOGY_TOLVIR;
                stone = 64397;
                break;
            case 8:
                currencyId = CURRENCY_TYPE_ARCHEAOLOGY_TROLL;
                stone = 63128;
                break;
            case 27:
                currencyId = CURRENCY_TYPE_ARCHEAOLOGY_VRYKUL;
                stone = 64395;
                break;
        }

        uint32 currencySale = 0;
        if (numberOfStones > 0)
        {
            if (m_player->HasItemCount(stone, numberOfStones, false))
                currencySale = 12 * numberOfStones;
        }

        bool found = false;
        for (uint8 x = 0; x < 5; x++)
        {
            uint32 project_now_1 = m_player->GetUInt32Value(PLAYER_FIELD_RESEARCHING_1 + x) & 0xFFFF;
            uint32 project_now_2 = m_player->GetUInt32Value(PLAYER_FIELD_RESEARCHING_1 + x) >> 16;

            if (project_now_1 == rp->id || project_now_2 == rp->id)
            {
                found = true;
                break;
            }
        }

        if (!found)
            return;

        int32 bp0 = int32(numberOfStones);
        if (currencyId != 0)
            if (m_player->GetCurrency(currencyId, false) >= rp->req_fragments - currencySale)
               m_player->CastCustomSpell(m_player, rp->spellId, &bp0, NULL, NULL, false);

    }
}

void ArcheologyMgr::LoadArcheologyDigSites(PreparedQueryResult digsiteResult, PreparedQueryResult currentArtifactsResult, PreparedQueryResult researchResult)
{
    // Reset dig position
    m_actualDigPos.m_positionX = 0.0f;
    m_actualDigPos.m_positionY = 0.0f;
    m_currentDigSite = 0;

    // Load archeology dig sites
    for (uint8 i = 0; i < 16; i++)
    {
        m_digSites[i] = 0;
        m_digSitesCount[i] = 0;
    }

    if (digsiteResult)
    {
        uint8 count = 0;
       do
       {
           Field* digFields = digsiteResult->Fetch();
           uint32 digId = digFields[0].GetUInt32();
           uint32 digCount = digFields[1].GetUInt32();
           m_digSites[count] = digId;
           m_digSitesCount[count] = digCount;
           count++;
       }
       while (digsiteResult->NextRow());
    }

    // Load archaeology current artifacts
    for (uint8 i = 0; i < 28; i++)
        m_researchProject[i] = 0;

    if (currentArtifactsResult)
    {
        do
        {
            Field* artFields = currentArtifactsResult->Fetch();
            uint32 artId = artFields[0].GetUInt32();
            uint32 branchId = artFields[1].GetUInt32();
            m_researchProject[branchId] = artId;
        } while (currentArtifactsResult->NextRow());
    }

    m_researchHistory.clear();

    if (researchResult)
    {
        do
        {
            Field* researchFields = researchResult->Fetch();
            ResearchHistoryEntry rhe(researchFields[0].GetUInt32(), researchFields[1].GetUInt32(), researchFields[2].GetUInt32(), researchFields[3].GetUInt32());
            m_researchHistory[researchFields[1].GetUInt32()] = rhe;
        } while (researchResult->NextRow());
    }

}

void ArcheologyMgr::SaveArcheology()
{
    if (m_player->HasSkill(SKILL_ARCHAEOLOGY))
    {
        SaveArcheologyDigSites();
        SaveArcheologyArtifacts();
    }
}

void ArcheologyMgr::SaveArcheologyDigSites()
{
    CharacterDatabase.PQuery("DELETE FROM character_digsites WHERE guid = %u", m_player->GetGUIDLow());
    for (uint8 i = 0; i < 16; ++i)
    {
        if (m_digSites[i] != 0)
        {
            PreparedStatement * stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_DIGSITES);
            stmt->setUInt32(0, m_player->GetGUIDLow());
            stmt->setUInt32(1, m_digSites[i]);
            stmt->setUInt32(2, m_digSitesCount[i]);
            CharacterDatabase.Execute(stmt);
        }
    }
}

void ArcheologyMgr::SaveArcheologyArtifacts()
{
    CharacterDatabase.PQuery("DELETE FROM character_current_artifacts WHERE guid = %u", m_player->GetGUIDLow());
    for (uint8 i = 1; i < 28; ++i)
    {
        if (i == 9)
            i = 27;

        if (m_researchProject[i] != 0)
        {
            PreparedStatement * stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_CURRENT_ARTEFACTS);
            stmt->setUInt32(0, m_player->GetGUIDLow());
            stmt->setUInt32(1, i);
            stmt->setUInt32(2, m_researchProject[i]);
            CharacterDatabase.Execute(stmt);
        }
    }
}

void ArcheologyMgr::TrackRessources(const uint32 &auraId, bool apply)
{
    if (auraId == 74268 && apply) // Archaeology track
    {
        GenerateResearchDigSites();
        GenerateSavedArtifacts();
    }
}

void ArcheologyMgr::CreateTreasureItem(const uint32 & spellId, int32 &quantity)
{
    for (uint8 i = 1; i < 28; ++i)
    {
        if (i == 9)
            i = 27;

        ResearchProjectEntry* rp = sResearchProjectStore.LookupRow(m_researchProject[i]);
        if (!rp)
            continue;

        if (rp->spellId == spellId)
        {
            int32 reqFragments = int32(rp->req_fragments);
            uint32 currencyId = 0;
            uint32 stone = 0;
            switch (rp->branchId)
            {
                case 1:
                    currencyId = CURRENCY_TYPE_ARCHEAOLOGY_DWARF;
                    stone = 52843;
                    break;
                case 2:
                    currencyId = CURRENCY_TYPE_ARCHEAOLOGY_DRAENEI;
                    stone = 64394;
                    break;
                case 3:
                    currencyId = CURRENCY_TYPE_ARCHEAOLOGY_FOSSIL;
                    stone = 0;
                    break;
                case 5:
                    currencyId = CURRENCY_TYPE_ARCHEAOLOGY_NERUBIAN;
                    stone = 64396;
                    break;
                case 4:
                    currencyId = CURRENCY_TYPE_ARCHEAOLOGY_NIGHT_ELF;
                    stone = 63127;
                    break;
                case 6:
                    currencyId = CURRENCY_TYPE_ARCHEAOLOGY_ORC;
                    stone = 64392;
                    break;
                case 7:
                    currencyId = CURRENCY_TYPE_ARCHEAOLOGY_TOLVIR;
                    stone = 64397;
                    break;
                case 8:
                    currencyId = CURRENCY_TYPE_ARCHEAOLOGY_TROLL;
                    stone = 63128;
                    break;
                case 27:
                    currencyId = CURRENCY_TYPE_ARCHEAOLOGY_VRYKUL;
                    stone = 64395;
                    break;
            }

            if (quantity > 0)
            {
                if (m_player->HasItemCount(stone, quantity, false))
                {
                    if (Item* pStone = m_player->GetItemByEntry(stone))
                    {
                        if (pStone->GetCount() > uint32(quantity))
                            pStone->SetCount(pStone->GetCount() - quantity);
                        else if (pStone->GetCount() == uint32(quantity))
                            m_player->DestroyItem(pStone->GetBagSlot(), pStone->GetSlot(), true);
                        else
                        {
                            for (int x = 0; x < quantity;)
                            {
                                if (x > 0)
                                {
                                    pStone = m_player->GetItemByEntry(stone);
                                    if (pStone && pStone->GetCount() > uint32(quantity) - x)
                                    {
                                        pStone->SetCount(pStone->GetCount() - quantity + x);
                                        break;
                                    }
                                }
                                if (pStone)
                                {
                                    x += pStone->GetCount();
                                    m_player->DestroyItem(pStone->GetBagSlot(), pStone->GetSlot(), true);
                                    pStone = NULL;
                                }
                                else
                                    break;
                            }
                        }

                        reqFragments -= 12 * quantity;
                    }
                }
            }

            if (m_researchHistory.find(rp->id) != m_researchHistory.end())
            {
                ResearchHistoryEntry rhe = m_researchHistory[rp->id];
                ++rhe.count;
                m_researchHistory[rp->id] = rhe;
                SQLTransaction trans = CharacterDatabase.BeginTransaction();
                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_RESEARCH_HISTORY);
                stmt->setUInt32(0, rhe.count);
                stmt->setUInt32(1, rhe.guid);
                stmt->setUInt32(2, rhe.projectId);
                trans->Append(stmt);
                CharacterDatabase.CommitTransaction(trans);

                // -1 indicates project is already researched (only for common items)
                // 0 is common, 1 is rare
                m_player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ARCHAEOLOGY_PROJECTS, rp->branchId, rp->quality > 0 ? 1 : -1, 1);
            }
            else
            {
                ResearchHistoryEntry rhe(m_player->GetGUIDLow(), rp->id, 1, time(NULL));
                m_researchHistory[rp->id] = rhe;
                SQLTransaction trans = CharacterDatabase.BeginTransaction();
                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_RESEARCH_HISTORY);
                stmt->setUInt32(0, rhe.guid);
                stmt->setUInt32(1, rhe.projectId);
                stmt->setUInt32(2, rhe.count);
                stmt->setUInt32(3, rhe.time);
                trans->Append(stmt);
                CharacterDatabase.CommitTransaction(trans);

                // item quality: 0 is common, 1 is rare
                m_player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ARCHAEOLOGY_PROJECTS, rp->branchId, rp->quality, 1);
            }

            m_player->ModifyCurrency(currencyId, -(reqFragments));
            GenerateResearchProject(rp->branchId, true, rp->spellId);
            quantity = 0;
            break;
        }
    }
}

uint8 ArcheologyMgr::getMaxDigsites()
{
    return MAX_RESEARCH_SITES;
    uint32 skill = m_player->GetSkillValue(SKILL_ARCHAEOLOGY);
    uint8 digsites = MAX_RESEARCH_SITES;
    if (skill > 300 && skill <= 375)
        digsites += 4;
    else if (skill > 375)
        digsites = 2 * MAX_RESEARCH_SITES;
    return digsites;
}


void WorldSession::HandleRequestResearchHistory(WorldPacket& recvPacket)
{
    std::map<uint32, ArcheologyMgr::ResearchHistoryEntry >* researchHistory = GetPlayer()->GetArcheologyMgr().GetReasearchHistory();
    uint32 count = 0;
    if (researchHistory && !researchHistory->empty())
        count = researchHistory->size();
    else
        return;

    WorldPacket data(SMSG_RESEARCH_SETUP_HISTORY, 22 + count * (32 + 32 + 32));

    data.WriteBits(count, 22);

    for (std::map<uint32, ArcheologyMgr::ResearchHistoryEntry>::const_iterator itr = researchHistory->begin(); itr != researchHistory->end(); ++itr)
    {
        data << uint32(itr->second.projectId);
        data << uint32(itr->second.count);
        data << uint32(itr->second.time);
    }

    SendPacket(&data);
}
