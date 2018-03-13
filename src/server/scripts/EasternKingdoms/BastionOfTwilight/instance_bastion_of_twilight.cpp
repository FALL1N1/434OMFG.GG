
#include "InstanceScript.h"
#include "ScriptMgr.h"
#include "bastion_of_twilight.h"

DoorData const doorData[] =
{
    {GOB_HALFUS_ENTRANCE,                   DATA_HALFUS,                 DOOR_TYPE_ROOM,       BOUNDARY_W   },
    {GOB_HALFUS_EXIT,                       DATA_HALFUS,                 DOOR_TYPE_PASSAGE,    BOUNDARY_S   },
    {GOB_DRAGON_SIBLINGS_DOOR_ENTRANCE,     DATA_VALIONA_THERALION,      DOOR_TYPE_ROOM,       BOUNDARY_W   },
    {GOB_DRAGON_SIBLINGS_DOOR_EXIT,         DATA_VALIONA_THERALION,      DOOR_TYPE_PASSAGE,    BOUNDARY_E   },
    {GOB_ASCENDANT_COUNCIL_ENTRANCE,        DATA_ASCENDANT_COUNCIL,      DOOR_TYPE_ROOM,       BOUNDARY_N   },
    {GOB_ASCENDANT_COUNCIL_EXIT,            DATA_ASCENDANT_COUNCIL,      DOOR_TYPE_PASSAGE,    BOUNDARY_S   },
    {GOB_CHOGALL_ENTRANCE,                  DATA_CHOGALL,                DOOR_TYPE_ROOM,       BOUNDARY_W   },
    {GOB_SINESTRA_DOOR,                     DATA_SINESTRA,               DOOR_TYPE_ROOM,       BOUNDARY_E   },
    {0,                                      0,                          DOOR_TYPE_ROOM,       BOUNDARY_NONE}, // END
};

TeleportationData const teleportationData[] =
{
    {DATA_TELEPORT_VT,                   DATA_VALIONA_THERALION    },
    {DATA_TELEPORT_COUNCIL,              DATA_ASCENDANT_COUNCIL    },
    {DATA_TELEPORT_CHOGALL,              DATA_CHOGALL              },
    {0,                                  0,                        }, // END
};

class instance_bastion_of_twilight : public InstanceMapScript
{
    public:
        instance_bastion_of_twilight() : InstanceMapScript("instance_bastion_of_twilight", 671) { }

        struct instance_bastion_of_twilight_InstanceMapScript : public InstanceScript
        {
            instance_bastion_of_twilight_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
            {
                SetHeaders(DataHeader);
                SetBossNumber(EncounterCount);
                LoadDoorData(doorData);
                LoadTeleportationData(teleportationData);
                halfusGUID           = 0;
                valionaGUID          = 0;
                theralionGUID        = 0;
                ascendantcouncilGUID = 0;
                feludiusGUID         = 0;
                ignaciousGUID        = 0;
                arionGUID            = 0;
                terrastraGUID        = 0;
                monstrosityGUID      = 0;
                chogallGUID          = 0;
                calenGUID            = 0;
                sinestraGUID         = 0;
                chogallExitGUID      = 0;
                chogallHalfusGUID    = 0;
                chogallValionaGUID   = 0;
                chogallCouncilGUID   = 0;
                data_phase           = 1;
                crushState           = 0;
                councilController    = 0;
                wrackDuration        = 60000;
            }

            void OnCreatureCreate(Creature* pCreature)
            {
                switch (pCreature->GetEntry())
                {
                    case NPC_HALFUS_WORMBREAKER:
                        halfusGUID = pCreature->GetGUID();
                        break;
                    case NPC_VALIONA_BOT:
                        valionaGUID = pCreature->GetGUID();
                        break;
                    case NPC_THERALION_BOT:
                        theralionGUID = pCreature->GetGUID();
                        break;
                    case NPC_ASCENDANT_COUNCIL:
                        ascendantcouncilGUID = pCreature->GetGUID();
                        break;
                    case NPC_ASCENDANT_CONTROLLER:
                        councilController = pCreature->GetGUID();
                        break;
                    case NPC_FELUDIUS:
                        feludiusGUID = pCreature->GetGUID();
                        break;
                    case NPC_IGNACIOUS:
                        ignaciousGUID = pCreature->GetGUID();
                        break;
                    case NPC_ARION:
                        arionGUID = pCreature->GetGUID();
                        break;
                    case NPC_TERRASTRA:
                        terrastraGUID = pCreature->GetGUID();
                        break;
                    case NPC_ELEMENTIUM_MONSTROSITY:
                        monstrosityGUID = pCreature->GetGUID();
                        break;
                    case NPC_CHOGALL:
                        chogallGUID = pCreature->GetGUID();
                        break;
                    case NPC_CALEN:
                        calenGUID = pCreature->GetGUID();
                        break;
                    case NPC_SINESTRA:
                        sinestraGUID = pCreature->GetGUID();
                        break;
                    case NPC_CHOGALL_HALFUS:
                        chogallHalfusGUID = pCreature->GetGUID();
                        break;
                    case NPC_CHOGALL_DRAGONS:
                        chogallValionaGUID = pCreature->GetGUID();
                        break;
                    case NPC_CHOGALL_COUNCIL:
                        chogallCouncilGUID = pCreature->GetGUID();
                        break;
                    case NPC_TELEPORTER:
                        pCreature->AddAura(95716, pCreature);
                        AddTeleporter(pCreature->GetGUID());
                        break;
                }
            }

            void OnGameObjectCreate(GameObject* go)
            {
                switch (go->GetEntry())
                {
                    case GOB_HALFUS_ENTRANCE:
                    case GOB_HALFUS_EXIT:
                    case GOB_DRAGON_SIBLINGS_DOOR_ENTRANCE:
                    case GOB_DRAGON_SIBLINGS_DOOR_EXIT:
                    case GOB_ASCENDANT_COUNCIL_ENTRANCE:
                    case GOB_ASCENDANT_COUNCIL_EXIT:
                    case GOB_CHOGALL_ENTRANCE:
                    case GOB_SINESTRA_DOOR:
                        AddDoor(go, true);
                        break;
                    case GOB_TRAPDOOR:
                        chogallExitGUID = go->GetGUID();
                        if (instance->IsHeroic() && IsDoneInHeroic(DATA_CHOGALL))
                            go->SetGoState(GO_STATE_ACTIVE);
                        else
                            go->SetGoState(GO_STATE_READY);
                        break;
                    default:
                        break;
                }
            }

            void SetData(uint32 type, uint32 data)
            {
                switch (type)
                {
                    case DATA_WRACK_DURATION:
                        wrackDuration = data;
                        break;
                    case DATA_AC_PHASE:
                        data_phase = data;
                        break;
                    case DATA_GRAVITY_CRUSH:
                        crushState = data;
                        break;
                    default:
                        break;
                }
            }

            uint32 GetData(uint32 type) const
            {
                switch (type)
                {
                    case DATA_WRACK_DURATION:
                        return wrackDuration;
                    case DATA_AC_PHASE:
                        return data_phase;
                    case DATA_GRAVITY_CRUSH:
                        return crushState;
                    default:
                        return 0;
                }
            }

            uint64 GetData64(uint32 type) const
            {
                switch (type)
                {
                    case NPC_HALFUS_WORMBREAKER:
                        return halfusGUID;
                    case NPC_VALIONA_BOT:
                        return valionaGUID;
                    case NPC_THERALION_BOT:
                        return theralionGUID;
                    case NPC_ASCENDANT_COUNCIL:
                        return ascendantcouncilGUID;
                    case NPC_ASCENDANT_CONTROLLER:
                        return  councilController;
                    case NPC_FELUDIUS:
                        return feludiusGUID;
                    case NPC_IGNACIOUS:
                        return ignaciousGUID;
                    case NPC_ARION:
                        return arionGUID;
                    case NPC_TERRASTRA:
                        return terrastraGUID;
                    case NPC_ELEMENTIUM_MONSTROSITY:
                        return monstrosityGUID;
                    case NPC_CHOGALL:
                        return chogallGUID;
                    case NPC_CALEN:
                        return calenGUID;
                    case NPC_SINESTRA:
                        return sinestraGUID;
                    case NPC_CHOGALL_HALFUS:
                        return chogallHalfusGUID;
                    case NPC_CHOGALL_DRAGONS:
                        return chogallValionaGUID;
                    case NPC_CHOGALL_COUNCIL:
                        return chogallCouncilGUID;
                    default:
                        break;
                }
                return 0;
            }

            bool SetBossState(uint32 data, EncounterState state)
            {
                if (!InstanceScript::SetBossState(data, state))
                    return false;

                switch(data)
                {
                    case DATA_HALFUS:
                    case DATA_VALIONA_THERALION:
                    case DATA_ASCENDANT_COUNCIL:
                    case DATA_SINESTRA:
                        break;
                    case DATA_CHOGALL:
                        if ((state == DONE || state == DONE_HM) && instance->IsHeroic())
                            if (GameObject* trapDoor = instance->GetGameObject(chogallExitGUID))
                                trapDoor->SetGoState(GO_STATE_ACTIVE);
                        break;
                }
                return true;
            }

        private:
            uint8 data_phase;
            uint8 crushState;
            uint32 wrackDuration;
            uint64 halfusGUID;
            uint64 valionaGUID;
            uint64 theralionGUID;
            uint64 ascendantcouncilGUID;
            uint64 feludiusGUID;
            uint64 ignaciousGUID;
            uint64 arionGUID;
            uint64 terrastraGUID;
            uint64 monstrosityGUID;
            uint64 chogallGUID;
            uint64 calenGUID;
            uint64 sinestraGUID;
            uint64 chogallExitGUID;
            uint64 chogallHalfusGUID;
            uint64 chogallValionaGUID;
            uint64 chogallCouncilGUID;
            uint64 councilController;
        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_bastion_of_twilight_InstanceMapScript(map);
        }
};

void AddSC_instance_bastion_of_twilight()
{
    new instance_bastion_of_twilight();
}
