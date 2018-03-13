/*
* Copyright (C) 2009 - 2016 omfg.gg <http://omfg.gg/>
*/

#include "Player.h"
#include "InstanceScript.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "throne_of_the_four_winds.h"


class npc_td4w_slipstream : public CreatureScript
{
public:
    npc_td4w_slipstream() : CreatureScript("npc_td4w_slipstream") { }

    struct npc_td4w_slipstreamAI : public ScriptedAI
    {
        npc_td4w_slipstreamAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            sleapStreamRefresh = 250;
        }

        void MovementInform(uint32 type, uint32 point)
        {
            if (type == EFFECT_MOTION_TYPE)
            {
                switch (point)
                {
                case EVENT_JUMP:
                    if (Unit* passenger = me->GetVehicleKit()->GetPassenger(0))
                    {
                        passenger->ExitVehicle(me);

                        passenger->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), 200.0f, 0.0f);
                        passenger->UpdatePosition(me->GetPositionX(), me->GetPositionY(), 200.0f, true);
                        passenger->CastSpell(passenger, 85275, true);
                        passenger->ToPlayer()->SetViewpoint(me, false);
                        passenger->RemoveAura(85269);
                    }
                    break;
                }
            }
        }

        void SetGUID(uint64 guid, int32)
        {
            me->RemoveAllAuras();
            me->SetSpeed(MOVE_FLIGHT, 3.0f);
            if (Player *player = Unit::GetPlayer(*me, guid))
                player->CastSpell(me, 85282, true);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void UpdateAI(const uint32 diff)
        {
            if (sleapStreamRefresh <= diff)
            {
                if (me->GetVehicleKit())
                    if (Unit* passenger = me->GetVehicleKit()->GetPassenger(0))
                    {
                        Position pos(*me);
                        passenger->Relocate(&pos);
                    }

                sleapStreamRefresh = 500;
            }
            else
                sleapStreamRefresh -= diff;
        }

        std::map<int, uint64 > _slipStream;
    private:
        uint32 sleapStreamRefresh;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_td4w_slipstreamAI(creature);
    }
};

void AddSC_throne_of_the_four_winds()
{
    new npc_td4w_slipstream();
}