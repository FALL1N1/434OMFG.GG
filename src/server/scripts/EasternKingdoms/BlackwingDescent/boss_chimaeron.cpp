
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "blackwing_descent.h"

enum Spells
{
    SPELL_BERSERK               = 26662,
    SPELL_BREAK                 = 82881,
    SPELL_CAUSTIC_SLIME_TARGET  = 82871,
    SPELL_CAUSTIC_SLIME         = 82913,
    SPELL_CAUSTIC_SLIME_AURA    = 82935,
    SPELL_MASSACRE              = 82848,
    SPELL_FEUD                  = 88872,
    SPELL_DOUBLE_ATTACK_EFF     = 82882,
    SPELL_DOUBLE_ATTACK_VIS     = 88826,
    SPELL_MORTALITY_AOE         = 82890,
    SPELL_MORTALITY_SELF        = 82934,
    SPELL_FAST_ASLEEP           = 82706,
    SPELL_MIXTURE_VIS           = 91106,
    SPELL_MIXTURE_EFF           = 82705,
    SPELL_SYSTEMS_FAILURE       = 88853,
    SPELL_REROUTE_POWER         = 88861,

    //    heroic
    SPELL_TELEPORTATION_VISUAL = 41232,
    SPELL_NEFARIUS_AURA = 78494,
    SPELL_SHADOW_WHIP           = 91304,
    SPELL_MOCKING_SHADOWS       = 91307
};

enum Events
{
    EVENT_ENRAGE        = 1,
    EVENT_BREAK,
    EVENT_CAUSTIC_SLIME,
    EVENT_MASSACRE,
    EVENT_MASSACRE_FEUD,
    EVENT_DOUBLE_ATTACK,
    EVENT_MASSACRE_KNOCKOUT,
    EVENT_FEUD_INTERRUPT,
    EVENT_ALLOW_DBLE_ATTACK,

    EVENT_GROUP_NORMAL  = 1,
};

enum Misc
{
    DATA_BOT_GUID,
    PATH_BILE_O_TRON            = 4441800,
    ACTION_START_COMBAT         = 1,
    ACTION_DEATH,
    ACTION_FEUD,
    ACTION_PHASE_2
};

enum Phases
{
    PHASE_NORMAL        = 1,
    PHASE_FEUD,
    PHASE_MORTALITY,
};

enum Quotes
{
    EMOTE_AGGRO,
    EMOTE_BILE_O_TRON,
    EMOTE_MASSACRE,
    EMOTE_KNOCKOUT,
    SAY_SLAY            = 1,
    SAY_DEATH
};

class boss_chimaeron : public CreatureScript
{
public:
    boss_chimaeron() : CreatureScript("boss_chimaeron") { }

    struct boss_chimaeronAI : public BossAI
    {
        boss_chimaeronAI(Creature* creature) : BossAI(creature, DATA_CHIMAERON)
        {
            DoCast(SPELL_FAST_ASLEEP);
        }

        void Reset()
        {
            _Reset();
            botGUID = 0;
            massacreCnt = 0;
            forceStart = false;
            mui_startEvent = 30000;
            phaseSwitch = urand(20, 22);
            events.SetPhase(PHASE_NORMAL);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
            if (Creature* nefarian = me->GetCreature(*me, nefarianGUID))
                nefarian->DespawnOrUnsummon();
            nefarianGUID = 0;
            if (instance)
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

            instance->DoRemoveAurasDueToSpellOnPlayers(76143);
            instance->DoRemoveAurasDueToSpellOnPlayers(89084);
        }

        void KilledUnit(Unit* who)
        {
            BossAI::KilledUnit(who);
            if (who->GetTypeId() == TYPEID_PLAYER)
                if (Creature* finkle = me->FindNearestCreature(NPC_FINKLE_EINHORN, 200.0f))
                    finkle->AI()->Talk(SAY_SLAY);
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();

            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            if (Creature* finkle = me->FindNearestCreature(NPC_FINKLE_EINHORN, 200.0f))
            {
                finkle->AI()->Talk(SAY_DEATH);
                finkle->DespawnOrUnsummon(1000);
            }
            if (Creature* nefarian = me->GetCreature(*me, nefarianGUID))
                nefarian->AI()->DoAction(ACTION_DEATH);

            me->RemoveAllAuras();
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MORTALITY_AOE);
            instance->DoRemoveAurasDueToSpellOnPlayers(76143);
            instance->DoRemoveAurasDueToSpellOnPlayers(89084);
            me->Relocate(me->GetPositionX(), me->GetPositionY(), 73.106209f);
        }

        void JustReachedHome()
        {
            _JustReachedHome();
            DoCast(SPELL_FAST_ASLEEP);
        }

        void DamageTaken(Unit* /*done_by*/, uint32 &damage)
        {
            if (me->HealthBelowPctDamaged(phaseSwitch, damage) && events.IsInPhase(PHASE_NORMAL) || me->HealthBelowPctDamaged(phaseSwitch, damage) && events.IsInPhase(PHASE_FEUD))
            {
                if (IsHeroic())
                    if (Creature* nefarian = me->GetCreature(*me, nefarianGUID))
                        nefarian->AI()->DoAction(ACTION_PHASE_2);

                if (events.IsInPhase(PHASE_FEUD))
                {
                    if (Spell* spell = me->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                        spell->SendChannelUpdate(0);
                    me->RemoveAurasDueToSpell(SPELL_FEUD);
                }

                events.CancelEvent(EVENT_MASSACRE);
                events.CancelEventGroup(EVENT_GROUP_NORMAL);
                events.SetPhase(PHASE_MORTALITY);
                events.ScheduleEvent(EVENT_DOUBLE_ATTACK, 15000, EVENT_GROUP_NORMAL, PHASE_MORTALITY);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
                DoCast(me, SPELL_MORTALITY_AOE, true);
                DoCast(me, SPELL_MORTALITY_SELF, true);
            }
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_START_COMBAT && !botGUID)
            {
                if (Creature* bot = me->SummonCreature(NPC_BILE_O_TRON_800, 0, 0, 0))
                {
                    Talk(EMOTE_BILE_O_TRON);
                    botGUID = bot->GetGUID();
                    bot->AddAura(SPELL_MIXTURE_VIS, bot);
                    bot->AddAura(SPELL_MIXTURE_EFF, bot);
                    bot->GetMotionMaster()->Clear();
                    bot->GetMotionMaster()->MovePath(PATH_BILE_O_TRON, true);
                    forceStart = true;
                }
            }
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();

            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            Talk(EMOTE_AGGRO);
            events.ScheduleEvent(EVENT_ENRAGE, 450000);
            ScheduleEvents();
            if (Creature* nefarian = me->SummonCreature(NPC_NEFARIUS_CHIMAERON, -113.135f, 43.535f, 80.136f, 4.680680f))
                nefarianGUID = nefarian->GetGUID();

            Map::PlayerList const& players = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                if (Player* player = itr->getSource())
                    player->CastSpell(player, 76143, true);
        }

        void JustSummoned(Creature* creature)
        {
            if (creature->GetEntry() == NPC_NEFARIUS_CHIMAERON)
                return;

            BossAI::JustSummoned(creature);
        }

        void ScheduleEvents()
        {
            events.ScheduleEvent(EVENT_BREAK, 4500, EVENT_GROUP_NORMAL, PHASE_NORMAL);
            events.ScheduleEvent(EVENT_CAUSTIC_SLIME, 15000, EVENT_GROUP_NORMAL, PHASE_NORMAL);
            events.ScheduleEvent(EVENT_MASSACRE, 26000, EVENT_GROUP_NORMAL, PHASE_NORMAL);
        }

        void MoveInLineOfSight(Unit* /*who*/)
        {
            // Do not engage on sight
        }

        void DoMeleeAttackIfReady()
        {
            if (me->getVictim())
            {
                if (me->isAttackReady() && !me->IsNonMeleeSpellCasted(false))
                {
                    if (me->IsWithinMeleeRange(me->getVictim()))
                    {
                        if (canDbleAttack)
                        {
                            DoCastVictim(SPELL_DOUBLE_ATTACK_EFF);
                            me->RemoveAurasDueToSpell(SPELL_DOUBLE_ATTACK_VIS);
                            canDbleAttack = false;
                        }
                        me->AttackerStateUpdate(me->getVictim());
                        me->resetAttackTimer();
                    }
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (forceStart)
            {
                if (mui_startEvent <= diff)
                {
                    if (!me->isInCombat())
                        me->SetInCombatWithZone();
                    forceStart = false;
                }
                else
                    mui_startEvent -= diff;
            }

            if (!UpdateVictim() && events.IsInPhase(PHASE_NORMAL))
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING) && events.IsInPhase(PHASE_NORMAL))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ENRAGE:
                    DoCast(SPELL_BERSERK);
                    break;
                case EVENT_BREAK:
                    DoCastVictim(SPELL_BREAK);
                    events.ScheduleEvent(EVENT_DOUBLE_ATTACK, 15000, EVENT_GROUP_NORMAL, PHASE_NORMAL);
                    break;
                case EVENT_CAUSTIC_SLIME:
                    DoCastAOE(SPELL_CAUSTIC_SLIME_TARGET, true);
                    if(events.IsInPhase(PHASE_NORMAL))
                        events.ScheduleEvent(EVENT_CAUSTIC_SLIME, 5000, EVENT_GROUP_NORMAL, PHASE_NORMAL);
                    break;
                case EVENT_MASSACRE:
                    if (!events.IsInPhase(PHASE_MORTALITY))
                    {
                        Talk(EMOTE_MASSACRE);
                        DoCast(SPELL_MASSACRE);
                        me->RemoveAurasDueToSpell(SPELL_DOUBLE_ATTACK_VIS);
                        canDbleAttack = false;
                        events.CancelEvent(EVENT_ALLOW_DBLE_ATTACK);
                        events.DelayEvents(7000, EVENT_GROUP_NORMAL);
                        events.ScheduleEvent(EVENT_MASSACRE_KNOCKOUT, 4000, EVENT_GROUP_NORMAL);
                        events.ScheduleEvent(EVENT_BREAK, 14000, EVENT_GROUP_NORMAL, IsHeroic() ? 0 : PHASE_NORMAL);
                    }
                    break;
                case EVENT_MASSACRE_KNOCKOUT:
                    if (roll_chance_i(40 + massacreCnt) && massacreCnt > 0)
                    {
                        massacreCnt = 20;
                        events.SetPhase(PHASE_FEUD);
                        events.CancelEventGroup(EVENT_GROUP_NORMAL);
                        DoCast(SPELL_FEUD);
                        // 3 Caustic Slime casts in a row
                        events.ScheduleEvent(EVENT_CAUSTIC_SLIME, 15000, EVENT_GROUP_NORMAL, PHASE_FEUD);
                        events.ScheduleEvent(EVENT_CAUSTIC_SLIME, 20000, EVENT_GROUP_NORMAL, PHASE_FEUD);
                        events.ScheduleEvent(EVENT_CAUSTIC_SLIME, 25000, EVENT_GROUP_NORMAL, PHASE_FEUD);
                        events.ScheduleEvent(EVENT_MASSACRE_FEUD, 30000, EVENT_GROUP_NORMAL, PHASE_FEUD);

                        if (IsHeroic())
                        {
                            if (Creature* nefarian = me->GetCreature(*me, nefarianGUID))
                            {
                                nefarian->AI()->DoAction(ACTION_FEUD);
                                events.ScheduleEvent(EVENT_FEUD_INTERRUPT, 2000);
                                events.ScheduleEvent(EVENT_DOUBLE_ATTACK, 2000, EVENT_GROUP_NORMAL, PHASE_FEUD);
                            }
                        }

                        if (Creature* bot = me->GetCreature(*me, botGUID))
                        {
                            Talk(EMOTE_KNOCKOUT);
                            bot->RemoveAllAuras();
                            bot->CastSpell(bot, SPELL_SYSTEMS_FAILURE, true);
                            bot->NearTeleportTo(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetOrientation());
                            bot->CastSpell(bot, SPELL_REROUTE_POWER, true);
                        }
                    }
                    else
                    {
                        if (massacreCnt == 0)
                            massacreCnt = 20;
                        massacreCnt += 20;
                        events.ScheduleEvent(EVENT_MASSACRE, 30000, EVENT_GROUP_NORMAL);
                    }
                    break;
                case EVENT_FEUD_INTERRUPT:
                    if(Spell* spell = me->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                        spell->SendChannelUpdate(0);
                        //spell->SendInterrupted(0);
                    me->RemoveAurasDueToSpell(SPELL_FEUD);
                    break;
                case EVENT_MASSACRE_FEUD:
                    if (!me->HasAura(SPELL_MORTALITY_SELF))
                    {
                        Talk(EMOTE_MASSACRE);
                        DoCast(SPELL_MASSACRE);
                        // re-enable bot
                        if (Creature* bot = me->GetCreature(*me, botGUID))
                        {
                            bot->GetMotionMaster()->MovePath(PATH_BILE_O_TRON, true);
                            bot->RemoveAurasDueToSpell(SPELL_SYSTEMS_FAILURE);
                        }
                        // cancel everything except Enrage and re-schedule
                        events.SetPhase(PHASE_NORMAL);
                        events.CancelEventGroup(EVENT_GROUP_NORMAL);
                        ScheduleEvents();
                        me->resetAttackTimer();
                    }
                    break;
                case EVENT_DOUBLE_ATTACK:
                    DoCast(SPELL_DOUBLE_ATTACK_VIS);
                    events.ScheduleEvent(EVENT_ALLOW_DBLE_ATTACK, 5000, 0);
                    break;
                case EVENT_ALLOW_DBLE_ATTACK:
                    canDbleAttack  = true;
                    break;
                }
            }
            DoMeleeAttackIfReady();
            EnterEvadeIfOutOfCombatArea(diff);
        }
    private:
        uint64 botGUID;
        uint64 nefarianGUID;
        uint8 massacreCnt;
        uint32 mui_startEvent;
        uint8 phaseSwitch;
        bool forceStart;
        bool canDbleAttack;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_chimaeronAI(creature);
    }
};

static const std::string GOSSIP_OPTIONS[5] =
{
    "I suppose you'll be needing a key for this cage? Wait, don't tell me. The horrific gibbering monster behind me ate it, right?",
    "You were trapped, as I recall. This situation seems oddly similar.",
    "Gnomes in Lava Suits, for example.",
    "No, I, uh, haven't seen it. You were saying?",
    "Restrictions? What restrictions?",
};

static const uint32 GOSSIP_TEXTS[6] = {16565,16593,16594,16595,16596,16597};

class npc_finkle_einhorn : public CreatureScript
{
    public:
        npc_finkle_einhorn() : CreatureScript("npc_finkle_einhorn") { }

        struct npc_finkle_einhornAI : public ScriptedAI
        {
            npc_finkle_einhornAI(Creature * creature) : ScriptedAI(creature)
            {
                instance = me->GetInstanceScript();
            }

            void Reset()
            {
                done = false;
                talkTimer = urand(10000, 15000);
            }

            void DoAction(int32 const /*action*/)
            {
                done = true;
            }

            void UpdateAI(uint32 const diff)
            {
                if (instance->GetBossState(DATA_CHIMAERON) == NOT_STARTED)
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                else
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

                if (done)
                    return;

                if (talkTimer <= diff)
                {
                    Talk(0);
                    talkTimer = urand(10000, 15000);
                }else talkTimer -= diff;
            }

        private:
            bool done;
            uint32 talkTimer;
            InstanceScript* instance;
        };

        bool OnGossipHello(Player* player, Creature* creature)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_OPTIONS[0], GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(GOSSIP_TEXTS[0], creature->GetGUID());

            creature->AI()->DoAction(0);
            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
        {
            player->PlayerTalkClass->ClearMenus();
            uint32 step = uint32(action - GOSSIP_ACTION_INFO_DEF);

            if (step <= 4)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_OPTIONS[step], GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + step + 1);

            if (step == 5)
            {
                if (Creature* chimaeron = creature->FindNearestCreature(NPC_CHIMAERON, 100.0f))
                    chimaeron->AI()->DoAction(ACTION_START_COMBAT);
            }

            if (step <= 5)
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTS[step], creature->GetGUID());
            return true;
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_finkle_einhornAI(creature);
        }
};

class npc_victor_nefarius_chimaeron : public CreatureScript
{
    public:
        npc_victor_nefarius_chimaeron() : CreatureScript("npc_victor_nefarius_chimaeron") {}

        enum
        {
            SAY_AGGRO,
            SAY_FEUD,
            SAY_DEATH,
            SAY_MOCKING_SHADOWS,
            SAY_DEATH_NON_HEROIC,
        };

        struct npc_victor_nefarius_chimaeronAI : public ScriptedAI
        {
            npc_victor_nefarius_chimaeronAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                DoCast(SPELL_NEFARIUS_AURA);
                DoCast(SPELL_TELEPORTATION_VISUAL);
                me->SetHover(true);
            }

            void Reset()
            {
                deathTimer = 0;
                outro = false;
            }

            void IsSummonedBy(Unit* /* who*/)
            {
                Talk(SAY_AGGRO);
            }

            void DoAction(int32 const action)
            {
                switch(action)
                {
                case ACTION_DEATH:
                    outro = true;
                    deathTimer = 3000;
                    break;
                case ACTION_FEUD:
                    Talk(SAY_FEUD);
                    me->CastSpell(me, SPELL_SHADOW_WHIP, true);
                    break;
                case ACTION_PHASE_2:
                    Talk(SAY_MOCKING_SHADOWS);
                    DoCast(SPELL_MOCKING_SHADOWS);
                    break;
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!outro)
                    return;

                if (deathTimer <= diff)
                {
                    outro = false;
                    Talk((IsHeroic() ? SAY_DEATH : SAY_DEATH_NON_HEROIC));
                    DoCast(SPELL_TELEPORTATION_VISUAL);
                    me->DespawnOrUnsummon(2000);
                } else deathTimer -= diff;
            }

        private:
            bool outro;
            uint32 deathTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_victor_nefarius_chimaeronAI(creature);
        }
};

class spell_finkles_mixture : public SpellScriptLoader
{
public:
    spell_finkles_mixture() : SpellScriptLoader("spell_finkles_mixture") { }

    class spell_finkles_mixture_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_finkles_mixture_AuraScript);

        void CalculateAmount(AuraEffect const* /* aurEff*/, int32 &amount, bool & canBeRecalculated)
        {
            amount = -1;
        }

        void Absorb(AuraEffect* /* aurEff*/, DamageInfo &dmgInfo, uint32 &absorbAmount)
        {
            Unit* target = GetTarget();
            uint32 damage = dmgInfo.GetDamage();
            uint32 hp = target->GetHealth();
            if (damage >= hp && hp > 10000)
                absorbAmount = ++damage - hp;
            else
                absorbAmount = 0;
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_finkles_mixture_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
            OnEffectAbsorb += AuraEffectAbsorbFn(spell_finkles_mixture_AuraScript::Absorb, EFFECT_1);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_finkles_mixture_AuraScript();
    }
};

class PlayerCheck : public std::unary_function<Unit*, bool>
{
public:
    explicit PlayerCheck(Unit* _caster) : caster(_caster) { }
    bool operator()(WorldObject* object)
    {
        return object == caster->getVictim() || !object->ToPlayer();
    }

private:
    Unit* caster;
};

class spell_caustic_slime_selector : public SpellScriptLoader
{
public:
    spell_caustic_slime_selector() : SpellScriptLoader("spell_caustic_slime_selector") { }

    class spell_caustic_slime_selector_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_caustic_slime_selector_SpellScript);

        void HandleDummy(SpellEffIndex effIndex)
        {
            if (Unit* caster = GetCaster())
                caster->CastSpell(GetHitUnit(), SPELL_CAUSTIC_SLIME, true);
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(PlayerCheck(GetCaster()));
            if (targets.empty())
                return;

            Trinity::Containers::RandomResizeList(targets, GetCaster()->GetMap()->Is25ManRaid() ? 4 : 1);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_caustic_slime_selector_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_caustic_slime_selector_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_caustic_slime_selector_SpellScript();
    }
};

class spell_below_10_percent_health : public SpellScriptLoader
{
public:
    spell_below_10_percent_health() : SpellScriptLoader("spell_below_10_percent_health") { }

    class spell_below_10_percent_health_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_below_10_percent_health_AuraScript);

        // cheap hack to make it have update calls
        void CalcPeriodic(AuraEffect const* /*effect*/, bool& isPeriodic, int32& amplitude)
        {
            isPeriodic = true;
            amplitude = 100;
        }

        void Update(AuraEffect const* effect)
        {
            if (Unit* target = GetTarget())
            {
                if (target->GetHealth() <= 10000 && !target->HasAura(89084))
                    target->AddAura(89084, target);
                else if (target->GetHealth() > 10000 && target->HasAura(89084))
                         target->RemoveAurasDueToSpell(89084);
            }
        }

        void Register()
        {
            DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_below_10_percent_health_AuraScript::CalcPeriodic, EFFECT_0, SPELL_AURA_DUMMY);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_below_10_percent_health_AuraScript::Update, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_below_10_percent_health_AuraScript();
    }
};

void AddSC_boss_chimaeron()
{
    new boss_chimaeron();
    new npc_finkle_einhorn();
    new npc_victor_nefarius_chimaeron();
    new spell_finkles_mixture();
    new spell_caustic_slime_selector();
    new spell_below_10_percent_health();
};
