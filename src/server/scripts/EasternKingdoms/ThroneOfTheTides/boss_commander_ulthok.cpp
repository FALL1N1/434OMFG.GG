
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "throne_of_the_tides.h"

#define SPELL_DARK_FISSURE          DUNGEON_MODE(76047,96311)
#define SPELL_DARK_FISSURE_AURA     DUNGEON_MODE(76066,91371)
#define SPELL_SQUEEZE               DUNGEON_MODE(76026,95463)

enum Spells
{
    SPELL_ENRAGE                = 76100,
    SPELL_CURSE_OF_FATIGUE      = 76094,
    SPELL_PULL_TARGET           = 67357,
    SPELL_QUEEZE_VEHICLE = 76038,
    SPELL_INTRO_FALL_VISUAL     = 76017,
    SPELL_INTRO_JUMP_VISUAL     = 82960,
};

enum Yells
{
    SAY_AGGRO                   = -1643007,
    SAY_AGGRO_WHISP             = -1643008,
    SAY_DEATH                   = -1643009,
    SAY_DEATH_WHISP             = -1643010,
};

class boss_commander_ulthok : public CreatureScript
{
public:
    boss_commander_ulthok() : CreatureScript("boss_commander_ulthok") {}

    struct boss_commander_ulthokAI : public ScriptedAI
    {
        boss_commander_ulthokAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            DarkFissureTimer = 22500;
            EnrageTimer = urand(12000,18000);
            SqueezeTimer = 25500;
            CurseTimer = 32000;
            start_event = false;
            mui_start_event = 500;
            SqueezeTargetGUID = 0;
            SqueezeStopTimer = 3600000;
            SqueezeRemovePassengers = 3600000;
            if (Vehicle *veh = me->GetVehicleKit())
                veh->RemoveAllPassengers();
            if (instance)
                instance->SetData(DATA_COMMANDER_ULTHOK, NOT_STARTED);
            me->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
            me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), 810.0f, me->GetOrientation());
        }

        void EnterCombat(Unit* /*who*/)
        {
            Talk(0);
            if (instance)
                instance->SetData(DATA_COMMANDER_ULTHOK, IN_PROGRESS);
        }

        void JustReachedHome() {}

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case 42 :
                start_event = true;
                break;
            default :
                break;
            }
        }

        void MovementInform(uint32 Type, uint32 Id)
        {
            if (Type == POINT_MOTION_TYPE && Id == 2)
                if (GameObject* corail = ObjectAccessor::GetGameObject(*me, instance->GetData64(GO_CORAIL)))
                {
                    corail->Use(me);
                    corail->Delete();
                    me->RemoveAura(SPELL_INTRO_FALL_VISUAL);
                    me->CastSpell(me, SPELL_INTRO_JUMP_VISUAL, true);
                    me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                }
        }

        void UpdateAI(const uint32 diff)
        {
            if (start_event)
            {
                if (mui_start_event <= diff)
                {
                    me->CastSpell(me, SPELL_INTRO_FALL_VISUAL, true);
                    me->GetMotionMaster()->MovePoint(2, me->GetPositionX(), me->GetPositionY(), 810.0f);
                    start_event = false;
                    mui_start_event = 2000;
                }
                else
                    mui_start_event -= diff;
                return;
            }

            if (!UpdateVictim())
                return;

            if (DarkFissureTimer <= diff)
            {
                Talk(1);
                DoCastVictim(SPELL_DARK_FISSURE);
                DarkFissureTimer = 22500;
            }
            else
                DarkFissureTimer -= diff;

            if (EnrageTimer <= diff)
            {
                DoCast(me, SPELL_ENRAGE);
                EnrageTimer = urand(12000,15000);
            }
            else
                EnrageTimer -= diff;

            if (SqueezeTimer <= diff)
            {
                if (Unit * SqueezeTarget = SelectTarget(SELECT_TARGET_RANDOM, 0))
                {
                    SqueezeTargetGUID = SqueezeTarget->GetGUID();
                    DoCast(SqueezeTarget, SPELL_SQUEEZE, true);
                    SqueezeStopTimer = 6000;
                }
                SqueezeTimer = 22500;
            }
            else
                SqueezeTimer -= diff;

            if (SqueezeStopTimer <= diff)
            {
                if (Unit *SqueezeTarget = Unit::GetUnit(*me, SqueezeTargetGUID))
                    SqueezeTarget->CastSpell(me, SPELL_QUEEZE_VEHICLE, true);
                SqueezeRemovePassengers = 2000;
                SqueezeStopTimer = 3600000;
            }
            else SqueezeStopTimer -= diff;

            if (SqueezeRemovePassengers <= diff)
            {
                if (Vehicle *veh = me->GetVehicleKit())
                    veh->RemoveAllPassengers();
                SqueezeRemovePassengers = 3600000;
            }
            else SqueezeRemovePassengers -= diff;

            if (CurseTimer <= diff)
            {
                if (Unit *SqueezeTarget = Unit::GetUnit(*me, SqueezeTargetGUID))
                    DoCast(SqueezeTarget, SPELL_CURSE_OF_FATIGUE, true);
                CurseTimer = 22500;
            }
            else
                CurseTimer -= diff;

            DoMeleeAttackIfReady();
        }

        void JustDied(Unit* /*killer*/)
        {
            Talk(2);
            if (instance)
                instance->SetData(DATA_COMMANDER_ULTHOK, DONE);
        }

    private:
        uint32 DarkFissureTimer;
        uint32 EnrageTimer;
        uint32 SqueezeTimer;
        uint32 CurseTimer;
        uint64 SqueezeTargetGUID;
        bool start_event;
        uint32 mui_start_event;
        uint32 SqueezeStopTimer;
        InstanceScript *instance;
        uint32 SqueezeRemovePassengers;
    };

    CreatureAI* GetAI(Creature *creature) const
    {
        return new boss_commander_ulthokAI (creature);
    }
};

class npc_fissure : public CreatureScript
{
public:
    npc_fissure() : CreatureScript("npc_fissure") { }

    struct npc_fissureAI : public ScriptedAI
    {
        npc_fissureAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            me->AddAura(SPELL_DARK_FISSURE_AURA, me);
            mui_despawn = 1000;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE|UNIT_FLAG_DISABLE_MOVE);
        }

        void UpdateAI(const uint32 diff)
        {
            if (mui_despawn <= diff)
            {
                if (instance->GetData(DATA_COMMANDER_ULTHOK_EVENT) != IN_PROGRESS)
                    me->DespawnOrUnsummon();
                mui_despawn = 1000;
            }
            else mui_despawn -= diff;
        }

    private :
        InstanceScript* instance;
        uint32 mui_despawn;

    };

    CreatureAI* GetAI(Creature *creature) const
    {
        return new npc_fissureAI (creature);
    }
};

void AddSC_boss_commander_ulthok()
{
    new boss_commander_ulthok();
    new npc_fissure();
}
