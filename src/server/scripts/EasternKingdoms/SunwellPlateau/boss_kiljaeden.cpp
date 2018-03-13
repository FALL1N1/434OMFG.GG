/*
 * Copyright (C) 2013-2014 OMFG.GG Network <http://www.omfg.gg/>
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
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

/* ScriptData
SDName: Boss_Kiljaeden
SD%Complete: 80
SDComment: Sinister Reflection Model, Armageddon Visual, SAY_KJ_SHADOWSPIKE3, Emote, End Sequence
SDCategory: Sunwell_Plateau
EndScriptData */

//TODO rewrite Armageddon
#include "Group.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellScript.h"
#include "sunwell_plateau.h"

#define GOSSIP_ITEM_LEADER    "Kil'jaeden in achtziger Modus schalten"
#define GOSSIP_ITEM_PLAYER    "Teleport zum Sonnenbrunnen"

/*** Speech and sounds***/
enum Yells
{
    SAY_KJ_OFFCOMBAT                            = 0,

    SAY_KALECGOS_ENCOURAGE                      = 0,
    SAY_KALECGOS_READY1                         = 1,
    SAY_KALECGOS_READY2                         = 2,
    SAY_KALECGOS_READY3                         = 3,
    SAY_KALECGOS_READY4                         = 4,
    SAY_KALECGOS_AWAKEN                         = 5,
    SAY_KALECGOS_LETGO                          = 6,
    SAY_KALECGOS_FOCUS                          = 7,
    SAY_KALECGOS_FATE                           = 8,
    SAY_KALECGOS_GOODBYE                        = 9,
    SAY_KALECGOS_JOIN                           = 10,

    SAY_KJ_DEATH                                = 0,
    SAY_KJ_SLAY                                 = 1,
    SAY_KJ_REFLECTION                           = 2,
    SAY_KJ_EMERGE                               = 3,
    SAY_KJ_DARKNESS                             = 4,
    SAY_KJ_PHASE3                               = 5,
    SAY_KJ_PHASE4                               = 6,
    SAY_KJ_PHASE5                               = 7,
    EMOTE_KJ_DARKNESS                           = 8,

    SAY_ANVEENA_IMPRISONED                      = 0,
    SAY_ANVEENA_LOST                            = 1,
    SAY_ANVEENA_KALEC                           = 2,
    SAY_ANVEENA_GOODBYE                         = 3,
};

/*** Spells used during the encounter ***/
enum Spells
{
    /* Hand of the Deceiver's spells and cosmetics */
    SPELL_SHADOW_BOLT_VOLLEY                    = 45770, // ~30 yard range Shadow Bolt Volley for ~2k(?) damage
    SPELL_SHADOW_INFUSION                       = 45772, // They gain this at 20% - Immunity to Stun/Silence and makes them look angry!
    SPELL_FELFIRE_PORTAL                        = 46875, // Creates a portal that spawns Felfire Fiends (LIVE FOR THE SWARM!1 FOR THE OVERMIND!)
    SPELL_SHADOW_CHANNELING                     = 46757, // Channeling animation out of combat

    /* Volatile Felfire Fiend's spells */
    SPELL_FELFIRE_FISSION                       = 45779, // Felfire Fiends explode when they die or get close to target.

    /* Kil'Jaeden's spells and cosmetics */
    SPELL_TRANS                                 = 23188, // Surprisingly, this seems to be the right spell.. (Where is it used?)
    SPELL_REBIRTH                               = 44200, // Emerge from the Sunwell
    SPELL_SOUL_FLAY                             = 45442, // 9k Shadow damage over 3 seconds. Spammed throughout all the fight.
    SPELL_SOUL_FLAY_SLOW                        = 47106,
    SPELL_LEGION_LIGHTNING                      = 45664, // Chain Lightning, 4 targets, ~3k Shadow damage, 1.5fk mana burn
    SPELL_FIRE_BLOOM                            = 45641, // Places a debuff on 5 raid members, which causes them to deal 2k Fire damage to nearby allies and selves. MIGHT NOT WORK
    SPELL_DESTROY_ALL_DRAKES                    = 46707, // when he use it?
    SPELL_CUSTOM_08_STATE                       = 45800,

    SPELL_SINISTER_REFLECTION                   = 45785, // Summon shadow copies of 5 raid members that fight against KJ's enemies//dont work
                                                //  45892  // right one for SPELL_SINISTER_REFLECTION but no EffectScriptEffect
    SPELL_COPY_WEAPON                           = 41055, // }
    SPELL_COPY_WEAPON2                          = 41054, // }
    SPELL_COPY_OFFHAND                          = 45206, // }- Spells used in Sinister Reflection creation
    SPELL_COPY_OFFHAND_WEAPON                   = 45205, // }
    SPELL_COPY_RANGED_WEAPON                    = 57593,

    SPELL_SHADOW_SPIKE                          = 46680, // Bombard random raid members with Shadow Spikes (Very similar to Void Reaver orbs)
    SPELL_FLAME_DART                            = 45737, // Bombards the raid with flames every 3(?) seconds
    SPELL_DARKNESS_OF_A_THOUSAND_SOULS          = 46605, // Begins a 8-second channeling, after which he will deal 50'000 damage to the raid
    SPELL_DARKNESS_OF_A_THOUSAND_SOULS_DAMAGE   = 45657,

    /* Armageddon spells wrong visual */
    SPELL_ARMAGEDDON_TRIGGER                    = 45909, // Meteor spell trigger missile should cast Creature on himself
    SPELL_ARMAGEDDON_VISUAL                     = 45911, // Does the hellfire visual to indicate where the meteor missle lands
    SPELL_ARMAGEDDON_VISUAL2                    = 45914, // Does the light visual to indicate where the meteor missle lands
    SPELL_ARMAGEDDON_VISUAL3                    = 24207, // This shouldn't correct but same as seen on the movie
    SPELL_ARMAGEDDON_SUMMON_TRIGGER             = 45921, // Summons the triggers that cast the spells on himself need random target select
    SPELL_ARMAGEDDON_DAMAGE                     = 45915, // This does the area damage

    /* Shield Orb Spells*/
    SPELL_SHADOW_BOLT                           = 45679, //45680,45679 would be correct but triggers to often //TODO fix console error

    /* Anveena's spells and cosmetics (Or, generally, everything that has "Anveena" in name) */
    SPELL_ANVEENA_PRISON                        = 46367, // She hovers locked within a bubble
    SPELL_ANVEENA_ENERGY_DRAIN                  = 46410, // Sunwell energy glow animation (Control mob uses this)
    SPELL_SACRIFICE_OF_ANVEENA                  = 46474, // This is cast on Kil'Jaeden when Anveena sacrifices herself into the Sunwell

    /* Sinister Reflection Spells */
    SPELL_SR_CURSE_OF_AGONY                     = 46190,
    SPELL_SR_SHADOW_BOLT                        = 47076,

    SPELL_SR_EARTH_SHOCK                        = 47071,

    SPELL_SR_FIREBALL                           = 47074,

    SPELL_SR_HEMORRHAGE                         = 45897,

    SPELL_SR_HOLY_SHOCK                         = 38921,
    SPELL_SR_HAMMER_OF_JUSTICE                  = 37369,

    SPELL_SR_HOLY_SMITE                         = 47077,
    SPELL_SR_RENEW                              = 47079,

    SPELL_SR_SHOOT                              = 16496,
    SPELL_SR_MULTI_SHOT                         = 48098,
    SPELL_SR_WING_CLIP                          = 40652,

    SPELL_SR_WHIRLWIND                          = 17207,

    SPELL_SR_MOONFIRE                           = 47072,
    SPELL_SR_PLAGUE_STRIKE                      = 58843, //Dk Spell!

    /*** Other Spells (used by players, etc) ***/
    SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT          = 45839,    // Possess the blue dragon from the orb to help the raid.
    SPELL_POSSESS_DRAKE_IMMUNE                  = 45838,    // sets drake unattackable
    SPELL_ENTROPIUS_BODY                        = 46819,    // Visual for Entropius at the Epilogue
    SPELL_RING_OF_BLUE_FLAMES                   = 45825,    // Cast this spell when the go is activated
    SPELL_SUMMON_BLUE_DRAKE                     = 45836,    // Summon Blue Drake
    SPELL_POWER_OF_THE_BLUE_FLIGHT              = 45833,    // spell casted by orb

    SPELL_REVITALIZE                            = 45860     // casted by power of blue flight
};

/*** Error messages ***/
#define ERROR_KJ_NOT_SUMMONED "TSCR ERROR: Unable to summon Kil'Jaeden for some reason"

/*** Others ***/
#define SHIELD_ORB_Z        40.000f
#define SHIELD_ORB_RADIUS   17.0f

enum Phase
{
    PHASE_DECEIVERS     = 1, // Fight 3 adds
    PHASE_NORMAL        = 2, // Kil'Jaeden emerges from the sunwell
    PHASE_DARKNESS      = 3, // At 85%, he gains few abilities; Kalecgos joins the fight
    PHASE_ARMAGEDDON    = 4, // At 55%, he gains even more abilities
    PHASE_SACRIFICE     = 5, // At 25%, Anveena sacrifices herself into the Sunwell; at this point he becomes enraged and has *significally* shorter cooldowns.
};

//Timers
enum KilJaedenTimers
{
    TIMER_SPEECH           = 0,

    //Phase 2 Timer
    TIMER_SOUL_FLAY        = 1,
    TIMER_LEGION_LIGHTNING = 2,
    TIMER_FIRE_BLOOM       = 3,
    TIMER_SUMMON_SHIELDORB = 4,

    //Phase 3 Timer
    TIMER_SHADOW_SPIKE     = 5,
    TIMER_FLAME_DART       = 6,
    TIMER_DARKNESS         = 7,
    TIMER_ORBS_EMPOWER     = 8,

    //Phase 4 Timer
    TIMER_ARMAGEDDON       = 9
};

// Locations of the Hand of Deceiver adds
Position DeceiverLocations[3]=
{
    {1684.7f, 614.42f, 28.058f, 0.0f},
    {1682.95f, 637.75f, 27.9231f, 0.0f},
    {1707.61f, 612.15f, 27.7946f, 0.0f},
};

float middleOfSunwell[2] = {1698.900f, 627.870f};  //center of Sunwell

struct Speech
{
    int32 textid;
    uint32 creatureData, timer;
};

// Timers
static Speech speeches[]=
{
    //Kil Phase 1 -> Phase 2
    {SAY_KJ_EMERGE,             DATA_KILJAEDEN,     0},
    {SAY_KALECGOS_JOIN,         DATA_KALECGOS_KJ,   26000},
    //Kil Phase 2 -> Phase 3
    {SAY_KALECGOS_AWAKEN,       DATA_KALECGOS_KJ,   10000},
    {SAY_ANVEENA_IMPRISONED,    DATA_ANVEENA,       5000},
    {SAY_KJ_PHASE3,             DATA_KILJAEDEN,     5000},
    //Kil Phase 3 -> Phase 4
    {SAY_KALECGOS_LETGO,        DATA_KALECGOS_KJ,   10000},
    {SAY_ANVEENA_LOST,          DATA_ANVEENA,       8000},
    {SAY_KJ_PHASE4,             DATA_KILJAEDEN,     7000},
    //Kil Phase 4 -> Phase 5
    {SAY_KALECGOS_FOCUS,        DATA_KALECGOS_KJ,   4000},
    {SAY_ANVEENA_KALEC,         DATA_ANVEENA,       11000},
    {SAY_KALECGOS_FATE,         DATA_KALECGOS_KJ,   2000},
    {SAY_ANVEENA_GOODBYE,       DATA_ANVEENA,       6000},
    {SAY_KJ_PHASE5,             DATA_KILJAEDEN,     5500},

    // use in End sequence?
    {SAY_KALECGOS_GOODBYE,      DATA_KALECGOS_KJ,   12000},
};

// predicate function to select target, player not use dragon
struct NoDragonTargetSelector : public std::unary_function<Unit *, bool> {
    NoDragonTargetSelector() {}

    bool operator() (Unit const* target) const
    {
        return (target->GetTypeId() == TYPEID_PLAYER && !target->HasAura(SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT) && target->isAlive());
    }
};
//AI for Kalecgos
class boss_kalecgos_kj : public CreatureScript
{
public:
    boss_kalecgos_kj() : CreatureScript("boss_kalecgos_kj") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_kalecgos_kjAI (creature);
    }

    struct boss_kalecgos_kjAI : public ScriptedAI
    {
        boss_kalecgos_kjAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        uint8 OrbsEmpowered;
        uint8 EmpowerCount;
        bool firstOrbEmpowered;

        void Reset()
        {
            if (instance && instance->GetData(DATA_MODUS80))
            {
                me->SetLevel(83);
                me->SetMaxHealth(10000000);
                me->SetHealth(me->GetMaxHealth());
            }
            firstOrbEmpowered = false;
            me->SetWalk(true);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            OrbsEmpowered = 0;
            EmpowerCount = 0;
            me->SetDisableGravity(true);
            me->SetHover(true);
            //            me->AddUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->setActive(true);

            for (uint8 i = 0; i < 4; ++i)
                if (GameObject* pOrb = GetOrb(i))
                    pOrb->SetGoType(GAMEOBJECT_TYPE_BUTTON);
        }

        GameObject* GetOrb(int32 index)
        {
            if (!instance)
                return NULL;

            switch(index)
            {
                case 0:
                    return instance->instance->GetGameObject(instance->GetData64(DATA_ORB_OF_THE_BLUE_FLIGHT_1));
                case 1:
                    return instance->instance->GetGameObject(instance->GetData64(DATA_ORB_OF_THE_BLUE_FLIGHT_2));
                case 2:
                    return instance->instance->GetGameObject(instance->GetData64(DATA_ORB_OF_THE_BLUE_FLIGHT_3));
                case 3:
                    return instance->instance->GetGameObject(instance->GetData64(DATA_ORB_OF_THE_BLUE_FLIGHT_4));
            }
            return NULL;
        }

        void ResetOrbs()
        {
            me->RemoveDynObject(SPELL_RING_OF_BLUE_FLAMES);
            for (uint8 i = 0; i < 4; ++i)
                if (GameObject* orb = GetOrb(i))
                {
                    orb->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    orb->ResetDoorOrButton();
                }
            firstOrbEmpowered = false;
        }

        void EmpowerOrb(bool all)
        {
            if (all)
            {
                me->RemoveDynObject(SPELL_RING_OF_BLUE_FLAMES);
                for (uint8 i = 0; i < 4; ++i)
                {
                    if (GameObject* orb = GetOrb(i))
                    {
                        orb->ResetDoorOrButton();
                        orb->CastSpell(me, SPELL_RING_OF_BLUE_FLAMES);
                        orb->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    }
                }
                Talk(SAY_KALECGOS_ENCOURAGE);
            }
            else
            {
                std::set<uint8> orbs;
                for (uint8 i = 0; i < 4; ++i)
                {
                    if (GameObject* orb = GetOrb(i))
                    {
                        if (orb->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE))
                            orbs.insert(i);
                    }
                }
                if (!orbs.empty())
                {
                    std::set<uint8>::iterator itr = orbs.begin();
                    std::advance(itr, urand(0, orbs.size() - 1));

                    if (GameObject* orb = GetOrb(*itr))
                    {
                        orb->ResetDoorOrButton();
                        orb->CastSpell(me, SPELL_RING_OF_BLUE_FLAMES);
                        orb->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);

                        if (firstOrbEmpowered)
                            Talk(SAY_KALECGOS_READY2);
                        else
                        {
                            Talk(SAY_KALECGOS_READY1);
                            firstOrbEmpowered = true;
                        }
                    }
                }
            }
        }

        void SetRingOfBlueFlames()
        {
            me->RemoveDynObject(SPELL_RING_OF_BLUE_FLAMES);
            for (uint8 i = 0; i < 4; ++i)
            {
                if (GameObject* orb = GetOrb(i))
                {
                    if (!orb->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE))
                        orb->CastSpell(me, SPELL_RING_OF_BLUE_FLAMES);
                }
            }
        }
        void UpdateAI(const uint32 /*diff*/)
        {}
    };

};

class go_orb_of_the_blue_flight : public GameObjectScript
{
public:
    go_orb_of_the_blue_flight() : GameObjectScript("go_orb_of_the_blue_flight") { }

    bool OnGossipHello(Player* player, GameObject* pGo)
    {
        if (!pGo || !player)
            return false;

        InstanceScript* instance = pGo->GetInstanceScript();

        if (Creature* summon = player->SummonCreature(CREATURE_POWER_OF_THE_BLUE_DRAGONFLIGHT, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 121000))
        {
            if (instance && instance->GetData(DATA_MODUS80))
            {
                summon->SetMaxHealth(978000);
            }
            else
            {
                summon->SetMaxHealth(97800);  //value from video
            }
            summon->SetLevel(player->getLevel());
            summon->SetHealth(summon->GetMaxHealth());
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);//can't be targeted
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);//can't be damaged
            player->CastSpell(player, SPELL_POSSESS_DRAKE_IMMUNE, true);
            summon->CastSpell(summon, SPELL_POSSESS_DRAKE_IMMUNE, true);
            player->CastSpell(summon, SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT, true);
       }
        pGo->UseDoorOrButton(3000);
        pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);

        if (instance)
            if (Creature* kalec = Unit::GetCreature(*player, instance->GetData64(DATA_KALECGOS_KJ)))
                CAST_AI(boss_kalecgos_kj::boss_kalecgos_kjAI, kalec->AI())->SetRingOfBlueFlames();

        return true;
    }

};

//AI for Kil'jaeden Event Controller
class mob_kiljaeden_controller : public CreatureScript
{
public:
    mob_kiljaeden_controller() : CreatureScript("mob_kiljaeden_controller") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_kiljaeden_controllerAI (creature);
    }

    struct mob_kiljaeden_controllerAI : public Scripted_NoMovementAI
    {
        mob_kiljaeden_controllerAI(Creature* creature) : Scripted_NoMovementAI(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        SummonList summons;

        bool bSummonedDeceivers;
        bool bKiljaedenDeath;

        uint32 uiRandomSayTimer;
        uint32 phase;
        uint8 deceiverDeathCount;

        void InitializeAI()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            me->SetReactState(REACT_PASSIVE);

            ScriptedAI::InitializeAI();
        }

        void Reset()
        {
            phase = PHASE_DECEIVERS;

            if (instance)
                if (Creature* kalecKJ = Unit::GetCreature((*me), instance->GetData64(DATA_KALECGOS_KJ)))
                    CAST_AI(boss_kalecgos_kj::boss_kalecgos_kjAI, kalecKJ->AI())->ResetOrbs();
            deceiverDeathCount = 0;
            bSummonedDeceivers = false;
            bKiljaedenDeath = false;
            uiRandomSayTimer = 30000;
            summons.DespawnAll();
        }

        void JustSummoned(Creature* summoned)
        {
            switch(summoned->GetEntry())
            {
                case CREATURE_HAND_OF_THE_DECEIVER:
                    summoned->CastSpell(summoned, SPELL_SHADOW_CHANNELING, false);
                    break;
                case CREATURE_ANVEENA:
                    summoned->SetDisableGravity(true);
                    summoned->SetHover(true);
                    //                    summoned->AddUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
                    summoned->CastSpell(summoned, SPELL_ANVEENA_PRISON, true);
                    summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    summoned->SetReactState(REACT_PASSIVE);
                    break;
                case CREATURE_KILJAEDEN:
                    summoned->CastSpell(summoned, SPELL_REBIRTH, false);
                    break;
            }
            summons.Summon(summoned);
        }

        void UpdateAI(const uint32 diff)
        {
            if (uiRandomSayTimer < diff)
            {
                if (instance && (instance->GetData(DATA_MURU_EVENT) != DONE) && (instance->GetData(DATA_KILJAEDEN_EVENT) == NOT_STARTED) && !instance->IsEncounterInProgress())
                    Talk(SAY_KJ_OFFCOMBAT);
                uiRandomSayTimer = 30000;
            } else uiRandomSayTimer -= diff;

            if (!bSummonedDeceivers)
            {
                for (uint8 i = 0; i < 3; ++i)
                    me->SummonCreature(CREATURE_HAND_OF_THE_DECEIVER, DeceiverLocations[i], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);

                DoSpawnCreature(CREATURE_ANVEENA,  0, 0, 40, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                DoCast(me, SPELL_ANVEENA_ENERGY_DRAIN);
                bSummonedDeceivers = true;
            }

            if (deceiverDeathCount > 2 && phase == PHASE_DECEIVERS)
            {
                me->RemoveAurasDueToSpell(SPELL_ANVEENA_ENERGY_DRAIN);
                phase = PHASE_NORMAL;
                DoSpawnCreature(CREATURE_KILJAEDEN, 0, 0,0, 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
            }
        }
    };

};


//AI for Kil'jaeden
class boss_kiljaeden : public CreatureScript
{
public:
    boss_kiljaeden() : CreatureScript("boss_kiljaeden") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_kiljaedenAI (creature);
    }

    struct boss_kiljaedenAI : public Scripted_NoMovementAI
    {
        boss_kiljaedenAI(Creature* creature) : Scripted_NoMovementAI(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        SummonList summons;

        uint8 phase;
        uint8 activeTimers;
        uint32 speechTimer;

        uint32 timer[10];
        uint32 waitTimer;
        uint8 speechCount;
        uint8 speechPhaseEnd;

        /* Boolean */
        bool isInDarkness;
        bool timerIsDeactivated[10];
        bool isWaiting;
        bool orbActivated;
        bool speechBegins;

        void Reset()
        {
            timerIsDeactivated[TIMER_SPEECH] = false;
            timer[TIMER_SPEECH]           = 0;

            //Phase 2 Timer
            timer[TIMER_SOUL_FLAY]        = 11000;
            timer[TIMER_LEGION_LIGHTNING] = 30000;
            timer[TIMER_FIRE_BLOOM]       = 20000;
            timer[TIMER_SUMMON_SHIELDORB] = 35000;

            //Phase 3 Timer
            timer[TIMER_SHADOW_SPIKE]     = 4000;
            timer[TIMER_FLAME_DART]       = 3000;
            timer[TIMER_DARKNESS]         = 45000;
            timer[TIMER_ORBS_EMPOWER]     = 35000;

            //Phase 4 Timer
            timer[TIMER_ARMAGEDDON]       = 2000;

            activeTimers = 5;
            waitTimer    = 0;
            speechCount = 0;
            speechTimer = 0;

            phase = PHASE_NORMAL;

            isInDarkness  = false;
            isWaiting     = false;
            orbActivated  = false;
            speechBegins  = true;

            if (instance)
            {
                if (Creature* kalec = Unit::GetCreature(*me, instance->GetData64(DATA_KALECGOS_KJ)))
                    kalec->RemoveDynObject(SPELL_RING_OF_BLUE_FLAMES);
            }
            if (instance && instance->GetData(DATA_MODUS80))
            {
              //  me->UpdateEntry(2300005, 0);

                me->SetLevel(83);
                if (CreatureTemplate const* cinfo = sObjectMgr->GetCreatureTemplate(CREATURE_KILJAEDEN))
                    me->SetMaxHealth(1000000 * cinfo->rangedattackpower);
                me->SetHealth(me->GetMaxHealth());
                me->SetMaxPower(POWER_MANA, 20000000);
                me->SetPower(POWER_MANA, me->GetMaxPower(POWER_MANA));
                me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1500);
                me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 2000);
                me->UpdateDamagePhysical(BASE_ATTACK);
                me->SetArmor(10573);
                me->SetLootMode(2);
            }
            me->SetFloatValue(UNIT_FIELD_COMBATREACH, 12);
            ChangeTimers(false, 0);
            summons.DespawnAll();
        }

        void ChangeTimers(bool status, uint32 WTimer)
        {
            for (uint8 i = 1; i < activeTimers; ++i)
                timerIsDeactivated[i] = status;

            if (WTimer > 0)
            {
                isWaiting = true;
                waitTimer = WTimer;
            }

            if (orbActivated)
                timerIsDeactivated[TIMER_ORBS_EMPOWER] = true;
            if (timer[TIMER_SHADOW_SPIKE] == 0)
                timerIsDeactivated[TIMER_SHADOW_SPIKE] = true;
            if (phase == PHASE_SACRIFICE)
                timerIsDeactivated[TIMER_SUMMON_SHIELDORB] = true;
        }

        void JustSummoned(Creature* summoned)
        {
            summoned->SetLevel(me->getLevel());
            summoned->setFaction(me->getFaction());
            summons.Summon(summoned);
        }

        void JustDied(Unit* /*killer*/)
        {
            Talk(SAY_KJ_DEATH);
            summons.DespawnAll();

            if (instance)
            {
                instance->SetData(DATA_KILJAEDEN_EVENT, DONE);
                if (instance->GetData(DATA_MODUS80))
                {
                    Map::PlayerList const& players = me->GetMap()->GetPlayers();
                    if (!players.isEmpty())
                        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                            if (Player* player = itr->getSource())
                                player->KilledMonsterCredit(411039, player->GetGUID());
                }

            }
        }

        void KilledUnit(Unit* /*victim*/)
        {
            Talk(SAY_KJ_SLAY);
        }

        void EnterEvadeMode()
        {
            Scripted_NoMovementAI::EnterEvadeMode();
            summons.DespawnAll();

            // Reset the controller
            if (instance)
            {
                instance->SetData(DATA_KILJAEDEN_EVENT, NOT_STARTED);
                if (Creature* control = Unit::GetCreature(*me, instance->GetData64(DATA_KILJAEDEN_CONTROLLER)))
                    CAST_AI(mob_kiljaeden_controller::mob_kiljaeden_controllerAI, control->AI())->Reset();
            }
        }

        void EnterCombat(Unit* /*who*/)
        {
            DoZoneInCombat();
            if (instance)
                instance->SetData(DATA_KILJAEDEN_EVENT, IN_PROGRESS);
        }

        void EnterNextPhase()
        {
            speechBegins  = true;
            orbActivated  = false;
            ChangeTimers(true, 0);//stop every cast, Shadow spike will reactivate em all
            timerIsDeactivated[TIMER_SHADOW_SPIKE] = false;
            timer[TIMER_SHADOW_SPIKE] = 100;
            // empowered orbs before darkness
            timer[TIMER_DARKNESS] = (phase == PHASE_SACRIFICE) ? 15000 : urand(15000,30000);
            timer[TIMER_ORBS_EMPOWER] = (phase == PHASE_SACRIFICE) ? 10000 : 3000;
        }

        void CastSinisterReflection()
        {
            Talk(SAY_KJ_REFLECTION);
            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, NoDragonTargetSelector()))
            {
                for (uint8 i = 0; i < 4; ++i)
                {
                    float x, y ,z;
                    target->GetPosition(x,y,z);
                    if (Creature* sinisterReflection = me->SummonCreature(CREATURE_SINISTER_REFLECTION, x, y, z, 0, TEMPSUMMON_DEAD_DESPAWN, 0))
                        sinisterReflection->AI()->AttackStart(target);
                }
                target->CastSpell(target, SPELL_SINISTER_REFLECTION, true);
            }
        }


        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim() || phase < PHASE_NORMAL)
                return;

            if (isWaiting)
            {
                if (waitTimer <= diff)
                {
                    isWaiting = false;
                    ChangeTimers(false, 0);
                } else waitTimer -= diff;
            }

            for (uint8 t = 0; t < activeTimers; ++t)
            {
                if (timer[t] < diff && !timerIsDeactivated[t])
                {
                    switch(t)
                    {
                    case TIMER_DARKNESS: //Phase 3
                        if (!me->IsNonMeleeSpellCasted(false))
                        {
                            // Begins to channel for 8 seconds, then deals 50'000 damage to all raid members.
                            if (!isInDarkness)
                            {
                                Talk(EMOTE_KJ_DARKNESS);
                                DoCast(SPELL_DARKNESS_OF_A_THOUSAND_SOULS);
                                ChangeTimers(true, 11000);
                                timer[TIMER_DARKNESS] = 8750;
                                timerIsDeactivated[TIMER_DARKNESS] = false;
                                if (phase == PHASE_SACRIFICE)
                                    timerIsDeactivated[TIMER_ARMAGEDDON] = false;
                                isInDarkness = true;
                            }
                            else
                            {
                                DoCast(SPELL_DARKNESS_OF_A_THOUSAND_SOULS_DAMAGE);
                                Talk(SAY_KJ_DARKNESS);
                                timer[TIMER_DARKNESS] = (phase == PHASE_SACRIFICE) ? 17000 : urand(32000, 50000);
                                timer[TIMER_SOUL_FLAY] = 1000;
                                timerIsDeactivated[TIMER_FIRE_BLOOM] = false;
                                isInDarkness = false;
                            }
                        }
                        break;
                        case TIMER_SPEECH:
                            if (speechBegins)
                            {
                                speechBegins=false;
                                switch(phase)
                                {
                                    case PHASE_NORMAL:
                                        speechPhaseEnd = 1;
                                        break;
                                    case PHASE_DARKNESS:
                                        speechPhaseEnd = 4;
                                        break;
                                    case PHASE_ARMAGEDDON:
                                        speechPhaseEnd = 7;
                                        break;
                                    case PHASE_SACRIFICE:
                                        speechPhaseEnd = 14;
                                        break;
                                }
                            }
                            if (speeches[speechCount].timer < speechTimer)
                            {
                                speechTimer = 0;
                                if (instance)
                                    if (Creature* speechCreature = Unit::GetCreature(*me, instance->GetData64(speeches[speechCount].creatureData)))
                                        speechCreature->AI()->Talk(speeches[speechCount].textid);
                                if (speechCount == 11)
                                {
                                    if (instance)
                                        if (Creature* anveena = Unit::GetCreature(*me, instance->GetData64(DATA_ANVEENA)))
                                        {
                                            anveena->RemoveAurasDueToSpell(SPELL_ANVEENA_PRISON);
                                            anveena->CastSpell(me, SPELL_SACRIFICE_OF_ANVEENA, false);
                                        }
                                }
                                else if (speechCount == 12)
                                {
                                    DoCast(SPELL_CUSTOM_08_STATE);
                                    ChangeTimers(true, 5500);
                                    timer[TIMER_SOUL_FLAY] = 0;
                                    timerIsDeactivated[TIMER_SPEECH] = false;
                                }
                                else if (speechCount == 13)
                                {
                                    if (Creature* anveena = Unit::GetCreature(*me, instance->GetData64(DATA_ANVEENA)))
                                        anveena->DisappearAndDie();
                                    me->RemoveAurasDueToSpell(SPELL_CUSTOM_08_STATE);
                                }
                                if (speechCount == speechPhaseEnd)
                                    timerIsDeactivated[TIMER_SPEECH] = true;
                                ++speechCount;
                            }
                            speechTimer += diff;
                            break;
                        case TIMER_SOUL_FLAY:
                            if (!me->IsNonMeleeSpellCasted(false))
                            {
                                DoCast(me->getVictim(), SPELL_SOUL_FLAY_SLOW, false);
                                DoCast(me->getVictim(), SPELL_SOUL_FLAY, false);
                                timer[TIMER_SOUL_FLAY] = 3500;
                            }
                            break;
                        case TIMER_LEGION_LIGHTNING:
                            if (!me->IsNonMeleeSpellCasted(false))
                            {
                                me->RemoveAurasDueToSpell(SPELL_SOUL_FLAY);
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, NoDragonTargetSelector()))
                                    DoCast(target, SPELL_LEGION_LIGHTNING);

                                timer[TIMER_LEGION_LIGHTNING] = (phase == PHASE_SACRIFICE) ? 18000 : 30000; // 18 seconds in PHASE_SACRIFICE
                                timer[TIMER_SOUL_FLAY] = 2500;
                            }
                            break;
                        case TIMER_FIRE_BLOOM:
                            if (timer[TIMER_DARKNESS] <= 20000)
                            {
                                timerIsDeactivated[TIMER_FIRE_BLOOM] = true;
                                break;
                            }

                            if (!me->IsNonMeleeSpellCasted(false))
                            {
                                me->RemoveAurasDueToSpell(SPELL_SOUL_FLAY);
                                DoCast(SPELL_FIRE_BLOOM);
                                timer[TIMER_FIRE_BLOOM] = 20000;//(Phase == PHASE_SACRIFICE) ? 25000 : 40000; // 25 seconds in PHASE_SACRIFICE
                                timer[TIMER_SOUL_FLAY] = 1000;
                            }
                            break;
                        case TIMER_SUMMON_SHIELDORB:
                            {
                                float x = 0;
                                float y = 0;
                                for (uint8 i = 1; i < phase; ++i)
                                {
                                    x = middleOfSunwell[0] + SHIELD_ORB_RADIUS * cos((i-1) * M_PI/2);
                                    y = middleOfSunwell[1] + SHIELD_ORB_RADIUS * sin((i-1) * M_PI/2);
                                    if (Creature* orb = me->SummonCreature(CREATURE_SHIELD_ORB, x, y, SHIELD_ORB_Z, 0, TEMPSUMMON_CORPSE_DESPAWN, 0))
                                        if (CreatureAI* orbAi = orb->AI())
                                            orbAi->DoAction(i%2);
                                }
                                timer[TIMER_SUMMON_SHIELDORB] = urand(30000,60000); // 30-60seconds cooldown
                                timer[TIMER_SOUL_FLAY] = 2000;
                                break;
                            }
                        case TIMER_SHADOW_SPIKE: //Phase 3
                            if (!me->IsNonMeleeSpellCasted(false))
                            {
                                CastSinisterReflection();
                                DoCast(SPELL_SHADOW_SPIKE);
                                ChangeTimers(true, 30000);
                                timer[TIMER_SHADOW_SPIKE] = 0;
                                timerIsDeactivated[TIMER_SPEECH] = false;
                            }
                            break;
                        case TIMER_FLAME_DART: //Phase 3
                            if (!me->IsNonMeleeSpellCasted(false))
                            {
                                DoCast(SPELL_FLAME_DART);
                                timer[TIMER_FLAME_DART] = 20000; //TODO Timer
                            }
                            break;
                        //case TIMER_DARKNESS: //Phase 3
                        //    if (!me->IsNonMeleeSpellCasted(false))
                        //    {
                        //        // Begins to channel for 8 seconds, then deals 50'000 damage to all raid members.
                        //        if (!isInDarkness)
                        //        {
                        //            Talk(EMOTE_KJ_DARKNESS);
                        //            DoCast(SPELL_DARKNESS_OF_A_THOUSAND_SOULS);
                        //            ChangeTimers(true, 10000);
                        //            timer[TIMER_DARKNESS] = 8750;
                        //            timerIsDeactivated[TIMER_DARKNESS] = false;
                        //            if (phase == PHASE_SACRIFICE)
                        //                timerIsDeactivated[TIMER_ARMAGEDDON] = false;
                        //            isInDarkness = true;
                        //        }
                        //        else
                        //        {
                        //            DoCast(SPELL_DARKNESS_OF_A_THOUSAND_SOULS_DAMAGE);
                        //            Talk(RAND(SAY_KJ_DARKNESS1,SAY_KJ_DARKNESS2,SAY_KJ_DARKNESS3));
                        //            timer[TIMER_DARKNESS] = (phase == PHASE_SACRIFICE) ? 17000 : urand(32000, 50000);
                        //            timer[TIMER_SOUL_FLAY] = 1000;
                        //            timerIsDeactivated[TIMER_FIRE_BLOOM] = false;
                        //            isInDarkness = false;
                        //        }
                        //    }
                        //    break;
                        case TIMER_ORBS_EMPOWER: //Phase 3
                            if (instance)
                                if (Creature* kalec = Unit::GetCreature(*me, instance->GetData64(DATA_KALECGOS_KJ)))
                                {
                                    switch (phase)
                                    {
                                    case PHASE_SACRIFICE:
                                        CAST_AI(boss_kalecgos_kj::boss_kalecgos_kjAI, kalec->AI())->EmpowerOrb(true);
                                        break;
                                    default:
                                        CAST_AI(boss_kalecgos_kj::boss_kalecgos_kjAI, kalec->AI())->EmpowerOrb(false);
                                        break;
                                    }
                                }
                            orbActivated = true;
                            timerIsDeactivated[TIMER_ORBS_EMPOWER] = true;
                            break;
                        case TIMER_ARMAGEDDON: //Phase 4
                            Unit *target = NULL;
                            //for (uint8 z = 0; z < 6; ++z)
                            //{
                                target = SelectTarget(SELECT_TARGET_RANDOM, 0, NoDragonTargetSelector());
                            //    if (!target || !target->HasAura(SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT,0))
                            //        break;
                            //}
                            if (target)
                            {
                                float x, y, z;
                                target->GetPosition(x, y, z);
                                me->SummonCreature(CREATURE_ARMAGEDDON_TARGET, x, y, z, 0, TEMPSUMMON_TIMED_DESPAWN, 15000);
                            }
                            timer[TIMER_ARMAGEDDON] = 2000; // No, I'm not kidding
                            break;
                     }
                }
            }
            DoMeleeAttackIfReady();
            //Time runs over!
            for (uint8 i = 0; i < activeTimers; ++i)
                if (!timerIsDeactivated[i])
                {
                    timer[i] -= diff;
                    if (((int32)timer[i]) < 0)
                        timer[i] = 0;
                }

            //Phase 3
            if (phase <= PHASE_NORMAL && !isInDarkness)
            {
                if (phase == PHASE_NORMAL && HealthBelowPct(85))
                {
                    phase = PHASE_DARKNESS;
                    activeTimers = 9;
                    EnterNextPhase();
                }
                else return;
            }

            //Phase 4
            if (phase <= PHASE_DARKNESS && !isInDarkness)
            {
                if (phase == PHASE_DARKNESS && HealthBelowPct(55))
                {
                    phase = PHASE_ARMAGEDDON;
                    activeTimers = 10;
                    EnterNextPhase();
                }
                else return;
            }

            //Phase 5 specific spells all we can
            if (phase <= PHASE_ARMAGEDDON && !isInDarkness)
            {
                if (phase == PHASE_ARMAGEDDON && HealthBelowPct(25))
                {
                    phase = PHASE_SACRIFICE;
                    EnterNextPhase();
                }
                else return;
            }
        }
    };

};
//AI for Hand of the Deceiver
class mob_hand_of_the_deceiver : public CreatureScript
{
public:
    mob_hand_of_the_deceiver() : CreatureScript("mob_hand_of_the_deceiver") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_hand_of_the_deceiverAI (creature);
    }

    struct mob_hand_of_the_deceiverAI : public ScriptedAI
    {
        mob_hand_of_the_deceiverAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        uint32 shadowBoltVolleyTimer;
        uint32 felfirePortalTimer;

        void Reset()
        {
            // TODO: Timers!
            shadowBoltVolleyTimer = urand(3000, 7000); // So they don't all cast it in the same moment.
            felfirePortalTimer = 20000;
            if (instance)
                instance->SetData(DATA_KILJAEDEN_EVENT, NOT_STARTED);

            if (instance && instance->GetData(DATA_MODUS80))
            {
                me->SetLevel(83);
                me->SetMaxHealth(10000000);
                me->SetHealth(me->GetMaxHealth());
                me->SetMaxPower(POWER_MANA, 2000000);
                me->SetPower(POWER_MANA, me->GetMaxPower(POWER_MANA));
                me->SetArmor(10573);
                me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 2000);
                me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 3000);
                me->UpdateDamagePhysical(BASE_ATTACK);
            }
        }

        void JustSummoned(Creature* summoned)
        {
            summoned->setFaction(me->getFaction());
            summoned->SetLevel(me->getLevel());
        }

        void EnterCombat(Unit* who)
        {
            if (instance)
            {
                instance->SetData(DATA_KILJAEDEN_EVENT, IN_PROGRESS);
                if (Creature* control = Unit::GetCreature(*me, instance->GetData64(DATA_KILJAEDEN_CONTROLLER)))
                    control->AddThreat(who, 1.0f);
            }
            me->InterruptNonMeleeSpells(true);
        }

        void JustDied(Unit* /*killer*/)
        {
            if (!instance)
                return;

            if (Creature* control = Unit::GetCreature(*me, instance->GetData64(DATA_KILJAEDEN_CONTROLLER)))
                ++(CAST_AI(mob_kiljaeden_controller::mob_kiljaeden_controllerAI, control->AI())->deceiverDeathCount);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!me->isInCombat())
                DoCast(me, SPELL_SHADOW_CHANNELING);

            if (!UpdateVictim())
                return;

            // Gain Shadow Infusion at 20% health
            if (HealthBelowPct(20) && !me->HasAura(SPELL_SHADOW_INFUSION, 0))
                DoCast(me, SPELL_SHADOW_INFUSION, true);

            // Shadow Bolt Volley - Shoots Shadow Bolts at all enemies within 30 yards, for ~2k Shadow damage.
            if (shadowBoltVolleyTimer <= diff)
            {
                DoCast(me->getVictim(), SPELL_SHADOW_BOLT_VOLLEY);
                shadowBoltVolleyTimer = urand(3000, 7000);
            }
            else
                shadowBoltVolleyTimer -= diff;

            // Felfire Portal - Creatres a portal, that spawns Volatile Felfire Fiends, which do suicide bombing.
            if (felfirePortalTimer <= diff)
            {
                if (Creature* portal = DoSpawnCreature(CREATURE_FELFIRE_PORTAL, 0, 0,0, 0, TEMPSUMMON_TIMED_DESPAWN, 20000))
                {
                    ThreatContainer::StorageType const &threatlist = me->getThreatManager().getThreatList();
                    for (ThreatContainer::StorageType::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
                    {
                        Unit* unit = Unit::GetUnit(*me, (*itr)->getUnitGuid());
                        if (unit)
                            portal->AddThreat(unit, 1.0f);
                    }
                }
                felfirePortalTimer = 20000;
            } else felfirePortalTimer -= diff;

            DoMeleeAttackIfReady();
        }
    };

};


//AI for Felfire Portal
class mob_felfire_portal : public CreatureScript
{
public:
    mob_felfire_portal() : CreatureScript("mob_felfire_portal") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_felfire_portalAI (creature);
    }

    struct mob_felfire_portalAI : public Scripted_NoMovementAI
    {
        mob_felfire_portalAI(Creature* creature) : Scripted_NoMovementAI(creature) {}

        uint32 uiSpawnFiendTimer;

        void Reset()
        {
            uiSpawnFiendTimer = 5000;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }

        void JustSummoned(Creature* summoned)
        {
            summoned->setFaction(me->getFaction());
            summoned->SetLevel(me->getLevel());
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (uiSpawnFiendTimer <= diff)
            {
                if (Creature* fiend = DoSpawnCreature(CREATURE_VOLATILE_FELFIRE_FIEND, 0, 0, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 20000))
                    fiend->AddThreat(SelectTarget(SELECT_TARGET_RANDOM,0), 100000.0f);
                uiSpawnFiendTimer = urand(4000,8000);
            } else uiSpawnFiendTimer -= diff;
        }
    };

};


//AI for Felfire Fiend
class mob_volatile_felfire_fiend : public CreatureScript
{
public:
    mob_volatile_felfire_fiend() : CreatureScript("mob_volatile_felfire_fiend") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_volatile_felfire_fiendAI (creature);
    }

    struct mob_volatile_felfire_fiendAI : public ScriptedAI
    {
        mob_volatile_felfire_fiendAI(Creature* c) : ScriptedAI(c)
        {
            instance = c->GetInstanceScript();
        }

        InstanceScript* instance;
        uint32 uiExplodeTimer;

        bool bLockedTarget;

        void Reset()
        {
            uiExplodeTimer = 2000;
            bLockedTarget = false;
            if (instance && instance->GetData(DATA_MODUS80))
            {
                me->SetLevel(83);
                me->SetMaxHealth(1000000);
                me->SetHealth(me->GetMaxHealth());
                me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1000);
                me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 2000);
                me->UpdateDamagePhysical(BASE_ATTACK);
            }
        }

        void DamageTaken(Unit* /*done_by*/, uint32& damage)
        {
            if (damage > me->GetHealth())
                DoCast(me, SPELL_FELFIRE_FISSION, true);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (!bLockedTarget)
            {
                me->AddThreat(me->getVictim(), 10000000.0f);
                bLockedTarget = true;
            }

            if (uiExplodeTimer)
            {
                if (uiExplodeTimer <= diff)
                    uiExplodeTimer = 0;
                else uiExplodeTimer -= diff;
            }
            else if (me->IsWithinDistInMap(me->getVictim(), 3)) // Explode if it's close enough to it's target
            {
                DoCast(me->getVictim(), SPELL_FELFIRE_FISSION);
                me->Kill(me);
            }
        }
    };

};


//AI for Armageddon target
class mob_armageddon : public CreatureScript
{
public:
    mob_armageddon() : CreatureScript("mob_armageddon") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_armageddonAI (creature);
    }

    struct mob_armageddonAI : public CreatureAI
    {
        mob_armageddonAI(Creature* c) : CreatureAI(c)
        {
            instance = c->GetInstanceScript();
            me->SetVisible(true);//visible to see all spell anims
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);//can't be targeted
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);//can't be damaged
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(11686);//invisible model, around a size of a player
        }

        InstanceScript* instance;

        uint8 spell;
        uint32 uiTimer;

        void Reset()
        {
            me->SetLevel(73);
            spell = 0;
            uiTimer = 0;
            if (instance && instance->GetData(DATA_MODUS80))
            {
                me->SetLevel(83);
                me->SetMaxHealth(1000000);
                me->SetHealth(me->GetMaxHealth());
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (uiTimer <= diff)
            {
                switch(spell)
                {
                    case 0:
                        DoCast(me, SPELL_ARMAGEDDON_VISUAL, true);
                        ++spell;
                        break;
                    case 1:
                        DoCast(me, SPELL_ARMAGEDDON_VISUAL2, true);
                        uiTimer = 4000;
                        ++spell;
                        break;
                    case 2:
                        me->CastSpell(me, SPELL_ARMAGEDDON_TRIGGER, true);
                        ++spell;
                        uiTimer = 5000;
                        break;
                    case 3:
                        me->CastSpell(me, SPELL_ARMAGEDDON_DAMAGE, true);
                        ++spell;
                        uiTimer = 3000;
                        break;
                    case 4:
                        me->DespawnOrUnsummon();
                        break;
                }
            } else uiTimer -=diff;
        }
    };

};


//AI for Shield Orbs
class mob_shield_orb : public CreatureScript
{
public:
    mob_shield_orb() : CreatureScript("mob_shield_orb") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_shield_orbAI (creature);
    }

    struct mob_shield_orbAI : public ScriptedAI
    {
        mob_shield_orbAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        bool bClockwise;
        bool next;
        uint32 uiTimer;
        uint32 uiCheckTimer;
        float x, y, c, mx, my;

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetDisableGravity(true);
            me->AddAura(SPELL_SHADOW_BOLT, me);
            mx = middleOfSunwell[0];
            my = middleOfSunwell[1];
            bClockwise = true;//urand(0,1);
            next = false;

            if (instance && instance->GetData(DATA_MODUS80))
            {
                me->SetLevel(83);
                me->SetMaxHealth(500000);
                me->SetHealth(me->GetMaxHealth());
            }

            c = acos((me->GetPositionX() - mx) / SHIELD_ORB_RADIUS);
        }

        void DoAction(const int32 param)
        {
            bClockwise = param;

            if (bClockwise)
                c += M_PI/32;
            else
                c -= M_PI/32;

            y = my + SHIELD_ORB_RADIUS * sin(c);
            x = mx + SHIELD_ORB_RADIUS * cos(c);

            me->GetMotionMaster()->MovePoint(1, x, y, SHIELD_ORB_Z);
        }

        void MovementInform(uint32 type, uint32 /*id*/)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            next = true;
        }

        void UpdateAI(const uint32 /*diff*/)
        {
            if (!next)
                return;
            next = false;
            if (bClockwise)
            {
                c += M_PI/32;

                if (c >= 2*M_PI)
                    c = 0;
            }
            else
            {
                c -= M_PI/32;

                if (c <= -2*M_PI)
                    c = 0;
            }
            y = my + SHIELD_ORB_RADIUS * sin(c);
            x = mx + SHIELD_ORB_RADIUS * cos(c);

            me->GetMotionMaster()->MovePoint(1, x, y, SHIELD_ORB_Z);
        }
    };

};

//AI for Sinister Reflection
class mob_sinster_reflection : public CreatureScript
{
public:
    mob_sinster_reflection() : CreatureScript("mob_sinster_reflection") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_sinster_reflectionAI (creature);
    }

    struct mob_sinster_reflectionAI : public ScriptedAI
    {
        mob_sinster_reflectionAI(Creature* c) : ScriptedAI(c)
        {
            instance = c->GetInstanceScript();
        }

        InstanceScript* instance;

        uint8 victimClass;
        uint32 uiTimer[3];

        void Reset()
        {
            uiTimer[0] = 0;
            uiTimer[1] = 0;
            uiTimer[2] = 0;
            victimClass = 0;

            if (instance && instance->GetData(DATA_MODUS80))
            {
                me->SetLevel(83);
                me->SetMaxHealth(500000);
                me->SetHealth(me->GetMaxHealth());
                me->SetArmor(10573);
                me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1000);
                me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 2000);
                me->UpdateDamagePhysical(BASE_ATTACK);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if ((victimClass == 0) && me->getVictim())
            {
                victimClass = me->getVictim()->getClass();
                switch (victimClass)
                {
                    case CLASS_DRUID:
                        break;
                    case CLASS_HUNTER:
                        break;
                    case CLASS_MAGE:
                        break;
                    case CLASS_WARLOCK:
                        break;
                    case CLASS_WARRIOR:
                        me->SetCanDualWield(true);
                        break;
                    case CLASS_PALADIN:
                        break;
                    case CLASS_PRIEST:
                        break;
                    case CLASS_SHAMAN:
                        me->SetCanDualWield(true);
                        break;
                    case CLASS_ROGUE:
                        me->SetCanDualWield(true);
                        break;
                    case CLASS_DEATH_KNIGHT:
                        me->SetCanDualWield(true);
                        break;
                }
            }

            switch(victimClass) {
                case CLASS_DRUID:
                    if (uiTimer[1] <= diff)
                    {
                        DoCast(SelectTarget(SELECT_TARGET_RANDOM, 0, NoDragonTargetSelector()), SPELL_SR_MOONFIRE);
                        uiTimer[1] = urand(2000, 4000);
                    }
                    DoMeleeAttackIfReady();
                    break;
                case CLASS_HUNTER:
                    if (uiTimer[1] <= diff)
                    {
                        DoCast(SelectTarget(SELECT_TARGET_RANDOM, 0, NoDragonTargetSelector()), SPELL_SR_MULTI_SHOT);
                        uiTimer[1] = urand(8000, 10000);
                    }
                    if (uiTimer[2] <= diff)
                    {
                        DoCast(me->getVictim(), SPELL_SR_SHOOT, false);
                        uiTimer[2] = urand(4000, 6000);
                    }
                    if (me->IsWithinMeleeRange(me->getVictim(), 6))
                    {
                        if (uiTimer[0] <= diff)
                        {
                            DoCast(me->getVictim(), SPELL_SR_WING_CLIP, false);
                            uiTimer[0] = urand(6000, 8000);
                        }
                        DoMeleeAttackIfReady();
                    }
                    break;
                case CLASS_MAGE:
                    if (uiTimer[1] <= diff)
                    {
                        DoCast(me->getVictim(), SPELL_SR_FIREBALL, false);
                        uiTimer[1] = urand(2000, 4000);
                    }
                    DoMeleeAttackIfReady();
                    break;
                case CLASS_WARLOCK:
                    if (uiTimer[1] <= diff)
                    {
                        DoCast(me->getVictim(), SPELL_SR_SHADOW_BOLT, false);
                        uiTimer[1] = urand(3000, 5000);
                    }
                    if (uiTimer[2] <= diff)
                    {
                        DoCast(SelectTarget(SELECT_TARGET_RANDOM, 0, NoDragonTargetSelector()), SPELL_SR_CURSE_OF_AGONY, true);
                        uiTimer[2] = urand(2000, 4000);
                    }
                    DoMeleeAttackIfReady();
                    break;
                case CLASS_WARRIOR:
                    if (uiTimer[1] <= diff)
                    {
                        DoCast(me->getVictim(), SPELL_SR_WHIRLWIND, false);
                        uiTimer[1] = urand(9000, 11000);
                    }
                    DoMeleeAttackIfReady();
                    break;
                case CLASS_PALADIN:
                    if (uiTimer[1] <= diff)
                    {
                        DoCast(me->getVictim(), SPELL_SR_HAMMER_OF_JUSTICE, false);
                        uiTimer[1] = urand(6000, 8000);
                    }
                    if (uiTimer[2] <= diff)
                    {
                        DoCast(me->getVictim(), SPELL_SR_HOLY_SHOCK, false);
                        uiTimer[2] = urand(2000, 4000);
                    }
                    DoMeleeAttackIfReady();
                    break;
                case CLASS_PRIEST:
                    if (uiTimer[1] <= diff)
                    {
                        DoCast(me->getVictim(), SPELL_SR_HOLY_SMITE, false);
                        uiTimer[1] = urand(4000, 6000);
                    }
                    if (uiTimer[2] <= diff)
                    {
                        if (me->HasAura(SPELL_SR_RENEW))
                        {
                            if (Creature* pKj = Unit::GetCreature(*me, instance->GetData64(DATA_KILJAEDEN)))
                                DoCast(pKj, SPELL_SR_RENEW, false);
                        }
                        else
                            DoCast(me, SPELL_SR_RENEW, false);

                        uiTimer[2] = urand(6000, 8000);
                    }
                    DoMeleeAttackIfReady();
                    break;
                case CLASS_SHAMAN:
                    if (uiTimer[1] <= diff)
                    {
                        DoCast(me->getVictim(), SPELL_SR_EARTH_SHOCK, false);
                        uiTimer[1] = urand(4000, 6000);
                    }
                    DoMeleeAttackIfReady();
                    break;
                case CLASS_ROGUE:
                    if (uiTimer[1] <= diff)
                    {
                        DoCast(me->getVictim(), SPELL_SR_HEMORRHAGE, false);
                        uiTimer[1] = urand(4000, 6000);
                    }
                    DoMeleeAttackIfReady();
                    break;
                case CLASS_DEATH_KNIGHT:
                    if (uiTimer[1] <= diff)
                    {
                        DoCast(me->getVictim(), SPELL_SR_PLAGUE_STRIKE, false);
                        uiTimer[1] = urand(4000, 6000);
                    }
                    DoMeleeAttackIfReady();
                    break;
                }
                TC_LOG_DEBUG("scripts", "Sinister-Timer");
                for (uint8 i = 0; i < 3; ++i)
                    uiTimer[i] -= diff;
            }
    };

};

class npc_sunwell_teleporter : public CreatureScript
{
public:
    npc_sunwell_teleporter() : CreatureScript("npc_sunwell_teleporter") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*Sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        player->PlayerTalkClass->SendCloseGossip();
        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF+1:
            if (InstanceScript* instance = player->GetInstanceScript())
                if ((player->GetMapId() == 580) && !instance->GetData(DATA_MODUS80) && (instance->GetData(DATA_KILJAEDEN_EVENT) == NOT_STARTED))
                    if ((player->GetGroup() && player->GetGroup()->isRaidGroup() && player->GetGroup()->IsLeader(player->GetGUID())) || player->isGameMaster())
                    {
                        instance->SetData(DATA_MODUS80, 1);
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_PLAYER, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
                        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
                    }
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            if (InstanceScript* instance = player->GetInstanceScript())
                if ((player->GetMapId() == 580) && instance->GetData(DATA_MODUS80) && (instance->GetData(DATA_KILJAEDEN_EVENT) == NOT_STARTED))
                    if ((player->GetGroup() && player->GetGroup()->isRaidGroup()) || player->isGameMaster())
                        player->NearTeleportTo(1768.0f, 685.1f, 71.3f, 2.4f);
            break;
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (InstanceScript* instance = player->GetInstanceScript())
            if ((player->GetMapId() == 580) && !instance->GetData(DATA_MODUS80) && (instance->GetData(DATA_KILJAEDEN_EVENT) == NOT_STARTED))
            {
                if ((player->GetGroup() && player->GetGroup()->isRaidGroup() && player->GetGroup()->IsLeader(player->GetGUID())) || player->isGameMaster())
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_LEADER, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            }
            else
                if (((player->GetMapId() == 580) && instance->GetData(DATA_MODUS80) && (instance->GetData(DATA_KILJAEDEN_EVENT) == NOT_STARTED)) || player->isGameMaster())
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_PLAYER, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }
};

class spell_kiljaeden_shadow_spike : public SpellScriptLoader
{
public:
    spell_kiljaeden_shadow_spike() : SpellScriptLoader("spell_kiljaeden_shadow_spike") { }

    class spell_kiljaeden_shadow_spike_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kiljaeden_shadow_spike_SpellScript)

        bool Load()
        {
            if (Unit* caster = GetCaster())
            {
                if (caster->GetAI())
                    if (Unit* target = caster->GetAI()->SelectTarget(SELECT_TARGET_RANDOM, 0, NoDragonTargetSelector()))
                        SetExplTargetUnit(target);
                return true;
            }
            return false;
        }

        void Register(){}
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_kiljaeden_shadow_spike_SpellScript();
    }
};

class IsNotSinister
{
public:
    bool operator()(WorldObject* unit)
    {
        return unit->GetEntry() != CREATURE_SINISTER_REFLECTION;
    }
};

class spell_kiljaeden_sinister_reflection : public SpellScriptLoader
{
public:
    spell_kiljaeden_sinister_reflection() : SpellScriptLoader("spell_kiljaeden_sinister_reflection") { }

    class spell_kiljaeden_sinister_reflection_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kiljaeden_sinister_reflection_SpellScript);

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(IsNotSinister());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_kiljaeden_sinister_reflection_SpellScript::FilterTargets, EFFECT_ALL, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_kiljaeden_sinister_reflection_SpellScript();
    }
};

class IsDragon
{
public:
    bool operator()(WorldObject* unit)
    {
        return unit->ToUnit() && unit->GetEntry() == CREATURE_POWER_OF_THE_BLUE_DRAGONFLIGHT;
    }
};

class spell_kiljaeden_armageddon : public SpellScriptLoader
{
public:
    spell_kiljaeden_armageddon() : SpellScriptLoader("spell_kiljaeden_armageddon") { }

    class spell_kiljaeden_armageddon_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kiljaeden_armageddon_SpellScript);

        void ModDestHeight(SpellEffIndex effIndex)
        {
            static Position const offset = {0.0f, 0.0f, 40.0f, 0.0f};
            WorldLocation* dest = const_cast<WorldLocation*>(GetExplTargetDest());
            if (!dest)
                return;
            dest->RelocateOffset(offset);
        }

        void Register()
        {
            OnEffectLaunch += SpellEffectFn(spell_kiljaeden_armageddon_SpellScript::ModDestHeight, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_kiljaeden_armageddon_SpellScript();
    }
};
class spell_kiljaeden_haste_revit : public SpellScriptLoader
{
public:
    spell_kiljaeden_haste_revit() : SpellScriptLoader("spell_kiljaeden_haste_revit") { }

    class spell_kiljaeden_haste_revit_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kiljaeden_haste_revit_SpellScript);

        bool Load()
        {
            if (Unit* caster = GetCaster())
            {
                if (InstanceScript* instance = caster->GetInstanceScript())
                {
                    if (caster->GetMapId() == 580 && instance->GetData(DATA_MODUS80) && GetSpellInfo()->Id == SPELL_REVITALIZE)
                    {
                        SetSpellValue(SPELLVALUE_BASE_POINT0, (GetSpellInfo()->Effects[EFFECT_0].BasePoints+1) * 3);
                        SetSpellValue(SPELLVALUE_BASE_POINT1, (GetSpellInfo()->Effects[EFFECT_1].BasePoints+1) * 3);
                   //     SetSpellValue(SPELLVALUE_BASE_POINT2, (GetSpellInfo()->Effects[EFFECT_2].BasePoints+1) * 3);
                    }
                }
            }
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(IsDragon());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_kiljaeden_haste_revit_SpellScript::FilterTargets, EFFECT_ALL, TARGET_UNIT_CONE_ALLY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_kiljaeden_haste_revit_SpellScript();
    }
};

class spell_kiljaeden_vengeance_of_blue_flight  : public SpellScriptLoader
{
    public:
        spell_kiljaeden_vengeance_of_blue_flight() : SpellScriptLoader("spell_kiljaeden_vengeance_of_blue_flight") { }

        class spell_kiljaeden_vengeance_of_blue_flightAuraScript : public AuraScript
        {
            PrepareAuraScript(spell_kiljaeden_vengeance_of_blue_flightAuraScript)

            void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit * caster = GetCaster();
                // caster may be not avalible (logged out for example)
                if (!caster)
                    return;

                caster->RemoveAurasDueToSpell(SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT);
                caster->RemoveAurasDueToSpell(SPELL_POSSESS_DRAKE_IMMUNE);
            }

            // function registering
            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_kiljaeden_vengeance_of_blue_flightAuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_MOD_POSSESS, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript *GetAuraScript() const
        {
            return new spell_kiljaeden_vengeance_of_blue_flightAuraScript();
        }
};

class spell_kj_80_damage3effAll : public SpellScriptLoader
{
public:
    spell_kj_80_damage3effAll() : SpellScriptLoader("spell_kj_80_damage3effAll") { }

    class spell_kj_80_damage3effAll_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kj_80_damage3effAll_SpellScript)

        bool Load()
        {
            if (Unit* caster = GetCaster())
            {
                if (InstanceScript* instance = caster->GetInstanceScript())
                {
                    if (caster->GetMapId() == 580 && instance->GetData(DATA_MODUS80))
                    {
                        SetSpellValue(SPELLVALUE_BASE_POINT0, (GetSpellInfo()->Effects[EFFECT_0].BasePoints+1) * 3);
                        SetSpellValue(SPELLVALUE_BASE_POINT1, (GetSpellInfo()->Effects[EFFECT_1].BasePoints+1) * 3);
                        SetSpellValue(SPELLVALUE_BASE_POINT2, (GetSpellInfo()->Effects[EFFECT_2].BasePoints+1) * 3);
                    }
                }
            }
            return true;
        }

        void Register(){}
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_kj_80_damage3effAll_SpellScript();
    }
};

class spell_kj_80_damage3eff0 : public SpellScriptLoader
{
public:
    spell_kj_80_damage3eff0() : SpellScriptLoader("spell_kj_80_damage3eff0") { }

    class spell_kj_80_damage3eff0_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kj_80_damage3eff0_SpellScript)

        bool Load()
        {
            if (Unit* caster = GetCaster())
            {
                if (InstanceScript* instance = caster->GetInstanceScript())
                {
                    if (caster->GetMapId() == 580 && instance->GetData(DATA_MODUS80))
                    {
                        SetSpellValue(SPELLVALUE_BASE_POINT0, (GetSpellInfo()->Effects[EFFECT_0].BasePoints+1) * 3);
                    }
                }
            }
            return true;
        }

        void Register(){}
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_kj_80_damage3eff0_SpellScript();
    }
};

class spell_kj_80_damage3eff1 : public SpellScriptLoader
{
public:
    spell_kj_80_damage3eff1() : SpellScriptLoader("spell_kj_80_damage3eff1") { }

    class spell_kj_80_damage3eff1_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kj_80_damage3eff1_SpellScript)

        bool Load()
        {
            if (Unit* caster = GetCaster())
            {
                if (InstanceScript* instance = caster->GetInstanceScript())
                {
                    if (caster->GetMapId() == 580 && instance->GetData(DATA_MODUS80))
                    {
                        SetSpellValue(SPELLVALUE_BASE_POINT1, (GetSpellInfo()->Effects[EFFECT_1].BasePoints+1) * 3);
                    }
                }
            }
            return true;
        }

        void Register(){}
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_kj_80_damage3eff1_SpellScript();
    }
};

class spell_kj_80_damage4effAll : public SpellScriptLoader
{
public:
    spell_kj_80_damage4effAll() : SpellScriptLoader("spell_kj_80_damage4effAll") { }

    class spell_kj_80_damage4effAll_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kj_80_damage4effAll_SpellScript)

        bool Load()
        {
            if (Unit* caster = GetCaster())
            {
                if (InstanceScript* instance = caster->GetInstanceScript())
                {
                    if (caster->GetMapId() == 580 && instance->GetData(DATA_MODUS80))
                    {
                        SetSpellValue(SPELLVALUE_BASE_POINT0, (GetSpellInfo()->Effects[EFFECT_0].BasePoints+1) * 4);
                        SetSpellValue(SPELLVALUE_BASE_POINT1, (GetSpellInfo()->Effects[EFFECT_1].BasePoints+1) * 4);
                        SetSpellValue(SPELLVALUE_BASE_POINT2, (GetSpellInfo()->Effects[EFFECT_2].BasePoints+1) * 4);
                    }
                }
            }
            return true;
        }

        void Register(){}
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_kj_80_damage4effAll_SpellScript();
    }
};

class spell_kj_80_damage4eff0 : public SpellScriptLoader
{
public:
    spell_kj_80_damage4eff0() : SpellScriptLoader("spell_kj_80_damage4eff0") { }

    class spell_kj_80_damage4eff0_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kj_80_damage4eff0_SpellScript)

            bool Load()
        {
            if (Unit* caster = GetCaster())
            {
                if (InstanceScript* instance = caster->GetInstanceScript())
                {
                    if (caster->GetMapId() == 580 && instance->GetData(DATA_MODUS80))
                    {
                        SetSpellValue(SPELLVALUE_BASE_POINT0, (GetSpellInfo()->Effects[EFFECT_0].BasePoints+1) * 4);
                    }
                }
            }
            return true;
        }

        void Register(){}
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_kj_80_damage4eff0_SpellScript();
    }
};

class spell_kj_80_damage4eff1 : public SpellScriptLoader
{
public:
    spell_kj_80_damage4eff1() : SpellScriptLoader("spell_kj_80_damage4eff1") { }

    class spell_kj_80_damage4eff1_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kj_80_damage4eff1_SpellScript)

            bool Load()
        {
            if (Unit* caster = GetCaster())
            {
                if (InstanceScript* instance = caster->GetInstanceScript())
                {
                    if (caster->GetMapId() == 580 && instance->GetData(DATA_MODUS80))
                    {
                        SetSpellValue(SPELLVALUE_BASE_POINT1, (GetSpellInfo()->Effects[EFFECT_1].BasePoints+1) * 4);
                    }
                }
            }
            return true;
        }

        void Register(){}
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_kj_80_damage4eff1_SpellScript();
    }
};

class spell_kj_80_damage5effAll : public SpellScriptLoader
{
public:
    spell_kj_80_damage5effAll() : SpellScriptLoader("spell_kj_80_damage5effAll") { }

    class spell_kj_80_damage5effAll_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kj_80_damage5effAll_SpellScript)

            bool Load()
        {
            if (Unit* caster = GetCaster())
            {
                if (InstanceScript* instance = caster->GetInstanceScript())
                {
                    if (caster->GetMapId() == 580 && instance->GetData(DATA_MODUS80))
                    {
                        SetSpellValue(SPELLVALUE_BASE_POINT0, (GetSpellInfo()->Effects[EFFECT_0].BasePoints+1) * 5);
                        SetSpellValue(SPELLVALUE_BASE_POINT1, (GetSpellInfo()->Effects[EFFECT_1].BasePoints+1) * 5);
                        SetSpellValue(SPELLVALUE_BASE_POINT2, (GetSpellInfo()->Effects[EFFECT_2].BasePoints+1) * 5);
                    }
                }
            }
            return true;
        }

        void Register(){}
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_kj_80_damage5effAll_SpellScript();
    }
};

class spell_kj_80_damage5eff0 : public SpellScriptLoader
{
public:
    spell_kj_80_damage5eff0() : SpellScriptLoader("spell_kj_80_damage5eff0") { }

    class spell_kj_80_damage5eff0_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kj_80_damage5eff0_SpellScript)

            bool Load()
        {
            if (Unit* caster = GetCaster())
            {
                if (InstanceScript* instance = caster->GetInstanceScript())
                {
                    if (caster->GetMapId() == 580 && instance->GetData(DATA_MODUS80))
                    {
                        SetSpellValue(SPELLVALUE_BASE_POINT0, (GetSpellInfo()->Effects[EFFECT_0].BasePoints+1) * 5);
                    }
                }
            }
            return true;
        }

        void Register(){}
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_kj_80_damage5eff0_SpellScript();
    }
};

class spell_kj_80_damage5eff1 : public SpellScriptLoader
{
public:
    spell_kj_80_damage5eff1() : SpellScriptLoader("spell_kj_80_damage5eff1") { }

    class spell_kj_80_damage5eff1_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kj_80_damage5eff1_SpellScript)

            bool Load()
        {
            if (Unit* caster = GetCaster())
            {
                if (InstanceScript* instance = caster->GetInstanceScript())
                {
                    if (caster->GetMapId() == 580 && instance->GetData(DATA_MODUS80))
                    {
                        SetSpellValue(SPELLVALUE_BASE_POINT1, (GetSpellInfo()->Effects[EFFECT_1].BasePoints+1) * 5);
                    }
                }
            }
            return true;
        }

        void Register(){}
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_kj_80_damage5eff1_SpellScript();
    }
};



void AddSC_boss_kiljaeden()
{
    new go_orb_of_the_blue_flight();
    new boss_kalecgos_kj();
    new boss_kiljaeden();
    new mob_kiljaeden_controller();
    new mob_hand_of_the_deceiver();
    new mob_felfire_portal();
    new mob_volatile_felfire_fiend();
    new mob_armageddon();
    new mob_shield_orb();
    new mob_sinster_reflection();
    new npc_sunwell_teleporter();
    new spell_kiljaeden_shadow_spike();
    new spell_kiljaeden_haste_revit();
    new spell_kiljaeden_vengeance_of_blue_flight();
    new spell_kiljaeden_sinister_reflection();
    new spell_kj_80_damage3effAll();
    new spell_kj_80_damage3eff0();
    new spell_kj_80_damage3eff1();
    new spell_kj_80_damage4effAll();
    new spell_kj_80_damage4eff0();
    new spell_kj_80_damage4eff1();
    new spell_kj_80_damage5effAll();
    new spell_kj_80_damage5eff0();
    new spell_kj_80_damage5eff1();
    new spell_kiljaeden_armageddon();
}
