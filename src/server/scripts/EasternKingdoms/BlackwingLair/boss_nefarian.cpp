/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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
SDName: Boss_Nefarian
SD%Complete: 80
SDComment: Some issues with class calls effecting more than one class
SDCategory: Blackwing Lair
EndScriptData */

#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"

enum Say
{
    SAY_RANDOM              = 0,
    SAY_RAISE_SKELETONS     = 1,
    SAY_SLAY                = 2,
    SAY_DEATH               = 3,

    SAY_MAGE                = 4,
    SAY_WARRIOR             = 5,
    SAY_DRUID               = 6,
    SAY_PRIEST              = 7,
    SAY_PALADIN             = 8,
    SAY_SHAMAN              = 9,
    SAY_WARLOCK             = 10,
    SAY_HUNTER              = 11,
    SAY_ROGUE               = 12,
};

enum Spells
{
    SPELL_SHADOWFLAME_INITIAL   = 22972,
    SPELL_SHADOWFLAME           = 22539,
    SPELL_BELLOWINGROAR         = 22686,
    SPELL_VEILOFSHADOW          = 7068,
    SPELL_CLEAVE                = 20691,
    SPELL_TAILLASH              = 23364,
    SPELL_BONECONTRUST          = 23363,     //23362, 23361

    SPELL_MAGE                  = 23410,     //wild magic
    SPELL_WARRIOR               = 23397,     //beserk
    SPELL_DRUID                 = 23398,     // cat form
    SPELL_PRIEST                = 23401,     // corrupted healing
    SPELL_PALADIN               = 23418,     //syphon blessing
    SPELL_SHAMAN                = 23425,     //totems
    SPELL_WARLOCK               = 23427,     //infernals
    SPELL_HUNTER                = 23436,     //bow broke
    SPELL_ROGUE                 = 23414     //Paralise
};

class boss_nefarian : public CreatureScript
{
public:
    boss_nefarian() : CreatureScript("boss_nefarian") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_nefarianAI (creature);
    }

    struct boss_nefarianAI : public ScriptedAI
    {
        boss_nefarianAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 ShadowFlame_Timer;
        uint32 BellowingRoar_Timer;
        uint32 VeilOfShadow_Timer;
        uint32 Cleave_Timer;
        uint32 TailLash_Timer;
        uint32 ClassCall_Timer;
        bool Phase3;

        uint32 DespawnTimer;

        void Reset()
        {
            ShadowFlame_Timer         = 12000;  // These times are probably wrong
            BellowingRoar_Timer       = 30000;
            VeilOfShadow_Timer        = 15000;
            Cleave_Timer              = 7000;
            TailLash_Timer            = 10000;
            ClassCall_Timer           = 35000;  // 35-40 seconds
            Phase3 = false;

            DespawnTimer              = 5000;
        }

        void KilledUnit(Unit* Victim)
        {
            if (rand()%5)
                return;

            Talk(SAY_SLAY, Victim->GetGUID());
        }

        void JustDied(Unit* /*Killer*/)
        {
            Talk(SAY_DEATH);
        }

        void EnterCombat(Unit* who)
        {
            Talk(SAY_RANDOM);

            DoCast(who, SPELL_SHADOWFLAME_INITIAL);
            DoZoneInCombat();
        }

        void UpdateAI(const uint32 diff)
        {
            if (DespawnTimer <= diff)
            {
                if (!UpdateVictim())
                    me->DespawnOrUnsummon();
                DespawnTimer = 5000;
            } else DespawnTimer -= diff;

            if (!UpdateVictim())
                return;

            //ShadowFlame_Timer
            if (ShadowFlame_Timer <= diff)
            {
                DoCastVictim(SPELL_SHADOWFLAME);
                ShadowFlame_Timer = 12000;
            } else ShadowFlame_Timer -= diff;

            //BellowingRoar_Timer
            if (BellowingRoar_Timer <= diff)
            {
                DoCastVictim(SPELL_BELLOWINGROAR);
                BellowingRoar_Timer = 30000;
            } else BellowingRoar_Timer -= diff;

            //VeilOfShadow_Timer
            if (VeilOfShadow_Timer <= diff)
            {
                DoCastVictim(SPELL_VEILOFSHADOW);
                VeilOfShadow_Timer = 15000;
            } else VeilOfShadow_Timer -= diff;

            //Cleave_Timer
            if (Cleave_Timer <= diff)
            {
                DoCastVictim(SPELL_CLEAVE);
                Cleave_Timer = 7000;
            } else Cleave_Timer -= diff;

            //TailLash_Timer
            if (TailLash_Timer <= diff)
            {
                //Cast NYI since we need a better check for behind target
                //DoCastVictimSPELL_TAILLASH);

                TailLash_Timer = 10000;
            } else TailLash_Timer -= diff;

            //ClassCall_Timer
            if (ClassCall_Timer <= diff)
            {
                //Cast a random class call
                //On official it is based on what classes are currently on the hostil list
                //but we can't do that yet so just randomly call one

                switch (urand(0, 8))
                {
                    case 0:
                        Talk(SAY_MAGE);
                        DoCast(me, SPELL_MAGE);
                        break;
                    case 1:
                        Talk(SAY_WARRIOR);
                        DoCast(me, SPELL_WARRIOR);
                        break;
                    case 2:
                        Talk(SAY_DRUID);
                        DoCast(me, SPELL_DRUID);
                        break;
                    case 3:
                        Talk(SAY_PRIEST);
                        DoCast(me, SPELL_PRIEST);
                        break;
                    case 4:
                        Talk(SAY_PALADIN);
                        DoCast(me, SPELL_PALADIN);
                        break;
                    case 5:
                        Talk(SAY_SHAMAN);
                        DoCast(me, SPELL_SHAMAN);
                        break;
                    case 6:
                        Talk(SAY_WARLOCK);
                        DoCast(me, SPELL_WARLOCK);
                        break;
                    case 7:
                        Talk(SAY_HUNTER);
                        DoCast(me, SPELL_HUNTER);
                        break;
                    case 8:
                        Talk(SAY_ROGUE);
                        DoCast(me, SPELL_ROGUE);
                        break;
                }

                ClassCall_Timer = 35000 + (rand() % 5000);
            } else ClassCall_Timer -= diff;

            //Phase3 begins when we are below X health
            if (!Phase3 && HealthBelowPct(20))
            {
                Phase3 = true;
                Talk(SAY_RAISE_SKELETONS);
            }

            DoMeleeAttackIfReady();
        }
    };
};

class ClassCheck
{
public:
    ClassCheck(Classes cl): _classes(cl) { }

    bool operator() (WorldObject *target)
    {
        if (target->GetTypeId() == TYPEID_PLAYER && target->ToUnit()->getClass() != _classes)
            return true;

        return false;
    }

private :
    Classes _classes;
};


//    SPELL_MAGE                  = 23410,     //wild magic
enum mageSpells
{
    SPELL_FIREBALL = 23411,
    SPELL_GIVRE = 23412,
    SPELL_ARCANE_BOOM = 23413,
};

class spell_nefarian_controled_mage : public SpellScriptLoader
{
public:
    spell_nefarian_controled_mage() : SpellScriptLoader("spell_nefarian_controled_mage") { }

    class spell_nefarian_controled_mage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefarian_controled_mage_SpellScript);

        bool Load()
        {
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(ClassCheck(CLASS_MAGE));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_nefarian_controled_mage_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    class spell_nefarian_controled_mage_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_nefarian_controled_mage_AuraScript);

        bool Validate(SpellInfo const* /*entry*/)
        {
            return true;
        }

        void HandleEffectPeriodic(AuraEffect const* aurEff)
        {
            if (Unit *owner = GetTarget())
            {
                switch (rand() % 3)
                {
                    case 0:
                        owner->CastSpell(owner, SPELL_FIREBALL, true);
                        break;
                    case 1:
                        owner->CastSpell(owner, SPELL_ARCANE_BOOM, true);
                        break;
                    case 2:
                        owner->CastSpell(owner, SPELL_GIVRE, true);
                        break;
                    default:
                        owner->CastSpell(owner, SPELL_FIREBALL, true);
                        break;
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_nefarian_controled_mage_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_nefarian_controled_mage_AuraScript();
    }

    SpellScript* GetSpellScript() const
    {
        return new spell_nefarian_controled_mage_SpellScript();
    }
};

//    SPELL_WARRIOR               = 23397,     //beserk
class spell_nefarian_controled_warrior : public SpellScriptLoader
{
public:
    spell_nefarian_controled_warrior() : SpellScriptLoader("spell_nefarian_controled_warrior") { }

    class spell_nefarian_controled_warrior_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefarian_controled_warrior_SpellScript);

        bool Load()
        {
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(ClassCheck(CLASS_WARRIOR));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_nefarian_controled_warrior_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_nefarian_controled_warrior_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_nefarian_controled_warrior_SpellScript();
    }
};

//    SPELL_DRUID                 = 23398,     // cat form
class spell_nefarian_controled_druid : public SpellScriptLoader
{
public:
    spell_nefarian_controled_druid() : SpellScriptLoader("spell_nefarian_controled_druid") { }

    class spell_nefarian_controled_druid_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefarian_controled_druid_SpellScript);

        bool Load()
        {
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(ClassCheck(CLASS_DRUID));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_nefarian_controled_druid_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_nefarian_controled_druid_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_nefarian_controled_druid_SpellScript();
    }
};

//    SPELL_PRIEST                = 23401,     // corrupted healing
class spell_nefarian_controled_priest : public SpellScriptLoader
{
public:
    spell_nefarian_controled_priest() : SpellScriptLoader("spell_nefarian_controled_priest") { }

    class spell_nefarian_controled_priest_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefarian_controled_priest_SpellScript);

        bool Load()
        {
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(ClassCheck(CLASS_PRIEST));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_nefarian_controled_priest_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_nefarian_controled_priest_SpellScript();
    }
};

//    SPELL_PALADIN               = 23418,     //syphon blessing
class spell_nefarian_controled_paladin : public SpellScriptLoader
{
public:
    spell_nefarian_controled_paladin() : SpellScriptLoader("spell_nefarian_controled_paladin") { }

    class spell_nefarian_controled_paladin_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefarian_controled_paladin_SpellScript);

        bool Load()
        {
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(ClassCheck(CLASS_PALADIN));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_nefarian_controled_paladin_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_nefarian_controled_paladin_SpellScript();
    }
};

//    SPELL_SHAMAN                = 23425,     //totems
enum totemSpells
{
    SPELL_TOTEM_1 = 23419,
    SPELL_TOTEM_2 = 23420,
    SPELL_TOTEM_3 = 23422,
    SPELL_TOTEM_4 = 23423,
};

class spell_nefarian_controled_shaman : public SpellScriptLoader
{
public:
    spell_nefarian_controled_shaman() : SpellScriptLoader("spell_nefarian_controled_shaman") { }

    class spell_nefarian_controled_shaman_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefarian_controled_shaman_SpellScript);

        bool Load()
        {
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(ClassCheck(CLASS_SHAMAN));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_nefarian_controled_shaman_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    class spell_nefarian_controled_shaman_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_nefarian_controled_shaman_AuraScript);

        bool Validate(SpellInfo const* /*entry*/)
        {
            return true;
        }

        void HandleEffectPeriodic(AuraEffect const* aurEff)
        {
            if (Unit *owner = GetTarget())
            {
                switch (rand() % 4)
                {
                    case 0:
                        owner->CastSpell(owner, SPELL_TOTEM_1, true);
                        break;
                    case 1:
                        owner->CastSpell(owner, SPELL_TOTEM_2, true);
                        break;
                    case 2:
                        owner->CastSpell(owner, SPELL_TOTEM_3, true);
                        break;
                    case 3:
                        owner->CastSpell(owner, SPELL_TOTEM_4, true);
                        break;
                    default:
                        owner->CastSpell(owner, SPELL_TOTEM_1, true);
                        break;
                }
            }
        }
        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_nefarian_controled_shaman_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_nefarian_controled_shaman_AuraScript();
    }

    SpellScript* GetSpellScript() const
    {
        return new spell_nefarian_controled_shaman_SpellScript();
    }
};

//    SPELL_WARLOCK               = 23427,     //infernals
class spell_nefarian_controled_warlock : public SpellScriptLoader
{
public:
    spell_nefarian_controled_warlock() : SpellScriptLoader("spell_nefarian_controled_warlock") { }

    class spell_nefarian_controled_warlock_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefarian_controled_warlock_SpellScript);

        bool Load()
        {
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(ClassCheck(CLASS_WARLOCK));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_nefarian_controled_warlock_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };


    class spell_nefarian_controled_warlock_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_nefarian_controled_warlock_AuraScript);

        void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* target = GetTarget())
                target->CastSpell(target, 23426, true);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_nefarian_controled_warlock_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }

    };

    SpellScript* GetSpellScript() const
    {
        return new spell_nefarian_controled_warlock_SpellScript();
    }

    AuraScript* GetAuraScript() const
    {
        return new spell_nefarian_controled_warlock_AuraScript();
    }

};

// SPELL_INFRNAL_SUMMON = 23426
//    SPELL_HUNTER                = 23436,     //bow broke
class spell_nefarian_controled_hunter : public SpellScriptLoader
{
public:
    spell_nefarian_controled_hunter() : SpellScriptLoader("spell_nefarian_controled_hunter") { }

    class spell_nefarian_controled_hunter_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefarian_controled_hunter_SpellScript);

        bool Load()
        {
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(ClassCheck(CLASS_HUNTER));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_nefarian_controled_hunter_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_nefarian_controled_hunter_SpellScript();
    }
};

//    SPELL_ROGUE                 = 23414     //Paralise
class spell_nefarian_controled_rogue : public SpellScriptLoader
{
public:
    spell_nefarian_controled_rogue() : SpellScriptLoader("spell_nefarian_controled_rogue") { }

    class spell_nefarian_controled_rogue_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefarian_controled_rogue_SpellScript);

        bool Load()
        {
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(ClassCheck(CLASS_ROGUE));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_nefarian_controled_rogue_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_nefarian_controled_rogue_SpellScript();
    }
};

class npc_nefarian_totem : public CreatureScript
{
public:
    npc_nefarian_totem() : CreatureScript("npc_nefarian_totem") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_nefarian_totemAI (creature);
    }

    enum totemEntrys
    {
        TOTEM_CORRUPTED_WINDFURY = 14666,
        TOTEM_CORRUPTED_FIRENOVA = 14662,
        TOTEM_CORRUPTED_STONESKIN = 14663,
        TOTEM_CORRUPTED_HEAL = 14664,
    };

    struct npc_nefarian_totemAI : public Scripted_NoMovementAI
    {
        npc_nefarian_totemAI(Creature* creature) : Scripted_NoMovementAI(creature)
        {
            _despawn = false;
        }

        void DamageTaken(Unit* /*done_by*/, uint32 &damage)
        {
            damage = 0;
            if (_despawn)
                return;
            _despawn = true;
            me->DespawnOrUnsummon(500);
        }

        void Reset()
        {
            switch(me->GetEntry())
            {
                case TOTEM_CORRUPTED_WINDFURY:
                    me->CastSpell(me, 32911, true);
                    break;
                case TOTEM_CORRUPTED_FIRENOVA:
                    me->CastSpell(me, 30941, true);
                    break;
                case TOTEM_CORRUPTED_STONESKIN:
                    me->CastSpell(me, 68893, true);
                    break;
                case TOTEM_CORRUPTED_HEAL:
                    me->CastSpell(me, 52042,true);
                    break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
        }

    private:
        bool _despawn;
    };

};

void AddSC_boss_nefarian()
{
    new boss_nefarian();
    new spell_nefarian_controled_mage();
    new spell_nefarian_controled_warrior();
    new spell_nefarian_controled_druid();
    new spell_nefarian_controled_priest();
    new spell_nefarian_controled_warlock();
    new spell_nefarian_controled_paladin();
    new spell_nefarian_controled_hunter();
    new spell_nefarian_controled_rogue();
    new spell_nefarian_controled_shaman();
    new npc_nefarian_totem();
}
