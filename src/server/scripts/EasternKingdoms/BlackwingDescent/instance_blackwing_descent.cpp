#include "blackwing_descent.h"
#include "Object.h"
#include "InstanceScript.h"
#include "ScriptMgr.h"

static const Position NefarianPos = {143.501663f, -241.307678f, 74.920464f, 2.260429f};
static const Position AtramedesPos = {292.893842f, -222.483643f, 61.624100f, 3.14f};

class instance_blackwing_descent : public InstanceMapScript
{
    public:
        instance_blackwing_descent() : InstanceMapScript("instance_blackwing_descent", 669) { }

        struct instance_blackwing_descent_InstanceMapScript : public InstanceScript
        {
            instance_blackwing_descent_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
            {
                SetHeaders(DataHeader);
                SetBossNumber(ENCOUNTER_COUNT);
                drakonidCnt             = 0;
                dwarvesCnt              = 0;
                magmawGUID              = 0;
                nefarianGUID            = 0;
                omnotronGUID            = 0;
                arcanotronGUID          = 0;
                electronGUID            = 0;
                magmatronGUID           = 0;
                toxitronGUID            = 0;
                bellGUID                = 0;
                atramedesGUID           = 0;
                omnotronNefarianGUID    = 0;
                chimaeronGUID           = 0;
                nefarian_elevatorGUID   = 0;
                trgCount = 0;
                onyxiaGUID = 0;
            }

            void OnGameObjectCreate(GameObject* go)
            {
                switch (go->GetEntry())
                {
                    case GO_ANCIENT_BELL:
                        bellGUID = go->GetGUID();
                        if (dwarvesCnt < 7)
                            go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
                        break;
                    case GO_WING_DOOR:
                        if (IsDone(DATA_MAGMAW) && IsDone(DATA_OMNOTRON_DEFENSE_SYSTEM))
                            go->SetGoState(GO_STATE_ACTIVE);
                        else go->SetGoState(GO_STATE_READY);
                        wingDoorGUID = go->GetGUID();
                        break;
                    case GO_NEFARIAN_ELEVATOR:
                        go->SetControlableTransport();
                        go->EnableCollision(true);
                        nefarian_elevatorGUID = go->GetGUID();
                        break;
                    default:
                        break;
                }
            }

            void OnCreatureCreate(Creature* creature)
            {
                switch (creature->GetEntry())
                {
                case NPC_DRAKONID_DRUDGE:
                    if (Creature* magmaw = this->instance->GetCreature(magmawGUID))
                    {
                        if (drakonidCnt < 2)
                        {
                            magmaw->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                            magmaw->SetReactState(REACT_PASSIVE);
                            if (creature->isDead())
                                SetData(DATA_DRAKONID, 1);
                        }
                    }
                    break;
                case NPC_MAGMAW:
                    magmawGUID = creature->GetGUID();
                    break;
                case NPC_ATRAMEDES:
                    atramedesGUID = creature->GetGUID();
                    break;
                case NPC_OMNOTRON:
                    omnotronGUID = creature->GetGUID();
                    break;
                case NPC_ELECTRON:
                    electronGUID = creature->GetGUID();
                    break;
                case NPC_TOXITRON:
                    toxitronGUID = creature->GetGUID();
                    break;
                case NPC_MAGMATRON:
                    magmatronGUID = creature->GetGUID();
                    break;
                case NPC_ARCANOTRON:
                    arcanotronGUID = creature->GetGUID();
                    break;
                case NPC_NEFARIUS_OMNOTRON:
                    omnotronNefarianGUID = creature->GetGUID();
                    break;
                case NPC_CHIMAERON:
                    chimaeronGUID = creature->GetGUID();
                    break;
                case NPC_MALORIAK:
                    maloriakGUID = creature->GetGUID();
                    break;
                case NPC_ONYXIA:
                    // temp implementation while npcs movements was fix on transports
                    creature->m_movementInfo.t_guid = nefarian_elevatorGUID;
                    creature->m_movementInfo.t_pos.Relocate(0, 0, 0, 0);
                    onyxiaGUID = creature->GetGUID();
                    break;
                case NPC_INVISIBLE_STALKER_CATACLYSM_BOSS:
                    // temp implementation while npcs movements was fix on transports
                    creature->m_movementInfo.t_guid = nefarian_elevatorGUID;
                    switch (trgCount)
                    {
                        case 0:
                            creature->m_movementInfo.t_pos.Relocate(34.12f, 0.00436599f, 8.380039f, 3.141593f);
                            break;
                        case 1:
                            creature->m_movementInfo.t_pos.Relocate(-16.8569f, 28.8511f, 8.329325f, 5.253441f);
                            break;
                        case 2:
                            creature->m_movementInfo.t_pos.Relocate(-16.9927f, -29.0132f, 8.373013f, 0.9773844f);
                            break;
                    }
                    trgCount++;
                    break;
                default:
                    break;
                }
            }

            void OnCreatureDeath(Creature * creature)
            {
                if (creature->GetEntry() == NPC_SPIRIT_OF_THAURISSIAN)
                {
                    ++dwarvesCnt;
                    if (dwarvesCnt == 7)
                    {
                        instance->SummonCreature(NPC_INTRO_NEFARIAN, NefarianPos);
                        SaveToDB();
                    }
                }
            }

            void SetData(uint32 type, uint32 data)
            {
                switch (type)
                {
                    case DATA_DRAKONID:
                        ++drakonidCnt;
                        if (drakonidCnt >= 2)
                        {
                            if (Creature* magmaw = instance->GetCreature(magmawGUID))
                            {
                                magmaw->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                                magmaw->SetReactState(REACT_AGGRESSIVE);
                            }
                        }
                        break;
                    case DATA_ATRAMEDES_INTRO:
                        if (GameObject* go = instance->GetGameObject(bellGUID))
                            go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
                        break;
                    case DATA_ATRAMEDES_SUMMON:
                        if (!atramedesGUID && !IsDone(DATA_ATRAMEDES))
                            instance->SummonCreature(NPC_ATRAMEDES, AtramedesPos);
                        break;
                    case DATA_NEFARIAN_ELEVATOR:
                    {
                        if (GameObject* go = instance->GetGameObject(nefarian_elevatorGUID))
                            go->SetGoState((GOState)data);
                        break;
                    }
                    default:
                        break;
                }
            }

            uint64 GetData64(uint32 type) const
            {
                switch (type)
                {
                case DATA_OMNOTRON_GUID:
                    return omnotronGUID;
                case DATA_NEFARIUS_OMNOTRON_GUID:
                    return omnotronNefarianGUID;
                case DATA_ARCANOTRON_GUID:
                    return arcanotronGUID;
                case DATA_ELECTRON_GUID:
                    return electronGUID;
                case DATA_MAGMATRON_GUID:
                    return magmatronGUID;
                case DATA_TOXITRON_GUID:
                    return toxitronGUID;
                case DATA_MAGMAW_GUID:
                    return magmawGUID;
                case DATA_CHIMAERON_GUID:
                    return chimaeronGUID;
                case DATA_MALORIAK_GUID:
                    return maloriakGUID;
                case NPC_ATRAMEDES:
                    return atramedesGUID;
                case DATA_NEFARIAN_ELEVATOR_GUID:
                    return nefarian_elevatorGUID;
                default:
                    return 0;
                }
            }

            bool SetBossState(uint32 type, EncounterState state)
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;
                CheckHeroicRecquirement();
                switch (type)
                {
                    case DATA_MAGMAW:
                    case DATA_OMNOTRON_DEFENSE_SYSTEM:
                        if (IsDone(DATA_MAGMAW) && IsDone(DATA_OMNOTRON_DEFENSE_SYSTEM))
                            if (GameObject* door = instance->GetGameObject(wingDoorGUID))
                                door->SetGoState(GO_STATE_ACTIVE);
                        break;
                    default:
                        break;
                }

                 return true;
            }

            void CheckHeroicRecquirement()
            {
                bool secondPart = IsDoneInHeroic(DATA_MAGMAW)
                    && IsDoneInHeroic(DATA_OMNOTRON_DEFENSE_SYSTEM)
                    && IsDoneInHeroic(DATA_MALORIAK)
                    && IsDoneInHeroic(DATA_ATRAMEDES)
                    && IsDoneInHeroic(DATA_CHIMAERON);
                if (!instance->IsHeroic())
                    secondPart = true;

                if (Creature *nefarian = instance->GetCreature(nefarianGUID))
                    nefarian->SetVisible(secondPart);

                if (Creature *onyxia = instance->GetCreature(onyxiaGUID))
                    onyxia->SetVisible(secondPart);
            }

            void OnPlayerEnter(Player* /*player*/)
            {
                CheckHeroicRecquirement();
            }

            void WriteSaveDataMore(std::ostringstream& data) override
            {
                data << drakonidCnt << ' ' << dwarvesCnt;
            }

            void ReadSaveDataMore(std::istringstream& data) override
            {
                data >> drakonidCnt >> dwarvesCnt;
            }

        private:
            uint32 drakonidCnt;
            uint32 dwarvesCnt;
            uint64 magmawGUID;
            uint64 bellGUID;
            uint64 atramedesGUID;
            uint64 nefarianGUID;
            uint64 oyxiaGUID;
            uint64 omnotronGUID;
            uint64 arcanotronGUID;
            uint64 electronGUID;
            uint64 magmatronGUID;
            uint64 toxitronGUID;
            uint64 omnotronNefarianGUID;
            uint64 chimaeronGUID;
            uint64 maloriakGUID;
            uint64 wingDoorGUID;
            uint64 nefarian_elevatorGUID;
            uint32 trgCount;
            uint64 onyxiaGUID;
        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_blackwing_descent_InstanceMapScript(map);
        }
};

void AddSC_instance_blackwing_descent()
{
    new instance_blackwing_descent();
}
