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

#ifndef DEF_ZULAMAN_H
#define DEF_ZULAMAN_H

#define DataHeader "ZA"

uint32 const EncounterCount = 7;

enum DataTypes
{
    DATA_AKILZONEVENT                   = 0,
    DATA_NALORAKKEVENT                  = 1,
    DATA_JANALAIEVENT                   = 2,
    DATA_HALAZZIEVENT                   = 3,
    DATA_HEXLORDEVENT                   = 4,
    DATA_DAAKARAEVENT                   = 5,
    DATA_GONGEVENT                      = 6,

    DATA_CHESTLOOTED                    = 7,
    TYPE_RAND_VENDOR_1                  = 8,
    TYPE_RAND_VENDOR_2                  = 9,

};

enum CreatureIds
{
    NPC_HARRISON_JONES                  = 24358,
    NPC_JANALAI                         = 23578,
    NPC_DAAKARA                         = 23863,
    NPC_HEXLORD                         = 24239,
    NPC_HALAZZI                         = 23577,
    NPC_NALORAKK                        = 23576,

    NPC_LYNX_SPIRIT                     = 23877,
    NPC_BEAR_SPIRIT                     = 23878,
    NPC_EAGLE_SPIRIT                    = 23880,
    NPC_DRAGONHAWK_SPIRIT               = 23879,

    NPC_HOSTAGE_1                        = 52941,
    NPC_HOSTAGE_2                        = 52939,
    NPC_HOSTAGE_3                        = 52943,
    NPC_HOSTAGE_4                        = 52945,

    NPC_HOSTAGE_CORPSE_1                 = 52942,
    NPC_HOSTAGE_CORPSE_2                 = 52940,
    NPC_HOSTAGE_CORPSE_3                 = 52944,
    NPC_HOSTAGE_CORPSE_4                 = 52946,
};

enum GameobjectIds
{
    GO_DOOR_HALAZZI_ENTRANCE            = 186304,
    GO_DOOR_HALAZZI_EXIT                = 186303,
    GO_GATE_HEXLORD_ENTRANCE            = 186305,
    GO_GATE_HEXLORD_EXIT                = 186306,
    GO_MASSIVE_GATE                     = 186728,
    GO_DOOR_AKILZON                     = 186858,
    GO_DOOR_DAAKARA                     = 186859,

    GO_HARKORS_SATCHEL                  = 187021, // akilzon
    GO_TANZARS_TRUNK                    = 186648, // nalorakk
    GO_KRAZS_PACKAGE                    = 186667, // janalai
    GO_ASHLIS_BAG                       = 186672, // hallazi

    GO_STRANGE_GONG                     = 187359
};

#endif
