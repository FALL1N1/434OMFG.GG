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

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "obsidian_sanctum.h"

#define MAX_ENCOUNTER     4

/* Obsidian Sanctum encounters:
0 - Sartharion
*/

class instance_obsidian_sanctum : public InstanceMapScript
{
public:
    instance_obsidian_sanctum() : InstanceMapScript("instance_obsidian_sanctum", 615) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_obsidian_sanctuinstanceMapScript(map);
    }

    struct instance_obsidian_sanctuinstanceMapScript : public InstanceScript
    {
        instance_obsidian_sanctuinstanceMapScript(Map* map) : InstanceScript(map) {}

        uint64 m_uiSartharionGUID;
        uint64 m_uiTenebronGUID;
        uint64 m_uiShadronGUID;
        uint64 m_uiVesperonGUID;

        bool m_bTenebronKilled;
        bool m_bShadronKilled;
        bool m_bVesperonKilled;
        bool isLesserIsMoreGroup;

        void Initialize()
        {
            SetBossNumber(MAX_ENCOUNTER);

            m_uiSartharionGUID = 0;
            m_uiTenebronGUID = 0;
            m_uiShadronGUID = 0;
            m_uiVesperonGUID = 0;

            m_bTenebronKilled = false;
            m_bShadronKilled = false;
            m_bVesperonKilled = false;
            isLesserIsMoreGroup = true;
        }

        void OnCreatureCreate(Creature* creature)
        {
            switch (creature->GetEntry())
            {
            case NPC_SARTHARION:
                m_uiSartharionGUID = creature->GetGUID();
                break;
                //three dragons below set to active state once created.
                //we must expect bigger raid to encounter main boss, and then three dragons must be active due to grid differences
            case NPC_TENEBRON:
                m_uiTenebronGUID = creature->GetGUID();
                creature->setActive(true);
                break;
            case NPC_SHADRON:
                m_uiShadronGUID = creature->GetGUID();
                creature->setActive(true);
                break;
            case NPC_VESPERON:
                m_uiVesperonGUID = creature->GetGUID();
                creature->setActive(true);
                break;
            }
        }

        bool SetBossState(uint32 type, EncounterState state)
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            if (state == IN_PROGRESS)
                //      if (isLesserIsMoreGroup)
                if (instance)
                    isLesserIsMoreGroup = instance->GetPlayersCountExceptGMs() < (uint32)(instance->GetDifficulty() ? 21 : 9);

            /*       if (state == DONE)
            {
            bool allBossesDone = true;
            for (uint32 i = 0; i < MAX_ENCOUNTER; ++i)
            {
            if (GetBossState(i) != DONE)
            {
            allBossesDone = false;
            break;
            }
            }
            if (allBossesDone && instance && isLesserIsMoreGroup)
            DoCompleteAchievement(instance->GetDifficulty() ? ACHIEV_LESS_IS_MORE_25 : ACHIEV_LESS_IS_MORE_10);
            }*/
            return true;
        }

        void SetData(uint32 uiType, uint32 uiData)
        {
            switch (uiType)
            {
            case DATA_TENEBRON_PREKILLED:
                m_bTenebronKilled = true;
                break;
            case DATA_SHADRON_PREKILLED:
                m_bShadronKilled = true;
                break;
            case DATA_VESPERON_PREKILLED:
                m_bVesperonKilled = true;
                break;
            }
        }

        uint32 GetData(uint32 uiType) const
        {
            switch (uiType)
            {
            case DATA_TENEBRON_PREKILLED:
                return m_bTenebronKilled;
            case DATA_SHADRON_PREKILLED:
                return m_bShadronKilled;
            case DATA_VESPERON_PREKILLED:
                return m_bVesperonKilled;
            }
            return 0;
        }

        uint64 GetData64(uint32 uiData) const
        {
            switch (uiData)
            {
            case DATA_SARTHARION:
                return m_uiSartharionGUID;
            case DATA_TENEBRON:
                return m_uiTenebronGUID;
            case DATA_SHADRON:
                return m_uiShadronGUID;
            case DATA_VESPERON:
                return m_uiVesperonGUID;
            }
            return 0;
        }

        bool CheckAchievementCriteriaMeet(uint32 criteria_id, Player const* /*source*/, Unit const* /*target = NULL*/, uint32 /*miscvalue1 = 0*/)
        {
            switch (criteria_id)
            {
                // Criteria for achievement 624: Less Is More (10 player)
            case 522:
            case 7189:
            case 7190:
            case 7191:
                // Criteria for achievement 1877: Less Is More (25 player)
            case 7185:
            case 7186:
            case 7187:
            case 7188:
                return isLesserIsMoreGroup;
            }
            return false;
        }

        //void Load(const char * data)
        //{
        //    std::istringstream loadStream(LoadBossState(data));
        //    //uint32 buff;
        //    //for (uint32 i = 0; i < MAX_ENCOUNTER; ++i)
        //    //    loadStream >> buff;

        //    //loadStream >> isLesserIsMoreGroup;
        //}
    };

};

void AddSC_instance_obsidian_sanctum()
{
    new instance_obsidian_sanctum();
}
