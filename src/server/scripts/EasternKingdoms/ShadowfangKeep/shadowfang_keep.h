
#ifndef DEF_SHADOWFANG_H
#define DEF_SHADOWFANG_H

#define DataHeader "SK"

enum ShadowfangKeepBoss
{
   BOSS_BARON_ASHBURY             = 46962,
   BOSS_BARON_SILVERLAINE         =  3887,
   BOSS_COMMANDER_SPRINGVALE      =  4278,
   BOSS_LORD_GODFREY              = 46964,
   BOSS_LORD_WALDEN               = 46963,
};

enum Npcs
{
   NPC_TORMENTED_OFFICER          = 50615,
   NPC_WAILING_GUARDSMAN          = 50613,
   NPC_DREAD_SCRYER               = 47141,
   NPC_DESECRATION_TR             = 50503,
   NPC_TOXIN_TRIGGER              = 50439,
   NPC_HAUNTED_TELEPORTER         = 51400,
   // Apothecary Trio - Love is in the Air Festival
   NPC_HUMMEL                     = 36296,
   NPC_BAXTER                     = 36565,
   NPC_FRYE                       = 36272,
//   NPC_HORDE_QUESTGIVER
//   NPC_ALLIANCE_QUESTGIVER
};

enum Achievements
{
   ACHIEVEMENT_TO_THE_GROUND      = 5504,
};

enum GameObjectIds
{
   GO_BARON_ASHBURY_DOOR          = 18895,
   GO_LORD_GODFREY_DOOR           = 18971,
   GO_SORCERS_GATE                = 18972,
};

enum Data
{
    DATA_BARON_ASHBURY_EVENT,
    DATA_BARON_SILVERLAINE_EVENT,
    DATA_COMMANDER_SPRINGVALE_EVENT,
    DATA_LORD_GODFREY_EVENT,
    DATA_LORD_WALDEN_EVENT,
    MAX_ENCOUNTER,

    TEAM_IN_INSTANCE
};

enum TeleporterDatas
{
    DATA_TELEPORT_HIGH_HALL = 1,
    DATA_TELEPORT_CHAPELLE,
    DATA_TELEPORT_REMPARTS,
    DATA_TELEPORT_LABO,
};

enum Data64
{
    DATA_BARON_ASHBURY,
    DATA_BARON_SILVERLAINE,
    DATA_COMMANDER_SPRINGVALE,
    DATA_LORD_GODFREY,
    DATA_LORD_WALDEN
};

#endif
