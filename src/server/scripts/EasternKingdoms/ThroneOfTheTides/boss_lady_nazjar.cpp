
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "throne_of_the_tides.h"

#define SPELL_WATERSPOUT_SUMMON DUNGEON_MODE(90495,90497) // summons tornado every 7/3 secs.

enum Spells
{
    // Boss
    SPELL_FUNGAL_SPORES     = 76001,
    SPELL_SHOCK_BLAST       = 76008,
    SPELL_SUMMON_GEYSER     = 75722,
    SPELL_WATERSPOUT        = 75683,

    // Adds
    SPELL_CHAIN_LIGHTNING   = 75813,
    SPELL_LIGHTNING_SURGE   = 75992,
    SPELL_ARC_SLASH         = 75907,
    SPELL_ENRAGE            = 75998,

    SPELL_GEYSER_START = 75699,
    SPELL_GEYSER_END = 75700,

    // GEYSERIT
    SPELL_GEYSERIT_UNK1 = 90444,
    SPELL_GEYSERIT_UNK2 = 90484,
    SPELL_GEYSERIT_UNK3 = 90461,
    SPELL_GEYSERIT_TORNADE_AURA = 90440,
    SPELL_EJECT_PASSENGER = 63109,
    SPELL_SUMMON_GEYSERIT = 90471,
};

enum Yells
{
    SAY_AGGRO                   = -1643001,
    SAY_66_PRECENT              = -1643002,
    SAY_33_PRECENT              = -1643003,
    SAY_DEATH                   = -1643004,
    SAY_KILL_1                  = -1643005,
    SAY_KILL_2                  = -1643006,
};

enum Phases
{
    PHASE_ALL           = 0,
    PHASE_NORMAL        = 1,
    PHASE_WATERSPOUT    = 2,
};

class boss_lady_nazjar : public CreatureScript
{
public:
    boss_lady_nazjar() : CreatureScript("boss_lady_nazjar") { }

    struct boss_lady_nazjarAI : public ScriptedAI
    {
        boss_lady_nazjarAI(Creature* creature) : ScriptedAI(creature)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, false);
            me->ApplySpellImmune(SPELL_GEYSER_END, IMMUNITY_ID,  SPELL_GEYSER_END, true);
            me->ApplySpellImmune(94046, IMMUNITY_ID,  94046, true);
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            Phased = false;
            RemoveSummons();

            Phase = PHASE_NORMAL;

            SpawnCount = 3;
            PhaseCount = 0;

            FungalSporesTimer = urand(8000, 13000);
            ShockBlastTimer = 22000;
            SummonGeyserTimer = urand(11000, 16000);

            me->RemoveAurasDueToSpell(SPELL_WATERSPOUT);
            me->RemoveAurasDueToSpell(SPELL_WATERSPOUT_SUMMON);

            me->GetMotionMaster()->MoveTargetedHome();

            if (instance)
                instance->SetData(DATA_LADY_NAZJAR, NOT_STARTED);

            checkPl = 1000;

            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, false);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            switch(summon->GetEntry())
            {
            case NPC_SUMMONED_WITCH:
            case NPC_SUMMONED_GUARD:
                SpawnCount--;
                break;
            }
        }

        void RemoveSummons()
        {
            if (SummonList.empty())
                return;

            for (std::list<uint64>::const_iterator itr = SummonList.begin(); itr != SummonList.end(); ++itr)
                if (Creature* temp = Unit::GetCreature(*me, *itr))
                    temp->DisappearAndDie();
            SummonList.clear();
        }

        void JustSummoned(Creature* summon)
        {
            switch (summon->GetEntry())
            {
            case NPC_SUMMONED_WITCH:
            case NPC_SUMMONED_GUARD:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                    summon->AI()->AttackStart(target);
                SummonList.push_back(summon->GetGUID());
                break;
            case NPC_SUMMONED_WATERSPOUT:
                switch (urand(0, 14))
                {
                case 0 :
                    summon->GetMotionMaster()->MovePoint(42, 216.73f, 827.56f, 809.32f);
                    break;
                case 1 :
                    summon->GetMotionMaster()->MovePoint(42, 219.73f, 813.56f, 809.32f);
                    break;
                case 2 :
                    summon->GetMotionMaster()->MovePoint(42, 227.73f, 804.56f, 809.32f);
                    break;
                case 3 :
                    summon->GetMotionMaster()->MovePoint(42, 219.73f, 790.56f, 809.32f);
                    break;
                case 4 :
                    summon->GetMotionMaster()->MovePoint(42, 214.73f, 781.56f, 809.32f);
                    break;
                case 5 :
                    summon->GetMotionMaster()->MovePoint(42, 202.73f, 774.56f, 809.32f);
                    break;
                case 6 :
                    summon->GetMotionMaster()->MovePoint(42, 192.73f, 768.56f, 809.32f);
                    break;
                case 7 :
                    summon->GetMotionMaster()->MovePoint(42, 180.73f, 774.56f, 809.32f);
                    break;
                case 8 :
                    summon->GetMotionMaster()->MovePoint(42, 170.73f, 778.56f, 809.32f);
                    break;
                case 9 :
                    summon->GetMotionMaster()->MovePoint(42, 158.73f, 800.56f, 809.32f);
                    break;
                case 10 :
                    summon->GetMotionMaster()->MovePoint(42, 163.73f, 813.56f, 809.32f);
                    break;
                case 11 :
                    summon->GetMotionMaster()->MovePoint(42, 165.73f, 827.56f, 809.32f);
                    break;
                case 12 :
                    summon->GetMotionMaster()->MovePoint(42, 180.73f, 830.56f, 809.32f);
                    break;
                case 13 :
                    summon->GetMotionMaster()->MovePoint(42, 192.73f, 840.56f, 809.32f);
                    break;
                case 14 :
                    summon->GetMotionMaster()->MovePoint(42, 205.73f, 830.56f, 809.32f);
                    break;
                default :
                    break;
                }
                summon->AddAura(90440, summon);
                break;
            }
        }

        void EnterCombat(Unit* /*who*/)
        {
            Talk(0);
            if (instance)
                instance->SetData(DATA_LADY_NAZJAR, IN_PROGRESS);
        }

        void JustDied(Unit* /*killer*/)
        {
            RemoveSummons();
            Talk(3);
            if (instance)
                instance->SetData(DATA_LADY_NAZJAR, DONE);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (SpawnCount == 0 && Phase == PHASE_WATERSPOUT)
            {
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, false);
                SpawnCount = 3;
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                Phase = PHASE_NORMAL;
                Phased = false;
                FungalSporesTimer = urand(8000, 13000);
                ShockBlastTimer = 22000;
                SummonGeyserTimer = urand(11000, 16000);
                me->RemoveAurasDueToSpell(SPELL_WATERSPOUT);
                me->RemoveAurasDueToSpell(SPELL_WATERSPOUT_SUMMON);
            }

            if (me->HealthBelowPct(67) && Phase == PHASE_NORMAL && PhaseCount == 0)
            {
                Talk(1);
                PhaseCount++;
                Phase = PHASE_WATERSPOUT;
                float x, y, z, o;
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
                me->GetHomePosition(x, y, z, o);
                me->NearTeleportTo(x, y, z, o);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                DoCast(me, SPELL_WATERSPOUT);
                me->AddAura(SPELL_WATERSPOUT_SUMMON, me);
                Position pos;
                me->GetPosition(&pos);
                me->SummonCreature(NPC_SUMMONED_WITCH, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000);
                me->SummonCreature(NPC_SUMMONED_WITCH, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000);
                me->SummonCreature(NPC_SUMMONED_GUARD, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000);
                Phase2EndTimer = 60000;
            }

            if (me->HealthBelowPct(34) && Phase == PHASE_NORMAL && PhaseCount == 1)
            {
                Talk(2);
                PhaseCount++;
                Phase = PHASE_WATERSPOUT;
                float x, y, z, o;
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
                me->GetHomePosition(x, y, z, o);
                me->NearTeleportTo(x, y, z, o);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                DoCast(me, SPELL_WATERSPOUT);
                me->AddAura(SPELL_WATERSPOUT_SUMMON, me);
                Position pos;
                me->GetPosition(&pos);
                me->SummonCreature(NPC_SUMMONED_WITCH, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000);
                me->SummonCreature(NPC_SUMMONED_WITCH, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000);
                me->SummonCreature(NPC_SUMMONED_GUARD, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000);
                Phase2EndTimer = 60000;
            }

            if (FungalSporesTimer <= diff && Phase == PHASE_NORMAL)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                    DoCast(target, SPELL_FUNGAL_SPORES);
                FungalSporesTimer = urand(5000,7000);
            }
            else
                FungalSporesTimer -= diff;

            if (ShockBlastTimer <= diff && Phase == PHASE_NORMAL)
            {
                DoCastVictim(SPELL_SHOCK_BLAST);
                ShockBlastTimer = urand(12000,15000);
            }
            else
                ShockBlastTimer -= diff;

            if (SummonGeyserTimer <= diff && Phase == PHASE_NORMAL)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                    DoCast(target, SPELL_SUMMON_GEYSER);
                SummonGeyserTimer = urand(13000,16000);
            }
            else
                SummonGeyserTimer -= diff;

            if (Phase == PHASE_WATERSPOUT)
            {
                if (Phase2EndTimer <= diff)
                {
                    SpawnCount = 3;
                    SetCombatMovement(true);
                    Phase = PHASE_NORMAL;
                    Phased = false;
                    FungalSporesTimer = urand(8000,13000);
                    ShockBlastTimer = 22000;
                    SummonGeyserTimer = urand(11000,16000);
                    me->RemoveAurasDueToSpell(SPELL_WATERSPOUT);
                    me->RemoveAurasDueToSpell(SPELL_WATERSPOUT_SUMMON);
                }
                else
                    Phase2EndTimer -= diff;
            }

            DoMeleeAttackIfReady();
        }

    private :
        std::list<uint64> SummonList;
        InstanceScript* instance;
        uint8 Phase;
        bool Phased;
        uint8 SpawnCount;
        uint8 PhaseCount;
        uint32 FungalSporesTimer;
        uint32 ShockBlastTimer;
        uint32 SummonGeyserTimer;
        uint32 Phase2EndTimer;
        uint32 checkPl;

    };

    CreatureAI* GetAI(Creature *creature) const
    {
        return new boss_lady_nazjarAI (creature);
    }
};

class npc_lady_nazjar_geyser : public CreatureScript
{
public:
    npc_lady_nazjar_geyser() : CreatureScript("npc_lady_nazjar_geyser") { }

    struct npc_lady_nazjar_geyserAI : public ScriptedAI
    {
        npc_lady_nazjar_geyserAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, false);
        }
    private :
        InstanceScript* instance;
        uint32 mui_geyser;

        void Reset()
        {
            DoCast(me, SPELL_GEYSER_START);
            mui_geyser = 5000;
            me->ApplySpellImmune(SPELL_GEYSER_END, IMMUNITY_ID,  SPELL_GEYSER_END, true);
            me->ApplySpellImmune(94046, IMMUNITY_ID,  94046, true);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE|UNIT_FLAG_DISABLE_MOVE);
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() != TYPEID_PLAYER)
            {
                switch (victim->GetEntry())
                {
                case NPC_SUMMONED_WITCH:
                case 49084:
                case NPC_SUMMONED_GUARD:
                case 57508:
                    if (IsHeroic())
                        instance->DoCompleteAchievement(HEROIC_NAZJAR_GEYSER_KILLED_TRASH);
                    break;
                default :
                    break;
                }

            }
        }

        void DamageTaken(Unit* /*doneBy*/, uint32 &damage)
        {
            damage = 0;
        }

        void UpdateAI(const uint32 diff)
        {
            if (mui_geyser <= diff)
            {
                DoCast(me, SPELL_GEYSER_END);
                me->DespawnOrUnsummon(1500);
                mui_geyser = 30000;
            }
            else mui_geyser -= diff;
        }
    };

    CreatureAI* GetAI(Creature *creature) const
    {
        return new npc_lady_nazjar_geyserAI (creature);
    }
};

class npc_lady_nazjar_tornade : public CreatureScript
{
public:
    npc_lady_nazjar_tornade() : CreatureScript("npc_lady_nazjar_tornade") { }

    struct npc_lady_nazjar_tornadeAI : public ScriptedAI
    {
        npc_lady_nazjar_tornadeAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, false);
        }

        void Reset()
        {
            enter = false;
            mui_geyser = 500;
            me->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
            me->SetSpeed(MOVE_RUN, 1.1f);
            me->SetSpeed(MOVE_WALK, 1.1f);
            me->SetReactState(REACT_PASSIVE);
        }

        void AttackStart(Unit *)
        {

        }

        void MovementInform(uint32 /*type*/, uint32 id)
        {
            if (id == 42)
                me->DespawnOrUnsummon();
        }

        void DamageTaken(Unit* , uint32 &damage)
        {
            damage = 0;
        }

        void UpdateAI(const uint32 diff)
        {
            if (enter)
                return;
            if (mui_geyser <= diff)
            {
                if (Player *pl = me->FindNearestPlayer(2.0f))
                {
                    if (!pl->GetVehicle())
                        if (me->IsWithinDistInMap(pl, 2.0f))
                        {
                            pl->CastSpell(pl, 90479, true);
                            pl->CastSpell(me, VEHICLE_SPELL_RIDE_HARDCODED, true);
                            enter = true;
                        }
                }
                mui_geyser = 100;
            }
            else mui_geyser -= diff;
        }

    private :
        InstanceScript* instance;
        uint32 mui_geyser;
        bool enter;
    };

    CreatureAI* GetAI(Creature *creature) const
    {
        return new npc_lady_nazjar_tornadeAI (creature);
    }
};

void AddSC_boss_lady_nazjar()
{
    new boss_lady_nazjar();
    new npc_lady_nazjar_geyser();
    new npc_lady_nazjar_tornade();
}
