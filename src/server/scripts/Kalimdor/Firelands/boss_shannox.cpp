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
Author: Nagash (for now)
Progress: 80%
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

#define SAY_INTRO    "Yes... I smell them too, Riplimb. Outsiders encroach on the Firelord's private grounds. Find their trail, find them for me, that I may dispense punishment!"
#define SOU_INTRO    24584
#define SAY_AGGRO    "Aha! The interlopers... Kill them! EAT THEM!" //24565
#define SOU_AGGRO    24565
#define SAY_MAGM1    "Now you BURN!!" //24576
#define SOU_MAGM1    24576
#define SAY_MAGM2    "Twist in flames, interlopers!" //24577
#define SOU_MAGM2    24577
#define SAY_FACERAGE "Go for the throat!" //24573
#define SOU_FACERAGE 24573
#define SAY_RIPDEAD  "Riplimb! No... no! Oh, you terrible little beasts! HOW COULD YOU?!" //24574
#define SOU_RIPDEAD  24574
#define SAY_RAGEDEAD "You murderers! Why... why would you kill such a noble animal?!" //24575
#define SOU_RAGEDEAD 24575
#define SAY_DEATH    "The pain... Lord of fire, it hurts..." //24568
#define SOU_DEATH    24568
#define SAY_KILL1    "Yes... oh yes!"  //24581
#define SOU_KILL1    24581
#define SAY_KILL2    "The Firelord will be most pleased!" //24580
#define SOU_KILL2    24580
#define SAY_KILL3    "Now you stay dead!" //24579
#define SOU_KILL3    24579
#define SAY_KILL4    "Dog food!" //24578
#define SOU_KILL4    24578
/*
Dog's Command
KILL!
Tear them down!
Sic 'em!
Spear Toss
Fetch your supper!
Disengage
Step off!
Back! Stay back!
Get away from me, outsiders!
Back, filth!*/



enum Spells
{
    //Shannox
    SPELL_ARCTIC_SLASH_10N = 99931,
    SPELL_ARCTIC_SLASH_25N = 101201,
    SPELL_ARCTIC_SLASH_10H = 101202,
    SPELL_ARCTIC_SLASH_25H = 101203,

    SPELL_BERSERK = 26662,

    SPELL_CALL_SPEAR = 100663,
    SPELL_HURL_SPEAR = 100002,   // Dummy Effect & Damage
    SPELL_SPEAR_VISUAL = 100005,   // Used to make the spear visible
    SPELL_HURL_SPEAR_SUMMON = 99978,    // Summons the Spear... Useless
    SPELL_HURL_SPEAR_DUMMY_SCRIPT = 100031,
    SPELL_MAGMA_RUPTURE_SHANNOX = 99840,

    SPELL_FRENZY_SHANNOX = 23537,
    SPELL_IMMOLATION_TRAP = 52606,

    // Riplimb
    SPELL_LIMB_RIP = 99832,
    SPELL_DOGGED_DETERMINATION = 101111,

    // Rageface
    SPELL_FACE_RAGE = 99947,
    SPELL_FACE_RAGE_BUFF = 100129,

    // Both Dogs
    SPELL_FRENZIED_DEVOLUTION = 100064,
    SPELL_FEEDING_FRENZY_TRIGGER = 100655,

    SPELL_WARY_10N = 100167, // Buff when the Dog gets in a Trap
    SPELL_WARY_25N = 101215,
    SPELL_WARY_10H = 101216,
    SPELL_WARY_25H = 101217,

    // Misc
    SPELL_SEPERATION_ANXIETY = 99835,

    //Spear Abilities
    SPELL_MAGMA_FLARE = 100495, // Fire damage within 50 yards.
    SPELL_MAGMA_RUPTURE = 100003, // Calls forth magma eruptions to damage nearby foes.
    SPELL_MAGMA_RUPTURE_VISUAL = 99841,

    //Traps Abilities
    CRYSTAL_PRISON_EFFECT = 99837,
    IMMOLATION_TRAP = 99838,
};

Position const bucketListPositions[5] = { { 0.0f, 0.0f, 0.0f, 0.0f },
{ 0.0f, 0.0f, 0.0f, 0.0f },
{ 0.0f, 0.0f, 0.0f, 0.0f },
{ 0.0f, 0.0f, 0.0f, 0.0f },
{ 0.0f, 0.0f, 0.0f, 0.0f } };

// Dogs Walking Distance to Shannox
const float walkRagefaceDistance = 5;
const float walkRagefaceAngle = 6.3f;
const float walkRiplimbDistance = 5;
const float walkRiplimbAngle = 6;

// If the Distance between the three > This Value, they get Separation Anxiety
const float maxDistanceBetweenShannoxAndDogs = 60;

enum Events
{
    //Shannox
    EVENT_IMMOLATION_TRAP = 1, // Every 10s
    EVENT_BERSERK,                                   // After 10m
    EVENT_ARCING_SLASH,                              // Every 12s
    EVENT_SUMMON_SPEAR,
    EVENT_SUMMON_CRYSTAL_PRISON,                     // Every 25s
    EVENT_HURL_SPEAR_OR_MAGMA_RUPTURE,               // Every 42s
    EVENT_HURL_SPEAR_2,                              // Used to call for the spear
    EVENT_HURL_SPEAR_3,                              // Unused for getting back riplimb

    //Riplimb
    EVENT_LIMB_RIP, // i Dont know...
    EVENT_RIPLIMB_RESPAWN_H,
    EVENT_TAKING_SPEAR_DELAY,
    EVENT_REPOP,

    //Rageface
    EVENT_FACE_RAGE, // Every 31s
    EVENT_LEAP,
    EVENT_CHANGE_TARGET,

    // Trigger for the Crystal Trap
    EVENT_CRYSTAL_TRAP_TRIGGER = 25,
    EVENT_IMMOLATION_TRAP_TRIGGER = 25,

    // Trigger for self Dispawn (Crystal Prison and Immolation Trap)
    EVENT_CRYSTAL_PRISON_DESPAWN = 26,
    EVENT_IMMOLATION_TRAP_DESPAWN = 26,
};

enum Actions
{
    ACTION_RETRIEVE_SPEAR = 0,
    ACTION_TAKE_SPEAR,
    ACTION_RETURN_SPEAR,
};

class boss_shannox : public CreatureScript
{
public:
    boss_shannox() : CreatureScript("boss_shannox") {}

    struct boss_shannoxAI : public BossAI
    {
        boss_shannoxAI(Creature * creature) : BossAI(creature, DATA_SHANNOX), summons(me)
        {
            instance = me->GetInstanceScript();

            softEnrage = false;
            riplimbIsRespawning = false;

            Reset();
        }

        InstanceScript* instance;
        bool softEnrage;
        bool riplimbIsRespawning;
        bool bucketListCheckPoints[5];
        bool hasSpear;
        SummonList summons;
        uint32 killtimer;
        bool introDone;

        void DespawnCreatures(uint32 entry, float distance)
        {
            std::list<Creature*> creatures;
            GetCreatureListWithEntryInGrid(creatures, me, entry, distance);

            if (creatures.empty())
                return;

            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                (*iter)->DespawnOrUnsummon();
        }

        void RespawnMob(uint32 entry)
        {
            std::list<Creature*> creatures;
            GetCreatureListWithEntryInGrid(creatures, me, entry, 100.0f);

            if (creatures.empty())
                return;

            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
            {
                if ((*iter))
                    (*iter)->Respawn();
            }
        }

        void InitializeAI()
        {
            introDone = false;
            SetEquipmentSlots(false, 53000, 0, 0);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            summon->setFaction(me->getFaction());
            summon->setActive(true);

            if (me->isInCombat())
                DoZoneInCombat(summon);
        }

        void Reset()
        {
            _Reset();
            summons.DespawnAll();
            me->LowerPlayerDamageReq(me->GetMaxHealth());
            killtimer = 0;
            me->RemoveAllAuras();
            softEnrage = false;
            riplimbIsRespawning = false;
            hasSpear = true;
            instance->DoRemoveAurasDueToSpellOnPlayers(CRYSTAL_PRISON_EFFECT);
            events.Reset();

            RespawnMob(NPC_RIPLIMB);
            RespawnMob(NPC_RAGEFACE);

            //DespawnCreatures(NPC_CRYSTAL_PRISON, 300.0f);
            //DespawnCreatures(NPC_CRYSTAL_TRAP, 300.0f);

            if (instance)
                instance->SetData(DATA_SHANNOX_EVENT, NOT_STARTED);
        }

        void DoAction(int32 actionId)
        {
            switch (actionId)
            {
            case ACTION_TAKE_SPEAR:
                events.CancelEvent(EVENT_HURL_SPEAR_OR_MAGMA_RUPTURE); //just to be sure everything doesn't interferee with the postponing
                events.ScheduleEvent(EVENT_HURL_SPEAR_OR_MAGMA_RUPTURE, urand(15000, 25000));
                SetEquipmentSlots(true);
                hasSpear = true;
                break;
            }
        }

        void DamageTaken(Unit* who, uint32 &damage)
        {
            // if he goes under 30% hp dogs will enrage
            if (!me || !me->isAlive())
                return;

            if (!instance)
                return;

            if (!me->GetMap()->IsHeroic())
            {
                if (me->HealthAbovePct(30) && (me->GetMaxHealth() *0.3f) > (me->GetHealth() - damage))
                {
                    if (Creature* pRiplimb = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_RIPLIMB)))
                        if (pRiplimb->isAlive())
                            pRiplimb->AddAura(SPELL_FRENZIED_DEVOLUTION, pRiplimb);

                    if (Creature* pRageface = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_RAGEFACE)))
                        if (pRageface->isAlive())
                            pRageface->AddAura(SPELL_FRENZIED_DEVOLUTION, pRageface);
                }
            }
        }

        void EnterCombat(Unit* victim)
        {
            _EnterCombat();
            if (!victim)
                return;

            if (!instance)
                return;

            me->MonsterYell(SAY_AGGRO, 0, 0);
            DoPlaySoundToSet(me, SOU_AGGRO);
            DoZoneInCombat(me);
            me->CallForHelp(20);

            if (Creature* pRiplimb = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_RIPLIMB)))
                DoZoneInCombat(pRiplimb);

            if (Creature* pRageface = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_RAGEFACE)))
                DoZoneInCombat(pRageface);

            if (instance)
                instance->SetData(DATA_SHANNOX_EVENT, IN_PROGRESS);

            events.ScheduleEvent(EVENT_IMMOLATION_TRAP, 10000);
            events.ScheduleEvent(EVENT_ARCING_SLASH, 12000);
            events.ScheduleEvent(EVENT_HURL_SPEAR_OR_MAGMA_RUPTURE, 20000);
            events.ScheduleEvent(EVENT_SUMMON_CRYSTAL_PRISON, 25000);
            events.ScheduleEvent(EVENT_BERSERK, 10 * MINUTE * IN_MILLISECONDS);
        }

        void JustDied(Unit* killer)
        {
            _JustDied();

            me->MonsterYell(SAY_DEATH, 0, 0);
            DoPlaySoundToSet(me, SOU_DEATH);
            me->LowerPlayerDamageReq(me->GetMaxHealth());
            summons.DespawnAll();
            instance->SetData(DATA_SHANNOX_EVENT, DONE);
            instance->DoRemoveAurasDueToSpellOnPlayers(CRYSTAL_PRISON_EFFECT);
            if (me->GetMap()->IsHeroic())
            {// this will give the achievement to players in heroic difficulty
                Map::PlayerList const& PlayerList = instance->instance->GetPlayers();
                AchievementEntry const* shannox = sAchievementStore.LookupEntry(5806);
                for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                {
                    if (Player* player = itr->getSource())
                    {
                        player->CompletedAchievement(shannox);
                    }
                }
            }
            if (Creature* pRiplimb = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_RIPLIMB)))
                pRiplimb->DespawnOrUnsummon();

            if (Creature* pRageface = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_RAGEFACE)))
                pRageface->DespawnOrUnsummon();
        }

        void KilledUnit(Unit* victim)
        {
            if (!victim || victim->GetTypeId() != TYPEID_PLAYER || killtimer > 0)
                return;
            int caso = urand(1, 4);
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
            case 4:
                me->MonsterYell(SAY_KILL4, 0, 0);
                DoPlaySoundToSet(me, SOU_KILL4);
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

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                case EVENT_BERSERK:
                    DoCast(me, SPELL_BERSERK);
                    break;
                case EVENT_ARCING_SLASH:
                    if (!hasSpear) //he doesn't have his spear
                    {
                        events.ScheduleEvent(EVENT_ARCING_SLASH, 1000);
                    }
                    else
                    {
                        DoCastVictim(RAID_MODE(SPELL_ARCTIC_SLASH_10N, SPELL_ARCTIC_SLASH_25N, SPELL_ARCTIC_SLASH_10H, SPELL_ARCTIC_SLASH_25H));
                        events.ScheduleEvent(EVENT_ARCING_SLASH, urand(6000, 10000));
                    }
                    break;
                case EVENT_HURL_SPEAR_OR_MAGMA_RUPTURE:
                    if (Creature* pRiplimb = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_RIPLIMB)))
                    {
                        if (pRiplimb->isDead())
                        {
                            // If riplimb is dead will cast magma rupture
                            DoCastVictim(SPELL_MAGMA_RUPTURE_SHANNOX);
                            events.ScheduleEvent(EVENT_HURL_SPEAR_OR_MAGMA_RUPTURE, urand(13000, 17000));
                            if (!hasSpear)
                                hasSpear = true;
                        }
                        else
                        {
                            // instead if riplimb is alive he will throw his spear
                            if (hasSpear)
                            {
                                events.ScheduleEvent(EVENT_HURL_SPEAR_OR_MAGMA_RUPTURE, 42000);
                                events.ScheduleEvent(EVENT_HURL_SPEAR_2, 5000); // this will make riplimb go searching for the spear

                                if (pRiplimb)
                                {
                                    Creature *spear = me->SummonCreature(NPC_SHANNOX_SPEAR,
                                        pRiplimb->GetPositionX()/*+(urand(0,20)-10)*/,
                                        pRiplimb->GetPositionY()/*+(urand(0,20)-10)*/,
                                        pRiplimb->GetPositionZ(),
                                        TEMPSUMMON_MANUAL_DESPAWN);
                                    me->CastSpell(spear, SPELL_HURL_SPEAR, false);
                                }

                                SetEquipmentSlots(false, 0, 0, 0);
                                hasSpear = false;
                            }
                            else
                            {//everything postponed if he has not his spear
                                events.ScheduleEvent(EVENT_HURL_SPEAR_OR_MAGMA_RUPTURE, urand(10000, 12000));
                            }

                        }
                    }
                    break;
                case EVENT_HURL_SPEAR_2:
                    if (Creature* pRiplimb = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_RIPLIMB)))
                        if (pRiplimb->GetAI())
                            pRiplimb->GetAI()->DoAction(ACTION_RETRIEVE_SPEAR);
                    break;
                case EVENT_SUMMON_CRYSTAL_PRISON:
                    if (Unit* tempTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, -5, true))
                        me->SummonCreature(NPC_CRYSTAL_TRAP,
                        tempTarget->GetPositionX(),
                        tempTarget->GetPositionY(),
                        tempTarget->GetPositionZ(),
                        TEMPSUMMON_MANUAL_DESPAWN);
                    events.ScheduleEvent(EVENT_SUMMON_CRYSTAL_PRISON, urand(23000, 27000));
                    break;
                case EVENT_IMMOLATION_TRAP:
                    if (Unit* tempTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 500, true))
                        me->SummonCreature(NPC_IMMOLATION_TRAP,
                        tempTarget->GetPositionX(),
                        tempTarget->GetPositionY(),
                        tempTarget->GetPositionZ(),
                        TEMPSUMMON_MANUAL_DESPAWN);
                    events.ScheduleEvent(EVENT_IMMOLATION_TRAP, urand(10000, 35000));
                    break;
                case EVENT_RIPLIMB_RESPAWN_H:
                    riplimbIsRespawning = false;
                    if (Creature* pRiplimb = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_RIPLIMB)))
                        pRiplimb->Respawn();
                    DoZoneInCombat();
                    break;
                default:
                    break;
                }
            }

            if (Creature* pRiplimb = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_RIPLIMB)))
            {
                if (Creature* pRageface = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_RAGEFACE)))
                {
                    if ((pRiplimb->isDead() || pRageface->isDead()) && !softEnrage)
                    {
                        // Heroic: Respawn Riplimb 30s after he is Death
                        if (pRiplimb->isDead() && RAID_MODE(false, false, true, true) && (!riplimbIsRespawning))
                        {
                            riplimbIsRespawning = true;
                            events.ScheduleEvent(EVENT_RIPLIMB_RESPAWN_H, 30000);
                        }
                        if (pRiplimb->isDead())
                        {
                            me->MonsterYell(SAY_RIPDEAD, 0, 0);
                            DoPlaySoundToSet(me, SOU_RIPDEAD);
                        }
                        else
                        {
                            me->MonsterYell(SAY_RAGEDEAD, 0, 0);
                            DoPlaySoundToSet(me, SOU_RAGEDEAD);
                        }
                        events.CancelEvent(EVENT_HURL_SPEAR_OR_MAGMA_RUPTURE);
                        events.ScheduleEvent(EVENT_HURL_SPEAR_OR_MAGMA_RUPTURE, 12000);
                        DoCast(me, SPELL_FRENZY_SHANNOX);
                        //me->MonsterTextEmote(SAY_ON_DOGS_FALL, 0, true);
                        softEnrage = true;
                    }

                    if ((pRiplimb->GetExactDist2d(me) >= maxDistanceBetweenShannoxAndDogs || pRageface->GetExactDist2d(me) >= maxDistanceBetweenShannoxAndDogs) && (!me->HasAura(SPELL_SEPERATION_ANXIETY)))
                        DoCast(me, SPELL_SEPERATION_ANXIETY);
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_shannoxAI(creature);
    }
};

class npc_rageface : public CreatureScript
{
public:
    npc_rageface() : CreatureScript("npc_rageface"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ragefaceAI(creature);
    }

    struct npc_ragefaceAI : public ScriptedAI
    {
        npc_ragefaceAI(Creature *c) : ScriptedAI(c)
        {
            instance = me->GetInstanceScript();

            Reset();
        }

        InstanceScript* instance;
        EventMap events;
        Unit* shallTarget;
        bool frenzy;

        void Reset()
        {
            me->RemoveAllAuras();
            events.Reset();
            frenzy = false;
            shallTarget = NULL;

            if (GetShannox() != NULL)
                me->GetMotionMaster()->MoveFollow(GetShannox(), walkRagefaceDistance, walkRagefaceAngle);
        }

        void KilledUnit(Unit * /*victim*/)
        {
        }

        void JustDied(Unit * /*victim*/)
        {
        }

        void EnterCombat(Unit * /*who*/)
        {
            DoZoneInCombat();

            if (Creature* shannox = GetShannox())
                DoZoneInCombat(shannox);

            me->CallForHelp(20);
            events.ScheduleEvent(EVENT_CHANGE_TARGET, urand(12000, 15000));
            events.ScheduleEvent(EVENT_LEAP, 30000);
            SelectNewTarget();

            if (me->GetMap() && me->GetMap()->IsHeroic())
                me->AddAura(SPELL_FEEDING_FRENZY_TRIGGER, me);
        }

        void SelectNewTarget()
        {
            shallTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 500, true);
            me->getThreatManager().resetAllAggro();
            me->AddThreat(shallTarget, 50000000.0f);
            me->GetMotionMaster()->MoveChase(shallTarget);
            me->Attack(shallTarget, true);
            events.ScheduleEvent(EVENT_CHANGE_TARGET, urand(10000, 12000));
        }

        void DamageTaken(Unit* who, uint32& damage)
        {

            if (damage >= uint32(RAID_MODE(30000, 45000, 30000, 45000)))
            {
                me->RemoveAura(SPELL_FACE_RAGE);
                if (me->getVictim())
                {
                    me->getVictim()->ClearUnitState(UNIT_STATE_STUNNED);
                    me->getVictim()->RemoveAurasDueToSpell(SPELL_FACE_RAGE);
                }

                if (me->HasAura(RAID_MODE(100129, 101212, 101213, 101214)))
                    me->GetAura(RAID_MODE(100129, 101212, 101213, 101214))->Remove();
                shallTarget = who;
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 5000.0f);
                me->GetMotionMaster()->MoveChase(who);
                me->Attack(who, true);
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!me->isInCombat() && GetShannox() != NULL)
                me->SetOrientation(GetShannox()->GetOrientation());

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (!UpdateVictim())
                return;

            if (me->getVictim() != NULL)
            {
                if (!me->HasAura(SPELL_FACE_RAGE) && me->getVictim()->HasAura(SPELL_FACE_RAGE))
                    me->getVictim()->RemoveAurasDueToSpell(SPELL_FACE_RAGE);
            }



            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_FACE_RAGE:
                    if (me->GetDistance2d(me->getVictim()) <= 1.0f)
                    {
                        me->GetMotionMaster()->Clear();
                        me->SetSpeed(MOVE_WALK, 2.8f, true);
                        me->SetSpeed(MOVE_RUN, 1.57143f, true);
                        DoCastVictim(SPELL_FACE_RAGE); //cast&damage
                        events.ScheduleEvent(EVENT_CHANGE_TARGET, 10000);
                        events.ScheduleEvent(EVENT_LEAP, 30000);
                    }
                    else
                    {
                        events.ScheduleEvent(EVENT_FACE_RAGE, 300);
                    }
                    break;
                case EVENT_LEAP:
                    //DoCastVictim(99945); buggy, workaround
                    me->SetSpeed(MOVE_WALK, 7.0f, true);
                    me->SetSpeed(MOVE_RUN, 7.0f, true);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_JUMPSTART);
                    events.CancelEvent(EVENT_CHANGE_TARGET);
                    events.ScheduleEvent(EVENT_FACE_RAGE, 1000);
                    break;
                case EVENT_CHANGE_TARGET:
                    SelectNewTarget();
                    break;
                default:
                    break;
                }
            }

            if (GetShannox() != NULL)
            {
                if (GetShannox()->GetDistance2d(me) >= maxDistanceBetweenShannoxAndDogs && !me->HasAura(SPELL_SEPERATION_ANXIETY))
                {
                    DoCast(me, SPELL_SEPERATION_ANXIETY);
                }

            }
            DoMeleeAttackIfReady();
        }

        Creature* GetShannox()
        {
            return ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_SHANNOX));
        }

    };
};

class npc_riplimb : public CreatureScript //Riplimb
{
public:
    npc_riplimb() : CreatureScript("npc_riplimb"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_riplimbAI(creature);
    }

    struct npc_riplimbAI : public ScriptedAI
    {
        npc_riplimbAI(Creature *c) : ScriptedAI(c), vehicle(c->GetVehicleKit())
        {
            instance = me->GetInstanceScript();
            Reset();
        }

        InstanceScript* instance;
        EventMap events;
        bool frenzy;
        bool movementResetNeeded;
        bool inTakingSpearPhase;
        Creature *spear;
        Creature *shannox;
        Vehicle* vehicle;

        void Reset()
        {
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SNARE, true);
            me->RemoveAllAuras();
            events.Reset();
            frenzy = false; // Is needed, that Frenzy is not casted twice on Riplimb
            movementResetNeeded = false; // Is needed for correct execution of the Phases
            inTakingSpearPhase = false; // Is needed for correct execution of the Phases

            if (GetShannox() != NULL)
                me->GetMotionMaster()->MoveFollow(GetShannox(), walkRiplimbDistance, walkRiplimbAngle);
        }

        void KilledUnit(Unit * /*victim*/)
        {
        }

        void JustDied(Unit * /*victim*/)
        {
        }

        void EnterCombat(Unit * who)
        {
            DoZoneInCombat();
            me->CallForHelp(20);

            if (Creature* shannox = GetShannox())
                DoZoneInCombat(shannox);

            spear = NULL;
            shannox = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_SHANNOX));
            me->GetMotionMaster()->MoveChase(me->getVictim());

            if (me->GetMap() && me->GetMap()->IsHeroic())
                me->AddAura(SPELL_FEEDING_FRENZY_TRIGGER, me);

            events.ScheduleEvent(EVENT_LIMB_RIP, 12000); //TODO Find out the correct Time
        }

        void DoAction(int32 actionId)
        {
            switch (actionId)
            {
            case ACTION_RETRIEVE_SPEAR:
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SNARE, false);
                //SetEquipmentSlots(false, 53000, 0,0);
                me->SetReactState(REACT_PASSIVE);
                spear = me->FindNearestCreature(NPC_SHANNOX_SPEAR, 200, true);
                if (spear)
                {
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MovePoint(0, spear->GetPositionX(), spear->GetPositionY(), spear->GetPositionZ());
                    events.CancelEvent(EVENT_LIMB_RIP);
                    events.ScheduleEvent(EVENT_TAKING_SPEAR_DELAY, 500);
                }
                break;
            }
        }

        void DamageTaken(Unit* who, uint32 &damage)
        {
            if (me->GetMap() && me->GetMap()->IsHeroic())
            {
                if (Creature* shannox = GetShannox())
                {
                    if (shannox->isAlive())
                    {
                        if (me->GetHealth() - damage <= 0 && me->GetReactState() != REACT_PASSIVE)
                        {
                            damage = 0;
                            me->AttackStop();
                            me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                            me->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE);
                            me->SetReactState(REACT_PASSIVE);
                            events.ScheduleEvent(EVENT_REPOP, 30000);
                        }
                    }
                }
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (!me->isInCombat() && GetShannox() != NULL)
                me->SetOrientation(GetShannox()->GetOrientation());

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_REPOP:
                    me->SetHealth(me->GetMaxHealth());
                    me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                    me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->AI()->AttackStart(me->getVictim());
                    break;
                case EVENT_LIMB_RIP:
                    DoCastVictim(SPELL_LIMB_RIP);
                    me->GetMotionMaster()->MoveChase(me->getVictim()); //repeat just to be sure
                    events.ScheduleEvent(EVENT_LIMB_RIP, urand(6000, 10000));
                    break;
                case EVENT_TAKING_SPEAR_DELAY:
                    spear = me->FindNearestCreature(NPC_SHANNOX_SPEAR, 200, true);
                    if (spear && me->GetExactDist2d(spear) <= 2)
                    {
                        //SetEquipmentSlots(false, 53000, 0,0);
                        //spear->EnterVehicle(me, 1);
                        events.ScheduleEvent(EVENT_HURL_SPEAR_3, 500);
                    }
                    else
                    {
                        me->GetMotionMaster()->Clear();
                        if (spear)//this is for prevernting the dog to be stuck if he gets in a trap
                            me->GetMotionMaster()->MovePoint(0, spear->GetPositionX(), spear->GetPositionY(), spear->GetPositionZ());
                        events.ScheduleEvent(EVENT_TAKING_SPEAR_DELAY, 500);
                    }
                    break;
                case EVENT_HURL_SPEAR_3:
                    shannox = me->FindNearestCreature(BOSS_SHANNOX, 200, true);
                    if (shannox) // crash prevention!
                    {
                        spear = me->FindNearestCreature(NPC_SHANNOX_SPEAR, 200, true);
                        if (spear)
                        {
                            //spear->ExitVehicle();
                            spear->DespawnOrUnsummon();
                            spear = NULL;
                        }
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MovePoint(0, shannox->GetPositionX(), shannox->GetPositionY(), shannox->GetPositionZ());
                        if (me->GetExactDist2d(shannox) <2)
                        {
                            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SNARE, true);
                            shannox->GetAI()->DoAction(ACTION_TAKE_SPEAR);
                            me->SetReactState(REACT_AGGRESSIVE);
                            me->GetMotionMaster()->Clear();
                            me->GetMotionMaster()->MoveChase(me->getVictim());
                            events.ScheduleEvent(EVENT_LIMB_RIP, 12000);
                        }
                        else
                        {
                            events.ScheduleEvent(EVENT_HURL_SPEAR_3, 500);
                        }
                    }
                    else
                    {
                        events.ScheduleEvent(EVENT_HURL_SPEAR_3, 500);
                    }
                    break;
                default:
                    break;
                }
            }

            if (GetShannox() != NULL)
            {
                if (GetShannox()->GetDistance2d(me) >= maxDistanceBetweenShannoxAndDogs && !me->HasAura(SPELL_SEPERATION_ANXIETY))
                {
                    DoCast(me, SPELL_SEPERATION_ANXIETY);
                }

            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

        inline Creature* GetShannox()
        {
            return ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_SHANNOX));
        }

        inline Creature* GetSpear()
        {
            return ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_SHANNOX_SPEAR));
        }
    };
};

class npc_shannox_spear : public CreatureScript //Shannox Spear
{
public:
    npc_shannox_spear() : CreatureScript("npc_shannox_spear"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_shannox_spearAI(creature);
    }

    struct npc_shannox_spearAI : public ScriptedAI
    {
        npc_shannox_spearAI(Creature *c) : ScriptedAI(c)
        {
        }

        uint32 timer;
        bool casted;

        void InitializeAI()
        {
            casted = false;
            timer = 4000;
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->AddAura(92211, me);
        }

        void UpdateAI(uint32 diff)
        {
            if (timer <= diff && !casted)
            {
                casted = true;
                float X;
                float Y;
                float Z;
                float degree = 0;

                // Calculate a very large spiral
                for (float width = 0; width <= 48; width += 0.2f)
                {
                    degree += 0.174f;
                    if (degree >= 6.28f)
                        degree -= 6.28f;
                    Y = sin(degree)*width;
                    X = cos(degree)*width;
                    Z = me->GetMap()->GetHeight(me->GetPositionX() + X, me->GetPositionY() + Y, me->GetPositionZ());
                    me->GetMap();
                    me->CastSpell(me->GetPositionX() + X,
                        me->GetPositionY() + Y,
                        Z,
                        SPELL_MAGMA_RUPTURE_VISUAL, true);
                }
                me->AddAura(SPELL_SPEAR_VISUAL, me);
                me->RemoveAura(92211);
            }
            else
                timer -= diff;
        }
    };
};

class npc_crystal_trap : public CreatureScript //Crystal Trap
{
public:
    npc_crystal_trap() : CreatureScript("npc_crystal_trap"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_crystal_trapAI(creature);
    }

    struct npc_crystal_trapAI : public ScriptedAI
    {
        npc_crystal_trapAI(Creature *c) : ScriptedAI(c), summons(me)
        {
            instance = me->GetInstanceScript();
            tempTarget = NULL;
            myPrison = NULL;
            //me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NOT_SELECTABLE);
        }

        InstanceScript* instance;
        Unit* tempTarget;
        Creature* myPrison;
        SummonList summons;
        bool despawning;
        uint32 timer;

        void JustDied(Unit * /*victim*/)
        {
        }

        void Reset()
        {
            despawning = false;
            timer = 4000;
            me->SetReactState(REACT_PASSIVE);
        }

        void EnterCombat(Unit * /*who*/)
        {
            despawning = false;
            timer = 4000;
            me->SetReactState(REACT_PASSIVE);
        }

        void UpdateAI(uint32 diff)
        {
            if (timer <= diff)
            {
                if (despawning)
                {
                    myPrison->DespawnOrUnsummon();
                    if (tempTarget)
                    {
                        tempTarget->RemoveAurasDueToSpell(CRYSTAL_PRISON_EFFECT);
                        // Cast Spell Wary on Ripclimb
                        if (tempTarget->ToCreature() && tempTarget->GetEntry() == NPC_RIPLIMB || tempTarget->GetEntry() == NPC_RAGEFACE)
                        {
                            DoCast(tempTarget, SPELL_WARY_10N, true);
                            tempTarget->ToCreature()->SetReactState(REACT_AGGRESSIVE);
                            if (tempTarget->getVictim())
                                tempTarget->GetMotionMaster()->MoveChase(tempTarget->getVictim());
                        }
                    }
                    me->DisappearAndDie();
                }
                else
                {
                    //searching for a target, first Riplimb...
                    if (Creature* Riplimb = me->FindNearestCreature(NPC_RIPLIMB, 2, true))
                    {
                        if (Riplimb->GetExactDist2d(me) <2 && !Riplimb->HasAura(SPELL_WARY_10N))
                            tempTarget = Riplimb;
                    }
                    else if (Creature* Rageface = me->FindNearestCreature(NPC_RAGEFACE, 2, true))
                    {//...now Rageface...
                        if (Rageface->GetExactDist2d(me)<2 && !Rageface->HasAura(SPELL_WARY_10N))
                            tempTarget = Rageface;
                    }
                    else
                    {//...and then players
                        if (SelectTarget(SELECT_TARGET_NEAREST, 0, 2, true) != NULL && SelectTarget(SELECT_TARGET_NEAREST, 0, 2, true)->GetExactDist2d(me))
                        {
                            Unit* trapped = SelectTarget(SELECT_TARGET_NEAREST, 0, 2, true);
                            if (trapped && trapped->ToPlayer() && trapped->GetExactDist2d(me)<2)
                                tempTarget = trapped;
                        }
                    }
                    if (tempTarget == NULL) // If no Target exists try to get a new Target in 0,5s
                    {
                        timer = 300;
                    }
                    else
                    { // Intialize Prison if tempTarget was set
                        myPrison = me->SummonCreature(NPC_CRYSTAL_PRISON,
                            me->GetPositionX(),
                            me->GetPositionY(),
                            me->GetPositionZ(), 0, TEMPSUMMON_MANUAL_DESPAWN);
                        if (myPrison)
                        {
                            myPrison->SetReactState(REACT_PASSIVE);
                            if (tempTarget && !tempTarget->ToPlayer())
                                myPrison->setFaction(me->getFaction());
                        }
                        DoCast(tempTarget, CRYSTAL_PRISON_EFFECT);
                        timer = 15000;
                        despawning = true;
                    }
                }
            }
            else
                timer -= diff;

            if (myPrison)
            {
                if (myPrison->isDead())
                {
                    myPrison->DespawnOrUnsummon();
                    if (tempTarget)
                        tempTarget->RemoveAurasDueToSpell(CRYSTAL_PRISON_EFFECT);
                    me->DisappearAndDie();
                }
            }
        }
    };
};

class npc_immolation_trap : public CreatureScript //Immolation Trap
{
public:
    npc_immolation_trap() : CreatureScript("npc_immolation_trap"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_immolation_trapAI(creature);
    }

    struct npc_immolation_trapAI : public ScriptedAI
    {
        npc_immolation_trapAI(Creature *c) : ScriptedAI(c)
        {
        }

        InstanceScript* instance;
        Unit* tempTarget;
        bool despawning;
        uint32 timer;

        void JustDied(Unit * /*victim*/)
        {
        }

        void Reset()
        {
            despawning = false;
            tempTarget = NULL;
            timer = 4000;
            me->SetReactState(REACT_PASSIVE);
        }

        void EnterCombat(Unit * /*who*/)
        {
            despawning = false;
            tempTarget = NULL;
            timer = 4000;
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NOT_SELECTABLE);
        }

        void UpdateAI(uint32 diff)
        {
            if (timer <= diff)
            {
                if (despawning)
                {
                    // Cast Spell Wary on Ripclimb
                    if (tempTarget && tempTarget->GetEntry() == NPC_RIPLIMB)
                        DoCast(tempTarget, SPELL_WARY_10N, true);
                    me->DisappearAndDie();
                }
                else
                {
                    //searching for a target, first Riplimb...
                    if (Creature* Riplimb = me->FindNearestCreature(NPC_RIPLIMB, 2, true))
                    {
                        if (Riplimb->GetExactDist2d(me) <= 2 && !Riplimb->HasAura(SPELL_WARY_10N))
                            tempTarget = Riplimb;
                    }
                    else if (Creature* Rageface = me->FindNearestCreature(NPC_RAGEFACE, 2, true))
                    {//...now Rageface...
                        if (Rageface->GetExactDist2d(me) <= 2 && !Rageface->HasAura(SPELL_WARY_10N))
                            tempTarget = Rageface;
                    }
                    else
                    {//...and then players
                        if (SelectTarget(SELECT_TARGET_NEAREST, 0, 2, true) != NULL && SelectTarget(SELECT_TARGET_NEAREST, 0, 2, true)->GetExactDist2d(me) <= 2)
                        {
                            tempTarget = SelectTarget(SELECT_TARGET_NEAREST, 0, 2, true);
                        }
                    }
                    if (tempTarget == NULL) // If no Target exists try to get a new Target in 0,5s
                    {
                        timer = 500;
                    }
                    else
                    { // Intialize Prison if tempTarget was set
                        DoCast(tempTarget, IMMOLATION_TRAP);
                        timer = 2000;
                        despawning = true;
                    }
                }
            }
            else
                timer -= diff;
        }

        Creature* GetRiplimb()
        {
            return ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_RIPLIMB));
        }
    };
};

void AddSC_boss_shannox()
{
    new boss_shannox();
    new npc_rageface();
    new npc_riplimb();
    new npc_shannox_spear();
    new npc_crystal_trap();
    new npc_immolation_trap();
};