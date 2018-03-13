/*
 * Copyright (C) 2010-2011 Trinity <http://www.projecttrinity.org/>
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

#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "InstanceScript.h"
#include "the_vortex_pinnacle.h"

class instance_the_vortex_pinnacle : public InstanceMapScript
{
    public:
        instance_the_vortex_pinnacle() : InstanceMapScript("instance_the_vortex_pinnacle", 657)
        {
        }

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_the_vortex_pinnacle_InstanceMapScript(map);
        }

        struct instance_the_vortex_pinnacle_InstanceMapScript: public InstanceScript
        {
            instance_the_vortex_pinnacle_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
            {
                SetHeaders(DataHeader);
            }

            uint64 grandVizierErtanGUID;
            uint64 altairusGUID;
            uint64 asaadGUID;
            uint64 npcWindGUID;
            uint32 goldenOrbCount;

            void Initialize()
            {
                grandVizierErtanGUID    = 0;
                altairusGUID            = 0;
                asaadGUID               = 0;
                npcWindGUID = 0;
                SetBossNumber(MAX_BOSSES);
                underMapTimer = 2000;
                goldenOrbCount = 0;
            }

            void OnCreatureCreate(Creature* creature)
            {
                switch (creature->GetEntry())
                {
                    case NPC_GRAND_VIZIER_ERTAN:
                        grandVizierErtanGUID = creature->GetGUID();
                        break;
                    case NPC_ALTAIRUS:
                        altairusGUID = creature->GetGUID();
                        break;
                    case NPC_ASAAD:
                        asaadGUID = creature->GetGUID();
                        break;
                    case NPC_WIND:
                        if (npcWindGUID != 0)
                            if (Creature *c = instance->GetCreature(npcWindGUID))
                                c->DespawnOrUnsummon();
                        npcWindGUID = creature->GetGUID();
                        break;
                    default:
                        break;
                }
            }

            bool SetBossState(uint32 type, EncounterState state)
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;
                return true;
            }

            uint64 GetData64(uint32 data) const
            {
                switch (data)
                {
                    case BOSS_GRAND_VIZIER_ERTAN:
                        return grandVizierErtanGUID;
                    case BOSS_ALTAIRUS:
                        return altairusGUID;
                    case BOSS_ASAAD:
                        return asaadGUID;
                    case NPC_WIND:
                        return npcWindGUID;
                }

                return 0;
            }

            void SetData(uint32 type, uint32 data)
            {
                switch (type)
                {
                    case DATA_GOLDEN_ORB:
                    {
                        goldenOrbCount++;
                        if (goldenOrbCount == 5)
                            DoCompleteAchievement(5289);
                    }
                    break;
                default:
                    break;
                }
            }

            void Update(uint32 diff)
            {
                if (underMapTimer <= diff)
                {
                    Position pos;
                    Map::PlayerList const& players = instance->GetPlayers();
                    for (Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
                    {
                        if (Player* player = i->getSource())
                        {
                            player->GetPosition(&pos);
                            if (player->GetPositionZ() <= 568.0f && !player->GetVehicle())
                            {
                                if (Creature *sp = instance->SummonCreature(NPC_SLIPSTREAM, pos))
                                {
                                    sp->AI()->SetGUID(player->GetGUID());
                                    sp->GetMotionMaster()->MoveJump(savePlayersPos[player->GetGUID()], 20, 50, 42);
                                }
                            }
                            else if (!player->IsFalling() && !player->GetVehicle() && player->GetPositionZ() > 568.0f)
                            {
                                float groundZ = player->GetMap()->GetHeight(player->GetPhaseMask(), pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), true, MAX_FALL_DISTANCE);

                                if ((groundZ + 1.00f) >= pos.GetPositionZ())
                                {
                                    pos.m_positionZ += 1.0f;
                                    savePlayersPos[player->GetGUID()] = pos;
                                }
                            }
                        }
                    }
                    underMapTimer = 1000;
                }
                else
                    underMapTimer -= diff;
            }

        private:
            uint32 underMapTimer;
            std::map<uint64, Position> savePlayersPos;
        };
};

void AddSC_instance_the_vortex_pinnacle()
{
    new instance_the_vortex_pinnacle();
}
