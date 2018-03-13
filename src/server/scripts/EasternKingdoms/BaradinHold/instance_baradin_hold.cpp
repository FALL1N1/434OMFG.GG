/*
 * Copyright (C) 2013-2014 OMFG.GG Network <http://www.omfg.gg/>
 * Copyright (C) 2011-2013 Devastion Cataclysm <http://www.cataclysm-wow.eu/>
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

#include "InstanceScript.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "BattlefieldMgr.h"
#include "baradin_hold.h"

DoorData const doorData[] =
{
    {GO_ARGALOTH_DOOR,  DATA_ARGALOTH, DOOR_TYPE_ROOM,  BOUNDARY_NONE},
    {GO_OCCUTHAR_DOOR,  DATA_OCCUTHAR, DOOR_TYPE_ROOM,  BOUNDARY_NONE},
    {GO_ALIZABAL_DOOR,  DATA_ALIZABAL, DOOR_TYPE_ROOM,  BOUNDARY_NONE}
};

class instance_baradin_hold: public InstanceMapScript
{
public:
    instance_baradin_hold() : InstanceMapScript("instance_baradin_hold", 757) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_baradin_hold_InstanceMapScript(map);
    }

    struct instance_baradin_hold_InstanceMapScript: public InstanceScript
    {
        instance_baradin_hold_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetBossNumber(MAX_ENCOUNTERS);
            LoadDoorData(doorData);
            ArgalothGUID = 0;
            OccutharGUID = 0;
            AlizabalGUID = 0;
            checkBattlefieldTimer = 60000;
        }

        void OnPlayerEnter(Player *player)
        {
            if (sWorld->getBoolConfig(CONFIG_TOL_BARAD_ENABLE))
            {
                Battlefield *pvpTB = sBattlefieldMgr->GetBattlefieldToZoneId(5095);
                if (pvpTB && !player->isGameMaster() && (player->GetTeamId() != pvpTB->GetDefenderTeam() || pvpTB->IsWarTime()))
                    player->TeleportTo(732, -616.94f, 1183.27f, 95.71f, player->GetOrientation());
            }
        }

        void OnCreatureCreate(Creature* creature)
        {
            switch(creature->GetEntry())
            {
                case BOSS_ARGALOTH:
                    ArgalothGUID = creature->GetGUID();
                    break;
                case BOSS_OCCUTHAR:
                    OccutharGUID = creature->GetGUID();
                    break;
                case BOSS_ALIZABAL:
                    AlizabalGUID = creature->GetGUID();
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch(go->GetEntry())
            {
                case GO_ARGALOTH_DOOR:
                case GO_OCCUTHAR_DOOR:
                case GO_ALIZABAL_DOOR:
                    AddDoor(go, true);
                    break;
            }
        }

        uint64 GetData64(uint32 data) const
        {
            switch (data)
            {
                case DATA_ARGALOTH:
                    return ArgalothGUID;
                case DATA_OCCUTHAR:
                    return OccutharGUID;
                case DATA_ALIZABAL:
                    return AlizabalGUID;
                default:
                    break;
            }
            return 0;
        }

        void OnGameObjectRemove(GameObject* go)
        {
            switch(go->GetEntry())
            {
                case GO_ARGALOTH_DOOR:
                case GO_OCCUTHAR_DOOR:
                case GO_ALIZABAL_DOOR:
                    AddDoor(go, false);
                    break;
            }
        }

        void Update(uint32 diff)
        {
            if (!sWorld->getBoolConfig(CONFIG_TOL_BARAD_ENABLE))
                return;

            if (checkBattlefieldTimer <= diff)
            {
                Battlefield *pvpTB = sBattlefieldMgr->GetBattlefieldToZoneId(5095);
                if (pvpTB && (pvpTB->CanQueue() || pvpTB->IsWarTime()))
                {
                    Map::PlayerList const& players = instance->GetPlayers();
                    for (Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
                        if (Player* player = i->getSource())
                            if (!player->isGameMaster())
                            {
                                if (pvpTB->IsWarTime())
                                    player->TeleportTo(732, -616.94f, 1183.27f, 95.71f, player->GetOrientation());
                                else if (pvpTB->CanQueue() && !player->HasAura(94964))
                                    player->CastSpell(player, 94964, true);
                            }
                }
                checkBattlefieldTimer = 60000;
            }
            else
                checkBattlefieldTimer -= diff;
        }

    private:
        uint64 ArgalothGUID;
        uint64 OccutharGUID;
        uint64 AlizabalGUID;
        uint32 checkBattlefieldTimer;
    };
};

void AddSC_instance_baradin_hold()
{
    new instance_baradin_hold();
}
