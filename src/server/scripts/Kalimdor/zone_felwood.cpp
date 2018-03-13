/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Felwood
SD%Complete: 95
SDComment: Quest support:
SDCategory: Felwood
EndScriptData */

/* ContentData
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"

/*#####
# Spell 88665 "Ruumbo's Silly Dance" scripted for quest 27995 "Dance for Ruumbo!"
#####*/

enum RuumbosSillyDance
{
	NPC_DRIZZLE = 47556,
	NPC_FERLI = 47558,
	MAP_KALIMDOR = 1
};

Position const DrizzleSpawnPos = { 3852.52f, -1321.92f, 213.3353f, 5.72468f };
Position const FerliSpawnPos = { 3850.44f, -1323.34f, 213.2113f, 5.637414f };

class spell_ruumbos_silly_dance : public SpellScriptLoader
{
public: spell_ruumbos_silly_dance() : SpellScriptLoader("spell_ruumbos_silly_dance") { }

		class spell_ruumbos_silly_dance_SpellScript : public SpellScript
		{
			PrepareSpellScript(spell_ruumbos_silly_dance_SpellScript);

			void HandleScriptEffect(SpellEffIndex /* effIndex */)
			{
				if (Player* player = GetHitPlayer())
				{
					player->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_DANCE);

					if (player->GetMap()->GetId() == MAP_KALIMDOR)
					{
						player->SummonCreature(NPC_DRIZZLE, DrizzleSpawnPos, TEMPSUMMON_TIMED_DESPAWN, 20000);
						player->SummonCreature(NPC_FERLI, FerliSpawnPos, TEMPSUMMON_TIMED_DESPAWN, 20000);
					}
				}
			}

			void Register() override
			{
				OnEffectHitTarget += SpellEffectFn(spell_ruumbos_silly_dance_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
			}
		};

		SpellScript* GetSpellScript() const override
		{
			return new spell_ruumbos_silly_dance_SpellScript();
		}
};


void AddSC_felwood()
{
	new spell_ruumbos_silly_dance();
}
