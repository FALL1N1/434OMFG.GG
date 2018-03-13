/*
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
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

#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "naxxramas.h"

//Stalagg
enum StalaggYells
{
    SAY_STAL_AGGRO          = 0, //not used
    SAY_STAL_SLAY           = 1, //not used
    SAY_STAL_DEATH          = 2  //not used
};

enum StalagSpells
{
    SPELL_POWERSURGE        = 28134,
    H_SPELL_POWERSURGE      = 54529,
    SPELL_MAGNETIC_PULL     = 28338,
    SPELL_STALAGG_TESLA     = 28097
};

//Feugen
enum FeugenYells
{
    SAY_FEUG_AGGRO          = 0, //not used
    SAY_FEUG_SLAY           = 1, //not used
    SAY_FEUG_DEATH          = 2 //not used
};

enum FeugenSpells
{
    SPELL_STATICFIELD       = 28135,
    H_SPELL_STATICFIELD     = 54528,
    SPELL_FEUGEN_TESLA      = 28109
};

// Thaddius DoAction
enum ThaddiusActions
{
    ACTION_FEUGEN_RESET,
    ACTION_FEUGEN_DIED,
    ACTION_STALAGG_RESET,
    ACTION_STALAGG_DIED,
    ACTION_SHOCKED
};

//generic
#define C_TESLA_COIL            16218           //the coils (emotes "Tesla Coil overloads!")

enum TeslaSpells
{
    SPELL_SHOCK                 = 28099,
    SPELL_STALAGG_CHAIN         = 28096,
    SPELL_STALAGG_TESLA_PASSIVE = 28097,
    SPELL_FEUGEN_TESLA_PASSIVE  = 28109,
    SPELL_FEUGEN_CHAIN          = 28111
};

//Thaddius
enum ThaddiusYells
{

    SAY_GREET               = 0,
    SAY_AGGRO               = 1,
    SAY_SLAY                = 2,
    SAY_ELECT               = 3,
    SAY_DEATH               = 4,
    SAY_SCREAM              = 5
};

enum ThaddiusSpells
{
    SPELL_POLARITY_SHIFT        = 28089,
    SPELL_BALL_LIGHTNING        = 28299,
    SPELL_POSITIVE_CHARGE       = 28062,
    SPELL_POSITIVE_CHARGE_STACK = 29659,
    SPELL_NEGATIVE_CHARGE       = 28085,
    SPELL_NEGATIVE_CHARGE_STACK = 29660,
    SPELL_CHAIN_LIGHTNING       = 28167,
    H_SPELL_CHAIN_LIGHTNING     = 54531,
    SPELL_BERSERK               = 27680,
    SPELL_POSITIVE_POLARITY     = 28059,
    SPELL_POLARITY_CHARGE_2     = 39088,
    SPELL_POLARITY_CHARGE_3     = 39091,
    SPELL_NEGATIVE_POLARITY     = 28084,
//   SPELL_THADIUS_SPAWN       = 28160,
};

enum Events
{
    EVENT_NONE,
    EVENT_OVERLOAD,
    EVENT_SHIFT,
    EVENT_CHAIN,
    EVENT_BERSERK,
};

enum Achievements
{
    ACHIEVEMENT_SHOCKING_10 = 2178,
    ACHIEVEMENT_SHOCKING_25 = 2179
};

enum Achievement
{
    DATA_POLARITY_SWITCH    = 76047605,
};
class boss_thaddius : public CreatureScript
{
public:
    boss_thaddius() : CreatureScript("boss_thaddius") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_thaddiusAI (creature);
    }

    struct boss_thaddiusAI : public BossAI
    {
        boss_thaddiusAI(Creature* creature) : BossAI(creature, BOSS_THADDIUS)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            // init is a bit tricky because thaddius shall track the life of both adds, but not if there was a wipe
            // and, in particular, if there was a crash after both adds were killed (should not respawn)

            // Moreover, the adds may not yet be spawn. So just track down the status if mob is spawn
            // and each mob will send its status at reset (meaning that it is alive)
            checkFeugenAlive = false;
            if (Creature *pFeugen = me->GetCreature(*me, instance->GetData64(DATA_FEUGEN)))
                checkFeugenAlive = pFeugen->isAlive();

            checkStalaggAlive = false;
            if (Creature *pStalagg = me->GetCreature(*me, instance->GetData64(DATA_STALAGG)))
                checkStalaggAlive = pStalagg->isAlive();

            if (!checkFeugenAlive && !checkStalaggAlive)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
                me->SetControlled(false, UNIT_STATE_STUNNED);
                me->SetReactState(REACT_AGGRESSIVE);
            }
            else
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
                me->SetControlled(true, UNIT_STATE_STUNNED);
                me->SetReactState(REACT_PASSIVE);
            }
        }

        bool checkStalaggAlive;
        bool checkFeugenAlive;
        bool bShocked;
        uint32 uiAddsTimer;

        void Reset()
        {
            _Reset();
            bShocked = false;
        }

        void KilledUnit(Unit* /*victim*/)
        {
            if (!(rand()%5))
                Talk(SAY_SLAY);
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            Talk(SAY_DEATH);
            if (InstanceScript *instance = me->GetInstanceScript())
            {
                if (!bShocked)
                     instance->DoCompleteAchievement(RAID_MODE(ACHIEVEMENT_SHOCKING_10, ACHIEVEMENT_SHOCKING_25));
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_POSITIVE_CHARGE_STACK);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_NEGATIVE_CHARGE_STACK);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_POSITIVE_POLARITY);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_POLARITY_CHARGE_2);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_POLARITY_CHARGE_3);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_NEGATIVE_POLARITY);
            }
        }

        void DoAction(const int32 action)
        {
            switch (action)
            {
                case ACTION_FEUGEN_RESET:
                    checkFeugenAlive = true;
                    break;
                case ACTION_FEUGEN_DIED:
                    checkFeugenAlive = false;
                    break;
                case ACTION_STALAGG_RESET:
                    checkStalaggAlive = true;
                    break;
                case ACTION_STALAGG_DIED:
                    checkStalaggAlive = false;
                    break;
                case ACTION_SHOCKED:
                    bShocked = true;
                    break;
            }

            if (!checkFeugenAlive && !checkStalaggAlive)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
                // REACT_AGGRESSIVE only reset when he takes damage.
                DoZoneInCombat();
            }
            else
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
                me->SetControlled(true, UNIT_STATE_STUNNED);
                me->SetReactState(REACT_PASSIVE);
            }
        }

        void EnterCombat(Unit * /*who*/)
        {
            _EnterCombat();
            Talk(SAY_AGGRO);
            events.ScheduleEvent(EVENT_OVERLOAD, 15000);
            events.ScheduleEvent(EVENT_SHIFT, 30000);
            events.ScheduleEvent(EVENT_CHAIN, urand(10000, 20000));
            events.ScheduleEvent(EVENT_BERSERK, 360000);
        }

      void EnterEvadeMode()
      {
          _EnterEvadeMode();
          Reset();
          if (Creature *pStalagg = me->GetCreature(*me, instance->GetData64(DATA_STALAGG)))
          {
              pStalagg->Respawn();
              pStalagg->GetMotionMaster()->MovePoint(0, pStalagg->GetHomePosition());
          }
          if (Creature *pFeugen = me->GetCreature(*me, instance->GetData64(DATA_FEUGEN)))
          {
              pFeugen->Respawn();
              pFeugen->GetMotionMaster()->MovePoint(0, pFeugen->GetHomePosition());
          }
      }

        void DamageTaken(Unit * /*pDoneBy*/, uint32 & /*uiDamage*/)
        {
            me->SetControlled(false, UNIT_STATE_STUNNED);
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void UpdateAI(const uint32 diff)
        {
            if (checkFeugenAlive && checkStalaggAlive)
                uiAddsTimer = 0;

            if (checkStalaggAlive != checkFeugenAlive)
            {
                uiAddsTimer += diff;
                if (uiAddsTimer > 5000)
                {
                    if (!checkStalaggAlive)
                    {
                        if (instance)
                            if (Creature *pStalagg = me->GetCreature(*me, instance->GetData64(DATA_STALAGG)))
                            {
                                pStalagg->Respawn();
                                pStalagg->GetMotionMaster()->MovePoint(0, pStalagg->GetHomePosition());
                            }
                    }
                    else
                    {
                        if (instance)
                            if (Creature *pFeugen = me->GetCreature(*me, instance->GetData64(DATA_FEUGEN)))
                            {
                                pFeugen->Respawn();
                                pFeugen->GetMotionMaster()->MovePoint(0, pFeugen->GetHomePosition());
                            }
                    }
                }
            }

            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_OVERLOAD:
                        //                        if (Creature *pTesla = me->FindNearestCreature(C_TESLA_COIL, 50))
                        //    pTesla->AI()->Talk(EMOTE_TESLA_OVERLOAD);
                        if (instance)
                        {
                            if (GameObject* go = GameObject::GetGameObject(*me, instance->GetData64(DATA_THADDIUS_TESLA05)))
                                go->UseDoorOrButton();
                            if (GameObject* go = GameObject::GetGameObject(*me, instance->GetData64(DATA_THADDIUS_TESLA06)))
                                go->UseDoorOrButton();
                        }
                        break;
                    case EVENT_SHIFT:
                        DoCastAOE(SPELL_POLARITY_SHIFT);
                        events.ScheduleEvent(EVENT_SHIFT, 30000);
                        return;
                    case EVENT_CHAIN:
                        DoCast(me->getVictim(), RAID_MODE(SPELL_CHAIN_LIGHTNING, H_SPELL_CHAIN_LIGHTNING));
                        events.ScheduleEvent(EVENT_CHAIN, urand(10000, 20000));
                        return;
                    case EVENT_BERSERK:
                        DoCast(me, SPELL_BERSERK);
                        return;
                }
            }

            if (events.GetTimer() > 15000 && !me->IsWithinMeleeRange(me->getVictim()))
                DoCast(me->getVictim(), SPELL_BALL_LIGHTNING);
            else
                DoMeleeAttackIfReady();
        }
    };

};

class mob_stalagg : public CreatureScript
{
public:
    mob_stalagg() : CreatureScript("mob_stalagg") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_stalaggAI(creature);
    }

    struct mob_stalaggAI : public ScriptedAI
    {
        mob_stalaggAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        uint32 powerSurgeTimer;
        uint32 magneticPullTimer;
        uint32 uiIdleTimer;
        uint32 uiShockTimer;
        uint64 uiTeslaGuid;
        uint32 uiChainTimer;
        bool bChainReset;
        bool bShock;
        bool bSwitch;
        Position homePosition;

        void Reset()
        {
            if (instance)
            {
                if (Creature *pThaddius = me->GetCreature(*me, instance->GetData64(DATA_THADDIUS)))
                    if (pThaddius->AI())
                        pThaddius->AI()->DoAction(ACTION_STALAGG_RESET);

                if (GameObject* go = GameObject::GetGameObject(*me, instance->GetData64(DATA_THADDIUS_TESLA06)))
                    go->ResetDoorOrButton();
            }
            powerSurgeTimer = urand(20000, 25000);
            magneticPullTimer = 20000;
            uiIdleTimer = 3*IN_MILLISECONDS;
            uiShockTimer = 1*IN_MILLISECONDS;
            uiChainTimer = 10*IN_MILLISECONDS;
            bChainReset = true;
            bShock = false;
            bSwitch = false;
            homePosition = me->GetHomePosition();
        }

        void JustDied(Unit * /*killer*/)
        {
            if (instance)
                if (Creature *pThaddius = me->GetCreature(*me, instance->GetData64(DATA_THADDIUS)))
                    if (pThaddius->AI())
                        pThaddius->AI()->DoAction(ACTION_STALAGG_DIED);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (bChainReset)
            {
                if (uiChainTimer <= uiDiff)
                {
                    if (Creature *pTesla = me->FindNearestCreature(C_TESLA_COIL, 50))
                    {
                        uiTeslaGuid = pTesla->GetGUID();
                        pTesla->CastSpell(me, SPELL_STALAGG_CHAIN, false);
                    }
                    bChainReset = false;
                    uiChainTimer = 3*IN_MILLISECONDS;
                } else uiChainTimer -= uiDiff;
            }

            if (!UpdateVictim())
                return;

            if (magneticPullTimer <= uiDiff)
            {
                if (Creature *pFeugen = me->GetCreature(*me, instance->GetData64(DATA_FEUGEN)))
                {
                    Unit* pStalaggVictim = me->getVictim();
                    Unit* pFeugenVictim = pFeugen->getVictim();

                    if (pFeugenVictim && pStalaggVictim)
                    {
                        // magnetic pull is not working. So just jump.

                        // reset aggro to be sure that feugen will not follow the jump
                        float uiTempThreat = pFeugen->getThreatManager().getThreat(pFeugenVictim);
                        pFeugen->getThreatManager().modifyThreatPercent(pFeugenVictim, -100);
                        pFeugenVictim->JumpTo(me, 0.3f);
                        pFeugen->AddThreat(pStalaggVictim, uiTempThreat);
                        pFeugen->SetReactState(REACT_PASSIVE);

                        uiTempThreat = me->getThreatManager().getThreat(pStalaggVictim);
                        me->getThreatManager().modifyThreatPercent(pStalaggVictim, -100);
                        pStalaggVictim->JumpTo(pFeugen, 0.3f);
                        me->AddThreat(pFeugenVictim, uiTempThreat);
                        me->SetReactState(REACT_PASSIVE);
                        uiIdleTimer = 3*IN_MILLISECONDS;
                        bSwitch = true;
                    }
                }

                magneticPullTimer = 20000;
            }
            else magneticPullTimer -= uiDiff;

            if (powerSurgeTimer <= uiDiff)
            {
                DoCast(me, RAID_MODE(SPELL_POWERSURGE, H_SPELL_POWERSURGE));
                powerSurgeTimer = urand(15000, 20000);
            } else powerSurgeTimer -= uiDiff;

            if (bSwitch)
                if (uiIdleTimer <= uiDiff)
                {
                    if (Creature *pFeugen = me->GetCreature(*me, instance->GetData64(DATA_FEUGEN)))
                        pFeugen->SetReactState(REACT_AGGRESSIVE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    bSwitch = false;
                } else uiIdleTimer -= uiDiff;

            if (me->GetDistance(homePosition) > 15)
            {
                if (uiShockTimer <= uiDiff)
                {
                    if (Creature *pTesla = Creature::GetCreature(*me, uiTeslaGuid))
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 200, true))
                            pTesla->CastSpell(pTarget, SPELL_SHOCK, false);
                    uiShockTimer = 1*IN_MILLISECONDS;
                    bShock = true;
                }else uiShockTimer -= uiDiff;
            } else if (bShock)
            {
                bShock = false;
                bChainReset = true;
            }

            DoMeleeAttackIfReady();
        }
    };

};

class mob_feugen : public CreatureScript
{
public:
    mob_feugen() : CreatureScript("mob_feugen") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_feugenAI(creature);
    }

    struct mob_feugenAI : public ScriptedAI
    {
        mob_feugenAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        uint32 staticFieldTimer;
        uint32 uiShockTimer;
        uint64 uiTeslaGuid;
        uint32 uiChainTimer;
        bool bChainReset;
        bool bShock;
        Position homePosition;

        void Reset()
        {
            if (instance)
            {
                if (Creature *pThaddius = me->GetCreature(*me, instance->GetData64(DATA_THADDIUS)))
                    if (pThaddius->AI())
                        pThaddius->AI()->DoAction(ACTION_FEUGEN_RESET);

                if (GameObject* go = GameObject::GetGameObject(*me, instance->GetData64(DATA_THADDIUS_TESLA05)))
                    go->ResetDoorOrButton();
            }
            staticFieldTimer = 5000;
            uiShockTimer = 1*IN_MILLISECONDS;
            uiChainTimer = 10*IN_MILLISECONDS;
            bChainReset = true;
            bShock = false;
            homePosition = me->GetHomePosition();
        }

        void JustDied(Unit * /*killer*/)
        {
            if (instance)
                if (Creature *pThaddius = me->GetCreature(*me, instance->GetData64(DATA_THADDIUS)))
                    if (pThaddius->AI())
                        pThaddius->AI()->DoAction(ACTION_FEUGEN_DIED);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (bChainReset)
            {
                if (uiChainTimer <= uiDiff)
                {
                    if (Creature *pTesla = me->FindNearestCreature(C_TESLA_COIL, 50))
                    {
                        uiTeslaGuid = pTesla->GetGUID();
                        pTesla->CastSpell(me, SPELL_FEUGEN_CHAIN, false);
                    }
                    bChainReset = false;
                    uiChainTimer = 3*IN_MILLISECONDS;
                } else uiChainTimer -= uiDiff;
            }

            if (!UpdateVictim())
                return;

            if (staticFieldTimer <= uiDiff)
            {
                DoCast(me, RAID_MODE(SPELL_STATICFIELD, H_SPELL_STATICFIELD));
                staticFieldTimer = 5000;
            } else staticFieldTimer -= uiDiff;

            if (me->GetDistance(homePosition) > 15)
            {
                if (uiShockTimer <= uiDiff)
                {
                    if (Creature *pTesla = Creature::GetCreature(*me, uiTeslaGuid))
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 200, true))
                            pTesla->CastSpell(pTarget, SPELL_SHOCK, false);
                    uiShockTimer = 1*IN_MILLISECONDS;
                    bShock = true;
                }else uiShockTimer -= uiDiff;
            } else if (bShock)
            {
                bShock = false;
                bChainReset = true;
            }
            DoMeleeAttackIfReady();
        }
    };

};

class spell_thaddius_pos_neg_charge : public SpellScriptLoader
{
public:
    spell_thaddius_pos_neg_charge() : SpellScriptLoader("spell_thaddius_pos_neg_charge") { }

    class spell_thaddius_pos_neg_charge_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_thaddius_pos_neg_charge_SpellScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_POSITIVE_CHARGE))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_POSITIVE_CHARGE_STACK))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_NEGATIVE_CHARGE))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_NEGATIVE_CHARGE_STACK))
                return false;
            return true;
        }

        bool Load()
        {
            return GetCaster()->GetTypeId() == TYPEID_PLAYER;
        }

        void HandleTargets(std::list<WorldObject*>& targets)
        {
            uint8 count = 0;
            for (std::list<WorldObject*>::iterator ihit = targets.begin(); ihit != targets.end(); ++ihit)
                if ((*ihit)->GetGUID() != GetCaster()->GetGUID())
                    if (Unit* target = (*ihit)->ToUnit())
                        if (target->HasAura(GetTriggeringSpell()->Id))
                            ++count;

            if (count)
            {
                uint32 setSpellId = 0;
                uint32 removeSpellId = 0;

                if (GetSpellInfo()->Id == SPELL_POSITIVE_CHARGE)
                {
                    setSpellId = SPELL_POSITIVE_CHARGE_STACK;
                    removeSpellId = SPELL_NEGATIVE_CHARGE_STACK;
                }
                else // if (GetSpellInfo()->Id == SPELL_NEGATIVE_CHARGE)
                {
                    setSpellId = SPELL_NEGATIVE_CHARGE_STACK;
                    removeSpellId = SPELL_POSITIVE_CHARGE_STACK;
                }

                GetCaster()->SetAuraStack(setSpellId, GetCaster(), count);
                GetCaster()->RemoveAura(removeSpellId);
            }
            else
            {
                GetCaster()->RemoveAura(SPELL_POSITIVE_CHARGE_STACK);
                GetCaster()->RemoveAura(SPELL_NEGATIVE_CHARGE_STACK);
            }
        }

        void HandleDamage(SpellEffIndex /*effIndex*/)
        {
            if (!GetTriggeringSpell())
                return;

            Unit* target = GetHitUnit();
            Unit* caster = GetCaster();

            if (target->HasAura(GetTriggeringSpell()->Id))
                SetHitDamage(0);
            else
            {
                if (target->GetTypeId() == TYPEID_PLAYER && caster->IsAIEnabled)
                    caster->ToCreature()->AI()->SetData(DATA_POLARITY_SWITCH, 1);
            }
        }

        void HandleAfterHit()
        {
            if (Unit * pCaster = GetCaster())
                if (InstanceScript* instance = pCaster->GetInstanceScript())
                    if (Creature* pThadius = CAST_CRE(Unit::GetUnit(*pCaster, instance->GetData64(DATA_THADDIUS))))
                        if (boss_thaddius::boss_thaddiusAI* pThadiusAI = CAST_AI(boss_thaddius::boss_thaddiusAI, pThadius->AI()))
                            if (Unit* target = GetHitUnit())
                            {
                                if (GetSpellInfo()->Id == 28062 || GetSpellInfo()->Id == 39090)
                                    if (target->HasAura(39091) || target->HasAura(28084))
                                        pThadiusAI->DoAction(ACTION_SHOCKED);

                                if (GetSpellInfo()->Id == 28085 || GetSpellInfo()->Id == 39093)
                                    if (target->HasAura(39088) || target->HasAura(28059))
                                        pThadiusAI->DoAction(ACTION_SHOCKED);
                            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_thaddius_pos_neg_charge_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_thaddius_pos_neg_charge_SpellScript::HandleTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
            AfterHit += SpellHitFn(spell_thaddius_pos_neg_charge_SpellScript::HandleAfterHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_thaddius_pos_neg_charge_SpellScript();
    }
};

class spell_thaddius_polarity_shift : public SpellScriptLoader
{
public:
    spell_thaddius_polarity_shift() : SpellScriptLoader("spell_thaddius_polarity_shift") { }

    class spell_thaddius_polarity_shift_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_thaddius_polarity_shift_SpellScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_POSITIVE_POLARITY) || !sSpellMgr->GetSpellInfo(SPELL_NEGATIVE_POLARITY))
                return false;
            return true;
        }

        void HandleDummy(SpellEffIndex /* effIndex */)
        {
            Unit* caster = GetCaster();
            if (Unit* target = GetHitUnit())
                target->CastSpell(target, roll_chance_i(50) ? SPELL_POSITIVE_POLARITY : SPELL_NEGATIVE_POLARITY, true, NULL, NULL, caster->GetGUID());
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_thaddius_polarity_shift_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_thaddius_polarity_shift_SpellScript();
    }
};

//class achievement_polarity_switch : public AchievementCriteriaScript
//{
    //public:
//    achievement_polarity_switch() : AchievementCriteriaScript("achievement_polarity_switch") { }
//
//    bool OnCheck(Player* /*source*/, Unit* target)
//    {
//        return target && target->GetAI()->GetData(DATA_POLARITY_SWITCH);
//    }
//};


void AddSC_boss_thaddius()
{
    new boss_thaddius();
    new mob_stalagg();
    new mob_feugen();
    new spell_thaddius_pos_neg_charge();
    new spell_thaddius_polarity_shift();
  //  new achievement_polarity_switch();
}
