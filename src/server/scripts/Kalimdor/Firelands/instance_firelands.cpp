/*
* Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
*

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
#include "ScriptPCH.h"
#include "InstanceScript.h"
#include "firelands.h"

#define MAX_ENCOUNTER 7

class instance_firelands : public InstanceMapScript
{
public:
    instance_firelands() : InstanceMapScript(FirelandsScriptName, 720) { }

    struct instance_firelands_InstanceScript : public InstanceScript
    {
        instance_firelands_InstanceScript(InstanceMap* map) : InstanceScript(map)
        {
        }

        void Initialize()
        {
            SetBossNumber(EncounterCount);
            for (uint8 i = 1; i < MAX_ENCOUNTER; ++i)
                uiEncounter[i] = NOT_STARTED;

            uiShannox = 0;
            uiRhyolith = 0;
            uiBethtilac = 0;
            uiAlysrazor = 0;
            uiBaleroc = 0;
            uiMajordomoStaghelm = 0;
            uiRagnaros = 0;
            slaintrash = 0;
            bridgeGUID = 0;
        }

        bool IsEncounterInProgress() const
        {
            for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                if (uiEncounter[i] == IN_PROGRESS)
                    return true;

            return false;
        }

        uint64 GetData64(uint32 identifier) const
        {
            switch (identifier)
            {
                //BOSSES
            case DATA_SHANNOX:                      return uiShannox;
            case DATA_RHYOLITH:						return uiRhyolith;
            case DATA_BETHTILAC:                    return uiBethtilac;
            case DATA_ALYSRAZOR:			        return uiAlysrazor;
            case DATA_BALEROC:					    return uiBaleroc;
            case DATA_MAJORDOMO_STAGHELM:			return uiMajordomoStaghelm;
            case DATA_RAGNAROS:				   	    return uiRagnaros;
                //NPCs
            case NPC_RAGEFACE:                      return uiRageface;
            case NPC_RIPLIMB:                       return uiRiplimb;
            case NPC_SHANNOX_SPEAR:                 return uiShannoxSpear;
            }

            return 0;
        }

        void OnPlayerEnter(Player* player)
        {
            if (!uiTeamInInstance)
                uiTeamInInstance = player->GetTeam();
        }

        void OnCreatureKill(Player* /*killer*/, Creature* killed)
        {
            switch (killed->GetEntry())
            {//IDs of the mobs counting toward the shannox spawn
            case 53115:
            case 53119:
            case 53130:
            case 53134:
            case 53141:
            case 53167:
            case 53185:
            case 53187:
            case 53188:
            case 53206:
            case 53220:
            case 53221:
            case 53222:
            case 53223:
            case 53639:
            case 53640:
            case 53732:
            case 53901:
            case 54073:
                slaintrash++;
                break;
            default:
                break;
            }
            if (slaintrash >= 25 && GetData(DATA_SHANNOX_EVENT) == TO_BE_DECIDED)
            {
                Creature* shannox = instance->SummonCreature(BOSS_SHANNOX, shannoxspawn);
                SetData(DATA_SHANNOX_EVENT, NOT_STARTED);
                if (shannox)
                {
                    shannox->MonsterYell("Yes... I smell them too, Riplimb. Outsiders encroach on the Firelord's private grounds. Find their trail, find them for me, that I may dispense punishment!", 0, 0);
                    shannox->PlayDistanceSound(24584);
                }
            }
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch (go->GetEntry())
            {
            case GO_RAID_BRIDGE_FORMING_DOOR:
                bridgeGUID = go->GetGUID();
                break;
            case GO_MAJORDOMO_BACKFIRE:
                if (uiEncounter[5] == DONE)
                    HandleGameObject(go->GetGUID(), true, go);
                break;
            case GO_BALEROC_DOOR:
                if (uiEncounter[0] == DONE && uiEncounter[1] == DONE && uiEncounter[2] == DONE && uiEncounter[3])
                    HandleGameObject(go->GetGUID(), true, go);
                break;
            }
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
            case DATA_SHANNOX_EVENT:
                uiEncounter[0] = data;
                break;
            case DATA_RHYOLITH_EVENT:
                uiEncounter[1] = data;
                break;
            case DATA_BETHTILAC_EVENT:
                uiEncounter[2] = data;
                break;
            case DATA_ALYSRAZOR_EVENT:
                uiEncounter[3] = data;
                break;
            case DATA_BALEROC_EVENT:
                uiEncounter[4] = data;
                break;
            case DATA_MAJORDOMO_STAGHELM_EVENT:
                uiEncounter[5] = data;
                break;
            case DATA_RAGNAROS_EVENT:
                uiEncounter[6] = data;
                break;
            }

            if (data == DONE)
                SaveToDB();
        }

        uint32 GetData(uint32 type) const
        {
            switch (type)
            {
            case DATA_SHANNOX_EVENT:                    return uiEncounter[0];
            case DATA_RHYOLITH_EVENT:			        return uiEncounter[1];
            case DATA_BETHTILAC_EVENT:					return uiEncounter[2];
            case DATA_ALYSRAZOR_EVENT:					return uiEncounter[3];
            case DATA_BALEROC_EVENT:                    return uiEncounter[4];
            case DATA_MAJORDOMO_STAGHELM_EVENT:         return uiEncounter[5];
            case DATA_RAGNAROS_EVENT:                   return uiEncounter[6];
            case DATA_CURRENT_ENCOUNTER_PHASE:          return encounterSharingPhase;

            }

            return 0;
        }

        std::string GetSaveData()
        {
            OUT_SAVE_INST_DATA;

            std::string str_data;

            std::ostringstream saveStream;
            saveStream << "P S " << uiEncounter[0] << " " << uiEncounter[1] << " " << uiEncounter[2] << " " << uiEncounter[3] << " " << uiEncounter[4] << " " << uiEncounter[5] << " " << uiEncounter[6];

            str_data = saveStream.str();

            OUT_SAVE_INST_DATA_COMPLETE;
            return str_data;
        }

        void Load(const char* in)
        {
            if (!in)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(in);

            char dataHead1, dataHead2;
            uint16 data0, data1, data2, data3, data4, data5, data6;

            std::istringstream loadStream(in);
            loadStream >> dataHead1 >> dataHead2 >> data0 >> data1 >> data2 >> data3 >> data4 >> data5 >> data6;

            if (dataHead1 == 'P' && dataHead2 == 'S')
            {
                uiEncounter[0] = data0;
                uiEncounter[1] = data1;
                uiEncounter[2] = data2;
                uiEncounter[3] = data3;
                uiEncounter[4] = data4;
                uiEncounter[5] = data5;
                uiEncounter[6] = data6;

                for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                    if (uiEncounter[i] == IN_PROGRESS)
                        uiEncounter[i] = NOT_STARTED;

            }
            else OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }

        void OnCreatureCreate(Creature* creature)
        {
            Map::PlayerList const &players = instance->GetPlayers();

            if (!players.isEmpty())
            {
                if (Player* pPlayer = players.begin()->getSource())
                    uiTeamInInstance = pPlayer->GetTeam();
            }

            switch (creature->GetEntry())
            {
                //Bosses!
            case BOSS_SHANNOX:
                uiShannox = creature->GetGUID();
                break;
            case BOSS_RHYOLITH:
                uiRhyolith = creature->GetGUID();
                break;
            case BOSS_BETHTILAC:
                uiBethtilac = creature->GetGUID();
                break;
            case BOSS_ALYSRAZOR:
                uiAlysrazor = creature->GetGUID();
                break;
            case BOSS_BALEROC:
                uiBaleroc = creature->GetGUID();
                break;
            case BOSS_MAJORDOMO_STAGHELM:
                uiMajordomoStaghelm = creature->GetGUID();
                break;
            case BOSS_RAGNAROS:
                uiRagnaros = creature->GetGUID();
                break;
                // Npcs
            case NPC_RAGEFACE:
                uiRageface = creature->GetGUID();
                break;
            case NPC_RIPLIMB:
                uiRiplimb = creature->GetGUID();
                break;
            case NPC_SHANNOX_SPEAR:
                uiShannoxSpear = creature->GetGUID();
                break;
            case NPC_SMOULDERING_HATCHLING:
                // Cannot directly start attacking here as the creature is not yet on map
                creature->m_Events.AddEvent(new DelayedAttackStartEvent(creature), creature->m_Events.CalculateTime(500));
                break;
            }
        }
    private:
        uint64 bridgeGUID;
        uint32 slaintrash;
        uint64 uiShannox;
        uint64 uiRhyolith;
        uint64 uiBethtilac;
        uint64 uiAlysrazor;
        uint64 uiBaleroc;
        uint64 uiMajordomoStaghelm;
        uint64 uiRagnaros;
        uint64 uiMonstrosity;
        uint64 uiRiplimb;
        uint64 uiRageface;
        uint64 uiShannoxSpear;
        uint32 uiTeamInInstance;
        uint32 encounterSharingPhase;
        uint32 uiEncounter[MAX_ENCOUNTER];

    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_firelands_InstanceScript(map);
    }
};

void AddSC_instance_firelands()
{
    new instance_firelands();
}