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
Author: Nagash - Bennu
Progress: 80%

ToDo

ALYSRAZOR dovrebbe arrivare dopo 20 sec al muro di fuoco per POI spawnare le Uova E LE uova ci dovrebbero mettere 10 secondi
a schiudersi, invece le spawna subito.

*/

#define SAY_INTRO1    "What have we here - visitors to our kingdom in the Firelands?"
#define SOU_INTRO1    24466 //8 secs
#define SAY_INTRO2    "You mortals may remember Alysra, who spirited me to freedom in Mount Hyjal. She, too has been reborn. Born of flame!"
#define SOU_INTRO2    24467 //14 secs
#define SAY_INTRO3    "I wish I could watch her reduce your pitiful band to cinders, but I am needed elsewhere. Farewell!"
#define SOU_INTRO3    24468 //14 secs
#define SAY_AGGRO     "I serve a new master now, mortals!"
#define SOU_AGGRO     24426
#define SAY_DEATH     "The light... mustn't... burn out..."
#define SOU_DEATH     24429
#define SAY_KILL1     "BURN!"
#define SOU_KILL1     24430
#define SAY_KILL2     "For his Glory!"
#define SOU_KILL2     24431
#define SAY_SKYES     "These skies are MINE!"
#define SOU_SKYES     24434
#define SAY_SKYE2     "I will burn you from the sky!"
#define SOU_SKYE2     24435
#define SAY_FIRE      "Fire... fire..."
#define SOU_FIRE      24436
#define SAY_REBORN    "Reborn in flame!"
#define SOU_REBORN    24437
#define SAY_HERALD    "None escape the rage of the Firelands!"  


// skin della piuma, 38146

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

enum Texts
{
    // Egg Pile
    EMOTE_CRACKING_EGGS = 0,    // The Molten Eggs begin to crack and splinter!
};

enum Spells
{
    // Majordomo
    SPELL_ROOTS = 100559,
    SPELL_ROOTS_DEBUFF = 100555,
    //Alysrazor
    SPELL_VOLCANIC_FIRE = 98462,
    SPELL_ABILITATE_MOLTEN_FEATHER_BAR = 101410,
    SPELL_MOLTING = 99464,
    SPELL_MOLTEN_FEATHER = 97128,
    SPELL_WINGS_OF_FLAME_TRIGGER = 98624,
    SPELL_WINGS_OF_FLAME_BUFF = 98619,
    SPELL_BLAZING_POWER = 99461,
    SPELL_BLAZING_POWER_TRIGGER = 99462,
    SPELL_ALYSRA_RAZOR = 100029,
    SPELL_FIRST_FIRESTORM = 99605,
    SPELL_BLAZING_CLAW = 99843,
    SPELL_INCENDIARY_CLOUD = 99426,
    SPELL_FIERY_VORTEX = 99793,
    SPELL_FIERY_TORNADO = 99817,
    SPELL_HARSH_WINDS = 100640,
    SPELL_BURNOUT = 99432,
    SPELL_FULL_POWER = 99504,
    SPELL_IGNITED = 99922,
    SPELL_ESSENCE_OF_THE_GREEN = 99433,

    //blazing talon initiate
    SPELL_FIEROBLAST = 101223,
    SPELL_BRUSHFIRE = 98868,
    SPELL_BLAZING_SHIELD = 101484,

    //blazing talon clawshaper
    SPELL_IGNITION = 99919,

    //voracious hatchling
    SPELL_SATED = 99359,
    SPELL_HUNGRY = 99361,
    SPELL_TANTRUM = 99362,
    SPELL_IMPRINTED = 100359,
    SPELL_GUSHING_WOUND = 99308,

    //worms
    SPELL_LAVA_SPEW = 99335,

    //molten feather
    SPELL_MOLTEN_FEATHER_COSMETIC = 99507,

    // Harbinger of Flame
    SPELL_FIRE_IT_UP = 100093,
    SPELL_FIEROBLAST_TRASH = 100094,
    SPELL_FIEROCLAST_BARRAGE = 100095,
    SPELL_FIRE_CHANNELING = 100109,

    // Blazing Monstrosity
    SPELL_RIDE_MONSTROSITY = 93970,
    SPELL_SHARE_HEALTH_LEFT = 101502,
    SPELL_SHARE_HEALTH_RIGHT = 101503,
    SPELL_SLEEP_ULTRA_HIGH_PRIORITY = 99480,
    SPELL_GENERIC_DUMMY_CAST = 100088,
    SPELL_LEFT_SIDE_SMACK_L = 100076,
    SPELL_RIGHT_SIDE_SMACK_L = 100078,
    SPELL_HEAD_BONK_L = 100080,
    SPELL_TICKLE_L = 100082,
    SPELL_KNOCKBACK_RIGHT = 100084,
    SPELL_KNOCKBACK_LEFT = 100085,
    SPELL_KNOCKBACK_FORWARD = 100086,
    SPELL_KNOCKBACK_BACK = 100087,
    SPELL_HEAD_BONK_R = 100089,
    SPELL_LEFT_SIDE_SMACK_R = 100090,
    SPELL_RIGHT_SIDE_SMACK_R = 100091,
    SPELL_TICKLE_R = 100092,
    SPELL_MOLTEN_BARRAGE_EFFECT_L = 100071,
    SPELL_MOLTEN_BARRAGE_LEFT = 100072,
    SPELL_MOLTEN_BARRAGE_RIGHT = 100073,
    SPELL_MOLTEN_BARRAGE_EFFECT_R = 100074,
    SPELL_MOLTEN_BARRAGE_VISUAL = 100075,
    SPELL_AGGRO_CLOSEST = 100462,
    SPELL_INVISIBILITY_AND_STEALTH_DETECTION = 18950,

    // Egg Pile
    SPELL_SUMMON_SMOULDERING_HATCHLING = 100096,
    SPELL_MOLTEN_EGG_TRASH_CALL_L = 100097,
    SPELL_MOLTEN_EGG_TRASH_CALL_R = 100098,
    SPELL_ALYSRAZOR_COSMETIC_EGG_XPLOSION = 100099,

    // Herald of the burning end
    SPELL_CATACLYSM_1 = 100761,
    SPELL_FIRST_MOLTEN_METEOR = 99553,

    // Molten Meteor
    SPELL_METEORIC_IMPACT = 99558,
    SPELL_MOLTEN_BOULDER = 99265,
};

#define SPELL_SHARE_HEALTH          (me->GetEntry() == NPC_BLAZING_MONSTROSITY_LEFT ? SPELL_SHARE_HEALTH_LEFT : SPELL_SHARE_HEALTH_RIGHT)
#define SPELL_MOLTEN_BARRAGE        (me->GetEntry() == NPC_BLAZING_MONSTROSITY_LEFT ? SPELL_MOLTEN_BARRAGE_LEFT : SPELL_MOLTEN_BARRAGE_RIGHT)
#define SPELL_MOLTEN_BARRAGE_EFFECT (me->GetEntry() == NPC_BLAZING_MONSTROSITY_LEFT ? SPELL_MOLTEN_BARRAGE_EFFECT_L : SPELL_MOLTEN_BARRAGE_EFFECT_R)

enum Events
{
    //Alysrazor
    EVENT_FIRST_FIRESTORM = 1,
    EVENT_SECOND_FIRESTORM,
    EVENT_HERALD_BURNING,
    EVENT_SPAWN_RING,
    EVENT_FIRE_CLOUD,
    EVENT_INTRO_1,
    EVENT_INTRO_2,
    EVENT_INTRO_3,
    EVENT_START,
    EVENT_DESP,
    EVENT_MOLTEN_EGGS,
    EVENT_IGNITE_EGGS,
    EVENT_HATCH_EGGS,
    EVENT_WORMS,

    EVENT_PHASE_1,
    EVENT_PATH,

    EVENT_PHASE_2,
    EVENT_SPAWN_RING2,
    EVENT_SPAWN_TORNADOES,
    EVENT_CONTINUE_SPAWN_TORNADOES,
    EVENT_CIRCLING,
    EVENT_MOVE_OVER_CENTER,
    EVENT_CHECK_DISTANCE,

    EVENT_PHASE_3,
    EVENT_DESPAWN_TORNADOES,
    EVENT_REGEN_ENERGY,

    EVENT_PHASE_4,
    EVENT_REGEN_ENERGY2,

    //druid of the flame
    EVENT_DRUID_BLAST,
    EVENT_TRANSFORM,
    EVENT_BRUSHFIRE,

    //clawshaper
    EVENT_START_CHANNELING,
    EVENT_CHECK_STATUS,

    //mounstrous hatchling
    EVENT_SATED,
    EVENT_HUNGRY,
    EVENT_CHECK_WORM,
    EVENT_GUSHING,

    // Blazing Monstrosity
    EVENT_START_SPITTING = 1,
    EVENT_CONTINUE_SPITTING = 2,

    // Harbinger of Flame
    EVENT_FIEROBLAST = 1,
    EVENT_FIEROCLAST_BARRAGE = 2,

    // Egg Pile
    EVENT_SUMMON_SMOULDERING_HATCHLING = 1,

    // Herald of the burning end
    EVENT_METEOR_PATH,
    EVENT_METEOR_POINT,
    EVENT_METEOR_EXPLOSION,
    EVENT_METEOR_STOP,
};

enum MiscData
{
    MODEL_INVISIBLE_STALKER = 11686,
    ANIM_KIT_BIRD_WAKE = 1469,
    ANIM_KIT_BIRD_TURN = 1473,
};

enum Actions
{
    ACTION_TURN_OR = 2,
    ACTION_TURN_ANT,
    ACTION_DIRECTION_0,
    ACTION_DIRECTION_1,
    ACTION_DIRECTION_2,
    ACTION_DIRECTION_3,
    ACTION_DIRECTION_4,
    ACTION_DIRECTION_5,
    ACTION_NOT_FIRST_ENC,
    ACTION_IGNITE,
};

Position const centerpos = { -43.069f, -275.88f, 55.00f, 0.0f };
Position const majordomo = { 32.42f, -335.46f, 52.40f, 2.44f };

Position const meteorpath[5] = {
    { 2.75f, -243.79f, 54.62f, 0.0f },
    { -15.75f, -225.74f, 55.10f, 0.0f },
    { -77.34f, -238.40f, 57.70f, 0.0f },
    { -91.46f, -285.89f, 54.88f, 0.0f },
    { -59.41f, -328.29f, 55.55f, 0.0f } };

Position const pospath[8] = {
    { 4.15f, -307.46f, 70.40f, 0.0f }, //starting position
    { -81.42f, -268.46f, 70.40f, 0.0f }, //other side of the arena
    { -81.42f, -268.46f, 110.40f, 0.0f }, //above the previous
    { -11.29f, -224.96f, 110.22f, 0.0f },
    { 37.21f, -280.23f, 118.92f, 0.0f },
    { -47.69f, -341.51f, 125.38f, 0.0f },
    { -94.52f, -246.98f, 113.90f, 0.0f },
    { 4.15f, -307.46f, 110.40f, 0.0f } }; // above starting position


class RespawnEggEvent : public BasicEvent
{
public:
    explicit RespawnEggEvent(Creature* egg) : _egg(egg) { }

    bool Execute(uint64 /*time*/, uint32 /*diff*/)
    {
        _egg->RestoreDisplayId();
        return true;
    }

private:
    Creature* _egg;
};

class MoltenEggCheck
{
public:
    explicit MoltenEggCheck(Creature* pile) : _eggPile(pile) { }

    bool operator()(Unit* object) const
    {
        if (object->GetEntry() != NPC_MOLTEN_EGG_TRASH)
            return false;

        if (object->GetDisplayId() != object->GetNativeDisplayId())
            return false;

        if (_eggPile->GetDistance2d(object) > 20.0f)
            return false;

        return true;
    }

private:
    Creature* _eggPile;
};

class TrashRespawnWorker
{
public:
    void operator()(Creature* creature) const
    {
        switch (creature->GetEntry())
        {
        case NPC_BLAZING_MONSTROSITY_LEFT:
        case NPC_BLAZING_MONSTROSITY_RIGHT:
        case NPC_EGG_PILE:
        case NPC_HARBINGER_OF_FLAME:
        case NPC_MOLTEN_EGG_TRASH:
            if (!creature->isAlive())
                creature->Respawn(true);
            break;
        case NPC_SMOULDERING_HATCHLING:
            creature->DespawnOrUnsummon();
            break;
        }
    }
};

static void AlysrazorTrashEvaded(Creature* creature)
{
    TrashRespawnWorker check;
    Trinity::CreatureWorker<TrashRespawnWorker> worker(creature, check);
    creature->VisitNearbyGridObject(SIZE_OF_GRIDS, worker);
}

class boss_alysrazor : public CreatureScript
{
public:
    boss_alysrazor() : CreatureScript("boss_alysrazor") {}

    struct boss_alysrazorAI : public BossAI
    {
        boss_alysrazorAI(Creature * creature) : BossAI(creature, DATA_ALYSRAZOR), summons(me)
        {
            instance = me->GetInstanceScript();
            firstenc = true;
            _meteorCounter = 0;
            _eggsCounter = 0;
        }

        InstanceScript* instance;
        uint32 phase;
        SummonList summons;
        uint32 killtimer;
        uint32 path;
        float ori;
        bool firstenc;
        bool worms;
        uint8 cloudcount;
        uint8 mana;
        uint8 _meteorCounter;
        uint8 _eggsCounter;

        void DespawnCreatures(uint32 entry, float distance)
        {
            std::list<Creature*> creatures;
            GetCreatureListWithEntryInGrid(creatures, me, entry, distance);

            if (creatures.empty())
                return;

            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                (*iter)->DespawnOrUnsummon();
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            if (!summon)
                return;
            summon->setActive(true);

            switch (summon->GetEntry())
            {
            case NPC_MOLTEN_FEATHER:
                summon->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                summon->CastSpell(summon, SPELL_MOLTEN_FEATHER_COSMETIC);
                break;
            case NPC_INCENDIARY_CLOUD:
            case NPC_BLAZING_POWER:
                summon->SetReactState(REACT_PASSIVE);
                summon->AddUnitMovementFlag(MOVEMENTFLAG_FLYING);
                summon->AddUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
                summon->AddUnitMovementFlag(MOVEMENTFLAG_CAN_FLY);
                summon->AddUnitMovementFlag(MOVEMENTFLAG_HOVER);
                summon->SetCanFly(true);
                summon->SetDisableGravity(true);
                summon->SetHover(true);
                summon->setFaction(me->getFaction());
                if (me->isInCombat())
                    DoZoneInCombat(summon);
                break;
            case NPC_MOLTEN_EGG:
                summon->setFaction(me->getFaction());
                summon->GetMotionMaster()->MoveFall();
                summon->SetReactState(REACT_PASSIVE);
                summon->AttackStop();
                summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
                break;
            default:
                summon->setFaction(me->getFaction());
                if (me->isInCombat())
                    DoZoneInCombat(summon);
                break;
            }
        }

        void Reset()
        {
            _Reset();
            summons.DespawnAll();
            DespawnCreatures(NPC_FIRST_MOLTEN_METEOR, 500.0f);
            DespawnCreatures(NPC_SMALL_METEOR, 500.0f);
            DespawnCreatures(NPC_BRUSHFIRE, 400.0f);
            me->RemoveAllGameObjects();
            me->LowerPlayerDamageReq(me->GetMaxHealth());
            killtimer = 0;
            _meteorCounter = 0;
            _eggsCounter = 0;
            me->RemoveAllAuras();
            events.Reset();
            Creature* majo = me->SummonCreature(NPC_MAJORDOMO_ALYSRAZOR, majordomo);
            if (!firstenc && majo && majo->GetAI())
                majo->GetAI()->DoAction(ACTION_NOT_FIRST_ENC);
            if (instance)
            {
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ABILITATE_MOLTEN_FEATHER_BAR);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MOLTEN_FEATHER);
                instance->SetData(DATA_ALYSRAZOR_EVENT, NOT_STARTED);
            }
            if (me->ToUnit())
            {
                me->ToUnit()->ApplyAuraToEntry(NPC_ALYSRAZOR_FIRE, SPELL_VOLCANIC_FIRE, false);
            }
        }

        void DoAction(int32 actionId)
        {
            switch (actionId)
            {
            case ACTION_IGNITE:
                mana++;
                me->SetPower(POWER_MANA, mana);
                if (mana >= 50 && phase == 3)
                    events.ScheduleEvent(EVENT_PHASE_4, 50);
                else if (mana >= 100 && phase == 4)
                    events.ScheduleEvent(EVENT_PHASE_1, 50);
                break;
            }
        }

        void DamageTaken(Unit* who, uint32 &damage)
        {
            if (me->GetAura(SPELL_BURNOUT))
                me->CastSpell(who, SPELL_ESSENCE_OF_THE_GREEN, true);
        }

        void EnterCombat(Unit* victim)
        {
            _EnterCombat();
            mana = 100;
            cloudcount = 1;
            firstenc = false;
            worms = false;
            if (!victim)
                return;
            me->SetReactState(REACT_PASSIVE);
            me->AddUnitMovementFlag(MOVEMENTFLAG_FLYING);
            me->SetCanFly(true);
            me->SetDisableGravity(true);
            me->SetHover(true);
            me->SetSpeed(MOVE_WALK, 3.2f, true);
            me->SetSpeed(MOVE_RUN, 3.2f, true);
            me->SetSpeed(MOVE_FLIGHT, 3.2f, true);
            me->AddAura(52414, me);

            me->MonsterYell(SAY_AGGRO, 0, 0);
            DoPlaySoundToSet(me, SOU_AGGRO);

            //me->MonsterYell(SAY_AGGRO, 0, 0);
            //DoPlaySoundToSet(me, SOU_AGGRO);
            DoZoneInCombat(me);
            me->SetCanRegen(false);
            me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER);
            DoCastAOE(SPELL_ABILITATE_MOLTEN_FEATHER_BAR);
            me->GetMotionMaster()->MovePoint(0, 47, -340, 110);
            phase = 1;
            path = 0;


            if (me->ToUnit())
            {
                me->ToUnit()->ApplyAuraToEntry(NPC_ALYSRAZOR_FIRE, SPELL_VOLCANIC_FIRE);
            }

            if (instance)
            {
                instance->SetData(DATA_ALYSRAZOR_EVENT, IN_PROGRESS);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ROOTS);
            }

            events.ScheduleEvent(EVENT_FIRST_FIRESTORM, 2000);
        }

        void JustDied(Unit* killer)
        {
            _JustDied();
            me->MonsterYell(SAY_DEATH, 0, 0);
            DoPlaySoundToSet(me, SOU_DEATH);
            me->LowerPlayerDamageReq(me->GetMaxHealth());
            summons.DespawnAll();
            DespawnCreatures(NPC_FIRST_MOLTEN_METEOR, 500.0f);
            DespawnCreatures(NPC_SMALL_METEOR, 500.0f);
            me->RemoveAllGameObjects();
            if (me->GetMap()->IsHeroic())
            {// this will give the achievement to players in heroic difficulty
                Map::PlayerList const& PlayerList = instance->instance->GetPlayers();
                AchievementEntry const* alysrazor = sAchievementStore.LookupEntry(5809);
                for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                {
                    if (Player* player = itr->getSource())
                    {
                        player->CompletedAchievement(alysrazor);
                    }
                }
            }
            if (instance)
            {
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ABILITATE_MOLTEN_FEATHER_BAR);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MOLTEN_FEATHER);
                instance->SetData(DATA_ALYSRAZOR_EVENT, DONE);
            }
            if (me->ToUnit())
            {
                me->ToUnit()->ApplyAuraToEntry(NPC_ALYSRAZOR_FIRE, SPELL_VOLCANIC_FIRE, false);
            }
        }

        void KilledUnit(Unit* victim)
        {
            if (!victim || victim->GetTypeId() != TYPEID_PLAYER || killtimer > 0)
                return;
            int caso = urand(1, 2);
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
                //me->MonsterYell(SAY_KILL3, 0, 0);
                //DoPlaySoundToSet(me, SOU_KILL3);
                break;
            case 4:
                //me->MonsterYell(SAY_KILL4, 0, 0);
                //DoPlaySoundToSet(me, SOU_KILL4);
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

            // Core bug, if we have react passive core don't remove creatures from trheat list.
            if (!me->getVictim())
                me->SetReactState(REACT_AGGRESSIVE);

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                case EVENT_FIRST_FIRESTORM:
                    DoCast(SPELL_FIRST_FIRESTORM);
                    me->ClearUnitState(UNIT_STATE_CASTING);
                    me->RemoveAura(52414);
                    events.ScheduleEvent(EVENT_PATH, 10000);
                    events.ScheduleEvent(EVENT_FIRE_CLOUD, 19000);
                    events.ScheduleEvent(EVENT_SPAWN_RING, 20000);
                    if (me->GetMap()->IsHeroic())
                    {
                        events.ScheduleEvent(EVENT_PHASE_2, 235000); //220000
                        events.ScheduleEvent(EVENT_HERALD_BURNING, 30000);
                        events.ScheduleEvent(EVENT_SECOND_FIRESTORM, 88000);
                    }
                    else
                    {
                        events.ScheduleEvent(EVENT_PHASE_2, 180000); //127000
                    }
                    break;
                case EVENT_SECOND_FIRESTORM:
                    if (Creature* worm = me->FindNearestCreature(NPC_PLUMP_LAVA_WORM, 200))
                    {
                        DespawnCreatures(NPC_PLUMP_LAVA_WORM, 300.0f);
                        DespawnCreatures(54020, 300.0f);
                        events.CancelEvent(EVENT_WORMS);
                        events.ScheduleEvent(EVENT_WORMS, 20000);
                    }

                    me->SummonCreature(NPC_FIRESTORM,
                        centerpos.GetPositionX(),
                        centerpos.GetPositionY(),
                        centerpos.GetPositionZ(),
                        0,
                        TEMPSUMMON_TIMED_DESPAWN, 11000);
                    if (_meteorCounter < 4)
                    {
                        events.ScheduleEvent(EVENT_SECOND_FIRESTORM, 88000);
                        events.CancelEvent(EVENT_HERALD_BURNING);
                        events.ScheduleEvent(EVENT_HERALD_BURNING, 37000);
                    }
                    break;
                case EVENT_PHASE_1:
                    phase = 1;
                    path = 2;
                    _meteorCounter = 0;
                    _eggsCounter = 0;
                    cloudcount = 1;
                    DoCast(SPELL_FULL_POWER);
                    me->RemoveAura(SPELL_IGNITED);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                    DespawnCreatures(NPC_BLAZING_TALON_CLAWSHAPER, 300.0f);
                    me->MonsterYell(SAY_SKYE2, LANG_UNIVERSAL, 0);
                    DoPlaySoundToSet(me, SOU_SKYE2);
                    me->GetMotionMaster()->MovePoint(0, pospath[1]);
                    me->SetSpeed(MOVE_WALK, 3.2f, true);
                    me->SetSpeed(MOVE_RUN, 3.2f, true);
                    me->SetSpeed(MOVE_FLIGHT, 3.2f, true);
                    events.ScheduleEvent(EVENT_PATH, 4000);
                    events.ScheduleEvent(EVENT_FIRE_CLOUD, 5000);
                    events.ScheduleEvent(EVENT_SPAWN_RING, 5000);
                    events.ScheduleEvent(EVENT_MOLTEN_EGGS, 15000);
                    events.ScheduleEvent(EVENT_WORMS, 15000);
                    if (me->GetMap()->IsHeroic())
                    {
                        events.ScheduleEvent(EVENT_HERALD_BURNING, 30000);
                        events.ScheduleEvent(EVENT_SECOND_FIRESTORM, 88000);
                        events.ScheduleEvent(EVENT_PHASE_2, 250000);
                    }
                    else
                    {
                        events.ScheduleEvent(EVENT_PHASE_2, 180000);
                    }
                    break;
                case EVENT_FIRE_CLOUD:
                {
                    if (RAID_MODE(false, false, true, true))
                        cloudcount = 3;
                    for (int i = 0; i< cloudcount; i++)
                    {
                        float dir = me->GetOrientation() + (M_PI / 2) + M_PI*(float)rand_norm();
                        if (dir > 2 * M_PI) dir -= 2 * M_PI;
                        float dist = 25.0f * (float)rand_norm() + 10.0f;
                        float height = me->GetPositionZ() + 30.0f * (float)rand_norm() - 15.0f;
                        if (height < centerpos.GetPositionZ() + 30)
                            height = centerpos.GetPositionZ() + 30;
                        if (Creature* cloud = me->SummonCreature(NPC_INCENDIARY_CLOUD,
                            me->GetPositionX() + dist*cos(dir),
                            me->GetPositionY() + dist*sin(dir),
                            height,
                            me->GetOrientation(),
                            TEMPSUMMON_TIMED_DESPAWN,
                            8000))
                        {
                            me->AddAura(SPELL_INCENDIARY_CLOUD, cloud);
                            cloud->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                            cloud->SetReactState(REACT_PASSIVE);
                        }
                    }
                }
                events.ScheduleEvent(EVENT_FIRE_CLOUD, 5000);
                break;
                case EVENT_SPAWN_RING:
                {
                    float dir = me->GetOrientation() + (M_PI / 2) + M_PI*(float)rand_norm();
                    if (dir > 2 * M_PI) dir -= 2 * M_PI;
                    float dist = 10.0f * (float)rand_norm() + 15.0f;
                    float height = me->GetPositionZ() + 20.0f * (float)rand_norm() - 10.0f;
                    if (height < centerpos.GetPositionZ() + 30)
                        height = centerpos.GetPositionZ() + 30;
                    if (Creature* ring = me->SummonCreature(NPC_BLAZING_POWER,
                        me->GetPositionX() + dist*cos(dir),
                        me->GetPositionY() + dist*sin(dir),
                        height,
                        me->GetOrientation(),
                        TEMPSUMMON_TIMED_DESPAWN,
                        5000))
                    {
                        me->AddAura(SPELL_BLAZING_POWER_TRIGGER, ring);
                        me->AddAura(34873, ring);// aura to let the dummy fly
                        ring->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                        ring->SetReactState(REACT_PASSIVE);
                    }
                }
                events.ScheduleEvent(EVENT_SPAWN_RING, 5000);
                break;
                case EVENT_SPAWN_RING2:
                {
                    float dir = M_PI* urand(0, 3);
                    float dist = 20.0f * (float)rand_norm() + 20.0f;
                    float height = centerpos.GetPositionZ() + 5;
                    if (Creature* ring = me->SummonCreature(NPC_BLAZING_POWER,
                        centerpos.GetPositionX() + dist*cos(dir),
                        centerpos.GetPositionY() + dist*sin(dir),
                        height,
                        me->GetOrientation(),
                        TEMPSUMMON_TIMED_DESPAWN,
                        5000))
                    {
                        me->AddAura(SPELL_BLAZING_POWER_TRIGGER, ring);
                        me->AddAura(34873, ring);// aura to let the dummy fly
                        ring->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                        ring->SetReactState(REACT_PASSIVE);
                    }
                }
                events.ScheduleEvent(EVENT_SPAWN_RING2, 5000);
                break;
                case EVENT_MOLTEN_EGGS:
                    _eggsCounter++;
                    for (uint32 i = 0; i< 2; i++)
                    {
                        float dir = 2 * M_PI*(float)rand_norm();
                        float dist = 10.0f * (float)rand_norm() + 10.0f;
                        me->SummonCreature(NPC_MOLTEN_EGG,
                            centerpos.GetPositionX() + dist*cos(dir),
                            centerpos.GetPositionY() + dist*sin(dir),
                            centerpos.GetPositionZ() + 30,
                            dir,
                            TEMPSUMMON_MANUAL_DESPAWN);
                    }
                    if (_meteorCounter == 2 || _meteorCounter == 4)
                        me->RemoveAllGameObjects();
                    DespawnCreatures(NPC_FIRST_MOLTEN_METEOR, 500.0f);
                    events.ScheduleEvent(EVENT_IGNITE_EGGS, 3000);
                    events.ScheduleEvent(EVENT_HATCH_EGGS, 6000);
                    if (me->GetMap()->IsHeroic())
                    {
                        events.ScheduleEvent(EVENT_MOLTEN_EGGS, 87000);
                    }
                    break;
                case EVENT_HERALD_BURNING:
                    _meteorCounter++;
                    me->SummonCreature(NPC_HERALD_BURNING,
                        centerpos.GetPositionX(),
                        centerpos.GetPositionY(),
                        centerpos.GetPositionZ(),
                        0,
                        TEMPSUMMON_TIMED_DESPAWN, 15000);
                    if (_meteorCounter < 4)
                        events.ScheduleEvent(EVENT_HERALD_BURNING, 30000);
                    break;
                case EVENT_IGNITE_EGGS:
                    if (me->ToUnit())
                        me->ToUnit()->ApplyAuraToEntry(NPC_MOLTEN_EGG, SPELL_BRUSHFIRE);
                    break;
                case EVENT_HATCH_EGGS:
                    while (Creature* egg = me->FindNearestCreature(NPC_MOLTEN_EGG, 400, true))
                    {
                        me->SummonCreature(NPC_VORACIOUS_HATCHLING,
                            egg->GetPositionX(),
                            egg->GetPositionY(),
                            egg->GetPositionZ() + 1,
                            egg->GetOrientation(),
                            TEMPSUMMON_MANUAL_DESPAWN);

                        egg->DespawnOrUnsummon();
                    }
                    break;
                case EVENT_PATH:
                    switch (path)
                    {
                    case 0:
                        me->SetSpeed(MOVE_WALK, 2.0f, true);
                        me->SetSpeed(MOVE_RUN, 2.0f, true);
                        me->SetSpeed(MOVE_FLIGHT, 2.0f, true);
                        me->GetMotionMaster()->MovePoint(0, pospath[path++]);
                        events.ScheduleEvent(EVENT_PATH, 3000);
                        break;
                    case 1:
                        me->SetSpeed(MOVE_WALK, 2.0f, true);
                        me->SetSpeed(MOVE_RUN, 2.0f, true);
                        me->SetSpeed(MOVE_FLIGHT, 2.0f, true);
                        me->GetMotionMaster()->MovePoint(0, pospath[path++]);
                        me->AddAura(SPELL_BLAZING_CLAW, me);
                        me->AddAura(SPELL_MOLTING, me);
                        events.ScheduleEvent(EVENT_PATH, 8000);
                        break;
                    case 2:
                    {
                        me->SetSpeed(MOVE_WALK, 3.2f, true);
                        me->SetSpeed(MOVE_RUN, 3.2f, true);
                        me->SetSpeed(MOVE_FLIGHT, 3.2f, true);
                        me->GetMotionMaster()->MovePoint(0, pospath[path++]);
                        me->RemoveAura(SPELL_BLAZING_CLAW);
                        me->RemoveAura(SPELL_MOLTING);
                        int v = roll_chance_i(50) ? -1 : 1;
                        if (_eggsCounter < 3)
                        {
                            me->SummonCreature(NPC_BLAZING_TALON_INITIATE, centerpos.GetPositionX() + 70, centerpos.GetPositionY() + 30 * v, 90, 0);
                            me->SummonCreature(NPC_BLAZING_TALON_INITIATE, centerpos.GetPositionX() - 70, centerpos.GetPositionY() - 30 * v, 90, 0);
                        }
                        if (!worms)
                        {
                            worms = true;
                            events.ScheduleEvent(EVENT_MOLTEN_EGGS, 1000);
                            events.ScheduleEvent(EVENT_WORMS, 15000);
                        }
                        events.ScheduleEvent(EVENT_PATH, 3000);
                    }
                    break;
                    case 3:
                        me->GetMotionMaster()->MovePoint(0, pospath[path++]);
                        events.ScheduleEvent(EVENT_PATH, 4900);
                        break;
                    case 4:
                        me->GetMotionMaster()->MovePoint(0, pospath[path++]);
                        events.ScheduleEvent(EVENT_PATH, 4900);
                        break;
                    case 5:
                        me->GetMotionMaster()->MovePoint(0, pospath[path++]);
                        events.ScheduleEvent(EVENT_PATH, 4900);
                        break;
                    case 6:
                        me->GetMotionMaster()->MovePoint(0, pospath[path++]);
                        events.ScheduleEvent(EVENT_PATH, 4900);
                        break;
                    case 7:
                        me->GetMotionMaster()->MovePoint(0, pospath[path]);
                        path = 0;
                        events.ScheduleEvent(EVENT_PATH, 6900);
                        cloudcount++;
                        break;
                    default:
                        break;
                    }
                    break;
                case EVENT_WORMS:
                    DespawnCreatures(NPC_PLUMP_LAVA_WORM, 300.0f);
                    DespawnCreatures(54020, 300.0f);
                    for (float wori = 0.0f; wori < 2 * M_PI; wori += 0.5*M_PI)
                    {
                        me->SummonCreature(NPC_PLUMP_LAVA_WORM,
                            centerpos.GetPositionX() + 30 * cos(wori),
                            centerpos.GetPositionY() + 30 * sin(wori),
                            centerpos.GetPositionZ() + 2);
                    }
                    events.ScheduleEvent(EVENT_WORMS, 60000);
                    break;
                case EVENT_PHASE_2:
                {
                    phase = 2;
                    DespawnCreatures(NPC_PLUMP_LAVA_WORM, 500.0f);
                    DespawnCreatures(54020, 300.0f);
                    DespawnCreatures(NPC_FIRST_MOLTEN_METEOR, 500.0f);
                    DespawnCreatures(NPC_SMALL_METEOR, 500.0f);
                    me->RemoveAllGameObjects();
                    me->MonsterYell(SAY_SKYES, LANG_UNIVERSAL, 0);
                    DoPlaySoundToSet(me, SOU_SKYES);
                    me->SetSpeed(MOVE_RUN, 30 / (2 * M_PI));
                    me->SetSpeed(MOVE_WALK, 30 / (2 * M_PI));
                    ori = float(rand_norm())*static_cast<float>(2 * M_PI);
                    float X = centerpos.GetPositionX() + 30 * cos(ori);
                    float Y = centerpos.GetPositionY() + 30 * sin(ori);
                    me->GetMotionMaster()->MovePoint(X, Y, centerpos.GetPositionZ() + 30, 0);
                    events.ScheduleEvent(EVENT_CIRCLING, 3000);
                    events.ScheduleEvent(EVENT_SPAWN_TORNADOES, 3000);
                    events.ScheduleEvent(EVENT_MOVE_OVER_CENTER, 33000);
                    events.ScheduleEvent(EVENT_CHECK_DISTANCE, 5000);
                    events.ScheduleEvent(EVENT_SPAWN_RING2, 5000);
                    events.CancelEvent(EVENT_PATH);
                    events.CancelEvent(EVENT_FIRE_CLOUD);
                    events.CancelEvent(EVENT_SPAWN_RING);
                    events.CancelEvent(EVENT_WORMS);
                    events.CancelEvent(EVENT_MOLTEN_EGGS);
                    events.CancelEvent(EVENT_HERALD_BURNING);
                    events.CancelEvent(EVENT_SECOND_FIRESTORM);
                }
                break;
                case EVENT_CHECK_DISTANCE:
                {
                    std::list<Player*> targets;
                    Trinity::AnyPlayerInObjectRangeCheck u_check(me, 500.0f);
                    Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, targets, u_check);
                    me->VisitNearbyWorldObject(500.0f, searcher);
                    for (std::list<Player*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
                    {//this will cast harsh winds on any player that is too far from the boss or is flying
                        if ((*iter)->GetExactDist2d(&centerpos) > 60.0f || (*iter)->GetPositionZ() > (centerpos.GetPositionZ() + 5.0f))
                            me->AddAura(SPELL_HARSH_WINDS, *iter);
                    }
                    events.ScheduleEvent(EVENT_CHECK_DISTANCE, 1500);
                }
                break;
                case EVENT_SPAWN_TORNADOES:
                    if (Creature* vortex = me->SummonCreature(NPC_FIERY_VORTEX, centerpos, TEMPSUMMON_MANUAL_DESPAWN))
                    {
                        vortex->SetReactState(REACT_PASSIVE);
                        vortex->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NOT_SELECTABLE);
                        vortex->CastSpell(vortex, SPELL_FIERY_VORTEX, false);
                        vortex->AddAura(339, vortex); //root
                        events.ScheduleEvent(EVENT_CONTINUE_SPAWN_TORNADOES, 2000);
                    }
                    break;
                case EVENT_CONTINUE_SPAWN_TORNADOES:
                {
                    bool orary = false;
                    int k = 1;
                    for (int i = 20; i <= 60; i += 10)
                    {
                        k++;
                        for (int j = 0; j <= k; j++)
                        {
                            if (Creature* torn = me->SummonCreature(NPC_FIERY_TORNADO, centerpos.GetPositionX() + i, centerpos.GetPositionY(), centerpos.GetPositionZ(), TEMPSUMMON_MANUAL_DESPAWN))
                                if (torn->GetAI())
                                {
                                    if (orary)
                                        torn->GetAI()->DoAction(ACTION_TURN_OR);
                                    else
                                        torn->GetAI()->DoAction(ACTION_TURN_ANT);
                                    switch (j)
                                    {
                                    case 0:
                                        torn->GetAI()->DoAction(ACTION_DIRECTION_0);
                                        break;
                                    case 1:
                                        torn->GetAI()->DoAction(ACTION_DIRECTION_1);
                                        break;
                                    case 2:
                                        torn->GetAI()->DoAction(ACTION_DIRECTION_2);
                                        break;
                                    case 3:
                                        torn->GetAI()->DoAction(ACTION_DIRECTION_3);
                                        break;
                                    case 4:
                                        torn->GetAI()->DoAction(ACTION_DIRECTION_4);
                                        break;
                                    case 5:
                                        torn->GetAI()->DoAction(ACTION_DIRECTION_5);
                                        break;
                                    }
                                }
                        }
                        orary = !orary;
                    }
                }
                break;
                case EVENT_CIRCLING:
                {
                    ori += static_cast<float>(M_PI* 0.03);
                    if (ori >= 2 * M_PI)
                        ori -= 2 * M_PI;
                    me->SetSpeed(MOVE_RUN, 30 / (M_PI));
                    me->SetSpeed(MOVE_WALK, 30 / (M_PI));
                    float X = centerpos.GetPositionX() + 30 * cos(ori);
                    float Y = centerpos.GetPositionY() + 30 * sin(ori);
                    me->GetMotionMaster()->MovePoint(0, X, Y, centerpos.GetPositionZ() + 30);
                    events.ScheduleEvent(EVENT_CIRCLING, 200);
                }
                break;
                case EVENT_MOVE_OVER_CENTER:
                {
                    if (me->ToUnit())
                    {
                        me->ToUnit()->ApplyAuraToEntry(NPC_FIERY_TORNADO, SPELL_FIERY_TORNADO, false);
                        me->ToUnit()->ApplyAuraToEntry(NPC_FIERY_VORTEX, SPELL_FIERY_VORTEX, false);
                    }
                    me->SetSpeed(MOVE_WALK, 3.2f, true);
                    me->SetSpeed(MOVE_RUN, 3.2f, true);
                    me->SetSpeed(MOVE_FLIGHT, 3.2f, true);
                    me->GetMotionMaster()->MovePoint(0, centerpos.GetPositionX(), centerpos.GetPositionY(), centerpos.GetPositionZ() + 30);
                    events.ScheduleEvent(EVENT_PHASE_3, 1000);
                    events.ScheduleEvent(EVENT_DESPAWN_TORNADOES, 2000);
                    events.CancelEvent(EVENT_CIRCLING);
                    events.CancelEvent(EVENT_SPAWN_RING2);
                }
                break;
                case EVENT_PHASE_3:
                {
                    phase = 3;
                    mana = 0;
                    me->SetPower(POWER_MANA, mana);
                    me->SetCanRegen(false);
                    me->MonsterYell(SAY_FIRE, LANG_UNIVERSAL, 0);
                    DoPlaySoundToSet(me, SOU_FIRE);
                    me->AddAura(SPELL_BURNOUT, me);
                    me->GetMotionMaster()->MoveFall();
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                    events.ScheduleEvent(EVENT_REGEN_ENERGY, 500);
                    events.CancelEvent(EVENT_CHECK_DISTANCE);
                    int v = roll_chance_i(50) ? -1 : 1;
                    me->SummonCreature(NPC_BLAZING_TALON_CLAWSHAPER, centerpos.GetPositionX() + 70, centerpos.GetPositionY() + 30 * v, 90, 0);
                    me->SummonCreature(NPC_BLAZING_TALON_CLAWSHAPER, centerpos.GetPositionX() - 70, centerpos.GetPositionY() - 30 * v, 90, 0);

                }
                break;
                case EVENT_REGEN_ENERGY:
                    mana = mana + 3;
                    me->SetPower(POWER_MANA, mana);
                    if (mana >= 50)
                        events.ScheduleEvent(EVENT_PHASE_4, 500);
                    else
                        events.ScheduleEvent(EVENT_REGEN_ENERGY, 2000);
                    break;
                case EVENT_DESPAWN_TORNADOES:
                    summons.DespawnEntry(NPC_FIERY_VORTEX);
                    summons.DespawnEntry(NPC_FIERY_TORNADO);
                    break;
                case EVENT_PHASE_4:
                    phase = 4;
                    me->SetPower(POWER_MANA, 100);
                    me->SetCanRegen(false);
                    me->MonsterYell(SAY_REBORN, LANG_UNIVERSAL, 0);
                    DoPlaySoundToSet(me, SOU_REBORN);
                    me->RemoveAura(SPELL_BURNOUT);
                    me->AddAura(SPELL_IGNITED, me);
                    me->AddAura(SPELL_BLAZING_CLAW, me);
                    events.ScheduleEvent(EVENT_REGEN_ENERGY2, 500);
                    break;
                case EVENT_REGEN_ENERGY2:
                    mana = mana + 2;
                    me->SetPower(POWER_MANA, mana);
                    if (mana >= 100)
                        events.ScheduleEvent(EVENT_PHASE_1, 500);
                    else
                        events.ScheduleEvent(EVENT_REGEN_ENERGY2, 1000);
                    break;
                default:
                    break;
                }
            }

        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_alysrazorAI(creature);
    }
};

class npc_alysrazor_majordomo : public CreatureScript
{
public:
    npc_alysrazor_majordomo() : CreatureScript("npc_alysrazor_majordomo"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_alysrazor_majordomoAI(creature);
    }

    struct npc_alysrazor_majordomoAI : public ScriptedAI
    {
        npc_alysrazor_majordomoAI(Creature *c) : ScriptedAI(c)
        {
        }

        uint32 timer;
        bool introDone;
        EventMap events;

        void InitializeAI()
        {
            introDone = false;
        }

        void Reset()
        {
            introDone = false;
            events.Reset();
        }

        void DoAction(int32 actionId)
        {
            switch (actionId)
            {
            case ACTION_NOT_FIRST_ENC:
                introDone = true;
                break;
            }
        }

        void EnterCombat(Unit* target)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
            DoZoneInCombat(me);
            if (!introDone)
            {
                me->MonsterYell(SAY_INTRO1, 0, 0);
                DoPlaySoundToSet(me, SOU_INTRO1);
                introDone = true;
                DoCastAOE(SPELL_ROOTS);
                events.ScheduleEvent(EVENT_INTRO_1, 8000);
            }
            else
            {
                me->SetVisible(false);
                events.ScheduleEvent(EVENT_START, 100);
                events.ScheduleEvent(EVENT_DESP, 200);
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_INTRO_1:
                    me->MonsterYell(SAY_INTRO2, 0, 0);
                    DoPlaySoundToSet(me, SOU_INTRO2);
                    events.ScheduleEvent(EVENT_INTRO_3, 14000);
                    events.ScheduleEvent(EVENT_START, 28000);
                    events.ScheduleEvent(EVENT_INTRO_2, 26000);
                    events.ScheduleEvent(EVENT_DESP, 35000);

                    break;
                case EVENT_INTRO_3:
                    me->MonsterYell(SAY_INTRO3, 0, 0);
                    DoPlaySoundToSet(me, SOU_INTRO3);
                    break;
                case EVENT_START:
                {
                    Creature* Alysra = me->FindNearestCreature(BOSS_ALYSRAZOR, 300, true);
                    if (!Alysra)
                        return;
                    else
                    {
                        if (GameObject* Volcano = me->FindNearestGameObject(209253, 300))
                        {
                            Volcano->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_DESTROYED);
                        }
                        DoZoneInCombat(Alysra);
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                        {
                            Alysra->CombatStart(target);
                            Alysra->Attack(target, false);
                        }
                        if (me->GetInstanceScript())
                        {
                            me->GetInstanceScript()->DoRemoveAurasDueToSpellOnPlayers(SPELL_ROOTS);
                            me->GetInstanceScript()->DoRemoveAurasDueToSpellOnPlayers(SPELL_ROOTS_DEBUFF);
                        }
                    }
                }
                break;
                case EVENT_INTRO_2:
                    me->SetDisplayId(38317);
                    me->GetMotionMaster()->MovePoint(0, 180, -270, 160);
                    me->AddUnitMovementFlag(MOVEMENTFLAG_FLYING);
                    me->SetCanFly(true);
                    me->SetDisableGravity(true);
                    me->SetHover(true);
                    me->SetSpeed(MOVE_WALK, 3.2f, true);
                    me->SetSpeed(MOVE_RUN, 3.2f, true);
                    me->SetSpeed(MOVE_FLIGHT, 3.2f, true);
                    break;
                case EVENT_DESP:
                    me->DespawnOrUnsummon();
                    break;
                default:
                    break;
                }
            }

        }
    };
};

class npc_voracious_hatchling : public CreatureScript
{
public:
    npc_voracious_hatchling() : CreatureScript("npc_voracious_hatchling"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_voracious_hatchlingAI(creature);
    }

    struct npc_voracious_hatchlingAI : public ScriptedAI
    {
        npc_voracious_hatchlingAI(Creature *c) : ScriptedAI(c)
        {
        }

        uint32 timer;
        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void DoAction(int32 actionId)
        {
            switch (actionId)
            {
            case 0:
                break;
            }
        }

        void DamageTaken(Unit* who, uint32& damage)
        {
            if (who && who->GetAura(SPELL_IMPRINTED))
            {
                damage += damage * 10;// ouch!
            }
        }

        void EnterCombat(Unit* target)
        {
            DoCast(71495);
            DoZoneInCombat(me);
            me->AddAura(SPELL_SATED, me);
            events.ScheduleEvent(EVENT_HUNGRY, 15500);
            events.ScheduleEvent(EVENT_GUSHING, 10000);
            if (Unit* target = this->SelectTarget(SELECT_TARGET_NEAREST, 0, 200, true))
                me->AddAura(SPELL_IMPRINTED, target);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_HUNGRY:
                    me->AddAura(SPELL_HUNGRY, me);
                    events.ScheduleEvent(EVENT_CHECK_WORM, 200);
                    break;
                case EVENT_GUSHING:
                    me->AddAura(RAID_MODE(99308, 100718, 100719, 100720), me->getVictim());
                    events.ScheduleEvent(EVENT_GUSHING, urand(25000, 40000));
                    break;
                case EVENT_CHECK_WORM:
                    if (Creature* worm = me->FindNearestCreature(NPC_PLUMP_LAVA_WORM, 100, true))
                        if (worm->GetExactDist2d(me) <= 4.0f)
                        {
                            worm->DespawnOrUnsummon();
                            me->RemoveAura(SPELL_HUNGRY);
                            me->RemoveAura(SPELL_TANTRUM);
                            me->AddAura(SPELL_SATED, me);
                            events.ScheduleEvent(EVENT_HUNGRY, 15500);
                            return;
                        }
                        else
                            events.ScheduleEvent(EVENT_CHECK_WORM, 200);
                    else
                        events.ScheduleEvent(EVENT_CHECK_WORM, 200);
                    break;
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_blazing_talon_initiate : public CreatureScript
{
public:
    npc_blazing_talon_initiate() : CreatureScript("npc_blazing_talon_initiate"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_blazing_talon_initiateAI(creature);
    }

    struct npc_blazing_talon_initiateAI : public ScriptedAI
    {
        npc_blazing_talon_initiateAI(Creature *c) : ScriptedAI(c), summons(me)
        {
        }

        uint32 timer;
        EventMap events;
        SummonList summons;
        bool gend;
        uint8 count;

        void InitializeAI()
        {
            gend = roll_chance_i(50);
            me->SetReactState(REACT_PASSIVE);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            summon->setActive(true);

            if (summon->GetEntry() == NPC_BRUSHFIRE)
            {
                DoZoneInCombat(summon);
                me->AddAura(98884, summon);
                summon->GetMotionMaster()->MovePoint(0, // let him go
                    summon->GetPositionX() + cos(summon->GetOrientation()) * 200,
                    summon->GetPositionY() + sin(summon->GetOrientation()) * 200,
                    summon->GetPositionZ());
                summon->SetReactState(REACT_PASSIVE);
                summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            }

        }

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* target)
        {
            me->SetReactState(REACT_PASSIVE);
            count = 0;
            me->AddUnitMovementFlag(MOVEMENTFLAG_FLYING);
            me->SetCanFly(true);
            me->SetDisableGravity(true);
            me->SetHover(true);
            DoZoneInCombat(me);
            events.ScheduleEvent(EVENT_TRANSFORM, 1500);
            events.ScheduleEvent(EVENT_DRUID_BLAST, 4500);
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!who)
                return;

            if (!who->ToCreature())
                return;

            if (me->GetDistance(who) > 200.0f)
                return;

            if (who->GetEntry() == NPC_FIRESTORM)
                me->CastSpell(me, SPELL_BLAZING_SHIELD, true);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_BRUSHFIRE:
                {
                    if (!me->HasUnitState(UNIT_STATE_CASTING))
                        DoCastVictim(SPELL_BRUSHFIRE);
                    count++;
                    if (count>urand(2, 3))
                        events.ScheduleEvent(EVENT_DRUID_BLAST, 4100);
                    else
                        events.ScheduleEvent(EVENT_BRUSHFIRE, 4100);
                }
                break;
                case EVENT_DRUID_BLAST:
                    if (!me->HasUnitState(UNIT_STATE_CASTING))
                        if (Unit* temptarget = SelectTarget(SELECT_TARGET_RANDOM_AT_THIS_FLOOR, 0, 50, true))
                            DoCast(temptarget, SPELL_FIEROBLAST);
                    count = 0;
                    events.ScheduleEvent(EVENT_BRUSHFIRE, 3100);
                    break;
                case EVENT_TRANSFORM:
                    me->SetReactState(REACT_AGGRESSIVE);
                    if (gend)
                        me->SetDisplayId(38504);// female
                    else
                        me->SetDisplayId(38503);// male
                    me->RemoveUnitMovementFlag(MOVEMENTFLAG_FLYING);
                    me->SetCanFly(false);
                    me->SetDisableGravity(false);
                    me->SetHover(false);
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveFall();
                    break;
                default:
                    break;
                }
            }

        }
    };
};

class npc_blazing_talon_clawshaper : public CreatureScript
{
public:
    npc_blazing_talon_clawshaper() : CreatureScript("npc_blazing_talon_clawshaper"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_blazing_talon_clawshaperAI(creature);
    }

    struct npc_blazing_talon_clawshaperAI : public ScriptedAI
    {
        npc_blazing_talon_clawshaperAI(Creature *c) : ScriptedAI(c), summons(me)
        {
        }

        uint32 timer;
        EventMap events;
        SummonList summons;
        bool gend;

        void InitializeAI()
        {
            gend = roll_chance_i(50);
        }

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* target)
        {
            me->AddUnitMovementFlag(MOVEMENTFLAG_FLYING);
            me->SetCanFly(true);
            me->SetDisableGravity(true);
            me->SetHover(true);
            DoZoneInCombat(me);
            events.ScheduleEvent(EVENT_TRANSFORM, 3500);
            events.ScheduleEvent(EVENT_START_CHANNELING, 5500);
            me->GetMotionMaster()->MovePoint(0, centerpos);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_CHECK_STATUS:
                    if (me->HasUnitState(UNIT_STATE_CASTING))
                    {
                        if (Creature* Aly = me->FindNearestCreature(BOSS_ALYSRAZOR, 200))
                        {
                            Aly->GetAI()->DoAction(ACTION_IGNITE);
                            events.ScheduleEvent(EVENT_CHECK_STATUS, 1000);
                        }
                    }
                    break;
                case EVENT_START_CHANNELING:
                    if (Creature* Aly = me->FindNearestCreature(BOSS_ALYSRAZOR, 200))
                    {
                        DoCast(Aly, SPELL_IGNITION);
                        events.ScheduleEvent(EVENT_CHECK_STATUS, 1000);
                    }
                    break;
                case EVENT_TRANSFORM:
                    me->SetReactState(REACT_AGGRESSIVE);
                    if (gend)
                        me->SetDisplayId(38504);// female
                    else
                        me->SetDisplayId(38503);// male
                    me->RemoveUnitMovementFlag(MOVEMENTFLAG_FLYING);
                    me->SetCanFly(false);
                    me->SetDisableGravity(false);
                    me->SetHover(false);
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveFall();
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);
                    break;
                default:
                    break;
                }
            }

            if (!me->HasUnitState(UNIT_STATE_CASTING))
                DoMeleeAttackIfReady();
        }
    };
};

class npc_alysrazor_worm : public CreatureScript
{
public:
    npc_alysrazor_worm() : CreatureScript("npc_alysrazor_worm"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_alysrazor_wormAI(creature);
    }

    struct npc_alysrazor_wormAI : public ScriptedAI
    {
        npc_alysrazor_wormAI(Creature *c) : ScriptedAI(c), summons(me)
        {
            dummy = NULL;
            ori = 0;
            timer = 0;
            flame = 0;
        }

        bool introDone;
        float ori;
        uint32 timer;
        uint32 flame;
        Creature* dummy;
        SummonList summons;

        void InitializeAI()
        {
            introDone = false;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE);
        }

        void JustDied(Unit* killer)
        {
            if (dummy)
                dummy->DespawnOrUnsummon();
        }

        void EnterCombat(Unit* target)
        {
            timer = 100;
            flame = 200;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE);
            me->SetReactState(REACT_PASSIVE);
            me->AttackStop();
            me->CastSpell(me, SPELL_LAVA_SPEW);
            ori = float(rand_norm())*static_cast<float>(2 * M_PI);
            //me->ClearUnitState(UNIT_STATE_CASTING);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;
            if (timer <= diff)
            {
                timer = 100;
                ori += 0.0628f;
                if (ori >= 2 * M_PI)
                    ori -= 2 * M_PI;
                me->SetOrientation(ori);
                me->SetFacingTo(ori);
            }
            else
                timer -= diff;
        }
    };
};

class npc_alysrazor_tornado : public CreatureScript
{
public:
    npc_alysrazor_tornado() : CreatureScript("npc_alysrazor_tornado"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_alysrazor_tornadoAI(creature);
    }

    struct npc_alysrazor_tornadoAI : public ScriptedAI
    {
        npc_alysrazor_tornadoAI(Creature *c) : ScriptedAI(c)
        {
            verse = 1;
            row = 1;
        }

        float ori;
        float row;
        float distance;
        int   verse;
        uint32 timer;

        void InitializeAI()
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }

        void DoAction(int32 actionId)
        {
            switch (actionId)
            {
            case ACTION_TURN_OR:
                verse = 1;
                break;
            case ACTION_TURN_ANT:
                verse = -1;
                break;
            case ACTION_DIRECTION_0:
            {
                distance = me->GetExactDist2d(centerpos.GetPositionX(), centerpos.GetPositionY());
                if (distance >52)
                    row = 6;
                else if (distance >42)
                    row = 5;
                else if (distance >32)
                    row = 4;
                else if (distance >22)
                    row = 3;
                else if (distance >12)
                    row = 2;
                else
                    row = 1;
                ori = (0 / row)*(2 * M_PI);
                me->SetSpeed(MOVE_RUN, distance / (4 * M_PI));
                me->SetSpeed(MOVE_WALK, distance / (4 * M_PI));
                float X = centerpos.GetPositionX() + distance*cos(ori);
                float Y = centerpos.GetPositionY() + distance*sin(ori);
                me->NearTeleportTo(centerpos.GetPositionX(), centerpos.GetPositionY(), centerpos.GetPositionZ(), 0);
                me->GetMotionMaster()->MovePoint(0, X, Y, me->GetPositionZ());
            }
            break;
            case ACTION_DIRECTION_1:
            {
                distance = me->GetExactDist2d(centerpos.GetPositionX(), centerpos.GetPositionY());
                if (distance >52)
                    row = 6;
                else if (distance >42)
                    row = 5;
                else if (distance >32)
                    row = 4;
                else if (distance >22)
                    row = 3;
                else if (distance >12)
                    row = 2;
                else
                    row = 1;
                ori = (1 / row)*(2 * M_PI);
                me->SetSpeed(MOVE_RUN, distance / (4 * M_PI));
                me->SetSpeed(MOVE_WALK, distance / (4 * M_PI));
                float X = centerpos.GetPositionX() + distance*cos(ori);
                float Y = centerpos.GetPositionY() + distance*sin(ori);
                me->NearTeleportTo(centerpos.GetPositionX(), centerpos.GetPositionY(), centerpos.GetPositionZ(), 0);
                me->GetMotionMaster()->MovePoint(0, X, Y, me->GetPositionZ());
            }
            break;
            case ACTION_DIRECTION_2:
            {
                distance = me->GetExactDist2d(centerpos.GetPositionX(), centerpos.GetPositionY());
                if (distance >52)
                    row = 6;
                else if (distance >42)
                    row = 5;
                else if (distance >32)
                    row = 4;
                else if (distance >22)
                    row = 3;
                else if (distance >12)
                    row = 2;
                else
                    row = 1;
                ori = (2 / row)*(2 * M_PI);
                me->SetSpeed(MOVE_RUN, distance / (4 * M_PI));
                me->SetSpeed(MOVE_WALK, distance / (4 * M_PI));
                float X = centerpos.GetPositionX() + distance*cos(ori);
                float Y = centerpos.GetPositionY() + distance*sin(ori);
                me->NearTeleportTo(centerpos.GetPositionX(), centerpos.GetPositionY(), centerpos.GetPositionZ(), 0);
                me->GetMotionMaster()->MovePoint(0, X, Y, me->GetPositionZ());
            }
            break;
            case ACTION_DIRECTION_3:
            {
                distance = me->GetExactDist2d(centerpos.GetPositionX(), centerpos.GetPositionY());
                if (distance >52)
                    row = 6;
                else if (distance >42)
                    row = 5;
                else if (distance >32)
                    row = 4;
                else if (distance >22)
                    row = 3;
                else if (distance >12)
                    row = 2;
                else
                    row = 1;
                ori = (3 / row)*(2 * M_PI);
                me->SetSpeed(MOVE_RUN, distance / (4 * M_PI));
                me->SetSpeed(MOVE_WALK, distance / (4 * M_PI));
                float X = centerpos.GetPositionX() + distance*cos(ori);
                float Y = centerpos.GetPositionY() + distance*sin(ori);
                me->NearTeleportTo(centerpos.GetPositionX(), centerpos.GetPositionY(), centerpos.GetPositionZ(), 0);
                me->GetMotionMaster()->MovePoint(0, X, Y, me->GetPositionZ());
            }
            break;
            case ACTION_DIRECTION_4:
            {
                distance = me->GetExactDist2d(centerpos.GetPositionX(), centerpos.GetPositionY());
                if (distance >52)
                    row = 6;
                else if (distance >42)
                    row = 5;
                else if (distance >32)
                    row = 4;
                else if (distance >22)
                    row = 3;
                else if (distance >12)
                    row = 2;
                else
                    row = 1;
                ori = (4 / row)*(2 * M_PI);
                me->SetSpeed(MOVE_RUN, distance / (4 * M_PI));
                me->SetSpeed(MOVE_WALK, distance / (4 * M_PI));
                float X = centerpos.GetPositionX() + distance*cos(ori);
                float Y = centerpos.GetPositionY() + distance*sin(ori);
                me->NearTeleportTo(centerpos.GetPositionX(), centerpos.GetPositionY(), centerpos.GetPositionZ(), 0);
                me->GetMotionMaster()->MovePoint(0, X, Y, me->GetPositionZ());
            }
            break;
            case ACTION_DIRECTION_5:
            {
                distance = me->GetExactDist2d(centerpos.GetPositionX(), centerpos.GetPositionY());
                ori = (5 / 6)*(2 * M_PI);
                me->SetSpeed(MOVE_RUN, distance / (4 * M_PI));
                me->SetSpeed(MOVE_WALK, distance / (4 * M_PI));
                float X = centerpos.GetPositionX() + distance*cos(ori);
                float Y = centerpos.GetPositionY() + distance*sin(ori);
                me->NearTeleportTo(centerpos.GetPositionX(), centerpos.GetPositionY(), centerpos.GetPositionZ(), 0);
                me->GetMotionMaster()->MovePoint(0, X, Y, me->GetPositionZ());
            }
            break;
            default:
                break;
            }

        }

        void EnterCombat(Unit* target)
        {
            timer = 1000;
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;
            if (timer <= diff)
            {
                timer = 100;
                ori += static_cast<float>(M_PI* 0.0075 * verse);
                if (ori >= 2 * M_PI)
                    ori -= 2 * M_PI;
                else if (ori < 0)
                    ori += 2 * M_PI;
                float X = centerpos.GetPositionX() + distance*cos(ori);
                float Y = centerpos.GetPositionY() + distance*sin(ori);
                me->GetMotionMaster()->MovePoint(0, X, Y, me->GetPositionZ());
            }
            else
                timer -= diff;
        }
    };
};

class npc_molten_feather : public CreatureScript
{
public:
    npc_molten_feather() : CreatureScript("npc_molten_feather") { }


    bool OnGossipHello(Player* player, Creature* creature)
    {
        switch (player->getClass())
        {
        case CLASS_MAGE:
            player->CastSpell(player, 98761, true);
            break;
        case CLASS_WARRIOR:
            player->CastSpell(player, 98762, true);
            break;
        case CLASS_WARLOCK:
            player->CastSpell(player, 98764, true);
            break;
        case CLASS_PRIEST:
            player->CastSpell(player, 98765, true);
            break;
        case CLASS_DRUID:
            player->CastSpell(player, 98766, true);
            break;
        case CLASS_ROGUE:
            player->CastSpell(player, 98767, true);
            break;
        case CLASS_HUNTER:
            player->CastSpell(player, 98768, true);
            break;
        case CLASS_PALADIN:
            player->CastSpell(player, 98769, true);
            break;
        case CLASS_SHAMAN:
            player->CastSpell(player, 98770, true);
            break;
        case CLASS_DEATH_KNIGHT:
            player->CastSpell(player, 98771, true);
            break;
        }
        if (player && player->GetAura(SPELL_MOLTEN_FEATHER) && player->GetAura(SPELL_MOLTEN_FEATHER)->GetStackAmount() >= 2)
            player->CastSpell(player, SPELL_WINGS_OF_FLAME_TRIGGER, true);
        player->CastSpell(player, SPELL_MOLTEN_FEATHER, true);
        creature->DespawnOrUnsummon();
        return true;
    }
};

class npc_harbinger_of_flame : public CreatureScript
{
public:
    npc_harbinger_of_flame() : CreatureScript("npc_harbinger_of_flame") { }

    struct npc_harbinger_of_flameAI : public ScriptedAI
    {
        npc_harbinger_of_flameAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void EnterCombat(Unit* /*target*/)
        {
            if (Creature* bird = ObjectAccessor::GetCreature(*me, me->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT)))
                DoZoneInCombat(bird, 200.0f);

            me->InterruptSpell(CURRENT_CHANNELED_SPELL);
            _events.Reset();
            _events.ScheduleEvent(EVENT_FIEROBLAST, 1);
            _events.ScheduleEvent(EVENT_FIEROCLAST_BARRAGE, 6000);
        }

        void JustReachedHome()
        {
            AlysrazorTrashEvaded(me);
        }

        void MoveInLineOfSight(Unit* unit)
        {
            if (me->isInCombat())
                return;

            if (!unit->isCharmedOwnedByPlayerOrPlayer())
                return;

            ScriptedAI::MoveInLineOfSight(unit);
        }

        void UpdateAI(uint32 diff)
        {
            if (!me->isInCombat())
                if (!me->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                    if (Creature* fireBird = me->FindNearestCreature((me->GetHomePosition().GetPositionY() > -275.0f ? NPC_BLAZING_MONSTROSITY_LEFT : NPC_BLAZING_MONSTROSITY_RIGHT), 100.0f))
                        DoCast(fireBird, SPELL_FIRE_CHANNELING);

            if (!UpdateVictim())
                return;

            _events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_FIEROBLAST:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, false, -SPELL_RIDE_MONSTROSITY))
                        DoCast(target, SPELL_FIEROBLAST_TRASH);
                    _events.RescheduleEvent(EVENT_FIEROBLAST, 500);  // cast time is longer, but thanks to UNIT_STATE_CASTING check it won't trigger more often (need this because this creature gets a stacking haste aura)
                    break;
                case EVENT_FIEROCLAST_BARRAGE:
                    DoCastAOE(SPELL_FIEROCLAST_BARRAGE);
                    _events.ScheduleEvent(EVENT_FIEROCLAST_BARRAGE, urand(9000, 12000));
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        EventMap _events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_harbinger_of_flameAI(creature);
    }
};

class npc_blazing_monstrosity : public CreatureScript
{
public:
    npc_blazing_monstrosity() : CreatureScript("npc_blazing_monstrosity") { }

    struct npc_blazing_monstrosityAI : public PassiveAI
    {
        npc_blazing_monstrosityAI(Creature* creature) : PassiveAI(creature), _summons(creature)
        {
        }

        void EnterEvadeMode()
        {
            _summons.DespawnAll();
            _events.Reset();
            PassiveAI::EnterEvadeMode();
        }

        void JustDied(Unit* /*killer*/)
        {
            _summons.DespawnAll();
            _events.Reset();
        }

        void JustReachedHome()
        {
            AlysrazorTrashEvaded(me);
        }

        void EnterCombat(Unit* /*target*/)
        {
            DoZoneInCombat();
            me->RemoveAurasDueToSpell(SPELL_SLEEP_ULTRA_HIGH_PRIORITY);
            me->PlayOneShotAnimKit(ANIM_KIT_BIRD_WAKE);
            _events.Reset();
            _events.ScheduleEvent(EVENT_START_SPITTING, 6000);
            _events.ScheduleEvent(EVENT_CONTINUE_SPITTING, 9000);
        }

        void PassengerBoarded(Unit* passenger, int8 /*seat*/, bool apply)
        {
            if (!apply)
                return;

            // Our passenger is another vehicle (boardable by players)
            DoCast(passenger, SPELL_SHARE_HEALTH, true);
            passenger->setFaction(35);
            passenger->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

            // Hack to relocate vehicle on vehicle so exiting players are not moved under map
            Movement::MoveSplineInit init(passenger);
            init.DisableTransportPathTransformations();
            init.MoveTo(0.6654003f, 0.0f, 1.9815f);
            init.SetFacing(0.0f);
            init.Launch();
        }

        void JustSummoned(Creature* summon)
        {
            _summons.Summon(summon);
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            _summons.Despawn(summon);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_START_SPITTING:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, false, -SPELL_RIDE_MONSTROSITY))
                        DoCast(target, SPELL_MOLTEN_BARRAGE);
                    break;
                case EVENT_CONTINUE_SPITTING:
                    DoCastAOE(SPELL_MOLTEN_BARRAGE_EFFECT);
                    if (Creature* egg = me->FindNearestCreature(NPC_EGG_PILE, 100.0f))
                        egg->AI()->DoAction(me->GetEntry());
                    break;
                }
            }
        }

    private:
        SummonList _summons;
        EventMap _events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_blazing_monstrosityAI(creature);
    }
};

class npc_molten_barrage : public CreatureScript
{
public:
    npc_molten_barrage() : CreatureScript("npc_molten_barrage") { }

    struct npc_molten_barrageAI : public NullCreatureAI
    {
        npc_molten_barrageAI(Creature* creature) : NullCreatureAI(creature)
        {
        }

        void AttackStart(Unit* target)
        {
            if (target)
                me->GetMotionMaster()->MoveFollow(target, 0.0f, 0.0f, MOTION_SLOT_IDLE);
        }

        void IsSummonedBy(Unit* /*summoner*/)
        {
            DoCastAOE(SPELL_AGGRO_CLOSEST, true);
            DoCast(me, SPELL_MOLTEN_BARRAGE_VISUAL);
            DoCast(me, SPELL_INVISIBILITY_AND_STEALTH_DETECTION, true);
        }

        void MovementInform(uint32 movementType, uint32 /*pointId*/)
        {
            if (movementType != EFFECT_MOTION_TYPE)
                return;

            DoCastAOE(SPELL_AGGRO_CLOSEST);
            me->ClearUnitState(UNIT_STATE_CANNOT_TURN);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_molten_barrageAI(creature);
    }
};

class npc_egg_pile : public CreatureScript
{
public:
    npc_egg_pile() : CreatureScript("npc_egg_pile") { }

    struct npc_egg_pileAI : public CreatureAI
    {
        npc_egg_pileAI(Creature* creature) : CreatureAI(creature)
        {
        }

        void AttackStart(Unit* /*target*/) { }

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            _events.Reset();
            _callHatchlingSpell = 0;
        }

        void JustDied(Unit* /*killer*/)
        {
            _events.Reset();
            std::list<Creature*> eggs;
            GetCreatureListWithEntryInGrid(eggs, me, NPC_MOLTEN_EGG_TRASH, 20.0f);
            for (std::list<Creature*>::const_iterator itr = eggs.begin(); itr != eggs.end(); ++itr)
                (*itr)->CastSpell(*itr, SPELL_ALYSRAZOR_COSMETIC_EGG_XPLOSION, TRIGGERED_FULL_MASK);

            DoCast(me, SPELL_ALYSRAZOR_COSMETIC_EGG_XPLOSION, true);
        }

        void JustReachedHome()
        {
            AlysrazorTrashEvaded(me);
        }

        void DoAction(int32 action)
        {
            if (action != NPC_BLAZING_MONSTROSITY_LEFT &&
                action != NPC_BLAZING_MONSTROSITY_RIGHT)
                return;

            if (action == NPC_BLAZING_MONSTROSITY_LEFT)
                Talk(EMOTE_CRACKING_EGGS);

            _callHatchlingSpell = (action == NPC_BLAZING_MONSTROSITY_LEFT) ? SPELL_MOLTEN_EGG_TRASH_CALL_L : SPELL_MOLTEN_EGG_TRASH_CALL_R;
            DoZoneInCombat();
            _events.Reset();
            _events.ScheduleEvent(EVENT_SUMMON_SMOULDERING_HATCHLING, 1);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            _events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SUMMON_SMOULDERING_HATCHLING:
                {
                    std::list<Creature*> eggs;
                    MoltenEggCheck check(me);
                    Trinity::CreatureListSearcher<MoltenEggCheck> searcher(me, eggs, check);
                    me->VisitNearbyGridObject(20.0f, searcher);
                    if (!eggs.empty())
                    {
                        Creature* egg = Trinity::Containers::SelectRandomContainerElement(eggs);
                        egg->CastSpell(egg, SPELL_SUMMON_SMOULDERING_HATCHLING, TRIGGERED_FULL_MASK);
                        egg->SetDisplayId(MODEL_INVISIBLE_STALKER);
                        egg->m_Events.AddEvent(new RespawnEggEvent(egg), egg->m_Events.CalculateTime(5000));
                    }

                    if (_callHatchlingSpell)
                        DoCastAOE(_callHatchlingSpell, true);
                    _events.ScheduleEvent(EVENT_SUMMON_SMOULDERING_HATCHLING, urand(6000, 10000));
                    break;
                }
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        EventMap _events;
        uint32 _callHatchlingSpell;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_egg_pileAI(creature);
    }
};

class npc_herald_burning : public CreatureScript
{
public:
    npc_herald_burning() : CreatureScript("npc_herald_burning") { }

    struct npc_herald_burningAI : public ScriptedAI
    {
        npc_herald_burningAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;
        uint8 rand;

        void Reset()
        {
            events.Reset();
        }

        void IsSummonedBy(Unit* summoner)
        {
            rand = urand(0, 4);
            me->CastSpell(me, 99199);
            me->MonsterYell(SAY_HERALD, 0, 0);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_PASSIVE);
            me->AttackStop();
            me->StopMoving();
            me->CastSpell(me, SPELL_CATACLYSM_1, false);
            me->SummonCreature(NPC_METEOR_CALLER, meteorpath[rand], TEMPSUMMON_TIMED_DESPAWN, 20000);
        }

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_herald_burningAI(creature);
    }
};

class npc_molten_meteor : public CreatureScript
{
public:
    npc_molten_meteor() : CreatureScript("npc_molten_meteor"){}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_molten_meteorAI(creature);
    }

    struct npc_molten_meteorAI : public ScriptedAI
    {
        npc_molten_meteorAI(Creature* creature) : ScriptedAI(creature)
        {

        }

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->SetReactState(REACT_PASSIVE);
            me->CastSpell(me, SPELL_METEORIC_IMPACT);
            events.ScheduleEvent(EVENT_METEOR_PATH, 2000);
        }
        void JustDied(Unit* killer)
        {
            float X = me->GetPositionX();
            float Y = me->GetPositionY();
            float Z = me->GetPositionZ();
            me->CastSpell(me, SPELL_METEORIC_IMPACT);
            me->SummonGameObject(GO_MOLTEN_METEOR, X, Y, Z, 0, 0, 0, 0, 0, me->GetRespawnTime());
            me->StopMoving();
        }

        void UpdateAI(uint32 diff)
        {

            if (Creature* caller = me->FindNearestCreature(NPC_METEOR_CALLER, 5.0f, true))
            {
                caller->ToCreature()->DisappearAndDie();
                events.ScheduleEvent(EVENT_METEOR_EXPLOSION, 1000);
            }
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    if (me->HealthAbovePct(1))
                        events.ScheduleEvent(EVENT_METEOR_STOP, 500);

                case EVENT_METEOR_PATH:
                    me->SetSpeed(MOVE_WALK, 0.6f, true);
                    me->SetSpeed(MOVE_RUN, 0.6f, true);
                    if (Creature* caller = me->FindNearestCreature(NPC_METEOR_CALLER, 100.0f, true))
                        me->GetMotionMaster()->MoveChase(caller);
                    break;

                case EVENT_METEOR_EXPLOSION:
                    me->CastSpell(me, SPELL_METEORIC_IMPACT, true);
                    me->SummonCreature(NPC_SMALL_METEOR, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 12000);
                    me->SummonCreature(NPC_SMALL_METEOR, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 12000);
                    me->SummonCreature(NPC_SMALL_METEOR, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 12000);
                    me->DisappearAndDie();
                    break;
                }
            }
        }
    };
};

class npc_small_meteor : public CreatureScript
{
public:
    npc_small_meteor() : CreatureScript("npc_small_meteor") { }

    struct npc_small_meteorAI : public ScriptedAI
    {
        npc_small_meteorAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        void IsSummonedBy(Unit* summoner)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetSpeed(MOVE_WALK, 1.0f, true);
            me->SetSpeed(MOVE_RUN, 1.0f, true);
            me->AddAura(SPELL_MOLTEN_BOULDER, me);
            me->GetMotionMaster()->MovePoint(0, meteorpath[urand(0, 4)]);
        }

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_small_meteorAI(creature);
    }
};

class spell_alysrazor_cosmetic_egg_xplosion : public SpellScriptLoader
{
public:
    spell_alysrazor_cosmetic_egg_xplosion() : SpellScriptLoader("spell_alysrazor_cosmetic_egg_xplosion") { }

    class spell_alysrazor_cosmetic_egg_xplosion_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_alysrazor_cosmetic_egg_xplosion_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sCreatureDisplayInfoStore.LookupEntry(MODEL_INVISIBLE_STALKER))
                return false;
            return true;
        }

        void HandleExplosion(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            GetHitUnit()->SetDisplayId(MODEL_INVISIBLE_STALKER);
            if (Creature* creature = GetHitCreature())
                creature->DespawnOrUnsummon(4000);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_alysrazor_cosmetic_egg_xplosion_SpellScript::HandleExplosion, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_alysrazor_cosmetic_egg_xplosion_SpellScript();
    }
};

class spell_alysrazor_turn_monstrosity : public SpellScriptLoader
{
public:
    spell_alysrazor_turn_monstrosity() : SpellScriptLoader("spell_alysrazor_turn_monstrosity") { }

    class spell_alysrazor_turn_monstrosity_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_alysrazor_turn_monstrosity_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_GENERIC_DUMMY_CAST))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_KNOCKBACK_RIGHT))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_KNOCKBACK_LEFT))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_KNOCKBACK_FORWARD))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_KNOCKBACK_BACK))
                return false;
            return true;
        }

        void KnockBarrage(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            GetHitUnit()->GetMotionMaster()->MoveIdle();
            if (TempSummon* summ = GetHitUnit()->ToTempSummon())
                if (Unit* summoner = summ->GetSummoner())
                    GetHitUnit()->CastSpell(summoner, SPELL_GENERIC_DUMMY_CAST, TRIGGERED_FULL_MASK);

            float angle = 0.0f;
            if (Unit* bird = GetCaster()->GetVehicleBase())
            {
                bird->SetInFront(GetHitUnit());
                angle = bird->GetOrientation();
            }

            uint32 spellId = 0;
            switch (GetSpellInfo()->Id)
            {
            case SPELL_RIGHT_SIDE_SMACK_R:
            case SPELL_RIGHT_SIDE_SMACK_L:
                spellId = SPELL_KNOCKBACK_RIGHT;
                angle -= M_PI * 0.5f;
                break;
            case SPELL_LEFT_SIDE_SMACK_R:
            case SPELL_LEFT_SIDE_SMACK_L:
                spellId = SPELL_KNOCKBACK_LEFT;
                angle += M_PI * 0.5f;
                break;
            case SPELL_HEAD_BONK_R:
            case SPELL_HEAD_BONK_L:
                spellId = SPELL_KNOCKBACK_FORWARD;
                break;
            case SPELL_TICKLE_R:
            case SPELL_TICKLE_L:
                spellId = SPELL_KNOCKBACK_BACK;
                angle -= M_PI;
                break;
            }

            // Cannot wait for object update to process facing spline, it's needed in next spell cast
            GetHitUnit()->SetOrientation(angle);
            GetHitUnit()->SetFacingTo(angle);
            GetHitUnit()->AddUnitState(UNIT_STATE_CANNOT_TURN);
            GetHitUnit()->CastSpell(GetHitUnit(), spellId, TRIGGERED_FULL_MASK);
        }

        void TurnBird(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            GetHitUnit()->PlayOneShotAnimKit(ANIM_KIT_BIRD_TURN);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_alysrazor_turn_monstrosity_SpellScript::KnockBarrage, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            OnEffectHitTarget += SpellEffectFn(spell_alysrazor_turn_monstrosity_SpellScript::TurnBird, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_alysrazor_turn_monstrosity_SpellScript();
    }
};

class spell_alysrazor_aggro_closest : public SpellScriptLoader
{
public:
    spell_alysrazor_aggro_closest() : SpellScriptLoader("spell_alysrazor_aggro_closest") { }

    class spell_alysrazor_aggro_closest_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_alysrazor_aggro_closest_SpellScript);

        bool Load()
        {
            return GetCaster()->GetTypeId() == TYPEID_UNIT;
        }

        void HandleEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            float curThreat = GetCaster()->getThreatManager().getThreat(GetHitUnit(), true);
            GetCaster()->getThreatManager().addThreat(GetHitUnit(), -curThreat + 50000.0f / std::min(1.0f, GetCaster()->GetDistance(GetHitUnit())));
        }

        void UpdateThreat()
        {
            GetCaster()->ClearUnitState(UNIT_STATE_CASTING);
            GetCaster()->GetAI()->AttackStart(GetCaster()->ToCreature()->SelectVictim());
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_alysrazor_aggro_closest_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
            AfterCast += SpellCastFn(spell_alysrazor_aggro_closest_SpellScript::UpdateThreat);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_alysrazor_aggro_closest_SpellScript();
    }
};

class spell_alysrazor_fieroblast : public SpellScriptLoader
{
public:
    spell_alysrazor_fieroblast() : SpellScriptLoader("spell_alysrazor_fieroblast") { }

    class spell_alysrazor_fieroblast_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_alysrazor_fieroblast_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_FIRE_IT_UP))
                return false;
            return true;
        }

        void FireItUp()
        {
            GetCaster()->CastSpell(GetCaster(), SPELL_FIRE_IT_UP, TRIGGERED_FULL_MASK);
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_alysrazor_fieroblast_SpellScript::FireItUp);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_alysrazor_fieroblast_SpellScript();
    }
};

class spell_aly_gushing_wound : public SpellScriptLoader
{
public:
    spell_aly_gushing_wound() : SpellScriptLoader("spell_aly_gushing_wound") { }

    class spell_aly_gushing_wound_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_aly_gushing_wound_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetOwner() && GetOwner()->ToUnit() && GetOwner()->ToUnit()->GetHealthPct()< 50)
            {
                GetOwner()->ToUnit()->RemoveAura(99308);
                GetOwner()->ToUnit()->RemoveAura(100718);
                GetOwner()->ToUnit()->RemoveAura(100719);
                GetOwner()->ToUnit()->RemoveAura(100720);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_aly_gushing_wound_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_aly_gushing_wound_AuraScript();
    }
};

class spell_aly_blazing_power : public SpellScriptLoader
{
public:
    spell_aly_blazing_power() : SpellScriptLoader("spell_aly_blazing_power") { }

    class spell_aly_blazing_power_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_aly_blazing_power_AuraScript);

        void OnAPPLY(AuraEffect const* aurEff, AuraEffectHandleModes mode)
        {

            if (!GetOwner() || !GetOwner()->ToUnit())
                return;
            else
            {
                Unit* owner = GetOwner()->ToUnit();
                if (owner->GetAura(SPELL_WINGS_OF_FLAME_BUFF))
                    owner->GetAura(SPELL_WINGS_OF_FLAME_BUFF)->RefreshDuration();
                if (owner->GetAura(SPELL_BLAZING_POWER) && owner->GetAura(SPELL_BLAZING_POWER)->GetStackAmount() >24)
                    owner->AddAura(SPELL_ALYSRA_RAZOR, owner);
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_aly_blazing_power_AuraScript::OnAPPLY, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_aly_blazing_power_AuraScript();
    }
};

class spell_cataclysm_event : public SpellScriptLoader
{
public:
    spell_cataclysm_event() : SpellScriptLoader("spell_cataclysm_event") { }

    class spell_cataclysm_event_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_cataclysm_event_SpellScript);

        void HandleSendEvent(SpellEffIndex /*effIndex*/)
        {
            if (Unit* caster = GetCaster())
            {
                caster->CastSpell(caster, SPELL_FIRST_MOLTEN_METEOR);
            }
        }

        void Register()
        {
            OnEffectHit += SpellEffectFn(spell_cataclysm_event_SpellScript::HandleSendEvent, EFFECT_0, SPELL_EFFECT_SEND_EVENT);
        }
    };
    SpellScript* GetSpellScript() const
    {
        return new spell_cataclysm_event_SpellScript();
    }
};

class TargetFilter
{
public:
    explicit TargetFilter(Unit* caster) : _caster(caster) { }

    bool operator()(WorldObject* unit) const
    {
        return !unit->IsWithinLOSInMap(_caster);
    }

private:
    Unit* _caster;
};

class spell_firestorm : public SpellScriptLoader
{
public:
    spell_firestorm() : SpellScriptLoader("spell_firestorm") { }

    class spell_firestorm_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_firestorm_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(TargetFilter(GetCaster()));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_firestorm_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_firestorm_SpellScript();
    }
};

class spell_lava_spew : public SpellScriptLoader
{
public:
    spell_lava_spew() : SpellScriptLoader("spell_lava_spew") { }

    class spell_lava_spew_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_lava_spew_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(TargetFilter(GetCaster()));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_lava_spew_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_24);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_lava_spew_SpellScript();
    }
};


void AddSC_boss_alysrazor()
{
    new boss_alysrazor();
    new npc_alysrazor_majordomo();
    new npc_voracious_hatchling();
    new npc_blazing_talon_initiate();
    new npc_blazing_talon_clawshaper();
    new npc_alysrazor_worm();
    new npc_alysrazor_tornado();
    new npc_molten_feather();
    new npc_harbinger_of_flame();
    new npc_blazing_monstrosity();
    new npc_molten_barrage();
    new npc_egg_pile();
    new npc_herald_burning();
    new npc_molten_meteor();
    new npc_small_meteor();
    new spell_alysrazor_cosmetic_egg_xplosion();
    new spell_alysrazor_turn_monstrosity();
    new spell_alysrazor_aggro_closest();
    new spell_alysrazor_fieroblast();
    new spell_aly_gushing_wound();
    new spell_aly_blazing_power();
    new spell_cataclysm_event();
    new spell_firestorm();
    new spell_lava_spew();
}