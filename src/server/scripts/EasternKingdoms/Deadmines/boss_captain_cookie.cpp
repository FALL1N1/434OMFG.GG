/*
 * Copyright (C) 2005-2011 MaNGOS <http://www.getmangos.com/>
 *
 * Copyright (C) 2008-2011 Trinity <http://www.trinitycore.org/>
 *
 * Copyright (C) 2006-2011 ScriptDev2 <http://www.scriptdev2.com/>
 *
 * Copyright (C) 2010-2011 Project Trinity <http://www.projecttrinity.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* ToDo
 * Cookie Walk
 * Food despawn after looting
 * Mount on Cauldron or is it a gobject?
 */

#include "PassiveAI.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "deadmines.h"

enum Spells
{
    SPELL_SUMMON_CAULDRON   = 89250,
    SPELL_CAULDRON_FIRE     = 89252,

    // Good Food
    SPELL_THROW_FOOD1       = 90557, // 48301
    SPELL_THROW_FOOD2       = 90560, // 48297
    SPELL_THROW_FOOD3       = 90556, // 48300
    SPELL_THROW_FOOD4       = 90559, // 48296
    SPELL_THROW_FOOD5       = 89263, // 48006
    SPELL_THROW_FOOD6       = 90555, // 48294

    // Bad Food
    SPELL_THROW_FOOD7       = 90606, // 48302
    SPELL_THROW_FOOD8       = 90603, // 48295
    SPELL_THROW_FOOD9       = 89739, // 48276
    SPELL_THROW_FOOD10      = 90605, // 48299
    SPELL_THROW_FOOD11      = 90602, // 48293
    SPELL_THROW_FOOD12      = 90604, // 48298

    // MISC (MURLOC
    SPELL_THROW_MURLOC      = 90680, // 48672

    // FOOD NPC
    SPELL_ROTTEN_AURA       = 89734,
    SPELL_FED               = 89267,
    SPELL_ILLNESS           = 89732,
    SPELL_GOOD_FOOD_AURA    = 89730,

    SPELL_BABY_MURLOC_DANCE = 25165,

    // Achievement
    SPELL_COOKIE_ACHIEVEMENT_AURA = 95650,
};

enum Texts
{
   EMOTE_START             = 0,
};

enum Events
{
   EVENT_THROW_FOOD        = 1,
   EVENT_FIRE,
   EVENT_ENRAGE,
};

enum npcs
{
    NPC_CAULDRON = 47754,
    NPC_BABY_MURLOCK = 48672,
};

enum points
{
    POINT_CAULDRON = 42,
};

class boss_captain_cookie : public CreatureScript
{
    public:
        boss_captain_cookie() : CreatureScript("boss_captain_cookie") {}

    struct boss_captain_cookieAI : public BossAI
    {
        boss_captain_cookieAI(Creature * creature) : BossAI(creature, BOSS_CAPTAIN_COOKIE)
        {
            me->ApplySpellImmune(0, IMMUNITY_ID, 89734, true);
            me->ApplySpellImmune(0, IMMUNITY_ID, 89252, true);
        }

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            _isSpecial = false;
            _Reset();
        }

        void EnterCombat(Unit * /*who*/)
        {
            me->GetMotionMaster()->MovePoint(POINT_CAULDRON, -64.46f, -820.76f, 41.14f, 0.0f);
            Talk(EMOTE_START);
            _EnterCombat();
            DoCastAOE(SPELL_COOKIE_ACHIEVEMENT_AURA);
        }

        void JustDied(Unit * /*killer*/)
        {
            if (Creature *paper = me->FindNearestCreature(NPC_VANESSA_PAPER, 50.0f))
                paper->AI()->Talk(0);
            _JustDied();
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;
            if (id == POINT_CAULDRON)
            {
                me->SetReactState(REACT_AGGRESSIVE);
                events.ScheduleEvent(EVENT_ENRAGE, 180000);
                events.ScheduleEvent(EVENT_THROW_FOOD, 5000);
                events.ScheduleEvent(EVENT_FIRE, 10000);
                DoCast(SPELL_SUMMON_CAULDRON);
            }
        }

        void JustSummoned(Creature *summon)
        {
            switch (summon->GetEntry())
            {
                case NPC_CAULDRON:
                    summon->CastSpell(summon, SPELL_CAULDRON_FIRE, true);
                    summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE);
                    break;
                case NPC_BABY_MURLOCK: // wtf ?
                    summon->CastSpell(summon, SPELL_BABY_MURLOC_DANCE, true);
                    break;
                default:
                    break;
            }
            BossAI::JustSummoned(summon);
        }

        uint32 GetRandomFoodSpell()
        {
            if (_isSpecial)
                return SPELL_THROW_MURLOC;
            switch (urand(0, 11))
            {
                case 0:
                    return SPELL_THROW_FOOD1;
                case 1:
                    return SPELL_THROW_FOOD2;
                case 2:
                    return SPELL_THROW_FOOD3;
                case 3:
                    return SPELL_THROW_FOOD4;
                case 4:
                    return SPELL_THROW_FOOD5;
                case 5:
                    return SPELL_THROW_FOOD6;
                case 6:
                    return SPELL_THROW_FOOD7;
                case 7:
                    return SPELL_THROW_FOOD8;
                case 8:
                    return SPELL_THROW_FOOD9;
                case 9:
                    return SPELL_THROW_FOOD10;
                case 10:
                    return SPELL_THROW_FOOD11;
                case 11:
                    return SPELL_THROW_FOOD12;
                default:
                    return SPELL_THROW_FOOD1;
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
            {
                summons.DespawnAll();
                return;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_THROW_FOOD:
                    {
                        DoCastRandom(GetRandomFoodSpell(), 0.0f);
                        events.ScheduleEvent(EVENT_THROW_FOOD, 2000);
                        break;
                    }
                    case EVENT_ENRAGE:
                        _isSpecial = true;
                        break;
                }
            }
        }

        private :
            bool _isSpecial;

    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new boss_captain_cookieAI(creature);
    }
};

class npc_food : public CreatureScript
{
public:
    npc_food() : CreatureScript("npc_food") {}

    struct npc_foodAI : public PassiveAI
    {
        npc_foodAI(Creature* pCreature) : PassiveAI(pCreature)
        {
            pInstance = pCreature->GetInstanceScript();
            me->ApplySpellImmune(0, IMMUNITY_ID, 89252, true);
        }

        void Reset()
        {
            if (IsRottenFood())
                me->AddAura(SPELL_ROTTEN_AURA, me);
            else
                me->AddAura(SPELL_GOOD_FOOD_AURA, me);
        }

        void IsSummonedBy(Unit* /*who*/)
        {
            if (Creature *creature = Unit::GetCreature(*me, pInstance->GetData64(BOSS_CAPTAIN_COOKIE)))
                creature->AI()->JustSummoned(me);
        }

        bool IsRottenFood()
        {
            switch (me->GetEntry())
            {
                case 48302:
                case 48295:
                case 48276:
                case 48299:
                case 48293:
                case 48298:
                    return true;
            }
            return false;
        }

        void OnSpellClick(Unit* player, bool& result)
        {
            if (pInstance)
            {
                if (Aura* grow = player->GetAura(pInstance->instance->IsHeroic() ? 92066 : 89732))
                {
                    if (grow->GetStackAmount() > 1)
                        player->RemoveAura(SPELL_COOKIE_ACHIEVEMENT_AURA);

                    if (!IsRottenFood())
                        grow->ModStackAmount(-1);
                }
            }
            me->DespawnOrUnsummon(100);
        }

    private:
        InstanceScript *pInstance;
    };

    CreatureAI* GetAI(Creature *pCreature) const
    {
        return new npc_foodAI(pCreature);
    }
};

class npc_cc_baby_murlock : public CreatureScript
{
public:
    npc_cc_baby_murlock() : CreatureScript("npc_cc_baby_murlock") {}

    struct npc_cc_baby_murlockAI : public PassiveAI
    {
        npc_cc_baby_murlockAI(Creature* pCreature) : PassiveAI(pCreature)
        {
            pInstance = pCreature->GetInstanceScript();
        }

        void Reset()
        {
        }

        void IsSummonedBy(Unit* /*who*/)
        {
            if (pInstance)
                if (Creature *creature = Unit::GetCreature(*me, pInstance->GetData64(BOSS_CAPTAIN_COOKIE)))
                    creature->AI()->JustSummoned(me);
        }
    private:
        InstanceScript *pInstance;
    };

    CreatureAI* GetAI(Creature *pCreature) const
    {
        return new npc_cc_baby_murlockAI(pCreature);
    }
};

void AddSC_boss_captain_cookie()
{
    new boss_captain_cookie();
    new npc_food();
    new npc_cc_baby_murlock();
}
