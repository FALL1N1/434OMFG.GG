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

/*
 * Comment: there is missing code on triggers,
 *          brann bronzebeard needs correct gossip info.
 *          requires more work involving area triggers.
 *          if reached brann speaks through his radio..
 */


/* Improve By Gabii - Paragon Server - Last Update : 23/09/2012 */


/* ---    TODO List :        ---
* Fix Emplacement des Passagers 0 (player), 1(Tourrelle), 2 (device). Visuellement + veritable position.
* Fix Spell Du bouclier du vehicule ( absorbe constamment...)
*/

#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"
#include "CombatAI.h"
#include "PassiveAI.h"
#include "ObjectMgr.h"
#include "SpellScript.h"
#include "Vehicle.h"
#include "VehicleDefines.h"
#include "ulduar.h"

enum Spells
{
    SPELL_PURSUED                  = 62374,
    SPELL_GATHERING_SPEED          = 62375,
    SPELL_BATTERING_RAM            = 62376,
    SPELL_FLAME_VENTS              = 62396,
    SPELL_MISSILE_BARRAGE          = 62400,
    SPELL_SYSTEMS_SHUTDOWN         = 62475,
    SPELL_OVERLOAD_CIRCUIT         = 62399,
    SPELL_START_THE_ENGINE         = 62472,
    SPELL_SEARING_FLAME            = 62402,
    SPELL_BLAZE                    = 62292,
    SPELL_TAR_PASSIVE              = 62288,
    SPELL_SMOKE_TRAIL              = 63575,
    SPELL_SMOKE_TRAIL_VISUAL       = 61364,
    SPELL_ELECTROSHOCK             = 62522,
    SPELL_NAPALM                   = 63666,
    SPELL_INVIS_AND_STEALTH_DETECT = 18950, // Passive
    //TOWER Additional SPELLS
    SPELL_THORIMS_HAMMER           = 62912, // Tower of Storms // 62911 should trigger it, but it doesn't in the proper place
    SPELL_MIMIRONS_INFERNO         = 62909, // Tower of Flames
    SPELL_MIMIRONS_INFERNO_DAMAGE  = 62910,
    SPELL_HODIRS_FURY              = 62533, // Tower of Frost
    SPELL_HODIRS_FURY_DAMAGE       = 62297,
    SPELL_FREYAS_WARD              = 62907, // Tower of Nature // 62906 should trigger it, but it doesn't in the proper place
    SPELL_FREYA_SUMMONS            = 62947, // Tower of Nature
    //TOWER ap & health spells
    SPELL_BUFF_TOWER_OF_STORMS     = 65076,
    SPELL_BUFF_TOWER_OF_FLAMES     = 65075,
    SPELL_BUFF_TOWER_OF_FR0ST      = 65077,
    SPELL_BUFF_TOWER_OF_LIFE       = 64482,
    // Additional Spells
    SPELL_FORCE_REACTION           = 57605, // temporary

    SPELL_LASH                     = 65062,
    SPELL_FREYA_S_WARD_EFFECT_1    = 62947,
    SPELL_FREYA_S_WARD_EFFECT_2    = 62907,
    SPELL_AUTO_REPAIR              = 62705,
    SPELL_BLUE_SKYBEAM             = 63769,

    SPELL_GREEN_SKYBEAM            = 63771,
    SPELL_RED_SKYBEAM              = 63772,
    SPELL_LIGHTNING_SKYBEAM        = 63773,

    AURA_DUMMY_BLUE                = 63294,
    AURA_DUMMY_GREEN               = 63295,
    AURA_DUMMY_YELLOW              = 63292,
    AURA_STEALTH_DETECTION         = 18950,

    SPELL_LIQUID_PYRITE            = 62494,
    SPELL_DUSTY_EXPLOSION          = 63360,
    SPELL_DUST_CLOUD_IMPACT        = 54740,
    SPELL_RIDE_VEHICLE             = 46598,
    SPELL_GEAR_SCALING             = 65266,
    SPELL_ANTI_AIR_ROCKET_DMG      = 62363,
    SPELL_GROUND_SLAM              = 62625
};

enum Creatures
{
    NPC_SEAT                       = 33114,
    NPC_DEFENSE_TURRET             = 33142, // bunny
    NPC_OVERLOAD_DEVICE            = 33143, // bunny
    NPC_DEFENSE_CANNON             = 33139, // bunny
    NPC_MECHANOLIFT                = 33214,
    NPC_LIQUID                     = 33189,
    NPC_CONTAINER                  = 33218,
    NPC_THORIM_BEACON              = 33365,
    NPC_MIMIRON_BEACON             = 33370,
    NPC_HODIR_BEACON               = 33212,
    NPC_FREYA_BEACON               = 33367,
    NPC_THORIM_TARGET_BEACON       = 33364,
    NPC_MIMIRON_TARGET_BEACON      = 33369,
    NPC_HODIR_TARGET_BEACON        = 33108,
    NPC_FREYA_TARGET_BEACON        = 33366,
    NPC_WRITHING_LASHER            = 33387, // bunny
    NPC_WARD_OF_LIFE               = 34275, // bunny
    NPC_LOREKEEPER                 = 33686, // Hard mode starter
    NPC_BRANN_BRONZBEARD           = 33579,
    NPC_DELORAH                    = 33701,
    NPC_ULDUAR_GAUNTLET_GENERATOR  = 33571, // Trigger tied to towers


    //Outro

    VOLANTE_BRANN_OUTRO = 34120,
    NPC_BRANN_OUTRO = 34119,
    NPC_RHYDIAN_OURTO = 33696,
};

enum Towers
{
    GO_TOWER_OF_STORMS    = 194377,
    GO_TOWER_OF_FLAMES    = 194371,
    GO_TOWER_OF_FROST     = 194370,
    GO_TOWER_OF_LIFE      = 194375,
};

enum Events
{
    EVENT_PURSUE               = 1,
    EVENT_MISSILE              = 2,
    EVENT_VENT                 = 3,
    EVENT_SPEED                = 4,
    EVENT_SUMMON               = 5,
    EVENT_SHUTDOWN             = 6,
    EVENT_REPAIR               = 7,
    EVENT_THORIMS_HAMMER       = 8,    // Tower of Storms
    EVENT_MIMIRONS_INFERNO     = 9,    // Tower of Flames
    EVENT_HODIRS_FURY          = 10,   // Tower of Frost
    EVENT_FREYAS_WARD          = 11,   // Tower of Nature
};

enum Seats
{
    SEAT_PLAYER    = 0,
    SEAT_TURRET    = 1,
    SEAT_DEVICE    = 2,
    SEAT_CANNON    = 7,
};

enum SeatCounts
{
    TWO_SEATS  = 2, // 10p
    FOUR_SEATS = 4  // 25p
};

enum Vehicles
{
    VEHICLE_SIEGE         = 33060,
    VEHICLE_CHOPPER       = 33062,
    VEHICLE_DEMOLISHER    = 33109,
};

#define EMOTE_PURSUE      "Le Leviathan des Flammes poursuit $N."
#define EMOTE_OVERLOAD    "Surchage des circuits du Leviathan des Flammes."
#define EMOTE_REPAIR      "Initialisation de la sequence de reparation automatique."
#define _DATA_SHUTOUT      29112912 // 2911, 2912 are achievement IDs   // TODO: Maybe get rid of those two defines
#define _DATA_ORBIT_ACHIEVEMENTS    1

enum Yells
{
    SAY_AGGRO            = 0,
    SAY_SLAY             = 1,
    SAY_DEATH            = 2,
    SAY_TARGET           = 3,
    SAY_HARDMODE         = 4,
    SAY_TOWER_NONE       = 5,
    SAY_TOWER_FROST      = 6,
    SAY_TOWER_FLAME      = 7,

    SAY_TOWER_NATURE     = 0,
    SAY_TOWER_STORM      = 1,
    SAY_PLAYER_RIDING    = 2,
    SAY_OVERLOAD         = 3,
};

enum AchievementData
{
    DATA_SHUTOUT            = 1,
    DATA_ORBIT_ACHIEVEMENTS = 3 // Hardmode only
};

enum Actions
{
    ACTION_ACTIVATE_HARD_MODE        = 5,
    ACTION_SPAWN_VEHICLES            = 6,
    ACTION_START_ENCOUNTER           = 10,
    ACTION_OVERLOAD_CIRCUIT          = 11,

    /* Also used, are settled in ulduar.h
    ACTION_TOWER_OF_STORM_DESTROYED  = 1,
    ACTION_TOWER_OF_FROST_DESTROYED  = 2,
    ACTION_TOWER_OF_FLAMES_DESTROYED = 3,
    ACTION_TOWER_OF_LIFE_DESTROYED   = 4,
    */

    //OUTRO LEVIATHAN
    ACTION_START_OUTRO,
};

#define TYPE_ULDUAR_EVENT 3132

Position const Misc[]=
{
    {266.689f, -33.391f, 409.99f, 0.0f},    // Thorim Beacon bunnys
    {379.158f, -33.318f, 409.81f, 0.0f},
    {266.611f, -133.387f, 409.81f, 0.0f},
    {158.411f, -33.311f, 409.81f, 0.0f},
    {266.699f, 66.632f, 409.81f, 0.0f}
};

Position const Center =
{
    354.8771f, -12.90240f, 409.803650f, 0.0f
};

Position const InfernoStart =
{
    390.93f, -13.91f, 409.81f, 0.0f
};

Position const PosSiege[] =
{
    {-814.59f, -64.54f, 429.92f, 5.969f},
    {-784.37f, -33.31f, 429.92f, 5.096f},
    {-808.99f, -52.10f, 429.92f, 5.668f},
    {-798.59f, -44.00f, 429.92f, 5.663f},
    {-812.83f, -77.71f, 429.92f, 0.046f},
};

Position const PosChopper[] =
{
    {-717.83f, -106.56f, 430.02f, 0.122f},
    {-717.83f, -114.23f, 430.44f, 0.122f},
    {-717.83f, -109.70f, 430.22f, 0.122f},
    {-718.45f, -118.24f, 430.26f, 0.052f},
    {-718.45f, -123.58f, 430.41f, 0.085f},
};

Position const PosDemolisher[] =
{
    {-724.12f, -176.64f, 430.03f, 2.543f},
    {-766.70f, -225.03f, 430.50f, 1.710f},
    {-729.54f, -186.26f, 430.12f, 1.902f},
    {-756.01f, -219.23f, 430.50f, 2.369f},
    {-798.01f, -227.24f, 429.84f, 1.446f},
};

Position const FreyaBeacons[] =
{
    {377.02f, -119.10f, 409.81f, 0.0f},
    {185.62f, -119.10f, 409.81f, 0.0f},
    {377.02f, 54.78f, 409.81f, 0.0f},
    {185.62f, 54.78f, 409.81f, 0.0f},
};

class boss_flame_leviathan : public CreatureScript
{
    public:
        boss_flame_leviathan() : CreatureScript("boss_flame_leviathan") {}

        struct boss_flame_leviathanAI : public BossAI
        {
            boss_flame_leviathanAI(Creature* creature) : BossAI(creature, BOSS_LEVIATHAN), vehicle(creature->GetVehicleKit()) {}

        public:
            void InitializeAI()
            {
                ASSERT(vehicle);
                if (!me->isDead())
                    Reset();
                Shutdown = pursueTarget = SpawnFreyaAdd = SpeechFreya = SpeechThorims = SpeechHodirs = TestCrash = 0;
                // Note: I'll consider those as active on start, since this is causally correct.

                towerOfStorms = towerOfLife = towerOfFlames = towerOfFrost = true;  // Towers
                Shutout = Unbroken = true;                                          // Achievs
                checkUnbrokenOnReset = false;
                DoCast(SPELL_INVIS_AND_STEALTH_DETECT);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_STUNNED);
            }

            void Reset()
            {
                _Reset();
                Pursued = false;
                Shutdown = pursueTarget = SpawnFreyaAdd = SpeechFreya = SpeechThorims = SpeechHodirs = TestCrash =0;
                HandleAccessorys(false);
                me->SetReactState(REACT_DEFENSIVE);
                if (checkUnbrokenOnReset) // A fight was already performed, the raid got wiped before starting this Reset() call -> Unbroken can only be done on first try!
                    SetData(DATA_UNBROKEN, 0);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PURSUED);
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                me->SetReactState(REACT_PASSIVE);   // Enforce react-type, unless PURSUE gehts active.
                events.ScheduleEvent(EVENT_PURSUE, 30*IN_MILLISECONDS);
                events.ScheduleEvent(EVENT_MISSILE, 2*IN_MILLISECONDS);
                events.ScheduleEvent(EVENT_VENT, 20*IN_MILLISECONDS);
                events.ScheduleEvent(EVENT_SPEED, 15*IN_MILLISECONDS);
                events.ScheduleEvent(EVENT_SUMMON, 1*IN_MILLISECONDS);
                PerformTowerCheck();
                HandleAccessorys(true);
                checkUnbrokenOnReset = true;
                Talk(SAY_AGGRO);
            }

            void JustReachedHome()
            {
                if (Creature* Brann =  ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_BRANN_EVENT_START_ULDU)))
                    Brann->AI()->SetData(TYPE_ULDUAR_EVENT, 5);
            }

            void Outro()
            {
                if (Creature* BrannIntro =  ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_BRANN_EVENT_START_ULDU)))
                    BrannIntro->AI()->SetData(TYPE_ULDUAR_EVENT, 6);
                if (Creature* MachineBrann = me->SummonCreature(VOLANTE_BRANN_OUTRO, 140.0f, -37.0f, 460.0f, 6.2f))
                {
                    MachineBrann->SetCanFly(true);
                    MachineBrann->SetSpeed(MOVE_RUN , 2.0f);
                    MachineBrann->SetSpeed(MOVE_WALK , 2.0f);
                    MachineBrann->SetSpeed(MOVE_FLIGHT , 2.0f);
                    if (Creature* BrannOutro = me->SummonCreature(NPC_BRANN_OUTRO, 140.0f, -37.0f, 460.0f, 6.2f))
                    {
                        float x, y, z;
                        me->GetPosition(x, y, z);
                        BrannOutro->EnterVehicle(MachineBrann);
                        //        z = me->GetMap()->GetHeight(me->GetPhaseMask(), x, y, z);
                        MachineBrann->GetMotionMaster()->MovePoint(ACTION_START_OUTRO, x - 10, y - 10, z + 1);
                    }
                }
            }

            bool HaveActiveTowers() const
            {
                return (towerOfFlames || towerOfFrost || towerOfLife || towerOfStorms);
            }

            uint8 GetActiveTowersCount() const
            {
                uint8 count = 0;
                if (towerOfFlames)  ++count;
                if (towerOfFrost)   ++count;
                if (towerOfLife)    ++count;
                if (towerOfStorms)  ++count;
                return count;
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
            }

            void HandleAccessorys(bool doInstall)
            {
                if (doInstall)
                {
                    for (uint8 i = RAID_MODE<uint8>(2, 0); i < 4; ++i)
                    {
                        if (Creature* target = me->SummonCreature(NPC_SEAT, *me))
                        {
                            target->EnterVehicle(me, i);
                            me->GetVehicleKit()->RelocatePassengers();
                            //printf("\n Position SIEGE %d : PositionX = %.2f PositionY = %.2f PositionZ = %.2f",i,target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
                            //target->SendMovementFlagUpdate();
                            if (Creature* turret = target->SummonCreature(NPC_DEFENSE_TURRET, *target))
                            {
                                turret->EnterVehicle(target, SEAT_TURRET);
                                //                                turret->SendMovementFlagUpdate();
                                //printf("\n Position TOURRETE %d : PositionX = %.2f PositionY = %.2f PositionZ = %.2f",i,turret->GetPositionX(), turret->GetPositionY(), turret->GetPositionZ());
                            }

                            if (Creature* device = target->SummonCreature(NPC_OVERLOAD_DEVICE, *target))
                            {
                                device->EnterVehicle(target, SEAT_DEVICE);
                                //                                device->SendMovementFlagUpdate();
                                //printf("\n Position DEVICE %d : PositionX = %.2f PositionY = %.2f PositionZ = %.2f",i,device->GetPositionX(), device->GetPositionY(), device->GetPositionZ());
                            }
                        }
                    }

                    if (Creature* cannon = me->SummonCreature(NPC_DEFENSE_CANNON, *me))
                        cannon->EnterVehicle(me, SEAT_CANNON);
                }
                else
                {
                    me->DespawnCreaturesInArea(NPC_DEFENSE_CANNON, 200.0f);
                    me->DespawnCreaturesInArea(NPC_OVERLOAD_DEVICE, 200.0f);
                    me->DespawnCreaturesInArea(NPC_DEFENSE_TURRET, 200.0f);
                    me->DespawnCreaturesInArea(NPC_SEAT, 200.0f);
                }
            }

            // Here: used for placing players
            void SetGUID(uint64 guid, int32 /*id*/ = 0)
            {
                if (!me->isInCombat())
                    return;

                if (Player* passenger = ObjectAccessor::GetPlayer(*me, guid))
                    for (uint8 i = RAID_MODE(2, 0); i < 4; ++i)
                        if (Unit* seat = vehicle->GetPassenger(i))
                            if (seat->GetVehicleKit()->HasEmptySeat(SEAT_PLAYER) && !seat->GetVehicleKit()->HasEmptySeat(SEAT_TURRET))
                            {
                                passenger->EnterVehicle(seat, SEAT_PLAYER);
                                //                                passenger->ClearUnitState(UNIT_STATE_ONVEHICLE);
                                return;
                            }
            }

            void PerformTowerCheck()
            {
                if (HaveActiveTowers())
                {
                    if (towerOfStorms)
                    {
                        if (!me->HasAura(SPELL_BUFF_TOWER_OF_STORMS))
                            me->CastSpell(me, SPELL_BUFF_TOWER_OF_STORMS, true);

                        events.ScheduleEvent(EVENT_THORIMS_HAMMER, 35*IN_MILLISECONDS);
                    }

                    if (towerOfFlames)
                    {
                        if (!me->HasAura(SPELL_BUFF_TOWER_OF_FLAMES))
                            me->CastSpell(me, SPELL_BUFF_TOWER_OF_FLAMES, true);

                        events.ScheduleEvent(EVENT_MIMIRONS_INFERNO, 70*IN_MILLISECONDS);
                    }

                    if (towerOfFrost)
                    {
                        if (!me->HasAura(SPELL_BUFF_TOWER_OF_FR0ST))
                            me->CastSpell(me, SPELL_BUFF_TOWER_OF_FR0ST, true);

                        events.ScheduleEvent(EVENT_HODIRS_FURY, 105*IN_MILLISECONDS);
                    }

                    if (towerOfLife)
                    {
                        if (!me->HasAura(SPELL_BUFF_TOWER_OF_LIFE))
                            me->CastSpell(me, SPELL_BUFF_TOWER_OF_LIFE, true);

                        events.ScheduleEvent(EVENT_FREYAS_WARD, 140*IN_MILLISECONDS);
                    }

                    Talk(SAY_HARDMODE);
                }
                else
                    Talk(SAY_TOWER_NONE);
            }

            void JustDied(Unit* /*victim*/)
            {
                _JustDied();
                // Set Field Flags 67108928 = 64 | 67108864 = UNIT_FLAG_UNK_6 | UNIT_FLAG_SKINNABLE
                // Set DynFlags 12
                // Set NPCFlags 0
                Talk(SAY_DEATH);
                // TODO: These chests should be somewhere around...
                // Check if changing the loot-mode as shown below works as considered
                if (GameObject* go = me->FindNearestGameObject(RAID_MODE(GO_LEVIATHAN_CHEST_10, GO_LEVIATHAN_CHEST_25), 250.0f))
                    if (HaveActiveTowers())
                        switch (GetActiveTowersCount())
                        {
                            case 4: go->AddLootMode(LOOT_MODE_HARD_MODE_4);
                            case 3: go->AddLootMode(LOOT_MODE_HARD_MODE_3);
                            case 2: go->AddLootMode(LOOT_MODE_HARD_MODE_2);
                            case 1: go->AddLootMode(LOOT_MODE_HARD_MODE_1);
                        }
                Outro();
            }

            void SpellHit(Unit* /*caster*/, SpellInfo const* spell)
            {
                if (!spell)
                    return;

                switch (spell->Id)
                {
                    case SPELL_OVERLOAD_CIRCUIT:
                        ++Shutdown;
                        break;
                }
            }

            uint32 GetData(uint32 type) const
            {
                switch (type)
                {
                    case DATA_SHUTOUT:
                        return static_cast<uint32>(Shutout);
                    case DATA_UNBROKEN:
                        return static_cast<uint32>(Unbroken);
                    case DATA_ORBIT_ACHIEVEMENTS:
                        return GetActiveTowersCount();
                    default:
                        break;
                }
                return 0;
            }

            void SetData(uint32 id, uint32 data)
            {
                if (id == DATA_UNBROKEN)
                    Unbroken = static_cast<bool>(data);
            }

            void RestartWhenCrash()
            {
                int8 NbrColossusMort = 0;
                std::list<Creature*> NpcColossus;
                GetCreatureListWithEntryInGrid(NpcColossus, me, 33237, 200.0f);

                if (NpcColossus.empty())
                    return;

                for (std::list<Creature*>::iterator iter = NpcColossus.begin(); iter != NpcColossus.end(); ++iter)
                {
                    if( (*iter)->isDead() )
                        NbrColossusMort++;
                }

                if (NbrColossusMort == 2)
                    instance->SetData(DATA_COLOSSUS, 2);

                return;
            }

            void UpdateAI(uint32 const diff)
            {
                if(TestCrash == 0)
                {
                    RestartWhenCrash();
                    TestCrash = 1;
                }

                if (!me->isInCombat())
                    return;

                if(Unit* Lacible = me->getVictim())
                {
                    if(Lacible->GetDisplayId() == 24914)
                    {
                        HandleAccessorys(false);
                        _Reset();
                        return;
                    }
                }

                if (me->getThreatManager().isThreatListEmpty())
                {
                    EnterEvadeMode();
                    return;
                }
                // Check for shutdown initialization
                if (Shutdown == RAID_MODE(TWO_SEATS, FOUR_SEATS))
                {
                    Shutdown = 0;
                    events.ScheduleEvent(EVENT_SHUTDOWN, 4000);
                    me->RemoveAurasDueToSpell(SPELL_OVERLOAD_CIRCUIT);
                    me->InterruptNonMeleeSpells(true);
                    return;
                }


                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (me->HasAura(SPELL_SYSTEMS_SHUTDOWN))
                {
                    me->SetReactState(REACT_PASSIVE);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED | UNIT_STATE_ROOT);
                    return;
                }
                else
                {
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED | UNIT_STATE_ROOT);
                }

                if(me->HasAura(SPELL_FLAME_VENTS))
                {
                    me->AddUnitState(UNIT_STATE_STUNNED | UNIT_STATE_ROOT);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
                }
                else
                {
                    me->ClearUnitState(UNIT_STATE_STUNNED | UNIT_STATE_ROOT);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
                }

                events.Update(diff);
                uint32 eventId = events.GetEvent();

                if (!me->getVictim())
                    UpdateVictim();

                switch (eventId)
                {
                    case 0: break;
                    case EVENT_PURSUE:
                        Talk(SAY_TARGET);
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 300.0f))
                            if (target->GetTypeId() == TYPEID_PLAYER || target->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_PLAYER_VEHICLE))
                                me->AddAura(SPELL_PURSUED, target);

                        events.RepeatEvent(30*IN_MILLISECONDS);
                        break;
                    case EVENT_MISSILE:
                        if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 400.0f))
                            DoCast(pTarget, SPELL_MISSILE_BARRAGE);

                        events.RepeatEvent(2*IN_MILLISECONDS);
                        break;
                    case EVENT_VENT:
                        DoCastAOE(SPELL_FLAME_VENTS);
                        events.RepeatEvent(20*IN_MILLISECONDS);
                        break;
                    case EVENT_SPEED:
                        DoCastAOE(SPELL_GATHERING_SPEED);
                        events.RepeatEvent(15*IN_MILLISECONDS);
                        break;
                    case EVENT_SUMMON:
                        if (summons.size() < 15)
                            if (Creature* lift = DoSummonFlyer(NPC_MECHANOLIFT, me, 30.0f, 50.0f, 0))
                                lift->GetMotionMaster()->MoveRandom(100);

                        events.RepeatEvent(2*IN_MILLISECONDS);
                        break;
                    case EVENT_SHUTDOWN:
                        Talk(SAY_OVERLOAD);
                        HandleAccessorys(false);
                        me->StopMoving();
                        me->MonsterTextEmote(EMOTE_OVERLOAD, 0, true);
                        me->CastSpell(me, SPELL_SYSTEMS_SHUTDOWN, true);

                        // Achievement fails once SHUTDOWN got active
                        Shutout = false;
                        events.CancelEvent(EVENT_SHUTDOWN);
                        events.ScheduleEvent(EVENT_REPAIR, 20*IN_MILLISECONDS);
                        break;
                    case EVENT_REPAIR:
                        me->MonsterTextEmote(EMOTE_REPAIR, 0, true);
                        me->ClearUnitState(UNIT_STATE_STUNNED | UNIT_STATE_ROOT);
                        HandleAccessorys(true);
                        events.CancelEvent(EVENT_REPAIR);
                        break;
                    case EVENT_THORIMS_HAMMER: // Tower of Storms
                        if(SpeechThorims == 0)
                        {
                            Talk(SAY_TOWER_STORM);
                            SpeechThorims = 1;
                        }
                        for (uint8 i = 0; i < RAID_MODE(7, 15); i++)
                        {
                            Position pos = Misc[0];
                            pos.m_positionX += frand(-100, 105);
                            pos.m_positionY += frand(-95, 95);
                            if (Creature* thorimBeacon = DoSummon(NPC_THORIM_BEACON, pos, 20*IN_MILLISECONDS, TEMPSUMMON_TIMED_DESPAWN))
                                thorimBeacon->GetMotionMaster()->MoveRandom(100);
                        }
                        events.RepeatEvent(40*IN_MILLISECONDS);
                        break;
                    case EVENT_MIMIRONS_INFERNO: // Tower of Flames
                        me->SummonCreature(NPC_MIMIRON_BEACON, InfernoStart);
                        Talk(SAY_TOWER_FLAME);
                        events.CancelEvent(EVENT_MIMIRONS_INFERNO);
                        break;
                    case EVENT_HODIRS_FURY:      // Tower of Frost
                        if(SpeechHodirs == 0)
                        {
                            Talk(SAY_TOWER_FROST);
                            SpeechHodirs = 1;
                        }
                        for (uint8 i = 0; i < RAID_MODE(5, 7); i++)
                        {
                            Position pos = Misc[0];
                            pos.m_positionX += frand(-100, 105);
                            pos.m_positionY += frand(-95, 95);
                            if (Creature* hodir = DoSummon(NPC_HODIR_BEACON, pos, 20*IN_MILLISECONDS, TEMPSUMMON_TIMED_DESPAWN ))
                                hodir->GetMotionMaster()->MoveRandom(100);
                        }
                        events.RepeatEvent(40*IN_MILLISECONDS);
                        break;
                    case EVENT_FREYAS_WARD:    // Tower of Nature
                        {
                            if(SpeechFreya == 0)
                            {
                                Talk(SAY_TOWER_NATURE);
                                SpeechFreya = 1;
                            }
                            if(SpawnFreyaAdd == 4)
                                SpawnFreyaAdd = 0;
                            if (Creature* AddFreya = me->SummonCreature(NPC_FREYA_BEACON, FreyaBeacons[SpawnFreyaAdd]))
                            {
                                DoCast(AddFreya,SPELL_FREYA_S_WARD_EFFECT_1,true) ;
                                DoCast(AddFreya,SPELL_FREYA_S_WARD_EFFECT_2,true) ;
                            }
                            SpawnFreyaAdd++;
                            events.RepeatEvent(20*IN_MILLISECONDS);
                            break;
                        }
                    default:
                        events.PopEvent();
                        break;
                }

                if (!me->HasAura(SPELL_SYSTEMS_SHUTDOWN))
                    DoBatteringRamIfReady();
                DoMeleeAttackIfReady();
            }

            void SpellHitTarget(Unit* target, SpellInfo const* spell)
            {
                if (!target || !spell)
                    return;

                // The Target hit by PURSUED should really be pursued, not only be a target of selected spells
                if (spell->Id == SPELL_PURSUED)
                {
                    // Force as spell-target - eases selection
                    pursueTarget = target->GetGUID();

                    // Force threat handling
                    DoResetThreat();
                    me->AddThreat(target, std::numeric_limits<float>::max());
                    me->GetMotionMaster()->MoveChase(target);

                    if (Player* player = target->GetCharmerOrOwnerPlayerOrPlayerItself())
                        me->MonsterTextEmote(EMOTE_PURSUE, player->GetGUID(), true);
                }
            }

            void KilledUnit(Unit* /*Victim*/)
            {
                if (!(rand()%5))
                    Talk(SAY_SLAY);
            }

            void DoAction(int32 const action)
            {
                // Stripped numeric dependency
                if (action == ACTION_TOWER_OF_FLAMES_DESTROYED || action == ACTION_TOWER_OF_FROST_DESTROYED || action == ACTION_TOWER_OF_LIFE_DESTROYED || action == ACTION_TOWER_OF_STORM_DESTROYED) // Tower destruction, debuff leviathan loot and reduce active tower count
                {
                    if (me->HasLootMode(LOOT_MODE_DEFAULT | LOOT_MODE_HARD_MODE_1 | LOOT_MODE_HARD_MODE_2 | LOOT_MODE_HARD_MODE_3 | LOOT_MODE_HARD_MODE_4) && GetActiveTowersCount() == 4)
                        me->RemoveLootMode(LOOT_MODE_HARD_MODE_4);

                    if (me->HasLootMode(LOOT_MODE_DEFAULT | LOOT_MODE_HARD_MODE_1 | LOOT_MODE_HARD_MODE_2 | LOOT_MODE_HARD_MODE_3) && GetActiveTowersCount() == 3)
                        me->RemoveLootMode(LOOT_MODE_HARD_MODE_3);

                    if (me->HasLootMode(LOOT_MODE_DEFAULT | LOOT_MODE_HARD_MODE_1 | LOOT_MODE_HARD_MODE_2) && GetActiveTowersCount() == 2)
                        me->RemoveLootMode(LOOT_MODE_HARD_MODE_2);

                    if (me->HasLootMode(LOOT_MODE_DEFAULT | LOOT_MODE_HARD_MODE_1) && GetActiveTowersCount() == 1)
                        me->RemoveLootMode(LOOT_MODE_HARD_MODE_1);
                }

                switch (action)
                {
                    case ACTION_TOWER_OF_STORM_DESTROYED:
                        towerOfStorms = false;
                        break;
                    case ACTION_TOWER_OF_FROST_DESTROYED:
                        towerOfFrost = false;
                        break;
                    case ACTION_TOWER_OF_FLAMES_DESTROYED:
                        towerOfFlames = false;
                        break;
                    case ACTION_TOWER_OF_LIFE_DESTROYED:
                        towerOfLife = false;
                        break;
                    case ACTION_ACTIVATE_HARD_MODE:  // Activate hard-mode enable all towers, apply buffs on leviathan
                        towerOfStorms = true;
                        towerOfLife = true;
                        towerOfFlames = true;
                        towerOfFrost = true;
                        me->SetLootMode(LOOT_MODE_DEFAULT | LOOT_MODE_HARD_MODE_1 | LOOT_MODE_HARD_MODE_2 | LOOT_MODE_HARD_MODE_3 | LOOT_MODE_HARD_MODE_4);
                        break;
                    case ACTION_MOVE_TO_CENTER_POSITION: // Triggered by 2 Collossus near door
                        if (!me->isDead())
                        {
                            me->SetHomePosition(Center.GetPositionX(), Center.GetPositionY(), Center.GetPositionZ(), 0);
                            me->GetMotionMaster()->MoveCharge(Center.GetPositionX(), Center.GetPositionY(), Center.GetPositionZ()); //position center
                            me->SetReactState(REACT_AGGRESSIVE);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_STUNNED);
                            return;
                        }
                        break;
                    case ACTION_OVERLOAD_CIRCUIT:
                        ++Shutdown; // Overloading is performed by "me", but it's expensive.
                        if (Shutdown == RAID_MODE(2, 4))
                        {
                            Shutdown = 0;
                            Shutout = false;
                            events.ScheduleEvent(EVENT_SHUTDOWN, 0);
                            me->InterruptNonMeleeSpells(true);

                            // force all players to exit
                            for (uint8 i = RAID_MODE(2, 0); i < 4; ++i)
                                if (Unit* seat = vehicle->GetPassenger(i))
                                    if (seat->GetVehicleKit())
                                        if (Unit* passenger = seat->GetVehicleKit()->GetPassenger(SEAT_PLAYER))
                                            passenger->ExitVehicle();
                        }

                        break;
                    default:
                        break;
                }
            }

            private:
            void DoBatteringRamIfReady()
            {
                if (me->isAttackReady())
                {
                    Unit* target = ObjectAccessor::GetUnit(*me, pursueTarget);
                    if (me->IsWithinCombatRange(target, 30.0f))
                    {
                        DoCast(target, SPELL_BATTERING_RAM);
                        me->resetAttackTimer();
                    }
                }
            }

            uint64 pursueTarget;
            Vehicle* vehicle;
            uint8 Shutdown, SpawnFreyaAdd, SpeechFreya, SpeechThorims, SpeechHodirs, TestCrash;
            bool towerOfStorms, towerOfLife, towerOfFlames, towerOfFrost, Shutout, Unbroken, Pursued, checkUnbrokenOnReset;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<boss_flame_leviathanAI>(creature);
        }
};


class npc_brann_bronzebeard_outro : public CreatureScript
{
public:
    npc_brann_bronzebeard_outro() : CreatureScript("npc_brann_bronzebeard_outro") { }

    struct npc_brann_bronzebeard_outroAI : public npc_escortAI
    {
        npc_brann_bronzebeard_outroAI(Creature* pCreature) : npc_escortAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;

        bool bSteppingBrann;
        uint32 uiStepBrann;
        uint32 uiPhaseTimerBrann;

        void Reset()
        {
            uiPhaseTimerBrann = 20000;
            bSteppingBrann = true;
            uiStepBrann = 0;
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }

        void WaypointReached(uint32 /*pointId*/) {}

        void JumpToNextStepBrann(uint32 uiTimerBrann)
        {
            uiPhaseTimerBrann = uiTimerBrann;
            ++uiStepBrann;
        }

        void UpdateAI(const uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);

            if (bSteppingBrann)
            {
                if (uiPhaseTimerBrann <= diff)
                {
                    switch(uiStepBrann)
                    {
                    case 0:
                    {
                        if (Creature* Rhydian = me->SummonCreature(NPC_RHYDIAN_OURTO, *me))
                        {
                            float x, y, z;
                            me->GetPosition(x, y, z);
                            //      z = me->GetMap()->GetHeight(me->GetPhaseMask(), x, y, z);
                            Rhydian->GetMotionMaster()->MovePoint(0, x + 1, y + 1, z);
                            Rhydian->SetFacingToObject(me);
                        }
                        if (Unit* Rhydian = me->FindNearestCreature(NPC_RHYDIAN_OURTO, 20.0f))
                            Rhydian->MonsterYell("Our friends fought well, Brann, but we're not done yet.", LANG_UNIVERSAL, 0);
                        JumpToNextStepBrann(5000);
                        break;
                    }
                    case 1:
                        me->MonsterYell("Perhaps so, but it's only a matter of time until we break back into Ulduar. Any luck finding a way to teleport inside?", LANG_UNIVERSAL, 0);
                        JumpToNextStepBrann(7000);
                        break;
                    case 2:
                        if (Unit* Rhydian = me->FindNearestCreature(NPC_RHYDIAN_OURTO, 20.0f))
                            Rhydian->MonsterYell("None at all. I suspect it has something to do with that giant mechanical construct that our scouts spotted in front of the gate.", LANG_UNIVERSAL, 0);
                        JumpToNextStepBrann(8000);
                        break;
                    case 3:
                        me->MonsterYell("What about the plated proto-drake and the fire giant that were spotted nearby? Think your mages can handle those?", LANG_UNIVERSAL, 0);
                        JumpToNextStepBrann(8000);
                        break;
                    case 4:
                        if (Unit* Rhydian = me->FindNearestCreature(NPC_RHYDIAN_OURTO, 20.0f))
                            Rhydian->MonsterYell("The Kirin Tor can't possibly spare any additional resources to take on anything that size. We may not have to though.", LANG_UNIVERSAL, 0);
                        JumpToNextStepBrann(7000);
                        break;
                    case 5:
                        if (Unit* Rhydian = me->FindNearestCreature(NPC_RHYDIAN_OURTO, 20.0f))
                            Rhydian->MonsterYell("We can sneak past them. As long as we can take down that construct in front of the gate, we should be able to get inside.", LANG_UNIVERSAL, 0);
                        JumpToNextStepBrann(7000);
                        break;
                    case 6:
                        me->MonsterYell("Sneak? What do you think we are, marmots?", LANG_UNIVERSAL, 0);
                        JumpToNextStepBrann(5000);
                        break;
                    case 7:
                        if (Unit* Rhydian = me->FindNearestCreature(NPC_RHYDIAN_OURTO, 20.0f))
                            Rhydian->MonsterYell(" We're hunting an old god, Brann.", LANG_UNIVERSAL, 0);
                        JumpToNextStepBrann(4000);
                        break;
                    case 8:
                        me->MonsterYell("Fine. If our allies are going to be the ones getting their hands dirty, we'll leave it to them to decide how to proceed.", LANG_UNIVERSAL, 0);
                        JumpToNextStepBrann(5000);
                        break;
                    default :
                        bSteppingBrann = false;
                        break;
                    }
                } else uiPhaseTimerBrann -= diff;
            }
        }

    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_brann_bronzebeard_outroAI(pCreature);
    }
};

class npc_flame_leviathan_defense_cannon : public CreatureScript
{
public:
    npc_flame_leviathan_defense_cannon() : CreatureScript("npc_flame_leviathan_defense_cannon") {}

    struct npc_flame_leviathan_defense_cannonAI : public ScriptedAI
    {
        npc_flame_leviathan_defense_cannonAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            NapalmTimer = 15*IN_MILLISECONDS;
            DoCast(me, AURA_STEALTH_DETECTION);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!me->getVictim())
                UpdateVictim();

            if (NapalmTimer <= diff)
            {
                if (Unit* target = me->SelectNearestTarget(200.0f))

                    if (CanAIAttack(target))
                        DoCast(target, SPELL_NAPALM, true);

                NapalmTimer = 10*IN_MILLISECONDS;
            }
            else
                NapalmTimer -= diff;
        }

        bool CanAIAttack(Unit const* who) const
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
                return false;
            return true;
        }

    private:
        uint32 NapalmTimer;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_flame_leviathan_defense_cannonAI(creature);
    }
};

class npc_flame_leviathan_seat : public CreatureScript
{
    public:
        npc_flame_leviathan_seat() : CreatureScript("npc_flame_leviathan_seat") {}

        struct npc_flame_leviathan_seatAI : public ScriptedAI
        {
            npc_flame_leviathan_seatAI(Creature* creature) : ScriptedAI(creature), vehicle(creature->GetVehicleKit())
            {
                ASSERT(vehicle);
                me->SetDisplayId(me->GetCreatureTemplate()->Modelid2);
            }

            void Reset()
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void SetImmunitys(Unit* target, bool apply)
            {
                target->ApplySpellImmune(0, IMMUNITY_ID, SPELL_PURSUED, apply);
                target->ApplySpellImmune(0, IMMUNITY_ID, SPELL_MISSILE_BARRAGE, apply);
                target->ApplySpellImmune(0, IMMUNITY_ID, SPELL_BATTERING_RAM, apply);
                target->ApplySpellImmune(0, IMMUNITY_ID, SPELL_LASH, apply);
                target->ApplySpellImmune(0, IMMUNITY_ID, SPELL_THORIMS_HAMMER, apply); // Thorims Hammer
                target->ApplySpellImmune(0, IMMUNITY_ID, SPELL_MIMIRONS_INFERNO_DAMAGE, apply); // Mimirons Inferno
                target->ApplySpellImmune(0, IMMUNITY_ID, SPELL_HODIRS_FURY_DAMAGE, apply); // Hodirs Fury
            }


            void PassengerBoarded(Unit* who, int8 seatId, bool apply)
            {
                if (!me->GetVehicle())
                    return;

                if (seatId == SEAT_PLAYER)
                {
                    SetImmunitys(who, apply);

                    if (!apply) // i.e. kick player out
                    {
                        who->RemoveAurasDueToSpell(SPELL_FORCE_REACTION);
                        who->CastSpell(who, SPELL_SMOKE_TRAIL, true);
                        who->StopMoving();
                        who->GetMotionMaster()->MoveKnockbackFrom(me->GetPositionX() + 20.0f, me->GetPositionY(), me->GetPositionZ() + 30.0f, 5.0f);
                        return;
                    }
                    else
                    {
                        me->AddAura(SPELL_FORCE_REACTION, who);
                        Talk(SAY_PLAYER_RIDING);
                    }

                    if (Creature* turret = me->GetVehicleKit()->GetPassenger(SEAT_TURRET)->ToCreature())
                    {
                        turret->setFaction(me->GetVehicleBase()->getFaction());
                        turret->SetUInt32Value(UNIT_FIELD_FLAGS, 0); // unselectable
                        if (CreatureAI* ai = turret->AI())
                            ai->AttackStart(who);
                    }

                    if (Creature* device = me->GetVehicleKit()->GetPassenger(SEAT_DEVICE)->ToCreature())
                    {
                        device->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                        device->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    }

                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                }
                else if (seatId == SEAT_TURRET)
                {
                    if (apply)
                        return;

                    if (Unit* device = vehicle->GetPassenger(SEAT_DEVICE))
                    {
                        device->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                        device->SetUInt32Value(UNIT_FIELD_FLAGS, 0); // unselectable
                    }
                }
            }

            private:
                Vehicle* vehicle;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_flame_leviathan_seatAI(creature);
        }
};

class npc_flame_leviathan_defense_turret : public CreatureScript
{
    public:
        npc_flame_leviathan_defense_turret() : CreatureScript("npc_flame_leviathan_defense_turret") {}

        struct npc_flame_leviathan_defense_turretAI : public TurretAI
        {
            npc_flame_leviathan_defense_turretAI(Creature* creature) : TurretAI(creature)
            {
                me->SetVisible(true);
            }

            void Reset()
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->setActive(true);
            }

            void DamageTaken(Unit* who, uint32 &damage)
            {
                if (!CanAIAttack(who))
                    damage = 0;

                if (damage >= me->GetHealth())
                    if (Vehicle* seat = me->GetVehicle())
                        if (Unit* device = seat->GetPassenger(SEAT_DEVICE))
                            device->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }


            bool CanAIAttack(Unit const* who) const
            {
                if (who->GetTypeId() != TYPEID_PLAYER || !who->GetVehicle() || who->GetVehicleBase()->GetEntry() != NPC_SEAT)
                    return false;

                return true;
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_flame_leviathan_defense_turretAI(creature);
        }
};

class npc_flame_leviathan_overload_device : public CreatureScript
{
    public:
        npc_flame_leviathan_overload_device() : CreatureScript("npc_flame_leviathan_overload_device") {}

        struct npc_flame_leviathan_overload_deviceAI : public PassiveAI
        {
            npc_flame_leviathan_overload_deviceAI(Creature* creature) : PassiveAI(creature)
            {
                me->SetVisible(true);
            }

            void InitializeAI()
            {
                me->setActive(true);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            }

            void OnSpellClick(Unit* /*player*/, bool& /*result*/)
            {
                if (me->GetVehicle())
                {
                    if (InstanceScript* myInst = me->GetInstanceScript())
                    {
                        if (Creature* leviathan = ObjectAccessor::GetCreature(*me, myInst->GetData64(BOSS_LEVIATHAN)))
                            leviathan->AI()->DoAction(ACTION_OVERLOAD_CIRCUIT);
                    }

                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    if (Unit* player = me->GetVehicle()->GetPassenger(SEAT_PLAYER))
                    {
                        me->GetVehicleBase()->CastSpell(player, SPELL_SMOKE_TRAIL, true);
                        player->GetMotionMaster()->MoveKnockbackFrom(me->GetVehicleBase()->GetPositionX(), me->GetVehicleBase()->GetPositionY(), 30.0f, 30.0f);
                        player->ExitVehicle();
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_flame_leviathan_overload_deviceAI(creature);
        }
};

class npc_flame_leviathan_safety_container : public CreatureScript
{
    public:
        npc_flame_leviathan_safety_container() : CreatureScript("npc_flame_leviathan_safety_container") {}

        struct npc_flame_leviathan_safety_containerAI : public PassiveAI
        {
            npc_flame_leviathan_safety_containerAI(Creature* creature) : PassiveAI(creature) {}

            void JustDied(Unit* /*killer*/)
            {
                float x, y, z;
                me->GetPosition(x, y, z);
                z = me->GetMap()->GetHeight(me->GetPhaseMask(), x, y, z);
                me->GetMotionMaster()->MovePoint(0, x, y, z);
                me->SetPosition(x, y, z, 0);
            }

            void UpdateAI(uint32 const /*diff*/) {}
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_flame_leviathan_safety_containerAI(creature);
        }
};

class npc_mechanolift : public CreatureScript
{
    public:
        npc_mechanolift() : CreatureScript("npc_mechanolift") {}

        struct npc_mechanoliftAI : public PassiveAI
        {
            npc_mechanoliftAI(Creature* creature) : PassiveAI(creature)
            {
                ASSERT(me->GetVehicleKit());
            }

            void Reset()
            {
                me->AddUnitMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_FLYING);
                me->SetVisible(true);
                MoveTimer = 0;
                me->GetMotionMaster()->MoveRandom(50);
            }

            void DamageTaken(Unit* /*who*/, uint32 &damage)
            {
                if (damage >= me->GetHealth())
                {
                    if (me->IsVisible()) // Works as a guard, summons should not be called multiple times.
                    {
                        me->SetVisible(false);
                        DoCast(SPELL_DUSTY_EXPLOSION);  // Note: Spell not castable if dead, thus, moved here
                        if (Creature* liquid = DoSummon(NPC_LIQUID, me, 0, 190*IN_MILLISECONDS, TEMPSUMMON_TIMED_DESPAWN))
                        {
                            float x, y, z;
                            me->GetPosition(x, y, z);
                            z = me->GetMap()->GetHeight(me->GetPhaseMask(), x, y, MAX_HEIGHT);

                            liquid->CastSpell(liquid, SPELL_DUST_CLOUD_IMPACT, true);
                            // Just hope that this works in time...

                            liquid->SetCanFly(true);
                            liquid->GetMotionMaster()->MovePoint(0, x, y, z);
                        }

                        me->RemoveUnitMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_FLYING);
                        //                        me->SendMovementFlagUpdate();
                    }
                }
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type == POINT_MOTION_TYPE && id == 1)
                    if (Creature* container = me->FindNearestCreature(NPC_CONTAINER, 5, true))
                        container->EnterVehicle(me);
            }

            // Looks a little bit curious, so I'll explain:
            // We are a lift, that tries to transport containers. As there isn't any other (yet known) way to handle this, it's done using a passenger <-> vehicle relation.
            void UpdateAI(const uint32 diff)
            {
                if (MoveTimer <= diff)
                {
                    if (me->GetVehicleKit()->HasEmptySeat(-1))
                    {
                        Creature* container = me->FindNearestCreature(NPC_CONTAINER, 50, true);
                        if (container)
                            if (!container->GetVehicle())
                                me->GetMotionMaster()->MovePoint(1, container->GetPositionX(), container->GetPositionY(), container->GetPositionZ());
                    }

                    MoveTimer = 30*IN_MILLISECONDS; //check next 30 seconds
                }
                else
                    MoveTimer -= diff;
            }

            private:
                uint32 MoveTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_mechanoliftAI(creature);
        }
};

class npc_liquid_pyrite : public CreatureScript
{
    enum { DISPLAY_ID_DAMAGED_PYRITE_CONTAINER = 28783 };

    public:
        npc_liquid_pyrite() : CreatureScript("npc_liquid_pyrite") {}

        struct npc_liquid_pyriteAI : public PassiveAI
        {
            npc_liquid_pyriteAI(Creature* creature) : PassiveAI(creature) {}

            void Reset()
            {
                DoCast(me, SPELL_LIQUID_PYRITE, true);
                me->SetDisplayId(28476);
                despawnTimer = 5*IN_MILLISECONDS;
            }

            void MovementInform(uint32 type, uint32 /*id*/)
            {
                if (type == POINT_MOTION_TYPE)
                {
                    DoCast(me, SPELL_DUSTY_EXPLOSION, true);
                    DoCast(me, SPELL_DUST_CLOUD_IMPACT, true);
                    me->SetDisplayId(DISPLAY_ID_DAMAGED_PYRITE_CONTAINER);
                }
            }

            void DamageTaken(Unit* /*who*/, uint32& damage)
            {
                damage = 0;
            }

            void UpdateAI(uint32 const diff)
            {
                if (despawnTimer <= diff)
                {
                    if (me->GetVehicle())
                        me->DisappearAndDie();
                    despawnTimer = 5*IN_MILLISECONDS;
                }
                else
                    despawnTimer -= diff;
            }

            private:
                uint32 despawnTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_liquid_pyriteAI(creature);
        }
};

class npc_pool_of_tar : public CreatureScript
{
    enum Flame
    {
        SPELL_FLAMES_1 = 65044,
        SPELL_FLAMES_2 = 65045
    };

    public:
        npc_pool_of_tar() : CreatureScript("npc_pool_of_tar") {}

        struct npc_pool_of_tarAI : public PassiveAI
        {
            npc_pool_of_tarAI(Creature* creature) : PassiveAI(creature)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            }

            void Reset()
            {
                //me->SetVisible(false);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->CastSpell(me, SPELL_TAR_PASSIVE, true);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            }

            void DamageTaken(Unit* /*who*/, uint32& damage)
            {
                damage = 0;
            }

            void SpellHit(Unit* /*caster*/, SpellInfo const* spell)
            {
                // Only these two spells can set the pool on fire.
                if ((spell->Id == SPELL_FLAMES_1 || spell->Id == SPELL_FLAMES_2) && !me->HasAura(SPELL_BLAZE))
                    me->CastSpell(me, SPELL_BLAZE, true);
            }

            void UpdateAI(uint32 const /*diff*/) {}
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_pool_of_tarAI(creature);
        }
};

class npc_colossus : public CreatureScript
{
    public:
        npc_colossus() : CreatureScript("npc_colossus") {}

        struct npc_colossusAI : public ScriptedAI
        {
            npc_colossusAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            void Reset()
            {
                groundSlamTimer = urand(8, 10) *IN_MILLISECONDS;
            }

            void JustDied(Unit* /*Who*/)
            {
                // Check is required, since there are 5 colossus in the instance :: Check if the distance is enough
                // Given distance ends up with ~106 x/y distance.
                if (me->GetDistance2d(Center.GetPositionX(), Center.GetPositionY()) < 150.0f)
                    instance->SetData(DATA_COLOSSUS, instance->GetData(DATA_COLOSSUS) + 1);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                if (groundSlamTimer <= diff)
                {
                    DoCastVictim(SPELL_GROUND_SLAM);
                    groundSlamTimer = urand(20, 25) * IN_MILLISECONDS;
                }
                else
                    groundSlamTimer -= diff;

                DoMeleeAttackIfReady();
            }

            private:
                InstanceScript* instance;
                uint32 groundSlamTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<npc_colossusAI>(creature);
        }
};

class npc_thorims_hammer : public CreatureScript
{
    public:
    npc_thorims_hammer() : CreatureScript("npc_thorims_hammer") { }

    struct npc_thorims_hammerAI : public Scripted_NoMovementAI
    {
        npc_thorims_hammerAI(Creature* creature) : Scripted_NoMovementAI(creature)
        {
            me->setActive(true);
            me->SetDisplayId(me->GetCreatureTemplate()->Modelid2);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_PASSIVE);
            me->AddAura(SPELL_LIGHTNING_SKYBEAM, me);
        }

        void Reset()
        {
            _timer = urand(2, 10) *IN_MILLISECONDS;
            _action = 1;
        }

        void UpdateAI(uint32 const diff)
        {
            if (_timer <= diff)
            {
                switch (_action)
                {
                    case 1:
                        me->AddAura(SPELL_LIGHTNING_SKYBEAM, me);
                        ++_action;
                        _timer = 4*IN_MILLISECONDS;
                        break;
                    case 2:
                        if (Creature* trigger = DoSummonFlyer(NPC_THORIM_TARGET_BEACON, me, 50.0f, 0, 10*IN_MILLISECONDS, TEMPSUMMON_TIMED_DESPAWN))
                        {
                            trigger->SetDisplayId(trigger->GetCreatureTemplate()->Modelid2);
                            trigger->CastSpell(me, SPELL_THORIMS_HAMMER, true);
                        }
                        ++_action;
                        _timer = 4*IN_MILLISECONDS;
                        break;
                    case 3:
                        me->RemoveAllAuras();
                        _timer = 30*IN_MILLISECONDS;
                        break;
                    }
                }
                else
                    _timer -= diff;
        }

            private:
            uint32 _action;
            uint32 _timer;
        };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_thorims_hammerAI(creature);
    }
};
class npc_mimirons_inferno : public CreatureScript
{
    public:
        npc_mimirons_inferno() : CreatureScript("npc_mimirons_inferno") {}

        struct npc_mimirons_infernoAI : public npc_escortAI
        {
            npc_mimirons_infernoAI(Creature* creature) : npc_escortAI(creature)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->SetReactState(REACT_PASSIVE);
                me->AddAura(SPELL_RED_SKYBEAM, me);
            }

            void Reset()
            {
                infernoTimer = 2*IN_MILLISECONDS;
            }

            void WaypointReached(uint32 /*pointId*/) {}

            void UpdateAI(uint32 const diff)
            {
                npc_escortAI::UpdateAI(diff);

                if (!HasEscortState(STATE_ESCORT_ESCORTING))
                    Start(false, true, 0, NULL, false, true);
                else
                {
                    if (infernoTimer <= diff)
                    {
                        if (Creature* trigger = DoSummonFlyer(NPC_MIMIRON_TARGET_BEACON, me, 20.0f, 0, 6*IN_MILLISECONDS, TEMPSUMMON_TIMED_DESPAWN))
                        {
                            trigger->SetDisplayId(trigger->GetCreatureTemplate()->Modelid2);
                            trigger->CastSpell(trigger->GetPositionX(),trigger->GetPositionY(),trigger->GetPositionZ()-20, SPELL_MIMIRONS_INFERNO_DAMAGE, true);
                            infernoTimer = 2*IN_MILLISECONDS;
                        }
                    }
                    else
                        infernoTimer -= diff;
                }
            }
        private:
            uint32 infernoTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_mimirons_infernoAI(creature);
        }
};
class npc_hodirs_fury : public CreatureScript
{
public:
    npc_hodirs_fury() : CreatureScript("npc_hodirs_fury") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new  npc_hodirs_furyAI(pCreature);
    }

    struct npc_hodirs_furyAI : public ScriptedAI
    {
        npc_hodirs_furyAI(Creature* pCreature) : ScriptedAI (pCreature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_AGGRESSIVE);
            me->setActive(true);
            me->SetDisplayId(me->GetCreatureTemplate()->Modelid2);
        }

        uint32 uiTargetChangeTimer;
        uint32 uiHodirFuryTimer;

        void Reset()
        {
            me->AddAura(SPELL_BLUE_SKYBEAM, me);
            uiTargetChangeTimer = 2*IN_MILLISECONDS;
            DoZoneInCombat();
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (me->GetDistance2d(who->GetPositionX(), who->GetPositionY()) < 3.0f && who->IsVehicle() )
            {
                if (Creature* targetBeacon = me->SummonCreature(NPC_HODIR_TARGET_BEACON, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()+70, 0, TEMPSUMMON_TIMED_DESPAWN, 20*IN_MILLISECONDS))
                {
                    targetBeacon->SetDisplayId(targetBeacon->GetCreatureTemplate()->Modelid2);
                    targetBeacon->CastSpell(me, SPELL_HODIRS_FURY_DAMAGE, true);
                }
                uiHodirFuryTimer = urand(7*IN_MILLISECONDS, 10*IN_MILLISECONDS);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (uiHodirFuryTimer > diff)
                uiHodirFuryTimer -= diff;

            if (!UpdateVictim())
                return;

            if (uiTargetChangeTimer <= diff)
            {
                if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                    me->GetMotionMaster()->MoveChase(pTarget);

                uiTargetChangeTimer = urand(4*IN_MILLISECONDS, 7*IN_MILLISECONDS);
            } else uiTargetChangeTimer -= diff;
        }
    };

};
class npc_freyas_ward : public CreatureScript
{
    public:
        npc_freyas_ward() : CreatureScript("npc_freyas_ward") {}

        struct npc_freyas_wardAI : public Scripted_NoMovementAI
        {
            npc_freyas_wardAI(Creature* creature) : Scripted_NoMovementAI(creature) {}

            void Reset()
            {
                me->AddAura(SPELL_GREEN_SKYBEAM, me);
                me->setActive(true);
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                summonTimer = 5*IN_MILLISECONDS;
            }

            void UpdateAI(uint32 const /*diff*/)
            {
                if (!me->HasAura(AURA_DUMMY_GREEN))
                    me->CastSpell(me, AURA_DUMMY_GREEN, true);

                if (!UpdateVictim())
                    return;
            }

            private:
                uint32 summonTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_freyas_wardAI(creature);
        }
};
class npc_freya_ward_of_life : public CreatureScript
{
    public:
        npc_freya_ward_of_life() : CreatureScript("npc_freya_ward_of_life") {}

        struct npc_freya_ward_of_lifeAI : public ScriptedAI
        {
            npc_freya_ward_of_lifeAI(Creature* creature) : ScriptedAI(creature) {}

            void Reset()
            {
                me->setActive(true);
                lashTimer = urand(2, 8) *IN_MILLISECONDS;
                if (Unit* target = me->FindNearestPlayer(200.0f))
                    me->GetMotionMaster()->MoveFollow(target, 0.0f, 0.0f);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                if (lashTimer <= diff)
                {
                    DoCast(SPELL_LASH);
                    lashTimer = urand(8, 12) *IN_MILLISECONDS;
                }
                else
                    lashTimer -= diff;

                DoMeleeAttackIfReady();
            }

            private:
                uint32 lashTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_freya_ward_of_lifeAI(creature);
        }
};


// TODO: Add script to database - not sure where exactly, seems to be a dummy that is used as a seat handler
class npc_leviathan_player_vehicle : public CreatureScript
{
    public:
        npc_leviathan_player_vehicle() : CreatureScript("npc_leviathan_player_vehicle") {}

        struct npc_leviathan_player_vehicleAI : public NullCreatureAI
        {
            npc_leviathan_player_vehicleAI(Creature* creature) : NullCreatureAI(creature)
            {
            }

            void SetImmunitys(Unit* /*target*/, bool apply)
            {
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, false);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DAZE, false);
                me->ApplySpellImmune(0, IMMUNITY_ID, 62845, apply);
            }

            void OnSpellClick(Unit* player, bool& /*result*/)
            {
                if (me->GetEntry() != 33109)
                    return;
                if (Vehicle *veh = me->GetVehicleKit())
                {
                    if (!veh->GetPassenger(0))
                        player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, me, TRIGGERED_IGNORE_CASTER_MOUNTED_OR_ON_VEHICLE);
                    else
                    {
                        if (Vehicle *veh = me->GetVehicleKit())
                        {
                            Unit *seatC = veh->GetPassenger(1);
                            if (!seatC)
                                seatC = me->SummonCreature(33167, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                            if (seatC)
                            {
                                seatC->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 2, me, TRIGGERED_IGNORE_CASTER_MOUNTED_OR_ON_VEHICLE);
                                if (seatC->GetVehicleKit())
                                    if (!seatC->GetVehicleKit()->GetPassenger(0))
                                        player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, seatC, TRIGGERED_IGNORE_CASTER_MOUNTED_OR_ON_VEHICLE);
                            }
                        }
                    }
                }
            }

            void PassengerBoarded(Unit* unit, int8 seat, bool apply)
            {
                if (!unit->ToPlayer())
                    return;

                if (seat == SEAT_PLAYER)
                {
                    SetImmunitys(unit, apply);

                    if (apply)
                        unit->CastSpell(me, SPELL_GEAR_SCALING, true);
                    else
                        me->RemoveAurasDueToSpell(SPELL_GEAR_SCALING);

                    if (InstanceScript* instance = me->GetInstanceScript())
                    {
                        if (Creature* leviathan = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_LEVIATHAN)))
                        {
                            if (leviathan->isInCombat())
                            {
                                me->SetInCombatWith(leviathan);
                                me->AddThreat(leviathan, 1.0f);
                                leviathan->SetInCombatWith(me);
                                leviathan->AddThreat(me, 1.0f);

                                if (apply)
                                    me->SetHealth(uint32(me->GetHealth() / 2));
                            }
                        }
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_leviathan_player_vehicleAI(creature);
        }
};

// npc lore keeper
#define GOSSIP_ITEM_1  "Activate secondary defensive systems"
#define GOSSIP_ITEM_2  "Confirmed"

enum gauntSays
{
    SAY_DELLORAH_1         = 0,
    SAY_DELLORAH_2         = 1,
    SAY_DELLORAH_3         = 2,
    SAY_DELLORAH_4         = 3,
    SAY_DELLORAH_5         = 4,
    SAY_DELLORAH_6         = 5,
    SAY_DELLORAH_7         = 6,
    SAY_DELLORAH_8         = 7,
    SAY_DELLORAH_9         = 8,

    SAY_KEEPER_1           = 0,
    SAY_KEEPER_2           = 1,
    SAY_KEEPER_3           = 2,
    SAY_KEEPER_4           = 3,
    SAY_KEEPER_5           = 4,
};

enum gauntActions
{
    ACTION_START_EVENT     = 4241,
    ACTION_HARD_MODE_EVENT = 4242
};

class npc_lorekeeper : public CreatureScript
{
    public:
        npc_lorekeeper() : CreatureScript("npc_lorekeeper") {}

        struct npc_lorekeeperAI : public ScriptedAI
        {
            npc_lorekeeperAI(Creature* creature) : ScriptedAI(creature)
            {
                eventStep = 0;
                mui_eventChange = 7000;
                eventPreStart = false;
                guid = 0;
                me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                _introDone = false;
            }

            void MoveInLineOfSight(Unit* who)
            {
                if (!_introDone && me->IsWithinDistInMap(who, 10.0f))
                {
                    if (who->GetTypeId() == TYPEID_PLAYER)
                    {
                        guid = who->GetGUID();
                        DoAction(ACTION_START_EVENT);
                        _introDone = true;
                    }
                }
            }

            void SetGUID(uint64 g, int32 /*type*/)
            {
                guid = g;
            }


            void DoAction(int32 const action)
            {
                // Start encounter
                if (action == ACTION_SPAWN_VEHICLES)
                {
          /*                    for (int32 i = 0; i < RAID_MODE(2, 5); ++i)
                        DoSummon(VEHICLE_SIEGE, PosSiege[i], 3000, TEMPSUMMON_CORPSE_TIMED_DESPAWN);
                    for (int32 i = 0; i < RAID_MODE(2, 5); ++i)
                        DoSummon(VEHICLE_CHOPPER, PosChopper[i], 3000, TEMPSUMMON_CORPSE_TIMED_DESPAWN);
                    for (int32 i = 0; i < RAID_MODE(2, 5); ++i)
                    DoSummon(VEHICLE_DEMOLISHER, PosDemolisher[i], 3000, TEMPSUMMON_CORPSE_TIMED_DESPAWN);*/
                    return;
                }
                else if (action == ACTION_START_EVENT)
                    eventPreStart = true;
                else if (action == ACTION_HARD_MODE_EVENT)
                {
                    dellorah = me->FindNearestCreature(33701, 30);
                    if (!dellorah)
                        return;
                    dellorah->AI()->Talk(SAY_DELLORAH_8 , guid);
                    dellorah->AI()->Talk(SAY_DELLORAH_9 , guid);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (mui_eventChange <= diff)
                {
                    mui_eventChange = 7000;
                    dellorah = me->FindNearestCreature(33701, 30);
                    if (!dellorah)
                        return;
                    if (eventPreStart)
                    {
                        switch (eventStep)
                        {
                        case 0:
                            dellorah->AI()->Talk(SAY_DELLORAH_1);
                            break;
                        case 1:
                            Talk(SAY_KEEPER_1);
                            break;
                        case 2:
                            dellorah->AI()->Talk(SAY_DELLORAH_2);
                            break;
                        case 3:
                            Talk(SAY_KEEPER_2);
                            break;
                        case 4:
                            dellorah->AI()->Talk(SAY_DELLORAH_3);
                            break;
                        case 5:
                            Talk(SAY_KEEPER_3);
                            break;
                        case 6:
                            Talk(SAY_KEEPER_4);
                            break;
                        case 7:
                            dellorah->AI()->Talk(SAY_DELLORAH_4);
                            break;
                        case 8:
                            if (Creature *ry = me->FindNearestCreature(33696, 30))
                                if (Creature* Brann = me->FindNearestCreature(NPC_BRANN_BRONZBEARD, 1000, true))
                                    ry->GetMotionMaster()->MovePoint(0, Brann->GetPositionX()+4, Brann->GetPositionY(), Brann->GetPositionZ());
                            dellorah->AI()->Talk(SAY_DELLORAH_5);
                            break;
                        case 9:
                            dellorah->AI()->Talk(SAY_DELLORAH_6);
                            break;
                        case 10:
                            Talk(SAY_KEEPER_5);
                            break;
                        case 11:
                            dellorah->AI()->Talk(SAY_DELLORAH_7);
                            eventPreStart = false;
                            eventStep = 0;
                            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                            DoAction(ACTION_SPAWN_VEHICLES);
                            break;
                        default:
                            eventPreStart = false;
                            eventStep = 0;
                            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                            break;
                        }
                        eventStep++;
                    }
                }
                else
                    mui_eventChange -= diff;
            }

        private :
            uint32 eventStep;
            uint32 mui_eventChange;
            bool eventPreStart;
            Creature *dellorah;
            uint64 guid;
            bool _introDone;
        };

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        InstanceScript* instance = creature->GetInstanceScript();
        if (!instance)
            return true;

        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF+1:
            if (player)
            {
                player->PrepareGossipMenu(creature);
                instance->instance->LoadGrid(364.0f, -16.0f); //make sure leviathan is loaded

                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
                player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
            }
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            if (player)
                player->CLOSE_GOSSIP_MENU();

            if (Creature* leviathan = instance->instance->GetCreature(instance->GetData64(BOSS_LEVIATHAN)))
            {
                leviathan->AI()->DoAction(ACTION_ACTIVATE_HARD_MODE);
                creature->SetVisible(false);
                //                        creature->AI()->DoAction(ACTION_SPAWN_VEHICLES); // spawn the vehicles
                creature->AI()->DoAction(ACTION_HARD_MODE_EVENT); // spawn the vehicles
                if (Creature* Delorah = creature->FindNearestCreature(NPC_DELORAH, 1000, true))
                {
                    if (Creature* Brann = creature->FindNearestCreature(NPC_BRANN_BRONZBEARD, 1000, true))
                    {
                        Delorah->GetMotionMaster()->MovePoint(0, Brann->GetPositionX()-4, Brann->GetPositionY(), Brann->GetPositionZ());
                        //TODO Talk(xxxx, Delorah, Branz); when reached at Brann
                    }
                }
            }
            break;
        }

        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        InstanceScript* instance = creature->GetInstanceScript();
        if (instance && instance->GetData(BOSS_LEVIATHAN) !=DONE && player)
        {
            player->PrepareGossipMenu(creature);

            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lorekeeperAI(creature);
    }
};


//enable hardmode
////npc_brann_bronzebeard this requires more work involving area triggers. if reached this guy speaks through his radio..
#define NPC_PENTARUS 33624
#define GOSSIP_ITEM_BRANN_3         "Nous sommes pret !"
#define SAY_BRANN_1                 "Pentarus, vous l'avez entendue. Que vos mages levent le bouclier pour laisser passer ces braves ames !"
#define SAY_PENTARUS_1              "Bien sur, Brann, nous allons faire tomber ce bouclier pour un moment."
#define SAY_BRANN_2                 "OK ! C'est parti. Tous a vos machines, j'vous causerai d'ici avec la radio !"
#define SAY_PENTARUS_2              "Mages du Kirin Tor, au signal de Brann, levez le bouclier ! Defendez la plate-forme et nos allies de vos vies ! Pour Dalaran !"
#define SAY_BRANN_3                 "Nos allies sont prets. Levez le bouclier et faites place !"
#define SAY_BRANN_4                 "On a apercu des nains de fer qui sortaient des fortins a la base des piliers juste devant vous ! Detruisez les fortins, ca les obligera a s'replier !" //15794

class npc_brann_bronzebeard : public CreatureScript
{
public:
    npc_brann_bronzebeard() : CreatureScript("npc_brann_bronzebeard") { }

    struct npc_brann_bronzebeardAI : public npc_escortAI
    {
        npc_brann_bronzebeardAI(Creature* pCreature) : npc_escortAI(pCreature), summons(pCreature)
        {
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        bool bSteppingBrann;
        uint32 uiStepBrann;
        uint32 uiPhaseTimerBrann;
        SummonList summons;

        void Reset()
        {
            uiPhaseTimerBrann = 1000;
            bSteppingBrann = false;
            uiStepBrann = 0;
        }

        void JumpToNextStepBrann(uint32 uiTimerBrann)
        {
            uiPhaseTimerBrann = uiTimerBrann;
            ++uiStepBrann;
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }

        void WaypointReached(uint32) {}

        void SetData(uint32 id, uint32 data)
        {
            switch(id)
            {
            case TYPE_ULDUAR_EVENT:
                switch(data)
                {
                case 4:
                    uiPhaseTimerBrann = 0;
                    bSteppingBrann = true;
                    uiStepBrann    = 0;
                    break;
                case 5:
                    uiPhaseTimerBrann = 0;
                    bSteppingBrann = false;
                    uiStepBrann    = 0;
                    summons.DespawnAll();
                    me->GetMotionMaster()->MoveTargetedHome();
                    if (Unit* pIngenieur = me->FindNearestCreature(33626, 1000.0f))
                        pIngenieur->GetMotionMaster()->MoveTargetedHome();
                    if (Unit* pBataille = me->FindNearestCreature(33662, 1000.0f))
                        pBataille->GetMotionMaster()->MoveTargetedHome();
                    if (Unit* pMage = me->FindNearestCreature(33672, 1000.0f))
                        pMage->GetMotionMaster()->MoveTargetedHome();
                    if (Unit* pPentarus = me->FindNearestCreature(NPC_PENTARUS, 1000.0f))
                        pPentarus->GetMotionMaster()->MoveTargetedHome();
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    break;
                case 6:
                    summons.DespawnAll();
                    break;
                }
                break;
            default:
                break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);

            if (bSteppingBrann)
            {
                if (uiPhaseTimerBrann <= diff)
                {
                    switch(uiStepBrann)
                    {
                    case 0:
                        me->MonsterSay(SAY_BRANN_1, LANG_UNIVERSAL, 0);
                        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        JumpToNextStepBrann(3000);
                        break;
                    case 1:
                        if (Unit* pPentarus = me->FindNearestCreature(NPC_PENTARUS, 20.0f))
                            pPentarus->MonsterSay(SAY_PENTARUS_1, LANG_UNIVERSAL, 0);
                        JumpToNextStepBrann(3000);
                        break;
                    case 2:
                        me->MonsterSay(SAY_BRANN_2, LANG_UNIVERSAL, 0);
                        JumpToNextStepBrann(8000);
                        break;
                    case 3:
                        if (Unit* pIngenieur = me->FindNearestCreature(33626, 50.0f))
                            pIngenieur->GetMotionMaster()->MovePoint(0, -777.336f,-45.084f,429.843f);
                        if (Unit* pBataille = me->FindNearestCreature(33662, 50.0f))
                        {
                            pBataille->GetMotionMaster()->MovePoint(0, -686.287f,-67.053f,427.960f);
                            pBataille->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_READY1H);
                        }
                        if (Unit* pMage = me->FindNearestCreature(33672, 50.0f))
                        {
                            pMage->GetMotionMaster()->MovePoint(0, -701.350f,-51.397f,429.483f);
                            pMage->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_READY1H);
                        }
                        if (Unit* pPentarus = me->FindNearestCreature(NPC_PENTARUS, 20.0f))
                        {
                            pPentarus->GetMotionMaster()->MovePoint(0, -686.287f,-34.389f,427.960f);
                            pPentarus->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_READY1H);
                        }
                        me->GetMotionMaster()->MovePoint(0, -673.477f,-52.912f,426.130f);
                        JumpToNextStepBrann(1500);
                        break;
                    case 4:
                        if (Unit* pPentarus = me->FindNearestCreature(NPC_PENTARUS, 50.0f))
                            pPentarus->MonsterSay(SAY_PENTARUS_2, LANG_UNIVERSAL, 0);
                        JumpToNextStepBrann(5000);
                        break;
                    case 5:
                        me->MonsterSay(SAY_BRANN_3, LANG_UNIVERSAL, 0);
                        JumpToNextStepBrann(4000);
                        break;
                    case 6:
                        if (me->FindNearestGameObject(194484, 250.0f))
                        {
                            me->CastSpell(me,69900,true);
                            if (GameObject *gob = me->FindNearestGameObject(GO_ULDUAR_DOME, 250.0f))
                                gob->RemoveFromWorld();
                            //              me->FindNearestGameObject(7000001, 250.0f)->RemoveFromWorld();
                        }
                        JumpToNextStepBrann(1500);
                        break;
                    case 7:
                        me->MonsterSay(SAY_BRANN_4, LANG_UNIVERSAL, 0);
                        bSteppingBrann = false;
                        for (int32 i = 0; i < RAID_MODE(2, 5); ++i)
                            DoSummon(VEHICLE_SIEGE, PosSiege[i], 3000, TEMPSUMMON_CORPSE_TIMED_DESPAWN);
                        for (int32 i = 0; i < RAID_MODE(2, 5); ++i)
                            DoSummon(VEHICLE_CHOPPER, PosChopper[i], 3000, TEMPSUMMON_CORPSE_TIMED_DESPAWN);
                        for (int32 i = 0; i < RAID_MODE(2, 5); ++i)
                            DoSummon(VEHICLE_DEMOLISHER, PosDemolisher[i], 3000, TEMPSUMMON_CORPSE_TIMED_DESPAWN);
                        break;
                    }
                } else uiPhaseTimerBrann -= diff;

                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }
        }
    };

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT,GOSSIP_ITEM_BRANN_3,GOSSIP_SENDER_MAIN,GOSSIP_ACTION_INFO_DEF);
        pPlayer->SEND_GOSSIP_MENU(100003, pCreature->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
    {
        switch(uiAction)
        {
            case GOSSIP_ACTION_INFO_DEF:
                pCreature->AI()->SetData(TYPE_ULDUAR_EVENT, 4);
                pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                pPlayer->CLOSE_GOSSIP_MENU();
                break;
            default:
                break;
        }
        return true;
    }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_brann_bronzebeardAI(pCreature);
    }
};

class npc_tower_ulduar : public CreatureScript
{
public:
    npc_tower_ulduar() : CreatureScript("npc_tower_ulduar") { }

    struct npc_tower_ulduarAI : public Scripted_NoMovementAI
    {
        npc_tower_ulduarAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        GameObject *portal;
        bool portalDead;
        uint32 mui_go_state;

        void Reset()
        {
            portalDead = false;
            mui_go_state = 1000;
            if ((portal = me->FindNearestGameObject(194415, 3.0f)) != NULL)
                return;
            portal = me->SummonGameObject(194415, me->GetPositionX(),  me->GetPositionY(),  me->GetPositionZ(),  me->GetOrientation(), 0, 0, 0, 0, 7*DAY);

            for (int i = 0; i < RAID_MODE(12, 15); i++)
                DoSumAdd();
        }

        void DoSumAdd()
        {
            if (me->GetPositionY() > -40)
                me->SummonCreature(33236, CalculateRandomLocation(me->GetPositionX() -20, 20), CalculateRandomLocation(me->GetPositionY() - 25, 20), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
            else
                me->SummonCreature(33236, CalculateRandomLocation(me->GetPositionX() - 20, 20), CalculateRandomLocation2(me->GetPositionY() + 25, 20), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
        }

        float CalculateRandomLocation(float Loc, uint32 radius)
        {
            float coord = Loc;
            coord -= rand()%radius;
            return coord;
        }

        float CalculateRandomLocation2(float Loc, uint32 radius)
        {
            float coord = Loc;
            coord += rand()%radius;
            return coord;
        }

        void DamageTaken(Unit* /*who*/, uint32 &damage)
        {
            damage = 0;
        }

        void SummonedCreatureDies(Creature* /*summon*/, Unit* /*killer*/)
        {
            if (!portalDead)
            {
                if (Creature *c = me->SummonCreature(33236, me->GetPositionX() -20, me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
                    c->AI()->DoZoneInCombat();
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (portalDead)
                return;
            if (mui_go_state <= diff)
            {
                if (portal->GetDestructibleState() == 2)
                    portalDead = true;
                mui_go_state = 1000;
            }
            else
                mui_go_state -= diff;
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_tower_ulduarAI(pCreature);
    }
};

class at_RX_214_repair_o_matic_station : public AreaTriggerScript
{
public:
    at_RX_214_repair_o_matic_station() : AreaTriggerScript("at_RX_214_repair_o_matic_station") {}

    bool OnTrigger(Player* player, const AreaTriggerEntry* /*at*/)
    {
        if (InstanceScript* instance = player->GetInstanceScript())
        {
            if (Creature* vehicle = player->GetVehicleCreatureBase())
            {
                if (!vehicle->HasAura(SPELL_AUTO_REPAIR))
                {
                    player->MonsterTextEmote(EMOTE_REPAIR, player->GetGUID(), true);
                    player->CastSpell(vehicle, SPELL_AUTO_REPAIR, true);
                    vehicle->SetFullHealth();
                    if (Creature* leviathan = ObjectAccessor::GetCreature(*player, instance->GetData64(BOSS_LEVIATHAN)))
                        leviathan->AI()->SetData(DATA_UNBROKEN, 0); // Unbroken failed
                }
            }
        }
        return true;
    }
};

class go_ulduar_tower : public GameObjectScript
{
public:
    go_ulduar_tower() : GameObjectScript("go_ulduar_tower") {}

    void OnDestroyed(GameObject* go, Player* /*player*/)
    {
        InstanceScript* instance = go->GetInstanceScript();
        if (!instance)
            return;

        switch (go->GetEntry())
        {
        case GO_TOWER_OF_STORMS:
            instance->ProcessEvent(go, EVENT_TOWER_OF_STORM_DESTROYED);
            break;
        case GO_TOWER_OF_FLAMES:
            instance->ProcessEvent(go, EVENT_TOWER_OF_FLAMES_DESTROYED);
            break;
        case GO_TOWER_OF_FROST:
            instance->ProcessEvent(go, EVENT_TOWER_OF_FROST_DESTROYED);
            break;
        case GO_TOWER_OF_LIFE:
            instance->ProcessEvent(go, EVENT_TOWER_OF_LIFE_DESTROYED);
            break;
        }

        Creature* trigger = go->FindNearestCreature(NPC_ULDUAR_GAUNTLET_GENERATOR, 15.0f, true);
        if (trigger)
            trigger->DisappearAndDie();
    }
};

/************************************************************************/
/*                          Achievements                                */
/************************************************************************/

class achievement_three_car_garage_demolisher : public AchievementCriteriaScript
{
public:
    achievement_three_car_garage_demolisher(const char* name) : AchievementCriteriaScript(name) {}

    bool OnCheck(Player* source, Unit* /*target*/)
    {
        if (Creature* vehicle = source->GetVehicleCreatureBase())
        {
            if (vehicle->GetEntry() == VEHICLE_DEMOLISHER)
                return true;
        }
        return false;
    }
};

class achievement_three_car_garage_chopper : public AchievementCriteriaScript
{
public:
    achievement_three_car_garage_chopper(const char* name) : AchievementCriteriaScript(name) {}

    bool OnCheck(Player* source, Unit* /*target*/)
    {
        if (Creature* vehicle = source->GetVehicleCreatureBase())
        {
            if (vehicle->GetEntry() == VEHICLE_CHOPPER)
                return true;
        }
        return false;
    }
};

class achievement_three_car_garage_siege : public AchievementCriteriaScript
{
public:
    achievement_three_car_garage_siege(const char* name) : AchievementCriteriaScript(name) {}

    bool OnCheck(Player* source, Unit* /*target*/)
    {
        if (Creature* vehicle = source->GetVehicleCreatureBase())
        {
            if (vehicle->GetEntry() == VEHICLE_SIEGE)
                return true;
        }
        return false;
    }
};

class achievement_shutout : public AchievementCriteriaScript
{
public:
    achievement_shutout(const char* name) : AchievementCriteriaScript(name) {}

    bool OnCheck(Player* /*source*/, Unit* target)
    {
        if (target)
            if (Creature* leviathan = target->ToCreature())
                return static_cast<bool>(leviathan->AI()->GetData(DATA_SHUTOUT));

        return false;
    }
};

class achievement_unbroken : public AchievementCriteriaScript
{
public:
    achievement_unbroken(const char* name) : AchievementCriteriaScript(name) {}

    bool OnCheck(Player* /*source*/, Unit* target)
    {
        if (target)
            if (InstanceScript* instance = target->GetInstanceScript())
                return static_cast<bool>(instance->GetData(DATA_UNBROKEN));

        return false;
    }
};

class achievement_orbital_bombardment : public AchievementCriteriaScript
{
public:
    achievement_orbital_bombardment(const char* name) : AchievementCriteriaScript(name) {}

    bool OnCheck(Player* /*source*/, Unit* target)
    {
        if (!target)
            return false;

        if (Creature* Leviathan = target->ToCreature())
            if (Leviathan->AI()->GetData(DATA_ORBIT_ACHIEVEMENTS) >= 1)
                return true;

        return false;
    }
};

class achievement_orbital_devastation : public AchievementCriteriaScript
{
public:
    achievement_orbital_devastation(const char* name) : AchievementCriteriaScript(name) {}

    bool OnCheck(Player* /*source*/, Unit* target)
    {
        if (!target)
            return false;

        if (Creature* Leviathan = target->ToCreature())
            if (Leviathan->AI()->GetData(DATA_ORBIT_ACHIEVEMENTS) >= 2)
                return true;

        return false;
    }
};

class achievement_nuked_from_orbit : public AchievementCriteriaScript
{
public:
    achievement_nuked_from_orbit(const char* name) : AchievementCriteriaScript(name) { }

    bool OnCheck(Player* /*source*/, Unit* target)
    {
        if (!target)
            return false;

        if (Creature* Leviathan = target->ToCreature())
            if (Leviathan->AI()->GetData(DATA_ORBIT_ACHIEVEMENTS) >= 3)
                return true;

        return false;
    }
};

class achievement_orbit_uary : public AchievementCriteriaScript
{
public:
    achievement_orbit_uary(const char* name) : AchievementCriteriaScript(name) {}

    bool OnCheck(Player* /*source*/, Unit* target)
    {
        if (!target)
            return false;

        if (Creature* Leviathan = target->ToCreature())
            if (Leviathan->AI()->GetData(DATA_ORBIT_ACHIEVEMENTS) == 4)
                return true;

        return false;
    }
};

/************************************************************************/
/*                              Spells                                  */
/************************************************************************/

class spell_anti_air_rocket : public SpellScriptLoader
{
    enum { NPC_WORLD_TRIGGER = 22515 };

public:
    spell_anti_air_rocket() : SpellScriptLoader("spell_anti_air_rocket") {}

    class spell_anti_air_rocket_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_anti_air_rocket_SpellScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_ANTI_AIR_ROCKET_DMG))
                return false;
            return true;
        }

        void HandleTriggerMissile(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            if (const WorldLocation* pos = GetExplTargetDest())
            {
                if (Creature* temp = GetCaster()->SummonCreature(NPC_WORLD_TRIGGER, *pos, TEMPSUMMON_TIMED_DESPAWN, 500))
                {
                    temp->SetReactState(REACT_PASSIVE);
                    temp->SetCanFly(true);
                    temp->SetVisible(false);
                    std::list<Creature*> list;
                    GetCreatureListWithEntryInGrid(list, GetCaster(), NPC_MECHANOLIFT, 100.0f);

                    while (!list.empty())
                    {
                        std::list<Creature*>::iterator itr = list.begin();
                        std::advance(itr, urand(0, list.size()-1));

                        if ((*itr)->IsInBetween(GetCaster(), temp, 10.0f))
                        {
                            GetCaster()->CastSpell((*itr)->GetPositionX(), (*itr)->GetPositionY(), (*itr)->GetPositionZ(), SPELL_ANTI_AIR_ROCKET_DMG, true);
                            return;
                        }
                        list.erase(itr);
                    }
                }
            }
        }

        void Register()
        {
            OnEffectHit += SpellEffectFn(spell_anti_air_rocket_SpellScript::HandleTriggerMissile, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_anti_air_rocket_SpellScript();
    }
};

struct FlameLeviathanPursuedTargetSelector
{
    bool operator() (WorldObject* u)
    {
        Unit *unit = u->ToUnit();

        if (!unit)
            return true;
        // check area
        if (unit->GetAreaId() != AREA_FORMATION_GROUNDS)
            return true;

        // ignore players loaded on leviathan seats
        if (unit->GetVehicleBase() && unit->GetVehicleBase()->GetEntry() == NPC_SEAT)
            return true;

        // if target is creature
        Creature* creatureTarget = unit->ToCreature();
        if (creatureTarget)
        {
            // needs to be one of the 3 vehicles
            if (creatureTarget->GetEntry() != VEHICLE_DEMOLISHER &&
                creatureTarget->GetEntry() != VEHICLE_SIEGE &&
                creatureTarget->GetEntry() != VEHICLE_CHOPPER)
                return true;

            // must be a valid vehicle installation
            Vehicle* vehicle = creatureTarget->GetVehicleKit();
            if (!vehicle)
                return true;

            // vehicle must be in use by player
            bool playerFound = false;
            for (SeatMap::const_iterator itr = vehicle->Seats.begin(); itr != vehicle->Seats.end() && !playerFound; ++itr)
                if (IS_PLAYER_GUID(itr->second.Passenger.Guid))
                    playerFound = true;

            return !playerFound;
        }

        return false;
    }
};

class spell_pursued : public SpellScriptLoader
{
    public:
        spell_pursued() : SpellScriptLoader("spell_pursued") {}

        class spell_pursued_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pursued_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if(!sSpellMgr->GetSpellInfo(62374))
                    return false;
                return true;
            }

            bool Load()
            {
                _target = NULL;
                return true;
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                // Step 1: Remove all theoretical targets that cannot be our target
                targets.remove_if(FlameLeviathanPursuedTargetSelector());
                if (targets.empty())
                {
                    if (Creature* caster = GetCaster()->ToCreature())
                        caster->AI()->EnterEvadeMode();
                    return;
                }

                // Step 2: Try to find Demolisher or Siege Engine
                std::list<WorldObject*> tempList;
                // try to find demolisher or siege engine first (not the current target)
                for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end(); ++itr)
                {
                    if (*itr)
                        _target = (*itr)->ToUnit();

                    if (!_target)
                        continue;

                    if (!_target->ToCreature() || _target->HasAura(SPELL_PURSUED))
                        continue;

                    if (_target->ToCreature()->GetEntry() == VEHICLE_SIEGE || _target->ToCreature()->GetEntry() == VEHICLE_DEMOLISHER)
                        tempList.push_back(_target);
                }

                if (tempList.empty())
                {
                    // no demolisher or siege engine, find a chopper (not the current target)
                    for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end(); ++itr)
                    {
                        if (*itr)
                            _target = (*itr)->ToUnit();

                        if (!_target)
                            continue;

                        if (!_target->ToCreature() || _target->HasAura(SPELL_PURSUED))
                            continue;

                        if (_target->ToCreature()->GetEntry() == VEHICLE_CHOPPER)
                            tempList.push_back(_target);
                    }
                }

                if (!tempList.empty())
                {
                    _target = Trinity::Containers::SelectRandomContainerElement(tempList)->ToUnit();
                    SetSelectedTarget(tempList);
                }
                else
                {
                    // No vehicles found, select a random target
                    _target = Trinity::Containers::SelectRandomContainerElement(targets)->ToUnit();
                    SetSelectedTarget(targets);
                }
            }

            void SetSelectedTarget(std::list<WorldObject*>& targets)
            {
                targets.clear();
                if(_target)
                    targets.push_back(_target);
            }

            void HandleScript(SpellEffIndex /*eff*/)
            {
                Creature* caster = GetCaster()->ToCreature();
                if (!caster)
                    return;

                caster->AI()->AttackStart(GetHitUnit());    // Chase target

                /*0                for (SeatMap::const_iterator itr = caster->GetVehicleKit()->Seats.begin(); itr != caster->GetVehicleKit()->Seats.end(); ++itr)
                {
                    if (IS_PLAYER_GUID(itr->second.PassengerGUID))
                    {
                        caster->MonsterTextEmote(EMOTE_PURSUE, itr->second.Passenger, true);
                        return;
                    }
                    }*/
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pursued_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                //OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pursue_SpellScript::FilterTargetsSubsequently, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_pursued_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
            }

            Unit* _target;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pursued_SpellScript();
        }
};

// This spell break the ice tomb of SPELL_HODIRS_FURY
class spell_leviathan_blaze_flames_effect : public SpellScriptLoader
{
    public:
        spell_leviathan_blaze_flames_effect() : SpellScriptLoader("spell_leviathan_blaze_flames_effect") {}

        class spell_leviathan_blaze_flames_effect_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_leviathan_blaze_flames_effect_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if(!sSpellMgr->GetSpellInfo(65044))
                    return false;
                return true;
            }

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                if (GetHitUnit()->HasAura(SPELL_HODIRS_FURY_DAMAGE))
                    GetHitUnit()->RemoveAura(SPELL_HODIRS_FURY_DAMAGE);
            }

            void SetDamage()
            {
                PreventHitDamage();
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_leviathan_blaze_flames_effect_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
                OnHit += SpellHitFn(spell_leviathan_blaze_flames_effect_SpellScript::SetDamage);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_leviathan_blaze_flames_effect_SpellScript();
        }
};

class spell_load_into_catapult : public SpellScriptLoader
{
    enum { SPELL_PASSENGER_LOADED = 62340 };

    public:
        spell_load_into_catapult() : SpellScriptLoader("spell_load_into_catapult") {}

        class spell_load_into_catapult_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_load_into_catapult_AuraScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if(!sSpellMgr->GetSpellInfo(64414))
                    return false;
                return true;
            }

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* ownerVehicle = GetUnitOwner();

                if (!ownerVehicle)
                    return;

                ownerVehicle->CastSpell(ownerVehicle, SPELL_PASSENGER_LOADED, true);
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* ownerVehicle = GetUnitOwner();

                if (!ownerVehicle)
                    return;

                ownerVehicle->RemoveAurasDueToSpell(SPELL_PASSENGER_LOADED);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_load_into_catapult_AuraScript::OnApply, EFFECT_0, SPELL_AURA_CONTROL_VEHICLE, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_load_into_catapult_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_CONTROL_VEHICLE, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_load_into_catapult_AuraScript();
        }
};

class spell_auto_repair : public SpellScriptLoader
{
    enum { SPELL_AUTO_REPAIR = 62705 };

    public:
        spell_auto_repair() : SpellScriptLoader("spell_auto_repair") {}

        class spell_auto_repair_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_auto_repair_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if(!sSpellMgr->GetSpellInfo(62705))
                    return false;
                return true;
            }

            void CheckCooldownForTarget()
            {
                if (!GetHitUnit()->HasAura(SPELL_AUTO_REPAIR) && GetHitUnit()->HasAuraEffect(SPELL_AUTO_REPAIR, EFFECT_2))   // Check presence of dummy aura indicating cooldown
                {
                    PreventHitEffect(EFFECT_0);
                    PreventHitDefaultEffect(EFFECT_1);
                    PreventHitDefaultEffect(EFFECT_2);
                    //! Currently this doesn't work: if we call PreventHitAura(), the existing aura will be removed
                    //! because of recent aura refreshing changes. Since removing the existing aura negates the idea
                    //! of a cooldown marker, we just let the dummy aura refresh itself without executing the other spelleffects.
                    //! The spelleffects can be executed by letting the dummy aura expire naturally.
                    //! This is a temporary solution only.
                    //PreventHitAura();
                }
            }

            void HandleScript(SpellEffIndex /*eff*/)
            {
                Vehicle* vehicle = GetHitUnit()->GetVehicleKit();
                if (!vehicle)
                    return;
                if (GetHitUnit()->HasAura(SPELL_AUTO_REPAIR))
                    return;

                Player* driver = vehicle->GetPassenger(0) ? vehicle->GetPassenger(0)->ToPlayer() : NULL;
                if (!driver)
                    return;

                driver->MonsterTextEmote(EMOTE_REPAIR, driver->GetGUID(), true);

                InstanceScript* instance = driver->GetInstanceScript();
                if (!instance)
                    return;

                // Actually should/could use basepoints (100) for this spell effect as percentage of health, but oh well.
                vehicle->GetBase()->SetFullHealth();

                // For achievement
                instance->SetData(DATA_UNBROKEN, 0);

            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_auto_repair_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
                BeforeHit += SpellHitFn(spell_auto_repair_SpellScript::CheckCooldownForTarget);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_auto_repair_SpellScript();
        }
};

class spell_systems_shutdown : public SpellScriptLoader
{
    public:
        spell_systems_shutdown() : SpellScriptLoader("spell_systems_shutdown") {}

        class spell_systems_shutdown_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_systems_shutdown_AuraScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if(!sSpellMgr->GetSpellInfo(62475))
                    return false;
                return true;
            }

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Creature* owner = GetOwner()->ToCreature();
                if (!owner)
                    return;

                //! This could probably in the SPELL_EFFECT_SEND_EVENT handler too:
                owner->AddUnitState(UNIT_STATE_STUNNED | UNIT_STATE_ROOT);
                owner->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
                owner->RemoveAurasDueToSpell(SPELL_GATHERING_SPEED);
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Creature* owner = GetOwner()->ToCreature();
                if (!owner)
                    return;

                owner->ClearUnitState(UNIT_STATE_STUNNED | UNIT_STATE_ROOT);
                owner->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_systems_shutdown_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_systems_shutdown_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_systems_shutdown_AuraScript();
        }
};

class spell_vehicle_throw_passenger : public SpellScriptLoader
{
    public:
        spell_vehicle_throw_passenger() : SpellScriptLoader("spell_vehicle_throw_passenger") {}

        class spell_vehicle_throw_passenger_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_vehicle_throw_passenger_SpellScript);

            void HandleDummyHit(SpellEffIndex /*effIndex*/)
            {
                Spell* baseSpell = GetSpell();
                SpellCastTargets targets = baseSpell->m_targets;
                int32 damage = GetEffectValue();
                if (targets.HasTraj())
                {
                    if (Vehicle* vehicle = GetCaster()->GetVehicleKit())
                        if (Unit* passenger = vehicle->GetPassenger(damage - 1))
                        {
                            std::list<Creature* > unitList;
                            Position pos;
                            GetCaster()->GetPosition(&pos);
                            // use 99 because it is 3d search
                            GetCaster()->GetCreatureListWithEntryInGrid(unitList, (uint32)NPC_SEAT, 99.0f);
                            //                            if (Spell* spell = GetSpell())
                            //    spell->SearchAreaTargets(unitList, 99, &pos, GetCaster(), TARGET_OBJECT_TYPE_DEST, TARGET_CHECK_ENTRY, NPC_SEAT);
                            float minDist = 99 * 99;
                            Unit* target = NULL;
                            for (std::list<Creature* >::iterator itr = unitList.begin(); itr != unitList.end(); ++itr)
                            {
                                if (Unit* unit = (*itr)->ToUnit())
                                    if (unit->GetEntry() == NPC_SEAT)
                                        if (Vehicle* seat = unit->GetVehicleKit())
                                        {
                                            if (!seat->GetPassenger(0))
                                            {
                                                if (Unit* device = seat->GetPassenger(2))
                                                {
                                                    if (!device->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                                                    {
                                                        Position pos;
                                                        targets.GetDstPos()->GetPosition(pos.m_positionX, pos.m_positionY, pos.m_positionZ);
                                                        float dist = unit->GetExactDistSq(pos.m_positionX, pos.m_positionY, pos.m_positionZ);
                                                        if (dist < minDist)
                                                        {
                                                            minDist = dist;
                                                            target = unit;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                            }
                            if (target)
                            {
                                passenger->ExitVehicle();
                                passenger->EnterVehicle(target, SEAT_PLAYER);
                                //                                passenger->ClearUnitState(UNIT_STATE_ONVEHICLE);
                            }
                            else
                            {
                                passenger->ExitVehicle();
                                float x, y, z;
                                targets.GetDstPos()->GetPosition(x, y, z);
                                passenger->GetMotionMaster()->MoveJump(x, y, z, targets.GetSpeedXY(), targets.GetSpeedZ());
                            }
                        }
                }
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_vehicle_throw_passenger_SpellScript::HandleDummyHit, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_vehicle_throw_passenger_SpellScript();
        }
};

class spell_freyas_ward_summon : public SpellScriptLoader
{
    public:
        spell_freyas_ward_summon() : SpellScriptLoader("spell_freyas_ward_summon") {}

        class spell_freyas_ward_summon_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_freyas_ward_summon_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if(!sSpellMgr->GetSpellInfo(62907))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);

                if (Unit* caster = GetCaster())
                    if (InstanceScript* instance = caster->GetInstanceScript())
                        if (Creature* leviathan = ObjectAccessor::GetCreature(*caster, instance->GetData64(BOSS_LEVIATHAN)))
                            for (uint8 i = 0; i < urand(3, 5); ++i)
                            {
                                Position pos;
                                leviathan->GetPosition(&pos);
                                leviathan->GetRandomNearPosition(pos, float(urand(5, 80)));
                                leviathan->SummonCreature(NPC_WRITHING_LASHER,pos,TEMPSUMMON_CORPSE_DESPAWN, 3000);
                            }

            }

            void HandleSummon(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);

                if (Unit* caster = GetCaster())
                    if (InstanceScript* instance = caster->GetInstanceScript())
                        if (Creature* leviathan = ObjectAccessor::GetCreature(*caster, instance->GetData64(BOSS_LEVIATHAN)))
                            leviathan->SummonCreature(NPC_WARD_OF_LIFE, GetExplTargetDest()->GetPositionX(), GetExplTargetDest()->GetPositionY(),
                            GetExplTargetDest()->GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 3000);
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_freyas_ward_summon_SpellScript::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
                OnEffectHit += SpellEffectFn(spell_freyas_ward_summon_SpellScript::HandleSummon, EFFECT_2, SPELL_EFFECT_SUMMON);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_freyas_ward_summon_SpellScript();
        }
};

struct FlameVentsTargetSelector
{
    bool operator() (WorldObject* unit)
    {
        if (unit->GetTypeId() != TYPEID_PLAYER)
        {
            if (unit->ToCreature()->GetEntry() == VEHICLE_SIEGE ||
                unit->ToCreature()->GetEntry() == VEHICLE_CHOPPER ||
                unit->ToCreature()->GetEntry() == VEHICLE_DEMOLISHER)
                return false;

            if (!unit->ToCreature()->isPet())
                return true;
        }

        return unit->ToUnit() && unit->ToUnit()->GetVehicle();
    }
};

class spell_leviathan_flame_vents_triggered : public SpellScriptLoader
{
    public:
        spell_leviathan_flame_vents_triggered() : SpellScriptLoader("spell_leviathan_flame_vents") { }

        class spell_leviathan_flame_vents_triggered_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_leviathan_flame_vents_triggered_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if(!sSpellMgr->GetSpellInfo(63847))
                    return false;
                return true;
            }

            // Filter has to be applied since the spell should only affect vehicles
            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                unitList.remove_if(FlameVentsTargetSelector());
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_leviathan_flame_vents_triggered_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_leviathan_flame_vents_triggered_SpellScript();
        }
};

class spell_shield_generator : public SpellScriptLoader
{
    private:
        enum { SPELL_SHIELD_GENERATOR = 64677 };
    public:
        spell_shield_generator() : SpellScriptLoader("spell_shield_generator") { }

        class spell_shield_generator_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_shield_generator_AuraScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if(!sSpellMgr->GetSpellInfo(SPELL_SHIELD_GENERATOR))
                    return false;
                return true;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                    if (Unit* siege = caster->GetVehicleBase())
                        amount = int32(siege->CountPctFromMaxHealth(15));
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_shield_generator_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_shield_generator_AuraScript();
        }
};

void AddSC_boss_flame_leviathan()
{
    new boss_flame_leviathan();                     // 10: 33113, 25: 34003 ; difficulty entry settled properly
    new npc_flame_leviathan_seat();                 // 33114
    new npc_flame_leviathan_defense_turret();       // 33142
    new npc_flame_leviathan_defense_cannon();       // 33139
    new npc_flame_leviathan_overload_device();      // 33143
    new npc_flame_leviathan_safety_container();     // 33218
    new npc_mechanolift();                          // 33214
    new npc_liquid_pyrite();                        // 33189
    new npc_pool_of_tar();                          // 33090
    new npc_colossus();                             // 10: 33237, 25: 34105 ; difficulty entry settled properly
    new npc_thorims_hammer();                       // 33365
    new npc_mimirons_inferno();                     // 33370
    new npc_hodirs_fury();                          // 33212
    new npc_freyas_ward();                          // 33367
    new npc_freya_ward_of_life();                   // 34275
    new npc_leviathan_player_vehicle();

    new npc_lorekeeper();                           // 33686
    new npc_brann_bronzebeard();
    new npc_tower_ulduar();
    new npc_brann_bronzebeard_outro();

    new at_RX_214_repair_o_matic_station();         // Area-Trigger 5369/5423
    new go_ulduar_tower();                          // 194375: Tower of Life; 194371: Tower of Flames; 194370: Tower of Frost; 194377: Tower of Storms

    // TODO: Those have to be copied for 10/25 man handling, since ScriptNames are selected distinct...
    // @see ObjectMgr.cpp @ 8602
    new achievement_three_car_garage_demolisher("achievement_three_car_garage_demolisher");     // 10048
    new achievement_three_car_garage_demolisher("achievement_three_car_garage_demolisher_25");  // 10051
    new achievement_three_car_garage_chopper("achievement_three_car_garage_chopper");           // 10046
    new achievement_three_car_garage_chopper("achievement_three_car_garage_chopper_25");        // 10049
    new achievement_three_car_garage_siege("achievement_three_car_garage_siege");               // 10047
    new achievement_three_car_garage_siege("achievement_three_car_garage_siege_25");            // 10050
    new achievement_shutout("achievement_shutout");                                             // 10054
    new achievement_shutout("achievement_shutout_25");                                          // 10055
    new achievement_unbroken("achievement_unbroken");                                           // 10044
    new achievement_unbroken("achievement_unbroken_25");                                        // 10045
    new achievement_orbital_bombardment("achievement_orbital_bombardment");                     // 10056
    new achievement_orbital_bombardment("achievement_orbital_bombardment_25");                  // 10059
    new achievement_orbital_devastation("achievement_orbital_devastation");                     // 10057
    new achievement_orbital_devastation("achievement_orbital_devastation_25");                  // 10060
    new achievement_nuked_from_orbit("achievement_nuked_from_orbit");                           // 10058
    new achievement_nuked_from_orbit("achievement_nuked_from_orbit_25");                        // 10061
    new achievement_orbit_uary("achievement_orbit_uary");                                       // 10218
    new achievement_orbit_uary("achievement_orbit_uary_25");                                    // 10219

    new spell_anti_air_rocket();                    // 64979
    new spell_pursued();                            // 62374
    new spell_leviathan_blaze_flames_effect();      // 65044
    new spell_load_into_catapult();                 // 64414
    new spell_auto_repair();                        // 62705
    new spell_systems_shutdown();                   // 62475
    new spell_vehicle_throw_passenger();            // 62324
    new spell_freyas_ward_summon();                 // 62907
    new spell_leviathan_flame_vents_triggered();    // 63847
    new spell_shield_generator();                   // 64677
}
