/*
 * Copyright (C) 2010 - 2012 ProjectSkyfire <http://www.projectskyfire.org/>
 *
 * Copyright (C) 2011 - 2012 ArkCORE <http://www.arkania.net/>
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

#ifndef DEF_THRONE_OF_THE_TIDES
#define DEF_THRONE_OF_THE_TIDES

enum Data64
{
    DATA_LADY_NAZJAR,
    DATA_COMMANDER_ULTHOK,
    DATA_ERUNAK_STONESPEAKER,
    DATA_MINDBENDER_GHURSHA,
    DATA_OZUMAT,
    DATA_NEPTULON,
};

enum Data
{
    DATA_LADY_NAZJAR_EVENT,
    DATA_COMMANDER_ULTHOK_EVENT,
    DATA_ERUNAK_STONESPEAKER_EVENT,
    DATA_MINDBENDER_GHURSHA_EVENT,
    DATA_OZUMAT_EVENT,
    DATA_PROGRESS_EVENT,
    DATA_EVENT_COMMANDER_ULTHOK_START
};

enum CreatureIds
{
    BOSS_LADY_NAZJAR                  = 40586,
    NPC_SUMMONED_WITCH                = 40634, // 2 of them
    NPC_SUMMONED_GUARD                = 40633,
    NPC_SUMMONED_WATERSPOUT           = 48571,

    BOSS_COMMANDER_ULTHOK             = 40765,
    NPC_DARK_FISSURE                  = 40784,

    BOSS_ERUNAK_STONESPEAKER          = 40825,
    BOSS_MINDBENDER_GHURSHA           = 40788,
    NPC_EARTH_SHARD                   = 45469,
    NPC_MIND_FOG                      = 40861,

    BOSS_OZUMAT                       = 42172,
    BOSS_NEPTULON                     = 40792,
    NPC_DEEP_MURLOC                   = 44658,
    NPC_MINDLASHER                    = 44715,
    NPC_BEHEMOTH                      = 44648,
    // SPELL WATER DOWN = 83479
    NPC_SAPPER                        = 44752,
    NPC_BEAST                         = 44841,
    NPC_WORLD_TRIGGER_INFINITE_AOI    = 361710,
    NPC_BLIGHT_OF_OZUMAT_ROOM         = 44834,

    BOSS_OZUMAT_EVENT                 = 40655,

    NPC_CAPTAIN_TAYLOR                = 50270,
    NPC_LEGIONNAIRE_NAZGRIM           = 50272,
    NPC_NEPTULON                      = 40792,
    NPC_THRONE_TELEPORTER_1           = 51395,
    NPC_THRONE_TELEPORTER_2           = 51391,
    NPC_FISSURE                       = 40789,

    NPC_CINEMATIC_TRIGGER             = 361710,
};

enum GameObjectIds
{
    GO_NEPUTOLON_CACHE                = 205216,
    GO_STRANGE_FOUNTAIN               = 207406,
    GO_ABYSSAL_MAW_01                 = 204338,
    GO_ABYSSAL_MAW_02                 = 204339,
    GO_ABYSSAL_MAW_03                 = 204340,
    GO_ABYSSAL_MAW_04                 = 204341,
    GO_CORAIL                         = 205542,
    GO_CONSOLE                        = 203199,
    GO_OZUMAT_CHEST_NORMAL            = 205216,
    GO_OZUMAT_CHEST_HEROIC            = 205215,
    GO_TENTACLE_LEFT                  = 208301,
    GO_TENTACLE_RIGHT                 = 208302,
    GO_WALL_TENTACLE_LEFT             = 207997,
    GO_WALL_TENTACLE_RIGHT            = 207998,
};

enum AchievemtsIds
{
    NORMAL_OZUMAT_DEFEAT = 4839,
    HEROIC_OZUMAT_DEFEAT = 5061,
    ACHIEVEMENT_THRONE_OF_THE_TIDES_GUILD_GROUP_CRITERIA = 14483,
    HEROIC_NAZJAR_GEYSER_KILLED_TRASH = 5285,
    HEROIC_DEFEAT_BEHEMOT_WITH_SURGE_EFFECT = 5286,
};

#endif
