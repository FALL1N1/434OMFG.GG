/* ScriptData
SDName: Instance_Shadowfang_Keep
SD%Complete: 60%
SDComment:
SDCategory: Shadowfang Keep
EndScriptData */

#include "InstanceScript.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "shadowfang_keep.h"

TeleportationData const teleportationData[] =
{
    {DATA_TELEPORT_HIGH_HALL,         DATA_BARON_ASHBURY_EVENT         },
    {DATA_TELEPORT_CHAPELLE,          DATA_BARON_SILVERLAINE_EVENT     },
    {DATA_TELEPORT_REMPARTS,          DATA_COMMANDER_SPRINGVALE_EVENT  },
    {DATA_TELEPORT_LABO,              DATA_LORD_WALDEN_EVENT           },
    {0,                               0,                               }, // END
};

DoorData const doorData[] =
{
    {GO_BARON_ASHBURY_DOOR,                 DATA_BARON_ASHBURY_EVENT,    DOOR_TYPE_PASSAGE,       BOUNDARY_W   },
    {GO_SORCERS_GATE,                       DATA_LORD_WALDEN_EVENT,      DOOR_TYPE_PASSAGE,       BOUNDARY_W   },
    {GO_LORD_GODFREY_DOOR,                  DATA_LORD_GODFREY_EVENT,     DOOR_TYPE_ROOM,          BOUNDARY_S   },
    {0,                                      0,                          DOOR_TYPE_ROOM,          BOUNDARY_NONE}, // END
};


class instance_shadowfang_keep : public InstanceMapScript
{
public:
    instance_shadowfang_keep() : InstanceMapScript("instance_shadowfang_keep", 33) { }

    struct instance_shadowfang_keep_InstanceMapScript: public InstanceScript
    {
        instance_shadowfang_keep_InstanceMapScript(InstanceMap* map): InstanceScript(map)  { }

        void Initialize()
        {
            SetHeaders(DataHeader);
            SetBossNumber(MAX_ENCOUNTER);
            LoadDoorData(doorData);
            LoadTeleportationData(teleportationData);

            BaronAshburyGUID        = 0;
            BaronSilverlaineGUID    = 0;
            CommanderSpringvaleGUID = 0;
            LordGodfreyGUID         = 0;
            LordWaldenGUID          = 0;
            ApothecaryHummelGUID    = 0;
            ApothecaryBaxterGUID    = 0;
            ApothecaryFryeGUID      = 0;
        }

        void OnCreatureCreate(Creature* creature)
        {
            switch (creature->GetEntry())
            {
                case BOSS_BARON_ASHBURY:
                    BaronAshburyGUID        = creature->GetGUID();
                    break;
                case BOSS_BARON_SILVERLAINE:
                    BaronSilverlaineGUID    = creature->GetGUID();
                    break;
                case BOSS_COMMANDER_SPRINGVALE:
                    CommanderSpringvaleGUID = creature->GetGUID();
                    break;
                case BOSS_LORD_GODFREY:
                    LordGodfreyGUID         = creature->GetGUID();
                    break;
                case BOSS_LORD_WALDEN:
                    LordWaldenGUID          = creature->GetGUID();
                    break;
                case NPC_HUMMEL:
                    ApothecaryHummelGUID    = creature->GetGUID();
                case NPC_BAXTER:
                    ApothecaryBaxterGUID    = creature->GetGUID();
                case NPC_FRYE:
                    ApothecaryFryeGUID      = creature->GetGUID();
                case NPC_HAUNTED_TELEPORTER:
                    AddTeleporter(creature->GetGUID());
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch (go->GetEntry())
            {
                case GO_SORCERS_GATE:
                case GO_BARON_ASHBURY_DOOR:
                case GO_LORD_GODFREY_DOOR:
                    AddDoor(go, true);
                    break;
            }
        }

        void OnPlayerEnter(Player* player)
        {
            if (!TeamInInstance)
                TeamInInstance = player->GetTeam();
        }

        uint32 GetData(uint32 identifier) const
        {
            if (identifier == TEAM_IN_INSTANCE)
                return TeamInInstance;

            return 0;
        }

        uint64 GetData64(uint32 identifier) const
        {
            switch (identifier)
            {
                case DATA_BARON_ASHBURY:
                    return BaronAshburyGUID;
                case DATA_BARON_SILVERLAINE:
                    return BaronSilverlaineGUID;
                case DATA_COMMANDER_SPRINGVALE:
                    return CommanderSpringvaleGUID;
                case DATA_LORD_GODFREY:
                    return LordGodfreyGUID;
                case DATA_LORD_WALDEN:
                    return LordWaldenGUID;
                case NPC_HUMMEL:
                    return ApothecaryHummelGUID;
                case NPC_BAXTER:
                    return ApothecaryBaxterGUID;
                case NPC_FRYE:
                    return ApothecaryFryeGUID;
            }
            return 0;
        }

        bool SetBossState(uint32 type, EncounterState state)
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            return true;
        }

        protected:
            uint32 TeamInInstance;

            uint64 BaronAshburyGUID;
            uint64 BaronSilverlaineGUID;
            uint64 CommanderSpringvaleGUID;
            uint64 LordGodfreyGUID;
            uint64 LordWaldenGUID;
            uint64 ApothecaryHummelGUID;
            uint64 ApothecaryBaxterGUID;
            uint64 ApothecaryFryeGUID;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_shadowfang_keep_InstanceMapScript(map);
    }
};

void AddSC_instance_shadowfang_keep()
{
    new instance_shadowfang_keep();
}
