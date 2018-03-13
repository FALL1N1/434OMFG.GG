/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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
SDName: Assembly of Iron encounter
SD%Complete: 60%
SDComment: Runes need DB support, chain lightning won't cast, supercharge won't cast (target error?) - it worked before during debugging.
SDCategory: Ulduar - Ulduar
EndScriptData */

#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "ulduar.h"

/*
7[17:27]    @DorianGrey: 2945, 2946 (But I'm on your side)          | 10088,10418,10419 - 10089,10420,10421
7[17:27]    @DorianGrey: 2947, 2948 (Can't do that while stunned)   | 10090,10422,10423 - 10091,10424,10425
7[17:27]    @DorianGrey: 2941, 2944 (I choose you)                  | 10084 - 10087
*/

enum AssemblySpells
{
    // Any boss
    SPELL_SUPERCHARGE                  = 61920,
    SPELL_BERSERK                      = 47008, // Hard enrage, don't know the correct ID.
    SPELL_CREDIT_MARKER                = 65195, // spell_dbc
    SPELL_IRON_BOOT_FLASK              = 58501,

    // Steelbreaker
    SPELL_HIGH_VOLTAGE_10              = 61890,
    SPELL_HIGH_VOLTAGE_25              = 63498,
    SPELL_FUSION_PUNCH_10              = 61903,
    SPELL_FUSION_PUNCH_25              = 63493,
    SPELL_STATIC_DISRUPTION_10         = 61911,
    SPELL_STATIC_DISRUPTION_CHECKED_10 = 61912, // Checked = spell works as target-check-trigger
    SPELL_STATIC_DISRUPTION_25         = 63494,
    SPELL_STATIC_DISRUPTION_CHECKED_25 = 63495,
    SPELL_OVERWHELMING_POWER_10        = 61888,
    SPELL_OVERWHELMING_POWER_25        = 64637,
    SPELL_MELTDOWN                     = 61889, // Triggered by overwhelming power
    SPELL_ELECTRICAL_CHARGE            = 61900,
    SPELL_ELECTRICAL_CHARGE_TRIGGER    = 61901,
    SPELL_ELECTRICAL_CHARGE_TRIGGERED  = 61902,

    // Runemaster Molgeim
    SPELL_SHIELD_OF_RUNES_10           = 62274,
    SPELL_SHIELD_OF_RUNES_10_BUFF      = 62277,
    SPELL_SHIELD_OF_RUNES_25           = 63489,
    SPELL_SHIELD_OF_RUNES_25_BUFF      = 63967,
    SPELL_SUMMON_RUNE_OF_POWER         = 63513,
    SPELL_RUNE_OF_POWER                = 61974,
    SPELL_RUNE_OF_DEATH_10             = 62269,
    SPELL_RUNE_OF_DEATH_25             = 63490,
    SPELL_RUNE_OF_SUMMONING            = 62273, // This is the spell that summons the rune
    SPELL_RUNE_OF_SUMMONING_VIS        = 62019, // Visual
    SPELL_RUNE_OF_SUMMONING_SUMMON     = 62020, // Spell that summons
    SPELL_LIGHTNING_ELEMENTAL_PASSIVE  = 62052, // Basic spell
    SPELL_LIGHTNING_BLAST_10           = 62054, // Triggered by SPELL_LIGHTNING_BLAST
    SPELL_LIGHTNING_BLAST_25           = 63491,

    // Stormcaller Brundir
    SPELL_CHAIN_LIGHTNING_10           = 61879,
    SPELL_CHAIN_LIGHTNING_25           = 63479,
    SPELL_OVERLOAD_10                  = 61869,
    SPELL_OVERLOAD_25                  = 63481,
    SPELL_LIGHTNING_WHIRL_10           = 61915,
    SPELL_LIGHTNING_WHIRL_25           = 63483,
    SPELL_LIGHTNING_WHIRL_DMG_10       = 61916,   // Periodic damage by spell above
    SPELL_LIGHTNING_WHIRL_DMG_25       = 63482,
    SPELL_LIGHTNING_TENDRILS_10        = 61887,
    SPELL_LIGHTNING_TENDRILS_25        = 63486,
    SPELL_LIGHTNING_TENDRILS_VISUAL    = 61883,
    SPELL_STORMSHIELD                  = 64187,
};

// Steelbreaker
#define SPELL_HIGH_VOLTAGE RAID_MODE(SPELL_HIGH_VOLTAGE_10, SPELL_HIGH_VOLTAGE_25)
#define SPELL_FUSION_PUNCH RAID_MODE(SPELL_FUSION_PUNCH_10, SPELL_FUSION_PUNCH_25)
#define SPELL_STATIC_DISRUPTION RAID_MODE(SPELL_STATIC_DISRUPTION_10, SPELL_STATIC_DISRUPTION_25)
#define SPELL_OVERWHELMING_POWER RAID_MODE(SPELL_OVERWHELMING_POWER_10, SPELL_OVERWHELMING_POWER_25)

// Molgeim
#define SPELL_SHIELD_OF_RUNES RAID_MODE(SPELL_SHIELD_OF_RUNES_10, SPELL_SHIELD_OF_RUNES_25)
#define SPELL_SHIELD_OF_RUNES_BUFF RAID_MODE(SPELL_SHIELD_OF_RUNES_10_BUFF, SPELL_SHIELD_OF_RUNES_25_BUFF)
#define SPELL_LIGHTNING_BLAST RAID_MODE(SPELL_LIGHTNING_BLAST_10, SPELL_LIGHTNING_BLAST_25)
#define SPELL_RUNE_OF_DEATH RAID_MODE(SPELL_RUNE_OF_DEATH_10, SPELL_RUNE_OF_DEATH_25)

// Brundir
#define SPELL_CHAIN_LIGHTNING RAID_MODE(SPELL_CHAIN_LIGHTNING_10, SPELL_CHAIN_LIGHTNING_25)
#define SPELL_OVERLOAD RAID_MODE(SPELL_OVERLOAD_10, SPELL_OVERLOAD_25)
#define SPELL_LIGHTNING_WHIRL RAID_MODE(SPELL_LIGHTNING_WHIRL_10, SPELL_LIGHTNING_WHIRL_25)
#define SPELL_LIGHTNING_WHIRL_DMG RAID_MODE(SPELL_LIGHTNING_WHIRL_DMG_10, SPELL_LIGHTNING_WHIRL_DMG_25)
#define SPELL_LIGHTNING_TENDRILS RAID_MODE(SPELL_LIGHTNING_TENDRILS_10, SPELL_LIGHTNING_TENDRILS_25)

enum AssemblyEvents
{
    // General
    EVENT_ENRAGE = 1,

    // Steelbreaker
    EVENT_FUSION_PUNCH,
    EVENT_STATIC_DISRUPTION,
    EVENT_OVERWHELMING_POWER,

    // Molgeim
    EVENT_RUNE_OF_POWER,
    EVENT_SHIELD_OF_RUNES,
    EVENT_RUNE_OF_DEATH,
    EVENT_RUNE_OF_SUMMONING,
    EVENT_LIGHTNING_BLAST,

    // Brundir
    EVENT_CHAIN_LIGHTNING,
    EVENT_OVERLOAD,
    EVENT_LIGHTNING_WHIRL,
    EVENT_LIGHTNING_TENDRILS_START,
    EVENT_LIGHTNING_TENDRILS_START_2,
    EVENT_STORMSHIELD,
    EVENT_THREAT_WIPE,
    EVENT_LIGHTNING_TENDRILS_FLIGHT_NEW_TARGET,
    EVENT_LIGHTNING_TENDRILS_FLIGHT_UPDATE_TARGET,
    EVENT_LIGHTNING_TENDRILS_ENDFLIGHT,
    EVENT_LIGHTNING_TENDRILS_GROUND,
    EVENT_LIGHTNING_TENDRILS_LAND,
    EVENT_MOVE_POSITION,
};

enum AssemblyActions
{
    ACTION_ADD_CHARGE           = 3,
    ACTION_UPDATEPHASE          = 4,
};

enum AssemblyYells
{
    SAY_STEELBREAKER_AGGRO   = 0,
    SAY_STEELBREAKER_SLAY    = 1,
    SAY_STEELBREAKER_POWER   = 2,
    SAY_STEELBREAKER_DEATH   = 3,
    SAY_STEELBREAKER_BERSERK = 4,

    SAY_MOLGEIM_AGGRO        = 0,
    SAY_MOLGEIM_SLAY         = 1,
    SAY_MOLGEIM_RUNE_DEATH   = 2,
    SAY_MOLGEIM_SUMMON       = 3,
    SAY_MOLGEIM_DEATH        = 4,
    SAY_MOLGEIM_BERSERK      = 5,

    SAY_BRUNDIR_AGGRO        = 0,
    SAY_BRUNDIR_SLAY         = 1,
    SAY_BRUNDIR_SPECIAL      = 2,
    SAY_BRUNDIR_FLIGHT       = 3,
    SAY_BRUNDIR_DEATH        = 4,
    SAY_BRUNDIR_BERSERK      = 5,
};

enum AssemblyNPCs
{
    NPC_WORLD_TRIGGER = 22515,
};

enum MovePoints
{
    POINT_FLY = 1,
    POINT_LAND,
    POINT_CHASE
};

enum Data
{
    DATA_I_CHOOSE_YOU_PHASE_CHECK = 1,
    DATA_CANT_DO_THAT_WHILE_STUNNED
};

#define EMOTE_OVERLOAD "Stormcaller Brundir begins to Overload!" // Move it to DB
#define FLOOR_Z        427.28f
#define FINAL_FLIGHT_Z 435.0f

void PostEncounterStuff(InstanceScript* inst)
{
    if (!inst)
      return;


    // Note: Sometimes, the players remain infight, even with the whole Assembly of Iron finished.
    // Seems that this occurs due to temporarily spawned triggers which despawn during fight or afterwards.
    // While a SEND_MSG_ATTACKSTART is sent once they spawn (e.g., for Rune of Power), its ATTACKSTOP never occurs in the server-logs.
    // Thus, I will force a combat-stop here.

    const Map::PlayerList& players = inst->instance->GetPlayers();

    for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
    {
        if (Player* p = it->getSource())
            p->CombatStop(true);
    }
    // Open door to Kologarn
    inst->HandleGameObject(inst->GetData64(GO_KOLOGARN_DOOR), true);
}

bool IsEncounterComplete(InstanceScript* instance, Creature* me)
{
    if (!instance || !me)
        return false;

    if (Creature* boss = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_STEELBREAKER)))
        if (boss->isAlive())
            return false;

    if (Creature* boss = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_BRUNDIR)))
        if (boss->isAlive())
            return false;

    if (Creature* boss = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MOLGEIM)))
        if (boss->isAlive())
            return false;

    return true;
}

void RespawnEncounter(InstanceScript* instance, Creature* me)
{
    if (!instance || !me)
        return;

    if (Creature* boss = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_STEELBREAKER)))
        if (!boss->isAlive())
        {
            boss->Respawn();
            boss->GetMotionMaster()->MoveTargetedHome();
        }

    if (Creature* boss = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_BRUNDIR)))
        if (!boss->isAlive())
        {
            boss->Respawn();
            boss->GetMotionMaster()->MoveTargetedHome();
        }

    if (Creature* boss = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MOLGEIM)))
        if (!boss->isAlive())
        {
            boss->Respawn();
            boss->GetMotionMaster()->MoveTargetedHome();
        }
}

void ResetEncounter(InstanceScript* instance, Creature* me)
{
    uint64 steelbreaker = instance->GetData64(BOSS_STEELBREAKER);
    uint64 brundir = instance->GetData64(BOSS_BRUNDIR);
    uint64 molgeim = instance->GetData64(BOSS_MOLGEIM);

    // Note: We must _not_ call EnterEvadeMode for ourself, since this was already done

    if (me->GetGUID() != steelbreaker)
        if (Creature* boss = ObjectAccessor::GetCreature(*me, steelbreaker))
            if (boss->isAlive() && boss->AI() && !boss->IsInEvadeMode())
            {
                boss->AddUnitState(UNIT_STATE_EVADE);
                boss->AI()->EnterEvadeMode();
            }

    if (me->GetGUID() != brundir)
        if (Creature* boss = ObjectAccessor::GetCreature(*me, brundir))
            if (boss->isAlive() && boss->AI() && !boss->IsInEvadeMode())
            {
                boss->AddUnitState(UNIT_STATE_EVADE);
                boss->AI()->EnterEvadeMode();
            }

    if (me->GetGUID() != molgeim)
        if (Creature* boss = ObjectAccessor::GetCreature(*me, molgeim))
            if (boss->isAlive() && boss->AI() && !boss->IsInEvadeMode())
            {
                boss->AddUnitState(UNIT_STATE_EVADE);
                boss->AI()->EnterEvadeMode();
            }

    instance->HandleGameObject(instance->GetData64(GO_KOLOGARN_DOOR), false);
}

void StartEncounter(InstanceScript* instance, Creature* caller)
{
    if (instance->GetBossState(BOSS_ASSEMBLY_OF_IRON) == IN_PROGRESS)
        return; // Prevent recursive calls

    instance->SetBossState(BOSS_ASSEMBLY_OF_IRON, IN_PROGRESS);

    if (Creature* boss = ObjectAccessor::GetCreature(*caller, instance->GetData64(BOSS_STEELBREAKER)))
       if (boss->isAlive() && caller->GetGUID()!=boss->GetGUID()) // Avoid redundant calls
       {
           boss->ClearUnitState(UNIT_STATE_EVADE);
           boss->SetInCombatWithZone();
           boss->AI()->EnterCombat(caller->getVictim());
       }

    if (Creature* boss = ObjectAccessor::GetCreature(*caller, instance->GetData64(BOSS_BRUNDIR)))
       if (boss->isAlive() && caller->GetGUID()!=boss->GetGUID()) // Avoid redundant calls
       {
           boss->ClearUnitState(UNIT_STATE_EVADE);
           boss->SetInCombatWithZone();
           boss->AI()->EnterCombat(caller->getVictim());
       }

    if (Creature* boss = ObjectAccessor::GetCreature(*caller, instance->GetData64(BOSS_MOLGEIM)))
        if (boss->isAlive() && caller->GetGUID()!=boss->GetGUID()) // Avoid redundant calls
        {
            boss->ClearUnitState(UNIT_STATE_EVADE);
            boss->SetInCombatWithZone();
            boss->AI()->EnterCombat(caller->getVictim());
        }
}

/************************************************************************/
/*                          Steelbreaker                                */
/************************************************************************/

class boss_steelbreaker : public CreatureScript
{
    public:
        boss_steelbreaker() : CreatureScript("boss_steelbreaker") {}

        struct boss_steelbreakerAI : public BossAI
        {
            boss_steelbreakerAI(Creature* creature) : BossAI(creature, BOSS_ASSEMBLY_OF_IRON) {}

            void Reset()
            {
                _Reset();
                phase = 0;
                superChargedCnt = 0;
                me->RemoveAllAuras();
                me->RemoveLootMode(LOOT_MODE_DEFAULT);
                me->ResetLootMode();
                ResetEncounter(instance, me);
                RespawnEncounter(instance, me);
            }

            void EnterCombat(Unit* /*who*/)
            {
                me->setActive(true);
                StartEncounter(instance, me);
                Talk(SAY_STEELBREAKER_AGGRO);
                DoZoneInCombat();
                DoCast(me, SPELL_HIGH_VOLTAGE);
                events.ScheduleEvent(EVENT_ENRAGE, 900000);
                events.ScheduleEvent(EVENT_FUSION_PUNCH, 15000);
                events.ScheduleEvent(EVENT_OVERWHELMING_POWER,  urand(2000, 5000));
                DoAction(ACTION_UPDATEPHASE);
            }

            uint32 GetData(uint32 type) const
            {
                if (type == DATA_I_CHOOSE_YOU_PHASE_CHECK)
                    return (phase >= 3) ? 1 : 0;
                return 0;
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case ACTION_ADD_CHARGE:
                        DoCast(me, SPELL_ELECTRICAL_CHARGE, true);
                        break;
                    case ACTION_UPDATEPHASE:
                        phase++;
                        events.SetPhase(phase);
                        if (phase >= 2)
                            events.RescheduleEvent(EVENT_STATIC_DISRUPTION, 30000);
                        if (phase >= 3)
                        {
                            DoCast(me, SPELL_ELECTRICAL_CHARGE, true);
                            uint32 nextSchedule = 0;
                            if (events.GetNextEventTime(EVENT_STATIC_DISRUPTION) > 0)   // Note: Function returns 0 if the event isn't scheduled yet.
                                nextSchedule = urand(2000, 5000);
                            else
                                nextSchedule = urand(20000, 30000);
                            events.RescheduleEvent(EVENT_OVERWHELMING_POWER, nextSchedule);
                            //                            me->AddLootMode(LOOT_MODE_HARD_MODE_2);
                        }
                        break;
                }
            }

            void JustDied(Unit* /*who*/)
            {
                Talk(SAY_STEELBREAKER_DEATH);
                if (IsEncounterComplete(instance, me))
                {
                    _JustDied();
                    PostEncounterStuff(instance);
                    instance->DoUpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_CREDIT_MARKER);
                }
                else
                {
                    events.Reset();
                    summons.DespawnAll();
                    DoCastAOE(SPELL_SUPERCHARGE);
                }

                if (Creature* Molgeim = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MOLGEIM)))
                {
                    if (Creature* Brundir = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_BRUNDIR)))
                    {
                        if (!Molgeim->isAlive() && !Brundir->isAlive())
                        {
                            me->AddLootMode(LOOT_MODE_HARD_MODE_1);
                            instance->DoCompleteAchievement(RAID_MODE(2941, 2944));
                            instance->DoCompleteAchievement(RAID_MODE(2947, 2948));
                            AchievementEntry const* pAE = sAchievementStore.LookupEntry(RAID_MODE(2945 ,2946));
                            if (!pAE)
                                return;
                            Map::PlayerList const & playerList = me->GetMap()->GetPlayers();
                            for (Map::PlayerList::const_iterator i = playerList.begin(); i != playerList.end(); ++i)
                                if (Player* player = i->getSource())
                                    if (player->isAlive())
                                        if (player->HasAura(58501))
                                            player->CompletedAchievement(pAE);
                        }
                        else
                        {
                            me->SetLootRecipient(NULL);
                        }
                    }
                }
            }

            void KilledUnit(Unit* /*who*/)
            {
                Talk(SAY_STEELBREAKER_SLAY);

                if (phase == 3)
                    DoCast(me, SPELL_ELECTRICAL_CHARGE);
            }

            void SpellHit(Unit* /*from*/, SpellInfo const* spell)
            {
                switch (spell->Id)
                {
                case SPELL_SUPERCHARGE:
                    me->SetHealth(me->GetMaxHealth());
                    events.CancelEvent(EVENT_FUSION_PUNCH);
                    events.ScheduleEvent(EVENT_FUSION_PUNCH, 15000);
                    superChargedCnt++;
                    DoAction(ACTION_UPDATEPHASE);
                    // Crazy hack, but since - whyever - stacking does not work automatically when the casts are fired from different NPCs...
                    // Note that it also does not work if the same NPC tries to cast the spell twice (as used in last commit)
                    if (Aura* charge = me->GetAura(SPELL_SUPERCHARGE))
                        charge->SetStackAmount(std::min<uint32>(2, superChargedCnt));
                    break;
                case SPELL_ELECTRICAL_CHARGE_TRIGGERED:
                    if (!me->isInCombat())
                        me->RemoveAurasDueToSpell(SPELL_ELECTRICAL_CHARGE_TRIGGERED);
                    break;
                }
            }

            void SpellHitTarget(Unit* target, SpellInfo const* spell)
            {
                if (spell->Id == SPELL_MELTDOWN && target && target->ToCreature())
                    target->CastSpell(me, SPELL_ELECTRICAL_CHARGE_TRIGGER, true);
            }

            // try to prefer ranged targets, targets that already have the aura should be avoided; tanks are allowed
            Unit* GetDisruptionTarget()
            {
                Map* map = me->GetMap();
                if (map && map->IsDungeon())
                {
                    std::list<Player*> playerList;
                    Map::PlayerList const& Players = map->GetPlayers();
                    for (Map::PlayerList::const_iterator itr = Players.begin(); itr != Players.end(); ++itr)
                    {
                        if (Player* player = itr->getSource())
                        {
                            if (player->isDead() || player->HasAura(SPELL_STATIC_DISRUPTION) || player->isGameMaster())
                                continue;

                            float Distance = player->GetDistance(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                            if (Distance < 15.0f || Distance > 100.0f)
                                continue;

                            playerList.push_back(player);
                        }
                    }

                    if (playerList.empty())
                    {
                        Unit* sel = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true, -SPELL_STATIC_DISRUPTION);
                        if (sel)
                            if (Player* p = sel->ToPlayer())
                                playerList.push_back(p);
                            else
                                return 0;
                        else
                            return 0;
                    }

                    return Trinity::Containers::SelectRandomContainerElement(playerList);
                }
                else
                    return 0;
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;


                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ENRAGE:
                            Talk(SAY_STEELBREAKER_BERSERK);
                            DoCast(SPELL_BERSERK);
                            break;
                        case EVENT_FUSION_PUNCH:
                            if (me->IsWithinMeleeRange(me->getVictim()))
                                DoCastVictim(SPELL_FUSION_PUNCH);
                            events.ScheduleEvent(EVENT_FUSION_PUNCH, urand(13000, 22000));
                            break;
                        case EVENT_STATIC_DISRUPTION:
                            if (Unit* target = GetDisruptionTarget())
                                DoCast(target, SPELL_STATIC_DISRUPTION);
                            events.ScheduleEvent(EVENT_STATIC_DISRUPTION, urand(20000, 25000));
                            break;
                        case EVENT_OVERWHELMING_POWER:
                            if (me->getVictim() && !me->getVictim()->HasAura(SPELL_OVERWHELMING_POWER))
                            {
                                if (Creature* Molgeim = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MOLGEIM)))
                                    if (Creature* Brundir = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_BRUNDIR)))
                                        if (!Molgeim->isAlive() && !Brundir->isAlive())
                                        {
                                            Talk(SAY_STEELBREAKER_POWER);
                                            DoCastVictim(SPELL_OVERWHELMING_POWER);
                                        }
                                events.ScheduleEvent(EVENT_OVERWHELMING_POWER, RAID_MODE(60000, 35000));
                            }
                            else
                                events.ScheduleEvent(EVENT_OVERWHELMING_POWER, 2000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

            private:
                uint32 phase;
                uint32 superChargedCnt;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<boss_steelbreakerAI>(creature);
        }
};

class spell_steelbreaker_static_disruption : public SpellScriptLoader
{
public:
    spell_steelbreaker_static_disruption() : SpellScriptLoader("spell_steelbreaker_static_disruption") {}

    class spell_steelbreaker_static_disruption_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_steelbreaker_static_disruption_SpellScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_STATIC_DISRUPTION_CHECKED_10))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_STATIC_DISRUPTION_CHECKED_25))
                return false;
            return true;
        }

        void HandleTriggerMissile(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            Unit* caster = GetCaster();
            Unit* target = GetExplTargetUnit();
            if (caster && target)
            {
                uint32 id = uint32(caster->GetMap()->GetDifficulty() == RAID_DIFFICULTY_10MAN_NORMAL ? SPELL_STATIC_DISRUPTION_CHECKED_10 : SPELL_STATIC_DISRUPTION_CHECKED_25);
                caster->CastSpell(target, id, true);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_steelbreaker_static_disruption_SpellScript::HandleTriggerMissile, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_steelbreaker_static_disruption_SpellScript();
    }
};

class spell_steelbreaker_electrical_charge : public SpellScriptLoader
{
public:
    spell_steelbreaker_electrical_charge() : SpellScriptLoader("spell_steelbreaker_electrical_charge") {}

    class spell_steelbreaker_electrical_charge_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_steelbreaker_electrical_charge_AuraScript);

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            Unit* caster = GetCaster();
            if (target && target->ToPlayer() && caster && GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
                target->CastSpell(caster, GetSpellInfo()->Effects[EFFECT_0].CalcValue(), true);
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_steelbreaker_electrical_charge_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_steelbreaker_electrical_charge_AuraScript();
    }
};

/************************************************************************/
/*                              Molgeim                                 */
/************************************************************************/

class boss_runemaster_molgeim : public CreatureScript
{
    public:
        boss_runemaster_molgeim() : CreatureScript("boss_runemaster_molgeim") {}

        struct boss_runemaster_molgeimAI : public BossAI
        {
            boss_runemaster_molgeimAI(Creature* creature) : BossAI(creature, BOSS_ASSEMBLY_OF_IRON) {}

            void Reset()
            {
                _Reset();
                phase = 0;
                superChargedCnt = 0;
                me->RemoveAllAuras();
                me->RemoveLootMode(LOOT_MODE_DEFAULT);
                me->ResetLootMode();
                ResetEncounter(instance, me);
                RespawnEncounter(instance, me);
            }

            void EnterCombat(Unit* /*who*/)
            {
                me->setActive(true);
                StartEncounter(instance, me);
                Talk(SAY_MOLGEIM_AGGRO);
                DoZoneInCombat();
                events.ScheduleEvent(EVENT_ENRAGE, 900000);
                events.ScheduleEvent(EVENT_SHIELD_OF_RUNES, 30000);
                events.ScheduleEvent(EVENT_RUNE_OF_POWER, 20000);
                DoAction(ACTION_UPDATEPHASE);
            }

            uint32 GetData(uint32 type) const
            {
                if (type == DATA_I_CHOOSE_YOU_PHASE_CHECK)
                    return (phase >= 3) ? 1 : 0;
                return 0;
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case ACTION_UPDATEPHASE:
                        phase++;
                       events.SetPhase(phase);
                        if (phase >= 2)
                            events.RescheduleEvent(EVENT_RUNE_OF_DEATH, 30000);
                        if (phase >= 3)
                        {
                            events.RescheduleEvent(EVENT_RUNE_OF_SUMMONING, urand(20000, 30000));
                        }
                        break;
                }
            }

            void JustDied(Unit* /*who*/)
            {
                Talk(SAY_MOLGEIM_DEATH);
                if (IsEncounterComplete(instance, me))
                {
                    _JustDied();
                    PostEncounterStuff(instance);
                    instance->DoUpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_CREDIT_MARKER);
                }
                else
                {
                    events.Reset();
                    summons.DespawnAll();
                    DoCastAOE(SPELL_SUPERCHARGE);
                }
                if (Creature* Steelbreaker = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_STEELBREAKER)))
                {
                    if (Creature* Brundir = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_BRUNDIR)))
                    {
                        if (!Steelbreaker->isAlive() && !Brundir->isAlive())
                        {
                            instance->DoCompleteAchievement(RAID_MODE(2947, 2948));
                            me->AddLootMode(LOOT_MODE_HARD_MODE_1);
                            instance->DoCompleteAchievement(RAID_MODE(2939, 2942));
                            AchievementEntry const* pAE = sAchievementStore.LookupEntry(RAID_MODE(2945 ,2946));
                            if (!pAE)
                                return;
                            Map::PlayerList const & playerList = me->GetMap()->GetPlayers();
                            for (Map::PlayerList::const_iterator i = playerList.begin(); i != playerList.end(); ++i)
                                if (Player* player = i->getSource())
                                    if (player->isAlive())
                                        if (player->HasAura(58501))
                                            player->CompletedAchievement(pAE);
                        }
                        else
                        {
                            me->SetLootRecipient(NULL);
                        }
                    }
                }
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
            }

            void KilledUnit(Unit* /*who*/)
            {
                Talk(SAY_MOLGEIM_SLAY);
            }

            void SpellHit(Unit* /*from*/, SpellInfo const* spell)
            {
                if (spell->Id == SPELL_SUPERCHARGE)
                {
                    me->SetHealth(me->GetMaxHealth());
                    events.RescheduleEvent(EVENT_SHIELD_OF_RUNES, 27000);
                    events.RescheduleEvent(EVENT_RUNE_OF_POWER, 25000);
                    superChargedCnt++;
                    DoAction(ACTION_UPDATEPHASE);
                    // Crazy hack, but since - whyever - stacking does not work automatically when the casts are fired from different NPCs...
                    // Note that it also does not work if the same NPC tries to cast the spell twice (as used in last commit)
                    if (Aura* charge = me->GetAura(SPELL_SUPERCHARGE))
                        charge->SetStackAmount(std::min<uint32>(2, superChargedCnt));
                }
            }

            void UpdateAI(uint32 const diff)
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
                        case EVENT_ENRAGE:
                            Talk(SAY_MOLGEIM_BERSERK);
                            DoCast(SPELL_BERSERK);
                            break;
                        case EVENT_RUNE_OF_POWER:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                            {
                                if (Creature *c = me->FindNearestCreature(33705, 100))
                                    c->DespawnOrUnsummon();
                                Position pos;
                                target->GetPosition(&pos);
                                me->SummonCreature(33705, pos, TEMPSUMMON_TIMED_DESPAWN, 59000);
                            }
                            events.ScheduleEvent(EVENT_RUNE_OF_POWER, 60000);
                            break;
                        case EVENT_SHIELD_OF_RUNES:
                            DoCast(me, SPELL_SHIELD_OF_RUNES);
                            events.ScheduleEvent(EVENT_SHIELD_OF_RUNES, urand(27000, 34000));
                            break;
                        case EVENT_RUNE_OF_DEATH:
                            if (Creature* Steelbreaker = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_STEELBREAKER)))
                                if (Creature* Brundir = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_BRUNDIR)))
                                    if (!Brundir->isAlive() || !Steelbreaker->isAlive())
                                    {
                                        Talk(SAY_MOLGEIM_RUNE_DEATH);
                                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                                            DoCast(target, SPELL_RUNE_OF_DEATH);
                                    }
                            events.ScheduleEvent(EVENT_RUNE_OF_DEATH, urand(30000, 40000));
                            break;
                        case EVENT_RUNE_OF_SUMMONING:
                            Talk(SAY_MOLGEIM_SUMMON);
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                            {
                                if (Creature* Steelbreaker = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_STEELBREAKER)))
                                    if (Creature* Brundir = ObjectAccessor::GetCreature(*me, instance ? instance->GetData64(BOSS_BRUNDIR) : 0))
                                        if (!Steelbreaker->isAlive() && !Brundir->isAlive())
                                            DoCast(target, SPELL_RUNE_OF_SUMMONING);
                            }
                            events.ScheduleEvent(EVENT_RUNE_OF_SUMMONING, urand(35000, 45000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            uint32 phase;
            uint32 superChargedCnt;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<boss_runemaster_molgeimAI>(creature);
        }
};

class mob_rune_of_power : public CreatureScript
{
    public:
        mob_rune_of_power() : CreatureScript("mob_rune_of_power") {}

        struct mob_rune_of_powerAI : public Scripted_NoMovementAI
        {
            mob_rune_of_powerAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                me->SetInCombatWithZone();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->setFaction(16); // Same faction as bosses
                DoCast(SPELL_RUNE_OF_POWER);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_rune_of_powerAI(creature);
        }
};

class mob_lightning_elemental : public CreatureScript
{
    public:
        mob_lightning_elemental() : CreatureScript("mob_lightning_elemental") {}

        struct mob_lightning_elementalAI : public ScriptedAI
        {
            mob_lightning_elementalAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetInCombatWithZone();
                castDone = false;
            }

            void EnterCombat(Unit* /* target */)
            {
                DoCast(me, SPELL_LIGHTNING_ELEMENTAL_PASSIVE);      // TODO: Check if both this spell and the other one below are required
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                {
                    me->AddThreat(target, 99999.9f);
                    AttackStart(target);
                }
            }

            void UpdateAI(uint32 const /*diff*/)
            {
                if (!UpdateVictim())
                    return;
                //                if (me->IsWithinMeleeRange(me->getVictim()) && !castDone)

                if (me->GetDistance(me->getVictim()) <= 3.0f && !castDone)
                {
                    me->CastSpell(me, SPELL_LIGHTNING_BLAST, true);
                    me->DespawnOrUnsummon(500);
                    castDone = true;
                }
            }

        private:
            bool castDone;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_lightning_elementalAI(creature);
        }
};

class mob_rune_of_summoning : public CreatureScript
{
    public:
        mob_rune_of_summoning() : CreatureScript("mob_rune_of_summoning") {}

        struct mob_rune_of_summoningAI : public Scripted_NoMovementAI
        {
            mob_rune_of_summoningAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->AddAura(SPELL_RUNE_OF_SUMMONING_VIS, me);
                summonCount = 0;
                summonTimer = 2000;
                instance = creature->GetInstanceScript();
            }

            void JustSummoned(Creature* summon)
            {
                if (Creature* Molgeim = ObjectAccessor::GetCreature(*me, instance ? instance->GetData64(BOSS_MOLGEIM) : 0))
                    Molgeim->AI()->JustSummoned(summon);    // Move ownership, take care that the spawned summon does not know about this
            }

            void UpdateAI(uint32 const diff)
            {
                if (summonTimer <= diff)
                    SummonLightningElemental();
                else
                    summonTimer -= diff;
            }

            void SummonLightningElemental()
            {
                me->CastSpell(me, SPELL_RUNE_OF_SUMMONING_SUMMON, false);   // Spell summons 32958
                if (++summonCount == 10)                        // TODO: Find out if this amount is right
                    me->DespawnOrUnsummon();
                else
                    summonTimer = 2000;                         // TODO: Find out of timer is right
            }

            private:
                InstanceScript* instance;
                uint32 summonCount;
                uint32 summonTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_rune_of_summoningAI(creature);
        }
};

/************************************************************************/

/*                             Brundir                                  */
/************************************************************************/

class boss_stormcaller_brundir : public CreatureScript
{
    public:
        boss_stormcaller_brundir() : CreatureScript("boss_stormcaller_brundir") {}

        struct boss_stormcaller_brundirAI : public BossAI
        {
            boss_stormcaller_brundirAI(Creature* creature) : BossAI(creature, BOSS_ASSEMBLY_OF_IRON) {}

            void Reset()
            {
                _Reset();
                phase = 0;
                superChargedCnt = 0;
                tendrilTarget = 0;
                forceLand = false;
                couldNotDoThat = true;
                me->RemoveAllAuras();
                me->RemoveLootMode(LOOT_MODE_DEFAULT);
                me->ResetLootMode();
                me->SetSpeed(MOVE_RUN, 1.12857f);

                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, false);  // Should be interruptible unless overridden by spell (Overload)
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, false);       // Reset immunity, Brundir can be stunned by default
                ResetEncounter(instance, me);
                RespawnEncounter(instance, me);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetHover(false);
            }

            void EnterCombat(Unit* /*who*/)
            {
                me->setActive(true);
                StartEncounter(instance, me);
                Talk(SAY_BRUNDIR_AGGRO);
                DoZoneInCombat();
                events.ScheduleEvent(EVENT_MOVE_POSITION, 1000);
                events.ScheduleEvent(EVENT_ENRAGE, 900000);
                events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, 4000);
                events.ScheduleEvent(EVENT_OVERLOAD, urand(60000, 120000));
                events.ScheduleEvent(EVENT_THREAT_WIPE, 10000);
                DoAction(ACTION_UPDATEPHASE);
            }

            uint32 GetData(uint32 type) const
            {
                switch (type)
                {
                    case DATA_I_CHOOSE_YOU_PHASE_CHECK:
                        return (phase >= 3) ? 1 : 0;
                    case DATA_CANT_DO_THAT_WHILE_STUNNED:
                        return couldNotDoThat ? 1 : 0;
                }

                return 0;
            }

            void SpellHitTarget(Unit* target, SpellInfo const* spell)
            {
                if (target->GetTypeId() == TYPEID_PLAYER)
                    if (couldNotDoThat)
                        switch (spell->Id)
                        {
                            case SPELL_CHAIN_LIGHTNING_10:
                            case SPELL_CHAIN_LIGHTNING_25:
                            case SPELL_LIGHTNING_WHIRL_DMG_10:
                            case SPELL_LIGHTNING_WHIRL_DMG_25:
                                couldNotDoThat = false;
                                break;
                        }
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case ACTION_UPDATEPHASE:
                        // Change internal phase. Note that the events should _only_ be scheduled if they are not.
                        phase++;
                       events.SetPhase(phase);
                        if (phase >= 2)
                            events.RescheduleEvent(EVENT_LIGHTNING_WHIRL, urand(15000, 25000));
                        if (phase >= 3)
                        {
                            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                            DoCast(me, SPELL_STORMSHIELD);
                            events.RescheduleEvent(EVENT_LIGHTNING_TENDRILS_START, urand(30000, 40000));
                        }
                        break;

                }
            }

            void JustDied(Unit* /*who*/)
            {
                Talk(SAY_BRUNDIR_DEATH);
                if (IsEncounterComplete(instance, me))
                {
                    _JustDied();
                    PostEncounterStuff(instance);
                    instance->DoUpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_CREDIT_MARKER);
                }
                else
                {
                    events.Reset();
                    summons.DespawnAll();
                    DoCastAOE(SPELL_SUPERCHARGE);
                }

                // Prevent to have Brundir somewhere in the air when he die in Air phase
                if (me->GetPositionZ() > FLOOR_Z)
                    me->GetMotionMaster()->MoveFall();

                if (Creature* Molgeim = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MOLGEIM)))
                {
                    if (Creature* Steelbreaker = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_STEELBREAKER)))
                    {
                        if (!Molgeim->isAlive() && !Steelbreaker->isAlive())
                        {
                            me->AddLootMode(LOOT_MODE_HARD_MODE_1);
                            AchievementEntry const* pAE = sAchievementStore.LookupEntry(RAID_MODE(2945 ,2946));
                            instance->DoCompleteAchievement(RAID_MODE(2940, 2943));
                            if (!pAE)
                                return;
                            Map::PlayerList const & playerList = me->GetMap()->GetPlayers();
                            for (Map::PlayerList::const_iterator i = playerList.begin(); i != playerList.end(); ++i)
                                if (Player* player = i->getSource())
                                    if (player->isAlive())
                                        if (player->HasAura(58501))
                                            player->CompletedAchievement(pAE);
                        }
                        else
                            me->SetLootRecipient(NULL);
                    }
                }
                me->SetHover(false);
            }

            void KilledUnit(Unit* /*who*/)
            {
                Talk(SAY_BRUNDIR_SLAY);
            }

            void SpellHit(Unit* /*from*/, SpellInfo const* spell)
            {
                if (spell->Id == SPELL_SUPERCHARGE)
                {
                    me->SetHealth(me->GetMaxHealth());
                    events.RescheduleEvent(EVENT_CHAIN_LIGHTNING, urand(7000, 12000));
                    events.RescheduleEvent(EVENT_OVERLOAD, urand(40000, 50000));
                    superChargedCnt++;
                    DoAction(ACTION_UPDATEPHASE);
                    // Crazy hack, but since - whyever - stacking does not work automatically when the casts are fired from different NPCs...
                    // Note that it also does not work if the same NPC tries to cast the spell twice (as used in last commit)
                    if (Aura* charge = me->GetAura(SPELL_SUPERCHARGE))
                        charge->SetStackAmount(std::min<uint32>(2, superChargedCnt));
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ENRAGE:
                            Talk(SAY_BRUNDIR_BERSERK);
                            DoCast(SPELL_BERSERK);
                            break;
                        case EVENT_CHAIN_LIGHTNING:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                                DoCast(target, SPELL_CHAIN_LIGHTNING);
                            events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, urand(7000, 10000));
                            break;
                        case EVENT_OVERLOAD:    // TODO: Check if this applies temporary interupt-immunity (as intended)
                            if (!me->HasUnitState(UNIT_STATE_STUNNED))
                            {
                                me->MonsterTextEmote(EMOTE_OVERLOAD, 0, true);
                                Talk(SAY_BRUNDIR_SPECIAL);
                                DoCast(SPELL_OVERLOAD);
                            }
                            events.ScheduleEvent(EVENT_OVERLOAD, urand(60000, 120000));
                            break;
                        case EVENT_LIGHTNING_WHIRL:
                            if (Creature* Molgeim = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MOLGEIM)))
                                if (Creature* Steelbreaker = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_STEELBREAKER)))
                                    if (!Molgeim->isAlive() || !Steelbreaker->isAlive())
                                        DoCast(SPELL_LIGHTNING_WHIRL);
                            events.ScheduleEvent(EVENT_LIGHTNING_WHIRL, urand(15000, 20000));
                            break;
                        case EVENT_THREAT_WIPE:
                            DoResetThreat();
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            {
                                me->AddThreat(target, 99999.9f);
                                me->GetMotionMaster()->MovePoint(POINT_CHASE, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
                            }
                            events.ScheduleEvent(EVENT_THREAT_WIPE, 10000);
                            break;
                        case EVENT_LIGHTNING_TENDRILS_START:
                            if (Creature* Molgeim = ObjectAccessor::GetCreature(*me, instance ? instance->GetData64(BOSS_MOLGEIM) : 0))
                                if (Creature* Steelbreaker = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_STEELBREAKER)))
                                    if (Molgeim->isAlive() || Steelbreaker->isAlive())
                                    {
                                        events.ScheduleEvent(EVENT_LIGHTNING_TENDRILS_START, 10000);
                                        break;
                                    }
                            me->SetSpeed(MOVE_RUN, 0.7f);
                            me->SetDisableGravity(false, true);
                            me->SetHover(true);
                            Talk(SAY_BRUNDIR_FLIGHT);
                            me->AttackStop();

                            me->SetReactState(REACT_PASSIVE);
                            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
                            events.DelayEvents(37000);  // Flight phase is 35 seconds, +2 as buffer
                            events.ScheduleEvent(EVENT_LIGHTNING_TENDRILS_START_2, 1500);
                            events.ScheduleEvent(EVENT_LIGHTNING_TENDRILS_FLIGHT_NEW_TARGET, 2500);
                            events.ScheduleEvent(EVENT_LIGHTNING_TENDRILS_ENDFLIGHT, 32500);
                            break;
                         case EVENT_LIGHTNING_TENDRILS_START_2:
                            DoCast(SPELL_LIGHTNING_TENDRILS);
                            DoCast(SPELL_LIGHTNING_TENDRILS_VISUAL);
                            break;
                         case EVENT_LIGHTNING_TENDRILS_FLIGHT_NEW_TARGET:
                             events.CancelEvent(EVENT_LIGHTNING_TENDRILS_FLIGHT_UPDATE_TARGET);
                             tendrilTarget = 0;
                             if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                             {
                                 me->GetMotionMaster()->MovePoint(POINT_FLY, target->GetPositionX(), target->GetPositionY(), FINAL_FLIGHT_Z);
                                 tendrilTarget = target->GetGUID();
                             }
                             events.ScheduleEvent(EVENT_LIGHTNING_TENDRILS_FLIGHT_NEW_TARGET, 6000);
                             events.ScheduleEvent(EVENT_LIGHTNING_TENDRILS_FLIGHT_UPDATE_TARGET, 250);
                             break;
                        case EVENT_LIGHTNING_TENDRILS_FLIGHT_UPDATE_TARGET:
                            if (Player* player = ObjectAccessor::GetPlayer(*me, tendrilTarget))
                            {
                                me->StopMoving();
                                me->GetMotionMaster()->Clear();
                                me->GetMotionMaster()->MoveIdle();
                                me->GetMotionMaster()->MovePoint(POINT_FLY, player->GetPositionX(), player->GetPositionY(), FINAL_FLIGHT_Z);
                            }
                            events.ScheduleEvent(EVENT_LIGHTNING_TENDRILS_FLIGHT_UPDATE_TARGET, 250);
                            break;
                        case EVENT_LIGHTNING_TENDRILS_ENDFLIGHT:
                            events.CancelEvent(EVENT_LIGHTNING_TENDRILS_FLIGHT_NEW_TARGET);
                            events.CancelEvent(EVENT_LIGHTNING_TENDRILS_FLIGHT_UPDATE_TARGET);
                            me->GetMotionMaster()->Initialize();
                            me->GetMotionMaster()->MovePoint(POINT_FLY, 1586.920166f, 119.848984f, FINAL_FLIGHT_Z);
                            events.ScheduleEvent(EVENT_LIGHTNING_TENDRILS_LAND, 4000);
                            break;
                        case EVENT_LIGHTNING_TENDRILS_LAND:
                            me->GetMotionMaster()->Initialize();
                            me->SetDisableGravity(true, true);
                            me->SetHover(false);
                            me->GetMotionMaster()->MovePoint(POINT_LAND, me->GetPositionX(), me->GetPositionY(), FLOOR_Z);
                            events.ScheduleEvent(EVENT_LIGHTNING_TENDRILS_GROUND, 2500);
                            break;
                        case EVENT_LIGHTNING_TENDRILS_GROUND:
                            me->SetSpeed(MOVE_RUN, 1.42857f);
                            me->SetHover(false);
                            me->RemoveAurasDueToSpell(SPELL_LIGHTNING_TENDRILS);
                            me->RemoveAurasDueToSpell(SPELL_LIGHTNING_TENDRILS_VISUAL);
                            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
                            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
                            DoStartMovement(me->getVictim());
                            me->getThreatManager().resetAllAggro();
                            events.ScheduleEvent(EVENT_LIGHTNING_TENDRILS_START, urand(40000, 80000));
                            break;
                        case EVENT_MOVE_POSITION:
                            if (me->IsWithinMeleeRange(me->getVictim()))
                            {
                                float x = frand(-25.0f, 25.0f);
                                float y = frand(-25.0f, 25.0f);
                                me->GetMotionMaster()->MovePoint(0, me->GetPositionX() + x, me->GetPositionY() + y, FLOOR_Z);
                                // Prevention to go outside the room or into the walls
                                if (Creature* trigger = me->FindNearestCreature(NPC_WORLD_TRIGGER, 100.0f, true))
                                    if (me->GetDistance(trigger) >= 50.0f)
                                        me->GetMotionMaster()->MovePoint(0, trigger->GetPositionX(), trigger->GetPositionY(), FLOOR_Z);
                            }
                            events.ScheduleEvent(EVENT_MOVE_POSITION, urand(7500, 10000));
                            break;
                        default:
                            break;
                    }
                }

                //if (me->GetPositionZ()>FLOOR_Z && !me->HasAura(SPELL_LIGHTNING_TENDRILS_VISUAL)) // in that case, we should not be above the floor
                //    me->GetMotionMaster()->MoveFall();

                if (!me->HasAura(SPELL_LIGHTNING_TENDRILS))
                    DoMeleeAttackIfReady();
            }

        private:
            uint32 phase;
            uint32 superChargedCnt;
            uint64 tendrilTarget;
            bool forceLand;
            bool couldNotDoThat;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<boss_stormcaller_brundirAI>(creature);
        }
};

class spell_shield_of_runes : public SpellScriptLoader
{
    public:
        spell_shield_of_runes() : SpellScriptLoader("spell_shield_of_runes") {}

        class spell_shield_of_runes_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_shield_of_runes_AuraScript);

            void OnAbsorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
            {
                uint32 damage = dmgInfo.GetDamage();

                if (absorbAmount > damage)
                    return;

                if (Unit* caster = GetCaster())
                    caster->CastSpell(caster, caster->GetMap()->GetDifficulty() == RAID_DIFFICULTY_10MAN_NORMAL ? SPELL_SHIELD_OF_RUNES_10_BUFF : SPELL_SHIELD_OF_RUNES_25_BUFF, true);
            }

            void Register()
            {
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_shield_of_runes_AuraScript::OnAbsorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_shield_of_runes_AuraScript();
        }
};

class spell_assembly_meltdown : public SpellScriptLoader
{
    public:
        spell_assembly_meltdown() : SpellScriptLoader("spell_assembly_meltdown") {}

        class spell_assembly_meltdown_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_assembly_meltdown_SpellScript);

            void HandleInstaKill(SpellEffIndex /*effIndex*/)
            {
                if (InstanceScript* instance = GetCaster()->GetInstanceScript())
                    if (Creature* Steelbreaker = ObjectAccessor::GetCreature(*GetCaster(), instance->GetData64(BOSS_STEELBREAKER)))
                        Steelbreaker->AI()->DoAction(ACTION_ADD_CHARGE);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_assembly_meltdown_SpellScript::HandleInstaKill, EFFECT_1, SPELL_EFFECT_INSTAKILL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_assembly_meltdown_SpellScript();
        }
};

//=== Supercharge
const uint32 AssemblyMembers[] =
{
    32867, // NPC_STEELBREAKER,
    32927, // NPC_RUNEMASTER_MOLGEIM,
    32857, // NPC_STORMCALLER_BRUNDIR,
};

struct IsNoAssemblyMember
{
    bool operator()(const WorldObject* target) const
    {
        if (const Creature* creature = target->ToCreature())
        {
            /* What happens here:
            State before:
                *searchStart = AssemblyMembers[0]
                *searchEnd = AssembyMemebers[3] // Never reached
            State after "find" (worst case):
                *searchStart = AssemblyMembers[3]
                *searchEnd = AssembyMemebers[3] // Never reached
            Follows the ".end() always refers to the first invalid argument" idiom
            */
            if ( std::find(AssemblyMembers, AssemblyMembers + (sizeof(AssemblyMembers) / sizeof(uint32)), creature->GetOriginalEntry()) != (AssemblyMembers + (sizeof(AssemblyMembers) / sizeof(uint32))) )
               return false;
            else
                return true;
        }
        else
            return true;
    }

    };

class achievement_i_choose_you : public AchievementCriteriaScript
{
    public:
        achievement_i_choose_you(const char* name) : AchievementCriteriaScript(name) {}

        bool OnCheck(Player* /*player*/, Unit* target)
        {
            if (!target)
                return false;

            if (Creature* boss = target->ToCreature())
                if (boss->AI()->GetData(DATA_I_CHOOSE_YOU_PHASE_CHECK))
                    return true;

            return false;
        }
};

class achievement_but_i_am_on_your_side : public AchievementCriteriaScript
{
    public:
        achievement_but_i_am_on_your_side(const char* name) : AchievementCriteriaScript(name) {}

        bool OnCheck(Player* player, Unit* target)
        {
            if (!target || !player)
                return false;

            if (Creature* boss = target->ToCreature())
                if (boss->AI()->GetData(DATA_I_CHOOSE_YOU_PHASE_CHECK) && player->HasAura(SPELL_IRON_BOOT_FLASK))
                    return true;

            return false;
        }
};

class achievement_cant_do_that_while_stunned : public AchievementCriteriaScript
{
    public:
        achievement_cant_do_that_while_stunned(const char* name) : AchievementCriteriaScript(name) {}

        bool OnCheck(Player* /*player*/, Unit* target)
        {
            if (!target)
                return false;

            if (Creature* boss = target->ToCreature())
                if (boss->AI()->GetData(DATA_I_CHOOSE_YOU_PHASE_CHECK))
                    if (InstanceScript* instance = boss->GetInstanceScript())
                        if (Creature* brundir = ObjectAccessor::GetCreature(*boss, instance->GetData64(BOSS_BRUNDIR)))
                            if (brundir->AI()->GetData(DATA_CANT_DO_THAT_WHILE_STUNNED))
                                return true;

            return false;
        }
};


class spell_supercharge : public SpellScriptLoader
{
    public:

       spell_supercharge() : SpellScriptLoader("spell_supercharge") {}

        class spell_supercharge_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_supercharge_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
               unitList.remove_if(IsNoAssemblyMember());
           }

            void Register()
            {
               OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_supercharge_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
               OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_supercharge_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
               OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_supercharge_SpellScript::FilterTargets, EFFECT_2, TARGET_UNIT_SRC_AREA_ENTRY);
           }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_supercharge_SpellScript();
        }
};

#define NPC_ARCHIVUM_CONSOLE 500000

#define ACTION_START_EVENT_1 42
#define ACTION_START_EVENT_2 43
#define ACTION_START_EVENT_3 44

#define FREYA_IMAGE 33876
#define HODIR_IMAGE 33879
#define MIMIRON_IMAGE 33880
#define THORIM_IMAGE 33878

Position const ImagesPos      = {1435.80f, 118.83f, 426.40f, 6.1318f};

class npc_brann_algalon : public CreatureScript
{
public:
    npc_brann_algalon() : CreatureScript("npc_brann_algalon") {}

    struct npc_brann_algalonAI : public ScriptedAI
    {
        npc_brann_algalonAI(Creature* creature) : ScriptedAI(creature)
        {
            eventStep = 0;
            mui_eventChange = 7000;
            eventPreStart = false;
            eventPreStart2 = false;
            eventPreStart3 = false;
            guid = 0;
            _introDone = false;
            brannStep = 0;
            consoleStep = 0;
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_START_EVENT_1)
            {
                eventPreStart = true;
                consoleStep = 0;
                brannStep = 0;
            }
            else if (action == ACTION_START_EVENT_2)
            {
                eventPreStart2 = true;
                consoleStep = 10;
                brannStep = 10;
            }
            else if (action == ACTION_START_EVENT_3)
            {
                eventPreStart3 = true;
                consoleStep = 16;
                brannStep = 0;
            }
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!_introDone && me->IsWithinDistInMap(who, 5.0f))
            {
                if (who->GetTypeId() == TYPEID_PLAYER)
                {
                    guid = who->GetGUID();
                    DoAction(ACTION_START_EVENT_1);
                    _introDone = true;
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (mui_eventChange <= diff)
            {
                mui_eventChange = 7000;
                console = me->FindNearestCreature(NPC_ARCHIVUM_CONSOLE, 30);
                if (!console)
                    return;
                if (eventPreStart)
                {
                    switch (eventStep)
                    {
                    case 0:
                    case 2:
                    case 4:
                        Talk(brannStep);
                        brannStep++;
                        break;
                    case 5:
                    case 7:
                    case 9:
                    case 11:
                    case 13:
                    case 15:
                    case 19:
                        mui_eventChange = 12000;
                        Talk(brannStep);
                        brannStep++;
                        break;
                    case 1:
                    case 3:
                    case 6:
                    case 8:
                    case 12:
                    case 14:
                        mui_eventChange = 12000;
                        console->AI()->Talk(consoleStep);
                        consoleStep++;
                        break;
                    case 16:
                        console->AI()->Talk(consoleStep);
                        mui_eventChange = 10000;
                        consoleStep++;
                        break;
                    case 17:
                    case 18:
                        console->AI()->Talk(consoleStep);
                        mui_eventChange = 5000;
                        consoleStep++;
                        break;
                    case 10:
                        console->AI()->Talk(consoleStep);
                        mui_eventChange = 25000;
                        consoleStep++;
                        break;
                    default:
                        eventPreStart = false;
                        eventStep = 0;
                        consoleStep = 0;
                        brannStep = 0;
                        DoAction(ACTION_START_EVENT_2);
                        return;
                    }
                    eventStep++;
                }
                else if (eventPreStart2)
                {
                    switch (eventStep)
                    {
                    case 0:
                    case 2:
                    case 7:
                    case 10:
                        Talk(brannStep);
                        mui_eventChange = 7000;
                        brannStep++;
                        break;
                    case 4:
                        Talk(brannStep);
                        mui_eventChange = 15000;
                        brannStep++;
                        break;
                    case 1:
                    case 3:
                    case 5:
                    case 6:
                        console->AI()->Talk(consoleStep);
                        mui_eventChange = 7000;
                        consoleStep++;
                        break;
                    case 8:
                    case 9:
                        console->AI()->Talk(consoleStep);
                        mui_eventChange = 12000;
                        consoleStep++;
                        break;
                    default :
                        eventPreStart2 = false;
                        eventStep = 0;
                        consoleStep = 0;
                        brannStep = 0;
                        DoAction(ACTION_START_EVENT_3);
                        return;
                    }
                    eventStep++;
                }
                else if (eventPreStart3)
                {
                    switch (eventStep)
                    {
                        case 0:
                            if ((sumImage = me->SummonCreature(FREYA_IMAGE, ImagesPos, TEMPSUMMON_MANUAL_DESPAWN)) != NULL)
                                sumImage->GetMotionMaster()->MoveRotate(20000, ROTATE_DIRECTION_LEFT);
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                        case 6:
                            console->AI()->Talk(consoleStep);
                            consoleStep++;
                            mui_eventChange = 6000;
                            /*freya*/
                            break;
                        case 7:
                            if (sumImage)
                                sumImage->DespawnOrUnsummon();
                            if ((sumImage = me->SummonCreature(HODIR_IMAGE, ImagesPos, TEMPSUMMON_MANUAL_DESPAWN)) != NULL)
                                sumImage->GetMotionMaster()->MoveRotate(20000, ROTATE_DIRECTION_LEFT);
                        case 8:
                        case 9:
                        case 10:
                        case 11:
                            console->AI()->Talk(consoleStep);
                            consoleStep++;
                            mui_eventChange = 6000;
                            /*hodir*/
                            break;
                        case 12:
                            if (sumImage)
                                sumImage->DespawnOrUnsummon();
                            if ((sumImage = me->SummonCreature(MIMIRON_IMAGE, ImagesPos, TEMPSUMMON_MANUAL_DESPAWN)) != NULL)
                                sumImage->GetMotionMaster()->MoveRotate(20000, ROTATE_DIRECTION_LEFT);
                        case 13:
                        case 14:
                        case 15:
                        case 16:
                            console->AI()->Talk(consoleStep);
                            consoleStep++;
                            mui_eventChange = 6000;
                            /*mimiron*/
                            break;
                        case 17:
                            if (sumImage)
                                sumImage->DespawnOrUnsummon();
                            if ((sumImage = me->SummonCreature(THORIM_IMAGE, ImagesPos, TEMPSUMMON_MANUAL_DESPAWN)) != NULL)
                                sumImage->GetMotionMaster()->MoveRotate(20000, ROTATE_DIRECTION_LEFT);
                        case 18:
                        case 19:
                        case 20:
                            console->AI()->Talk(consoleStep);
                            consoleStep++;
                            mui_eventChange = 6000;
                            /*thorim*/
                            break;
                        default :
                            if (sumImage)
                                sumImage->DespawnOrUnsummon();
                            sumImage = NULL;
                            eventPreStart3 = false;
                            eventStep = 0;
                            consoleStep = 0;
                            brannStep = 0;
                            return;
                    }
                    eventStep++;
                }
            }
            else
                mui_eventChange -= diff;
        }

    private :
        uint32 eventStep;
        uint32 mui_eventChange;
        bool eventPreStart, eventPreStart2, eventPreStart3;
        Creature *console;
        uint64 guid;
        bool _introDone;
        uint32 brannStep, consoleStep;
        Creature *sumImage;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_brann_algalonAI(creature);
    }
};



void AddSC_boss_assembly_of_iron()
{
    new boss_steelbreaker();
    new spell_steelbreaker_static_disruption();
    new spell_steelbreaker_electrical_charge();
    new boss_runemaster_molgeim();
    new boss_stormcaller_brundir();
    new mob_lightning_elemental();
    new mob_rune_of_summoning();
    new mob_rune_of_power();
    new spell_shield_of_runes();
    new spell_assembly_meltdown();
    new spell_supercharge();
    //    new achievement_i_choose_you("achievement_i_choose_you");
    //    new achievement_i_choose_you("achievement_i_choose_you_25");
    new achievement_but_i_am_on_your_side("achievement_but_i_am_on_your_side");
    new achievement_but_i_am_on_your_side("achievement_but_i_am_on_your_side_25");
    new achievement_cant_do_that_while_stunned("achievement_cant_do_that_while_stunned");
    new achievement_cant_do_that_while_stunned("achievement_cant_do_that_while_stunned_25");

    new npc_brann_algalon();
}

// Steelbreaker
#undef SPELL_HIGH_VOLTAGE
#undef SPELL_FUSION_PUNCH
#undef SPELL_STATIC_DISRUPTION
#undef SPELL_OVERWHELMING_POWER

// Molgeim
#undef SPELL_SHIELD_OF_RUNES
#undef SPELL_SHIELD_OF_RUNES_BUFF
#undef SPELL_LIGHTNING_BLAST

// Brundir
#undef SPELL_CHAIN_LIGHTNING
#undef SPELL_OVERLOAD
#undef SPELL_LIGHTNING_WHIRL
#undef SPELL_LIGHTNING_WHIRL_DMG
#undef SPELL_LIGHTNING_TENDRILS
