/*
* Copyright (C) 2010-2011 Project Trinity <http://www.projecttrinity.org/>
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

#ifndef DEF_BLACKWING_DESCENT_H
#define DEF_BLACKWING_DESCENT_H

#define DataHeader "BD"

enum SharedSpells
{
    SPELL_BERSERK1 = 26662,
    SPELL_BERSERK2 = 64238,
};

enum Data
{
    //Encounters
    DATA_MAGMAW                  = 0,
    DATA_OMNOTRON_DEFENSE_SYSTEM = 1,
    DATA_MALORIAK                = 2,
    DATA_ATRAMEDES               = 3,
    DATA_CHIMAERON               = 4,
    DATA_NEFARIAN                = 5,
    ENCOUNTER_COUNT              = 6,

    //Additional Entities
    DATA_MAGMAW_GUID             = 7,
    DATA_ARCANOTRON_GUID         = 8,
    DATA_ELECTRON_GUID           = 9,
    DATA_MAGMATRON_GUID          = 10,
    DATA_TOXITRON_GUID           = 11,
    DATA_ONYXIA_GUID             = 12,
    DATA_BILE_O_TRON_800         = 13,
    DATA_OMNOTRON_GUID           = 14,
    DATA_NEFARIUS_OMNOTRON_GUID  = 15,
    DATA_CHIMAERON_GUID          = 16,

    DATA_DRAKONID                = 17,
    DATA_ATRAMEDES_INTRO         = 18,
    DATA_ATRAMEDES_SUMMON        = 19,
    DATA_MALORIAK_GUID           = 20,

    DATA_NEFARIAN_ELEVATOR       = 21,

    DATA_NEFARIAN_ELEVATOR_GUID  = 22,

    /*ACHIEVEMENTS*/
    DATA_SILENCE_IS_GOLDEN,
};

enum Creatures
{
    NPC_DRAKONID_DRUDGE      = 42362,
    NPC_MAGMAW               = 41570,
    NPC_MAGMAWS_HEAD         = 42347,
    NPC_MAGMAWS_HEAD2        = 48270,
    NPC_MAGMAWS_PINCER       = 41620,
    NPC_MAGMAWS_PINCER2      = 41789,
    NPC_PILLAR_OF_FLAME      = 41843,
    NPC_SPIKE_STALKER        = 41767,
    NPC_IGNITION             = 49447,
    NPC_ROOM_STALKER         = 47196,
    NPC_LAVA_PARASITE        = 42321,
    NPC_OMNOTRON             = 42186,
    NPC_ARCANOTRON           = 42166,
    NPC_ELECTRON             = 42179,
    NPC_MAGMATRON            = 42178,
    NPC_TOXITRON             = 42180,
    NPC_POISON_BOMB          = 42897,
    NPC_CHEMICAL_BOMB        = 42934,
    NPC_POWER_GENERATOR      = 42733,
    NPC_MALORIAK             = 41378,
    NPC_ATRAMEDES            = 41442,
    NPC_CHIMAERON            = 43296,
    NPC_FINKLE_EINHORN       = 44202,
    NPC_BILE_O_TRON_800      = 44418,
    NPC_NEFARIAN             = 41376,
    NPC_ONYXIA               = 41270,
    NPC_LORD_VICTOR_NEFARIAN = 41379,
    NPC_SPIRIT_OF_THAURISSIAN= 43126,
    NPC_INTRO_NEFARIAN       = 43396,
    NPC_NEFARIUS_OMNOTRON    = 49226,
    NPC_NEFARIUS_CHIMAERON   = 48964,
    NPC_NEFARIUS_ATRAMEDES   = 49580,
    NPC_NEFARIUS_MALORIAK    = 49799,
    NPC_INVISIBLE_STALKER_CATACLYSM_BOSS = 42098,
};

enum GameObjects
{
    GO_ANCIENT_BELL         = 204276,
    GO_WING_DOOR            = 205830,
    GO_NEFARIAN_ELEVATOR    = 207834,
    GO_NEFARIAN_ELEVATOR_COLLISION = 500000,
};

#endif
