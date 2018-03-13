/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2009-2013 Frostmourne <http://www.frostmourne-wow.eu/>
 *
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
#ifndef DEF_BLACKROCK_CAVERNS_H
#define DEF_BLACKROCK_CAVERNS_H

#define DataHeader "BC"

uint32 const EncounterCount         = 6;

enum Data
{
    BOSS_ROMOGG_BONECRUSHER         = 1,
    BOSS_CORLA                      = 2,
    BOSS_KARSH_STEELBENDER          = 3,
    BOSS_BEAUTY                     = 4,
    BOSS_ASCENDANT_LORD_OBSIDIUS    = 5,

    DATA_ROMOGG_BONECRUSHER         = 6,
    DATA_CORLA                      = 7,
    DATA_KARSH_STEELBENDER          = 8,
    DATA_BEAUTY                     = 9,
    DATA_ASCENDANT_LORD_OBSIDIUS    = 10,

    // additional datas
    DATA_SPAWN_LEFT                 = 11,
    DATA_SPAWN_RIGHT                = 12,
    DATA_SPAWN_CENTER               = 13,
    DATA_ZEALOT_LEFT                = 14,
    DATA_ZEALOT_RIGHT               = 15,
    DATA_ZEALOT_CENTER              = 16
};

enum TeleporterDatas
{
    DATA_TELEPORT_CORLA = 1,
    DATA_TELEPORT_KARSH,
    DATA_TELEPORT_BEAUTY
};

enum Creatures
{
    // Romogg Bonecrusher
    NPC_ROMOGG_BONECRUSHER          = 39665,
    NPC_ANGERED_EARTH               = 50376,
    NPC_CHAINS_OF_WOE               = 40447,
    NPC_QUAKE_TRIGGER               = 40401,

    // Corla Herald of Twilight
    NPC_CORLA                       = 39679,
    NPC_TWILIGHT_ZEALOT_CORLA       = 50284,
    NPC_NETHER_ESSENCE_TRIGGER      = 39842,

    // Karsh Steelbender
    NPC_BOUND_FLAMES                = 50417,
    NPC_LAVA_SPOUT_TRIGGER          = 39842,
    NPC_KARSH_STEELBENDER           = 39698,

    // Beauty
    NPC_BEAUTY                      = 39700,
    NPC_RUNTY                       = 40008,
    NPC_BUSTER                      = 40013,
    NPC_LUCKY                       = 40015,

    // Ascendant Lord Obsidius
    NPC_ASCENDANT_LORD_OBSIDIUS     = 39705,
    NPC_SHADOW_OF_OBSIDIUS          = 40817,

    // Various NPCs
    NPC_RAZ_THE_CRAZED              = 39670,
    NPC_FINKLE_EINHORN              = 49476,
    NPC_TWILIGHT_PORTAL             = 51340,
};

enum Gameobjects
{
    GO_STONE_TABLET                 = 207412,
};

enum Misc
{
    ACTION_RAZ_START_EVENT,
};

#endif
