#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "MapManager.h"
#include "Spell.h"
#include "Vehicle.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CreatureTextMgr.h"

#include "bastion_of_twilight.h"

#define EMOTE_ARION_THUNDERSHOCK       "The air around you crackles with energy..."
#define EMOTE_TERRASTRA_QUAKE          "The ground beneath you rumbles ominously..."

/*
Merge order Elementium Monstrosity + talks:
Arion_P3                "An impressive display..."
TERRASTRA_P3            "...to have made it this far."
Feludius_P3             "But now witness true power..."
IGNACIOUS_P3            "The fury of the elements! "
*/

enum FeludiusYells
{
  SAY_F_AGGRO = 0,
  SAY_F_KILL,
  SAY_F_GLACIATE,
  SAY_F_ELEMENTS,
  SAY_F_MONSTROSITY,
};

enum IgnaciousYells
{
  SAY_I_AGGRO = 0,
  SAY_I_KILL,
  SAY_I_RISING_FLAMES,
  SAY_I_ELEMENTS,
  SAY_I_MONSTROSITY,
};

enum TerrastraYells
{
  SAY_T_KILL = 0,
  SAY_T_GRAVITY_WELL,
  SAY_T_QUAKE,
  SAY_T_ELEMENTS,
  SAY_T_MONSTROSITY,
};

enum ArionYells
{
  SAY_A_AGGRO = 0,
  SAY_A_KILL,
  SAY_A_CALL_WINDS,
  SAY_A_ELEMENTS,
  SAY_A_MONSTROSITY,
};

enum MonstrosityYells
{
  SAY_M_AGGRO = 0,
  SAY_M_KILL,
  SAY_M_KILL_2,
  SAY_M_DIE,
  SAY_M_GRAVITY_CRUSH,
};

enum Spells // Non-commented spells work completely. 88919 to 88922 visuals before encounter.
{
    /***** GENERAL *****/
    SPELL_HITTIN_YA                          = 94132, // tank exclusion
    SPELL_HITTIN_YA_PLAYER                   = 94131, // tank exclusion

    SPELL_SELF_ROOT                          = 42716,
    SPELL_PACIFY_SELF                        = 63726,

    SPELL_TELEPORT_VISUAL                    = 87459,
    SPELL_SUMMON_MONSTROSITY_EXPLODE         = 84207,
    SPELL_ELEMENTAL_STASIS                   = 82285, // With AddAura.

    /***** FELUDIUS *****/
    SPELL_HYDRO_LANCE                        = 82752,

    SPELL_WATER_BOMB                         = 82699, // See next spell, is the dummy effect spell on EFFECT_0. Script: Summon npc under target.
    SPELL_WATER_BOMB_DMG                     = 82700, // Npc casts this on self. Causes Waterlogged on targets.
    SPELL_WATERLOGGED                        = 82762, // Only cast on spot, no radius.

    SPELL_FROZEN                             = 82772, // Feludius's Glaciate causes all Waterlogged enemies to become Frozen
    SPELL_GLACIATE                           = 82746, // Script to decrease damage with distance.
    // Above spell has the extra script effect: Players afflicted by Waterlogged when Feludius uses Glaciate become Frozen and suffer additional damage.

    SPELL_HEART_OF_ICE                       = 82665, // Triggers 82667 damage spell.
    //Script: Deals increasing Frost damage every 2 sec, but also causes nearby players within 10 yards to become Frost Imbued.
    SPELL_FROST_IMBUED                      = 82666, // Has dummy aura, needs script to increase damage done to Ignacious.
    SPELL_ICY_REPRISAL                      = 82817,

    // CAST PHASE
    SPELL_FROZEN_ORB_PRE_SUMMON             = 92267,
    SPELL_FROZEN_ORB_SUMMON                 = 92269, // Summons orb after 5 sec.

    SPELL_GLACIATE_DAMAGE                   = 92548,
    SPELL_FROZEN_ORB_PERIODIC               = 92302, // trigger SPELL_FROZEN_ORB_DUMMY 500 ms 5 yards
    SPELL_FROZEN_ORB_DUMMY                  = 92303, // target area enemy
    SPELL_FROZEN_ORB_INCREASE_RUN_SPEED     = 99250, // Increase by 2%. Needed every sec. Mob on self.
    SPELL_FROST_BEACON                      = 92307, // Aura cast by npc on Frozen Orb target. HEROIC!

    /***** IGNACIOUS *****/
    SPELL_FLAME_TORRENT                      = 82777,

    SPELL_BURNING_BLOOD                      = 82660, // Script: Deals increasing Fire damage every 2 sec, but also causes nearby players within 10 yards to become Flame Imbued.
    SPELL_FLAME_IMBUED                       = 82663, // Has dummy aura, needs script to increase damage done to Feludius.
    SPELL_BURNING_REPRISAL                   = 93352,

    SPELL_INFERNO_RUSH_JUMP                  = 82856, // Jump and missile with damage and knockback.
    SPELL_INFERNO_RUSH_CHARGE                = 82859, // charge back to the last target
    SPELL_INFERNO_LEAP_MISSILE               = 82857,
    SPELL_INFERNO_RUSH_AURA                  = 88579, // Aura for the npc. Triggers 82860 - damage spell every 0.5 sec. Npc is defined, summoned on jump and then follows Ignacious.
    //Ignacious leaps to a distant target, knocking away all enemies within 10 yards of the point of impact and inflicting 23125 to 26875 Fire damage.
    //Ignacious then rushes back to his last target, leaving a trail of fire in his wake. Standing within this fire inflicts 13875 to 16125 Fire damage every 0.5 sec.

    SPELL_AEGIS_OF_FLAME                     = 82631,
    SPELL_RISING_FLAMES                      = 82636, // Aura with trigger every 2 sec. for damage spell 82643. Works.
    SPELL_RISING_FLAMES_INC_DMG              = 82639, // Each time Rising Flames pulses, Ignacious increases his damage dealt by 3/5% for 15 sec.

    // CAST PHASE
    SPELL_FLAME_STRIKE_MARKER                = 92211,
    SPELL_FLAME_STRIKE_TARGET                = 92212, // Visual dummy for hit area.

    SPELL_FLAME_STRIKE_DAMAGE                = 92215, // HEROIC!
    SPELL_FLAME_STRIKE_PERIODIC              = 92214, // Triggered by above.


    /***** ARION *****/
    SPELL_CALL_WINDS                         = 83491, // Summons 44747 Violent Cyclone.
    SPELL_LASHING_WINDS_VISUAL               = 83472, // Visual dummy for npc.
    SPELL_SWIRLING_WINDS                     = 83500, // Inflicted by Violent Cyclone to players it touches. Makes them levitate 2 mins.
    // The Grounded effect cancels the Swirling Winds effect.

    SPELL_LIGHTNING_ROD                      = 83099, // Dummy for marking Chain Lightning target. 25-player raid -> 3 players affected.
    SPELL_CHAIN_LIGHTNING                    = 83300,
    SPELL_THUNDERSHOCK                       = 83067, // Players with the Grounded effect take greatly reduced damage.

    SPELL_DISPERSE                           = 83087, // Has Script Effect for 83078.
    SPELL_DISPERSE_TELEPORT                  = 83078, // Actual teleport within 5 yards of target. Cast on random raid member in room through above.
    SPELL_LIGHTNING_BLAST                    = 83070, // Cast on Arion's tank immediately after Disperse.

    // CAST PHASE
    SPELL_STATIC_OVERLOAD                    = 92067, // On a target, HEROIC!
    SPELL_STATIC_OVERLOAD_DMG_AURA           = 92068, // Triggered by above on allies within 10 y. Adds this 10s aura to them.

    /***** TERRASTRA *****/
    // ATTACK PHASE - ToDo: Script Harden Skin.
    SPELL_GRAVITY_WELL_DECR_MOVEMENT         = 83587, // Works completely.
    AURA_GRAVITY_WELL_PULL                   = 79333,
    AURA_GRAVITY_WELL_DMG_GROUNDED           = 83579,
    SPELL_GRAVITY_WELL_PULL                  = 79332, // Trigg by aura, 10y.
    SPELL_GRAVITY_WELL_DMG_GROUNDED          = 83578, // Trigg by aura, works properly.
    SPELL_GRAVITY_WELL_VISUAL                = 79245, // Mob on self. - 10y vis.
    SPELL_GRAV_WELL_MOB_VISUAL               = 95760, // Mob on self. - self vis.
    SPELL_GROUNDED                           = 83581, // The Swirling Winds effect cancels the Grounded effect.

    SPELL_ERUPTION                           = 83692, // Damage spell, works completely.
    SPELL_ERUPTION_SUMMON_MOB                = 83661, // Summon mob who casts this on self.
    SPELL_ERUPTION_VISUAL                    = 83662, // Visual mob aura.

    SPELL_HARDEN_SKIN                        = 83718, // Eff 3 needs script for dummy to return on remove 50% damage received.
    SPELL_QUAKE                              = 83565, // Players with the Swirling Winds effect take no damage.

    // CAST PHASE
    SPELL_GRAVITY_CORE                       = 92075, // On a target, HEROIC!
    SPELL_GRAVITY_CORE_SLOW                  = 92076, // Triggered by above on allies within 10 y. Adds this 10s aura to them.

    /***** ELEMENTIUM MONSTROSITY *****/
    SPELL_LAVA_SEED                          = 84913, // Dummy cast by boss. Needs script.
    AURA_LAVA_SEED_VISUAL                    = 84911, // Npc on self.
    SPELL_LAVA_EXPLODE                       = 84912, // Works properly.

    SPELL_GRAVITY_CRUSH                      = 84948, // This is damage spell. In 10 player 1 target, 25 player three targets trapped.
    SPELL_GRAVITY_CRUSH_CONTROL_VEHICLE      = 84952, // This is control vehicle spell.

    SPELL_ELECTRIC_INSTABILITY_TARGET        = 84529, // On a single player, handled through dummy script for spell.
    SPELL_ELECTRIC_INSTABILITY_AURA          = 84526, // Triggers dummy spell every sec.
    SPELL_ELECTRIC_INSTABILITY_DUMMY         = 84527, // Dummy spell, casts target spell on each player.

    SPELL_CRYOGENIC_AURA                     = 84918,
    SPELL_LIQUID_ICE_GROW                    = 84917, // Increase size 40%. Periodically cast by npc.
    SPELL_LIQUID_ICE_PERIODIC                = 84914, // Npc on self.
    SPELL_LIQUID_ICE_DAMAGE                  = 84915, // Damage spell triggered by above. Needs 6y radius.
};

enum Creatures
{
    // Bosses
    NPC_WATER_BOMB   = 44201, // Scripted.
    NPC_INFERNO_RUSH = 47501, // Scripted on Ignacious boss script.
    NPC_CALL_WINDS   = 44747, // Scripted.
    NPC_GRAVITY_WELL = 44824, // Scripted.
    NPC_ERUPTION_TGT = 44845, // Scripted on Terrastra boss script.
    // Heroic
    NPC_FROZEN_ORB_SUMMONER = 49517,
    NPC_FROZEN_ORB   = 49518, // Gets a target, goes to it casting beacon on it, casts Glaciate if reaches. // Scripted.
    NPC_ROCKET_STRIKE = 34047,
    NPC_FLAME_STRIKE = 49432, // Summ by boss on random target. Puts visual on self then after 5 secs comes damage spell. // Scripted.
    // Monstrosity
    NPC_LIQUID_ICE   = 45452, // Scripted.
    NPC_LAVA_SEED    = 48538, // Scripted.
    NPC_GRAVITY_CRUSH = 45476, // Scripted.
};

enum Models
{
    MODEL_FELUDIUS                           = 34822,
    MODEL_IGNACIOUS                          = 34821,
    MODEL_ARION                              = 34823,
    MODEL_TERRASTRA                          = 34824,
    MODEL_INVISIBLE                          = 11686,
};

enum Events
{
    /***** Controller *****/
    EVENT_MOVE_AND_INVISIBLE = 1, // Switch to p3 and wait for last phase.
    EVENT_TIME_TO_SPAWN_MONSTROSITY,

    EVENT_COUNCIL_DISPLAY_VISUAL,
    EVENT_FELUDIUS_MOVE_CENTER,
    EVENT_IGNACIOUS_MOVE_CENTER,
    EVENT_ARION_MOVE_CENTER,
    EVENT_TERRASTRA_MOVE_CENTER,
    EVENT_SUMMON_MONSTROSITY,

    EVENT_CAST_STASIS,
    EVENT_REMOVE_STASIS,

    /***** Bosses *****/
    // Feludius
    EVENT_HYDRO_LANCE,
    EVENT_WATER_BOMB,
    EVENT_GLACIATE,
    EVENT_HEART_OF_ICE,

    EVENT_FROST_ORB, // Heroic.

    // Ignacious
    EVENT_SAY_I_AGGRO,
    EVENT_FLAME_TORRENT,
    EVENT_BURNING_BLOOD,
    EVENT_INFERNO_RUSH,
    EVENT_INFERNO_RUSH_MISSILE,
    EVENT_SUMMON_INFERNO_RUSH_NPC,
    EVENT_AEGIS_OF_FLAME,
    EVENT_RISING_FLAMES,

    EVENT_FLAME_STRIKE, // Heroic.

    // Arion
    EVENT_SAY_A_AGGRO,
    EVENT_CALL_WINDS,
    EVENT_LIGHTNING_ROD,
    EVENT_CHAIN_LIGHTNING,
    EVENT_THUNDERSHOCK,
    EVENT_DISPERSE,
    EVENT_LIGHTNING_BLAST,

    EVENT_STATIC_OVERLOAD, // Heroic.

    // Terrastra
    EVENT_GRAVITY_WELL,
    EVENT_ERUPTION,
    EVENT_HARDEN_SKIN,
    EVENT_QUAKE,

    EVENT_GRAVITY_CORE, // Heroic.
    EVENT_LAVA_SEED,
    EVENT_GRAVITY_CRUSH,

    /***** Npc's *****/
    EVENT_WATER_BOMB_EXPLODE,
    EVENT_CHECK_WINDS_PLAYER,
    EVENT_SEED_EXPLODE,
    EVENT_FLAME_STRIKE_DAMAGE,
    EVENT_ATTACK_FROZEN_TARGET,
    EVENT_CHECK_FROZEN_TARGET,
    EVENT_FROZEN_DESPAWN,
    EVENT_RESET_CHECK
};

enum Actions // Controller Actions.
{
    ACTION_COUNCIL_MOVE_AND_INVISIBLE,
    ACTION_COUNCIL_SPAWN_MONSTROSITY,
    ACTION_COUNCIL_PHASE_THREE,
    ACTION_DESPAWN,
    ACTION_RESET_CHECK,
};

enum Phases // Boss Phases.
{
    PHASE_ALL = 0,
    PHASE_COMBAT,
    PHASE_BALCONY,
    PHASE_NONE,
};

class AllTankCheck
{
public:
    bool operator()(WorldObject* target) const
    {
        return target->GetTypeId() == TYPEID_PLAYER && target->ToPlayer()->HasAura(SPELL_HITTIN_YA_PLAYER);
    }
};

#define healthController RAID_MODE<uint32>(23190480, 73008200, 42430648, 125058752) // Controller health on different diffs.

class npc_ascendant_council_controller : public CreatureScript // 43691
{
public:
    npc_ascendant_council_controller() : CreatureScript("npc_ascendant_council_controller") { }

    struct npc_ascendant_council_controllerAI : public ScriptedAI
    {
        npc_ascendant_council_controllerAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
        }

        void Reset()
        {
            started = false;
            me->SetMaxHealth(healthController);
            me->SetHealth(healthController);

            Creature* feludius = me->FindNearestCreature(NPC_FELUDIUS, 500.0f, true);
            Creature* ignacious = me->FindNearestCreature(NPC_IGNACIOUS, 500.0f, true);
            Creature* arion = me->FindNearestCreature(NPC_ARION, 500.0f, true);
            Creature* terrastra = me->FindNearestCreature(NPC_TERRASTRA, 500.0f, true);

            if (feludius && ignacious && arion && terrastra) // Check to prevent any damn crashes.
            {
                // Set correct boss health on any diff.
                feludius->SetMaxHealth(healthController / 4);
                feludius->SetHealth(healthController / 4);
                feludius->SetDisplayId(MODEL_FELUDIUS);
                feludius->IsAIEnabled = true;
                feludius->AI()->EnterEvadeMode();
                feludius->GetMotionMaster()->MoveTargetedHome();
                ignacious->SetMaxHealth(healthController / 4);
                ignacious->SetHealth(healthController / 4);
                ignacious->SetDisplayId(MODEL_IGNACIOUS);
                ignacious->IsAIEnabled = true;
                ignacious->AI()->EnterEvadeMode();
                ignacious->GetMotionMaster()->MoveTargetedHome();
                arion->SetMaxHealth(healthController / 4);
                arion->SetHealth(healthController / 4);
                arion->SetDisplayId(MODEL_ARION);
                arion->IsAIEnabled = true;
                arion->AI()->EnterEvadeMode();
                arion->GetMotionMaster()->MoveTargetedHome();
                terrastra->SetMaxHealth(healthController / 4);
                terrastra->SetHealth(healthController / 4);
                terrastra->SetDisplayId(MODEL_TERRASTRA);
                terrastra->IsAIEnabled = true;
                terrastra->AI()->EnterEvadeMode();
                terrastra->GetMotionMaster()->MoveTargetedHome();

            }

            events.Reset();
            instance->SetData(DATA_ASCENDANT_COUNCIL, NOT_STARTED);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GROUNDED);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SWIRLING_WINDS);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            summon->setActive(true);

            switch(summon->GetEntry())
            {
                case NPC_ELEMENTIUM_MONSTROSITY:
                    summon->AddAura(SPELL_CRYOGENIC_AURA, summon);
                    summon->AddAura(SPELL_ELECTRIC_INSTABILITY_AURA, summon);
                    summon->SetMaxHealth(me->GetMaxHealth());
                    summon->SetHealth(me->GetHealth());
                    if (me->isInCombat())
                        summon->AI()->DoZoneInCombat(summon, 150.0f);
                    break;
                default:
                    if (me->isInCombat())
                        summon->AI()->DoZoneInCombat(summon, 150.0f);
                    break;
            }
        }

        void JustDied(Unit* /*victim*/)
        {
            instance->SetData(DATA_ASCENDANT_COUNCIL, DONE);
        }

        void EnterEvadeMode()
        {
            Reset();
            me->DeleteThreatList();
            me->CombatStop();
            if (Creature* element = me->FindNearestCreature(NPC_ELEMENTIUM_MONSTROSITY, 200.0f))
                element->AI()->DoAction(ACTION_DESPAWN);
            instance->SetData(DATA_ASCENDANT_COUNCIL, FAIL);
        }

        void EnterCombat(Unit* /*who*/)
        {
            started = false;
            instance->SetData(DATA_ASCENDANT_COUNCIL,IN_PROGRESS);
        }

        void DoAction(const int32 action)
        {
            switch(action)
            {
                case ACTION_COUNCIL_MOVE_AND_INVISIBLE:
                    events.ScheduleEvent(EVENT_MOVE_AND_INVISIBLE, 100);
                    events.ScheduleEvent(EVENT_TIME_TO_SPAWN_MONSTROSITY, 1000);
                    break;
                case ACTION_COUNCIL_SPAWN_MONSTROSITY:
                    events.ScheduleEvent(EVENT_COUNCIL_DISPLAY_VISUAL, 100);
                    events.ScheduleEvent(EVENT_ARION_MOVE_CENTER, 500);
                    events.ScheduleEvent(EVENT_TERRASTRA_MOVE_CENTER, 4500);
                    events.ScheduleEvent(EVENT_FELUDIUS_MOVE_CENTER, 8500);
                    events.ScheduleEvent(EVENT_IGNACIOUS_MOVE_CENTER, 12500);
                    events.ScheduleEvent(EVENT_SUMMON_MONSTROSITY, 16500);
                    events.ScheduleEvent(EVENT_CAST_STASIS, 10); // Has 17 seconds duration.
                    break;
                case ACTION_RESET_CHECK:
                    me->SetInCombatWithZone();
                    events.ScheduleEvent(EVENT_RESET_CHECK, 2000);
                    break;
                default:
                    break;
            }
        }

        void CastElementalStasis()
        {
            Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers();

            if (!PlayerList.isEmpty())
              for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                if (Player* playr = i->getSource())
                    playr->AddAura(SPELL_ELEMENTAL_STASIS, playr); // Trap all players while Monstrosity spawns.
        }

        void UpdateAI(const uint32 diff)
        {
            if (!instance)
                return;

            // At this point, before Monstrosity spawns, they have invisible models.
            Creature* feludius = me->FindNearestCreature(NPC_FELUDIUS, 500.0f, true);
            Creature* ignacious = me->FindNearestCreature(NPC_IGNACIOUS, 500.0f, true);
            Creature* arion = me->FindNearestCreature(NPC_ARION, 500.0f, true);
            Creature* terrastra = me->FindNearestCreature(NPC_TERRASTRA, 500.0f, true);

            events.Update(diff);

            if (feludius && ignacious && arion && terrastra) // Check to prevent any damn crashes.
            {
                if (arion->HealthBelowPct(25) && !started || terrastra->HealthBelowPct(25) && !started) // Check for final phase
                {
                    started = true;
                    DoAction(ACTION_COUNCIL_MOVE_AND_INVISIBLE);
                }

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_MOVE_AND_INVISIBLE:
                            feludius->RemoveAllAuras();
                            feludius->SetDisplayId(MODEL_INVISIBLE);
                            feludius->NearTeleportTo(-987.137268f, -604.340881f, 831.901978f, 2.340129f);
                            feludius->AI()->DoAction(ACTION_COUNCIL_PHASE_THREE);
                            feludius->IsAIEnabled = false;
                            feludius->CombatStop(true);
                            feludius->SetReactState(REACT_PASSIVE);
                            ignacious->RemoveAllAuras();
                            ignacious->SetDisplayId(MODEL_INVISIBLE);
                            ignacious->NearTeleportTo(-1029.934570f, -561.583069f, 831.901978f, 5.552412f);
                            ignacious->AI()->DoAction(ACTION_COUNCIL_PHASE_THREE);
                            ignacious->IsAIEnabled = false;
                            ignacious->CombatStop(true);
                            ignacious->SetReactState(REACT_PASSIVE);
                            arion->RemoveAllAuras();
                            arion->SetDisplayId(MODEL_INVISIBLE);
                            arion->NearTeleportTo(-987.451172f, -561.495605f, 831.901978f, 3.893771f);
                            arion->AI()->DoAction(ACTION_COUNCIL_PHASE_THREE);
                            arion->IsAIEnabled = false;
                            arion->CombatStop(true);
                            arion->SetReactState(REACT_PASSIVE);
                            terrastra->RemoveAllAuras();
                            terrastra->SetDisplayId(MODEL_INVISIBLE);
                            terrastra->NearTeleportTo(-1028.885132f, -602.902100f, 831.901987f, 0.783580f);
                            terrastra->AI()->DoAction(ACTION_COUNCIL_PHASE_THREE);
                            terrastra->IsAIEnabled = false;
                            terrastra->CombatStop(true);
                            terrastra->SetReactState(REACT_PASSIVE);
                            break;
                        case EVENT_TIME_TO_SPAWN_MONSTROSITY:
                            DoAction(ACTION_COUNCIL_SPAWN_MONSTROSITY);
                            break;
                        case EVENT_COUNCIL_DISPLAY_VISUAL:
                            feludius->SetDisplayId(MODEL_FELUDIUS);
                            feludius->CastSpell(feludius, SPELL_TELEPORT_VISUAL, true);
                            ignacious->SetDisplayId(MODEL_IGNACIOUS);
                            ignacious->CastSpell(ignacious, SPELL_TELEPORT_VISUAL, true);
                            arion->SetDisplayId(MODEL_ARION);
                            arion->CastSpell(arion, SPELL_TELEPORT_VISUAL, true);
                            terrastra->SetDisplayId(MODEL_TERRASTRA);
                            terrastra->CastSpell(terrastra, SPELL_TELEPORT_VISUAL, true);
                            break;
                        case EVENT_FELUDIUS_MOVE_CENTER:
                            feludius->AI()->Talk(SAY_F_ELEMENTS);
                            feludius->GetMotionMaster()->MovePoint(1, -1008.604492f, -582.742920f, 831.901245f);
                            break;
                        case EVENT_TERRASTRA_MOVE_CENTER:
                            terrastra->AI()->Talk(SAY_T_ELEMENTS);
                            terrastra->GetMotionMaster()->MovePoint(1, -1008.604492f, -582.742920f, 831.901245f);
                            break;
                        case EVENT_ARION_MOVE_CENTER:
                            arion->AI()->Talk(SAY_A_ELEMENTS);
                            arion->GetMotionMaster()->MovePoint(1, -1008.604492f, -582.742920f, 831.901245f);
                            break;
                        case EVENT_IGNACIOUS_MOVE_CENTER:
                            ignacious->AI()->Talk(SAY_I_ELEMENTS);
                            ignacious->GetMotionMaster()->MovePoint(1, -1008.604492f, -582.742920f, 831.901245f);
                            break;
                        case EVENT_SUMMON_MONSTROSITY:
                            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, feludius); // Remove
                            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, ignacious); // Remove
                            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, arion); // Remove
                            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, terrastra); // Remove
                            feludius->SetVisible(false);
                            terrastra->SetVisible(false);
                            arion->SetVisible(false);
                            ignacious->SetVisible(false);
                            if (Creature* monstosity = me->SummonCreature(NPC_ELEMENTIUM_MONSTROSITY, -1008.824f, -582.617f, 831.902f, 0.026f, TEMPSUMMON_MANUAL_DESPAWN))
                                monstosity->CastSpell(monstosity, SPELL_SUMMON_MONSTROSITY_EXPLODE, true);
                            break;
                        case EVENT_CAST_STASIS:
                            CastElementalStasis();
                            break;
                        case EVENT_RESET_CHECK:
                        {
                            bool playerFound = false;
                            std::list<WorldObject*> targetList;
                            Map::PlayerList const& players = me->GetMap()->GetPlayers();
                            if (!players.isEmpty())
                            {
                                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                                {
                                    if (Player* target = itr->getSource())
                                    {
                                        if (target->isAlive() && me->GetDistance2d(target) < 100.0f && !target->isGameMaster())
                                        {
                                            playerFound = true;
                                            break;
                                        }
                                    }
                                }
                            }
                            events.ScheduleEvent(EVENT_RESET_CHECK, 2000);
                            if (!playerFound)
                                EnterEvadeMode();
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
        }
        private:
            InstanceScript* instance;
            SummonList summons;
            EventMap events;
            bool started;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ascendant_council_controllerAI (creature);
    }
};

class boss_feludius : public CreatureScript
{
    public:
        boss_feludius() : CreatureScript("boss_feludius") { }

        struct boss_feludiusAI : public BossAI
        {
            boss_feludiusAI(Creature* creature) : BossAI(creature, DATA_ASCENDANT_COUNCIL) {}

            void Reset()
            {
                me->SetReactState(REACT_AGGRESSIVE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NON_ATTACKABLE);
                me->SetVisible(true);
                wentUp = false;
                _Reset();
            }

            void EnterEvadeMode()
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NON_ATTACKABLE);
                me->RemoveAllAuras();
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me); // Remove
                _EnterEvadeMode();
                BossAI::EnterEvadeMode();
            }

            void EnterCombat(Unit* who)
            {
                // Being first, Feludius/Ignacious set all bosses in combat. This is how it's done on off, by pull, not through controller.
                Creature* controller = me->FindNearestCreature(NPC_ASCENDANT_CONTROLLER, 500.0f, true); // Check controller too.
                Creature* ignacious = me->FindNearestCreature(NPC_IGNACIOUS, 500.0f, true);
                Creature* arion = me->FindNearestCreature(NPC_ARION, 500.0f, true);
                Creature* terrastra = me->FindNearestCreature(NPC_TERRASTRA, 500.0f, true);
                wentUp = false;
                if (controller && ignacious && arion && terrastra) // Check to prevent any damn crashes.
                {
                    if (!controller->isInCombat())
                    {
                        controller->AI()->DoAction(ACTION_RESET_CHECK);
                        controller->AI()->AttackStart(who);
                    }
                    if (!ignacious->isInCombat())
                        ignacious->AI()->AttackStart(who);
                    if (!arion->isInCombat())
                        arion->AI()->AttackStart(who);
                    if (!terrastra->isInCombat())
                        terrastra->AI()->AttackStart(who);
                }

                Talk(SAY_F_AGGRO);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me); // Add
                me->SetReactState(REACT_AGGRESSIVE);
                events.SetPhase(PHASE_COMBAT);
                events.ScheduleEvent(EVENT_HYDRO_LANCE, urand(6000, 10000), 0, PHASE_COMBAT);
                events.ScheduleEvent(EVENT_WATER_BOMB, 15000, 0, PHASE_COMBAT);
                events.ScheduleEvent(EVENT_GLACIATE, 30000, 0, PHASE_COMBAT);
                events.ScheduleEvent(EVENT_HEART_OF_ICE, 18000, 0, PHASE_COMBAT); // Then 22 sec.
                me->CastSpell(me, SPELL_HITTIN_YA, true);
                _EnterCombat();
            }

            void EnterPhaseBalcony()
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                me->InterruptSpell(CURRENT_GENERIC_SPELL, false);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_PASSIVE);
                DoCast(me, SPELL_SELF_ROOT);
                DoCast(me, SPELL_TELEPORT_VISUAL);
                me->NearTeleportTo(-1057.930f, -533.382f, 877.684f, 5.478f);
                events.SetPhase(PHASE_BALCONY);
                if (IsHeroic())
                     events.ScheduleEvent(EVENT_FROST_ORB, 16000, 0, PHASE_BALCONY);
            }

            void JustSummoned(Creature* summon)
            {
                summon->setActive(true);
                if (summon->GetEntry() == NPC_FROZEN_ORB_SUMMONER)
                    summon->AddAura(SPELL_FROZEN_ORB_SUMMON, summon);
                BossAI::JustSummoned(summon);
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_F_KILL);
            }

            void DamageTaken(Unit* /*who*/, uint32& damage)
            {
                if (damage > ((me->GetHealth() / 4) * 3)) // Check needed for correct split.
                    damage = (me->GetHealth() / 4) * 3;

                if (Creature* controller = me->FindNearestCreature(NPC_ASCENDANT_CONTROLLER, 500.0f, true))
                    controller->SetHealth(controller->GetHealth() - damage);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
            }

            void UpdateAI(const uint32 diff)
            {
                if (!events.IsInPhase(PHASE_BALCONY) && !UpdateVictim())
                    return;

                if (Creature* ignacious = me->FindNearestCreature(NPC_IGNACIOUS, 500.0f, true))
                {
                    if ((me->HealthBelowPct(26) || ignacious->HealthBelowPct(26)) && !wentUp)
                    {
                        EnterPhaseBalcony();
                        wentUp = true;
                    }
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_HYDRO_LANCE:
                            if(Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 500.0f, true))
                                DoCast(target, SPELL_HYDRO_LANCE);
                            events.ScheduleEvent(EVENT_HYDRO_LANCE, urand(12000, 14000), 0, PHASE_COMBAT);
                            break;
                        case EVENT_WATER_BOMB:
                        {
                            waterbombs = 0;
                            Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers();
                            if (!PlayerList.isEmpty())
                                for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                                    if (Player* player = i->getSource())
                                    {
                                        Position pos;
                                        player->GetRandomNearPosition(pos, 5.0f);

                                        if (roll_chance_i(50))
                                        {
                                            me->SummonCreature(NPC_WATER_BOMB, pos, TEMPSUMMON_TIMED_DESPAWN, 15000);
                                            waterbombs++;
                                        }

                                        if (roll_chance_i(30)) // 40% chance of a extra random bomb - no one knows if this is correct :P
                                        {
                                            waterbombs++;
                                            Position pos;
                                            player->GetRandomNearPosition(pos, 40.0f);
                                            me->SummonCreature(NPC_WATER_BOMB, pos, TEMPSUMMON_TIMED_DESPAWN, 15000);
                                        }
                                    }

                            if (waterbombs < 10) // we should have min 10 waterbombs
                            {
                                uint8 extraCount = 10 - waterbombs;
                                for (uint8 i = 0; i < extraCount; ++i)
                                {
                                    Position pos;
                                    me->GetRandomNearPosition(pos, 100.0f);
                                    me->SummonCreature(NPC_WATER_BOMB, pos, TEMPSUMMON_TIMED_DESPAWN, 15000);
                                }

                            }
                            DoCast(me, SPELL_WATER_BOMB);
                            events.ScheduleEvent(EVENT_WATER_BOMB, urand(30000, 34000), 0, PHASE_COMBAT);
                            break;
                        }
                        case EVENT_GLACIATE:
                            Talk(SAY_F_GLACIATE);
                            DoCast(me, SPELL_GLACIATE);
                            events.ScheduleEvent(EVENT_GLACIATE, urand(36000, 40000), 0, PHASE_COMBAT);
                            break;
                        case EVENT_HEART_OF_ICE:
                            if(Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 500.0f, true, -SPELL_FLAME_IMBUED))
                                DoCast(target, SPELL_HEART_OF_ICE);
                            events.ScheduleEvent(EVENT_HEART_OF_ICE, urand(22000, 24000), 0, PHASE_COMBAT);
                            break;
                        case EVENT_FROST_ORB:
                            DoCastRandom(SPELL_FROZEN_ORB_PRE_SUMMON, 0.0f, false, -SPELL_HITTIN_YA_PLAYER);
                            events.ScheduleEvent(EVENT_FROST_ORB, urand(19000, 21000), 0, PHASE_BALCONY);
                            break;
                        default:
                            break;
                    }
                }
                if (events.IsInPhase(PHASE_COMBAT) && !me->HealthBelowPct(26))
                    DoMeleeAttackIfReady();
            }

    private:
        uint8 waterbombs;
        bool wentUp;
        Unit* wbTarget;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_feludiusAI(creature);
    }
};

class boss_ignacious : public CreatureScript
{
    public:
        boss_ignacious() : CreatureScript("boss_ignacious") { }

        struct boss_ignaciousAI : public BossAI
        {
            boss_ignaciousAI(Creature* creature) : BossAI(creature, DATA_ASCENDANT_COUNCIL) {}

            void Reset()
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NON_ATTACKABLE);
                me->SetVisible(true);
                wentUp = false;
                lastTarget = NULL;
                maxInfernos = 0;
                _Reset();
            }

            void EnterEvadeMode()
            {
                me->RemoveAllAuras();
                me->SetReactState(REACT_AGGRESSIVE);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me); // Remove
                _EnterEvadeMode();
                BossAI::EnterEvadeMode();
            }

            void EnterCombat(Unit* who)
            {
                // Being first, Feludius/Ignacious set all bosses in combat. This is how it's done on off, by pull, not through controller.
                Creature* feludius = me->FindNearestCreature(NPC_FELUDIUS, 500.0f, true);
                Creature* arion = me->FindNearestCreature(NPC_ARION, 500.0f, true);
                Creature* terrastra = me->FindNearestCreature(NPC_TERRASTRA, 500.0f, true);

                wentUp = false;

                if (feludius && arion && terrastra) // Check to prevent any damn crashes.
                {
                    if (!feludius->isInCombat())
                        feludius->AI()->AttackStart(who);
                    if (!arion->isInCombat())
                        arion->AI()->AttackStart(who);
                    if (!terrastra->isInCombat())
                        terrastra->AI()->AttackStart(who);
                }

                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me); // Add
                me->SetReactState(REACT_AGGRESSIVE);
                events.SetPhase(PHASE_COMBAT);
                events.ScheduleEvent(EVENT_SAY_I_AGGRO, 4500, 0, PHASE_COMBAT); // Delay this so does not come in conflict with Feludius aggro say.
                events.ScheduleEvent(EVENT_FLAME_TORRENT, urand(6000, 10000), 0, PHASE_COMBAT);
                events.ScheduleEvent(EVENT_INFERNO_RUSH, urand(14000, 16000), 0, PHASE_COMBAT);
                events.ScheduleEvent(EVENT_AEGIS_OF_FLAME, 31000, 0, PHASE_COMBAT); // Then 60;
                events.ScheduleEvent(EVENT_BURNING_BLOOD, 28000, 0, PHASE_COMBAT); // Then 22 sec.
                me->CastSpell(me, SPELL_HITTIN_YA, true);
                _EnterCombat();
            }

            void EnterPhaseBalcony()
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                me->InterruptSpell(CURRENT_GENERIC_SPELL, false);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_PASSIVE);
                DoCast(me, SPELL_SELF_ROOT);
                DoCast(me, SPELL_TELEPORT_VISUAL);
                me->NearTeleportTo(-1057.890f, -631.515f, 877.682f, 0.797f);
                events.SetPhase(PHASE_BALCONY);
                if (IsHeroic())
                    events.ScheduleEvent(EVENT_FLAME_STRIKE, 20000, 0, PHASE_BALCONY);
            }

            void JustSummoned(Creature* summon)
            {
                summon->setActive(true);
                if (summon->GetEntry() == NPC_INFERNO_RUSH)
                {
                    summon->SetReactState(REACT_PASSIVE);
                    summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                    summon->AddAura(SPELL_INFERNO_RUSH_AURA, summon);
                }
                BossAI::JustSummoned(summon);
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_I_KILL);
            }

            void DamageTaken(Unit* /*who*/, uint32& damage)
            {
                if (damage > ((me->GetHealth() / 4) * 3)) // Check needed for correct split.
                    damage = (me->GetHealth() / 4) * 3;

                if (Creature* controller = me->FindNearestCreature(NPC_ASCENDANT_CONTROLLER, 500.0f, true))
                    controller->SetHealth(controller->GetHealth() - damage);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_COUNCIL_PHASE_THREE)
                    summons.DespawnEntry(NPC_FLAME_STRIKE);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!events.IsInPhase(PHASE_BALCONY) && !UpdateVictim())
                    return;

                if (Creature* feludius = me->FindNearestCreature(NPC_FELUDIUS, 500.0f, true))
                {
                    if ((me->HealthBelowPct(26) || feludius->HealthBelowPct(26)) && !wentUp)
                    {
                        EnterPhaseBalcony();
                        wentUp = true;
                    }
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SAY_I_AGGRO:
                            Talk(SAY_I_AGGRO);
                            break;
                        case EVENT_FLAME_TORRENT:
                            DoCast(me, SPELL_FLAME_TORRENT);
                            events.ScheduleEvent(EVENT_FLAME_TORRENT, urand(12000, 14000), 0, PHASE_COMBAT);
                            break;
                        case EVENT_INFERNO_RUSH:
                            lastTarget = me->getVictim();
                            if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST))
                            {
                                float dist = me->GetDistance2d(target->GetPositionX(), target->GetPositionY());
                                float speedXY = 40.0f;
                                float speed = (dist / speedXY) * 1100;
                                target->GetPosition(&_posMissile);
                                events.DelayEvents(speed + 10);
                                events.ScheduleEvent(EVENT_INFERNO_RUSH_MISSILE, speed, 0, PHASE_COMBAT);
                                maxInfernos = floor(me->GetDistance2d(target) / 6.0f);
                                if (maxInfernos < 5)
                                    maxInfernos = 5;
                                me->GetMotionMaster()->MoveJump(target->GetPositionX(),target->GetPositionY(), target->GetPositionZ(), 40.0f, 40.0f);
                            }
                            events.ScheduleEvent(EVENT_INFERNO_RUSH, urand(30000, 34000), 0, PHASE_COMBAT);
                            break;
                        case EVENT_INFERNO_RUSH_MISSILE:
                        {
                            me->CastSpell(_posMissile.m_positionX, _posMissile.m_positionY, _posMissile.m_positionZ, SPELL_INFERNO_RUSH_JUMP, true);
                            me->CastSpell(_posMissile.m_positionX, _posMissile.m_positionY, _posMissile.m_positionZ, SPELL_INFERNO_LEAP_MISSILE, true);
                            if (lastTarget && me->IsInMap(lastTarget))
                            {
                                chargeTimer = (me->GetDistance2d(lastTarget) / 45.0) * 1000;
                                DoCast(lastTarget, SPELL_INFERNO_RUSH_CHARGE, true);
                            } else chargeTimer = 1000;
                            events.DelayEvents(chargeTimer+10);
                            events.ScheduleEvent(EVENT_SUMMON_INFERNO_RUSH_NPC, chargeTimer, 0, PHASE_ALL);
                            break;
                        }
                        case EVENT_SUMMON_INFERNO_RUSH_NPC:
                            for (uint8 i = 0; i < maxInfernos; ++i)
                            {
                                float dist = i * 6.0f;
                                Position pos;
                                me->GetNearPosition(pos, dist, M_PI);
                                me->SummonCreature(NPC_INFERNO_RUSH, pos, TEMPSUMMON_TIMED_DESPAWN, 30000);
                            }
                            break;
                        case EVENT_AEGIS_OF_FLAME:
                            DoCast(me, SPELL_AEGIS_OF_FLAME);
                            events.DelayEvents(2500); // make sure that flame torrent isn't casted before rising flames
                            events.ScheduleEvent(EVENT_RISING_FLAMES, 2000, 0, PHASE_COMBAT);
                            events.ScheduleEvent(EVENT_AEGIS_OF_FLAME, urand(58000, 62000), 0, PHASE_COMBAT);
                            break;
                        case EVENT_RISING_FLAMES:
                            Talk(SAY_I_RISING_FLAMES);
                            DoCast(me, SPELL_RISING_FLAMES);
                            break;
                        case EVENT_BURNING_BLOOD:
                            if (Unit* victim = SelectTarget(SELECT_TARGET_RANDOM, 0, 500.0f, true, -SPELL_HEART_OF_ICE))
                                DoCast(victim, SPELL_BURNING_BLOOD);
                            events.ScheduleEvent(EVENT_BURNING_BLOOD, urand(22000, 24000), 0, PHASE_COMBAT);
                            break;
                        case EVENT_FLAME_STRIKE:
                            if (Unit* victim = SelectTarget(SELECT_TARGET_RANDOM, 0, 500.0f, true, -SPELL_HITTIN_YA_PLAYER))
                            {
                                if (Creature *c = me->SummonCreature(NPC_FLAME_STRIKE, victim->GetPositionX(), victim->GetPositionY(), victim->GetPositionZ(), 0))
                                {
                                    me->AddAura(SPELL_FLAME_STRIKE_MARKER, c);
                                    me->CastSpell(c, SPELL_FLAME_STRIKE_TARGET, false);
                                }
                            }
                            events.ScheduleEvent(EVENT_FLAME_STRIKE, urand(19000, 21000), 0, PHASE_BALCONY);
                            break;
                        default:
                            break;
                    }
                }
                if (events.IsInPhase(PHASE_COMBAT) && !me->HealthBelowPct(26))
                    DoMeleeAttackIfReady();
            }
    private:
        uint8 maxInfernos;
        uint32 chargeTimer;
        Unit* lastTarget;
        bool wentUp;
        Position _posMissile;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_ignaciousAI(creature);
    }
};

class boss_arion : public CreatureScript
{
    public:
        boss_arion() : CreatureScript("boss_arion") { }

        struct boss_arionAI : public BossAI
        {
            boss_arionAI(Creature* creature) : BossAI(creature, DATA_ASCENDANT_COUNCIL) {}

            void Reset()
            {
                me->SetReactState(REACT_AGGRESSIVE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NON_ATTACKABLE);
                me->SetVisible(true);
                wentDown = false;
                _Reset();
            }

            void EnterEvadeMode()
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                _EnterEvadeMode();
                BossAI::EnterEvadeMode();
            }

            void EnterCombat(Unit* /*who*/)
            {
                wentDown = false;
                DoCast(me, SPELL_SELF_ROOT);
                events.SetPhase(PHASE_BALCONY);
                if (IsHeroic())
                    events.ScheduleEvent(EVENT_STATIC_OVERLOAD, 20000, 0, PHASE_BALCONY);
                me->CastSpell(me, SPELL_HITTIN_YA, true);
                _EnterCombat();
            }

            void EnterPhaseCombat()
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                me->RemoveAllAuras();
                me->CastSpell(me, SPELL_HITTIN_YA, true);
                DoCast(me, SPELL_TELEPORT_VISUAL);
                me->NearTeleportTo(-1051.869f, -599.499f, 835.193f, 0.496f);
                me->SetReactState(REACT_AGGRESSIVE);
                events.SetPhase(PHASE_COMBAT);
                events.ScheduleEvent(EVENT_SAY_A_AGGRO, 4500, 0, PHASE_COMBAT); // Delay this so does not come in conflict with Terrastra aggro say.
                events.ScheduleEvent(EVENT_LIGHTNING_ROD, urand(6000, 10000), 0, PHASE_COMBAT); // 15 sec.
                events.ScheduleEvent(EVENT_DISPERSE, urand(14000, 16000), 0, PHASE_COMBAT); // 30 sec repeat.
                events.ScheduleEvent(EVENT_CALL_WINDS, 10000, 0, PHASE_COMBAT);
                events.ScheduleEvent(EVENT_THUNDERSHOCK, 70000, 0, PHASE_COMBAT); // Then 33 sec.
            }

            void JustSummoned(Creature* summon)
            {
                summon->setActive(true);
                BossAI::JustSummoned(summon);
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_A_KILL);
            }

            void DamageTaken(Unit* /*who*/, uint32& damage)
            {
                if (damage > ((me->GetHealth() / 4) * 3)) // Check needed for correct split.
                    damage = (me->GetHealth() / 4) * 3;

                if (Creature* controller = me->FindNearestCreature(NPC_ASCENDANT_CONTROLLER, 500.0f, true))
                    controller->SetHealth(controller->GetHealth() - damage);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_COUNCIL_PHASE_THREE)
                    summons.DespawnEntry(NPC_CALL_WINDS);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!events.IsInPhase(PHASE_BALCONY) && !UpdateVictim())
                    return;

                if (Creature* feludius = me->FindNearestCreature(NPC_FELUDIUS, 500.0f, true))
                {
                    if (Creature* ignacious = me->FindNearestCreature(NPC_IGNACIOUS, 500.0f, true))
                    {
                        if ((feludius->HealthBelowPct(26) || ignacious->HealthBelowPct(26)) && !wentDown)
                        {
                            EnterPhaseCombat();
                            wentDown = true;
                        }
                    }
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SAY_A_AGGRO:
                            Talk(SAY_A_AGGRO);
                            break;
                        case EVENT_LIGHTNING_ROD:
                        {
                            int preventInfiniteLoop = 0;
                            for (int cnt = 0; preventInfiniteLoop < RAID_MODE(10, 25, 10, 25) && cnt < RAID_MODE(1, 3, 1, 3); preventInfiniteLoop++)
                            {
                                lightningVictim[cnt] = NULL;
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 500.0f, true, -SPELL_LIGHTNING_ROD))
                                    if (!target->HasAura(SPELL_HITTIN_YA_PLAYER))
                                    {
                                        me->AddAura(SPELL_LIGHTNING_ROD, target);
                                        lightningVictim[cnt] = target;
                                        cnt++;
                                    }
                            }
                            events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, urand(5000, 14000), 0, PHASE_COMBAT);
                            events.ScheduleEvent(EVENT_LIGHTNING_ROD, urand(24000, 28000), 0, PHASE_COMBAT);
                            break;
                        }
                        case EVENT_CHAIN_LIGHTNING:
                            DoCast(SPELL_CHAIN_LIGHTNING);
                            break;
                        case EVENT_DISPERSE:
                            DoCast(me, SPELL_DISPERSE);
                            events.ScheduleEvent(EVENT_LIGHTNING_BLAST, 1510, 0, PHASE_COMBAT);
                            events.ScheduleEvent(EVENT_DISPERSE, urand(29000, 31000), 0, PHASE_COMBAT);
                            break;
                        case EVENT_LIGHTNING_BLAST:
                            me->StopMoving();
                            if (me->getVictim())
                                me->SetFacingToObject(me->getVictim());
                            DoCastVictim(SPELL_LIGHTNING_BLAST);
                            break;
                        case EVENT_THUNDERSHOCK:
                            me->MonsterTextEmote(EMOTE_ARION_THUNDERSHOCK, 0, false);
                            DoCast(me, SPELL_THUNDERSHOCK);
                            events.ScheduleEvent(EVENT_THUNDERSHOCK, 70000, 0, PHASE_COMBAT);
                            break;
                        case EVENT_CALL_WINDS:
                            Talk(SAY_A_CALL_WINDS);
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 500.0f, true))
                                DoCast(target, SPELL_CALL_WINDS);
                            events.ScheduleEvent(EVENT_CALL_WINDS, urand(27000, 33000), 0, PHASE_COMBAT);
                            break;
                        case EVENT_STATIC_OVERLOAD:
                            DoCast(SPELL_STATIC_OVERLOAD);
                            events.ScheduleEvent(EVENT_STATIC_OVERLOAD, urand(19000, 21000), 0, PHASE_BALCONY);
                            break;
                        default:
                            break;
                    }
                }

                if (events.IsInPhase(PHASE_COMBAT) && !me->HealthBelowPct(26))
                    DoMeleeAttackIfReady();
            }
    private:
        Unit* lightningVictim[3];
        bool wentDown;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_arionAI(creature);
    }
};

class boss_terrastra : public CreatureScript
{
    public:
        boss_terrastra() : CreatureScript("boss_terrastra") { }

        struct boss_terrastraAI : public BossAI
        {
            boss_terrastraAI(Creature* creature) : BossAI(creature, DATA_ASCENDANT_COUNCIL) {}

            void Reset()
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_AGGRESSIVE);
                me->SetVisible(true);
                wentDown = false;
                _Reset();
            }

            void EnterEvadeMode()
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                _EnterEvadeMode();
                BossAI::EnterEvadeMode();
            }

            void EnterCombat(Unit* /*who*/)
            {
                wentDown = false;
                DoCast(me, SPELL_SELF_ROOT);
                events.SetPhase(PHASE_BALCONY);
                if (IsHeroic())
                    events.ScheduleEvent(EVENT_GRAVITY_CORE, 23000, 0, PHASE_BALCONY);
                me->CastSpell(me, SPELL_HITTIN_YA, true);
                _EnterCombat();
            }

            void EnterPhaseCombat()
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                me->RemoveAllAuras();
                me->CastSpell(me, SPELL_HITTIN_YA, true);
                DoCast(me, SPELL_TELEPORT_VISUAL);
                me->NearTeleportTo(-1050.369f, -565.689f, 835.221f, 5.946f);
                me->SetReactState(REACT_AGGRESSIVE);
                events.SetPhase(PHASE_COMBAT);
                events.ScheduleEvent(EVENT_ERUPTION, urand(6000, 10000), 0, PHASE_COMBAT); // 15 sec.
                events.ScheduleEvent(EVENT_GRAVITY_WELL, 10000, 0, PHASE_COMBAT); //  every 27-33 sec.
                events.ScheduleEvent(EVENT_QUAKE, 35000, 0, PHASE_COMBAT); // 70 sec repeat.
                events.ScheduleEvent(EVENT_HARDEN_SKIN, urand(31000, 33000), 0, PHASE_COMBAT); // 42 sec repeat
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_STATIC_OVERLOAD);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GRAVITY_CORE);
            }

            void JustSummoned(Creature* summon)
            {
                summon->setActive(true);
                if (summon->GetEntry() == NPC_ERUPTION_TGT)
                {
                    summon->CastSpell(summon, SPELL_ERUPTION, false);
                    summon->AddAura(SPELL_ERUPTION_VISUAL, summon);
                    summon->DespawnOrUnsummon(5000);
                }
                BossAI::JustSummoned(summon);
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_T_KILL);
            }

            void DamageTaken(Unit* /*who*/, uint32& damage)
            {
                if (damage > ((me->GetHealth() / 4) * 3)) // Check needed for correct split.
                    damage = (me->GetHealth() / 4) * 3;

                if (Creature* controller = me->FindNearestCreature(NPC_ASCENDANT_CONTROLLER, 500.0f, true))
                    controller->SetHealth(controller->GetHealth() - damage);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_COUNCIL_PHASE_THREE)
                    summons.DespawnEntry(NPC_GRAVITY_WELL);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!events.IsInPhase(PHASE_BALCONY) && !UpdateVictim())
                    return;

                if (Creature* feludius = me->FindNearestCreature(NPC_FELUDIUS, 500.0f, true)) // Check to prevent any damn crashes.
                {
                    if (Creature* ignacious = me->FindNearestCreature(NPC_IGNACIOUS, 500.0f, true))
                    {
                        if ((feludius->HealthBelowPct(26) || ignacious->HealthBelowPct(26)) && !wentDown) // Check for Combat phase.
                        {
                            EnterPhaseCombat();
                            wentDown = true;
                        }
                    }
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_QUAKE:
                            Talk(SAY_T_QUAKE);
                            me->MonsterTextEmote(EMOTE_TERRASTRA_QUAKE, 0, false);
                            DoCast(me, SPELL_QUAKE);
                            events.ScheduleEvent(EVENT_QUAKE, 70000, 0, PHASE_COMBAT);
                            break;
                        case EVENT_ERUPTION:
                            if (Unit* victim = SelectTarget(SELECT_TARGET_RANDOM, 0, 500.0f, true))
                                DoCast(victim, SPELL_ERUPTION_SUMMON_MOB);
                            events.ScheduleEvent(EVENT_ERUPTION, urand(14000, 16000), 0, PHASE_COMBAT);
                            break;
                        case EVENT_HARDEN_SKIN:
                            DoCast(me, SPELL_HARDEN_SKIN);
                            events.ScheduleEvent(EVENT_HARDEN_SKIN, urand(41000, 43000), 0, PHASE_COMBAT);
                            break;
                        case EVENT_GRAVITY_WELL:
                            Talk(SAY_T_GRAVITY_WELL);
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 500.0f, true))
                                me->SummonCreature(NPC_GRAVITY_WELL, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 30000);
                            events.ScheduleEvent(EVENT_GRAVITY_WELL, urand(27000, 33000), 0, PHASE_COMBAT);
                            break;
                        case EVENT_GRAVITY_CORE:
                            DoCast(SPELL_GRAVITY_CORE);
                            events.ScheduleEvent(EVENT_GRAVITY_CORE, 20000, 0, PHASE_BALCONY);
                            break;
                        default:
                            break;
                    }
                }
                if (events.IsInPhase(PHASE_COMBAT) && !me->HealthBelowPct(26))
                    DoMeleeAttackIfReady();
            }
    private:
        bool wentDown;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_terrastraAI(creature);
    }
};

class boss_monstrosity : public CreatureScript
{
    public:
        boss_monstrosity() : CreatureScript("boss_monstrosity") { }

        struct boss_monstrosityAI : public BossAI
        {
            boss_monstrosityAI(Creature* creature) : BossAI(creature, DATA_ASCENDANT_COUNCIL)
            {
                liquidIce = 0;
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_DESPAWN)
                {
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                    summons.DespawnAll();
                    me->DespawnOrUnsummon(1000);
                }
            }

            void JustSummoned(Creature* summon)
            {
                if (summon->GetEntry() == NPC_GRAVITY_CRUSH)
                {
                    summon->SetDisableGravity(true);
                    summon->GetMotionMaster()->MovePoint(0, summon->GetPositionX(), summon->GetPositionY(), summon->GetPositionZ() + 30.0f, false);
                }
                if (summon->GetEntry() == NPC_LIQUID_ICE)
                {
                    summon->SetInCombatWithZone();
                    ++liquidIce;
                    return;
                }
                summon->setActive(true);
                BossAI::JustSummoned(summon);
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(RAND(SAY_M_KILL, SAY_M_KILL_2));
            }

            void JustDied(Unit* killer)
            {
                me->SetLootRecipient(killer);
                me->RemoveAurasDueToSpell(SPELL_ELECTRIC_INSTABILITY_AURA);
                Talk(SAY_M_DIE);
                summons.DespawnAll();
                if (liquidIce < 2)
                    instance->DoCompleteAchievement(5311); // Elementary

                Creature* controller = me->FindNearestCreature(NPC_ASCENDANT_CONTROLLER, 500.0f, true);
                Creature* feludius = me->FindNearestCreature(NPC_FELUDIUS, 500.0f, true);
                Creature* ignacious = me->FindNearestCreature(NPC_IGNACIOUS, 500.0f, true);
                Creature* arion = me->FindNearestCreature(NPC_ARION, 500.0f, true);
                Creature* terrastra = me->FindNearestCreature(NPC_TERRASTRA, 500.0f, true);

                if (controller && feludius && ignacious && arion && terrastra) // Check to prevent any damn crashes.
                {
                    feludius->Kill(feludius); // To count as killed on addons. No Killed talk since AI is disabled :D.
                    terrastra->Kill(terrastra);
                    arion->Kill(arion);
                    ignacious->Kill(ignacious);
                    controller->DisappearAndDie();
                }
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me); // Remove
                _JustDied();
            }

            void EnterCombat(Unit* /*who*/)
            {
                me->LowerPlayerDamageReq(me->GetMaxHealth());
                Talk(SAY_M_AGGRO);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me); // Add
                count = 0;
                events.ScheduleEvent(EVENT_LAVA_SEED, 13000); // every 23 sec.
                events.ScheduleEvent(EVENT_GRAVITY_CRUSH, urand(25000, 27000)); // every 24 sec.
                DoCast(me, SPELL_ELECTRIC_INSTABILITY_AURA, true);
                me->CastSpell(me, SPELL_HITTIN_YA, true);
                _EnterCombat();
            }

            void DamageTaken(Unit* /*who*/, uint32& damage)
            {
                if (Creature* controller = me->FindNearestCreature(NPC_ASCENDANT_CONTROLLER, 500.0f, true))
                    controller->SetHealth(controller->GetHealth() - damage);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_LAVA_SEED:
                            DoCast(me, SPELL_LAVA_SEED);
                            events.ScheduleEvent(EVENT_LAVA_SEED, urand(22000, 23000));
                            break;
                        case EVENT_GRAVITY_CRUSH:
                            Talk(SAY_M_GRAVITY_CRUSH);
                            instance->SetData(DATA_GRAVITY_CRUSH, NOT_STARTED);
                            me->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), false); // HACK (players see nothing from this, but this avoid the channel interrupt)
                            DoCast(me, SPELL_GRAVITY_CRUSH, true);
                            events.ScheduleEvent(EVENT_GRAVITY_CRUSH, urand(24000, 25000));
                            break;
                        default:
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
    private:
        uint8 count, mode, liquidIce;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_monstrosityAI(creature);
    }
};

class npc_feludius_waterbomb : public CreatureScript // 44201
{
    public:
        npc_feludius_waterbomb() : CreatureScript("npc_feludius_waterbomb") { }

        struct npc_feludius_waterbombAI : public ScriptedAI
        {
            npc_feludius_waterbombAI(Creature* creature) : ScriptedAI(creature) { }

            void SpellHit(Unit* /*attacker*/, SpellInfo const* spell) override
            {
                if (spell->Id == SPELL_WATER_BOMB)
                {
                    DoCast(me, SPELL_WATER_BOMB_DMG);
                    me->DespawnOrUnsummon(1000);
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_feludius_waterbombAI(creature);
        }
};

class npc_arion_callwinds : public CreatureScript // 44747
{
    public:
        npc_arion_callwinds() : CreatureScript("npc_arion_callwinds") {}

        struct npc_arion_callwindsAI : public ScriptedAI
        {
            npc_arion_callwindsAI(Creature* creature) : ScriptedAI(creature)
            {
                creature->SetReactState(REACT_PASSIVE);
                creature->CastSpell(creature, SPELL_LASHING_WINDS_VISUAL, true);
                creature->GetMotionMaster()->MoveRandom(100.0f);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_arion_callwindsAI(creature);
        }
};

class npc_terrastra_gravitywell : public CreatureScript // 44824
{
    public:
        npc_terrastra_gravitywell() : CreatureScript("npc_terrastra_gravitywell") {}

        struct npc_terrastra_gravitywellAI : public ScriptedAI
        {
            npc_terrastra_gravitywellAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                creature->SetReactState(REACT_PASSIVE);
                creature->CastSpell(creature, SPELL_GRAV_WELL_MOB_VISUAL, true);
                creature->CastSpell(creature, AURA_GRAVITY_WELL_PULL, true); // Pull players.
                creature->CastSpell(creature, AURA_GRAVITY_WELL_DMG_GROUNDED, true); // Deal damage.
            }
        private:
            InstanceScript* instance;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_terrastra_gravitywellAI(creature);
        }
};

class npc_monstrosity_liquid_ice : public CreatureScript // 45452
{
    public:
        npc_monstrosity_liquid_ice() : CreatureScript("npc_monstrosity_liquid_ice") {}

        struct npc_monstrosity_liquid_iceAI : public ScriptedAI
        {
            npc_monstrosity_liquid_iceAI(Creature* creature) : ScriptedAI(creature)
            {
                creature->SetReactState(REACT_PASSIVE);
                creature->CastSpell(creature, SPELL_LIQUID_ICE_PERIODIC, true);
            }

            void EnterEvadeMode() {}
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_monstrosity_liquid_iceAI(creature);
        }
};

class npc_monstrosity_lava_seed : public CreatureScript // 48538
{
    public:
        npc_monstrosity_lava_seed() : CreatureScript("npc_monstrosity_lava_seed") {}

        struct npc_monstrosity_lava_seedAI : public ScriptedAI
        {
            npc_monstrosity_lava_seedAI(Creature* creature) : ScriptedAI(creature)
            {
                creature->SetReactState(REACT_PASSIVE);
            }

            void SpellHit(Unit* /*attacker*/, SpellInfo const* spell)
            {
                if (spell->Id == SPELL_LAVA_SEED)
                {
                    me->CastSpell(me, AURA_LAVA_SEED_VISUAL, true);
                    events.ScheduleEvent(EVENT_SEED_EXPLODE, IsHeroic() ? 2000 : 3000);
                }
            }

            void UpdateAI(const uint32 diff)
            {
                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SEED_EXPLODE:
                            DoCast(me, SPELL_LAVA_EXPLODE);
                            me->RemoveAurasDueToSpell(AURA_LAVA_SEED_VISUAL);
                            break;
                    }
                }
            }
        private:
            EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_monstrosity_lava_seedAI(creature);
        }
};

// HEROIC NPC-s.

class npc_ignacious_flame_strike : public CreatureScript // 49432
{
    public:
        npc_ignacious_flame_strike() : CreatureScript("npc_ignacious_flame_strike") {}

        struct npc_ignacious_flame_strikeAI : public ScriptedAI
        {
            npc_ignacious_flame_strikeAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
                events.ScheduleEvent(EVENT_FLAME_STRIKE_DAMAGE, 5000);
            }

            void UpdateAI(const uint32 diff)
            {
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FLAME_STRIKE_DAMAGE:
                            DoCast(me, SPELL_FLAME_STRIKE_DAMAGE);
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_ignacious_flame_strikeAI(creature);
        }
};

class npc_feludius_frozen_orb : public CreatureScript // 49518
{
    public:
        npc_feludius_frozen_orb() : CreatureScript("npc_feludius_frozen_orb") {}

        struct npc_feludius_frozen_orbAI : public ScriptedAI
        {
            npc_feludius_frozen_orbAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            void Reset()
            {
                vicGUID = 0;
                me->CastSpell(me, SPELL_FROZEN_ORB_PERIODIC, true);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                events.ScheduleEvent(EVENT_ATTACK_FROZEN_TARGET, 100);
            }

            void DoAction(const int32 action)
            {
                if (action == ACTION_DESPAWN)
                {
                    events.Reset();
                    if (Unit *victim = Unit::GetUnit(*me, vicGUID))
                        victim->RemoveAurasDueToSpell(SPELL_FROST_BEACON);
                    me->DespawnOrUnsummon(100);
                }
            }

            void UpdateAI(const uint32 diff)
            {
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ATTACK_FROZEN_TARGET:
                        {
                            me->SetInCombatWithZone();
                            if(Unit* victim = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true, -SPELL_HITTIN_YA_PLAYER))
                            {
                                vicGUID = victim->GetGUID();
                                me->AddAura(SPELL_FROST_BEACON, victim);
                                me->AI()->AttackStart(victim);
                                me->AddThreat(victim,  100000.0f);
                            }
                            events.ScheduleEvent(EVENT_CHECK_FROZEN_TARGET, 1000);
                            break;
                        }
                        case EVENT_CHECK_FROZEN_TARGET:
                        {
                            Unit *vic = Unit::GetUnit(*me, vicGUID);
                            if (!vic || !vic->isAlive() || !vic->HasAura(SPELL_FROST_BEACON))
                                events.ScheduleEvent(EVENT_ATTACK_FROZEN_TARGET, 100);
                            else
                                events.ScheduleEvent(EVENT_CHECK_FROZEN_TARGET, 1000);
                            break;
                        }
                    }
                }
            }
        private:
            uint64 vicGUID;
            InstanceScript* instance;
            EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_feludius_frozen_orbAI(creature);
        }
};

class spell_feludius_waterbomb : public SpellScriptLoader // 82699.
{
public:
    spell_feludius_waterbomb() : SpellScriptLoader("spell_feludius_waterbomb") {}

    class spell_feludius_waterbomb_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_feludius_waterbomb_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end();)
            {
                if ((*itr)->GetTypeId() == TYPEID_UNIT && (*itr)->ToCreature()->GetEntry() == NPC_WATER_BOMB)
                    ++itr;
                else
                    targets.erase(itr++);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_feludius_waterbomb_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_feludius_waterbomb_SpellScript();
    }
};

class WaterloggedCheck
{
public:
    bool operator()(WorldObject* target) const
    {
        if (Unit* unit = target->ToUnit())
            return !unit->HasAura(SPELL_WATERLOGGED);
        return false;
    }
};

class spell_feludius_glaciate: public SpellScriptLoader // 82746
{
public:
    spell_feludius_glaciate () : SpellScriptLoader("spell_feludius_glaciate") { }

    class spell_feludius_glaciateSpellScript: public SpellScript
    {
        PrepareSpellScript(spell_feludius_glaciateSpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(WaterloggedCheck());
        }

        void CalculateDamage(SpellEffIndex /*effIndex*/)
        {
            if (!GetHitUnit())
                return;

            float distance = GetCaster()->GetDistance2d(GetHitUnit());

            if (distance > 1.0f)
                SetHitDamage(int32(GetHitDamage() - ((GetCaster()->GetInstanceScript()->instance->IsHeroic() ? 25000 : 20000) * distance))); // Drops to 10k after 50 yards.

            if (GetHitDamage() < 10000) // Don't let it go lower then 10k. This is min you can get.
                SetHitDamage(10000);
        }

        void EffectScriptEffect(SpellEffIndex /*effIndex*/)
        {
            GetHitUnit()->RemoveAurasDueToSpell(SPELL_WATERLOGGED);
            GetCaster()->CastSpell(GetHitUnit(), SPELL_FROZEN, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_feludius_glaciateSpellScript::CalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_feludius_glaciateSpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(spell_feludius_glaciateSpellScript::EffectScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript () const
    {
        return new spell_feludius_glaciateSpellScript();
    }
};


class DisperseFilter
{
public:
    bool operator()(WorldObject* target) const
    {
        if (target->GetTypeId() == TYPEID_UNIT)
            if (target->GetEntry() == 44553)
                return false;
        return true;
    }
};

class spell_arion_disperse: public SpellScriptLoader // 83087
{
public:
    spell_arion_disperse () : SpellScriptLoader("spell_arion_disperse") { }

    class spell_arion_disperseSpellScript: public SpellScript
    {
        PrepareSpellScript(spell_arion_disperseSpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(DisperseFilter());
            Trinity::Containers::RandomResizeList(targets, 1);
        }

        void EffectScriptEffect(SpellEffIndex /*effIndex*/)
        {
            GetCaster()->CastSpell(GetHitUnit(), SPELL_DISPERSE_TELEPORT, true);
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_arion_disperseSpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnEffectHitTarget += SpellEffectFn(spell_arion_disperseSpellScript::EffectScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript () const override
    {
        return new spell_arion_disperseSpellScript();
    }
};

class spell_arion_thundershock: public SpellScriptLoader // 83067
{
public:
    spell_arion_thundershock () : SpellScriptLoader("spell_arion_thundershock") { }

    class spell_arion_thundershockSpellScript: public SpellScript
    {
        PrepareSpellScript(spell_arion_thundershockSpellScript);

        void CalculateDamage(SpellEffIndex /*effIndex*/)
        {
            if (!GetHitUnit())
                return;

            float distance = GetCaster()->GetDistance2d(GetHitUnit());

            if (distance > 1.0f)
                SetHitDamage(int32(GetHitDamage() - ((GetCaster()->GetInstanceScript()->instance->Is25ManRaid() ? 3000 : 1500) * distance))); // Depends on distance from caster.

            if (GetHitUnit()->HasAura(SPELL_GROUNDED))
                SetHitDamage(int32(GetHitDamage() / 3));

            if (GetHitDamage() < 5000) // Don't let it go lower then 5k. This is min you can get.
                SetHitDamage(5000);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_arion_thundershockSpellScript::CalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript () const
    {
        return new spell_arion_thundershockSpellScript();
    }
};

class spell_terrastra_quake: public SpellScriptLoader // 83565
{
public:
    spell_terrastra_quake () : SpellScriptLoader("spell_terrastra_quake") { }

    class spell_terrastra_quakeSpellScript: public SpellScript
    {
        PrepareSpellScript(spell_terrastra_quakeSpellScript);

        void CalculateDamage(SpellEffIndex /*effIndex*/)
        {
            if (!GetHitUnit())
                return;

            if (GetHitUnit()->HasAura(SPELL_SWIRLING_WINDS))
                SetHitDamage(0);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_terrastra_quakeSpellScript::CalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript () const
    {
        return new spell_terrastra_quakeSpellScript();
    }
};

class spell_ignacious_inferno_rush : public SpellScriptLoader // 82860.
{
public:
    spell_ignacious_inferno_rush() : SpellScriptLoader("spell_ignacious_inferno_rush") { }

    class spell_ignacious_inferno_rush_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ignacious_inferno_rush_SpellScript);

        void AddSpecialEffect(SpellEffIndex /*effIndex*/)
        {
            if (Unit* target = GetHitUnit())
            {
                if (target->HasAura(SPELL_WATERLOGGED))
                    target->RemoveAurasDueToSpell(SPELL_WATERLOGGED);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_ignacious_inferno_rush_SpellScript::AddSpecialEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_ignacious_inferno_rush_SpellScript();
    }
};

class spell_feludius_heart_of_ice : public SpellScriptLoader
{
public:
    spell_feludius_heart_of_ice() : SpellScriptLoader("spell_feludius_heart_of_ice") { }

    class spell_feludius_heart_of_ice_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_feludius_heart_of_ice_AuraScript);

        void onPeriodicTick(AuraEffect const* aurEff)
        {
            if (AuraEffect* effect = GetAura()->GetEffect(EFFECT_0))
                effect->SetAmount(2000 * aurEff->GetTickNumber());

            if (Unit* caster = GetTarget())
            {
                if (aurEff->GetId() == SPELL_HEART_OF_ICE)
                    caster->CastSpell(caster, SPELL_FROST_IMBUED, true);
                else if (aurEff->GetId() == SPELL_BURNING_BLOOD)
                    caster->CastSpell(caster, SPELL_FLAME_IMBUED, true);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_feludius_heart_of_ice_AuraScript::onPeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_feludius_heart_of_ice_AuraScript();
    }
};

class TankCheck : public std::unary_function<Unit*, bool>
{
public:
    explicit TankCheck(Unit* _caster) : caster(_caster) { }

    bool operator()(WorldObject* object)
    {
        if (Unit* tank = caster->GetAI()->SelectTarget(SELECT_TARGET_TOPAGGRO, 0))
            return object == tank;

        return false;
    }

private:
    Unit* caster;
};

class spell_gravity_crush : public SpellScriptLoader
{
public:
    spell_gravity_crush() : SpellScriptLoader("spell_gravity_crush") {}

    class spell_gravity_crush_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_gravity_crush_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            instance = GetCaster()->GetInstanceScript();

            if (!instance)
                return;

            if (instance->GetData(DATA_GRAVITY_CRUSH) == NOT_STARTED)
            {
                instance->SetData(DATA_GRAVITY_CRUSH, IN_PROGRESS);
                targets.remove_if(TankCheck(GetCaster()));
                if (targets.empty())
                    return;

                Trinity::Containers::RandomResizeList(targets, GetCaster()->GetMap()->Is25ManRaid() ? 3 : 1);
                for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end(); itr++)
                    saveTargets.push_back(*itr);
            }
            else
            {
                targets.clear();
                for (std::list<WorldObject*>::iterator itr = saveTargets.begin(); itr != saveTargets.end(); itr++)
                    targets.push_back(*itr);
            }
        }

        void EffectScriptEffect(SpellEffIndex /*effIndex*/)
        {
            PreventHitDefaultEffect(EFFECT_2);
            Unit* caster = GetCaster();
            Unit* target = GetHitUnit();
            if (!caster || !target)
                return;


            if (Creature* vehicle = GetCaster()->SummonCreature(NPC_GRAVITY_CRUSH, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 6500))
                target->CastSpell(vehicle, SPELL_GRAVITY_CRUSH_CONTROL_VEHICLE, true);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_gravity_crush_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_gravity_crush_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_gravity_crush_SpellScript::FilterTargets, EFFECT_2, TARGET_UNIT_SRC_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(spell_gravity_crush_SpellScript::EffectScriptEffect, EFFECT_2, SPELL_EFFECT_FORCE_CAST);
        }

    private:
        std::list<WorldObject*> saveTargets;
        InstanceScript* instance;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_gravity_crush_SpellScript();
    }
};

class ExactDistanceCheck
{
public:
    ExactDistanceCheck(Unit* source, float dist) : _source(source), _dist(dist) {}

    bool operator()(WorldObject* unit)
    {
        return _source->GetExactDist2d(unit) > _dist;
    }

private:
    Unit* _source;
    float _dist;
};

class spell_liquid_ice : public SpellScriptLoader
{
public:
    spell_liquid_ice() : SpellScriptLoader("spell_liquid_ice") { }

    class spell_liquid_ice_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_liquid_ice_SpellScript);

        void CorrectRange(std::list<WorldObject*>& targets) // todo figure out the 100% correct factor
        {
            targets.remove_if(ExactDistanceCheck(GetCaster(), 2.60f * GetCaster()->GetFloatValue(OBJECT_FIELD_SCALE_X)));
        }

        void AddSpecialEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(EFFECT_1);
            if (GetHitUnit()->GetTypeId() != TYPEID_UNIT)
                return;

            if (GetHitUnit()->GetEntry() == NPC_ELEMENTIUM_MONSTROSITY)
                GetCaster()->CastSpell(GetCaster(), SPELL_LIQUID_ICE_GROW, true);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_liquid_ice_SpellScript::CorrectRange, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_liquid_ice_SpellScript::CorrectRange, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
            OnEffectHitTarget += SpellEffectFn(spell_liquid_ice_SpellScript::AddSpecialEffect, EFFECT_1, SPELL_EFFECT_APPLY_AURA);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_liquid_ice_SpellScript();
    }
};

class spell_cryogenic_aura : public SpellScriptLoader
{
public:
    spell_cryogenic_aura() : SpellScriptLoader("spell_cryogenic_aura") { }

    class spell_cryogenic_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_cryogenic_aura_AuraScript);

        void OnPeriodic(AuraEffect const* aurEff)
        {
            GetCaster()->SummonCreature(NPC_LIQUID_ICE, GetCaster()->GetPositionX(), GetCaster()->GetPositionY(), GetCaster()->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 120000);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_cryogenic_aura_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_cryogenic_aura_AuraScript();
    }
};

class spell_rising_flames : public SpellScriptLoader
{
public:
    spell_rising_flames() : SpellScriptLoader("spell_rising_flames") { }

    class spell_rising_flames_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_rising_flames_AuraScript);

        void onPeriodicTick(AuraEffect const* aurEff)
        {
            GetCaster()->CastSpell(GetCaster(), SPELL_RISING_FLAMES_INC_DMG, true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_rising_flames_AuraScript::onPeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_rising_flames_AuraScript();
    }
};

class spell_lashing_winds : public SpellScriptLoader
{
public:
    spell_lashing_winds() : SpellScriptLoader("spell_lashing_winds") { }

    class spell_lashing_winds_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_lashing_winds_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit *caster = GetCaster())
                if (Unit* target = GetTarget())
                {
                    if (target->HasAura(SPELL_GROUNDED))
                        target->RemoveAurasDueToSpell(SPELL_GROUNDED);

                    caster->CastSpell(target, SPELL_SWIRLING_WINDS, true);
                }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_lashing_winds_AuraScript::OnApply, EFFECT_2, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_lashing_winds_AuraScript();
    }
};

class spell_grounded : public SpellScriptLoader
{
public:
    spell_grounded() : SpellScriptLoader("spell_grounded") { }

    class spell_grounded_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_grounded_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* target = GetTarget())
            {
                if (target->HasAura(SPELL_SWIRLING_WINDS))
                    target->RemoveAurasDueToSpell(SPELL_SWIRLING_WINDS);
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_grounded_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_grounded_AuraScript();
    }
};

class spell_harden_skin : public SpellScriptLoader
{
public:
    spell_harden_skin() : SpellScriptLoader("spell_harden_skin") { }

    class spell_harden_skin_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_harden_skin_AuraScript);

        void Absorb(AuraEffect* /*aurEff*/, DamageInfo &dmgInfo, uint32 &absorbAmount)
        {
            uint32 damageToReduce = dmgInfo.GetDamage();
            absorbAmount = CalculatePct(damageToReduce, 50);
        }

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
        {
            AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
            if (removeMode != AURA_REMOVE_BY_DAMAGE)
                return;

            if (GetSpellInfo())
            {
                uint32 damage = GetSpellInfo()->Effects[EFFECT_1].BasePoints;
                GetTarget()->DealDamage(GetTarget(), damage, NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            }
        }

        void Register()
        {
            OnEffectAbsorb += AuraEffectAbsorbFn(spell_harden_skin_AuraScript::Absorb, EFFECT_1);
            AfterEffectRemove += AuraEffectRemoveFn(spell_harden_skin_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_harden_skin_AuraScript();
    }
};


class DebuffCheck
{
public:
    bool operator()(WorldObject* target) const
    {
        if (Unit* unit = target->ToUnit())
            return !unit->HasAura(SPELL_LIGHTNING_ROD);
        return false;
    }
};

class spell_chain_lightning : public SpellScriptLoader
{
public:
    spell_chain_lightning() : SpellScriptLoader("spell_chain_lightning") { }

    class spell_chain_lightning_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_chain_lightning_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(DebuffCheck());
        }

        void HandleDummy(SpellEffIndex effIndex)
        {
            GetCaster()->CastSpell(GetHitUnit(), GetSpellInfo()->Effects[EFFECT_0].BasePoints, true);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_chain_lightning_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(spell_chain_lightning_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_chain_lightning_SpellScript();
    }
};

class spell_electric_instability : public SpellScriptLoader
{
public:
    spell_electric_instability() : SpellScriptLoader("spell_electric_instability") { }

    class spell_electric_instability_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_electric_instability_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (AuraEffect const* aurEff = GetCaster()->GetAuraEffect(SPELL_ELECTRIC_INSTABILITY_AURA, EFFECT_0, GetCaster()->GetGUID()))
            {
                uint8 hitPlr = 0;
                uint32 ticknumber = aurEff->GetTickNumber();
                bool is25Raid = GetCaster()->GetMap()->Is25ManRaid();

                hitPlr = ticknumber / 20;
                if (hitPlr == 0)
                    hitPlr = 1;
                else if (hitPlr > (is25Raid ? 25 : 10))
                    hitPlr = (is25Raid ? 25 : 10);

                Trinity::Containers::RandomResizeList(targets, hitPlr);
            }
        }

        void HandleDummy(SpellEffIndex effIndex)
        {
            GetCaster()->CastSpell(GetHitUnit(), SPELL_ELECTRIC_INSTABILITY_TARGET, true);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_electric_instability_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(spell_electric_instability_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_electric_instability_SpellScript();
    }
};

class spell_hell_jump : public SpellScriptLoader
{
public:
    spell_hell_jump() : SpellScriptLoader("spell_hell_jump") { }

    class spell_hell_jump_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hell_jump_SpellScript);

        void ModSpeed(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
        }

        void Register()
        {
            OnEffectHit += SpellEffectFn(spell_hell_jump_SpellScript::ModSpeed, EFFECT_1, SPELL_EFFECT_TRIGGER_MISSILE);
            OnEffectHit += SpellEffectFn(spell_hell_jump_SpellScript::ModSpeed, EFFECT_0, SPELL_EFFECT_JUMP_DEST);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hell_jump_SpellScript();
    }
};

class StaticOverloadCheck
{
public:
    bool operator()(WorldObject* target) const
    {
        if (Unit* unit = target->ToUnit())
            return unit->HasAura(SPELL_STATIC_OVERLOAD);
        return false;
    }
};

class spell_gravity_core : public SpellScriptLoader
{
public:
    spell_gravity_core() : SpellScriptLoader("spell_gravity_core") { }

    class spell_gravity_core_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_gravity_core_AuraScript);

        void OnPeriodic(AuraEffect const* aurEff)
        {
            if (Unit* owner = GetTarget())
            {
                std::list<Player *> _players = owner->GetPlayersInRange(2.0f, true);
                for (std::list<Player *>::iterator itr = _players.begin(); itr != _players.end(); itr++)
                {
                    if (Player *target = *itr)
                        if (target->HasAura(SPELL_STATIC_OVERLOAD))
                        {
                            target->RemoveAurasDueToSpell(SPELL_STATIC_OVERLOAD);
                            owner->RemoveAurasDueToSpell(SPELL_GRAVITY_CORE);
                        }
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_gravity_core_AuraScript::OnPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    class spell_gravity_core_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_gravity_core_SpellScript);

        bool Load()
        {
            _target = NULL;
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(AllTankCheck());
            targets.remove_if(StaticOverloadCheck());
            Trinity::Containers::RandomResizeList(targets, 1);
            if (!targets.empty())
                _target = targets.front();
        }

        void SetTarget(std::list<WorldObject*>& targets)
        {
            targets.clear();
            if (_target)
                targets.push_back(_target);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_gravity_core_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_gravity_core_SpellScript::SetTarget, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    private:
        WorldObject *_target;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_gravity_core_SpellScript();
    }

    AuraScript* GetAuraScript() const
    {
        return new spell_gravity_core_AuraScript();
    }
};

class spell_frozen_orb_periodic : public SpellScriptLoader
{
public:
    spell_frozen_orb_periodic() : SpellScriptLoader("spell_frozen_orb_periodic") { }

    class spell_frozen_orb_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_frozen_orb_periodic_AuraScript);

        void OnPeriodic(AuraEffect const* aurEff)
        {
            if (Unit *owner = GetUnitOwner())
                owner->AddAura(SPELL_FROZEN_ORB_INCREASE_RUN_SPEED, owner);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_frozen_orb_periodic_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_frozen_orb_periodic_AuraScript();
    }
};

class spell_frozen_orb_dummy : public SpellScriptLoader
{
public:
    spell_frozen_orb_dummy() : SpellScriptLoader("spell_frozen_orb_dummy") { }

    class spell_frozen_orb_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_frozen_orb_dummy_SpellScript);

        void HandleDummy(SpellEffIndex effIndex)
        {
            if (Unit *caster = GetCaster())
                if (Creature *orb = caster->ToCreature())
                    if (Unit *target = GetHitUnit())
                    {
                        if (target->HasAura(SPELL_FROST_BEACON))
                        {
                            target->RemoveAurasDueToSpell(SPELL_FROST_BEACON);
                            orb->AI()->DoCast(SPELL_GLACIATE_DAMAGE);
                            orb->DespawnOrUnsummon(100);
                        }
                    }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_frozen_orb_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_frozen_orb_dummy_SpellScript();
    }
};

class spell_ignacius_flame_strike_dummy : public SpellScriptLoader
{
public:
    spell_ignacius_flame_strike_dummy() : SpellScriptLoader("spell_ignacius_flame_strike_dummy") { }

    class spell_ignacius_flame_strike_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ignacius_flame_strike_dummy_SpellScript);

        void HandleDummy(SpellEffIndex effIndex)
        {
            if (Unit *owner = GetCaster())
                if (Creature *flame = owner->ToCreature())
                    if (Unit *target = GetHitUnit())
                        if (Creature *orb = target->ToCreature())
                        {
                            orb->AI()->DoAction(ACTION_DESPAWN);
                            flame->DespawnOrUnsummon(100);
                        }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_ignacius_flame_strike_dummy_SpellScript::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_ignacius_flame_strike_dummy_SpellScript();
    }
};

class GravityCoreCheck
{
public:
    bool operator()(WorldObject* target) const
    {
        if (Unit* unit = target->ToUnit())
            return unit->HasAura(SPELL_GRAVITY_CORE);
        return false;
    }
};

class spell_arion_static_overload : public SpellScriptLoader // 92067.
{
public:
    spell_arion_static_overload() : SpellScriptLoader("spell_arion_static_overload") {}

    class spell_arion_static_overload_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_arion_static_overload_SpellScript);

        bool Load()
        {
            _target = NULL;
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(AllTankCheck());
            targets.remove_if(GravityCoreCheck());
            Trinity::Containers::RandomResizeList(targets, 1);
            if (!targets.empty())
                _target = targets.front();
        }

        void SetTarget(std::list<WorldObject*>& targets)
        {
            targets.clear();
            if (_target)
                targets.push_back(_target);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_arion_static_overload_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_arion_static_overload_SpellScript::SetTarget, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }

    private:
        WorldObject *_target;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_arion_static_overload_SpellScript();
    }
};

// All are verified below to DB, creature_template & spell_script_names
void AddSC_boss_ascendant_council()
{
    new npc_ascendant_council_controller();
    new boss_feludius();
    new boss_ignacious();
    new boss_arion();
    new boss_terrastra();
    new boss_monstrosity();
    new npc_feludius_waterbomb();
    new npc_arion_callwinds();
    new npc_terrastra_gravitywell();
    new npc_ignacious_flame_strike();
    new npc_feludius_frozen_orb();
    new npc_monstrosity_liquid_ice();
    new npc_monstrosity_lava_seed();
    new spell_feludius_waterbomb();
    new spell_feludius_glaciate();
    new spell_arion_disperse();
    new spell_arion_thundershock();
    new spell_terrastra_quake();
    new spell_ignacious_inferno_rush();
    new spell_feludius_heart_of_ice();
    new spell_gravity_crush();
    new spell_liquid_ice();
    new spell_cryogenic_aura();
    new spell_rising_flames();
    new spell_lashing_winds();
    new spell_grounded();
    new spell_harden_skin();
    new spell_chain_lightning();
    new spell_electric_instability();
    new spell_hell_jump();
    new spell_gravity_core();
    new spell_frozen_orb_dummy();
    new spell_frozen_orb_periodic();
    new spell_ignacius_flame_strike_dummy();
    new spell_arion_static_overload();
}
