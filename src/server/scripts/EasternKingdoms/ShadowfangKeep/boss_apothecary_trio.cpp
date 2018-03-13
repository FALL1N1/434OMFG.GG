 /*
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
#include "ScriptedGossip.h"
#include "SpellInfo.h"
#include "shadowfang_keep.h"
#include "Group.h"
#include "LFGMgr.h"
#include "Player.h"



enum Entities
{
    NPC_CRAZED_APOTHECARY   = 36568,
    NPC_VIAL_BUNNY          = 36530,
};

enum Texts
{
    SAY_AGGRO_1         = 0, // Did they bother to tell you who I am and why Iam doing this?
    SAY_AGGRO_2         = 1, // ...or are they just using you like they do everybody else?
    SAY_AGGRO_3         = 2, // But what does it matter. It is time for this to end.
    SAY_CALL_BAXTER     = 3, // Baxter! Get in here and help! NOW!
    SAY_CALL_FRYE       = 4, // It is time, Frye! Attack!
    SAY_SUMMON_ADDS     = 5, // Apothecaries! Give your life for The Crown!
    SAY_DEATH_HUMMEL    = 6, // ...please don''t think less of me.

    SAY_DEATH_BAXTER    = 0, // It has been the greatest honor of my life to server with you, Hummel.
    SAY_DEATH_FRYE      = 0, // Great. We're nto gutless, we're incompetent.
};

enum spells
{
    // NPC_HUMMEL
    SPELL_ALLURING_PERFUME           = 68589,
    SPELL_ALLURING_PERFUME_SPRAY     = 68607,

    // NPC_BAXTER
    SPELL_IRRESISTIBLE_COLOGNE       = 68946,
    SPELL_IRRESISTIBLE_COLOGNE_SPRAY = 68948,

    // NPC_HUMMEL + NPC_BAXTER
    SPELL_TABLE_APPEAR               = 69216,
    SPELL_SUMMON_TABLE               = 69218,
    SPELL_CHAIN_REACTION             = 68821,
    
    // NPC_FRYE
    SPELL_THROW_PERFUME              = 68799,
    SPELL_THROW_COLOGNE              = 68841,
    SPELL_ALLURING_PERFUME_SPILL     = 68798,
    SPELL_IRRESISTIBLE_COLOGNE_SPILL = 68614,

    // NPC_CRAZED_APOTHECARY
    SPELL_UNSTABLE_REACTION          = 68957,

    // NPC_VIAL_BUNNY
    SPELL_COLOGNE_SPILL_DMG          = 68934,
    SPELL_PERFUME_SPILL_DMG          = 68927,
};

enum Actions
{
    START_INTRO,
    START_FIGHT,
    APOTHECARY_DIED,
    SPAWN_CRAZED,
};

enum Phases
{
    PHASE_NORMAL,
    PHASE_INTRO,
};

enum Faction
{
    FACTION_HOSTILE = 14,
};

const Position Loc[]=
{
    // spawn points
    {-215.776443f, 2242.365479f, 79.769257f, 0.0f},
    {-169.500702f, 2219.286377f, 80.613045f, 0.0f},
    {-200.056641f, 2152.635010f, 79.763107f, 0.0f},
    {-238.448242f, 2165.165283f, 89.582985f, 0.0f}
};

const Position fryePos[]=
{
    {-209.342758f, 2222.672852f, 79.762497f, 0.0f},
    {-196.308014f, 2210.541016f, 79.762726f, 0.0f},
    {-204.588913f, 2198.001709f, 79.764236f, 0.0f},
    {-218.863174f, 2197.279297f, 79.764252f, 0.0f},
    {-218.473434f, 2213.379883f, 79.762840f, 0.0f},
    {-207.460266f, 2206.341797f, 79.763794f, 0.0f}
};

const char* GOSSIP_ITEM_START = "Begin the battle.";

void SetInCombat(Creature* self)
{
    self->AI()->DoZoneInCombat(self, 150.0f);

    if (!self->isInCombat())
        self->AI()->EnterEvadeMode();
}

/*######
## apothecary hummel
######*/

class npc_apothecary_hummel : public CreatureScript
{
    public:
        npc_apothecary_hummel() : CreatureScript("npc_apothecary_hummel") { }

        bool OnGossipHello(Player* player, Creature* creature)
        {
            if (creature->isQuestGiver())
                player->PrepareQuestMenu(creature->GetGUID());

            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_START, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
        {
             if (action == GOSSIP_ACTION_INFO_DEF + 1)
                creature->AI()->DoAction(START_INTRO);

            player->CLOSE_GOSSIP_MENU();
            return true;
        }

        struct npc_apothecary_hummelAI : public ScriptedAI
        {
            npc_apothecary_hummelAI(Creature* creature) : ScriptedAI(creature), summons(me)
            {
                instance = creature->GetInstanceScript();
            }

            void Reset() override
            {
                me->RestoreFaction();
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                me->AddAura(SPELL_ALLURING_PERFUME, me);
                phase = PHASE_NORMAL;
                step = 0;
                deadCount = 0;
                stepTimer = 1500;
                sprayTimer = urand(4000, 7000);
                chainReactionTimer = urand(10000, 25000);
                crazedTimer = 8000;
                firstCrazed = false;

                me->SetCorpseDelay(900); // delay despawn while still fighting baxter or frye
                summons.DespawnAll();
                                
                if (Creature* baxter = Creature::GetCreature(*me, instance->GetData64(NPC_BAXTER)))
                    if (!baxter->isAlive())
                        baxter->Respawn(true);
                    else
                        baxter->GetMotionMaster()->MoveTargetedHome();

                if (Creature* frye = Creature::GetCreature(*me, instance->GetData64(NPC_FRYE)))
                    if (!frye->isAlive())
                        frye->Respawn(true);
                    else
                        frye->GetMotionMaster()->MoveTargetedHome();
            }

            void DoAction(int32 const action) override
            {
                switch (action)
                {
                    case START_INTRO:
                    {

                        if (Creature* baxter = Creature::GetCreature(*me, instance->GetData64(NPC_BAXTER)))
                            baxter->AI()->DoAction(START_INTRO);
                        if (Creature* frye = Creature::GetCreature(*me, instance->GetData64(NPC_FRYE)))
                            frye->AI()->DoAction(START_INTRO);

                        phase = PHASE_INTRO;
                        me->SetReactState(REACT_PASSIVE);
                        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        SetInCombat(me);
                        break;
                    }
                    case START_FIGHT:
                    {
                        phase = PHASE_NORMAL;
                        me->setFaction(FACTION_HOSTILE);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        break;
                    }
                    case APOTHECARY_DIED:
                    {
                        ++deadCount;
                        if (deadCount > 2) // all 3 apothecarys dead, set lootable
                        {
                            summons.DespawnAll();
                            me->SetCorpseDelay(90); // set delay
                            me->setDeathState(JUST_DIED); // update delay
                            me->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

                            Map::PlayerList const& players = me->GetMap()->GetPlayers();
                            if (!players.isEmpty())
                            {
                                if (Group* group = players.begin()->getSource()->GetGroup())
                                    if (group->isLFGGroup())
                                        sLFGMgr->FinishDungeon(group->GetGUID(), 288);
                            }
                        }
                        else
                        {
                            if (me->HasFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE))
                                me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
                        }
                        break;
                    }
                    case SPAWN_CRAZED:
                    {
                        uint8 i = urand(0, 3);
                        me->SummonCreature(NPC_CRAZED_APOTHECARY, Loc[i], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30 * IN_MILLISECONDS);
                        
                        if (!firstCrazed)
                        {
                            Talk(SAY_SUMMON_ADDS);
                            firstCrazed = true;
                        }
                        break;
                    }
                }
            }

            void UpdateAI(uint32 const diff) override
            {
                if (phase == PHASE_INTRO)
                {
                    if (stepTimer <= diff)
                    {
                        ++step;
                        switch (step)
                        {
                            case 1:
                            {
                                Talk(SAY_AGGRO_1);
                                stepTimer = 3500;
                                break;
                            }
                            case 2:
                            {
                                Talk(SAY_AGGRO_2);
                                stepTimer = 3500;
                                break;
                            }
                            case 3:
                            {
                                Talk(SAY_AGGRO_3);
                                stepTimer = 3000;
                                break;
                            }
                            case 4:
                            {
                                DoAction(START_FIGHT);
                                break;
                            }
                        }
                    }
                    else
                        stepTimer -= diff;
                }
                else // PHASE_NORMAL
                {
                    if (!UpdateVictim())
                        return;
                    
                    if (me->HasUnitState(UNIT_STATE_CASTING))
                        return;

                    if (chainReactionTimer <= diff)
                    {
                        DoCast(me, SPELL_TABLE_APPEAR, true);
                        DoCast(me, SPELL_SUMMON_TABLE, true);
                        DoCast(SPELL_CHAIN_REACTION);
                        chainReactionTimer = urand(15000, 25000);
                    }
                    else
                        chainReactionTimer -= diff;

                    if (sprayTimer <= diff)
                    {
                        DoCastVictim(SPELL_ALLURING_PERFUME_SPRAY);
                        sprayTimer = urand(8000, 15000);
                    }
                    else
                        sprayTimer -= diff;

                    if (crazedTimer <= diff)
                    {
                        DoAction(SPAWN_CRAZED);
                        crazedTimer = 8000;
                    }
                    else
                        crazedTimer -= diff;

                    DoMeleeAttackIfReady();
                }
            }

            void JustSummoned(Creature* summon) override
            {
                summons.Summon(summon);
                summon->setFaction(FACTION_HOSTILE);
                summon->SetReactState(REACT_PASSIVE);
                summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }

            void JustDied(Unit* /*killer*/) override
            {
                Talk(SAY_DEATH_HUMMEL);
                DoAction(APOTHECARY_DIED);
            }

        private:
            InstanceScript* instance;
            SummonList summons;
            uint8 deadCount;
            uint8 phase;
            uint8 step;
            uint32 stepTimer;
            uint32 sprayTimer;
            uint32 chainReactionTimer;
            uint32 crazedTimer;
            bool firstCrazed;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_apothecary_hummelAI(creature);
        }
};

/*######
## apothecary baxter
######*/

class npc_apothecary_baxter : public CreatureScript
{
    public:
        npc_apothecary_baxter() : CreatureScript("npc_apothecary_baxter") { }

        struct npc_apothecary_baxterAI : public ScriptedAI
        {
            npc_apothecary_baxterAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            void Reset() override
            {
                me->RestoreFaction();
                me->AddAura(SPELL_IRRESISTIBLE_COLOGNE, me);                
                _waitTimer = 20000;
                sprayTimer = urand(4000, 7000);
                chainReactionTimer = urand (10000, 25000);
                phase = PHASE_NORMAL;

                if (Creature* hummel = Creature::GetCreature(*me, instance->GetData64(NPC_HUMMEL)))
                    if (!hummel->isAlive())
                        hummel->Respawn(true);
                    else
                        hummel->GetMotionMaster()->MoveTargetedHome();

                if (Creature* frye = Creature::GetCreature(*me, instance->GetData64(NPC_FRYE)))
                    if (!frye->isAlive())
                        frye->Respawn(true);
                    else
                        frye->GetMotionMaster()->MoveTargetedHome();
            }

            void DoAction(int32 const action) override
            {
                switch (action)
                {
                    case START_INTRO:
                    {
                        me->GetMotionMaster()->MovePoint(0, -220.502213f, 2206.999268f, 79.763680f);
                        phase = PHASE_INTRO;
                        break;
                    }
                    case START_FIGHT:
                    {
                        if (Creature* hummel = Creature::GetCreature(*me, instance->GetData64(NPC_HUMMEL)))
                            hummel->AI()->Talk(SAY_CALL_BAXTER);

                        phase = PHASE_NORMAL;
                        me->setFaction(FACTION_HOSTILE);
                        SetInCombat(me);
                        break;
                    }
                }
            }

            void UpdateAI(uint32 const diff) override
            {
                if (phase == PHASE_INTRO)
                {
                    if (_waitTimer <= diff)
                    {
                        DoAction(START_FIGHT);
                    }
                    else
                        _waitTimer -= diff;
                }
                else // PHASE_NORMAL
                {
                    if (!UpdateVictim())
                        return;

                    if (me->HasUnitState(UNIT_STATE_CASTING))
                        return;

                    if (chainReactionTimer <= diff)
                    {
                        DoCast(me, SPELL_TABLE_APPEAR, true);
                        DoCast(me, SPELL_SUMMON_TABLE, true);
                        DoCast(SPELL_CHAIN_REACTION);
                        chainReactionTimer = urand(15000, 25000);
                    }
                    else
                        chainReactionTimer -= diff;

                    if (sprayTimer <= diff)
                    {
                        DoCastVictim(SPELL_IRRESISTIBLE_COLOGNE_SPRAY);
                        sprayTimer = urand(8000, 15000);
                    }
                    else
                        sprayTimer -= diff;

                    DoMeleeAttackIfReady();
                }
            }

            void JustDied(Unit* /*killer*/) override
            {
                Talk(SAY_DEATH_BAXTER);
                if (Creature* hummel = Creature::GetCreature(*me, instance->GetData64(NPC_HUMMEL)))
                    hummel->AI()->DoAction(APOTHECARY_DIED);
            }

        private:
            InstanceScript* instance;
            uint32 chainReactionTimer;
            uint32 sprayTimer;
            uint32 _waitTimer;
            uint8 phase;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_apothecary_baxterAI(creature);
        }
};

/*######
## apothecary frye
######*/

class npc_apothecary_frye : public CreatureScript
{
    public:
        npc_apothecary_frye() : CreatureScript("npc_apothecary_frye") { }

        struct npc_apothecary_fryeAI : public ScriptedAI
        {
            npc_apothecary_fryeAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            void Reset() override
            {
                me->RestoreFaction();
                _waitTimer = 28000;
                _throwTimer = urand(2000, 4000);
                _moveTimer = 10000;
                _targetSwitchTimer = urand(1000, 2000);
                phase = PHASE_NORMAL;

                if (Creature* hummel = Creature::GetCreature(*me, instance->GetData64(NPC_HUMMEL)))
                    if (!hummel->isAlive())
                        hummel->Respawn(true);
                    else
                        hummel->GetMotionMaster()->MoveTargetedHome();

                if (Creature* baxter = Creature::GetCreature(*me, instance->GetData64(NPC_HUMMEL)))
                    if (!baxter->isAlive())
                        baxter->Respawn(true);
                    else
                        baxter->GetMotionMaster()->MoveTargetedHome();
            }

            void DoAction(int32 const action) override
            {
                switch (action)
                {
                    case START_INTRO:
                    {
                        me->GetMotionMaster()->MovePoint(0, -197.915985f, 2197.781006f, 79.762688f);
                        phase = PHASE_INTRO;
                        break;
                    }
                    case START_FIGHT:
                    {
                        if (Creature* hummel = Creature::GetCreature(*me, instance->GetData64(NPC_HUMMEL)))
                            hummel->AI()->Talk(SAY_CALL_FRYE);

                        phase = PHASE_NORMAL;
                        me->setFaction(FACTION_HOSTILE);
                        SetInCombat(me);
                        break;
                    }
                }
            }

            void SummonBunny(Unit* target, bool perfume)
            {
                if (!target)
                    return;

                if (Creature* bunny = me->SummonCreature(NPC_VIAL_BUNNY, *target, TEMPSUMMON_TIMED_DESPAWN, 25*IN_MILLISECONDS))
                {
                    bunny->setFaction(FACTION_HOSTILE);
                    bunny->SetReactState(REACT_PASSIVE);
                    bunny->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NON_ATTACKABLE |UNIT_FLAG_NOT_SELECTABLE);
                    bunny->CastSpell(bunny, perfume ? SPELL_ALLURING_PERFUME_SPILL : SPELL_IRRESISTIBLE_COLOGNE_SPILL, true, NULL, NULL, me->GetGUID());
                }
            }

            void SpellHitTarget(Unit* target, SpellInfo const* spell) override
            {
                switch (spell->Id)
                {
                    case SPELL_THROW_PERFUME:
                        SummonBunny(target, true);
                        break;
                    case SPELL_THROW_COLOGNE:
                        SummonBunny(target, false);
                        break;
                }
            }

            void UpdateAI(uint32 const diff) override
            {
                if (phase == PHASE_INTRO)
                {
                    if (_waitTimer <= diff)
                    {
                        DoAction(START_FIGHT);
                    }
                    else
                        _waitTimer -= diff;
                }
                else // PHASE_NORMAL
                {
                    if (!UpdateVictim())
                        return;

                    if (_throwTimer <= diff)
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            DoCast(target, urand(0, 1) ? SPELL_THROW_PERFUME : SPELL_THROW_COLOGNE);
                        _throwTimer = urand(5000, 7500);
                    }
                    else
                        _throwTimer -= diff;

                    if (_targetSwitchTimer <= diff)
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100, true))
                        {
                            me->getThreatManager().modifyThreatPercent(me->getVictim(), -100);
                            me->AddThreat(target, 9999999.9f);
                        }
                        _targetSwitchTimer = urand(5000, 10000);
                    }
                    else
                        _targetSwitchTimer -= diff;

                    if (_moveTimer <= diff)
                    {
                        uint8 i = urand(0, 5);
                        me->GetMotionMaster()->MovePoint(2, fryePos[i]);
                        _moveTimer = 10000;
                    }
                    else
                        _moveTimer -= diff;

                    DoMeleeAttackIfReady();
                }
            }

            void JustDied(Unit* /*killer*/) override
            {
                Talk(SAY_DEATH_FRYE);
                if (Creature* hummel = Creature::GetCreature(*me, instance->GetData64(NPC_HUMMEL)))
                    hummel->AI()->DoAction(APOTHECARY_DIED);
            }

        private:
            InstanceScript* instance;
            uint32 _targetSwitchTimer; 
            uint32 _waitTimer;
            uint32 _throwTimer;
            uint32 _moveTimer;
            uint8 phase;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_apothecary_fryeAI(creature);
        }
};

/*######
## npc_crazed_apothecary
######*/

class npc_crazed_apothecary : public CreatureScript
{
    public:
        npc_crazed_apothecary() : CreatureScript("npc_crazed_apothecary") { }

        struct npc_crazed_apothecaryAI : public ScriptedAI
        {
            npc_crazed_apothecaryAI(Creature* creature) : ScriptedAI(creature) {}

            void IsSummonedBy(Unit* /*summoner*/) override
            {
                DoZoneInCombat();
                _explodeTimer = urand(8000,10000);
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))                
                    me->GetMotionMaster()->MoveFollow(target, 0.0f, float(2 * M_PI*rand_norm()));
                else
                    me->DespawnOrUnsummon();
            }

            void UpdateAI(uint32 const diff) override
            {
                if (_explodeTimer <= diff)
                {
                    DoCast(me, SPELL_UNSTABLE_REACTION);
                }
                else
                    _explodeTimer -= diff;
            }

        private:
            uint32 _explodeTimer;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_crazed_apothecaryAI(creature);
        }
};

/*######
## spell trio voids
######*/

class spell_trio_irresistible_cologne : public SpellScriptLoader
{
    public:
        spell_trio_irresistible_cologne() : SpellScriptLoader("spell_trio_irresistible_cologne") { }

    class spell_trio_irresistible_cologne_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_trio_irresistible_cologne_AuraScript)

            bool Validate(SpellInfo const * /*spellEntry*/)
        {
            if (!sSpellStore.LookupEntry(SPELL_COLOGNE_SPILL_DMG))
                return false;
            return true;
        }

        void HandleEffectPeriodic(AuraEffect const * aurEff)
        {
            Unit* aurEffCaster = aurEff->GetCaster();
            if (!aurEffCaster)
                return;

            Unit * target = GetTarget();
            aurEffCaster->CastSpell(target, SPELL_COLOGNE_SPILL_DMG, true);
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_trio_irresistible_cologne_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_trio_irresistible_cologne_AuraScript();
    }
};

class spell_trio_alluring_perfume : public SpellScriptLoader
{
    public:
        spell_trio_alluring_perfume() : SpellScriptLoader("spell_trio_alluring_perfume") { }

    class spell_trio_alluring_perfume_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_trio_alluring_perfume_AuraScript)

            bool Validate(SpellInfo const* /*spellEntry*/)
        {
            if (!sSpellStore.LookupEntry(SPELL_PERFUME_SPILL_DMG))
                return false;
            return true;
        }

        void HandleEffectPeriodic(AuraEffect const * aurEff)
        {
            Unit* aurEffCaster = aurEff->GetCaster();
            if (!aurEffCaster)
                return;

            Unit * target = GetTarget();
            aurEffCaster->CastSpell(target, SPELL_PERFUME_SPILL_DMG, true);
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_trio_alluring_perfume_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_trio_alluring_perfume_AuraScript();
    }
};

void AddSC_boss_apothecary_trio()
{
    new npc_apothecary_hummel();
    new npc_apothecary_baxter();
    new npc_apothecary_frye();
    new npc_crazed_apothecary();
    new spell_trio_irresistible_cologne();
    new spell_trio_alluring_perfume();
}
