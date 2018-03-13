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

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "ScriptedCreature.h"
#include "Map.h"
#include "PoolMgr.h"
#include "AccountMgr.h"
#include "halls_of_origination.h"
#include "Player.h"
#include "WorldPacket.h"
#include "WorldSession.h"

DoorData const doorData[] =
{
    {GO_ANHUURS_DOOR,                 DATA_TEMPLE_GUARDIAN_ANHUUR, DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    {GO_ANHUURS_BRIDGE,               DATA_TEMPLE_GUARDIAN_ANHUUR, DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    {GO_DOODAD_ULDUM_ELEVATOR_COL01,  DATA_TEMPLE_GUARDIAN_ANHUUR, DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    {GO_VAULT_OF_LIGHTS_DOOR,         DATA_VAULT_OF_LIGHTS,        DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    {GO_DOODAD_ULDUM_LIGHTMACHINE_02, DATA_EARTH_WARDEN,           DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    {GO_DOODAD_ULDUM_LASERBEAMS01,    DATA_EARTH_WARDEN,           DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    {GO_DOODAD_ULDUM_LIGHTMACHINE_01, DATA_FIRE_WARDEN,            DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    {GO_DOODAD_ULDUM_LASERBEAMS_01,   DATA_FIRE_WARDEN,            DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    {GO_DOODAD_ULDUM_LIGHTMACHINE_03, DATA_WATER_WARDEN,           DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    {GO_DOODAD_ULDUM_LASERBEAMS_03,   DATA_WATER_WARDEN,           DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    {GO_DOODAD_ULDUM_LIGHTMACHINE_04, DATA_AIR_WARDEN,             DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    {GO_DOODAD_ULDUM_LASERBEAMS_02,   DATA_AIR_WARDEN,             DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    {0,                              0,                            DOOR_TYPE_ROOM,    BOUNDARY_NONE }
};

class instance_halls_of_origination : public InstanceMapScript
{
    public:
        instance_halls_of_origination() : InstanceMapScript(HoOScriptName, 644) { }

        struct instance_halls_of_origination_InstanceMapScript : public InstanceScript
        {
            instance_halls_of_origination_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
            {
                SetHeaders(DataHeader);
                SetBossNumber(EncounterCount);
                LoadDoorData(doorData);
                TempleGuardianAnhuurGUID = 0;
                AnhuursBridgeGUID = 0;
                AnhuursDoorGUID = 0;
                AnhuurRightBeaconGUID = 0;
                AnhuurLeftBeaconGUID = 0;
                BrannBronzebeardGUID = 0;
                AnraphetGUID = 0;
                AnraphetDoorGUID = 0;
                SunMirrorGUID = 0;
                _deadElementals = 0;
                AstralRainGUID = 0;
                CelestialCallGUID = 0;
                VeilofSkyGUID = 0;
                SeteshGUID = 0;
            }

            void OnGameObjectCreate(GameObject* go)
            {
                switch (go->GetEntry())
                {
                    case GO_ANHUURS_BRIDGE:
                        AnhuursBridgeGUID = go->GetGUID();
                    case GO_DOODAD_ULDUM_ELEVATOR_COL01:
                    case GO_VAULT_OF_LIGHTS_DOOR:
                    case GO_DOODAD_ULDUM_LIGHTMACHINE_01:
                    case GO_DOODAD_ULDUM_LIGHTMACHINE_02:
                    case GO_DOODAD_ULDUM_LIGHTMACHINE_03:
                    case GO_DOODAD_ULDUM_LIGHTMACHINE_04:
                    case GO_DOODAD_ULDUM_LASERBEAMS01:
                    case GO_DOODAD_ULDUM_LASERBEAMS_01:
                    case GO_DOODAD_ULDUM_LASERBEAMS_02:
                    case GO_DOODAD_ULDUM_LASERBEAMS_03:
                        AddDoor(go, true);
                        break;
                    case GO_ANHUURS_DOOR:
                        AnhuursDoorGUID = go->GetGUID();
                        AddDoor(go, true);
                        break;
                    case GO_ANHUURS_RIGHT_BEACON:
                        AnhuurRightBeaconGUID = go->GetGUID();
                        break;
                    case GO_ANHUURS_LEFT_BEACON:
                        AnhuurLeftBeaconGUID = go->GetGUID();
                        break;
                    case GO_SUN_MIRROR:
                        SunMirrorGUID = go->GetGUID();
                        break;
                    case GO_ANRAPHET_DOOR:
                        AnraphetDoorGUID = go->GetGUID();
                        break;
                    case GO_HOO_TELEPORTER:
                        teleporterSet.insert(go->GetGUID());
                        if (IsDone(DATA_TEMPLE_GUARDIAN_ANHUUR))
                            go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                        break;
                }
            }

            void OnGameObjectRemove(GameObject* go)
            {
                switch (go->GetEntry())
                {
                    case GO_ANHUURS_BRIDGE:
                    case GO_DOODAD_ULDUM_ELEVATOR_COL01:
                    case GO_ANHUURS_DOOR:
                    case GO_VAULT_OF_LIGHTS_DOOR:
                    case GO_DOODAD_ULDUM_LIGHTMACHINE_01:
                    case GO_DOODAD_ULDUM_LIGHTMACHINE_02:
                    case GO_DOODAD_ULDUM_LIGHTMACHINE_03:
                    case GO_DOODAD_ULDUM_LIGHTMACHINE_04:
                    case GO_DOODAD_ULDUM_LASERBEAMS01:
                    case GO_DOODAD_ULDUM_LASERBEAMS_01:
                    case GO_DOODAD_ULDUM_LASERBEAMS_02:
                    case GO_DOODAD_ULDUM_LASERBEAMS_03:
                        AddDoor(go, false);
                        break;
                }
            }

            void OnCreatureCreate(Creature* creature)
            {
                switch (creature->GetEntry())
                {
                    case BOSS_TEMPLE_GUARDIAN_ANHUUR:
                        TempleGuardianAnhuurGUID = creature->GetGUID();
                        break;
                    case NPC_BRANN_BRONZEBEARD_0:
                        BrannBronzebeardGUID = creature->GetGUID();
                        break;
                    case BOSS_ANRAPHET:
                        AnraphetGUID = creature->GetGUID();
                        break;
                    case BOSS_ISISET:
                        IsisetGUID = creature->GetGUID();
                        break;
                    case BOSS_AMMUNAE:
                        AmmunaeGUID = creature->GetGUID();
                        break;
                    case BOSS_SETESH:
                        SeteshGUID = creature->GetGUID();
                        break;
                    case BOSS_RAJH:
                        RajhGUID = creature->GetGUID();
                        break;
                    case NPC_ASTRAL_RAIN:
                        AstralRainGUID = creature->GetGUID();
                        break;
                    case NPC_CELESTIAL_CALL:
                        CelestialCallGUID = creature->GetGUID();
                        break;
                    case NPC_VEIL_OF_SKY:
                        VeilofSkyGUID = creature->GetGUID();
                        break;
                }
            }

            uint32 GetData(uint32 data) const
            {
                switch (data)
                {
                    case DATA_DEAD_ELEMENTALS:
                        return _deadElementals;
                    default:
                        break;
                }

                return 0;
            }

            uint64 GetData64(uint32 index) const
            {
                switch (index)
                {
                    case DATA_ANHUUR_BRIDGE:
                        return AnhuursBridgeGUID;
                    case DATA_ANHUUR_DOOR:
                        return AnhuursDoorGUID;
                    case DATA_ANHUUR_LEFT_BEACON:
                        return AnhuurLeftBeaconGUID;
                    case DATA_ANHUUR_RIGHT_BEACON:
                        return AnhuurRightBeaconGUID;
                    case DATA_ANHUUR_GUID:
                        return TempleGuardianAnhuurGUID;
                    case DATA_BRANN_0_GUID:
                        return BrannBronzebeardGUID;
                    case DATA_ANRAPHET_GUID:
                        return AnraphetGUID;
                    case DATA_ASTRAL_RAIN_GUID:
                        return AstralRainGUID;
                    case DATA_CELESTIAL_CALL_GUID:
                        return CelestialCallGUID;
                    case DATA_VEIL_OF_SKY_GUID:
                        return VeilofSkyGUID;
                    case DATA_RAJH_GUID:
                        return RajhGUID;
                    case DATA_SETESH_GUID:
                        return SeteshGUID;
                    case DATA_ANRAPHET_DOOR:
                        return AnraphetDoorGUID;
                }

                return 0;
            }

            void IncreaseDeadElementals(uint8 cnt = 1)
            {
                    _deadElementals += cnt;
                if (_deadElementals == 4)
                {
                    if (GameObject* mirror = instance->GetGameObject(SunMirrorGUID))
                        mirror->SetGoState(GO_STATE_ACTIVE);
                    if (GameObject* door = instance->GetGameObject(AnraphetDoorGUID))
                        door->SetGoState(GO_STATE_ACTIVE);
                }
            }

            void OnUnitDeath(Unit* unit)
            {
                Creature* creature = unit->ToCreature();
                if (!creature)
                    return;

                switch (creature->GetEntry())
                {
                    case NPC_FIRE_WARDEN:
                    case NPC_EARTH_WARDEN:
                    case NPC_WATER_WARDEN:
                    case NPC_AIR_WARDEN:
                        uint32 data = creature->GetEntry() - WARDEN_ENTRY_DATA_DELTA;
                        SetBossState(data, IN_PROGRESS); // Needs to be set to IN_PROGRESS or else the gameobjects state won't be updated
                        SetBossState(data, DONE);
                        IncreaseDeadElementals();
                        if (Creature* brann = instance->GetCreature(BrannBronzebeardGUID))
                            brann->AI()->DoAction(ACTION_ELEMENTAL_DIED);
                        break;
                }
            }

            bool SetBossState(uint32 type, EncounterState state)
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;

                if (type == DATA_TEMPLE_GUARDIAN_ANHUUR && (state == DONE || state == DONE_HM))
                {
                    for (std::set<uint64>::iterator itr = teleporterSet.begin(); itr != teleporterSet.end(); ++itr)
                        if (GameObject* teleporter = instance->GetGameObject((*itr)))
                            teleporter->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                }
                return true;
            }

            void WriteSaveDataMore(std::ostringstream& data) override
            {
                data << _deadElementals;
            }

            void ReadSaveDataMore(std::istringstream& data) override
            {
                uint32 tmp;
                data >> tmp;
                IncreaseDeadElementals(tmp);
            }

        protected:
            uint64 TempleGuardianAnhuurGUID;
            uint64 AnhuursBridgeGUID;
            uint64 AnhuursDoorGUID;
            uint64 AnhuurRightBeaconGUID;
            uint64 AnhuurLeftBeaconGUID;
            uint64 BrannBronzebeardGUID;
            uint64 AnraphetGUID;
            uint64 AnraphetDoorGUID;
            uint64 SunMirrorGUID;
            uint32 _deadElementals;
            uint64 IsisetGUID;
            uint64 BrannGUID;
            uint64 AmmunaeGUID;
            uint64 RajhGUID;
            uint64 SeteshGUID;
            uint64 AstralRainGUID;
            uint64 CelestialCallGUID;
            uint64 VeilofSkyGUID;
            std::set<uint64> teleporterSet;
        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_halls_of_origination_InstanceMapScript(map);
        }
};

void AddSC_instance_halls_of_origination()
{
    new instance_halls_of_origination();
}
