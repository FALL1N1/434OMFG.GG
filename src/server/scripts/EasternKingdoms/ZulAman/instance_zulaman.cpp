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
SDName: instance_zulaman
SD%Complete: 80
SDComment:
SDCategory: Zul'Aman
EndScriptData */

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "zulaman.h"
#include "Player.h"
#include "TemporarySummon.h"

enum Misc
{
    MAX_ENCOUNTER                  = 7,
    RAND_VENDOR                    = 2,
    WORLDSTATE_SHOW_TIMER          = 3104,
    WORLDSTATE_TIME_TO_SACRIFICE   = 3106
};

// Chests spawn at bear/eagle/dragonhawk/lynx bosses
// The loots depend on how many bosses have been killed, but not the entries of the chests
// But we cannot add loots to gameobject, so we have to use the fixed loot_template
struct SHostageInfo
{
    uint32 npc, go; // FIXME go Not used
    float x, y, z, o;
};

static SHostageInfo HostageInfo[] =
{
    {23999, 187021, 400, 1414, 74.36f, 3.3f}, // eagle
    {23790, 186648, -57, 1343, 40.77f, 3.2f}, // bear
    {24001, 186672, -35, 1134, 18.71f, 1.9f}, // dragonhawk
    {24024, 186667, 413, 1117,  6.32f, 3.1f}  // lynx
};

Position const HarrisonJonesLoc = {120.687f, 1674.0f, 42.0217f, 1.59044f};

DoorData const doorData[] =
{
    { GO_MASSIVE_GATE,                  DATA_GONGEVENT,         DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    { GO_DOOR_HALAZZI_ENTRANCE,         DATA_HALAZZIEVENT,      DOOR_TYPE_ROOM,    BOUNDARY_NONE },
    { GO_DOOR_HALAZZI_EXIT,             DATA_HALAZZIEVENT,      DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    // { GO_GATE_HEXLORD_ENTRANCE,         DATA_HEXLORDEVENT,      DOOR_TYPE_ROOM,    BOUNDARY_NONE },
    { GO_GATE_HEXLORD_EXIT,             DATA_HEXLORDEVENT,      DOOR_TYPE_PASSAGE, BOUNDARY_NONE },
    { GO_DOOR_AKILZON,                  DATA_AKILZONEVENT,      DOOR_TYPE_ROOM,    BOUNDARY_NONE },
    { GO_DOOR_DAAKARA,                  DATA_DAAKARAEVENT,      DOOR_TYPE_ROOM,    BOUNDARY_NONE },
    { 0,                                0,                      DOOR_TYPE_ROOM,    BOUNDARY_NONE }
};

class instance_zulaman : public InstanceMapScript
{
    public:
        instance_zulaman()
            : InstanceMapScript("instance_zulaman", 568)
        {
        }

        struct instance_zulaman_InstanceMapScript : public InstanceScript
        {
            instance_zulaman_InstanceMapScript(Map* map) : InstanceScript(map) {}

            uint64 HarkorsSatchelGUID;
            uint64 TanzarsTrunkGUID;
            uint64 AshlisBagGUID;
            uint64 KrazsPackageGUID;
            uint64 StrangeGongGUID;
            uint64 HarrisonJonesGUID;
            uint64 HexlordGUID;

            uint64 HexLordGateGUID;
            uint64 DaakaraGateGUID;
            uint64 MassiveGateGUID;
            uint64 AkilzonDoorGUID;
            uint64 DaakaraDoorGUID;
            uint64 HalazziDoorGUID;

            uint64 uiLynxSpirit;
            uint64 uiBearSpirit;
            uint64 uiDragonhawkSpirit;
            uint64 uiEagleSpirit;

            uint32 QuestTimer;
            uint16 BossKilled;
            uint16 QuestMinute;
            uint16 ChestLooted;

            uint32 RandVendor[RAND_VENDOR];
            uint64 HostageGUID[4];

            void Initialize()
            {
                SetHeaders(DataHeader);
                SetBossNumber(EncounterCount);
                LoadDoorData(doorData);

                HarkorsSatchelGUID = 0;
                TanzarsTrunkGUID = 0;
                AshlisBagGUID = 0;
                KrazsPackageGUID = 0;
                StrangeGongGUID = 0;
                HexLordGateGUID = 0;
                DaakaraGateGUID = 0;
                MassiveGateGUID = 0;
                AkilzonDoorGUID = 0;
                HalazziDoorGUID = 0;
                DaakaraDoorGUID = 0;

                HarrisonJonesGUID = 0;
                HexlordGUID = 0;

                QuestTimer = 0;
                QuestMinute = 0;
                BossKilled = 0;
                ChestLooted = 0;

                for (uint8 i = 0; i < RAND_VENDOR; ++i)
                    RandVendor[i] = NOT_STARTED;
                for (int i = 0; i < 4; i++)
                    HostageGUID[i] = 0;
            }

            void OnPlayerEnter(Player* /*player*/)
            {
                if (!HarrisonJonesGUID)
                    instance->SummonCreature(NPC_HARRISON_JONES, HarrisonJonesLoc);
            }

            void OnCreatureCreate(Creature* creature)
            {
                switch (creature->GetEntry())
                {
                    case NPC_HARRISON_JONES:
                        HarrisonJonesGUID = creature->GetGUID();
                        break;
                    case NPC_LYNX_SPIRIT:
                        uiLynxSpirit = creature->GetGUID();
                        break;
                    case NPC_BEAR_SPIRIT:
                        uiBearSpirit = creature->GetGUID();
                        break;
                    case NPC_DRAGONHAWK_SPIRIT:
                        uiDragonhawkSpirit = creature->GetGUID();
                        break;
                    case NPC_EAGLE_SPIRIT:
                        uiEagleSpirit = creature->GetGUID();
                        break;
                    case NPC_HOSTAGE_1:
                        HostageGUID[0] = creature->GetGUID();
                        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        break;
                    case NPC_HOSTAGE_2:
                        HostageGUID[1] = creature->GetGUID();
                        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        break;
                    case NPC_HOSTAGE_3:
                        HostageGUID[2] = creature->GetGUID();
                        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        break;
                    case NPC_HOSTAGE_4:
                        HostageGUID[3] = creature->GetGUID();
                        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        break;
                    case NPC_HEXLORD:
                        HexlordGUID = creature->GetGUID();
                        break;
                    case NPC_JANALAI:
                    case NPC_DAAKARA:
                    case NPC_HALAZZI:
                    case NPC_NALORAKK:
                    default:
                        break;
                }
                CheckInstanceStatus();
            }

            void OnGameObjectCreate(GameObject* go)
            {
                switch (go->GetEntry())
                {
                    case GO_DOOR_HALAZZI_ENTRANCE:
                        AddDoor(go, true);
                        break;
                    case GO_DOOR_HALAZZI_EXIT:
                        AddDoor(go, true);
                        HalazziDoorGUID = go->GetGUID();
                        break;
                    case GO_GATE_HEXLORD_EXIT:
                        AddDoor(go, true);
                        break;
                    case GO_GATE_HEXLORD_ENTRANCE:
                        HexLordGateGUID = go->GetGUID();
                        break;
                    case GO_MASSIVE_GATE:
                        MassiveGateGUID = go->GetGUID();
                        break;
                    case GO_DOOR_AKILZON:
                        AddDoor(go, true);
                        AkilzonDoorGUID = go->GetGUID();
                        break;
                    case GO_DOOR_DAAKARA:
                        AddDoor(go, true);
                        DaakaraDoorGUID = go->GetGUID();
                        break;

                    case GO_HARKORS_SATCHEL:
                        HarkorsSatchelGUID = go->GetGUID();
                        go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED | GO_FLAG_NOT_SELECTABLE | GO_FLAG_NODESPAWN);
                        break;
                    case GO_TANZARS_TRUNK:
                        TanzarsTrunkGUID = go->GetGUID();
                        go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED | GO_FLAG_NOT_SELECTABLE | GO_FLAG_NODESPAWN);
                        break;
                    case GO_KRAZS_PACKAGE:
                        KrazsPackageGUID = go->GetGUID();
                        go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED | GO_FLAG_NOT_SELECTABLE | GO_FLAG_NODESPAWN);
                        break;
                    case GO_ASHLIS_BAG:
                        AshlisBagGUID = go->GetGUID();
                        go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED | GO_FLAG_NOT_SELECTABLE | GO_FLAG_NODESPAWN);
                        break;

                    case GO_STRANGE_GONG:
                        StrangeGongGUID = go->GetGUID();
                        break;
                    default: break;
                }
                CheckInstanceStatus();
            }

            uint32 CheckChestAccess()
            {
                uint32 bossCount = 0;
                if (IsDone(DATA_NALORAKKEVENT))
                    bossCount++;
                if (IsDone(DATA_AKILZONEVENT))
                    bossCount++;
                if (IsDone(DATA_JANALAIEVENT))
                    bossCount++;
                if (IsDone(DATA_HALAZZIEVENT))
                    bossCount++;
                return bossCount;
            }

            void FreeHostage(uint8 num)
            {
                if (!QuestMinute)
                    return;

                Map::PlayerList const &PlayerList = instance->GetPlayers();
                if (PlayerList.isEmpty())
                    return;

                if (Creature* Hostage = instance->GetCreature(HostageGUID[num]))
                {
                    Hostage->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    Hostage->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                }
            }

            void SacrifiedHostage()
            {
                if (Creature* Hostage = instance->GetCreature(HostageGUID[0]))
                {
                    Hostage->UpdateEntry(NPC_HOSTAGE_CORPSE_1);
                    Hostage->CastSpell(Hostage, 42726, true);
                }
                if (Creature* Hostage = instance->GetCreature(HostageGUID[1]))
                {
                    Hostage->UpdateEntry(NPC_HOSTAGE_CORPSE_2);
                    Hostage->CastSpell(Hostage, 42726, true);
                }
                if (Creature* Hostage = instance->GetCreature(HostageGUID[2]))
                {
                    Hostage->UpdateEntry(NPC_HOSTAGE_CORPSE_3);
                    Hostage->CastSpell(Hostage, 42726, true);
                }
                if (Creature* Hostage = instance->GetCreature(HostageGUID[3]))
                {
                    Hostage->UpdateEntry(NPC_HOSTAGE_CORPSE_4);
                    Hostage->CastSpell(Hostage, 42726, true);
                }
            }

            void CheckInstanceStatus()
            {
                if (IsDone(DATA_NALORAKKEVENT) && IsDone(DATA_AKILZONEVENT) && IsDone(DATA_JANALAIEVENT) && IsDone(DATA_HALAZZIEVENT))
                {
                    if (Creature* hexlord = instance->GetCreature(HexlordGUID))
                        hexlord->SetVisible(true);

                    HandleGameObject(HexLordGateGUID, true);
                }
                else
                {
                    if (Creature* hexlord = instance->GetCreature(HexlordGUID))
                        hexlord->SetVisible(false);

                    HandleGameObject(HexLordGateGUID, false);
                }

            }

            void WriteSaveDataMore(std::ostringstream& data) override
            {
                data << ' ' << QuestMinute << ' ' << ChestLooted;
            }

            void ReadSaveDataMore(std::istringstream& data) override
            {
                uint32 tmpState;
                data >> tmpState;
                QuestMinute = tmpState;
                if (QuestMinute)
                    DoUpdateWorldState(WORLDSTATE_SHOW_TIMER, 1);
                else
                    DoUpdateWorldState(WORLDSTATE_SHOW_TIMER, 0);
                DoUpdateWorldState(WORLDSTATE_TIME_TO_SACRIFICE, QuestMinute);
                data >> tmpState;
                ChestLooted = tmpState;
            }

            void FillInitialWorldStates(WorldPacket& data)
            {
                data << uint32(WORLDSTATE_SHOW_TIMER) << uint32(0);
                data << uint32(WORLDSTATE_TIME_TO_SACRIFICE) << uint32(QuestMinute);
            }

            bool SetBossState(uint32 type, EncounterState state)
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;

                switch (type)
                {
                    case DATA_AKILZONEVENT:
                    case DATA_NALORAKKEVENT:
                        if (state == DONE || state == DONE_HM)
                        {
                            if (QuestMinute)
                            {
                                QuestMinute += 5;
                                DoUpdateWorldState(WORLDSTATE_TIME_TO_SACRIFICE, QuestMinute);
                            }
                        }
                    case DATA_JANALAIEVENT:
                    case DATA_HALAZZIEVENT:
                        if ((state == DONE || state == DONE_HM) && QuestTimer)
                            FreeHostage(type);
                        break;
                    case DATA_GONGEVENT:
                    {
                        if (state == DONE || state == DONE_HM)
                        {
                            QuestMinute = 15;
                            DoUpdateWorldState(WORLDSTATE_SHOW_TIMER, 1);
                            DoUpdateWorldState(WORLDSTATE_TIME_TO_SACRIFICE, QuestMinute);
                        }
                        break;
                    }
                }

                if (state == DONE || state == DONE_HM)
                    CheckInstanceStatus();

                return true;
            }

            void OpenChest(uint8 chestCount)
            {
                int64 chestGUID = 0;
                switch (chestCount)
                {
                    case 1:
                        chestGUID = HarkorsSatchelGUID;
                        break;
                    case 2:
                        chestGUID = TanzarsTrunkGUID;
                        break;
                    case 3:
                        chestGUID = KrazsPackageGUID;
                        break;
                    case 4:
                        chestGUID = AshlisBagGUID;
                        break;
                }
                if (GameObject* loot = instance->GetGameObject(chestGUID))
                    loot->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED | GO_FLAG_NOT_SELECTABLE | GO_FLAG_NODESPAWN);

            }

            void SetData(uint32 type, uint32 data)
            {
                switch (type)
                {
                    case DATA_CHESTLOOTED:
                        if ((ChestLooted + 1) == CheckChestAccess() && QuestMinute)
                        {
                            ++ChestLooted;
                            OpenChest(ChestLooted);
                            SaveToDB();
                        }
                        break;
                    case TYPE_RAND_VENDOR_1:
                        RandVendor[0] = data;
                        break;
                    case TYPE_RAND_VENDOR_2:
                        RandVendor[1] = data;
                        break;
                }
            }

            uint32 GetData(uint32 type) const
            {
                switch (type)
                {
                    case DATA_CHESTLOOTED:
                        return ChestLooted;
                    case TYPE_RAND_VENDOR_1:
                        return RandVendor[0];
                    case TYPE_RAND_VENDOR_2:
                        return RandVendor[1];
                    default:
                        return 0;
                }
            }

            void Update(uint32 diff)
            {
                if (QuestMinute)
                {
                    if (QuestTimer <= diff)
                    {
                        QuestMinute--;
                        SaveToDB();
                        QuestTimer += 60000;
                        if (QuestMinute)
                        {
                            DoUpdateWorldState(WORLDSTATE_SHOW_TIMER, 1);
                            DoUpdateWorldState(WORLDSTATE_TIME_TO_SACRIFICE, QuestMinute);
                        }
                        else
                        {
                            DoUpdateWorldState(WORLDSTATE_SHOW_TIMER, 0);
                            SacrifiedHostage();
                        }
                    }
                    QuestTimer -= diff;
                }
            }

            uint64 GetData64(uint32 type) const
            {
                switch (type)
                {
                    case GO_STRANGE_GONG:       return StrangeGongGUID;
                    case GO_MASSIVE_GATE:       return MassiveGateGUID;
                    case NPC_LYNX_SPIRIT:       return uiLynxSpirit;
                    case NPC_BEAR_SPIRIT:       return uiBearSpirit;
                    case NPC_DRAGONHAWK_SPIRIT: return uiDragonhawkSpirit;
                    case NPC_EAGLE_SPIRIT:      return uiEagleSpirit;
                }

                return 0;
            }

        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_zulaman_InstanceMapScript(map);
        }
};

void AddSC_instance_zulaman()
{
    new instance_zulaman();
}
