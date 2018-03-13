/*
 * Copyright (C) 2013-2015 OMFG.GG Network <http://www.omfg.gg/>
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

#ifndef WELL_OF_ETERNITY_H_
#define WELL_OF_ETERNITY_H_

#define WellOfEternityScriptName "instance_well_of_eternity"
#define DataHeader "WOE"

uint32 const EncounterCount = 3;

enum Datas
{
    // Encounters
    DATA_PEROTHARN              = 0,
    DATA_QUEEN_AZSHARA          = 1,
    DATA_MANNOROTH              = 2,

    // Data64
    DATA_FIREWALL_ONE           = 3,
    DATA_FIREWALL_TWO           = 4,
    DATA_FIREWALL_THREE         = 5,
    DATA_COURTYYARD_DOOR_ONE    = 6,
    DATA_ILLIDAN_PART_ONE       = 7,
    DATA_ENERGY_FOCUS_ONE       = 8,
    DATA_ENERGY_FOCUS_TWO       = 9,
    DATA_ENERGY_FOCUS_THREE     = 10,
    DATA_LIGHT_OF_ELUNE         = 11,
    DATA_ROYAL_CACHE            = 12,
    DATA_TYRANDE_WHISPERWIND    = 13,
    DATA_MALFURION_STORMRAGE    = 14,
    DATA_ILLIDAN_PART_TWO       = 15,
    DATA_CAPTAIN_VAROTHEN       = 16,
    DATA_DOOMBRINGER            = 17,
    DATA_PORTAL                 = 18,
    DATA_DOOMGUARD_PORTAL       = 19,
    DATA_MANNOROTH_CACHE        = 20,
    DATA_CHROMIE                = 21,
    DATA_LAZY_EYE_ACHIEVEMENT   = 22,
    DATA_NOZDORMU               = 23,
};

enum CreatureIds
{
    NPC_PEROTHARN               = 55085,
    NPC_CORRUPTED_ARCANIST      = 55654,
    NPC_DREADLORD_DEFENDER      = 55656,
    NPC_LEGION_DEMON            = 55503,
    NPC_ILLIDAN_PART_ONE        = 55500,
    NPC_DISTRACT_STALKER        = 58200,
    NPC_FEL_CRYSTAL_STALKER     = 55965,
    NPC_FEL_CRYSTAL             = 55917,
    NPC_GUARDIAN_DEMON          = 54927,
    NPC_NOZDORMU                = 55624,
    // Perotharn
    NPC_FEL_FLAMES_TRIGGER      = 57329,
    NPC_HUNTING_CIRCLE          = 56182,
    NPC_HUNTING_STALKER         = 56248,
    NPC_EYE_OF_THE_PEROTHARN    = 55879,
    // Queen Azshara
    NPC_QUEEN_AZSHARA           = 54853,
    NPC_FIRE_MAGE               = 54882,
    NPC_ARCANE_MAGE             = 54884,
    NPC_FROST_MAGE              = 54883,
    NPC_HAND_OF_THE_QUEEN       = 54728,
    NPC_HAMMER_OF_DIVINITY      = 54864,
    NPC_HAMMER_OF_DIVINITY_2    = 54865,
    // Mannoroth
    NPC_TYRANDE_WHISPERWIND     = 55524,
    NPC_MALFURION_STORMRAGE     = 55570,
    NPC_ILLIDAN_PART_TWO        = 55532,
    NPC_ABYSSAL_DOOMBRINGER     = 55510,
    NPC_DREADLORD_DEBILITATOR   = 55762,
    NPC_HIGHGUARD_ELITE         = 55426,
    NPC_SHADOWBAT               = 55453,
    NPC_CAPTAIN_VAROTHEN        = 55419,
    NPC_MANNOROTH               = 54969,
    NPC_EMBEDDED_BLADE_CAPTAIN  = 55837,
    NPC_EMBEDDED_BLADE          = 55838,
    NPC_CHROMIE                 = 57913,
    NPC_DOOMGUARD_ANNIHILATOR   = 55519,
    NPC_FLAME_TARGET            = 55091,
    NPC_FEL_FLAME               = 55502,
    NPC_GENERAL_PURPOSE_BUNNY   = 54020,
    NPC_FELHOUND                = 56001,
    NPC_FELGUARD                = 56002,
    NPC_DOOMGUARD               = 57410,
    NPC_DOOMGUARD_PORTAL        = 56087,
    NPC_DOOMGUARD_DEVASTATOR    = 55739,
    NPC_INFERNO                 = 56036,
};

enum GameObjectIds
{
    GO_LARGE_FIREWALL_DOOR          = 210234,
    GO_SMALL_FIREWALL_DOOR          = 210130,
    GO_INVISIBLE_FIREWALL_DOOR      = 210097,
    GO_COURTYARD_DOOR_01            = 210084,
    GO_PORTAL_ENERGY_FOCUS_ONE      = 209366,
    GO_PORTAL_ENERGY_FOCUS_TWO      = 209447,
    GO_PORTAL_ENERGY_FOCUS_THREE    = 209448,

    // Queen Aszhara
    GO_LIGHT_OF_ELUNE               = 177418,
    GO_ROYAL_CACHE                  = 210025,

    // Mannoroth
    GO_MINOR_CACHE_OF_THE_ASPECTS   = 209541,
};

enum sharedSpells
{
    SPELL_PLAYER_SHADOWCLOAK            = 103004,
    SPELL_PLAYER_SHADOW_WALK            = 102994,
    SPELL_PLAYER_SHADOW_WALK_VISUAL     = 103020,
    SPELL_NIGHT_ELF_ILLUSION_MALE       = 108465,
    SPELL_NIGHT_ELF_ILLUSION_FEMALE     = 108466
};

enum sharedActions
{
    ACTION_DESPAWN_PORTAL_ONE   = 10,
    ACTION_DESPAWN_PORTAL_TWO   = 11,
    ACTION_DESPAWN_PORTAL_THREE = 12,
    ACTION_START_COMBAT         = 13,
    ACTION_COMBAT_FINISHED      = 14
};

const Position eventWaypoints[3] =
{
    { 3193.949f, -4926.029f, 194.179f, 0.0000f },
    { 3215.734f, -4909.068f, 194.179f, 0.3088f },
    { 3334.899f, -4891.346f, 181.077f, 0.0000f }
};

const Position NozdormuPositions[2] =
{
    { 3433.530f, -5017.490f, 196.782f, 5.3756f },
    { 3094.930f, -5546.470f, 18.715f, 3.036f}
};

struct WellOfEternityEventSpawns
{
    uint32 npcId;
    float x;
    float y;
    float z;
    float orientation;
};

template<class AI>
inline AI* GetWellOfEternityAI(Creature* creature)
{
    return GetInstanceAI<AI>(creature, WellOfEternityScriptName);
}

#endif // WELL_OF_ETERNITY_H_
