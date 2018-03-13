#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "SpellAuraEffects.h"
#include "SmartAI.h"
#include "throne_of_the_tides.h"

#define MURLOC_ESCALVE 39960
#define MAX_WAVE 5
#define SPELL_MURLOC_LEACH_VISUAL 74568
#define SPELL_MURLOC_LEACH_VISUAL2 74563

class at_neptulon_pinacle : public AreaTriggerScript
{
public:
    at_neptulon_pinacle() : AreaTriggerScript("at_neptulon_pinacle") { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* /*areaTrigger*/)
    {
        InstanceScript* instance = player->GetInstanceScript();
        if (!instance || instance->GetData(DATA_PROGRESS_EVENT) >= 2)
            return true;
        instance->SetData(DATA_PROGRESS_EVENT, 2);
        return true;
    }
};

class at_abyssal_halls : public AreaTriggerScript
{
public:
    at_abyssal_halls() : AreaTriggerScript("at_abyssal_halls") { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* /*areaTrigger*/)
    {
        InstanceScript* instance = player->GetInstanceScript();
        if (!instance || instance->GetData(DATA_PROGRESS_EVENT) >= 1)
            return true;
        instance->SetData(DATA_PROGRESS_EVENT, 1);
        return true;
    }
};

class npc_declencheur_vaque_aqua : public CreatureScript
{
public:
    npc_declencheur_vaque_aqua() : CreatureScript("npc_declencheur_vaque_aqua") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_declencheur_vaque_aquaAI(creature);
    }

    struct npc_declencheur_vaque_aquaAI : public ScriptedAI
    {
        npc_declencheur_vaque_aquaAI(Creature* creature) : ScriptedAI(creature), _instance(creature->GetInstanceScript())
        {
        }

        void Reset ()
        {
            _invok = 10000;
            nbrWave = 0;
            summonTime = false;
            start_move = false;
            _instance->HandleGameObject(_instance->GetData64(GO_ABYSSAL_MAW_01), false, _instance->instance->GetGameObject(_instance->GetData64(GO_ABYSSAL_MAW_01)));
            for (int cnt = 0; cnt < 5; cnt++)
                precWave[cnt] = 0;
        }

        void JustSummoned(Creature* summon)
        {
            summon->CastSpell(me, SPELL_MURLOC_LEACH_VISUAL, true);
        }

        void summonMurlocWave()
        {
            if (Creature *c = me->SummonCreature(MURLOC_ESCALVE, me->GetPositionX() - 30.0f, me->GetPositionY(),  798.0f + 2.0f,  me->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,
                                                 600*IN_MILLISECONDS))
                precWave[0] = c->GetGUID();
            if (Creature *c = me->SummonCreature(MURLOC_ESCALVE, me->GetPositionX() + 2.0f - 30.0f, me->GetPositionY() + 2.0f,  798.0f + 2.0f,  me->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,
                                                 600*IN_MILLISECONDS))
                precWave[1] = c->GetGUID();
            if (Creature *c = me->SummonCreature(MURLOC_ESCALVE, me->GetPositionX() - 30.0f, me->GetPositionY() + 2.0f,  798.0f + 2.0f,  me->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,
                                                 600*IN_MILLISECONDS))
                precWave[2] = c->GetGUID();
            if (Creature *c = me->SummonCreature(MURLOC_ESCALVE, me->GetPositionX() + 2.0f - 30.0f, me->GetPositionY(),  798.0f + 2.0f,  me->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,
                                                 600*IN_MILLISECONDS))
                precWave[3] = c->GetGUID();
            if (Creature *c = me->SummonCreature(MURLOC_ESCALVE, me->GetPositionX() - 2.0f - 30.0f, me->GetPositionY(),  798.0f + 2.0f,  me->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,
                                                 600*IN_MILLISECONDS))
                precWave[4] = c->GetGUID();
        }

        void moveMurlocWave()
        {
            for (int cnt = 0; cnt < 5; cnt++)
                if (Creature *murloc = Unit::GetCreature(*me, precWave[cnt]))
                    murloc->GetMotionMaster()->MovePoint(1, -147.0f + frand(-2.0f, 2.0f), 802.55f + frand(-2.0f, 2.0f), 796.1f);
        }

        void UpdateAI(const uint32 diff)
        {
            if (_instance->GetData(DATA_PROGRESS_EVENT) < 2)
                return;
            if (!start_move)
            {
                if (_invok <= diff)
                {
                    moveMurlocWave();
                    if (nbrWave >= MAX_WAVE)
                    {
                        summonTime = false;
                        start_move = true;
                    }
                    else
                        summonMurlocWave();
                    nbrWave++;
                    _invok = 10000;
                }
                else _invok -= diff;
            }
            else
            {
                Talk(1);
                _instance->DoUseDoorOrButton(_instance->GetData64(GO_ABYSSAL_MAW_01));
                me->GetMotionMaster()->MovePoint(42, 34.73f, 799.56f, 806.32f);
                me->DespawnOrUnsummon(10000);
                start_move = false;
            }
        }

    private:
        InstanceScript* const _instance;
        uint32 _invok;
        uint32 nbrWave;
        bool summonTime, start_move;
        uint64 precWave[5];
    };
};

class go_defence_throne: public GameObjectScript
{
public:
    go_defence_throne () :
        GameObjectScript("go_defence_throne")
    {
    }

    bool OnGossipHello(Player* pPlayer, GameObject *go)
    {
        if (InstanceScript* instance = go->GetInstanceScript())
        {
            if (instance->GetData(DATA_LADY_NAZJAR_EVENT) != DONE  || instance->GetData(DATA_PROGRESS_EVENT) >= 3)
                return true;
            instance->SetData(DATA_PROGRESS_EVENT, 3);
            instance->SetData(DATA_EVENT_COMMANDER_ULTHOK_START, IN_PROGRESS);
            instance->SendCinematicStartToPlayers(169);
            return false;
        }
        return true;
    }
};

class at_abyssal_halls_x : public AreaTriggerScript
{
public:
    at_abyssal_halls_x() : AreaTriggerScript("at_abyssal_halls_x") { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* /*areaTrigger*/)
    {
        InstanceScript* instance = player->GetInstanceScript();
        if (!instance || instance->GetData(DATA_LADY_NAZJAR_EVENT) != DONE || instance->GetData(DATA_COMMANDER_ULTHOK) == DONE)
            return true;
        Position pos = {51.10f, 802.15f, 842.0f, 3.22f};
        if (!instance->instance->GetCreature(instance->GetData64(DATA_COMMANDER_ULTHOK)))
            if (Creature *c = instance->instance->SummonCreature(40765, pos))
                c->AI()->DoAction(42);
        return true;
    }
};

#define TELEPORTEUR_AURA 95293

class npc_teleporteur_throme_of_tides : public CreatureScript
{
public:
    npc_teleporteur_throme_of_tides() : CreatureScript("npc_teleporteur_throme_of_tides") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        InstanceScript* pInstance = creature->GetInstanceScript();

        if (pInstance &&
            pInstance->GetData(DATA_COMMANDER_ULTHOK_EVENT) == DONE)
        {
            if (creature->GetPositionZ() > 250.0f)
                player->TeleportTo(player->GetMapId(), -611.17f, 806.87f, 246.0f, 6.22f);
            else
                player->TeleportTo(player->GetMapId(), -314.26f, 807.80f, 261.4f, 0.01f);
            return true;
        }
        return false;
    }
};

void AddSC_throne_of_the_tides()
{
    new npc_declencheur_vaque_aqua();
    new go_defence_throne();
    new npc_teleporteur_throme_of_tides();
    new at_neptulon_pinacle();
    new at_abyssal_halls();
    new at_abyssal_halls_x();
}
