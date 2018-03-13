
#include "InstanceScript.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "throne_of_the_tides.h"

#define ENCOUNTERS 5

Position const AquaEvent[11] =
{
    {-442.99f, 770.45f, 272.0f, 0.01745329f},
    {-441.00f, 770.45f, 272.0f, 0.01745329f},
    {-440.99f, 830.45f, 272.0f, 0.01745329f},
    {-441.04f, 830.45f, 272.0f, 0.01745329f},
    {-18.99f, 802.45f, 808.5f, 3.14745329f},
    {-134.99f, 820.45f, 803.0f, 0.01745329f},
    {-134.00f, 780.45f, 803.0f, 0.01745329f},
    {-97.99f, 820.45f, 803.0f, 0.01745329f},
    {-97.04f, 780.45f, 803.0f, 0.01745329f},
    {-68.99f, 820.45f, 803.0f, 0.01745329f},
    {-68.00f, 780.45f, 803.0f, 0.01745329f}
};

class instance_throne_of_the_tides: public InstanceMapScript
{
public:
    instance_throne_of_the_tides() :
        InstanceMapScript("instance_throne_of_the_tides", 643) {}

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_throne_of_the_tides_InstanceMapScript(map);
    }

    struct instance_throne_of_the_tides_InstanceMapScript: public InstanceScript
    {
        instance_throne_of_the_tides_InstanceMapScript(InstanceMap* map) : InstanceScript(map) {}

        void Initialize()
        {
            uiLadyNazjar = 0;
            uiCommanderUlthok = 0;
            uiErunakStonespeaker = 0;
            uiMindbenderGhrusha = 0;
            uiNeptulon = 0;
            uiOzumat = 0;
            uiProgressEvent = 0;
            go_abyssal = 0;
            go_abyssal2 = 0;
            go_abyssal3 = 0;
            go_abyssal4 = 0;
            go_corail = 0;
            go_console = 0;
            go_ozumat_chest = 0;
            go_tentacle_left = 0;
            go_tentacle_right = 0;
            go_wall_tentacle_left = 0;
            go_wall_tentacle_right = 0;
            uiBossOzumatEvent = 0;

            startUlthorEvent = false;
            mui_timerUlthokEvent = 5000;

            startAreaTriggerEvent = false;
            mui_timerAreaTriggerEvent = 3600000;

            mui_timerAreaTriggerEvent = 3600000;
            startAreaTriggerEvent = false;

            TeamInInstance = 0;

            for (uint8 i = 0; i < ENCOUNTERS; ++i)
                uiEncounter[i] = NOT_STARTED;
        }

        void OnPlayerEnter(Player* player)
        {
            if (!TeamInInstance)
                TeamInInstance = player->GetTeam();
        }

        bool IsEncounterInProgress() const
        {
            for (uint8 i = 0; i < ENCOUNTERS; ++i)
            {
                if (uiEncounter[i] == IN_PROGRESS)
                    return true;
            }
            return false;
        }

        void OnCreatureCreate(Creature* pCreature)
        {
            if (!TeamInInstance)
            {
                Map::PlayerList const &players = instance->GetPlayers();
                if (!players.isEmpty())
                    if (Player* player = players.begin()->getSource())
                        TeamInInstance = player->GetTeam();
            }

            switch (pCreature->GetEntry())
            {
                case BOSS_LADY_NAZJAR:
                    uiLadyNazjar = pCreature->GetGUID();
                    break;
                case BOSS_COMMANDER_ULTHOK:
                    uiCommanderUlthok = pCreature->GetGUID();
                    break;
                case BOSS_ERUNAK_STONESPEAKER:
                    uiErunakStonespeaker = pCreature->GetGUID();
                    break;
                case BOSS_MINDBENDER_GHURSHA:
                    uiMindbenderGhrusha = pCreature->GetGUID();
                    break;
                case BOSS_OZUMAT:
                    uiOzumat = pCreature->GetGUID();
                    break;
                case BOSS_NEPTULON:
                    uiNeptulon = pCreature->GetGUID();
                    break;
                case NPC_CINEMATIC_TRIGGER:
                    cinematicTrigger.push_back(pCreature->GetGUID());
                    break;
                case BOSS_OZUMAT_EVENT:
                    uiBossOzumatEvent = pCreature->GetGUID();
                    break;
                case NPC_CAPTAIN_TAYLOR:
                    if (TeamInInstance == HORDE)
                        pCreature->UpdateEntry(NPC_LEGIONNAIRE_NAZGRIM);
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch (go->GetEntry())
            {
            case GO_ABYSSAL_MAW_01 :
                go_abyssal = go->GetGUID();
                HandleGameObject(go_abyssal, true, go);
                break;
            case GO_ABYSSAL_MAW_02 :
                go_abyssal2 = go->GetGUID();
                HandleGameObject(go_abyssal2, true, go);
                break;
            case GO_ABYSSAL_MAW_03 :
                go_abyssal3 = go->GetGUID();
                HandleGameObject(go_abyssal3, false, go);
                break;
            case GO_ABYSSAL_MAW_04 :
                go_abyssal4 = go->GetGUID();
                HandleGameObject(go_abyssal4, false, go);
                break;
            case GO_CORAIL:
                go_corail = go->GetGUID();
                break;
            case GO_CONSOLE:
                go_console = go->GetGUID();
                HandleGameObject(go_console, false, go);
                break;
            case GO_OZUMAT_CHEST_NORMAL:
            case GO_OZUMAT_CHEST_HEROIC:
                go_ozumat_chest = go->GetGUID();
                //                HandleGameObject(go_ozumat_chest, false, go);
                break;
            case GO_TENTACLE_LEFT:
                go_tentacle_left = go->GetGUID();
                break;
            case GO_TENTACLE_RIGHT:
                go_tentacle_right = go->GetGUID();
                break;
            case GO_WALL_TENTACLE_LEFT:
                go_wall_tentacle_left = go->GetGUID();
                HandleGameObject(go_wall_tentacle_left, false, go);
                break;
            case GO_WALL_TENTACLE_RIGHT:
                go_wall_tentacle_right = go->GetGUID();
                HandleGameObject(go_wall_tentacle_right, false, go);
                break;
            }
        }

        uint64 GetData64(uint32 identifier) const
        {
            switch (identifier)
            {
            case DATA_LADY_NAZJAR:
                return uiLadyNazjar;
            case DATA_COMMANDER_ULTHOK:
                return uiCommanderUlthok;
            case DATA_ERUNAK_STONESPEAKER:
                return uiErunakStonespeaker;
            case DATA_MINDBENDER_GHURSHA:
                return uiMindbenderGhrusha;
            case DATA_OZUMAT:
                return uiOzumat;
            case DATA_NEPTULON:
                return uiNeptulon;
            case GO_ABYSSAL_MAW_01:
                return go_abyssal;
            case GO_ABYSSAL_MAW_02:
                return go_abyssal2;
            case GO_ABYSSAL_MAW_03:
                return go_abyssal3;
            case GO_ABYSSAL_MAW_04:
                return go_abyssal4;
            case GO_CORAIL:
                return go_corail;
            case GO_CONSOLE:
                return go_console;
            case GO_OZUMAT_CHEST_NORMAL:
            case GO_OZUMAT_CHEST_HEROIC:
                return go_ozumat_chest;
            case GO_TENTACLE_LEFT:
                return go_tentacle_left;
            case GO_TENTACLE_RIGHT:
                return go_tentacle_right;
            case GO_WALL_TENTACLE_LEFT:
                return go_wall_tentacle_left;
            case GO_WALL_TENTACLE_RIGHT:
                return go_wall_tentacle_right;
            case BOSS_OZUMAT_EVENT:
                return uiBossOzumatEvent;
            default :
                return 0;
            }
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
            case DATA_LADY_NAZJAR:
                uiEncounter[0] = data;
                if (data == NOT_STARTED)
                {
                    if (GameObject* go = instance->GetGameObject(go_console))
                        HandleGameObject(go_console, false, go);
                    if (GameObject* go = instance->GetGameObject(go_abyssal2))
                        HandleGameObject(go_abyssal2, true, go);
                }
                else if (data == IN_PROGRESS)
                {
                    if (GameObject* go = instance->GetGameObject(go_console))
                        HandleGameObject(go_console, true, go);
                    if (GameObject* go = instance->GetGameObject(go_abyssal2))
                        HandleGameObject(go_abyssal2, false, go);
                }
                else if (data == DONE)
                    if (GameObject* go = instance->GetGameObject(go_abyssal2))
                        HandleGameObject(go_abyssal2, false, go);
                break;
            case DATA_COMMANDER_ULTHOK:
                if (data == NOT_STARTED)
                {
                    if (GameObject* go = instance->GetGameObject(go_abyssal))
                        HandleGameObject(go_abyssal, true, go);
                    if (GameObject* go = instance->GetGameObject(go_abyssal2))
                        HandleGameObject(go_abyssal2, true, go);
                    if (GameObject* go = instance->GetGameObject(go_abyssal4))
                        HandleGameObject(go_abyssal4, true, go);
                    if (GameObject* go = instance->GetGameObject(go_abyssal3))
                        HandleGameObject(go_abyssal3, true, go);
                }
                else if (data == IN_PROGRESS)
                {
                    if (GameObject* go = instance->GetGameObject(go_abyssal))
                        HandleGameObject(go_abyssal, false, go);
                    if (GameObject* go = instance->GetGameObject(go_abyssal2))
                        HandleGameObject(go_abyssal2, false, go);
                    if (GameObject* go = instance->GetGameObject(go_abyssal4))
                        HandleGameObject(go_abyssal4, false, go);
                    if (GameObject* go = instance->GetGameObject(go_abyssal3))
                        HandleGameObject(go_abyssal3, false, go);
                }
                else if (data == DONE)
                {
                    if (GameObject* go = instance->GetGameObject(go_abyssal))
                        HandleGameObject(go_abyssal, true, go);
                    if (GameObject* go = instance->GetGameObject(go_abyssal2))
                        HandleGameObject(go_abyssal2, true, go);
                    if (GameObject* go = instance->GetGameObject(go_abyssal4))
                        HandleGameObject(go_abyssal4, true, go);
                    if (GameObject* go = instance->GetGameObject(go_abyssal3))
                        HandleGameObject(go_abyssal3, true, go);
                    if (GameObject* go = instance->GetGameObject(go_wall_tentacle_left))
                    {
                        HandleGameObject(go_wall_tentacle_left, false, go);
                        go->Delete();
                    }
                    if (GameObject* go = instance->GetGameObject(go_wall_tentacle_right))
                    {
                        HandleGameObject(go_wall_tentacle_right, false, go);
                        go->Delete();
                    }
                }
                uiEncounter[1] = data;
                break;
            case DATA_ERUNAK_STONESPEAKER:
                uiEncounter[2] = data;
                break;
            case DATA_MINDBENDER_GHURSHA:
                uiEncounter[3] = data;
                break;
            case DATA_OZUMAT:
                uiEncounter[4] = data;
                if (data == DONE)
                    if (GameObject* go = instance->GetGameObject(go_ozumat_chest))
                        go->SetRespawnTime(go->GetRespawnDelay());
                break;
            case DATA_PROGRESS_EVENT:
                uiProgressEvent = data;
                switch (uiProgressEvent)
                {
                    case 1:
                    {
                        aquariumEventMob.clear();
                        if (Creature *c = instance->SummonCreature(40584, AquaEvent[3]))
                        {
                            c->SetReactState(REACT_PASSIVE);
                            c->CastSpell(c, 74048, true);
                            c->GetMotionMaster()->MoveJump(-411.89f, 810.28f, 249.0f, 20.0f, 20.0f);
                            aquariumEventMob.push_back(c->GetGUID());
                        }
                        if (Creature *c = instance->SummonCreature(41139, AquaEvent[2]))
                        {
                            c->SetReactState(REACT_PASSIVE);
                            c->CastSpell(c, 74048, true);
                            c->GetMotionMaster()->MoveJump(-408.20f, 807.12f, 249.0f, 20.0f, 20.0f);
                            aquariumEventMob.push_back(c->GetGUID());
                        }
                        if (Creature *c = instance->SummonCreature(40584, AquaEvent[1]))
                        {
                            c->SetReactState(REACT_PASSIVE);
                            c->CastSpell(c, 74048, true);
                            c->GetMotionMaster()->MoveJump(-410.44f, 798.90f, 249.0f, 20.0f, 20.0f);
                            aquariumEventMob.push_back(c->GetGUID());
                        }
                        if (Creature *c = instance->SummonCreature(41139, AquaEvent[0]))
                        {
                            c->SetReactState(REACT_PASSIVE);
                            c->CastSpell(c, 74048, true);
                            c->GetMotionMaster()->MoveJump(-407.78f, 803.42f, 249.0f, 20.0f, 20.0f);
                            aquariumEventMob.push_back(c->GetGUID());
                        }
                        mui_timerAreaTriggerEvent = 2800;
                        startAreaTriggerEvent = true;
                        break;
                    }
                    case 2 :
                    {
                        aquariumEventMob.clear();
                        if (Creature *c = instance->SummonCreature(40584, AquaEvent[5]))
                        {
                            c->CastSpell(c, 74048, true);
                            c->GetMotionMaster()->MoveJump(-134.0f, 804.0f, 797.0f, 15.0f, 15.0f);
                            aquariumEventMob.push_back(c->GetGUID());
                        }
                        if (Creature *c = instance->SummonCreature(40584, AquaEvent[6]))
                        {
                            c->CastSpell(c, 74048, true);
                            c->GetMotionMaster()->MoveJump(-134.0f, 799.0f, 797.0f, 15.0f, 15.0f);
                            aquariumEventMob.push_back(c->GetGUID());
                        }
                        if (Creature *c = instance->SummonCreature(40584, AquaEvent[7]))
                        {
                            c->CastSpell(c, 74048, true);
                            c->GetMotionMaster()->MoveJump(-97.0f, 804.0f, 797.0f, 15.0f, 15.0f);
                            aquariumEventMob.push_back(c->GetGUID());
                        }
                        if (Creature *c = instance->SummonCreature(41139, AquaEvent[8]))
                        {
                            c->CastSpell(c, 74048, true);
                            c->GetMotionMaster()->MoveJump(-97.0f, 799.0f, 797.0f, 15.0f, 15.0f);
                            aquariumEventMob.push_back(c->GetGUID());
                        }
                        if (Creature *c = instance->SummonCreature(41139, AquaEvent[9]))
                        {
                            c->CastSpell(c, 74048, true);
                            c->GetMotionMaster()->MoveJump(-68.0f, 804.0f, 797.0f, 15.0f, 15.0f);
                            aquariumEventMob.push_back(c->GetGUID());
                        }
                        if (Creature *c = instance->SummonCreature(41139, AquaEvent[10]))
                        {
                            c->CastSpell(c, 74048, true);
                            c->GetMotionMaster()->MoveJump(-68.0f, 799.0f, 797.0f, 15.0f, 15.0f);
                            aquariumEventMob.push_back(c->GetGUID());
                        }
                        mui_timerAreaTriggerEvent = 1500;
                        startAreaTriggerEvent = true;
                        break;
                    }
                    case 3:
                    {
                        for (std::list<uint64 >::const_iterator itr = cinematicTrigger.begin(); itr != cinematicTrigger.end(); itr++)
                            if (Creature *c = instance->GetCreature(*itr))
                                c->CastSpell(c, 85170, true);
                        if (Creature *c = instance->GetCreature(uiBossOzumatEvent))
                        {
                            c->CastSpell(c, 85170, true);
                            mui_timerCinematicEvent = 15000;
                            startCinematicEvent = true;
                        }
                        if (GameObject* go = instance->GetGameObject(GetData64(GO_ABYSSAL_MAW_02)))
                            HandleGameObject(GetData64(GO_ABYSSAL_MAW_02), true, go);
                        break;
                    }
                    default :
                        break;
                }
                break;
            case DATA_EVENT_COMMANDER_ULTHOK_START:
                startUlthorEvent = true;
                break;
            default :
                break;
            }
            if (type != DATA_PROGRESS_EVENT)
                if (data == DONE)
                {
                    OUT_SAVE_INST_DATA;

                    std::ostringstream saveStream;
                    saveStream << "T D" << uiEncounter[0] << ' ' << uiEncounter[1] << ' ' << uiEncounter[2] << ' '
                               << uiEncounter[3] << uiEncounter[4];

                    strInstData = saveStream.str();

                    SaveToDB();
                    OUT_SAVE_INST_DATA_COMPLETE;
                }
        }

        uint32 GetData(uint32 type) const
        {
            switch (type)
            {
                case DATA_LADY_NAZJAR_EVENT:
                    return uiEncounter[0];
                case DATA_COMMANDER_ULTHOK_EVENT:
                    return uiEncounter[1];
                case DATA_ERUNAK_STONESPEAKER_EVENT:
                   return uiEncounter[2];
                case DATA_MINDBENDER_GHURSHA_EVENT:
                    return uiEncounter[3];
                case DATA_OZUMAT_EVENT:
                    return uiEncounter[4];
                case DATA_PROGRESS_EVENT:
                    return uiProgressEvent;
            }
            return 0;
        }

        std::string GetSaveData()
        {

            return strInstData;
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
            std::istringstream loadStream(in);
            loadStream >> dataHead1 >> dataHead2 >> uiEncounter[0] >> uiEncounter[1] >> uiEncounter[2] >> uiEncounter[3] >> uiEncounter[4];
            if (dataHead1 == 'T' && dataHead2 == 'D')
            {
                for (uint8 i = 0; i < ENCOUNTERS; ++i)
                    if (uiEncounter[i] == IN_PROGRESS)
                        uiEncounter[i] = NOT_STARTED;
            }
            else OUT_LOAD_INST_DATA_FAIL;
            OUT_LOAD_INST_DATA_COMPLETE;
        }

        void Update(uint32 diff)
        {

            if (startCinematicEvent)
            {
                if (mui_timerCinematicEvent <= diff)
                {
                    if (Creature *c = instance->GetCreature(uiBossOzumatEvent))
                    {
                        c->GetMotionMaster()->MovePoint(0, 271.88f, 992.52f, 368.13f);
                        c->DespawnOrUnsummon(30000);
                    }
                    if (GameObject* go = instance->GetGameObject(go_tentacle_left))
                        go->Delete();
                    if (GameObject* go = instance->GetGameObject(go_tentacle_right))
                        go->Delete();
                    startCinematicEvent = false;
                    mui_timerCinematicEvent = 3600000;
                }
                else
                    mui_timerCinematicEvent -= diff;
            }

            if (startAreaTriggerEvent)
            {
                if (mui_timerAreaTriggerEvent <= diff)
                {
                    for (std::list<uint64 >::const_iterator itr = aquariumEventMob.begin(); itr != aquariumEventMob.end(); itr++)
                        if (Creature *c = instance->GetCreature(*itr))
                        {
                            c->SetHomePosition(c->GetPositionX(), c->GetPositionY(), c->GetPositionZ(), 3.04f);
                            c->RemoveAura(74048);
                            c->SetFacingTo(3.04f);
                            c->SetReactState(REACT_AGGRESSIVE);
                            if (CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(c->GetEntry()))
                                c->LoadEquipment(creatureInfo->equipmentId, true);
                            c->SetUInt32Value(UNIT_NPC_EMOTESTATE, 425);
                        }
                    mui_timerAreaTriggerEvent = 3600000;
                    startAreaTriggerEvent = false;
                }
                else
                    mui_timerAreaTriggerEvent -= diff;
            }

            if (startUlthorEvent)
            {
                if (mui_timerUlthokEvent <= diff)
                {
                    if (!instance->GetCreature(uiCommanderUlthok))
                    {
                        bool start_event = true;
                        Map::PlayerList const &PlayerList = instance->GetPlayers();
                        if (!PlayerList.isEmpty())
                            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                                if (Player* player = i->getSource())
                                    if (player->IsWatchingCinematic())
                                    {
                                        start_event = false;
                                        break;
                                    }
                        startUlthorEvent = false;
                    }
                    mui_timerUlthokEvent = 5000;
                }
                else
                    mui_timerUlthokEvent -= diff;
            }
        }

    private :
        uint32 uiEncounter[ENCOUNTERS];
        uint32 uiProgressEvent;

        uint64 uiLadyNazjar;
        uint64 uiCommanderUlthok;
        uint64 uiErunakStonespeaker;
        uint64 uiMindbenderGhrusha;
        uint64 uiOzumat;
        uint64 uiNeptulon;
        uint64 uiBossOzumatEvent;

        uint64 go_abyssal;
        uint64 go_abyssal2;
        uint64 go_abyssal3;
        uint64 go_abyssal4;
        uint64 go_corail;
        uint64 go_console;
        uint64 go_ozumat_chest;

        uint64 go_tentacle_left;
        uint64 go_tentacle_right;

        uint64 go_wall_tentacle_left;
        uint64 go_wall_tentacle_right;

        bool startUlthorEvent;
        bool startAreaTriggerEvent;
        bool startCinematicEvent;
        uint32 mui_timerCinematicEvent;
        uint32 mui_timerUlthokEvent;
        uint32 mui_timerAreaTriggerEvent;
        std::string strInstData;

        std::list<uint64 > aquariumEventMob;
        std::list<uint64 > cinematicTrigger;

        uint32 TeamInInstance;
    };
};

void AddSC_instance_throne_of_the_tides()
{
    new instance_throne_of_the_tides();
}
