/*
 * Copyright (C) 2013-2014 OMFG.GG Network <http://www.omfg.gg/>
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
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

#include "BattlefieldMgr.h"
#include "BattlefieldTB.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "SpellScript.h"

enum eTBqueuenpctext
{
    TB_NPCQUEUE_TEXT_H_NOWAR       = 110006,
    TB_NPCQUEUE_TEXT_H_QUEUE       = 110002,
    TB_NPCQUEUE_TEXT_H_WAR         = 110005,
    TB_NPCQUEUE_TEXT_A_NOWAR       = 110003,
    TB_NPCQUEUE_TEXT_A_QUEUE       = 110001,
    TB_NPCQUEUE_TEXT_A_WAR         = 110004,
    TB_NPCQUEUE_TEXTOPTION_JOIN    = -1732009,
};

enum Events
{
    EVENT_CHECK_NEAR_TOWER  = 1,
};

enum GameObjectEntrys
{
    GO_TOWER_WEST   = 204588,
    GO_TOWER_SOUTH  = 204590,
    GO_TOWER_EAST   = 204589
};

#define NPC_TB_VEHICLE 45344
#define ACTION_REWARD_ACHIEVEMENT   1

class npc_tb_spiritguide : public CreatureScript
{
public:
    npc_tb_spiritguide() : CreatureScript("npc_tb_spiritguide") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        BattlefieldTB* BfTB = (BattlefieldTB*)sBattlefieldMgr->GetBattlefieldByBattleId(BATTLEFIELD_BATTLEID_TB);
        if (BfTB)
        {
            GraveyardVect gy = BfTB->GetGraveyardVector();
            for (uint8 i = 0; i<gy.size(); i++)
            {
                if (gy[i]->GetControlTeamId() == player->GetTeamId())
                {
                   player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, sObjectMgr->GetTrinityStringForDBCLocale(((BfGraveYardTB*)gy[i])->GetTextId()), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+i);
                }
            }
        }

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
    {
        player->CLOSE_GOSSIP_MENU();

        BattlefieldTB* BfTB = (BattlefieldTB*)sBattlefieldMgr->GetBattlefieldByBattleId(BATTLEFIELD_BATTLEID_TB);
        if (BfTB)
        {
            GraveyardVect gy = BfTB->GetGraveyardVector();
            for (uint8 i = 0; i<gy.size(); i++)
            {
                if (action-GOSSIP_ACTION_INFO_DEF == i && gy[i]->GetControlTeamId() == player->GetTeamId())
                {
                    const WorldSafeLocsEntry* ws = sWorldSafeLocsStore.LookupEntry(gy[i]->GetGraveyardId());
                    player->TeleportTo(ws->map_id, ws->x, ws->y, ws->z, 0);
                }
            }
        }
        return true;
    }
};

class npc_tol_barad_battlemage : public CreatureScript
{
    public:
    npc_tol_barad_battlemage() : CreatureScript("npc_tol_barad_battlemage") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        BattlefieldTB* BfTB = (BattlefieldTB*)sBattlefieldMgr->GetBattlefieldByBattleId(BATTLEFIELD_BATTLEID_TB);
        if (BfTB)
        {
            if (BfTB->IsWarTime())
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, sObjectMgr->GetTrinityStringForDBCLocale(TB_NPCQUEUE_TEXTOPTION_JOIN), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
                player->SEND_GOSSIP_MENU(BfTB->GetDefenderTeam() ? TB_NPCQUEUE_TEXT_H_WAR : TB_NPCQUEUE_TEXT_A_WAR, creature->GetGUID());
            }
            else
            {
                uint32 uiTime = BfTB->GetTimer()/1000;
                player->SendUpdateWorldState(5332, time(NULL)+uiTime);
                if (uiTime < 15 * MINUTE)
                {
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, sObjectMgr->GetTrinityStringForDBCLocale(TB_NPCQUEUE_TEXTOPTION_JOIN), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
                    player->SEND_GOSSIP_MENU(BfTB->GetDefenderTeam() ? TB_NPCQUEUE_TEXT_H_QUEUE : TB_NPCQUEUE_TEXT_A_QUEUE, creature->GetGUID());
                }
                else
                    player->SEND_GOSSIP_MENU(BfTB->GetDefenderTeam() ? TB_NPCQUEUE_TEXT_H_NOWAR:TB_NPCQUEUE_TEXT_A_NOWAR, creature->GetGUID());
            }
        }
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 /*action*/)
    {
        player->CLOSE_GOSSIP_MENU();

        if (Battlefield *BfTB = sBattlefieldMgr->GetBattlefieldByBattleId(BATTLEFIELD_BATTLEID_TB))
            if (BfTB->CanQueue() || BfTB->IsWarTime())
                BfTB->InvitePlayerToQueue(player);
        return true;
    }
};

#define BATTLEFIELD_BATTLEID_TB 21
enum vehSpells
{
    SPELL_THICK_LAYER_OF_RUST                   = 95330,
    SPELL_LEAVE_SIEGE_MODE                      = 85078,
    SPELL_RIDE_TOL_BARAD_VEHICLE                = 84754,
    SPELL_SIEGE_CANNON_PERIODIC                 = 85167,
    SPELL_SIEGE_CANNON                          = 85122,
    SPELL_SIEGE_CANNON_MISSILE                  = 85123,
    SPELL_DEPLOY_SIEGE_MODE                     = 84974,
    SPELL_TOWER_RANGE_FINDER                    = 84979,
};

#define NPC_SIEGE_ENGINE_TURRET 45564
#define SIEGE_TURRET_SEAT_ID 7

class npc_tol_barad_vehicle : public CreatureScript
{
public:
    npc_tol_barad_vehicle() : CreatureScript("npc_tol_barad_vehicle") { }

    struct npc_tol_barad_vehicleAI : public ScriptedAI
    {
        npc_tol_barad_vehicleAI(Creature* creature) : ScriptedAI(creature) {}

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}
        void OnCharmed(bool /*apply*/) {}
        void OnSpellClick(Unit* player, bool& result) {}
        void JustDied(Unit* /*killer*/) {}

        void Reset()
        {
            playerGUID = 0;
            me->CastSpell(me, SPELL_THICK_LAYER_OF_RUST, true);
            events.Reset();
            events.ScheduleEvent(EVENT_CHECK_NEAR_TOWER, 2000);
        }

        void DoAction(int32 const action) override
        {
            if (action == ACTION_REWARD_ACHIEVEMENT)
                if (Player* player = ObjectAccessor::GetPlayer(*me, playerGUID))
                    if (AchievementEntry const* TowerPlower = sAchievementStore.LookupEntry(5415))
                        player->CompletedAchievement(TowerPlower);
        }

        void PassengerBoarded(Unit* who, int8 seatId, bool apply) override
        {
            BattlefieldTB *tb = (BattlefieldTB*)sBattlefieldMgr->GetBattlefieldByBattleId(BATTLEFIELD_BATTLEID_TB);
            if (!tb)
                return;
            if (apply)
            {
                if (who->GetTypeId() == TYPEID_PLAYER)
                {
                    if (seatId == SIEGE_TURRET_SEAT_ID || !who->HasAura(SPELL_VETERAN) || me->HasAura(SPELL_DEPLOY_SIEGE_MODE))
                        who->ExitVehicle();
                    else
                    {
                        playerGUID = who->GetGUID();
                        me->RemoveAurasDueToSpell(SPELL_THICK_LAYER_OF_RUST);
                    }

                }
                else if (who->GetEntry() == NPC_SIEGE_ENGINE_TURRET)
                    who->setFaction(tb->GetAttackerTeam() == TEAM_HORDE ? 85 : 12);
            }
            else
            {
                if (who->GetTypeId() == TYPEID_PLAYER)
                    me->setFaction(tb->GetAttackerTeam() == TEAM_HORDE ? 85 : 12);
                else if (who->GetEntry() == NPC_SIEGE_ENGINE_TURRET)
                    me->CastSpell(me, SPELL_LEAVE_SIEGE_MODE, true);
            }
        }

        void SpellHit(Unit* /*pUnit*/, SpellInfo const* spell)
        {
            if (spell->Id == SPELL_DEPLOY_SIEGE_MODE)
                me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            else if (spell->Id == SPELL_LEAVE_SIEGE_MODE)
            {
                me->RemoveAurasDueToSpell(SPELL_DEPLOY_SIEGE_MODE);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);
            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CHECK_NEAR_TOWER:
                    {
                        std::list<WorldObject*> targetList;
                        Trinity::AllWorldObjectsInRange objects(me, 100.00f);
                        Trinity::WorldObjectListSearcher<Trinity::AllWorldObjectsInRange> searcher(me, targetList, objects, GRID_MAP_TYPE_MASK_GAMEOBJECT);
                        me->VisitNearbyObject(100.00f, searcher);
                        targetList.remove_if([](WorldObject* target){
                            return target->GetEntry() != GO_TOWER_WEST && target->GetEntry() != GO_TOWER_EAST && target->GetEntry() != GO_TOWER_SOUTH;
                        });

                        for (WorldObject* target : targetList)
                            if (target->ToGameObject()->GetDestructibleState() != GO_DESTRUCTIBLE_DESTROYED)
                                DoCast(me, SPELL_TOWER_RANGE_FINDER, true);
                        events.ScheduleEvent(EVENT_CHECK_NEAR_TOWER, 2000);
                        break;
                    }
                    default:
                        break;
                }
            }
        }

    private:
        uint64 playerGUID;
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_tol_barad_vehicleAI (creature);
    }
};

class npc_tol_barad_vehicle_turret : public CreatureScript
{
public:
    npc_tol_barad_vehicle_turret() : CreatureScript("npc_tol_barad_vehicle_turret") { }

    struct npc_tol_barad_vehicle_turretAI : public ScriptedAI
    {
        npc_tol_barad_vehicle_turretAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void Reset()
        {
            check_vehicle_timer = 2000;
        }

        void IsSummonedBy(Unit* summoner)
        {
            if (summoner->HasAura(SPELL_DEPLOY_SIEGE_MODE))
                me->DespawnOrUnsummon();
            else
            {
                me->CastSpell(me, SPELL_SIEGE_CANNON_PERIODIC, true);
                me->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, SIEGE_TURRET_SEAT_ID + 1, summoner, false);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (check_vehicle_timer <= diff)
            {
                if (!me->GetVehicle())
                    me->DespawnOrUnsummon();
                check_vehicle_timer = 2000;
            }
            else check_vehicle_timer -= diff;
        }

    private:
        uint32 check_vehicle_timer;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_tol_barad_vehicle_turretAI(creature);
    }
};

class spell_tol_barad_turret_periodic : public SpellScriptLoader
{
public:
    spell_tol_barad_turret_periodic() : SpellScriptLoader("spell_tol_barad_turret_periodic") { }

    class spell_tol_barad_turret_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_tol_barad_turret_periodic_AuraScript);

        bool LaunchEffect(Unit *caster, GameObject *go)
        {
            if (!go)
                return false;
            if (go->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_DESTROYED))
                return false;
            else if (Creature* missileTarget = caster->SummonCreature(36171, go->GetPositionX(), go->GetPositionY(), go->GetPositionZ() + 50, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 2000))
            {
                if (go->GetGOValue() && go->GetGOValue()->Building.Health <= 1500)
                {
                    if (GetTarget()->GetVehicleBase())
                        GetTarget()->GetVehicleBase()->ToCreature()->AI()->DoAction(ACTION_REWARD_ACHIEVEMENT);
                }
                caster->CastSpell(missileTarget, SPELL_SIEGE_CANNON_MISSILE, true);
            }
            return true;
        }

        void HandlePeriodic(AuraEffect const* aurEff)
        {
            PreventDefaultAction();
            if (Unit *caster = GetCaster())
            {
                bool validPosition = false;
                validPosition = LaunchEffect(caster, caster->FindNearestGameObject(GO_TOWER_WEST, 200));
                if (!validPosition)
                    validPosition = LaunchEffect(caster, caster->FindNearestGameObject(GO_TOWER_SOUTH, 200));
                if (!validPosition)
                    validPosition = LaunchEffect(caster, caster->FindNearestGameObject(GO_TOWER_EAST, 200));
                if (!validPosition)
                    caster->ExitVehicle();
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_tol_barad_turret_periodic_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_tol_barad_turret_periodic_AuraScript();
    }
};

class spell_eject_passenger_1 : public SpellScriptLoader
{
public:
    spell_eject_passenger_1() : SpellScriptLoader("spell_eject_passenger_1") { }

    class spell_eject_passenger_1_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_eject_passenger_1_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            return true;
        }

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            if (Unit *caster = GetCaster())
                if (Vehicle *veh = caster->GetVehicleKit())
                    if (Unit* target = veh->GetPassenger(0)) // player seat
                        target->ExitVehicle();
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_eject_passenger_1_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_eject_passenger_1_SpellScript();
    }
};

class spell_tol_barad_missile_damage : public SpellScriptLoader
{
public:
    spell_tol_barad_missile_damage() : SpellScriptLoader("spell_tol_barad_missile_damage") { }

    class spell_tol_barad_missile_damage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_tol_barad_missile_damage_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.clear();
            if (Unit *caster = GetCaster())
            {
                if (GameObject * go = caster->FindNearestGameObject(GO_TOWER_WEST, 200))
                    targets.push_back(go);
                if (GameObject * go = caster->FindNearestGameObject(GO_TOWER_SOUTH, 200))
                    targets.push_back(go);
                if (GameObject * go = caster->FindNearestGameObject(GO_TOWER_EAST, 200))
                    targets.push_back(go);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_tol_barad_missile_damage_SpellScript::FilterTargets, EFFECT_0, TARGET_GAMEOBJECT_DEST_AREA);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_tol_barad_missile_damage_SpellScript();
    }
};


void AddSC_tol_barad()
{
   new npc_tol_barad_battlemage();
   new npc_tb_spiritguide();
   new npc_tol_barad_vehicle();
   new spell_eject_passenger_1();
   new npc_tol_barad_vehicle_turret();
   new spell_tol_barad_turret_periodic();
   new spell_tol_barad_missile_damage();
}
