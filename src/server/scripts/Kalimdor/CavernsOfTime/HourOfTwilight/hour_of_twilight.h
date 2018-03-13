/*
 * Copyright (C) 2013-2014 OMFG.GG Network <http://www.omfg.gg/>
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

#ifndef HOUR_OF_TWILIGHT_H_
#define HOUR_OF_TWILIGHT_H_

#define HourOfTwilightScriptName "instance_hour_of_twilight"
#define DataHeader "HT"

uint32 const EncounterCount = 3;

enum Datas
{
    // Encounters
    DATA_ARCURION        = 1,
    DATA_ASIRA,
    DATA_ARCHBISHOP,

    // Additional Objects
    DATA_THRALL_EVENT_0,
    DATA_THRALL_EVENT_1,
    DATA_THRALL_EVENT_2,
    DATA_THRALL_EVENT_3,
    DATA_THRALL,

    DATA_EVENT_0,
    DATA_EVENT_1,
    DATA_EVENT_2,
    DATA_EVENT_3,

    DATA_HORDE_PORTAL,
    DATA_ALLIANCE_PORTAL
};

enum CreatureIds
{
    //Dungeon Bosses
    NPC_ARCURION            = 54590,
    NPC_ASIRA               = 54968,
    NPC_ARCHBISHOP          = 54938,
    NPC_FROZEN_SERVITOR     = 54555,
    NPC_THRALL              = 54971,
    NPC_THRALL_EVENT_0      = 54548,
    NPC_THRALL_EVENT_1      = 55779,
    NPC_THRALL_EVENT_2      = 54972,
    NPC_THRALL_EVENT_3      = 54634,
    NPC_TWILIGHT_SPARK      = 55466
};

enum GameObjectIds
{
    GO_ICEWALL              = 210048,
    GO_HORDE_PORTAL         = 209081,
    GO_ALLIANCE_PORTAL      = 209080,
};

enum Actions
{
    ACTION_START_COMBAT = 1,
    ACTION_BOSS_PHASE_3,
    ACTION_STOP_COMBAT,
    ACTION_KILL_CREATURE,
};

enum Misc
{
    SPELL_DELETE_ICY_TOMB   = 102722,
};

template<class AI>
inline AI* GetHourOfTwilightAI(Creature* creature)
{
    return GetInstanceAI<AI>(creature, HourOfTwilightScriptName);
}

#endif // HOUR_OF_TWILIGHT_H_
