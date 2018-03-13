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

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "ScriptedCreature.h"
#include "zulgurub.h"

DoorData const doorData[] =
{
    { GO_VENOXIS_COIL,                  DATA_VENOXIS,           DOOR_TYPE_ROOM, BOUNDARY_NONE },
    { GO_ARENA_DOOR_1,                  DATA_MANDOKIR,          DOOR_TYPE_ROOM, BOUNDARY_NONE },
    { GO_FORCEFIELD,                    DATA_KILNARA,           DOOR_TYPE_ROOM, BOUNDARY_NONE },
    { GO_ZANZIL_DOOR,                   DATA_ZANZIL,            DOOR_TYPE_ROOM, BOUNDARY_NONE },
    { GO_THE_CACHE_OF_MADNESS_DOOR,     DATA_CACHE_OF_MADNESS,  DOOR_TYPE_ROOM, BOUNDARY_NONE },
    { 0,                                0,                      DOOR_TYPE_ROOM, BOUNDARY_NONE }
};

const Position DwarvenArtifactSP[5]=
{
    {-11915.6f, -1893.65f, 63.8770f, 1.60570f},
    {-11910.5f, -1902.63f, 63.7369f, 1.30900f},
    {-11908.1f, -1913.17f, 64.2613f, 1.78024f},
    {-11890.0f, -1902.46f, 63.5871f, 2.49582f},
    {-11898.1f, -1866.09f, 63.8182f, 4.10152f},
};

const Position ElvenArtifactSP[5]=
{
    {-11854.2f, -1913.29f, 63.7067f, 2.02458f},
    {-11848.1f, -1895.26f, 63.7176f, 2.63545f},
    {-11864.6f, -1911.97f, 63.9063f, 1.72788f},
    {-11857.9f, -1906.62f, 63.7067f, 5.56760f},
    {-11851.6f, -1899.81f, 63.8158f, 2.33874f},
};

const Position TrollArtifactSP[5]=
{
    {-11914.3f, -1874.56f, 63.8285f, 5.75959f},
    {-11885.0f, -1850.56f, 63.8087f, 5.28835f},
    {-11873.8f, -1901.87f, 63.6181f, 1.72788f},
    {-11887.3f, -1902.98f, 63.5894f, 0.80285f},
    {-11845.6f, -1875.98f, 63.7353f, 3.31613f},
};

const Position FossilSP[5]=
{
    {-11851.8f, -1847.89f, 64.1662f, 0.00000f},
    {-11868.0f, -1840.68f, 63.9099f, 4.32842f},
    {-11852.5f, -1857.78f, 63.9159f, 4.43314f},
    {-11858.1f, -1850.68f, 63.8261f, 0.00000f},
    {-11857.6f, -1843.13f, 63.7030f, 2.86234f},
};

const Position TikiTorchSP[6]=
{
    {-11933.2f, -1824.54f, 51.7838f, 1.53589f},
    {-11919.8f, -1824.58f, 51.4590f, 1.53589f},
    {-11903.5f, -1824.38f, 51.5542f, 1.51844f},
    {-11879.6f, -1824.81f, 50.8839f, 1.46608f},
    {-11864.6f, -1824.44f, 51.1218f, 1.50098f},
    {-11849.5f, -1824.58f, 51.4813f, 1.55334f},
};

class instance_zulgurub : public InstanceMapScript
{
    public:
        instance_zulgurub() : InstanceMapScript(ZGScriptName, 859) { }

    private:
        struct instance_zulgurub_InstanceMapScript : public InstanceScript
        {
            instance_zulgurub_InstanceMapScript(Map* map) : InstanceScript(map)
            {
                SetHeaders(DataHeader);
                SetBossNumber(EncounterCount);
                LoadDoorData(doorData);
            }

            uint64 jindoGUID;
            uint8 activatedArtifactCount;
            uint8 tikiMaskId;

            void Initialize()
            {
                jindoGUID = 0;
                activatedArtifactCount = 0;
                tikiMaskId = 0;

                for (int i = 0; i < 6; ++i)
                    if (Creature* torch = instance->SummonCreature(52419, TikiTorchSP[i]))
                        torch->GetAI()->SetData(DATA_POSITION_ID, i);

                int d_roll = urand(0, 4);
                int e_roll = urand(0, 4);
                int t_roll = urand(0, 4);
                int f_roll = urand(0, 4);

                for (int i = 0; i < 5; ++i)
                {
                    instance->SummonCreature(i == d_roll ? 52446 : 52449, DwarvenArtifactSP[i]);
                    instance->SummonCreature(i == e_roll ? 52450 : 52451, ElvenArtifactSP[i]);
                    instance->SummonCreature(i == t_roll ? 52452 : 52453, TrollArtifactSP[i]);
                    instance->SummonCreature(i == f_roll ? 52454 : 52455, FossilSP[i]);
                }
            }

            bool SetBossState(uint32 type, EncounterState state)
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;

                CheckRecquirement();

                return true;
            }

            void CheckRecquirement()
            {
                bool jind = IsDone(DATA_VENOXIS) && IsDone(DATA_MANDOKIR) && IsDone(DATA_KILNARA) && IsDone(DATA_ZANZIL);
                if (Creature* jindo = instance->GetCreature(jindoGUID))
                {
                    jindo->SetVisible(jind);
                    jindo->SetReactState(jind ? REACT_AGGRESSIVE : REACT_PASSIVE);
                }
            }

            void SetData(uint32 type, uint32 data)
            {
                switch (type)
                {
                    case DATA_CACHE_OF_MADNESS_ARTIFACT:
                    {
                        switch (data)
                        {
                            case IN_PROGRESS:
                                ++activatedArtifactCount;
                                break;
                            case DONE:
                            {
                                uint32 bossEntry = NPC_GRILEK;
                                switch (rand() % 3)
                                {
                                    case 0:
                                        bossEntry = NPC_GRILEK;
                                        break;
                                    case 1:
                                        bossEntry = NPC_WUSHOOLAY;
                                        break;
                                    case 2:
                                        bossEntry = NPC_RENATAKI;
                                        break;
                                    case 3:
                                        bossEntry = NPC_HAZZARAH;
                                        break;
                                }

                                if (Creature* boss = instance->SummonCreature(bossEntry, EdgeofMadnessSP))
                                    boss->CastSpell(boss, SPELL_CACHE_OF_MADNESS_BOSS_SUMMON, false);
                                break;
                            }
                        }
                        break;
                    }
                    case DATA_TIKI_MASK_ID:
                    {
                        switch (data)
                        {
                            case IN_PROGRESS:
                                ++tikiMaskId;
                                break;
                            case NOT_STARTED:
                                tikiMaskId = 0;
                                break;
                        }
                        break;
                    }
                }
            }

            uint32 GetData(uint32 type) const
            {
                switch (type)
                {
                    case DATA_CACHE_OF_MADNESS_ARTIFACT: return activatedArtifactCount;
                    case DATA_TIKI_MASK_ID:              return tikiMaskId;
                }

                return 0;
            }

            void OnCreatureCreate(Creature* creature)
            {
                switch (creature->GetEntry())
                {
                    case NPC_JINDO:
                        creature->setActive(true);
                        jindoGUID = creature->GetGUID();
                        break;
                }

                CheckRecquirement();
            }

            void OnGameObjectCreate(GameObject* go)
            {
                switch (go->GetEntry())
                {
                    case GO_VENOXIS_COIL:
                    case GO_ARENA_DOOR_1:
                    case GO_FORCEFIELD:
                    case GO_ZANZIL_DOOR:
                    case GO_THE_CACHE_OF_MADNESS_DOOR:
                        AddDoor(go, true);
                        break;
                }
            }

            void OnGameObjectRemove(GameObject* go)
            {
                switch (go->GetEntry())
                {
                    case GO_VENOXIS_COIL:
                    case GO_ARENA_DOOR_1:
                    case GO_FORCEFIELD:
                    case GO_ZANZIL_DOOR:
                    case GO_THE_CACHE_OF_MADNESS_DOOR:
                        AddDoor(go, false);
                        break;
                }
            }

            uint64 GetData64(uint32 type) const
            {
                if (type == DATA_JINDO)
                    return jindoGUID;

                return 0;
            }
        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_zulgurub_InstanceMapScript(map);
        }
};

void AddSC_instance_zulgurub()
{
    new instance_zulgurub();
}
