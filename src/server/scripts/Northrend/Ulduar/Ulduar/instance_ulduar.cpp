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

#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "InstanceScript.h"
#include "ulduar.h"

static DoorData const doorData[] =
{
    {   GO_LEVIATHAN_DOOR, BOSS_LEVIATHAN,    DOOR_TYPE_ROOM, BOUNDARY_S      },
    {   GO_XT_002_DOOR,    BOSS_XT002,        DOOR_TYPE_ROOM, BOUNDARY_S      },
    { GO_YOGG_SARON_DOOR,               BOSS_YOGG_SARON,        DOOR_TYPE_ROOM,         BOUNDARY_S      },
    {   0,                 0,                 DOOR_TYPE_ROOM, BOUNDARY_NONE   },
};

// Used to map boss-identifier to "player-died-in-fight-against"-flag, since not all "bosses" are relevant
static const uint32 BossId_2_PlayerDiedFlag[][2] =
{
    { BOSS_LEVIATHAN        , DEAD_FLAME_LEVIATHAN  },
    { BOSS_IGNIS            , DEAD_IGNIS            },
    { BOSS_RAZORSCALE       , DEAD_RAZORSCALE       },
    { BOSS_XT002            , DEAD_XT002            },
    { BOSS_ASSEMBLY_OF_IRON , DEAD_ASSEMBLY         },
    { BOSS_STEELBREAKER     , DEAD_NONE             },
    { BOSS_MOLGEIM          , DEAD_NONE             },
    { BOSS_BRUNDIR          , DEAD_NONE             },
    { BOSS_KOLOGARN         , DEAD_KOLOGARN         },
    { BOSS_AURIAYA          , DEAD_AURIAYA          },
    { BOSS_MIMIRON          , DEAD_MIMIRON          },
    { BOSS_HODIR            , DEAD_HODIR            },
    { BOSS_THORIM           , DEAD_THORIM           },
    { BOSS_FREYA            , DEAD_FREYA            },
    { BOSS_BRIGHTLEAF       , DEAD_NONE             },
    { BOSS_IRONBRANCH       , DEAD_NONE             },
    { BOSS_STONEBARK        , DEAD_NONE             },
    { BOSS_VEZAX            , DEAD_VEZAX            },
    { BOSS_YOGG_SARON        , DEAD_YOGGSARON        },
    { BOSS_ALGALON          , DEAD_ALGALON          },
};

class instance_ulduar : public InstanceMapScript
{
    public:
        instance_ulduar() : InstanceMapScript(UlduarScriptName, MAP_ULDUAR) { }

        struct instance_ulduar_InstanceMapScript : public InstanceScript
        {
            instance_ulduar_InstanceMapScript(InstanceMap* map) : InstanceScript(map) { }

        private:
            // 2012/06/24 @DorianGrey - reordered those, due to logical relation (GO to npc and vice versa). Maybe change that again.

            // Leviathan
            uint64 leviathanChestGUID;
            uint64 LeviathanGUID;
            uint64 LeviathanGateGUID;
            std::list<uint64> LeviathanDoorGUIDList;

            uint64 uiBrannGUID;

            // Ignis
            uint64 IgnisGUID;

            // Razorscale
            uint64 RazorscaleGUID;
            uint64 RazorscaleController;
            uint64 RazorHarpoonGUIDs[4];
            uint64 ExpeditionCommanderGUID;

            // XT-002
            uint64 XT002GUID;
            uint64 XT002DoorGUID;
            uint64 XTToyPileGUIDs[4];

            // Assembly of Iron
            uint64 AssemblyGUIDs[3];
            uint64 IronCouncilEntranceGUID;
            uint64 ArchivumDoorGUID;

            // Kologarn
            uint64 KologarnGUID;
            uint64 KologarnChestGUID;
            uint64 KologarnBridgeGUID;
            uint64 KologarnDoorGUID;
            std::set<uint64> mRubbleSpawns;

            // Auriaya
            uint64 AuriayaGUID;

            // Hodir
            uint64 HodirGUID;
            uint64 HodirIceDoorGUID;
            uint64 HodirStoneDoorGUID;
            uint64 HodirEntranceDoorGUID;
            uint64 HodirChestGUID;
            uint64 HodirRareCacheGUID;
            uint32 DataCaille;
            uint32 DataGare;

            // Mimiron
            uint64 MimironTrainGUID;
            uint64 MimironGUID;
            uint64 LeviathanMKIIGUID;
            uint64 VX001GUID;
            uint64 AerialUnitGUID;
            uint64 MimironElevatorGUID;
            uint64 MimironChestHMGUID;
            std::list<uint64> MimironDoorGUIDList;

            // Thorim
            uint64 ThorimCtrlGUID;
            uint64 ThorimGUID;
            uint64 ThorimLightningFieldGUID;
            uint64 RunicColossusGUID;
            uint64 RuneGiantGUID;
            uint64 RunicDoorGUID;
            uint64 StoneDoorGUID;
            uint64 ThorimChestGUID;
            uint64 ThorimDarkIronPortCullisGUID;


            // Freya
            uint64 FreyaGUID;
            uint64 ElderBrightleafGUID;
            uint64 ElderIronbranchGUID;
            uint64 ElderStonebarkGUID;
            uint64 FreyaChestGUID;
            uint64 KeeperGUIDs[3];

            // Vezax
            uint64 WayToYoggGUID;
            uint64 VezaxGUID;
            uint64 VezaxDoorGUID;

            // Yogg-Saron
            uint64 YoggSaronGUID;
            uint64 VoiceOfYoggSaronGUID;
            uint64 SaraGUID;
            uint64 BrainOfYoggSaronGUID;
            uint64 YKeeperGUIDs[4];

            // Algalon
            uint64 uiAlgalonGUID;
            uint64 AlgalonBrannGUID;
            uint64 AlgalonDoorGUID;
            uint64 AlgalonFloorOOCGUID;
            uint64 AlgalonFloorCOMGUID;
            uint64 AlgalonBridgeGUID;
            uint64 AlgalonGlobeGUID;
            uint64 AlgalonForceFieldGUID;
            uint32 AlgalonIntroDone;
            uint32 SignalTimerState;
            uint32 SignalTimer;
            uint32 SignalTimerMinutes;
            uint32 uiAlgalonCountdown;
            uint32 uiAlgalonKillCount;
            uint32 uiCountdownTimer;
            uint64 GiftOfTheObserverGUID;

            EventMap _events;
            uint32 _algalonTimer;
            bool _summonAlgalon;
            bool _algalonSummoned;
            uint32 _maxArmorItemLevel;
            uint32 _maxWeaponItemLevel;


            uint64 AlgalonSigilDoorGUID[3];
            uint64 AlgalonFloorGUID[2];
            uint64 AlgalonUniverseGUID;
            uint64 AlgalonTrapdoorGUID;

            // Miscellaneous
            uint32 ColossusData;
            uint32 SupportKeeperFlag;
            uint32 PlayerDeathFlag;

            uint32 TeamInInstance;
            uint32 HodirRareCacheData;
            //   62 - not ready to engage, i.e. Yogg-Saron is still alive
            //   61 - ready to engage, not engaged yet
            // < 61 - engaged, timer running
            //    0 - failed

            uint8 elderCount;
            bool conSpeedAtory;
            uint8 illusion;
            uint8 keepersCount;

            uint32 armhf;

            bool _summonObservationRingKeeper[4];
            bool _summonYSKeeper[4];
            uint64 BrainRoomDoorGUIDs[3];

        public:
            void Initialize()
            {
                // Pretty please: Use type-safe fill instead of raw memset !
                SetHeaders(DataHeader);
                SetBossNumber(MAX_ENCOUNTER);
                LoadDoorData(doorData);

                DataCaille = 0;
                DataGare = 0;

                // Leviathan
                leviathanChestGUID  = 0;
                LeviathanGUID       = 0;
                LeviathanGateGUID   = 0;

                uiBrannGUID = 0;

                // Ignis
                IgnisGUID = 0;

                // Razorscale
                RazorscaleGUID          = 0;
                RazorscaleController    = 0;
                ExpeditionCommanderGUID = 0;
                std::fill(RazorHarpoonGUIDs, RazorHarpoonGUIDs + 4, 0);

                // XT-002
                XT002GUID       = 0;
                XT002DoorGUID   = 0;
                std::fill(XTToyPileGUIDs, XTToyPileGUIDs + 4, 0);

                // Assembly of Iron
                std::fill(AssemblyGUIDs, AssemblyGUIDs + 3, 0);
                IronCouncilEntranceGUID = 0;
                ArchivumDoorGUID        = 0;

                // Kologarn
                KologarnGUID        = 0;
                KologarnChestGUID   = 0;
                KologarnBridgeGUID  = 0;
                KologarnDoorGUID    = 0;
                armhf = 0;

                // Auriaya
                AuriayaGUID = 0;

                // Hodir
                HodirGUID               = 0;
                HodirIceDoorGUID        = 0;
                HodirStoneDoorGUID      = 0;
                HodirEntranceDoorGUID   = 0;
                HodirChestGUID          = 0;
                HodirRareCacheGUID      = 0;

                // Mimiron
                MimironTrainGUID    = 0;
                MimironGUID         = 0;
                LeviathanMKIIGUID   = 0;
                VX001GUID           = 0;
                AerialUnitGUID      = 0;
                MimironElevatorGUID = 0;
                MimironChestHMGUID = 0;

                // Thorim
                ThorimCtrlGUID               = 0;
                ThorimGUID                   = 0;
                ThorimLightningFieldGUID     = 0;
                RunicColossusGUID            = 0;
                RuneGiantGUID                = 0;
                RunicDoorGUID                = 0;
                StoneDoorGUID                = 0;
                ThorimChestGUID              = 0;
                ThorimDarkIronPortCullisGUID = 0;


                // Freya
                FreyaGUID           = 0;
                ElderBrightleafGUID = 0;
                ElderIronbranchGUID = 0;
                ElderStonebarkGUID  = 0;
                FreyaChestGUID      = 0;

                // Vezax
                WayToYoggGUID   = 0;
                VezaxGUID       = 0;
                VezaxDoorGUID   = 0;

                // Yogg-Saron
                YoggSaronGUID                    = 0;
                VoiceOfYoggSaronGUID             = 0;
                SaraGUID                         = 0;
                BrainOfYoggSaronGUID             = 0;

                // Algalon
                uiAlgalonGUID             = 0;
                AlgalonBrannGUID          = 0;
                AlgalonDoorGUID           = 0;
                AlgalonBridgeGUID         = 0;
                AlgalonGlobeGUID          = 0;
                AlgalonFloorOOCGUID       = 0;
                AlgalonFloorCOMGUID       = 0;
                AlgalonForceFieldGUID     = 0;
                SignalTimerState          = NOT_STARTED;
                SignalTimer               = 0;
                SignalTimerMinutes        = 0;
                uiAlgalonKillCount        = 0;
                uiAlgalonCountdown        = 62;
                uiCountdownTimer          = 1*MINUTE*IN_MILLISECONDS;
                AlgalonIntroDone = false;
                GiftOfTheObserverGUID            = 0;
                _algalonSummoned                 = false;
                _summonAlgalon                   = false;

                _algalonTimer                    = 61;
                _maxArmorItemLevel               = 0;
                _maxWeaponItemLevel              = 0;

                memset(AlgalonSigilDoorGUID, 0, sizeof(AlgalonSigilDoorGUID));
                memset(AlgalonFloorGUID, 0, sizeof(AlgalonFloorGUID));
                memset(_summonObservationRingKeeper, false, sizeof(_summonObservationRingKeeper));
                AlgalonUniverseGUID = 0;
                AlgalonTrapdoorGUID = 0;
                // Creatures
                memset(YKeeperGUIDs, 0, sizeof(YKeeperGUIDs));
                memset(KeeperGUIDs, 0, sizeof(KeeperGUIDs));
                memset(BrainRoomDoorGUIDs, 0, sizeof(BrainRoomDoorGUIDs));

                // Miscellaneous
                ColossusData        = 0;
                SupportKeeperFlag   = 0;
                PlayerDeathFlag     = 0;

                TeamInInstance      = 0;
                HodirRareCacheData  = 0;
                elderCount          = 0;
                illusion                         = 0;
                keepersCount                     = 0;

                conSpeedAtory       = false;

            }

            void Update(uint32 diff)
            {
                if (_events.Empty())
                    return;

                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_UPDATE_ALGALON_TIMER:
                        SaveToDB();
                        DoUpdateWorldState(WORLD_STATE_ALGALON_DESPAWN_TIMER, --_algalonTimer);
                        if (_algalonTimer)
                            _events.ScheduleEvent(EVENT_UPDATE_ALGALON_TIMER, 60000);
                        else
                        {
                            DoUpdateWorldState(WORLD_STATE_ALGALON_TIMER_ENABLED, 0);
                            _events.CancelEvent(EVENT_UPDATE_ALGALON_TIMER);
                            if (Creature* algalon = instance->GetCreature(uiAlgalonGUID))
                                algalon->AI()->DoAction(EVENT_DESPAWN_ALGALON);
                        }
                        break;
                    }
                }
            }

            void OpenDoor(uint64 guid)
            {
                if (!guid)
                    return;
                if (GameObject* go = instance->GetGameObject(guid))
                    go->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);
            }

            void CloseDoor(uint64 guid)
            {
                if (!guid)
                    return;
                if (GameObject* go = instance->GetGameObject(guid))
                    go->SetGoState(GO_STATE_READY);
            }

            void OnPlayerEnter(Player* player)
            {
                if (!TeamInInstance)
                    TeamInInstance = player->GetTeam();


                if (_summonAlgalon)
                {
                    _summonAlgalon = false;
                    TempSummon* algalon = instance->SummonCreature(NPC_ALGALON, AlgalonLandPos);
                    if (_algalonTimer && _algalonTimer <= 60)
                        algalon->AI()->DoAction(ACTION_INIT_ALGALON);
                    else
                        algalon->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                }
                // Keepers at Observation Ring
                if (GetBossState(BOSS_FREYA) == DONE && _summonObservationRingKeeper[0] && !YKeeperGUIDs[0])
                {
                    _summonObservationRingKeeper[0] = false;
                    instance->SummonCreature(NPC_FREYA_OBSERVATION_RING, ObservationRingKeepersPos[0]);
                }
                if (GetBossState(BOSS_HODIR) == DONE && _summonObservationRingKeeper[1] && !YKeeperGUIDs[1])
                {
                    _summonObservationRingKeeper[1] = false;
                    instance->SummonCreature(NPC_HODIR_OBSERVATION_RING, ObservationRingKeepersPos[1]);
                }
                if (GetBossState(BOSS_THORIM) == DONE && _summonObservationRingKeeper[2] && !YKeeperGUIDs[2])
                {
                    _summonObservationRingKeeper[2] = false;
                    instance->SummonCreature(NPC_THORIM_OBSERVATION_RING, ObservationRingKeepersPos[2]);
                }
                if (GetBossState(BOSS_MIMIRON) == DONE && _summonObservationRingKeeper[3] && !YKeeperGUIDs[3])
                {
                    _summonObservationRingKeeper[3] = false;
                    instance->SummonCreature(NPC_MIMIRON_OBSERVATION_RING, ObservationRingKeepersPos[3]);
                }

                // Keepers in Yogg-Saron's room
                if (_summonYSKeeper[0])
                    instance->SummonCreature(NPC_FREYA_YS, YSKeepersPos[0]);
                if (_summonYSKeeper[1])
                    instance->SummonCreature(NPC_HODIR_YS, YSKeepersPos[1]);
                if (_summonYSKeeper[2])
                    instance->SummonCreature(NPC_THORIM_YS, YSKeepersPos[2]);
                if (_summonYSKeeper[3])
                    instance->SummonCreature(NPC_MIMIRON_YS, YSKeepersPos[3]);
            }

            void __OnPlayerDeath(Player* /*player*/)
            {
                for (uint8 i = 0; i < MAX_ENCOUNTER; i++)
                {
                    if (GetBossState(UlduarBosses(i)) == IN_PROGRESS)
                    {
                        PlayerDeathFlag |= BossId_2_PlayerDiedFlag[i][1];
                    }
                    if (GetBossState(BOSS_ALGALON) == IN_PROGRESS)
                    {
                        uiAlgalonKillCount++; // He feeds on your tears
                    }
                }
            }

            void __OnCreatureDeath(Creature* creature)
            {
                switch (creature->GetEntry())
                {
                    case NPC_CORRUPTED_SERVITOR:
                    case NPC_MISGUIDED_NYMPH:
                    case NPC_GUARDIAN_LASHER:
                    case NPC_FOREST_SWARMER:
                    case NPC_MANGROVE_ENT:
                    case NPC_IRONROOT_LASHER:
                    case NPC_NATURES_BLADE:
                    case NPC_GUARDIAN_OF_LIFE:
                        if (!conSpeedAtory)
                        {
                            DoStartTimedAchievement(ACHIEVEMENT_TIMED_TYPE_EVENT, ACHIEVEMENT_CRITERIA_CON_SPEED_ATORY);
                            conSpeedAtory = true;
                        }
                        break;
                    // reward leviathan kill all over the formation grounds area
                    // otherwise, there may occur some problems due to kill-credit since he's killed using vehicles
                    case NPC_LEVIATHAN:
                        {
                            Map::PlayerList const& playerList = instance->GetPlayers();

                            if (playerList.isEmpty())
                                return;

                            for (Map::PlayerList::const_iterator i = playerList.begin(); i != playerList.end(); ++i)
                            {
                                if (Player* player = i->getSource())
                                {
                                    // has been rewarded
                                    if (player->IsAtGroupRewardDistance(creature))
                                        continue;

                                    // is somewhere else
                                    if (player->GetAreaId() != AREA_FORMATION_GROUNDS)
                                        continue;

                                    if (player->isAlive() || !player->GetCorpse())
                                        player->KilledMonsterCredit(NPC_LEVIATHAN, 0);
                                }
                            }
                        }
                        break;
                    default:
                        break;
                }
            }

            void OnUnitDeath(Unit* unit)
            {
                if (Player* player = unit->ToPlayer())
                    __OnPlayerDeath(player);
                else if (Creature* creature = unit->ToCreature())
                    __OnCreatureDeath(creature);
            }

            bool CheckAchievementCriteriaMeet(uint32 criteria_id, Player const* /*source*/, Unit const* /*target = NULL*/, uint32 /*miscvalue1 = 0*/)
            {
                switch (criteria_id)
                {
                        // Kills without Death Achievement
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_FLAMELEVIATAN_10:
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_FLAMELEVIATAN_25:
                        return !(PlayerDeathFlag & DEAD_FLAME_LEVIATHAN);
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_IGNIS_10:
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_IGNIS_25:
                        return !(PlayerDeathFlag & DEAD_IGNIS);
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_RAZORSCALE_10:
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_RAZORSCALE_25:
                        return !(PlayerDeathFlag & DEAD_RAZORSCALE);
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_XT002_10:
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_XT002_25:
                        return !(PlayerDeathFlag & DEAD_XT002);
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_ASSEMBLY_10:
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_ASSEMBLY_25:
                        return !(PlayerDeathFlag & DEAD_ASSEMBLY);
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_KOLOGARN_10:
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_KOLOGARN_25:
                        return !(PlayerDeathFlag & DEAD_KOLOGARN);
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_AURIAYA_10:
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_AURIAYA_25:
                        return !(PlayerDeathFlag & DEAD_AURIAYA);
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_HODIR_10:
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_HODIR_25:
                        return !(PlayerDeathFlag & DEAD_HODIR);
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_THORIM_10:
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_THORIM_25:
                        return !(PlayerDeathFlag & DEAD_THORIM);
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_FREYA_10:
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_FREYA_25:
                        return !(PlayerDeathFlag & DEAD_FREYA);
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_MIMIRON_10:
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_MIMIRON_25:
                        return !(PlayerDeathFlag & DEAD_MIMIRON);
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_VEZAX_10:
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_VEZAX_25:
                        return !(PlayerDeathFlag & DEAD_VEZAX);
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_YOGGSARON_10:
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_YOGGSARON_25:
                        return !(PlayerDeathFlag & DEAD_YOGGSARON);
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_ALGALON_10:
                    case ACHIEVEMENT_CRITERIA_KILL_WITHOUT_DEATHS_ALGALON_25:
                        return !(PlayerDeathFlag & DEAD_ALGALON);
                    case CRITERIA_WAITS_DREAMING_STORMWIND_25:
                    case CRITERIA_WAITS_DREAMING_STORMWIND_10:
                        return illusion == STORMWIND_ILLUSION;
                    case CRITERIA_WAITS_DREAMING_CHAMBER_25:
                    case CRITERIA_WAITS_DREAMING_CHAMBER_10:
                        return illusion == CHAMBER_ILLUSION;
                    case CRITERIA_WAITS_DREAMING_ICECROWN_25:
                    case CRITERIA_WAITS_DREAMING_ICECROWN_10:
                        return illusion == ICECROWN_ILLUSION;
                }

                // Yogg-Saron
                switch (criteria_id)
                {
                    case ACHIEVEMENT_CRITERIA_THE_ASSASSINATION_OF_KING_LLANE_10:
                    case ACHIEVEMENT_CRITERIA_THE_ASSASSINATION_OF_KING_LLANE_25:
                        {
                            if (GetBossState(BOSS_YOGG_SARON) != IN_PROGRESS)
                                return false;


                            return false;
                        }
                    case ACHIEVEMENT_CRITERIA_THE_TORTURED_CHAMPION_10:
                    case ACHIEVEMENT_CRITERIA_THE_TORTURED_CHAMPION_25:
                        {
                            if (GetBossState(BOSS_YOGG_SARON) != IN_PROGRESS)
                                return false;


                            return false;
                        }
                    case ACHIEVEMENT_CRITERIA_FORGING_OF_THE_DEMON_SOUL_10:
                    case ACHIEVEMENT_CRITERIA_FORGING_OF_THE_DEMON_SOUL_25:
                        {
                            if (GetBossState(BOSS_YOGG_SARON) != IN_PROGRESS)
                                return false;


                            return false;
                        }
                }
                return false;
            }

            void OnCreatureCreate(Creature* creature)
            {
                switch (creature->GetEntry())
                {
                    case NPC_BRANN_EVENT_START_ULDU:
                        uiBrannGUID = creature->GetGUID();
                        break;
                    case NPC_ALGALON:
                        uiAlgalonGUID = creature->GetGUID();
                        if (AlgalonIntroDone && !SignalTimerMinutes)
                            creature->DespawnOrUnsummon();
                        creature->setActive(true);
                        return;
                    case NPC_BRANN_BRONZBEARD_ALG:
                        AlgalonBrannGUID = creature->GetGUID();
                        creature->setActive(true);
                        return;

                    // Misc. guys
                    case NPC_RUNIC_COLOSSUS:
                        RunicColossusGUID = creature->GetGUID();
                        break;
                    case NPC_RUNE_GIANT:
                        RuneGiantGUID = creature->GetGUID();
                        break;

                    // Flame Leviathan
                    case NPC_LEVIATHAN:
                        LeviathanGUID = creature->GetGUID();
                        break;

                    // Ignis
                    case NPC_IGNIS:
                        IgnisGUID = creature->GetGUID();
                        break;

                    // Razorscale
                    case NPC_RAZORSCALE:
                        RazorscaleGUID = creature->GetGUID();
                        break;
                    case NPC_RAZORSCALE_CONTROLLER:
                        RazorscaleController = creature->GetGUID();
                        break;
                    case NPC_EXPEDITION_COMMANDER:
                        ExpeditionCommanderGUID = creature->GetGUID();
                        break;

                    // XT002
                    case NPC_XT002:
                        XT002GUID = creature->GetGUID();
                        break;
                    case NPC_XT_TOY_PILE:
                        for (uint8 i = 0; i < 4; ++i)
                            if (!XTToyPileGUIDs[i])
                            {
                                XTToyPileGUIDs[i] = creature->GetGUID();
                                break;
                            }
                        break;

                    // Assembly of Iron
                    case NPC_STEELBREAKER:
                        AssemblyGUIDs[0] = creature->GetGUID();
                        break;
                    case NPC_MOLGEIM:
                        AssemblyGUIDs[1] = creature->GetGUID();
                        break;
                    case NPC_BRUNDIR:
                        AssemblyGUIDs[2] = creature->GetGUID();
                        break;

                    // Kologarn
                    case NPC_KOLOGARN:
                        KologarnGUID = creature->GetGUID();
                        break;
                    case NPC_KOLOGARN_BRIDGE:
                        // The below hacks are courtesy of the grid/visibility-system
                        // TODO: Check if this is still required
                        // What happens? The NPC as corpse build the bridge, but must not be present until Kologarn was beaten
                        if (GetBossState(BOSS_KOLOGARN) == DONE)
                        {
                            creature->setDeathState(CORPSE);
                            creature->DestroyForNearbyPlayers();
                            creature->UpdateObjectVisibility(true);
                        }
                        else
                        {
                            creature->setDeathState(CORPSE);
                            creature->RemoveCorpse(true);
                        }
                        break;

                    // Auriaya
                    case NPC_AURIAYA:
                        AuriayaGUID = creature->GetGUID();
                        break;

                    // Mimiron
                    case NPC_MIMIRON:
                        MimironGUID = creature->GetGUID();
                        break;
                    case NPC_LEVIATHAN_MKII:
                        LeviathanMKIIGUID = creature->GetGUID();
                        break;
                    case NPC_VX_001:
                        VX001GUID = creature->GetGUID();
                        break;
                    case NPC_AERIAL_COMMAND_UNIT:
                        AerialUnitGUID = creature->GetGUID();
                        break;

                    // Hodir
                    case NPC_HODIR:
                        HodirGUID = creature->GetGUID();
                        break;
                        // Hodir's Helper NPCs
                    case NPC_EIVI_NIGHTFEATHER:
                        if (TeamInInstance == HORDE)
                            creature->UpdateEntry(NPC_TOR_GREYCLOUD, HORDE);
                        break;
                    case NPC_ELLIE_NIGHTFEATHER:
                        if (TeamInInstance == HORDE)
                            creature->UpdateEntry(NPC_KAR_GREYCLOUD, HORDE);
                        break;
                    case NPC_ELEMENTALIST_MAHFUUN:
                        if (TeamInInstance == HORDE)
                            creature->UpdateEntry(NPC_SPIRITWALKER_TARA, HORDE);
                        break;
                    case NPC_ELEMENTALIST_AVUUN:
                        if (TeamInInstance == HORDE)
                            creature->UpdateEntry(NPC_SPIRITWALKER_YONA, HORDE);
                        break;
                    case NPC_MISSY_FLAMECUFFS:
                        if (TeamInInstance == HORDE)
                            creature->UpdateEntry(NPC_AMIRA_BLAZEWEAVER, HORDE);
                        break;
                    case NPC_SISSY_FLAMECUFFS:
                        if (TeamInInstance == HORDE)
                            creature->UpdateEntry(NPC_VEESHA_BLAZEWEAVER, HORDE);
                        break;
                    case NPC_FIELD_MEDIC_PENNY:
                        if (TeamInInstance == HORDE)
                            creature->UpdateEntry(NPC_BATTLE_PRIEST_ELIZA, HORDE);
                        break;
                    case NPC_FIELD_MEDIC_JESSI:
                        if (TeamInInstance == HORDE)
                            creature->UpdateEntry(NPC_BATTLE_PRIEST_GINA, HORDE);
                        break;

                    // Thorim
                    case NPC_THORIM:
                        ThorimGUID = creature->GetGUID();
                        break;
                    case NPC_THORIM_CTRL:
                        ThorimCtrlGUID = creature->GetGUID();
                        break;
                    case NPC_MERCENARY_CAPTAIN_A:
                        if (TeamInInstance == ALLIANCE)
                            creature->UpdateEntry(NPC_MERCENARY_CAPTAIN_H, 1692);
                        break;
                    case NPC_MERCENARY_SOLDIER_A:
                        if (TeamInInstance == ALLIANCE)
                            creature->UpdateEntry(NPC_MERCENARY_SOLDIER_H, 1692);
                        break;


                    // Freya
                    case NPC_FREYA:
                        FreyaGUID = creature->GetGUID();
                        break;
                        // Freya's Keeper
                    case NPC_IRONBRANCH:
                        KeeperGUIDs[0] = creature->GetGUID();
                        if (GetBossState(BOSS_FREYA) == DONE)
                            creature->DespawnOrUnsummon();
                        break;
                    case NPC_BRIGHTLEAF:
                        KeeperGUIDs[1] = creature->GetGUID();
                        if (GetBossState(BOSS_FREYA) == DONE)
                            creature->DespawnOrUnsummon();
                        break;
                    case NPC_STONEBARK:
                        KeeperGUIDs[2] = creature->GetGUID();
                        if (GetBossState(BOSS_FREYA) == DONE)
                            creature->DespawnOrUnsummon();
                        break;
                    // Vezax
                    case NPC_VEZAX:
                        VezaxGUID = creature->GetGUID();
                        break;
                    // Yogg-Saron
                    case NPC_YOGG_SARON:
                        YoggSaronGUID = creature->GetGUID();
                        break;
                    case NPC_VOICE_OF_YOGG_SARON:
                        VoiceOfYoggSaronGUID = creature->GetGUID();
                        break;
                    case NPC_BRAIN_OF_YOGG_SARON:
                        BrainOfYoggSaronGUID = creature->GetGUID();
                        break;
                    case NPC_SARA:
                        SaraGUID = creature->GetGUID();
                        break;
                    case NPC_FREYA_YS:
                        YKeeperGUIDs[0] = creature->GetGUID();
                        _summonYSKeeper[0] = false;
                        SaveToDB();
                        ++keepersCount;
                        break;
                    case NPC_HODIR_YS:
                        YKeeperGUIDs[1] = creature->GetGUID();
                        _summonYSKeeper[1] = false;
                        SaveToDB();
                        ++keepersCount;
                        break;
                    case NPC_THORIM_YS:
                        YKeeperGUIDs[2] = creature->GetGUID();
                        _summonYSKeeper[2] = false;
                        SaveToDB();
                        ++keepersCount;
                        break;
                    case NPC_MIMIRON_YS:
                        YKeeperGUIDs[3] = creature->GetGUID();
                        _summonYSKeeper[3] = false;
                        SaveToDB();
                        ++keepersCount;
                        break;
                    case NPC_SANITY_WELL:
                        creature->SetReactState(REACT_PASSIVE);
                        break;
                }
            }

            void OnCreatureRemove(Creature* creature)
            {
                switch (creature->GetEntry())
                {
                    case NPC_BRANN_BRONZBEARD_ALG:
                        if (AlgalonBrannGUID == creature->GetGUID())
                            AlgalonBrannGUID = 0;
                        break;
                }
            }

            void OnGameObjectCreate(GameObject* gameObject)
            {
                switch (gameObject->GetEntry())
                {
                    case GO_CELESTIAL_PLANETARIUM_ACCESS_10:
                    case GO_CELESTIAL_PLANETARIUM_ACCESS_25:
                        //if (_algalonSummoned)
                        //   gameObject->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
                        break;
                    case GO_DOODAD_UL_SIGILDOOR_01:
                        AlgalonSigilDoorGUID[0] = gameObject->GetGUID();
                        HandleGameObject(0, false, gameObject);
                        if (_algalonSummoned)
                        {
                            //                            gameObject->SetGoState(GO_STATE_ACTIVE);
                        }
                        break;
                    case GO_DOODAD_UL_SIGILDOOR_02:
                        AlgalonSigilDoorGUID[1] = gameObject->GetGUID();
                        HandleGameObject(0, false, gameObject);
                        if (_algalonSummoned)
                            gameObject->SetGoState(GO_STATE_ACTIVE);
                        break;
                    case GO_DOODAD_UL_SIGILDOOR_03:
                        AlgalonSigilDoorGUID[2] = gameObject->GetGUID();
                        HandleGameObject(0, true, gameObject);
                        AddDoor(gameObject, true);
                        break;
                    case GO_DOODAD_UL_UNIVERSEFLOOR_01:
                        AlgalonFloorGUID[0] = gameObject->GetGUID();
                        HandleGameObject(0, false, gameObject);
                        AddDoor(gameObject, true);
                        break;
                    case GO_DOODAD_UL_UNIVERSEFLOOR_02:
                        AlgalonFloorGUID[1] = gameObject->GetGUID();
                        HandleGameObject(0, false, gameObject);
                        AddDoor(gameObject, true);
                        break;
                    case GO_DOODAD_UL_UNIVERSEGLOBE01:
                        AlgalonUniverseGUID = gameObject->GetGUID();
                        HandleGameObject(0, false, gameObject);
                        AddDoor(gameObject, true);
                        break;
                    case GO_DOODAD_UL_ULDUAR_TRAPDOOR_03:
                        AlgalonTrapdoorGUID = gameObject->GetGUID();
                        HandleGameObject(0, false, gameObject);
                        AddDoor(gameObject, true);
                        break;

                    case GO_ULDUAR_DOME:
                        gameObject->EnableCollision(true);
                        break;

                    // Leviathan related
                    case GO_LEVIATHAN_DOOR:
                        AddDoor(gameObject, true);
                        break;
                    case GO_LEVIATHAN_GATE:
                        LeviathanGateGUID = gameObject->GetGUID();
                        //if (GetBossState(BOSS_LEVIATHAN) == IN_PROGRESS)
                        //    gameObject->SetGoState(GO_STATE_ACTIVE);

                        if (GetBossState(BOSS_LEVIATHAN) == DONE)
                            gameObject->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);
                        break;
                    case GO_LEVIATHAN_CHEST_10:
                    case GO_LEVIATHAN_CHEST_25:
                        leviathanChestGUID = gameObject->GetGUID();
                        break;

                    // Razorscale related
                    case GO_RAZOR_HARPOON_1:
                        RazorHarpoonGUIDs[0] = gameObject->GetGUID();
                        break;
                    case GO_RAZOR_HARPOON_2:
                        RazorHarpoonGUIDs[1] = gameObject->GetGUID();
                        break;
                    case GO_RAZOR_HARPOON_3:
                        RazorHarpoonGUIDs[2] = gameObject->GetGUID();
                        break;
                    case GO_RAZOR_HARPOON_4:
                        RazorHarpoonGUIDs[3] = gameObject->GetGUID();
                        break;
                    case GO_MOLE_MACHINE:
                        if (GetBossState(BOSS_RAZORSCALE) == IN_PROGRESS)
                            gameObject->SetGoState(GO_STATE_ACTIVE);
                        break;

                    // XT002 related
                    case GO_XT_002_DOOR:
                        AddDoor(gameObject, true);
                        XT002DoorGUID = gameObject->GetGUID();
                        HandleGameObject(0, GetBossState(BOSS_LEVIATHAN) == DONE, gameObject);
                        break;

                    // Iron-Council related
                    case GO_IRON_COUNCIL_ENTRANCE:
                        IronCouncilEntranceGUID = gameObject->GetGUID();
                        break;
                    case GO_ARCHIVUM_DOOR:
                        ArchivumDoorGUID = gameObject->GetGUID();
                        HandleGameObject(0, GetBossState(BOSS_ASSEMBLY_OF_IRON) == DONE, gameObject);
                        break;

                    // Kologarn related
                    case GO_KOLOGARN_CHEST_HERO:
                    case GO_KOLOGARN_CHEST:
                        KologarnChestGUID = gameObject->GetGUID();
                        break;
                    case GO_KOLOGARN_BRIDGE:
                        KologarnBridgeGUID = gameObject->GetGUID();
                        if (GetBossState(BOSS_KOLOGARN) == DONE)
                            HandleGameObject(0, false, gameObject);
                        break;
                    case GO_KOLOGARN_DOOR:
                        KologarnDoorGUID = gameObject->GetGUID();
                        break;

                    // Thorim related
                    case GO_THORIM_DARK_IRON_PROTCULLIS:
                        ThorimDarkIronPortCullisGUID = gameObject->GetGUID();
                        break;
                    case GO_THORIM_CHEST_HERO:
                    case GO_THORIM_CHEST:
                        ThorimChestGUID = gameObject->GetGUID();
                        break;
                    case GO_THORIM_LIGHTNING_FIELD:
                        ThorimLightningFieldGUID = gameObject->GetGUID();
                        break;
                    case GO_THORIM_STONE_DOOR:
                        StoneDoorGUID = gameObject->GetGUID();
                        break;
                    case GO_THORIM_RUNIC_DOOR:
                        RunicDoorGUID = gameObject->GetGUID();
                        break;


                    // Hodir related
                    case GO_HODIR_RARE_CACHE_OF_WINTER_HERO:
                    case GO_HODIR_RARE_CACHE_OF_WINTER:
                        HodirRareCacheGUID = gameObject->GetGUID();
                        break;
                    case GO_HODIR_CHEST_HERO:
                    case GO_HODIR_CHEST:
                        HodirChestGUID = gameObject->GetGUID();
                        break;
                    case GO_HODIR_OUT_DOOR_ICE:
                        HodirIceDoorGUID = gameObject->GetGUID();
                        if (GetBossState(BOSS_HODIR) == DONE)
                            HandleGameObject(HodirIceDoorGUID, true);
                        break;
                    case GO_HODIR_OUT_DOOR_STONE:
                        HodirStoneDoorGUID = gameObject->GetGUID();
                        if (GetBossState(BOSS_HODIR) == DONE)
                            HandleGameObject(HodirIceDoorGUID, true);
                        break;

                    // Freya related
                    case GO_FREYA_CHEST_HERO:
                    case GO_FREYA_CHEST:
                        FreyaChestGUID = gameObject->GetGUID();
                        break;

                    // Mimiron related
                    case GO_MIMIRON_TRAIN:
                        gameObject->setActive(true);
                        MimironTrainGUID = gameObject->GetGUID();
                        break;
                    case GO_MIMIRON_ELEVATOR:
                        gameObject->setActive(true);
                        MimironElevatorGUID = gameObject->GetGUID();
                        gameObject->SetGoState(GO_STATE_ACTIVE);
                        gameObject->EnableCollision(false);
                        break;
                    case GO_MIMIRON_DOOR_1:
                    case GO_MIMIRON_DOOR_2:
                    case GO_MIMIRON_DOOR_3:
                        gameObject->setActive(true);
                        MimironDoorGUIDList.push_back(gameObject->GetGUID());
                        break;
                   case GO_MIMIRON_CHEST_HM:
                       gameObject->EnableCollision(false);
                       MimironChestHMGUID = gameObject->GetGUID();
                       break;

                    // Vezax related
                    case GO_WAY_TO_YOGG:
                        WayToYoggGUID = gameObject->GetGUID();
                        if (GetBossState(BOSS_FREYA) == DONE &&
                            GetBossState(BOSS_MIMIRON) == DONE &&
                            GetBossState(BOSS_HODIR) == DONE &&
                            GetBossState(BOSS_THORIM) == DONE)
                            gameObject->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED);
                        break;
                    case GO_VEZAX_DOOR:
                        VezaxDoorGUID = gameObject->GetGUID();
                        HandleGameObject(0, false, gameObject);
                        break;

                    // Yogg-Saron related
                    case GO_YOGG_SARON_DOOR:
                        AddDoor(gameObject, true);
                        break;
                    case GO_BRAIN_ROOM_DOOR_1:
                        BrainRoomDoorGUIDs[0] = gameObject->GetGUID();
                        break;
                    case GO_BRAIN_ROOM_DOOR_2:
                        BrainRoomDoorGUIDs[1] = gameObject->GetGUID();
                        break;
                    case GO_BRAIN_ROOM_DOOR_3:
                        BrainRoomDoorGUIDs[2] = gameObject->GetGUID();
                        break;
                    case GO_GIFT_OF_THE_OBSERVER_10:
                    case GO_GIFT_OF_THE_OBSERVER_25:
                        GiftOfTheObserverGUID = gameObject->GetGUID();
                        break;
                }
            }

            void OnGameObjectRemove(GameObject* gameObject)
            {
                switch (gameObject->GetEntry())
                {
                    case GO_LEVIATHAN_DOOR:
                        AddDoor(gameObject, false);
                        break;
                    case GO_XT_002_DOOR:
                        AddDoor(gameObject, false);
                        break;
                    case GO_YOGG_SARON_DOOR:
                        AddDoor(gameObject, false);
                        break;
                    default:
                        break;
                }
            }

            void ProcessEvent(WorldObject* /*gameObject*/, uint32 eventId)
            {
                // Flame Leviathan's Tower Event triggers
                Creature* FlameLeviathan = instance->GetCreature(LeviathanGUID);
                if (FlameLeviathan && FlameLeviathan->isAlive()) // No leviathan, no event triggering ;)
                    switch (eventId)
                    {
                        case EVENT_TOWER_OF_STORM_DESTROYED:
                            FlameLeviathan->AI()->DoAction(ACTION_TOWER_OF_STORM_DESTROYED);
                            break;
                        case EVENT_TOWER_OF_FROST_DESTROYED:
                            FlameLeviathan->AI()->DoAction(ACTION_TOWER_OF_FROST_DESTROYED);
                            break;
                        case EVENT_TOWER_OF_FLAMES_DESTROYED:
                            FlameLeviathan->AI()->DoAction(ACTION_TOWER_OF_FLAMES_DESTROYED);
                            break;
                        case EVENT_TOWER_OF_LIFE_DESTROYED:
                            FlameLeviathan->AI()->DoAction(ACTION_TOWER_OF_LIFE_DESTROYED);
                            break;
                    }
            }


            bool SetBossState(uint32 type, EncounterState state)
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;

                if (UlduarBosses(type) <= BOSS_ALGALON)
                    if (GetBossState(UlduarBosses(type)) != DONE)
                        InstanceScript::SetBossState(UlduarBosses(type), state);

                switch (type)
                {
                    case BOSS_LEVIATHAN:
                        for (std::list<uint64>::iterator i = LeviathanDoorGUIDList.begin(); i != LeviathanDoorGUIDList.end(); ++i)
                        {
                            if (GameObject* gameObject = instance->GetGameObject(*i))
                                gameObject->SetGoState(state == IN_PROGRESS ? GO_STATE_READY : GO_STATE_ACTIVE );
                        }

                        if (state == DONE)
                        {
                            if (GameObject* gameObject = instance->GetGameObject(leviathanChestGUID))
                                gameObject->SetRespawnTime(gameObject->GetRespawnDelay());

                            HandleGameObject(XT002DoorGUID, true);
                        }
                        break;
                    case BOSS_IGNIS:
                    case BOSS_RAZORSCALE:
                    case BOSS_XT002:
                        // Door should closed during these fights :o
                        HandleGameObject(XT002DoorGUID, state != IN_PROGRESS);
                        break;
                    case BOSS_ASSEMBLY_OF_IRON:
                        // Prevent fleeing :o
                        HandleGameObject(IronCouncilEntranceGUID, state != IN_PROGRESS);
                        if (state == DONE)
                            HandleGameObject(ArchivumDoorGUID, true);
                        break;
                        break;
                    case BOSS_AURIAYA:
                        break;
                    case BOSS_FREYA:
                        if (state == DONE)
                            instance->SummonCreature(NPC_FREYA_OBSERVATION_RING, ObservationRingKeepersPos[0]);
                        break;
                    case BOSS_MIMIRON:
                        for (std::list<uint64>::iterator i = MimironDoorGUIDList.begin(); i != MimironDoorGUIDList.end(); ++i)
                        {
                            if (GameObject* gameobject = instance->GetGameObject(*i))
                                gameobject->SetGoState(state == IN_PROGRESS ? GO_STATE_READY : GO_STATE_ACTIVE);
                        }
                        if (state == DONE)
                            instance->SummonCreature(NPC_MIMIRON_OBSERVATION_RING, ObservationRingKeepersPos[3]);
                        break;
                    case BOSS_VEZAX:
                        if (state == DONE)
                            HandleGameObject(VezaxDoorGUID, true);
                        break;
                    case BOSS_YOGG_SARON:
                        break;
                    case BOSS_KOLOGARN:
                        if (state == DONE)
                        {
/*
                            if (GameObject* gameObject = instance->GetGameObject(KologarnChestGUID))
                            {
                                gameObject->SetRespawnTime(gameObject->GetRespawnDelay());
                                gameObject->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                            }
*/
                            HandleGameObject(KologarnBridgeGUID, false);
                        }
                        // Due to HordeGuides, the door should be open when the encounter is in progress. (It opens after finishing AoI.)
                        if (state == IN_PROGRESS)
                            HandleGameObject(KologarnDoorGUID, false);
                        else
                            HandleGameObject(KologarnDoorGUID, true);

                        break;
                    case BOSS_HODIR:
                        if (state == DONE)
                        {
                            HandleGameObject(HodirIceDoorGUID, true);
                            HandleGameObject(HodirStoneDoorGUID, true);
                            if (GameObject* HodirRareCache = instance->GetGameObject(HodirRareCacheGUID))
                                if (GetData(DATA_HODIR_RARE_CACHE) == 1)
                                    HodirRareCache->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);

                            if (GameObject* HodirChest = instance->GetGameObject(HodirChestGUID))
                                HodirChest->SetRespawnTime(HodirChest->GetRespawnDelay());
                            instance->SummonCreature(NPC_HODIR_OBSERVATION_RING, ObservationRingKeepersPos[1]);
                        }
                        HandleGameObject(HodirEntranceDoorGUID, state != IN_PROGRESS);
                        break;
                    case BOSS_THORIM:
                        if (state == DONE)
                        {
                            if (GameObject* gameObject = instance->GetGameObject(ThorimChestGUID))
                                gameObject->SetRespawnTime(gameObject->GetRespawnDelay());
                            if (GameObject* gameObject = instance->GetGameObject(ThorimLightningFieldGUID))
                                gameObject->SetGoState(state == IN_PROGRESS ? GO_STATE_READY : GO_STATE_ACTIVE);
                            instance->SummonCreature(NPC_THORIM_OBSERVATION_RING, ObservationRingKeepersPos[2]);
                        }
                        break;
                    case BOSS_ALGALON:
                    {
                        if (state == DONE)
                        {
                            _events.CancelEvent(EVENT_UPDATE_ALGALON_TIMER);
                            _events.CancelEvent(EVENT_DESPAWN_ALGALON);
                            DoUpdateWorldState(WORLD_STATE_ALGALON_TIMER_ENABLED, 0);
                            _algalonTimer = 61;
                            if (GameObject* gameObject = instance->GetGameObject(GiftOfTheObserverGUID))
                                gameObject->SetRespawnTime(gameObject->GetRespawnDelay());
                            // get item level (recheck weapons)
                            Map::PlayerList const& players = instance->GetPlayers();
                            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                                if (Player* player = itr->getSource())
                                    for (uint8 slot = EQUIPMENT_SLOT_MAINHAND; slot <= EQUIPMENT_SLOT_RANGED; ++slot)
                                        if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                                            if (item->GetTemplate()->ItemLevel > _maxWeaponItemLevel)
                                                _maxWeaponItemLevel = item->GetTemplate()->ItemLevel;

                        }
                        if (state != IN_PROGRESS)
                        {
                            //                            HandleGameObject(AlgalonTrapdoorGUID, true);
                            HandleGameObject(AlgalonSigilDoorGUID[0], true);
                            HandleGameObject(AlgalonSigilDoorGUID[1], true);
                            HandleGameObject(AlgalonSigilDoorGUID[2], true);
                            HandleGameObject(AlgalonUniverseGUID, false);
                        }
                        if (state == IN_PROGRESS)
                        {
                            // get item level (armor cannot be swapped in combat)
                            Map::PlayerList const& players = instance->GetPlayers();
                            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                            {
                                if (Player* player = itr->getSource())
                                {
                                    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
                                    {
                                        if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
                                            continue;

                                        if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                                        {
                                            if (slot >= EQUIPMENT_SLOT_MAINHAND && slot <= EQUIPMENT_SLOT_RANGED)
                                            {
                                                if (item->GetTemplate()->ItemLevel > _maxWeaponItemLevel)
                                                    _maxWeaponItemLevel = item->GetTemplate()->ItemLevel;
                                            }
                                            else if (item->GetTemplate()->ItemLevel > _maxArmorItemLevel)
                                                _maxArmorItemLevel = item->GetTemplate()->ItemLevel;
                                        }
                                    }
                                }
                            }
                            HandleGameObject(AlgalonSigilDoorGUID[0], false);
                            HandleGameObject(AlgalonSigilDoorGUID[1], false);
                            HandleGameObject(AlgalonSigilDoorGUID[2], false);
                            //                            HandleGameObject(AlgalonTrapdoorGUID, false);
                            HandleGameObject(AlgalonUniverseGUID, true);
                        }
                        break;
                    }
                    default:
                        break;
                }
                if (GetBossState(BOSS_FREYA) == DONE &&
                    GetBossState(BOSS_MIMIRON) == DONE &&
                    GetBossState(BOSS_HODIR) == DONE &&
                    GetBossState(BOSS_THORIM) == DONE)
                    if (WayToYoggGUID)
                        HandleGameObject(WayToYoggGUID, true);
                        //go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED);
                if (GetBossState(BOSS_FREYA) == DONE)
                    _summonObservationRingKeeper[0] = true;
                if (GetBossState(BOSS_HODIR) == DONE)
                    _summonObservationRingKeeper[1] = true;
                if (GetBossState(BOSS_THORIM) == DONE)
                    _summonObservationRingKeeper[2] = true;
                if (GetBossState(BOSS_MIMIRON) == DONE)
                    _summonObservationRingKeeper[3] = true;

                return true;
            }

            EncounterState GetBossState(uint32 id)
            {
                if (UlduarBosses(id) <= BOSS_ALGALON)
                    return InstanceScript::GetBossState(UlduarBosses(id));
                return NOT_STARTED;
            }

            void SetData(uint32 type, uint32 data)
            {
                switch (type)
                {
                    case DATA_ALGALON_INTRO:
                        AlgalonIntroDone = data;
                        SaveToDB();
                        break;
                    case DATA_ALGALON_TIMER:
                    {
                        if (SignalTimerState == data)
                            break;
                        switch (data)
                        {
                            case IN_PROGRESS:
                                SignalTimer = 60000;
                                SignalTimerMinutes = 60;
                                DoUpdateWorldState(WORLDSTATE_SHOW_TIMER, 1);
                                DoUpdateWorldState(WORLDSTATE_ALGALON_TIMER , SignalTimerMinutes);
                                break;
                            case DONE:
                                SignalTimer = 0;
                                SignalTimerMinutes = 0;
                                DoUpdateWorldState(WORLDSTATE_SHOW_TIMER, 0);
                                break;
                            default:
                                break;
                        }
                        SignalTimerState = data;
                        SaveToDB();
                        break;
                    }
                    case DATA_ARM_HF:
                        armhf = data;
                        break;
                    case DATA_CAILLE :
                        DataCaille = data;
                        break;
                    case DATA_GARE_GEL :
                        DataGare = data;
                        break;
                    case DATA_COLOSSUS:
                        ColossusData = data;
                        if (data == 2)
                        {
                            if (Creature* Leviathan = instance->GetCreature(LeviathanGUID))
                                Leviathan->AI()->DoAction(ACTION_MOVE_TO_CENTER_POSITION);
                            if (GameObject* gameObject = instance->GetGameObject(LeviathanGateGUID))
                                gameObject->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);
                            SaveToDB();
                        }
                        break;
                    case DATA_CALL_TRAM:
                        if (GameObject* go = instance->GetGameObject(MimironTrainGUID))
                            go->SetGoState(GOState(data));
                        break;
                    case DATA_MIMIRON_ELEVATOR:
                        if (GameObject* go = instance->GetGameObject(MimironElevatorGUID))
                            go->SetGoState(GOState(data));
                        break;
                    case DATA_ILLUSION:
                        illusion = data;
                        break;
                    case DATA_RUNIC_DOOR:
                        if (GameObject* go = instance->GetGameObject(RunicDoorGUID))
                            go->SetGoState(GOState(data));
                        break;
                    case DATA_STONE_DOOR:
                        if (GameObject* go = instance->GetGameObject(StoneDoorGUID))
                            go->SetGoState(GOState(data));
                        break;
                    case DATA_ADD_HELP_FLAG:
                        SupportKeeperFlag |= UlduarKeeperSupport(data);
                        break;
                    case DATA_HODIR_RARE_CACHE:
                        HodirRareCacheData = data;
                        if (!HodirRareCacheData)
                        {
                            if (Creature* Hodir = instance->GetCreature(HodirGUID))
                                if (GameObject* gameObject = instance->GetGameObject(HodirRareCacheGUID))
                                    Hodir->RemoveGameObject(gameObject, false);
                        }
                        break;
                    case DATA_UNBROKEN:
                        if (Creature* Leviathan = instance->GetCreature(LeviathanGUID))
                            Leviathan->AI()->SetData(type, data);
                        break;
                    case EVENT_DESPAWN_ALGALON:
                        DoUpdateWorldState(WORLD_STATE_ALGALON_TIMER_ENABLED, 1);
                        DoUpdateWorldState(WORLD_STATE_ALGALON_DESPAWN_TIMER, 60);
                        _algalonTimer = 60;
                        _events.ScheduleEvent(EVENT_DESPAWN_ALGALON, 3600000);
                        _events.ScheduleEvent(EVENT_UPDATE_ALGALON_TIMER, 60000);
                        break;
                    case DATA_ALGALON_SUMMON_STATE:
                        _algalonSummoned = true;
                        break;
                    default:
                        break;
                }
            }

            uint64 GetData64(uint32 data) const
            {
                switch (data)
                {
                    case NPC_BRANN_EVENT_START_ULDU :   return uiBrannGUID;
                    case BOSS_ALGALON:              return uiAlgalonGUID;
                    case NPC_BRANN_BRONZBEARD_ALG:        return AlgalonBrannGUID;
                    case BOSS_IGNIS:                return IgnisGUID;
                    case BOSS_KOLOGARN:             return KologarnGUID;
                    case BOSS_AURIAYA:              return AuriayaGUID;
                    case BOSS_HODIR:                return HodirGUID;
                    case BOSS_THORIM:               return ThorimGUID;
                    case BOSS_FREYA:                return FreyaGUID;
                    case BOSS_VEZAX:                return VezaxGUID;

                    // Leviathan
                    case BOSS_LEVIATHAN:            return LeviathanGUID;
                    case DATA_RUNIC_COLOSSUS:       return RunicColossusGUID;
                    case DATA_RUNE_GIANT:           return RuneGiantGUID;

                    // Razorscale
                    case BOSS_RAZORSCALE:           return RazorscaleGUID;
                    case DATA_RAZORSCALE_CONTROL:   return RazorscaleController;
                    case DATA_EXPEDITION_COMMANDER: return ExpeditionCommanderGUID;
                    case GO_RAZOR_HARPOON_1:        return RazorHarpoonGUIDs[0];
                    case GO_RAZOR_HARPOON_2:        return RazorHarpoonGUIDs[1];
                    case GO_RAZOR_HARPOON_3:        return RazorHarpoonGUIDs[2];
                    case GO_RAZOR_HARPOON_4:        return RazorHarpoonGUIDs[3];

                    // XT002
                    case BOSS_XT002:                return XT002GUID;
                    case DATA_TOY_PILE_0:
                    case DATA_TOY_PILE_1:
                    case DATA_TOY_PILE_2:
                    case DATA_TOY_PILE_3:           return XTToyPileGUIDs[data - DATA_TOY_PILE_0];

                    // Assembly of Iron
                    case BOSS_STEELBREAKER:         return AssemblyGUIDs[0];
                    case BOSS_MOLGEIM:              return AssemblyGUIDs[1];
                    case BOSS_BRUNDIR:              return AssemblyGUIDs[2];

                    // Freya's Keepers
                    case BOSS_BRIGHTLEAF:           return KeeperGUIDs[0];
                    case BOSS_IRONBRANCH:           return KeeperGUIDs[1];
                    case BOSS_STONEBARK:            return KeeperGUIDs[2];

                    // Mimiron
                    case BOSS_MIMIRON:              return MimironGUID;
                    case DATA_LEVIATHAN_MK_II:      return LeviathanMKIIGUID;
                    case DATA_VX_001:               return VX001GUID;
                    case DATA_AERIAL_UNIT:          return AerialUnitGUID;

                    // Yogg-Saron
                    case BOSS_YOGG_SARON:
                        return YoggSaronGUID;
                    case DATA_VOICE_OF_YOGG_SARON:
                        return VoiceOfYoggSaronGUID;
                    case DATA_BRAIN_OF_YOGG_SARON:
                        return BrainOfYoggSaronGUID;
                    case DATA_SARA:
                        return SaraGUID;
                    case GO_BRAIN_ROOM_DOOR_1:
                        return BrainRoomDoorGUIDs[0];
                    case GO_BRAIN_ROOM_DOOR_2:
                        return BrainRoomDoorGUIDs[1];
                    case GO_BRAIN_ROOM_DOOR_3:
                        return BrainRoomDoorGUIDs[2];
                    case DATA_FREYA_YS:
                        return YKeeperGUIDs[0];
                    case DATA_HODIR_YS:
                        return YKeeperGUIDs[1];
                    case DATA_THORIM_YS:
                        return YKeeperGUIDs[2];
                    case DATA_MIMIRON_YS:
                        return YKeeperGUIDs[3];

                        // Thorim
                    case GO_THORIM_DARK_IRON_PROTCULLIS:
                        return ThorimDarkIronPortCullisGUID;
                    case GO_THORIM_CHEST_HERO:
                    case GO_THORIM_CHEST:
                        return ThorimChestGUID;
                    case GO_THORIM_LIGHTNING_FIELD:
                        return ThorimLightningFieldGUID;
                    case GO_THORIM_STONE_DOOR:
                        return StoneDoorGUID;
                    case GO_THORIM_RUNIC_DOOR:
                        return RunicDoorGUID;
                    case NPC_THORIM:
                        return ThorimGUID;
                    case NPC_THORIM_CTRL:
                        return ThorimCtrlGUID;
                    case DATA_HODIR_RARE_CACHE:
                        return HodirRareCacheGUID;

                case DATA_SIGILDOOR_01:
                    return AlgalonSigilDoorGUID[0];
                case DATA_SIGILDOOR_02:
                    return AlgalonSigilDoorGUID[1];
                case DATA_SIGILDOOR_03:
                    return AlgalonSigilDoorGUID[2];
                case DATA_UNIVERSE_FLOOR_01:
                    return AlgalonFloorGUID[0];
                case DATA_UNIVERSE_FLOOR_02:
                    return AlgalonFloorGUID[1];
                case DATA_UNIVERSE_GLOBE:
                    return AlgalonUniverseGUID;
                case DATA_ALGALON_TRAPDOOR:
                    return AlgalonTrapdoorGUID;
                }

                return 0;
            }

            uint32 GetData(uint32 type) const
            {
                switch (type)
                {
                    case DATA_HEROLD:
                        return uint32(_maxArmorItemLevel <= MAX_HERALD_ARMOR_ITEMLEVEL && _maxWeaponItemLevel <= MAX_HERALD_WEAPON_ITEMLEVEL);
                    case DATA_ALGALON_INTRO:
                        return AlgalonIntroDone;
                    case DATA_ALGALON_TIMER:
                        return SignalTimerState;
                    case DATA_ARM_HF :
                        return armhf;
                    case DATA_GARE_GEL :
                        return DataGare;
                    case DATA_CAILLE :
                        return DataCaille;
                    case DATA_COLOSSUS:
                        return ColossusData;
                    case DATA_KEEPER_SUPPORT_YOGG:
                        return SupportKeeperFlag;
                    case DATA_HODIR_RARE_CACHE:
                        return HodirRareCacheData;
                    case DATA_UNBROKEN:
                        if (Creature* Leviathan = instance->GetCreature(LeviathanGUID))
                            return Leviathan->AI()->GetData(type);
                        break;
                    case DATA_ILLUSION:
                        return illusion;
                    case DATA_KEEPERS_COUNT:
                        return keepersCount;
                    default:
                        break;
                }
                return 0;
            }

            void FillInitialWorldStates(WorldPacket& packet)
            {
                packet << uint32(WORLD_STATE_ALGALON_TIMER_ENABLED) << uint32(_algalonTimer && _algalonTimer <= 60);
                packet << uint32(WORLD_STATE_ALGALON_DESPAWN_TIMER) << uint32(std::min<uint32>(_algalonTimer, 60));
            }

            void WriteSaveDataMore(std::ostringstream& data) override
            {
                data << GetData(DATA_COLOSSUS) << ' ' << _algalonTimer << ' ' << uint32(_algalonSummoned ? 1 : 0) << ' ' << PlayerDeathFlag;
            }

            void ReadSaveDataMore(std::istringstream& data) override
            {
                uint32 tempState;
                data >> tempState;
                if (tempState == IN_PROGRESS || tempState > SPECIAL)
                    tempState = NOT_STARTED;
                SetData(DATA_COLOSSUS, tempState);

                data >> _algalonTimer;
                data >> tempState;
                _algalonSummoned = tempState != 0;
                if (_algalonSummoned && GetBossState(BOSS_ALGALON) != DONE)
                {
                    _summonAlgalon = true;
                    if (_algalonTimer && _algalonTimer <= 60)
                    {
                        _events.ScheduleEvent(EVENT_UPDATE_ALGALON_TIMER, 60000);
                        DoUpdateWorldState(WORLD_STATE_ALGALON_TIMER_ENABLED, 1);
                        DoUpdateWorldState(WORLD_STATE_ALGALON_DESPAWN_TIMER, _algalonTimer);
                    }
                }
                data >> tempState;
                PlayerDeathFlag = tempState;
            }
        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_ulduar_InstanceMapScript(map);
        }
};

// TODO: Register script-name at database (not done yet 2012/06/24 14:50)
class go_call_tram : public GameObjectScript
{
public:
    go_call_tram() : GameObjectScript("go_call_tram") { }

    bool OnGossipHello(Player* /*pPlayer*/, GameObject* pGo)
    {
        InstanceScript* pInstance = pGo->GetInstanceScript();

        if (!pInstance)
            return false;

        switch (pGo->GetEntry())
        {
            // Activate
            case 194437:    // [ok]
            case 194912:
                pInstance->SetData(DATA_CALL_TRAM, 0);
                break;
            // Call
            case 194438:
            case 194914:    // [ok]
                pInstance->SetData(DATA_CALL_TRAM, 1);
                break;
//             case 194914:    // At sanctuary
//             case 194437:
//                 pInstance->SetData(DATA_CALL_TRAM, 0);
//                 break;
//             case 194912:    // At Mimiron
//             case 194438:
//                 pInstance->SetData(DATA_CALL_TRAM, 1);
//                 break;
        }
        return true;
    }
};

void AddSC_instance_ulduar()
{
    new instance_ulduar();
    new go_call_tram();
}
