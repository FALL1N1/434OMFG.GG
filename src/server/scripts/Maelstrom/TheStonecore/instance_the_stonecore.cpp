/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "InstanceScript.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "the_stonecore.h"

#define ENCOUNTERS     4

TeleportationData const teleportationData[] =
{
    {DATA_TELEPORT_CORBORUS, DATA_CORBORUS_EVENT             },
    {DATA_TELEPORT_SLABHIDE, DATA_SLABHIDE_EVENT             },
    {DATA_TELEPORT_OZRUK,    DATA_OZRUK_EVENT                },
    {0,                      0                               }, // END
};

class instance_the_stonecore : public InstanceMapScript
{
    public:
        instance_the_stonecore() : InstanceMapScript("instance_the_stonecore", 725) { }

        struct instance_the_stonecore_InstanceMapScript : public InstanceScript
        {
            instance_the_stonecore_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
            {
                SetHeaders(DataHeader);
                SetBossNumber(ENCOUNTERS);
                LoadTeleportationData(teleportationData);
                CorborusGUID = 0;
            }

            bool SetBossState(uint32 type, EncounterState state)
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;

                 return true;
            }

            uint64 GetData64(uint32 type) const
            {
                switch (type)
                {
                    case DATA_CORBORUS:
                        return CorborusGUID;
                    case DATA_ROCKDOOR:
                        return RockdoorGUID;
                    default:
                        break;
                }

                return 0;
            }

            void OnCreatureCreate(Creature* creature)
            {
                switch (creature->GetEntry())
                {
                    case BOSS_CORBORUS:
                        CorborusGUID = creature->GetGUID();
                        break;
                    case NPC_ROCK_BORER:
                        if (Creature* corbus = instance->GetCreature(CorborusGUID))
                        {
                            if (GetBossState(DATA_CORBORUS) == IN_PROGRESS)
                                corbus->AI()->JustSummoned(creature);
                        }
                        break;
                    case NPC_STONECORE_TELEPORTER1:
                        AddTeleporter(creature->GetGUID());
                        break;
                    default:
                        break;
                }
            }

            void OnGameObjectCreate(GameObject* gameobject)
            {
                switch (gameobject->GetEntry())
                {
                    case GO_ROCKDOOR_BREAK:
                        RockdoorGUID = gameobject->GetGUID();
                        break;
                    default:
                        break;
                }
            }

        protected:
            EventMap Events;
            uint64 CorborusGUID;
            uint64 RockdoorGUID;
        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_the_stonecore_InstanceMapScript(map);
        }
};

void AddSC_instance_the_stonecore()
{
    new instance_the_stonecore();
}
