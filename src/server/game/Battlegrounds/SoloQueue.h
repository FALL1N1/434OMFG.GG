#ifndef SOLO_QUEUE_H
#define SOLO_QUEUE_H

#define SPELL_DESERTER_BUFF 26013

enum SoloQueueTalentCategory
{
    TALENT_CAT_UNKNOWN  = 0,
    TALENT_CAT_RANGE    = 1,
    TALENT_CAT_HEALER   = 2,
    TALENT_CAT_MELEE    = 3
};

struct SoloQueueInfo                                                                // stores information about the group in queue (also used when joined as solo!)
{
    uint64  playerGuid;                                                             // player guid
    uint32  rating;                                                                 // Solo Team rating
    uint32  ArenaMatchmakerRating;                                                  // Player MMR
    uint32  teamId;                                                                 // Solo Arena Team Id
    uint32  ratingRange = 0;                                                        // Rating range for team forming
    uint32  ratingRangeIncreaseCounter = 0;                                         // Rating increase counter for team forming
    uint32  lastMmrUpdate = getMSTime();                                            // Rating update timer
    uint32  team;                                                                   // ALLIANCE or HORDE
};

#define sSoloQueueMgr SoloQueue::instance()

class SoloQueue
{
    public:
        SoloQueue();
        ~SoloQueue();
        void Update(const uint32 diff);
        uint32 GetPlayerCountInQueue(SoloQueueTalentCategory talentCategory, bool allPlayers = false);
        void AddPlayer(Player* player);
        bool RemovePlayer(uint64 guid);
        bool IsPlayerInSoloQueue(Player* player);
        bool CheckRequirements(Player* player);
        static SoloQueue* instance();
    private:
        void IncreasePlayerMMrRange(SoloQueueInfo* playerInfo);
        bool IsInMmrRange(uint32 min, uint32 max, uint32 mmr) { return mmr >= min && mmr <= max; };
        uint8 GetPlayerTalentCategory(Player* player);
        std::multimap<uint32/* mmr */, SoloQueueInfo*> queuedHealers;
        std::multimap<uint32/* mmr */, SoloQueueInfo*> queuedMelees;
        std::multimap<uint32/* mmr */, SoloQueueInfo*> queuedRanges;
        std::map<uint64, SoloQueueInfo*> allPlayersInQueue;
        uint32 lastUpdateTime;
};

#endif