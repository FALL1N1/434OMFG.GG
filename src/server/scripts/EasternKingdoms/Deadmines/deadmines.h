/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#ifndef DEF_DEADMINES_H
#define DEF_DEADMINES_H

#define DataHeader "DM"

enum DeadminesBossData
{
    BOSS_GLUBTOK_DATA               = 0,
    BOSS_HELIX_DATA                 = 1,
    BOSS_FOE_REAPER_5000_DATA       = 2,
    BOSS_CAPTAIN_COOKIE             = 3,
    BOSS_ADMIRAL_RISNARL            = 4,
    BOSS_EVENT_VANCLEEF             = 5,
    BOSS_VANCLEEF                   = 6,
    MAX_ENCOUNTER                   = 7,

    BOSS_GLUBTOK_EVENT_DATA,
    BOSS_HELIX_EVENT_DATA,
    BOSS_REEPER_EVENT_DATA,
    DATA_START_VANCLEEF_EVENT,
    DATA_END_VANCLEEF_EVENT,
    DATA_PREPARE_VANCLEEF_EVENT,
    DATA_GLUBTOK_EVENT,
    DATA_HELIX_EVENT,
    DATA_REEPER_EVENT,
    DATA_WORGEN_EVENT,
    DATA_TRAP_COMPLETE,
	DATA_RIPSNARL,
	DATA_VANESSA,
	DATA_VANNESSA_NIGHTMARE,


    DATA_FIREWALL,
    DATA_PRODIGY,
	DATA_CANNON_BLAST_TIMER,

	DATA_NIGHTMARE_HELIX,
	DATA_NIGHTMARE_MECHANICAL,
};

enum DeadminesBoss
{
    NPC_GLUBTOK                     = 47162,
    NPC_HELIX                       = 47296,
    NPC_CAPTAIN_COOKIE              = 47739,
    NPC_VANCLEEF                    = 49541,
	NPC_FOE_REAPER_5000             = 43778,
	NPC_ADMIRAL_RIPSNARL            = 47626,
    // combat npcs
	NPC_HARVEST_TARGET              = 47468,
    NPC_FIRE_BLOSSOM                = 48957,
    NPC_FIRE_BLOSSOM_BUNNY          = 47282,
    NPC_FROST_BLOSSOM               = 48958,
    NPC_FROST_BLOSSOM_BUNNY         = 47284,
};

enum DeadminesNPCs
{
    NPC_LUMBERING_OAF               = 47297,
	NPC_OGRE_HENCHMAN               = 48230,
	NPC_DEFIAS_PIRATE               = 48522,
	NPC_SCORCH_MARK_BUNNY_JMF       = 48446,
	NPC_GENERAL_PURPOSE_BUNNY_JMF   = 45979,
	NPC_GENERAL_PURPOSE_BUNNY_JMF2  = 47242,
	NPC_VAPOR                       = 47714,
	NPC_ROPE_ANCHOR                 = 49552,
	NPC_TRAP_BUNNY                  = 49454,
};

enum eSound
{
    SOUND_CANNONFIRE                = 1400,
    SOUND_DESTROYDOOR               = 3079,
};

enum DeadmineGOBs
{
    GO_GLUBTOK_DOOR                 = 13965,
    GO_HELIX_DOOR_ENTRANCE          = 17153,
    GO_HELIX_DOOR_EXIT              = 16400,
    GO_FOE_ENTRANCE                 = 17154,
    GO_FOE_EXIT                     = 16399,
    GO_IRON_CLAD_DOOR               = 16397,
};

const Position CookieSpawn = 
{
	-88.1319f, -819.33f, 39.23453f, 0.0f
};

Position const platformJump =  {-229.03f, -576.95f, 51.3f, 0.0f};
Position const vancleefEventSpawn =  {-75.74f, -819.87f, 47.48f, 0.02f};
Position const Part1EventPos =  {-174.47f, -578.50f, 19.32f, 3.20f};
Position const Part2EventPos =  {-158.11f, -580.29f, 19.32f, 3.10f};
Position const Part3EventPos =  {-100.72f, -663.50f, 7.42f, 1.97f};

Position const PartHarringtom[3] =
{
    {-88.96f, -720.50f, 8.51f},
    {6.78f, -766.50f, 9.63f, 2.53f},
    {-83.86f, -775.28f, 28.38f, 1.71f},
};

Position const PartWorgen[6] =
{
    {0.85f, -764.91f, 9.72f, 5.47f},
    {7.85f, -764.41f, 9.59f, 4.35f},
    {4.84f, -770.50f, 9.63f, 1.30f},

    {-97.79166f, -717.8542f, 8.668088f, 4.520403f},
    {-94.40278f, -719.7274f, 8.598646f, 3.560472f},
    {-101.9167f, -718.7552f, 8.726379f, 5.51524f},
};

enum Achievements
{
    ACHIEVEMENT_VIGOROUS_VANCLEEF_VINDICATOR = 5371,
    ACHIEVEMENT_IM_ON_A_DIET                 = 5370,
};

enum VancCleefEventSpells
{
    SPELL_MAGMA_VEHICLE_DUMMY = 92378,

    SPELL_EJECT_ALL_PASSENGERS = 68576,
    SPELL_FORECAST_MAGMA_TRAP_THROW = 92489,
    SPELL_FORECAST_MAGMA_TRAP_JUMP = 92438,


    SPELL_NIGHTMARE_SLOW = 92559,
    SPELL_NIGHTMARE = 92563,
    SPELL_NIGHTMARE_1 = 92113,
    SPELL_NIGHTMARE_FLAME = 92169, // entry 51594


    SPELL_FORGOTTEN_AURA = 48143, // on NPC_VANCLEEF_EVENT_2, NPC_GLUBTOK_EVENT
    SPELL_COSMETIC_ALPHA_50_PCT = 69676, // on NPC_VANCLEEF_EVENT_2, NPC_GLUBTOK_EVENT

    SPELL_NIGHTMARE_AURA_CANCEL = 92583, // casted by NPC_VANCLEEF_EVENT_2
    SPELL_AURA_NIGHTMARE_2 = 92565, // casted by NPC_VANCLEEF_EVENT_2
    SPELL_AURA_NIGHTMARE_3 = 92566, // casted by NPC_VANCLEEF_EVENT_2
    SPELL_AURA_NIGHTMARE_4 = 92567, // casted by NPC_VANCLEEF_EVENT_2
    SPELL_AURA_NIGHTMARE_5 = 92568, // casted by NPC_VANCLEEF_EVENT_2

    SPELL_SUMMON_ICETOMB = 92189, // casted by 45979 (bunny) summon NPC_COLLAPSING_ICICLE
    SPELL_ICETOMB_TARGET = 92210, // casted by 45979 (bunny) 1087757
    SPELL_GLACIATE = 92201, // casted by NPC_COLLAPSING_ICICLE trigger 92203 and 62463 that summon 33174
    SPELL_GLACIATE_DUMMY = 92202, // casted by NPC_COLLAPSING_ICICLE

    SPELL_HELIX_VEHICLE = 88337,
    SPELL_SHADOW_FOG = 92792, // casted by 45979 (bunny)
    SPELL_HELIX_CT_VEHICLE_DUMMY = 88349,
    SPELL_HELIX_CT_VEHICLE = 88360,

    SPELL_LIGHTING = 95519, // 49521
    SPELL_LIGHTING_2 = 92277, // 49521

    SPELL_ADRENALINE = 92604,
    SPELL_ROPE_RAY                  = 43785, // on 49552
    SPELL_VAPOR_EVENT = 92401,
    SPELL_AURA_VAPOR_EVENT = 92399,

	SPELL_THROW = 91038,
	SPELL_THROW_H = 91039,
	SPELL_CANNONBALL = 89697, // 91066 (HC) supported by Spell Difficulty
};

enum VanCleefEvents
{
    EVENT_VANCLEEF_EVENT = 1,
    CHECK_PLAYERS_ALIVE,
    CHECK_ACH_FAILED,
    PHASE_EVENT_NOT_STARTED,
    PHASE_EVENT_IN_PROGRESS,
    PHASE_EVENT_DONE,
    PHASE_EVENT_GLUBTOK,
    PHASE_EVENT_HELIX,
    PHASE_REEPER,
    PHASE_WORGEN,
    ACTION_START_FIGHT = 7,
    ACTION_ACTIVATE,
    ACTION_DESACTIVATE,
    ACTION_INIT_EVENT,
    ACTION_EJECT_PASSENGERS,
    ACTION_EMOTE_TRAP,
};

enum CanCleefEventnpcs
{
    // vancleef evet
    NPC_MAGMA_VEHICLE               = 49454,
    NPC_VANCLEEF_EVENT              = 49429,
    NPC_NIGHTMARE_FLAMES            = 51594,
    NPC_VANCLEEF_EVENT_2            = 49671,
    NPC_GLUBTOK_EVENT               = 49670,
    NPC_COLLAPSING_ICICLE           = 49481,
    NPC_SNOWPACKED_ICICLE           = 33174,
    NPC_HELIX_EVENT                 = 49674,
    NPC_REEPER_EVENT                = 49681,

    NPC_DARKWEB_DEVOURER            = 49494,
    NPC_CHATTERING_HORROR           = 49495,
    NPC_NIGHTMARE_SKITTERLING       = 49493,

    NPC_VANESSA_LIGHTING_PLATTER = 49520,
    NPC_VANESSA_LIGHTING_PLATTER_PASSENGER = 49521,

    NPC_ENRAGED_WORGEN = 49532,

    NPC_HERIC_HARRINGTOM = 49535,
    NPC_JAMES_HARRINGTOM = 49539,
    NPC_EMMA_HARRINGTOM = 49534,
    NPC_CALLISA_HARRINGTOM = 49536,

    NPC_EDWIN_VANCLEEF = 42697, // ???

    NPC_VANESSA_PAPER = 49564,
    NPC_VAPOR_VALVE = 49457,
};

#endif
