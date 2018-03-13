/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "culling_of_stratholme.h"

enum Spells
{
    SPELL_CORRUPTING_BLIGHT                     = 60588,
    SPELL_VOID_STRIKE                           = 60590
};

enum Yells
{
    SAY_AGGRO                                   = 0,
    SAY_DEATH                                   = 1,
    SAY_FAIL                                    = 2
};

class boss_infinite_corruptor : public CreatureScript
{
public:
    boss_infinite_corruptor() : CreatureScript("boss_infinite_corruptor") { }

    struct boss_infinite_corruptorAI : public ScriptedAI
    {
        boss_infinite_corruptorAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }


        void Reset()
        {
            uiEscapeTimer = 2000;
            uiBlightTimer = urand(7000, 9000);
            uiVoidStrikeTimer = urand(6000, 10000);
            if (instance)
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CORRUPTING_BLIGHT);
        }

        void EnterCombat(Unit* /*who*/)
        {
            Talk(SAY_AGGRO);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (uiBlightTimer < diff)
            {
                DoCastRandom(SPELL_CORRUPTING_BLIGHT, 100.0f);
                uiBlightTimer = urand(7000, 9000);
            }
            else
                uiBlightTimer -= diff;

            if (uiVoidStrikeTimer < diff)
            {
                DoCastVictim(SPELL_VOID_STRIKE, false);
                uiVoidStrikeTimer = urand(6000, 10000);
            }
            else
                uiVoidStrikeTimer -= diff;

            DoMeleeAttackIfReady();
        }

        void JustDied(Unit* /*killer*/)
        {
            Talk(SAY_DEATH);
            if (instance)
            {
                instance->SetData(DATA_INFINITE_EVENT, DONE);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CORRUPTING_BLIGHT);
            }
        }

    private:
        InstanceScript* instance;
        uint32 uiBlightTimer;
        uint32 uiEscapeTimer;
        uint32 uiVoidStrikeTimer;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_infinite_corruptorAI(creature);
    }
};

void AddSC_boss_infinite_corruptor()
{
    new boss_infinite_corruptor();
}
