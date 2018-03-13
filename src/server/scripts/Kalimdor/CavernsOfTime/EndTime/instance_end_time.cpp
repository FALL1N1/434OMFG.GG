#include "ScriptPCH.h"
#include "endTime.h"

#define MAX_ENCOUNTERS 5

DoorData const doorData[] =
{
    { GO_BAINE_WALL,       BOSS_ECHO_OF_BAINE,         DOOR_TYPE_ROOM,         BOUNDARY_S    },
    { 0,                   0,                          DOOR_TYPE_ROOM,         BOUNDARY_NONE }
};

class instance_end_time : public InstanceMapScript
{
public:
    instance_end_time() : InstanceMapScript("instance_end_time", 938) { }

    struct instance_end_time_InstanceMapScript: public InstanceScript
    {
        instance_end_time_InstanceMapScript(InstanceMap *map) : InstanceScript(map)
        {
            SetHeaders(DataHeader);
            SetBossNumber(MAX_ENCOUNTERS);
            LoadDoorData(doorData);
        }

        void Initialize()
        {
            dataEchoOfJainaGUID = 0;
            dataEchoOfBaineGUID = 0;
            dataEchoOfSylvanasGUID = 0;
            dataEchoOfTyrandeGUID = 0;
            dataMurozondGUID = 0;
            dataTyrandeGauntlet = 0;
            fragCount = 0;
            hourGlassOfTimeGUID = 0;
            murosondCacheGUID = 0;
        }

        void FillInitialWorldStates(WorldPacket& data)
        {
            data << uint32(WORLDSTATE_SHOW_FRAGMENTS) << uint32(fragCount < 5);
            data << uint32(WORLDSTATE_FRAGMENTS_COLLECTED) << uint32(fragCount);
        }

        void OnCreatureCreate(Creature* pCreature)
        {
            switch (pCreature->GetEntry())
            {
                case NPC_ECHO_OF_JAINA:
                    dataEchoOfJainaGUID = pCreature->GetGUID();
                    if (fragCount < MAX_FRAGMENTS)
                    {
                        pCreature->SetVisible(false);
                        pCreature->SetReactState(REACT_PASSIVE);
                        pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE);
                    }
                    break;
                case NPC_ECHO_OF_BAINE:
                    dataEchoOfBaineGUID = pCreature->GetGUID();
                    break;
                case NPC_ECHO_OF_SYLVANAS:
                    dataEchoOfSylvanasGUID = pCreature->GetGUID();
                    break;
                case NPC_ECHO_OF_TYRANDE:
                    dataEchoOfTyrandeGUID = pCreature->GetGUID();
                    break;
                case NPC_MUROZOND:
                    dataMurozondGUID = pCreature->GetGUID();
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch (go->GetEntry())
            {
                case GO_MUROZOND_CACHE:
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    murosondCacheGUID = go->GetGUID();
                    break;
                case GO_ET_TELEPORT:
                    break;
                case GO_BAINE_WALL:
                    AddDoor(go, true);
                    break;
                case GO_HOURGLASS_OF_TIME:
                    hourGlassOfTimeGUID = go->GetGUID();
                    break;
                default:
                    break;
            }
        }

        void OnPlayerLeave(Player* player) override
        {
            player->RemoveAurasDueToSpell(101865); // Molten fists periodic
            player->RemoveAurasDueToSpell(101866); // Molten fists proc aura
        }

        uint64 GetData64(uint32 data) const
        {
            switch (data)
            {
                case DATA_ECHO_OF_JAINA_GUID:
                   return dataEchoOfJainaGUID;
                case DATA_ECHO_OF_BAINE_GUID:
                    return dataEchoOfBaineGUID;
                case DATA_ECHO_OF_SYLVANAS_GUID:
                    return dataEchoOfSylvanasGUID;
                case DATA_ECHO_OF_TYRANDE_GUID:
                    return dataEchoOfTyrandeGUID;
                case DATA_MUROZOND_GUID:
                    return dataMurozondGUID;
                case DATA_HOURGLASS_OF_TIME_GUID:
                    return hourGlassOfTimeGUID;
            }
            return 0;
        }

        uint32 GetData(uint32 data) const
        {
            switch (data)
            {
                case DATA_TYRANDE_GAUNTLET:
                   return dataTyrandeGauntlet;
                default:
                    return 0;
            }
            return 0;
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
                case DATA_FRAGMENTS:
                {
                    fragCount++;
                    if (fragCount >= MAX_FRAGMENTS)
                    {
                        if (Creature *jaina = instance->GetCreature(dataEchoOfJainaGUID))
                        {
                            DoUpdateWorldState(WORLDSTATE_SHOW_FRAGMENTS, 0);
                            jaina->SetVisible(true);
                            jaina->SetReactState(REACT_AGGRESSIVE);
                            jaina->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE);
                        }
                    }
                    else
                        DoUpdateWorldState(WORLDSTATE_FRAGMENTS_COLLECTED, fragCount);
                    break;
                }
                case DATA_TYRANDE_GAUNTLET:
                    dataTyrandeGauntlet = data;
                    break;
                default:
                    break;
            }
        }

        bool SetBossState(uint32 type, EncounterState state)
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            switch (type)
            {
                case BOSS_ECHO_OF_JAINA:
                case BOSS_ECHO_OF_BAINE:
                case BOSS_ECHO_OF_SYLVANAS:
                case BOSS_ECHO_OF_TYRANDE:
                    break;
                case BOSS_MUROZOND:
                    if (state == DONE || state == DONE_HM)
                        if (GameObject *go = instance->GetGameObject(murosondCacheGUID))
                            go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    break;
                default:
                    break;
            }

            return true;
        }

        void WriteSaveDataMore(std::ostringstream& data) override
        {
            data << fragCount << ' ' << dataTyrandeGauntlet;
        }

        void ReadSaveDataMore(std::istringstream& data) override
        {
            data >> fragCount;
            data >> dataTyrandeGauntlet;
        }

    private:
        uint64 dataEchoOfJainaGUID;
        uint64 dataEchoOfBaineGUID;
        uint64 dataEchoOfSylvanasGUID;
        uint64 dataEchoOfTyrandeGUID;
        uint64 dataMurozondGUID;
        uint64 hourGlassOfTimeGUID;
        uint64 murosondCacheGUID;
        uint32 fragCount;
        uint32 dataTyrandeGauntlet;
    };

    InstanceScript* GetInstanceScript(InstanceMap *map) const
    {
        return new instance_end_time_InstanceMapScript(map);
    }
};

void AddSC_instance_end_time()
{
    new instance_end_time;
}
