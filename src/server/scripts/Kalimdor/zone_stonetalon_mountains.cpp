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

#include "Vehicle.h"
enum QUEST_ROCKET_RESCUE
{
	SPELL_MAGNETIC_PULL = 76847,
	SPELL_MAGNETIC_TOSS = 76872,
	SPELL_MAGNETIC_PULL_STUN = 76848,


};

class stonetalon_mountains_large_daddy_magnetic_pull : public SpellScriptLoader
{
public:
	stonetalon_mountains_large_daddy_magnetic_pull() : SpellScriptLoader("stonetalon_mountains_large_daddy_magnetic_pull") { }


	class stonetalon_mountains_large_daddy_magnetic_pull_SpellScript : public SpellScript
	{
		PrepareSpellScript(stonetalon_mountains_large_daddy_magnetic_pull_SpellScript);
		void HandleScript(SpellEffIndex effIndex)
		{

			Unit* target = GetHitUnit();
			if (target)
			{
				target->CastSpell(GetCaster(), SPELL_MAGNETIC_PULL);
				//GetCaster()->CastSpell(target, SPELL_MAGNETIC_PULL_STUN);
			}
		}


		void Register() override
		{
			OnEffectHitTarget += SpellEffectFn(stonetalon_mountains_large_daddy_magnetic_pull_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
		}
	};

	SpellScript* GetSpellScript() const override
	{
		return new stonetalon_mountains_large_daddy_magnetic_pull_SpellScript();
	}
};

class stonetalon_mountains_large_daddy_magnetic_toss : public SpellScriptLoader
{
public:
	stonetalon_mountains_large_daddy_magnetic_toss() : SpellScriptLoader("stonetalon_mountains_large_daddy_magnetic_toss") { }


	class stonetalon_mountains_large_daddy_magnetic_toss_SpellScript : public SpellScript
	{
		PrepareSpellScript(stonetalon_mountains_large_daddy_magnetic_toss_SpellScript);
		void HandleScript(SpellEffIndex effIndex)
		{
			Unit* target = GetHitUnit();
			if (!GetCaster()->IsVehicle())
				return;

			for (SeatMap::const_iterator itr = GetCaster()->GetVehicleKit()->Seats.begin(); itr != GetCaster()->GetVehicleKit()->Seats.end(); ++itr)
			{

				Unit* passenger = ObjectAccessor::GetUnit(*GetCaster(), itr->second.Passenger.Guid);
				if (passenger && passenger->GetTypeId() == TYPEID_PLAYER)
				{
					//passenger->RemoveAurasDueToSpell(SPELL_MAGNETIC_PULL_STUN);
					passenger->ExitVehicle();
					passenger->CastSpell(passenger, SPELL_MAGNETIC_TOSS);
				}

			}
		}

		void Register() override
		{
			OnEffectHitTarget += SpellEffectFn(stonetalon_mountains_large_daddy_magnetic_toss_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
		}
	};

	SpellScript* GetSpellScript() const override
	{
		return new stonetalon_mountains_large_daddy_magnetic_toss_SpellScript();
	}
};

/*######
## Quest: Rumble in the Lumber... Mill. Handling credit upon clicking the gameobject. 
######*/

class go_cage_base : public GameObjectScript
{
public:
    go_cage_base() : GameObjectScript("go_cage_base") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        go->UseDoorOrButton();
            if (Creature* prisoner = go->FindNearestCreature(41071, 5.0f, true))
                player->KilledMonsterCredit(41071, 0);
        return true;
    }
};

void AddSC_stonetalon_mountains()
{
    new stonetalon_mountains_large_daddy_magnetic_toss();
    new stonetalon_mountains_large_daddy_magnetic_pull();
    new go_cage_base();
}