/*
 * Copyright (C) 2013-2014 OMFG.GG Network <http://www.omfg.gg/>
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
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

#include "InstanceScript.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "blackrock_caverns.h"

TeleportationData const teleportationData[] =
{
    {DATA_TELEPORT_CORLA,             BOSS_CORLA             },
    {DATA_TELEPORT_KARSH,             BOSS_KARSH_STEELBENDER },
    {DATA_TELEPORT_BEAUTY,            BOSS_BEAUTY            },
    {0,                               0                      }, // END
};

class instance_blackrock_caverns : public InstanceMapScript
{
public:
    instance_blackrock_caverns() : InstanceMapScript("instance_blackrock_caverns", 645) { }

    struct instance_blackrock_cavernsInstanceMapScript : public InstanceScript
    {
        instance_blackrock_cavernsInstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetHeaders(DataHeader);
            SetBossNumber(EncounterCount);
            LoadTeleportationData(teleportationData);
            RomoggBonecrusherGUID      = 0;
            CorlaGUID                  = 0;
            KarshSteelbenderGUID       = 0;
            BeautyGUID                 = 0;
            AscendantLordObsidiusGUID  = 0;
        }

        void OnCreatureCreate(Creature* creature)
        {
           switch (creature->GetEntry())
            {
                case NPC_ROMOGG_BONECRUSHER:
                    RomoggBonecrusherGUID = creature->GetGUID();
                    break;
                case NPC_CORLA:
                    CorlaGUID = creature->GetGUID();
                    break;
                case NPC_KARSH_STEELBENDER:
                    KarshSteelbenderGUID = creature->GetGUID();
                    break;
                case NPC_BEAUTY:
                    BeautyGUID = creature->GetGUID();
                    break;
                case NPC_ASCENDANT_LORD_OBSIDIUS:
                    AscendantLordObsidiusGUID = creature->GetGUID();
                    break;
                case NPC_RAZ_THE_CRAZED:
                    RazGUID = creature->GetGUID();
                    break;
                case NPC_TWILIGHT_PORTAL:
                    AddTeleporter(creature->GetGUID());
                    break;
            }
        }

        uint64 GetData64(uint32 data) const
        {
            switch (data)
            {
                case DATA_ROMOGG_BONECRUSHER:
                    return RomoggBonecrusherGUID;
                case DATA_CORLA:
                    return CorlaGUID;
                case DATA_KARSH_STEELBENDER:
                    return KarshSteelbenderGUID;
                case DATA_BEAUTY:
                    return BeautyGUID;
                case DATA_ASCENDANT_LORD_OBSIDIUS:
                    return AscendantLordObsidiusGUID;
            }
            return 0;
        }

        bool SetBossState(uint32 type, EncounterState state)
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            if (type == BOSS_ROMOGG_BONECRUSHER && (state == DONE || state == DONE_HM))
            {
                if (Creature *raz = instance->GetCreature(RazGUID))
                    raz->AI()->DoAction(ACTION_RAZ_START_EVENT);
            }
            return true;
        }

        private:
            uint64 RomoggBonecrusherGUID;
            uint64 CorlaGUID;
            uint64 KarshSteelbenderGUID;
            uint64 BeautyGUID;
            uint64 AscendantLordObsidiusGUID;
            uint64 RazGUID;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_blackrock_cavernsInstanceMapScript(map);
    }
};

void AddSC_instance_blackrock_caverns()
{
    new instance_blackrock_caverns();
}
