/*Copyright (C) 2012 SkyMist Project.
*
*
* This file is NOT free software. Third-party users can NOT redistribute it or modify it :).
* If you find it, you are either hacking something, or very lucky (presuming someone else managed to hack it).
*/

#ifndef DEF_BASTION_OF_TWILIGHT_H
#define DEF_BASTION_OF_TWILIGHT_H

#define BOTScriptName "instance_bastion_of_twilight"
#define DataHeader "BT"

uint32 const EncounterCount = 5;

enum DataTypes
{
    // Encounters
    DATA_HALFUS                     = 0,
    DATA_VALIONA_THERALION          = 1,
    DATA_ASCENDANT_COUNCIL          = 2,
    DATA_CHOGALL                    = 3,
    DATA_SINESTRA                   = 4,

    // NPC GUIDs
    DATA_AC_PHASE                   = 6,
    DATA_FELUDIUS                   = 7,
    DATA_IGNACIOUS                  = 8,
    DATA_ARION                      = 9,
    DATA_TERRASTRA                  = 10,
    DATA_ELEMENTIUM_MONSTROSITY     = 11,

    // custom content
    DATA_GRAVITY_CRUSH              = 12,
    DATA_WRACK_DURATION             = 13
};

enum TelepportationsData
{
    // Teleportations
    DATA_TELEPORT_VT       = 1,
    DATA_TELEPORT_COUNCIL,
    DATA_TELEPORT_CHOGALL,
    DATA_TELEPORT_SINESTRA,
};

enum CreaturesIds
{
    // Halfus
    NPC_HALFUS_WORMBREAKER                      = 44600,
    // Valiona and Theralion
    NPC_VALIONA_BOT                             = 45992,
    NPC_THERALION_BOT                           = 45993,
    // Ascendant Lord Council
    NPC_ASCENDANT_CONTROLLER                    = 43691,
    NPC_ASCENDANT_COUNCIL                       = 45420,
    NPC_FELUDIUS                                = 43687,
    NPC_IGNACIOUS                               = 43686,
    NPC_ARION                                   = 43688,
    NPC_TERRASTRA                               = 43689,
    NPC_ELEMENTIUM_MONSTROSITY                  = 43735,
    // Cho'gall
    NPC_CHOGALL                                 = 43324,
    // Sinestra
    NPC_SINESTRA                                = 45213,
    NPC_CALEN                                   = 46277,

   // Intro's and cutscenes - Misc
    NPC_CHOGALL_HALFUS                          = 46965,
    NPC_CHOGALL_DRAGONS                         = 48142,
    NPC_CHOGALL_COUNCIL                         = 46900,

    NPC_TELEPORTER                              = 920001,
};

enum Gameobjects
{
    GOB_HALFUS_ENTRANCE                 = 205222,
    GOB_HALFUS_EXIT                     = 205223,
    GOB_DRAGON_SIBLINGS_DOOR_ENTRANCE   = 205224,
    GOB_DRAGON_SIBLINGS_DOOR_EXIT       = 205225,
    GOB_ASCENDANT_COUNCIL_ENTRANCE      = 205226,
    GOB_ASCENDANT_COUNCIL_EXIT          = 205227,
    GOB_CHOGALL_ENTRANCE                = 205228,
    GOB_TRAPDOOR                        = 205898,
    GOB_SINESTRA_DOOR                   = 207679,
};

#endif // DEF_BASTION_OF_TWILIGHT_H
