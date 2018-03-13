/*
* Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

/*
##### Script Info #####
Author: Not Nagash - Bennu
Progress: 90%

TODO:
-as GlaDOS says: test, wanna do tests.

SideNotes:
-manca il drink magma quando esce dalla propria piattaforma
*/

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "PassiveAI.h"
#include "SpellScript.h"
#include "MoveSplineInit.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "firelands.h"

/*
*       IMPORTANT DEFINES
* modificatele per cambiare particolari aspetti del comportamento del boss
*/
#define DAMAGETHRESHOLD  RAID_MODE(15000, 21000, 15000, 21000)  //la quantità minima di danni per far sì che rhyolith si giri in quella direzione
// valore di default: 12k 10man, 18k 25 man
#define MAXSTEERAMOUNT   20     //quanto velocemente può cambiar direzione al massimo?   20 = 28,6 gradi al secondo
#define PRENERF4_2       false  //l'ai si deve comportare come prima del nerf di fine 4.2?


#define SAY_AGGRO    "Hah? Hruumph? Soft little fleshy-things? Here? Nuisances, nuisances!" 
#define SOU_AGGRO    24537
#define SAY_DEATH    "Broken. Mnngghhh... broken..." 
#define SOU_DEATH    24545
#define SAY_STOMP1   "Stomp now."
#define SOU_STOMP1   24556
#define SAY_STOMP2   "I'll crush you underfoot!"
#define SOU_STOMP2   24557
#define SAY_MOLTEN   "Eons I have slept undisturbed... Now this... Creatures of flesh, now you will BURN!" 
#define SOU_MOLTEN   24558
#define SAY_WEAK1    "Augh - smooshy little pests, look what you've done!" 
#define SOU_WEAK1    24542
#define SAY_WEAK2    "Oh you little beasts..." 
#define SOU_WEAK2    24544
#define SAY_WEAK3    "Graaahh!" 
#define SOU_WEAK3    24541
#define SAY_WEAK4    "Uurrghh now you... you infuriate me!" 
#define SOU_WEAK4    24543
#define SAY_WEAK5    "Oww now hey." 
#define SOU_WEAK5    24540
#define SAY_KILL1    "Squeak, little pest."  
#define SOU_KILL1    24548
#define SAY_KILL2    "Finished." 
#define SOU_KILL2     24546
#define SAY_KILL3    "So soft!" 
#define SOU_KILL3    24547
#define SAY_IGNI1    "Buuurrrnn!" 
#define SOU_IGNI1    24550
#define SAY_IGNI2    "Consuuuuuuume!" 
#define SOU_IGNI2    24553
#define SAY_IGNI3    "Succumb to living flame." 
#define SOU_IGNI3    24551
#define SAY_IGNI4    "My inner fire can never die!" 
#define SOU_IGNI4    24552
#define SAY_IGNI5    "Flesh, buuurrrns." 
#define SOU_IGNI5    24554


#define PHASE1_SKIN         38414
#define PHASE1_FIRST_SKIN   38669
#define PHASE1_SECOND_SKIN  38676
#define PHASE2_SKIN         38594


enum Spells
{
    SPELL_ABILITATE_BALANCE_BAR = 98226, // self explanatory

    SPELL_OBSIDIAN_ARMOR = 98632, // 1% damage reduction per stack
    SPELL_CONCUSSIVE_STOMP = 97282, // simply his stomp, will spawn volcanoes
    SPELL_BURNING_FEET = 98837, // speedup after destroying a volcano
    SPELL_DRINK_MAGMA = 98034, // wipe incoming
    SPELL_MOLTEN_SPEW = 98043, // wipe assured 
    SPELL_IMMOLATION = 99846, // phase 2 aoe damage aura
    SPELL_HEATED_VOLCANO = 98493, // visual effect to make volcanoes erupt
    SPELL_SUPERHEATED = 101304,// +10% damage every 10 secs, pratically an enrage
    SPELL_VOLCANIC_BIRTH = 98010, // spawn volcanoes

    //VOLCANOES/Craters
    SPELL_VOLCANO_ACTIVE = 98264, // used after 3-4 seconds, random damage aoe
    SPELL_VOLCANO_ERUPTION = 98492, // every 2 secs per 20 secs, random damage aoe (more than the previous)
    SPELL_IGNITE_VOLCANO = 98493, // visual effect only
    SPELL_MOLTEN_ARMOR = 98255, // 2%/5% damage increase per stack (Normal/Heroic Mode)
    SPELL_CRATER_MAGMA = 98472, // aoe damage in lava
    SPELL_MAGMA_FLOW_GROUND = 97230, // aura that will cause signs on ground
    SPELL_MAGMA_FLOW_EXPLO = 97234, // burst of magma
    SPELL_MAGMA_FLOW_COSMETIC = 97225, // magma into the crater
    SPELL_VOLCANO_SMOKE = 97699, // volcano smoke 

    //Adds
    SPELL_SUMMON_SPARK = 98552, // 15k aoe damage and summon a spark
    SPELL_SUMMON_FRAGMENT = 98136, // 15k aoe damage and summon a fragment
    SPELL_SPARK_IMMOLATION = 98597, // aoe damage
    SPELL_SPARK_INFERNAL_RAGE = 98596, // +10% damage per stack
    SPELL_FRAGMENT_MELTDOWN = 98649, // damage equal to 100%/50% remaining health (10/25 player mode)

    //heroic
    SPELL_UNLEASHED_FLAME_AURA = 101313, // will trigger the next one
    SPELL_UNLEASHED_FLAME_DAMAGE = 100974, // damage spell, actually unused in script
    SPELL_UNLEASHED_FLAME_VISUAL = 86956,  // this is just the visual effect

};

enum Events
{
    EVENT_UPDATE_PATH = 1, // every 1 sec
    EVENT_STOMP,                            // roughtly every 20 seconds on phase 1
    EVENT_SPAWN_VOLCANO,                    // auto- esplicative, happens 3 secs after the previous
    EVENT_IGNITE_VOLCANO,                   // every 22 to 28 seconds   
    EVENT_CHECK_VOLCANO,                    // used by feet to check if there's a nearby volcano to shatter
    EVENT_DRINK_MAGMA,                      // enrage start
    EVENT_SPIT,                             // this will called after the previous
    EVENT_GETTING_BACK,                     // just after the spit, will make im get back to the platform (if the group survives)

    EVENT_SUPERHEATED,                      // 6/5 mins after start (Normal/Heroic mode)
    // questi due si alternano
    EVENT_SPAWN_FRAGMENTS,                  // used 20 - 22 secs after the next
    EVENT_SPAWN_SPARK,                      // used 20 - 22 secs after the previous
    EVENT_SWITCH_MODEL,                     // display id change on losing obsidian armor

    //Volcano\craters
    EVENT_VOLCANO_ACTIVE,                   // after a lil of seconds
    EVENT_VOLCANO_ERUPTION,                 // every 2 secs for 20 seconds
    EVENT_RADIATE,                          // used by the craters to spawn the 4-6 paths of fire
    EVENT_BLAZE,                            // used by the craters to explode all their nearby fire patches

    //Adds
    EVENT_INFERNAL_RAGE,                    // every 5 secs, will increase damage by 10%
    EVENT_MELTDOWN,                         // 30 seconds after spawn

    //Heroics
    EVENT_CHECK_RHYOLITH,                   // used by the liquid obsidian to check the distance from rhyolith
    EVENT_UNLEASHED_FLAME,                  // just an event to keep em moving in circle  
};

enum Actions
{
    //Core actions
    ACTION_STEER_RIGHT = 1, // from rightfoot to rhyolith, notice to turn direction 
    ACTION_STEER_LEFT,                      // from leftfoot to rhyolith, notice to turn direction 
    ACTION_LOSE_ARMOR,                      // from feet to rhyolith, used when crashing active volcanoes

    //volcano
    ACTION_ERUPT,                           // used by rhyolith to command one of his volcanoes

    //Heroics
    ACTION_ADD_ARMOR,                       // from obsidian to rhyolith, will also propagate to his feet
};

Position const centerPosition = { -371.52f, -318.9f, 100.3f, 0.0f };

Position const fourcircles[4] = { { -363.90f, -336.20f, 100.2f, 0.0f }, //43 y
{ -373.25f, -300.73f, 100.2f, 0.0f }, //51 y
{ -391.30f, -321.04f, 100.2f, 0.0f }, //40 y
{ -339.19f, -308.35f, 100.2f, 0.0f } };//27 y

class boss_rhyolith : public CreatureScript
{
public:
    boss_rhyolith() : CreatureScript("boss_rhyolith") {}

    struct boss_rhyolithAI : public BossAI
    {
        boss_rhyolithAI(Creature * creature) : BossAI(creature, DATA_RHYOLITH), summons(me), vehicle(creature->GetVehicleKit())
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;  //
        SummonList summons;
        Vehicle* vehicle;
        bool introDone;
        bool ambiturner;
        int32  direction;
        uint32 killtimer;
        bool moving;
        bool phase1;
        float ori;

        void InitializeAI()
        {
            phase1 = true;
            moving = false;
            introDone = false;
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            summon->setFaction(me->getFaction());
            if (summon->GetEntry() == LEFT_FOOT_RHYOLITH || summon->GetEntry() == RIGHT_FOOT_RHYOLITH)
            {
                summon->SetMaxHealth(me->GetMaxHealth() / 2);
                summon->SetHealth(summon->GetMaxHealth());
            }
            else if (summon->GetEntry() == NPC_MOVEMENT_CONTROLLER)
            {
                //summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                //summon->SetReactState(REACT_PASSIVE);
                summon->SetVisible(false);
                summon->setFaction(35);
                me->GetMotionMaster()->MoveChase(summon);
            }
            else if (summon->GetEntry() == NPC_LIQUID_OBSIDIAN)
            {//liquid obsidian will always follow him
                summon->GetMotionMaster()->MoveChase(me);
            }
            if (me->isInCombat())
                DoZoneInCombat(summon);
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            summons.RemoveNotExisting();
        }

        void EnterCombat(Unit* victim)
        {
            _EnterCombat();

            me->SetDisplayId(PHASE1_SKIN);
            me->MonsterYell(SAY_AGGRO, 0, 0);
            DoPlaySoundToSet(me, SOU_AGGRO);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            DoCastAOE(SPELL_ABILITATE_BALANCE_BAR);
            direction = 25;
            if (GameObject* bridge = me->FindNearestGameObject(209255, 500))
            {
                bridge->SetGoState(GO_STATE_ACTIVE);
            }
            if (instance)
            {
                instance->DoCastSpellOnPlayers(SPELL_ABILITATE_BALANCE_BAR);
                instance->SetData(DATA_RHYOLITH_EVENT, IN_PROGRESS);
                int32 value = 50 - direction;
                instance->DoSetPowerOnPlayers(POWER_ALTERNATE_POWER, value, 50);
            }
            phase1 = true;
            ambiturner = true;
            DoZoneInCombat(me);
            me->SetReactState(REACT_PASSIVE);
            ori = me->GetOrientation();
            me->AttackStop();
            moving = true;
            me->SetSpeed(MOVE_WALK, 0.5f, true);
            me->SetSpeed(MOVE_RUN, 0.5f, true);
            me->SetWalk(true);
            if (!me->HasAura(SPELL_OBSIDIAN_ARMOR))
            {//just a check to readd
                me->AddAura(SPELL_OBSIDIAN_ARMOR, me);
                if (Aura* obsidian = me->GetAura(SPELL_OBSIDIAN_ARMOR))
                    obsidian->SetStackAmount(80);
            }
            me->SummonCreature(NPC_MOVEMENT_CONTROLLER,
                me->GetPositionX(),
                me->GetPositionY(),
                me->GetPositionZ());
            if (Creature* leftfoot = me->SummonCreature(LEFT_FOOT_RHYOLITH,
                me->GetPositionX(),
                me->GetPositionY(),
                me->GetPositionZ() + 12.0f,
                0, TEMPSUMMON_MANUAL_DESPAWN))
            {
                leftfoot->EnterVehicle(me, 0);
                leftfoot->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                //leftfoot->ClearUnitState(UNIT_STATE_ONVEHICLE);
                leftfoot->AddAura(SPELL_OBSIDIAN_ARMOR, me);
                if (Aura* lobsidian = leftfoot->AddAura(SPELL_OBSIDIAN_ARMOR, leftfoot))
                    lobsidian->SetStackAmount(80);
            }
            if (Creature* rightfoot = me->SummonCreature(RIGHT_FOOT_RHYOLITH,
                me->GetPositionX(),
                me->GetPositionY(),
                me->GetPositionZ() + 12.0f,
                0, TEMPSUMMON_MANUAL_DESPAWN))
            {
                rightfoot->EnterVehicle(me, 1);
                rightfoot->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                //rightfoot->ClearUnitState(UNIT_STATE_ONVEHICLE);
                rightfoot->AddAura(SPELL_OBSIDIAN_ARMOR, me);
                if (Aura* robsidian = rightfoot->AddAura(SPELL_OBSIDIAN_ARMOR, rightfoot))
                    robsidian->SetStackAmount(80);
            }
            events.ScheduleEvent(EVENT_UPDATE_PATH, 500, 0, 0);
            events.ScheduleEvent(EVENT_STOMP, 18000, 0, 0);
            events.ScheduleEvent(EVENT_SUPERHEATED, RAID_MODE(360000, 360000, 300000, 300000)); //superheated, after 6 minutes in normal, 5 in heroic mode
            events.ScheduleEvent(EVENT_SWITCH_MODEL, 1000);
            if (roll_chance_i(50))
                events.ScheduleEvent(EVENT_SPAWN_FRAGMENTS, 15000, 0, 0);
            else
                events.ScheduleEvent(EVENT_SPAWN_SPARK, 15000, 0, 0);

            me->LowerPlayerDamageReq(me->GetHealth());
        }

        void Reset()
        {
            _Reset();

            me->SetDisplayId(PHASE1_SKIN);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->AddAura(SPELL_OBSIDIAN_ARMOR, me);
            if (Aura* obsidian = me->GetAura(SPELL_OBSIDIAN_ARMOR))
                obsidian->SetStackAmount(80);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ABILITATE_BALANCE_BAR);
            events.Reset();
            summons.DespawnAll();
            me->LowerPlayerDamageReq(me->GetMaxHealth());
            killtimer = 0;

            if (GameObject* bridge = me->FindNearestGameObject(209255, 500))
            {
                bridge->SetGoState(GO_STATE_READY);
            }
            if (instance)
            {
                instance->SetData(DATA_RHYOLITH_EVENT, NOT_STARTED);
            }

            me->LowerPlayerDamageReq(me->GetHealth());
        }

        void DoAction(int32 actionId)
        {
            switch (actionId)
            {
            case ACTION_STEER_LEFT:
                if (direction < 50 && instance)
                {
                    direction++;
                    int32 value = 50 - direction;
                    instance->DoSetPowerOnPlayers(POWER_ALTERNATE_POWER, value, 0);
                    if (direction > 28)
                    {// a small tolerance is given
                        ambiturner = false;
                    }
                }
                break;
            case ACTION_STEER_RIGHT:
                if (direction > 0 && instance)
                {
                    direction--;
                    int32 value = 50 - direction;
                    instance->DoSetPowerOnPlayers(POWER_ALTERNATE_POWER, value, 0);
                }
                break;
            case ACTION_LOSE_ARMOR:
            {
                int caso = urand(1, 5);
                switch (caso)
                {
                case 1:
                    me->MonsterYell(SAY_WEAK1, 0, 0);
                    DoPlaySoundToSet(me, SOU_WEAK1);
                    break;
                case 2:
                    me->MonsterYell(SAY_WEAK2, 0, 0);
                    DoPlaySoundToSet(me, SOU_WEAK2);
                    break;
                case 3:
                    me->MonsterYell(SAY_WEAK3, 0, 0);
                    DoPlaySoundToSet(me, SOU_WEAK3);
                    break;
                case 4:
                    me->MonsterYell(SAY_WEAK4, 0, 0);
                    DoPlaySoundToSet(me, SOU_WEAK4);
                    break;
                case 5:
                    me->MonsterYell(SAY_WEAK5, 0, 0);
                    DoPlaySoundToSet(me, SOU_WEAK5);
                    break;
                default:
                    break;
                }
                //DoCast(me, SPELL_BURNING_FEET, true);  //speedbuff
                uint32 stacks = 0;
                if (RAID_MODE(true, true, false, false))
                {//normal mode...
                    if (PRENERF4_2)
                    {//pre nerf, 10 stacks per active volcano
                        stacks = 10;
                    }
                    else
                    {//post nerf, 16 stacks per active volcano
                        stacks = 16;
                    }
                }
                else
                {//heroic mode, 10 stacks per active volcano
                    stacks = 10;
                }
                if (Aura* armor = me->GetAura(SPELL_OBSIDIAN_ARMOR))
                {
                    if (armor->GetStackAmount() <= stacks)
                    {//remove
                        armor->Remove();
                    }
                    else
                    {// just drop stacks
                        armor->SetStackAmount(armor->GetStackAmount() - stacks);
                    }
                }
                Creature* rightfoot = me->FindNearestCreature(RIGHT_FOOT_RHYOLITH, 100);
                Creature* leftfoot = me->FindNearestCreature(LEFT_FOOT_RHYOLITH, 100);
                if (rightfoot && leftfoot)
                {
                    if (Aura* rarmor = rightfoot->GetAura(SPELL_OBSIDIAN_ARMOR))
                    {
                        if (rarmor->GetStackAmount() <= stacks)
                        {//remove
                            rarmor->Remove();
                        }
                        else
                        {// just drop stacks
                            rarmor->SetStackAmount(rarmor->GetStackAmount() - stacks);
                        }
                    }
                    if (Aura* larmor = leftfoot->GetAura(SPELL_OBSIDIAN_ARMOR))
                    {

                        if (larmor->GetStackAmount() <= stacks)
                        {//remove
                            larmor->Remove();
                        }
                        else
                        {// just drop stacks
                            larmor->SetStackAmount(larmor->GetStackAmount() - stacks);
                        }
                    }
                }
                //Heroic Only
                if (me->GetMap()->IsHeroic())
                {
                    for (int i = 0; i< 5; i++)
                    {//Liquid obsidian will spawns on the borders
                        float dir = float(rand_norm())*static_cast<float>(2 * M_PI);
                        me->SummonCreature(NPC_LIQUID_OBSIDIAN,
                            centerPosition.GetPositionX() + 35.0f*cos(dir),
                            centerPosition.GetPositionY() + 42.0f*sin(dir),
                            me->GetPositionZ(), 0, TEMPSUMMON_MANUAL_DESPAWN);
                    }
                }

            }
            break;
            case ACTION_ADD_ARMOR:
            {// only used in heroic difficulty
                if (Aura* armor = me->GetAura(SPELL_OBSIDIAN_ARMOR))
                {// increase by 1 stack 
                    armor->SetStackAmount(armor->GetStackAmount() + 1);
                }
                else
                {// or add aura if not present
                    me->AddAura(SPELL_OBSIDIAN_ARMOR, me);
                }
                Creature* rightfoot = me->FindNearestCreature(RIGHT_FOOT_RHYOLITH, 100);
                Creature*  leftfoot = me->FindNearestCreature(LEFT_FOOT_RHYOLITH, 100);
                if (rightfoot && leftfoot)
                {
                    if (Aura* rarmor = rightfoot->GetAura(SPELL_OBSIDIAN_ARMOR))
                    {// increase by 1 stack 
                        rarmor->SetStackAmount(rarmor->GetStackAmount() + 1);
                    }
                    else
                    {// or add aura if not present
                        rightfoot->AddAura(SPELL_OBSIDIAN_ARMOR, rightfoot);
                    }
                    if (Aura* larmor = leftfoot->GetAura(SPELL_OBSIDIAN_ARMOR))
                    {// increase by 1 stack 
                        larmor->SetStackAmount(larmor->GetStackAmount() + 1);
                    }
                    else
                    {// or add aura if not present
                        leftfoot->AddAura(SPELL_OBSIDIAN_ARMOR, leftfoot);
                    }
                }
            }
            break;
            default:
                break;
            }
        }

        void DamageTaken(Unit* who, uint32 &damage)
        {
            if (!me || !me->isAlive())
                return;
        }

        void JustDied(Unit* killer)
        {
            _JustDied();

            me->LowerPlayerDamageReq(me->GetHealth());
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ABILITATE_BALANCE_BAR);
            me->MonsterYell(SAY_DEATH, 0, 0);
            DoPlaySoundToSet(me, SOU_DEATH);
            me->LowerPlayerDamageReq(me->GetMaxHealth());
            summons.DespawnAll();
            if (GameObject* bridge = me->FindNearestGameObject(209255, 500))
            {
                bridge->SetGoState(GO_STATE_READY);
            }
            if (me->GetMap()->IsHeroic())
            {// this will give the achievement to players in heroic difficulty
                Map::PlayerList const& PlayerList = instance->instance->GetPlayers();
                AchievementEntry const* rhyolith = sAchievementStore.LookupEntry(5808);
                for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                {
                    if (Player* player = itr->getSource())
                    {
                        player->CompletedAchievement(rhyolith);
                    }
                }
            }
            if (ambiturner)
            {//this will give the achievement to players if is a success
                Map::PlayerList const& PlayerList = instance->instance->GetPlayers();
                AchievementEntry const* rhyolith_lord = sAchievementStore.LookupEntry(5810);
                for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                {
                    if (Player* player = itr->getSource())
                    {//id dell'achievement ambiturner
                        player->CompletedAchievement(rhyolith_lord);
                    }
                }
            }
            instance->SetData(DATA_RHYOLITH_EVENT, DONE);
            instance->DoUpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, 53772, 1);
        }

        void KilledUnit(Unit* victim)
        {
            if (!victim || victim->GetTypeId() != TYPEID_PLAYER || killtimer > 0)
                return;
            int caso = urand(1, 3);
            switch (caso)
            {
            case 1:
                me->MonsterYell(SAY_KILL1, 0, 0);
                DoPlaySoundToSet(me, SOU_KILL1);
                break;
            case 2:
                me->MonsterYell(SAY_KILL2, 0, 0);
                DoPlaySoundToSet(me, SOU_KILL2);
                break;
            case 3:
                me->MonsterYell(SAY_KILL3, 0, 0);
                DoPlaySoundToSet(me, SOU_KILL3);
                break;
            default:
                break;
            }
            killtimer = 4000;
        }

        void UpdateAI(uint32 diff)
        {
            if (killtimer >= diff)
                killtimer -= diff;

            events.Update(diff);

            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (!me->getVictim())
                me->SetReactState(REACT_AGGRESSIVE);

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                case EVENT_STOMP:
                    if (!me->HasUnitState(UNIT_STATE_CASTING))
                        DoCast(SPELL_CONCUSSIVE_STOMP);
                    me->GetMotionMaster()->Clear();
                    me->StopMoving();
                    events.CancelEvent(EVENT_UPDATE_PATH);
                    events.ScheduleEvent(EVENT_STOMP, urand(25000, 35000));
                    events.ScheduleEvent(EVENT_SPAWN_VOLCANO, 3000);
                    events.ScheduleEvent(EVENT_UPDATE_PATH, 3100);
                    events.ScheduleEvent(EVENT_IGNITE_VOLCANO, 15000);
                    break;
                case EVENT_SUPERHEATED:
                {
                    me->CastSpell(me, SPELL_SUPERHEATED, false);
                    events.ScheduleEvent(SPELL_SUPERHEATED, 10000);
                }
                break;
                case EVENT_UNLEASHED_FLAME:
                {
                    for (int i = 0; i< 3; i++)
                    {//the 3 eyes wandering in the platform
                        float dir = float(rand_norm())*static_cast<float>(2 * M_PI);
                        float dist = float(rand_norm())*static_cast<float>(20.0f);
                        me->SummonCreature(NPC_UNLEASHED_FLAME,
                            me->GetPositionX() + dist*cos(dir),
                            me->GetPositionY() + dist*sin(dir),
                            me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 15000);
                    }
                    events.ScheduleEvent(EVENT_UNLEASHED_FLAME, 16000);
                }
                break;
                case EVENT_DRINK_MAGMA: //wipe incoming
                    me->GetMotionMaster()->Clear();
                    me->StopMoving();
                    me->CastSpell(me, SPELL_DRINK_MAGMA, false);
                    events.ScheduleEvent(EVENT_SPIT, 3100, 0, 0);
                    break;
                case EVENT_SPIT: //wipe assured
                    events.ScheduleEvent(EVENT_GETTING_BACK, 3000, 0, 0);
                    if (Unit* tempTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 200, true))
                    {
                        float xx = tempTarget->GetPositionX();
                        float yy = tempTarget->GetPositionY();
                        if (Creature* movementcontroller = me->FindNearestCreature(NPC_MOVEMENT_CONTROLLER, 200))
                        {//this will make him get back when spitting lava
                            movementcontroller->NearTeleportTo(xx, yy, me->GetPositionZ(), 0, false);
                            me->GetMotionMaster()->MoveChase(movementcontroller);
                        }
                    }
                    DoCast(SPELL_MOLTEN_SPEW);
                    break;
                case EVENT_GETTING_BACK:
                    if (Unit* tempTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 200, true))
                    {
                        float xx = tempTarget->GetPositionX();
                        float yy = tempTarget->GetPositionY();
                        if (Creature* movementcontroller = me->FindNearestCreature(NPC_MOVEMENT_CONTROLLER, 200))
                        {
                            movementcontroller->NearTeleportTo(xx, yy, me->GetPositionZ(), 0, false);
                            me->GetMotionMaster()->MoveChase(movementcontroller);
                        }
                        ori = me->GetOrientation();
                        direction = 25;
                        events.ScheduleEvent(EVENT_UPDATE_PATH, 2500, 0, 0);
                    }
                    break;
                case EVENT_SPAWN_SPARK:
                {
                    float dir = float(rand_norm())*static_cast<float>(2 * M_PI);
                    me->CastSpell(me->GetPositionX() + 12 * cos(dir), me->GetPositionY() + 12 * sin(dir), me->GetPositionZ(), SPELL_SUMMON_SPARK, true);
                    events.ScheduleEvent(EVENT_SPAWN_FRAGMENTS, urand(22000, 28000));
                }
                break;
                case EVENT_SPAWN_FRAGMENTS:
                {
                    float dir = float(rand_norm())*static_cast<float>(2 * M_PI);
                    me->CastSpell(me->GetPositionX() + 12 * cos(dir), me->GetPositionY() + 12 * sin(dir), me->GetPositionZ(), SPELL_SUMMON_FRAGMENT, true);
                    for (int i = 0; i< 4; i++)
                    {//the spell will summon only one, so there's the need to summon another 4 fragments
                        float dir2 = float(rand_norm())*static_cast<float>(2 * M_PI);
                        me->SummonCreature(NPC_FRAGMENT_RHYOLITH,
                            me->GetPositionX() + 12 * cos(dir) + 6 * cos(dir2),
                            me->GetPositionY() + 12 * sin(dir) + 6 * sin(dir2),
                            me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 31000);
                    }
                    events.ScheduleEvent(EVENT_SPAWN_SPARK, urand(22000, 28000));
                }
                break;
                case EVENT_SPAWN_VOLCANO:
                {
                    int caso = urand(1, 2);
                    switch (caso)
                    {
                    case 1:
                        me->MonsterYell(SAY_STOMP1, 0, 0);
                        DoPlaySoundToSet(me, SOU_STOMP1);
                        break;
                    case 2:
                        me->MonsterYell(SAY_STOMP2, 0, 0);
                        DoPlaySoundToSet(me, SOU_STOMP2);
                        break;
                    default:
                        break;
                    }
                    for (uint32 i = urand(0, 1); i<3; i++)
                    {//the pattern is elliptical
                        float dir = float(rand_norm())*static_cast<float>(2 * M_PI);
                        float dist = 25.0f * (float)rand_norm() + 15.0f;
                        Position pos;
                        me->GetNearPosition(pos, dist, dir);
                        me->CastSpell(
                            centerPosition.GetPositionX() + dist*cos(dir),
                            centerPosition.GetPositionY() + 1.2f * dist*sin(dir),
                            centerPosition.GetPositionZ(),
                            SPELL_VOLCANIC_BIRTH, false);

                    }
                }
                break;
                case EVENT_IGNITE_VOLCANO:
                {
                    //events.ScheduleEvent(EVENT_IGNITE_VOLCANO, urand (20000, 30000));
                    int caso = urand(1, 5);
                    switch (caso)
                    {
                    case 1:
                        me->MonsterYell(SAY_IGNI1, 0, 0);
                        DoPlaySoundToSet(me, SOU_IGNI1);
                        break;
                    case 2:
                        me->MonsterYell(SAY_IGNI2, 0, 0);
                        DoPlaySoundToSet(me, SOU_IGNI2);
                        break;
                    case 3:
                        me->MonsterYell(SAY_IGNI3, 0, 0);
                        DoPlaySoundToSet(me, SOU_IGNI3);
                        break;
                    case 4:
                        me->MonsterYell(SAY_IGNI4, 0, 0);
                        DoPlaySoundToSet(me, SOU_IGNI4);
                        break;
                    case 5:
                        me->MonsterYell(SAY_IGNI5, 0, 0);
                        DoPlaySoundToSet(me, SOU_IGNI5);
                        break;
                    default:
                        break;
                    }
                    if (Creature* Volc = me->FindNearestCreature(NPC_VOLCANO, 200, true))
                    {
                        Volc->GetAI()->DoAction(ACTION_ERUPT);
                        //DoCast(Volc, SPELL_HEATED_VOLCANO, true);
                    }
                }
                break;
                case EVENT_UPDATE_PATH:
                {
                    Creature* rightfoot = me->FindNearestCreature(RIGHT_FOOT_RHYOLITH, 100);
                    Creature*  leftfoot = me->FindNearestCreature(LEFT_FOOT_RHYOLITH, 100);
                    if (rightfoot && leftfoot)
                    {
                        if (rightfoot->GetHealth() + leftfoot->GetHealth() <= me->GetMaxHealth()* 0.255f && phase1)
                        {//trigger phase 2 at 25% health
                            me->SetHealth(rightfoot->GetHealth() + leftfoot->GetHealth());
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                            me->SetSpeed(MOVE_WALK, 1.5f, true);
                            me->SetSpeed(MOVE_RUN, 1.5f, true);
                            events.CancelEvent(EVENT_UPDATE_PATH);
                            events.CancelEvent(EVENT_SPAWN_SPARK);
                            events.CancelEvent(EVENT_SPAWN_FRAGMENTS);
                            events.CancelEvent(EVENT_IGNITE_VOLCANO);
                            events.CancelEvent(EVENT_STOMP);
                            events.CancelEvent(EVENT_SWITCH_MODEL);
                            if (RAID_MODE(false, false, true, true))
                            {
                                events.ScheduleEvent(EVENT_UNLEASHED_FLAME, 5000);
                            }
                            me->GetMotionMaster()->Clear();
                            me->SetStandState(UNIT_STAND_STATE_SIT);
                            me->SetReactState(REACT_AGGRESSIVE);
                            me->GetMotionMaster()->MoveChase(me->getVictim());
                            me->SetDisplayId(PHASE2_SKIN);  //skin seconda fase (fusa)
                            me->AddAura(SPELL_IMMOLATION, me);
                            me->MonsterYell(SAY_MOLTEN, 0, 0);
                            DoPlaySoundToSet(me, SOU_MOLTEN);
                            summons.DespawnAll();
                            if (Aura* armor = me->GetAura(SPELL_OBSIDIAN_ARMOR))
                                armor->Remove();
                            //despawn piedi
                            rightfoot->DespawnOrUnsummon();
                            leftfoot->DespawnOrUnsummon();
                            phase1 = false;
                        }
                        else if (rightfoot->GetHealth()* 1.05f < leftfoot->GetHealth() || leftfoot->GetHealth()* 1.05f < rightfoot->GetHealth())
                        {//if the difference in health of the two legs is more than 5% of the lowest, redistribute hp
                            uint32 newhealth = (rightfoot->GetHealth() + leftfoot->GetHealth()) / 2;
                            rightfoot->SetHealth(newhealth);
                            leftfoot->SetHealth(newhealth);
                        }
                    }
                    ori += M_PI * 2 + float(((int)direction - 25) * 0.001f) * MAXSTEERAMOUNT;
                    if (ori > M_PI * 2) ori -= M_PI * 2;
                    if (ori > M_PI * 2) ori -= M_PI * 2; // double check, it could be greater than 12.56
                    float x = me->GetPositionX() + cos(ori) * 40;
                    float y = me->GetPositionY() + sin(ori) * 40;
                    if (Creature* movementcontroller = me->FindNearestCreature(NPC_MOVEMENT_CONTROLLER, 200))
                    {
                        movementcontroller->NearTeleportTo(x, y, me->GetPositionZ(), 0, false);
                        me->GetMotionMaster()->MoveChase(movementcontroller);
                    }
                    if (direction >25)
                    {//la direzione tende a tornare a 25 (dritto) quanto più viene portata agli estremi (50 o 0)
                        if (direction  > 41)
                            direction -= 3;
                        else if (direction > 33)
                            direction -= 2;
                        else
                            direction--;
                    }
                    else if (direction <25)
                    {
                        if (direction < 9)
                            direction += 3;
                        else if (direction < 17)
                            direction += 2;
                        else
                            direction++;
                    }

                    if (instance)
                    {
                        int32 value = 50 - direction;
                        instance->DoSetPowerOnPlayers(POWER_ALTERNATE_POWER, value, 0);
                    }
                    if (me->GetExactDist2d(fourcircles[0].GetPositionX(), fourcircles[0].GetPositionY()) < 43.0f ||
                        me->GetExactDist2d(fourcircles[1].GetPositionX(), fourcircles[1].GetPositionY()) < 51.0f ||
                        me->GetExactDist2d(fourcircles[2].GetPositionX(), fourcircles[2].GetPositionY()) < 40.0f ||
                        me->GetExactDist2d(fourcircles[3].GetPositionX(), fourcircles[3].GetPositionY()) < 27.0f)
                    {//if he is inside one of the four circles, he is inside his platform
                        events.ScheduleEvent(EVENT_UPDATE_PATH, 1000, 0, 0);
                    }
                    else
                    {//drink magma!
                        me->GetMotionMaster()->Clear();
                        if (Creature* movementcontroller = me->FindNearestCreature(NPC_MOVEMENT_CONTROLLER, 200))
                        {
                            movementcontroller->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, false);
                            me->GetMotionMaster()->MoveChase(movementcontroller);
                            me->StopMoving();
                        }
                        events.ScheduleEvent(EVENT_DRINK_MAGMA, 100, 0, 0);
                        events.CancelEvent(EVENT_STOMP);
                        events.ScheduleEvent(EVENT_STOMP, 12000, 0, 0);
                    }
                }
                break;
                case EVENT_SWITCH_MODEL:
                {//change display id when lose armor
                    Creature* rightfoot = me->FindNearestCreature(RIGHT_FOOT_RHYOLITH, 100);
                    Creature* leftfoot = me->FindNearestCreature(LEFT_FOOT_RHYOLITH, 100);
                    if (rightfoot && leftfoot)
                    {
                        if (!rightfoot->HasAura(SPELL_OBSIDIAN_ARMOR))
                            return;

                        if (!leftfoot->HasAura(SPELL_OBSIDIAN_ARMOR))
                            return;

                        if (rightfoot->GetAura(SPELL_OBSIDIAN_ARMOR)->GetStackAmount() > 25 && me->GetAura(SPELL_OBSIDIAN_ARMOR)->GetStackAmount() < 50)
                        {
                            me->SetDisplayId(PHASE1_FIRST_SKIN);
                        }
                        else if (rightfoot->GetAura(SPELL_OBSIDIAN_ARMOR)->GetStackAmount() < 25)
                        {
                            me->SetDisplayId(PHASE1_SECOND_SKIN);
                        }
                        if (leftfoot->GetAura(SPELL_OBSIDIAN_ARMOR)->GetStackAmount() > 25 && me->GetAura(SPELL_OBSIDIAN_ARMOR)->GetStackAmount() < 50)
                        {
                            me->SetDisplayId(PHASE1_FIRST_SKIN);
                        }
                        else if (leftfoot->GetAura(SPELL_OBSIDIAN_ARMOR)->GetStackAmount() < 25)
                        {
                            me->SetDisplayId(PHASE1_SECOND_SKIN);
                        }
                        events.ScheduleEvent(EVENT_SWITCH_MODEL, 1000);
                    }
                }
                break;
                default:
                    break;
                }
            }

            if (!phase1)
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_rhyolithAI(creature);
    }
};

class right_foot_rhyolith : public CreatureScript
{
public:
    right_foot_rhyolith() : CreatureScript("right_foot_rhyolith"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new right_foot_rhyolithAI(creature);
    }

    struct right_foot_rhyolithAI : public ScriptedAI
    {
        right_foot_rhyolithAI(Creature *c) : ScriptedAI(c)
        {
            instance = me->GetInstanceScript();
            Reset();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            me->RemoveAllAuras();
            events.Reset();
        }

        void DamageTaken(Unit* who, uint32 &damage)
        {
            if (!me || !me->isAlive())
                return;
            //every hit that originally should have hit hard will cause rhyolith to steer
            //modificare la threshold dal define in cima al file
            uint32 treshold = DAMAGETHRESHOLD;
            uint32 stacks = 0;
            if (me->GetAura(SPELL_OBSIDIAN_ARMOR))
                stacks = me->GetAura(SPELL_OBSIDIAN_ARMOR)->GetStackAmount();
            treshold -= treshold / 100 * stacks;
            Creature* rhyolith = me->FindNearestCreature(BOSS_RHYOLITH, 100, true);
            if (damage >= treshold && rhyolith)
            {
                rhyolith->GetAI()->DoAction(ACTION_STEER_RIGHT); //steer that direction
            }
        }

        void EnterCombat(Unit * who)
        {
            DoZoneInCombat();
            events.ScheduleEvent(EVENT_CHECK_VOLCANO, 3000);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;
            if (!me->getVictim()) {}

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_CHECK_VOLCANO:
                    events.ScheduleEvent(EVENT_CHECK_VOLCANO, 500);
                    if (Creature* Volc = me->FindNearestCreature(NPC_VOLCANO, 7.0f))
                    {
                        if (Volc->HasAura(SPELL_VOLCANO_ACTIVE))
                        {//not erupting volcano, will give to rhyolith molten armor
                            if (Creature* rhyolith = me->FindNearestCreature(BOSS_RHYOLITH, 100, true))
                                rhyolith->AddAura(SPELL_MOLTEN_ARMOR, rhyolith);
                        }
                        else
                        {//if it was erupting it will remove some armor from rhyolith (and spawn a crater)
                            if (Creature* rhyolith = me->FindNearestCreature(BOSS_RHYOLITH, 100, true))
                                if (rhyolith->GetAI())
                                {
                                    rhyolith->GetAI()->DoAction(ACTION_LOSE_ARMOR);
                                    rhyolith->SummonCreature(NPC_CRATER, Volc->GetPositionX(), Volc->GetPositionY(), Volc->GetPositionZ(), 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 45000);
                                }
                        }
                        //aaaaand dies!
                        Volc->DespawnOrUnsummon();
                    }
                    break;
                default:
                    break;
                }
            }
        }

    };
};


class left_foot_rhyolith : public CreatureScript //      /tickle!
{
public:
    left_foot_rhyolith() : CreatureScript("left_foot_rhyolith"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new left_foot_rhyolithAI(creature);
    }

    struct left_foot_rhyolithAI : public ScriptedAI
    {
        left_foot_rhyolithAI(Creature *c) : ScriptedAI(c)
        {
            instance = me->GetInstanceScript();
            Reset();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            me->RemoveAllAuras();
            events.Reset();
        }

        void DamageTaken(Unit* who, uint32 &damage)
        {
            if (!me || !me->isAlive())
                return;
            // every hit that originally should have hit hard will cause rhyolith to steer
            //modificare la threshold dal define in cima al file
            uint32 treshold = DAMAGETHRESHOLD;
            uint32 stacks = 0;
            if (me->GetAura(SPELL_OBSIDIAN_ARMOR))
                stacks = me->GetAura(SPELL_OBSIDIAN_ARMOR)->GetStackAmount();
            treshold -= treshold / 100 * stacks;
            Creature* rhyolith = me->FindNearestCreature(BOSS_RHYOLITH, 100, true);
            if (damage >= treshold && rhyolith)
            {
                rhyolith->GetAI()->DoAction(ACTION_STEER_LEFT); //steer that direction
            }
        }

        void EnterCombat(Unit * who)
        {
            DoZoneInCombat();
            events.ScheduleEvent(EVENT_CHECK_VOLCANO, 3000);
        }

        void DoAction(int32 actionId)
        {
            switch (actionId)
            {
            case EVENT_CHECK_VOLCANO:
                events.ScheduleEvent(EVENT_CHECK_VOLCANO, 500);
                if (Creature* Volc = me->FindNearestCreature(NPC_VOLCANO, 7.0f))
                {
                    if (Volc->HasAura(SPELL_VOLCANO_ACTIVE))
                    {//not erupting volcano, will give to rhyolith molten armor
                        if (Creature* rhyolith = me->FindNearestCreature(BOSS_RHYOLITH, 100, true))
                            rhyolith->AddAura(SPELL_MOLTEN_ARMOR, rhyolith);
                    }
                    else
                    {//if it was erupting it will remove some armor from rhyolith (and spawn a crater)
                        if (Creature* rhyolith = me->FindNearestCreature(BOSS_RHYOLITH, 100, true))
                            if (rhyolith->GetAI())
                            {
                                rhyolith->GetAI()->DoAction(ACTION_LOSE_ARMOR);
                                rhyolith->SummonCreature(NPC_CRATER, Volc->GetPositionX(), Volc->GetPositionY(), Volc->GetPositionZ(), 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 45000);
                            }
                    }
                    //aaaaand dies!
                    Volc->DespawnOrUnsummon();
                }
                break;
            default:
                break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;
            if (!me->getVictim()) {}

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_CHECK_VOLCANO:
                    events.ScheduleEvent(EVENT_CHECK_VOLCANO, 500);
                    if (Creature* Volc = me->FindNearestCreature(NPC_VOLCANO, 50))
                    {
                        if (me->GetExactDist2d(Volc) <= 5.0f)
                        {
                            if (Volc->HasAura(SPELL_VOLCANO_ACTIVE))
                            {//not erupting volcano, will give to rhyolith molten armor
                                if (Creature* rhyolith = me->FindNearestCreature(BOSS_RHYOLITH, 100, true))
                                    rhyolith->AddAura(SPELL_MOLTEN_ARMOR, rhyolith);
                            }
                            else
                            {//if it was erupting it will remove some armor from rhyolith (and spawn a crater)
                                if (Creature* rhyolith = me->FindNearestCreature(BOSS_RHYOLITH, 100, true))
                                    if (rhyolith->GetAI())
                                    {
                                        rhyolith->GetAI()->DoAction(ACTION_LOSE_ARMOR);
                                    }
                            }
                            me->SummonCreature(NPC_CRATER, Volc->GetPositionX(), Volc->GetPositionY(), Volc->GetPositionZ(), 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 45000);
                            //aaaaand dies!
                            Volc->DespawnOrUnsummon();
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }
    };
};

class npc_rhyolith_volcano : public CreatureScript //Un - Shattered Volcano
{
public:
    npc_rhyolith_volcano() : CreatureScript("npc_rhyolith_volcano"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rhyolith_volcanoAI(creature);
    }

    struct npc_rhyolith_volcanoAI : public ScriptedAI
    {
        npc_rhyolith_volcanoAI(Creature *c) : ScriptedAI(c)
        {
        }

        EventMap events;

        void InitializeAI()
        {
            events.Reset();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->AddAura(SPELL_VOLCANO_SMOKE, me);
        }

        void EnterCombat(Unit * who)
        {
            events.ScheduleEvent(EVENT_VOLCANO_ACTIVE, urand(3000, 12000));
        }

        void DoAction(int32 actionId)
        {
            switch (actionId)
            {
            case ACTION_ERUPT:
                if (Aura* active = me->GetAura(SPELL_VOLCANO_ACTIVE))
                    active->Remove();
                events.ScheduleEvent(EVENT_VOLCANO_ERUPTION, 2000);
                events.ScheduleEvent(EVENT_VOLCANO_ACTIVE, 21000);
                break;
            default:
                break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;
            if (!me->getVictim()) {}

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_VOLCANO_ACTIVE:
                    me->AddAura(SPELL_VOLCANO_ACTIVE, me);
                    me->RemoveAura(SPELL_VOLCANO_SMOKE);
                    events.CancelEvent(EVENT_VOLCANO_ERUPTION);
                    break;
                case EVENT_VOLCANO_ERUPTION:
                    for (int i = 0; i< RAID_MODE(3, 6, 3, 6); i++)
                    {
                        if (Unit* tempTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 500, true))
                        {
                            DoCast(tempTarget, SPELL_VOLCANO_ERUPTION, true);
                        }
                    }
                    events.ScheduleEvent(EVENT_VOLCANO_ERUPTION, 2000);
                    break;
                default:
                    break;
                }
            }
        }
    };
};

class npc_rhyolith_crater : public CreatureScript //Shattered Volcano
{
public:
    npc_rhyolith_crater() : CreatureScript("npc_rhyolith_crater"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rhyolith_craterAI(creature);
    }

    struct npc_rhyolith_craterAI : public ScriptedAI
    {
        npc_rhyolith_craterAI(Creature *c) : ScriptedAI(c), summons(me)
        {

        }

        uint32 count;
        float dir;
        float offset;
        bool asc;
        uint32 num;
        SummonList summons;
        EventMap events;


        void Reset()
        {
            events.Reset();
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            summon->SetReactState(REACT_PASSIVE);
            if (me->isInCombat())
                DoZoneInCombat(summon);
            summon->setFaction(me->getFaction());
        }

        void EnterCombat(Unit* who)
        {
            count = 3;
            offset = 4;
            asc = false;
            num = urand(4, 6);
            dir = float(rand_norm())*static_cast<float>(2 * M_PI);
            events.ScheduleEvent(EVENT_RADIATE, 10000);
            me->AddAura(SPELL_CRATER_MAGMA, me);
            me->CastSpell(me, SPELL_MAGMA_FLOW_COSMETIC);
        }

        void InitializeAI()
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;
            if (!me->getVictim()) {}

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_RADIATE://NON VA TOCCATO DA QUI
                    count++;
                    for (uint32 i = 0; i<num; i++)
                    {
                        //dir is increased by 90° for 4patches, by 72° for 5 and 60° for 6
                        float newdir = dir + (2 * M_PI)*i / num;
                        if (newdir >= (2 * M_PI))
                            newdir -= (2 * M_PI);
                        float pdist = offset < 0 ? -sqrt(-offset) : sqrt(offset);
                        if (Creature* magma = me->SummonCreature(NPC_MOLTEN_ERUPTION,
                            me->GetPositionX() + count * 2.0f *cos(newdir) + pdist *sin(newdir),
                            me->GetPositionY() + count * 2.0f *sin(newdir) - pdist *cos(newdir),
                            me->GetMap()->GetHeight(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()),
                            0, TEMPSUMMON_TIMED_DESPAWN, 10500))
                        {
                            magma->CastSpell(magma, SPELL_MAGMA_FLOW_GROUND, true);
                            magma->SetReactState(REACT_PASSIVE);
                            magma->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
                        }
                    }
                    if (offset == 4)
                        asc = false;
                    else if (offset == -4)
                        asc = true;
                    if (asc)
                        offset++;
                    else
                        offset--;
                    if (count < 20) //patches will go up to 40y away from the center
                        events.ScheduleEvent(EVENT_RADIATE, 600);
                    else
                        events.ScheduleEvent(EVENT_BLAZE, 500);
                    //A QUI
                    break;
                case EVENT_BLAZE:
                    if (!summons.empty())
                    {
                        for (std::list<uint64>::const_iterator itr = summons.begin(); itr != summons.end(); ++itr)
                        {
                            if (Creature* minion = Unit::GetCreature(*me, *itr))
                            {
                                me->CastSpell(minion->GetPositionX(), minion->GetPositionY(), minion->GetPositionZ(), SPELL_MAGMA_FLOW_EXPLO, true);
                                minion->DespawnOrUnsummon();
                            }
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }
    };
};

class npc_rhyolith_spark : public CreatureScript
{
public:
    npc_rhyolith_spark() : CreatureScript("npc_rhyolith_spark"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rhyolith_sparkAI(creature);
    }

    struct npc_rhyolith_sparkAI : public ScriptedAI
    {
        npc_rhyolith_sparkAI(Creature *c) : ScriptedAI(c)
        {
            instance = me->GetInstanceScript();
            Reset();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            me->RemoveAllAuras();
            events.Reset();
        }

        void EnterCombat(Unit * who)
        {
            DoZoneInCombat();
            DoCast(me, SPELL_SPARK_IMMOLATION, true);
            events.ScheduleEvent(EVENT_INFERNAL_RAGE, 5000);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;
            if (!me->getVictim()) {}

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_INFERNAL_RAGE:
                    me->CastSpell(me, SPELL_SPARK_INFERNAL_RAGE, false);
                    events.ScheduleEvent(EVENT_INFERNAL_RAGE, 5000);
                    break;
                default:
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class npc_rhyolith_fragment : public CreatureScript
{
public:
    npc_rhyolith_fragment() : CreatureScript("npc_rhyolith_fragment"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rhyolith_fragmentAI(creature);
    }

    struct npc_rhyolith_fragmentAI : public ScriptedAI
    {
        npc_rhyolith_fragmentAI(Creature *c) : ScriptedAI(c)
        {
            instance = me->GetInstanceScript();
            Reset();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            me->RemoveAllAuras();
            events.Reset();
        }

        void EnterCombat(Unit * who)
        {
            DoZoneInCombat();
            events.ScheduleEvent(EVENT_MELTDOWN, 30000);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;
            if (!me->getVictim()) {}

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_MELTDOWN:
                    if (Unit* tempTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 500, true))
                    {//calculations for taking the correct damage value from health remaining needed
                        DoCast(tempTarget, SPELL_FRAGMENT_MELTDOWN, true);
                    }
                    break;
                default:
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class npc_liquid_obsidian : public CreatureScript
{
public:
    npc_liquid_obsidian() : CreatureScript("npc_liquid_obsidian"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_liquid_obsidianAI(creature);
    }

    struct npc_liquid_obsidianAI : public ScriptedAI
    {
        npc_liquid_obsidianAI(Creature *c) : ScriptedAI(c)
        {
            instance = me->GetInstanceScript();
            Reset();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            me->RemoveAllAuras();
            events.Reset();
        }

        void EnterCombat(Unit * who)
        {
            DoZoneInCombat();
            me->SetReactState(REACT_PASSIVE);
            me->SetSpeed(MOVE_WALK, 0.64f, true);
            me->SetSpeed(MOVE_RUN, 0.64f, true);
            me->SetWalk(true);
            events.ScheduleEvent(EVENT_CHECK_RHYOLITH, 500);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;
            if (!me->getVictim()) {}

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_CHECK_RHYOLITH:
                    if (Creature* rhyolith = me->FindNearestCreature(BOSS_RHYOLITH, 100, true))
                    {//if near rhyolith will merge
                        if (me->GetExactDist2d(rhyolith) <= 15.0f && rhyolith->GetAI())
                        {
                            rhyolith->GetAI()->DoAction(ACTION_ADD_ARMOR);
                            me->DespawnOrUnsummon();
                        }
                        else
                        {
                            events.ScheduleEvent(EVENT_CHECK_RHYOLITH, 500);
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }

    };
};

class npc_unleashed_flame : public CreatureScript
{
public:
    npc_unleashed_flame() : CreatureScript("npc_unleashed_flame"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_unleashed_flameAI(creature);
    }

    struct npc_unleashed_flameAI : public ScriptedAI
    {
        npc_unleashed_flameAI(Creature *c) : ScriptedAI(c)
        {
        }

        float ori;
        float distance;
        uint32 timer;
        Position Spawnpoint;

        void InitializeAI()
        {
            Spawnpoint.Relocate(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }

        void EnterCombat(Unit* target)
        {
            me->SetSpeed(MOVE_WALK, 1.5f, true);
            me->SetSpeed(MOVE_RUN, 1.5f, true);
            ori = 0;
            distance = 20.0f;
            timer = 100;
            me->AddAura(SPELL_UNLEASHED_FLAME_AURA, me);
            if (Creature* rhyolith = me->FindNearestCreature(BOSS_RHYOLITH, 100, true))
            {
                me->CastSpell(rhyolith, SPELL_UNLEASHED_FLAME_VISUAL, true);
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;
            if (timer <= diff)
            {
                timer = 250;
                ori += static_cast<float>(M_PI* 0.5);
                if (ori >= 2 * M_PI)
                    ori -= 2 * M_PI;
                float X = Spawnpoint.GetPositionX() + distance*cos(ori);
                float Y = Spawnpoint.GetPositionY() + distance*sin(ori);
                me->GetMotionMaster()->MovePoint(0, X, Y, me->GetPositionZ());
            }
            else
                timer -= diff;
        }
    };
};

class spell_meltdown : public SpellScriptLoader
{
public:
    spell_meltdown() : SpellScriptLoader("spell_meltdown") { }

    SpellScript* GetSpellScript() const
    {
        return new spell_meltdown_SpellScript();
    }

    class spell_meltdown_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_meltdown_SpellScript);

        void CalculateDamage(SpellEffIndex /*effIndex*/)
        {
            if (Unit* caster = GetCaster())
            {
                int32 damage = caster->GetHealth();
                if (caster->GetMap() && (caster->GetMap()->GetDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL || caster->GetMap()->GetDifficulty() == RAID_DIFFICULTY_25MAN_HEROIC))
                {
                    damage /= 2;
                }
                SetHitDamage(damage);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_meltdown_SpellScript::CalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };
};

void AddSC_boss_rhyolith()
{
    new boss_rhyolith();
    new right_foot_rhyolith();
    new left_foot_rhyolith();
    new npc_rhyolith_volcano();
    new npc_rhyolith_crater();
    new npc_rhyolith_spark();
    new npc_rhyolith_fragment();
    new npc_liquid_obsidian();
    new npc_unleashed_flame();
    new spell_meltdown();
}