
#include "PassiveAI.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "blackwing_descent.h"

enum Spells
{
    SPELL_SHARE_LIFE                = 79920, // casted by 42166 on all bosses 42179 11:11:06.047
    SPELL_POWERED_DOWN              = 82265, // transform
    SPELL_SHUTTING_DOWN             = 78746, // triggers inactive
    SPELL_INACTIVE                  = 78726,
    SPELL_ACTIVATED                 = 78740,
    SPELL_ENERGY_DRAIN              = 78725, // -100% regen
    SPELL_RECHARGE_ARCANOTRON       = 78699, // Beams
    SPELL_RECHARGE_ELECTRON         = 78697,
    SPELL_RECHARGE_MAGMATRON        = 78698,
    SPELL_RECHARGE_TOXITRON         = 78700,
    SPELL_POWER_CONVERSION          = 79729, // Arcanotron Shield
    SPELL_UNSTABLE_SHIELD           = 79900, // Electron Shield
    SPELL_BARRIER                   = 79582, // Magmatron Shield
    SPELL_POISON_SHELL              = 79835, // Toxitron Shield

    // Arcantron
    SPELL_POWER_GENERATOR           = 79626, // 11:11:20.953  11:13:23.890
    SPELL_ARCANE_ANNIHILATOR        = 79710, //
    SPELL_POWER_GENERATOR_AURA      = 79628, // trigger SPELL_POWER_GENERATOR_TRIGGER
    SPELL_POWER_GENERATOR_TRIGGER   = 79629,
    // heroic
    SPELL_NEFARIUS_SURCHARGE        = 91881,  // casted by NPC_NEFARIUS_OMNOTRON (49226) on NPC_POWER_GENERATOR (42733) 11:11:31.187 11:13:32.875
    SPELL_OVERCHARGED_GENERATOR     = 91857, // casted by NPC_POWER_GENERATOR (42733) on itself
    SPELL_OVERPOWER_GENERATOR       = 91858, // casted by NPC_POWER_GENERATOR (42733) on 5 player
    SPELL_ARCANIC_RETURN            = 91879, // casted by NPC_POWER_GENERATOR (42733) 11:11:39.390 11:13:42.297
    // Power generator
    SPELL_GROW_STACKER              = 91861,
    SPELL_ARCANE_BLOWBACK           = 91879,
    SPELL_GROW                      = 91860,

    // Electron
    SPELL_VISUAL_CHAINES            = 79573,
    SPELL_LIGHTNING_CONDUCTOR       = 79888,
    SPELL_ELECTRICAL_DISCHARGE      = 79879,
    SPELL_SHADOW_INFUSION           = 92048,
    SPELL_STATIC_HORION             = 79911,
    // heroic
    SPELL_LIGHTING_INFUSION         = 92048, // trigger SPELL_LIGHTING_INFUSION_TRIGGER
    SPELL_LIGHTING_INFUSION_TRIGGER         = 92050,
    SPELL_SHADOW_CONDUCTOR          = 92053,

    // Magmatron
    SPELL_INCINERATION              = 79023,
    SPELL_ACQUIRING_TARGET          = 79501,
    SPELL_BACKDRAFT                 = 79617,
    // heroic
    SPELL_ENCASTING_SHADOWS         = 92023, // casted by NPC_NEFARIUS_OMNOTRON (49226) on tank

    // Toxitron
    SPELL_CHEMICAL_BOMB             = 80157,
    SPELL_POISON_PROTOCOL           = 80053,
    SPELL_FIXATE                    = 80094,
    SPELL_CHEMICAL_CLOUD            = 80162,
    SPELL_CHEMICAL_CLOUD_NPC        = 80163,
    SPELL_POISON_BOMB               = 80086,
    SPELL_POISON_EXPLOSION          = 80092,
    SPELL_POISON_PUDDLE             = 80095,
    // Heroic
    SPELL_SHADOW_TELEPORT = 91823, // X: -326.2469 Y: -392.9111 Z: 213.8098 on himself hit NPC_CHEMICAL_BOMB(42934) 11:12:05.750
    SPELL_GRIP_OF_DEATH             = 91849, // casted by NPC_NEFARIUS_OMNOTRON (49226) on all raid 11:12:05.750 after SPELL_SHADOW_TELEPORT
    SPELL_SHADOW_TELEPORT_BACK = 91854, // target dest db X: -302.386 Y: -350.7 Z: 220.482

    SPELL_BERSERK                   = 26662,
    // nefarius
    SPELL_NEFARIUS_AURA = 78494,

    //SOUNDS NEFARIUS
    // 23374
    // 23376
    // 23378
    // 23375
    // move  X: -326.2469 Y: -392.9111 Z: 213.8098 to X: -326.2469 Y: -392.9111 Z: 213.8098

};

enum Texts
{
    EMOTE_SHIELD                = 0,
    SAY_ARCANOTRON_ACTIVATE     = 0,
    SAY_ELECTRON_ACTIVATE,
    SAY_MAGMATRON_ACTIVATE,
    SAY_TOXITRON_ACTIVATE,
    SAY_ARCANOTRON_SHIELD,
    SAY_ELECTRON_SHIELD,
    SAY_MAGMATRON_SHIELD,
    SAY_TOXITRON_SHIELD,
    SAY_MAGMATRON_FLAMETHROWER,
    SAY_AGGRO,
    SAY_DEATH,
    SAY_NEF_ARCANOTRON          = 0,
    SAY_NEF_ELECTRON,
    SAY_NEF_MAGMATRON,
    SAY_NEF_TOXITRON,
    SAY_NEF_AGGRO,
    SAY_NEF_DEATH
};

enum Actions
{
    ACTION_COMBAT,
    ACTION_DEATH,
    ACTION_GRIP,
    ACTION_OVERCHARGE_GENERATOR,
    ACTION_ENCASTING_SHADOW,
    ACTION_SHADOW_INFUSION,
    ACTION_RESET_EVENTS,
    ACTION_ACTIVATE,
    ACTION_FAIL_ACHIEVEMENT,
    ACTION_BERSERK
};

enum Events
{
    // Omnotron
    EVENT_BEAM_NEXT         = 1,
    EVENT_PLAYER_CHECK,
    EVENT_RESET,

    // Arcanotron
    EVENT_GENERATOR,
    EVENT_ANNIHILATOR,

    // Electron
    EVENT_CONDUCTOR,
    EVENT_DISCHARGE,

    // Magmatron
    EVENT_INCINERATION,
    EVENT_ACQUIRING_TARGET,
    EVENT_TAUNT_UNAPPLY,

    // Toxitron
    EVENT_CHEMICAL_BOMB,
    EVENT_POISON_PROTOCOL,

    // Nefarius
    EVENT_DEATH,
    EVENT_INTRO,
    EVENT_GRIP,
    EVENT_GRIP_END,
    EVENT_OVERCHARGE_GENERATOR,
    EVENT_ENCASTING_SHADOW,
    EVENT_SHADOW_INFUSION,
    EVENT_HEROIC_COOLDOWN,

    EVENT_BERSERK
};

enum Misc
{
    EMOTE_CONDUCTOR         = 1,
    EMOTE_FLAMETHROWER      = 1
};

// 67236 on bots, invis detection

struct Tron
{
    uint8 data;
    uint32 beamId;
    uint32 shieldId;
    uint8 activation;
    uint8 textId;
};

static const Tron bots[4] =
{
    {DATA_ARCANOTRON_GUID, SPELL_RECHARGE_ARCANOTRON, SPELL_POWER_CONVERSION, SAY_ARCANOTRON_ACTIVATE, SAY_ARCANOTRON_SHIELD},
    {DATA_ELECTRON_GUID, SPELL_RECHARGE_ELECTRON, SPELL_UNSTABLE_SHIELD, SAY_ELECTRON_ACTIVATE, SAY_ELECTRON_SHIELD},
    {DATA_MAGMATRON_GUID, SPELL_RECHARGE_MAGMATRON, SPELL_BARRIER, SAY_MAGMATRON_ACTIVATE, SAY_MAGMATRON_SHIELD},
    {DATA_TOXITRON_GUID, SPELL_RECHARGE_TOXITRON, SPELL_POISON_SHELL, SAY_TOXITRON_ACTIVATE, SAY_TOXITRON_SHIELD},
};

static const uint32 achievementCriterias[4] = { 15653, 15651, 15654, 15652 };

// Generic AI for bots

struct generic_botAI : public BossAI
{
    generic_botAI(Creature* creature, uint32 bossId) : BossAI(creature, bossId)
    {
        me->setPowerType(POWER_ENERGY);
    }

    void OmnotronTalk(uint8 ID)
    {
        if (Creature* omnotron = me->GetCreature(*me, instance->GetData64(DATA_OMNOTRON_GUID)))
            omnotron->AI()->Talk(ID);
    }

    void Reset()
    {
        for (uint8 i = 0; i < 4; ++i)
            achievement[i] = true;

        updateHealthTimer = 1000;
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_UNK_31); // from sniffs
        me->SetHealth(me->GetMaxHealth()); // health regen is disabled
        me->SetReactState(REACT_PASSIVE);
        me->CastSpell(me, SPELL_INACTIVE, true);
        me->CastSpell(me, SPELL_POWERED_DOWN, true);
        me->CastSpell(me, SPELL_ENERGY_DRAIN, true);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        if (Creature* omnotron = me->GetCreature(*me, instance->GetData64(DATA_OMNOTRON_GUID)))
            omnotron->AI()->Reset();
        me->GetMotionMaster()->Clear();
        _Reset();
    }

    void InitTron()
    {
        updateHealthTimer = 1000;
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_UNK_31); // from sniffs
        me->SetHealth(me->GetMaxHealth()); // health regen is disabled
        me->SetReactState(REACT_PASSIVE);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
    }

    void JustReachedHome()
    {
        me->CastSpell(me, SPELL_INACTIVE, true);
        me->CastSpell(me, SPELL_POWERED_DOWN, true);
        me->CastSpell(me, SPELL_ENERGY_DRAIN, true);
        _JustReachedHome();
    }

    void JustDied(Unit* killer)
    {
        _JustDied();
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_UNK_31);
        for (uint8 i = 0; i < 4; ++i)
            if (achievement[i])
                instance->DoCompleteCriteria(achievementCriterias[i]);
    }

    void EnterEvadeMode()
    {
        _EnterEvadeMode();
        me->RemoveAllAurasExceptType(SPELL_AURA_SHARE_DAMAGE_PCT);
        me->GetMotionMaster()->MoveTargetedHome();
    }

    void DamageTaken(Unit* attacker, uint32 &damage)
    {
        if (damage >= me->GetHealth())
        {
            if (attacker->GetTypeId() == TYPEID_PLAYER)
                killBots(attacker);
            else if (Unit* player = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                     killBots(player);

            if (Creature* omnotron = me->GetCreature(*me, instance->GetData64(DATA_OMNOTRON_GUID)))
                omnotron->AI()->DoAction(ACTION_DEATH);
        }

        if (attacker == me)
            return;

        if (Creature* omnotron = me->GetCreature(*me, instance->GetData64(DATA_OMNOTRON_GUID)))
            if (omnotron->GetHealth() > me->GetHealth())
                omnotron->SetHealth(me->GetHealth() - damage);
    }

    void killBots(Unit* killer)
    {
        if (instance->GetBossState(DATA_OMNOTRON_DEFENSE_SYSTEM) == IN_PROGRESS)
        {
            instance->SetBossState(DATA_OMNOTRON_DEFENSE_SYSTEM, IsHeroic() ? DONE_HM : DONE);
            for (int i = 0; i < 4; ++i)
            {
                if (Creature* bot = Creature::GetCreature(*me, instance->GetData64(bots[i].data)))
                {
                    bot->SetInCombatWith(killer);
                    bot->LowerPlayerDamageReq(bot->GetMaxHealth());
                    if (bot->isAlive())
                    {
                        killer->Kill(bot);
                        bot->SetLootRecipient(killer);
                    }
                }
            }
        }
    }

    void JustSummoned(Creature* summon)
    {
        BossAI::JustSummoned(summon);

        if (summon->GetEntry() == NPC_POWER_GENERATOR)
        {
            summon->ApplySpellImmune(SPELL_POWER_GENERATOR_TRIGGER, IMMUNITY_ID,  SPELL_POWER_GENERATOR_TRIGGER, true);
            summon->AddAura(SPELL_POWER_GENERATOR_AURA, summon);
            summon->DespawnOrUnsummon(60000);
        }
    }

    void EnterCombat(Unit* who)
    {
        me->SetInCombatWithZone();
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveIdle();
        if (Creature* omnotron = me->GetCreature(*me, instance->GetData64(DATA_OMNOTRON_GUID)))
            omnotron->AI()->DoAction(ACTION_COMBAT);

        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

        DoCast(SPELL_ACTIVATED);
        DoCast(SPELL_ENERGY_DRAIN);
    }

    void UpdateAI(uint32 const diff)
    {
        if (instance->GetBossState(DATA_OMNOTRON_DEFENSE_SYSTEM) == IN_PROGRESS)
            if (updateHealthTimer <= diff)
            {
                if (Creature* omnotron = me->GetCreature(*me, instance->GetData64(DATA_OMNOTRON_GUID)))
                    me->SetHealth(omnotron->GetHealth());
                updateHealthTimer = 1000;
            }
            else updateHealthTimer -= diff;
    }
private:
    uint32 updateHealthTimer;
public:
    bool achievement[4];
};

// Controller
class boss_omnotron : public CreatureScript
{
    public:
        boss_omnotron() : CreatureScript("boss_omnotron") { }

        struct boss_omnotronAI : public BossAI
        {
            boss_omnotronAI(Creature* creature) : BossAI(creature, DATA_OMNOTRON_DEFENSE_SYSTEM)
            {
                firstSpawn = true;
            }

            void Reset()
            {
                _Reset();
                oldBot = 0;
                currentBot = 0;
                nextBot = 0;
                cur = 0;
                for (uint8 i = 0; i < 4; ++i)
                    botSequenz[i] = 0;
                target = NULL;
                first = true;
                me->CombatStop(true);
                mui_start = 10000;
                canStart = false;
                mui_start_script = 1000;
                started = false;
                me->SetHealth(me->GetMaxHealth());
            }

            void EnterEvadeMode()
            {
                me->GetMotionMaster()->MoveTargetedHome();
                _EnterEvadeMode();
            }

            void InitBots()
            {
                for (uint8 i = 0; i < 4; ++i)
                {
                    botSequenz[i] = i;
                    if (Creature* tron = Creature::GetCreature(*me, instance->GetData64(bots[i].data)))
                    {
                        CAST_AI(generic_botAI, tron->AI())->InitTron();

                        if (firstSpawn)
                        {
                            tron->CastSpell(tron, SPELL_INACTIVE, true);
                            tron->CastSpell(tron, SPELL_POWERED_DOWN, true);
                            tron->CastSpell(tron, SPELL_ENERGY_DRAIN, true);
                        }
                    }

                }

                srand(time(NULL));
                std::random_shuffle(botSequenz, botSequenz + 3);

                currentBot = botSequenz[cur];

                if (Creature* tron = Creature::GetCreature(*me, instance->GetData64(bots[currentBot].data)))
                {
                    // Enable and make us follow waypoints
                    tron->RemoveAllAurasExceptType(SPELL_AURA_SHARE_DAMAGE_PCT);
                    tron->SetReactState(REACT_AGGRESSIVE);
                    tron->GetMotionMaster()->MovePath(NPC_OMNOTRON * 100, true);
                    tron->SetPower(POWER_ENERGY, 100);
                    tron->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_UNK_31); // from sniffs
                }
                firstSpawn = false;
            }

            void BeamNext()
            {
                if (cur < 3)
                    ++cur;
                else
                    cur = 0;

                nextBot = botSequenz[cur];
                if (Creature* bot = Creature::GetCreature(*me, instance->GetData64(bots[nextBot].data)))
                    DoCast(bot, bots[nextBot].beamId, true);
            }

            void ShieldOld()
            {
                oldBot = currentBot;

                if (Creature* bot = Creature::GetCreature(*me, instance->GetData64(bots[oldBot].data)))
                {
                    bot->CastWithDelay(IsHeroic() ? 3000 : 4500, bot, bots[oldBot].shieldId, false);
                    Talk(bots[oldBot].textId);
                    bot->AI()->Talk(EMOTE_SHIELD);
                    target = bot->getVictim();
                }
            }

            void ActivateNext()
            {
                currentBot = nextBot;
                if (Creature* bot = Creature::GetCreature(*me, instance->GetData64(bots[currentBot].data)))
                {
                    bot->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_UNK_31);
                    bot->SetReactState(REACT_AGGRESSIVE);
                    bot->RemoveAurasDueToSpell(SPELL_INACTIVE);
                    bot->RemoveAurasDueToSpell(SPELL_POWERED_DOWN);
                    bot->CastSpell(bot, SPELL_ENERGY_DRAIN, true);
                    bot->CastSpell(bot, SPELL_ACTIVATED, true);
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, bot);
                    bot->AI()->DoAction(ACTION_ACTIVATE);
                    if (target)
                        bot->GetMotionMaster()->MoveChase(target);
                }
            }

            void DeactivateOld()
            {
                if (!first)
                    if (Creature* bot = Creature::GetCreature(*me, instance->GetData64(bots[oldBot].data)))
                    {
                        bot->SetReactState(REACT_PASSIVE);
                        bot->RemoveAurasDueToSpell(SPELL_ACTIVATED);
                        bot->CastSpell(bot, SPELL_SHUTTING_DOWN, false);
                        bot->CastSpell(bot, SPELL_POWERED_DOWN, true);
                        bot->CastSpell(bot, SPELL_ENERGY_DRAIN, true);
                        bot->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_UNK_31);
                        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, bot);
                        bot->AttackStop();
                        bot->AI()->DoAction(ACTION_RESET_EVENTS);
                    }
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case ACTION_COMBAT:
                    {
                        if (!me->isInCombat())
                        {
                            me->SetInCombatWithZone();
                            BeamNext();
                        }
                        break;
                    }
                    case ACTION_DEATH:
                    {
                        Talk(SAY_DEATH);
                        _JustDied();
                        me->DespawnOrUnsummon(5000);
                        if (Creature* nefarius = me->GetCreature(*me, instance->GetData64(DATA_NEFARIUS_OMNOTRON_GUID)))
                            nefarius->AI()->DoAction(ACTION_DEATH);
                        break;
                    }
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                events.ScheduleEvent(EVENT_BEAM_NEXT, IsHeroic() ? 30000 : 45000);
                events.ScheduleEvent(EVENT_PLAYER_CHECK, 5000);
                if (IsHeroic())
                    events.ScheduleEvent(EVENT_BERSERK, 10 * MINUTE* IN_MILLISECONDS);
                Talk(SAY_AGGRO);
                if (IsHeroic())
                    me->SummonCreature(NPC_NEFARIUS_OMNOTRON, -302.386f, -350.7f, 220.482f, 4.7f);
            }

            void DamageTaken(Unit* attacker, uint32 &damage)
            {
                damage = 0;
            }

            void UpdateAI(uint32 const diff)
            {
                if (!started)
                {
                    if (mui_start <= diff)
                    {
                        InitBots();
                        started = true;
                    }
                    else
                        mui_start -= diff;
                }

                if (mui_start_script <= diff)
                {
                    if (Creature* tron = Creature::GetCreature(*me, instance->GetData64(bots[currentBot].data)))
                    {
                        canStart = false;
                        if (tron->isInCombat())
                            canStart = true;
                    }
                    else
                        canStart = false;
                    mui_start_script = 1000;
                }
                else mui_start_script -= diff;

                if (!canStart)
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_BEAM_NEXT: // activates next npc
                        {
                            DeactivateOld();
                            ShieldOld();
                            ActivateNext();
                            BeamNext();
                            if (first)
                                first = false;
                            events.ScheduleEvent(EVENT_BEAM_NEXT, IsHeroic() ? 30000 : 45000);
                            break;
                        }
                        case EVENT_PLAYER_CHECK:
                            if (!SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            {
                                for (int i = 0; i < 4; ++i)
                                    if (Creature* bot = Creature::GetCreature(*me, instance->GetData64(bots[i].data)))
                                        bot->AI()->EnterEvadeMode();
                                events.ScheduleEvent(EVENT_RESET, 5000);
                            }
                            events.ScheduleEvent(EVENT_PLAYER_CHECK, 5000);
                            break;
                        case EVENT_RESET:
                            Reset();
                            break;
                        case EVENT_BERSERK:
                            for (uint8 i = 0; i < 4; ++i)
                            {
                                if (Creature* tron = Creature::GetCreature(*me, instance->GetData64(bots[i].data)))
                                    tron->AI()->DoAction(ACTION_BERSERK);
                            }
                        default:
                            break;
                    }
                }
            }

        private:
            uint8 oldBot;
            uint8 currentBot;
            uint8 nextBot;
            uint8 botSequenz[4];
            uint8 cur;
            Unit* target;
            bool first;
            uint32 mui_start;
            bool started;
            uint32 mui_start_script;
            bool canStart;
            bool firstSpawn;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_omnotronAI(creature);
        }
};

// Individual AI for each bot
class npc_arcanotron : public CreatureScript
{
    public:
        npc_arcanotron() : CreatureScript("npc_arcanotron") { }

        struct npc_arcanotronAI : public generic_botAI
        {
            npc_arcanotronAI(Creature* creature) : generic_botAI(creature, DATA_OMNOTRON_DEFENSE_SYSTEM) { }

            void Reset()
            {
                generic_botAI::Reset();
            }

            void JustReachedHome()
            {
                generic_botAI::JustReachedHome();
            }

            void EnterCombat(Unit* who)
            {
                generic_botAI::EnterCombat(who);
                events.Reset();
                events.ScheduleEvent(EVENT_GENERATOR, 15000);
                events.ScheduleEvent(EVENT_ANNIHILATOR, 2000);
            }

            void EnterEvadeMode()
            {
                generic_botAI::EnterEvadeMode();
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_RESET_EVENTS)
                    events.Reset();
                else if (action == ACTION_ACTIVATE)
                {
                    events.Reset();
                    events.ScheduleEvent(EVENT_GENERATOR, 15000);
                    events.ScheduleEvent(EVENT_ANNIHILATOR, 2000);
                }
                else if (action == ACTION_BERSERK)
                    DoCast(me, SPELL_BERSERK, true);
            }

            void DamageTaken(Unit* attacker, uint32 &damage)
            {
                generic_botAI::DamageTaken(attacker, damage);
            }

            void SpellHitTarget(Unit* victim, SpellInfo const* spell)
            {
                if (spell->Id == RAID_MODE(79710, 91540, 91541, 91542))
                    achievement[0] = false;
            }

            void UpdateAI(uint32  const diff)
            {
                generic_botAI::UpdateAI(diff);

                if (!UpdateVictim() || me->HasAura(SPELL_INACTIVE))
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_GENERATOR:
                        {
                            if (IsHeroic())
                                if (Creature* nefarius = me->GetCreature(*me, instance->GetData64(DATA_NEFARIUS_OMNOTRON_GUID)))
                                    nefarius->AI()->DoAction(ACTION_OVERCHARGE_GENERATOR);
                            DoCast(SPELL_POWER_GENERATOR);
                            events.ScheduleEvent(EVENT_GENERATOR, 30000);
                            break;
                        }
                        case EVENT_ANNIHILATOR:
                            DoCastRandom(SPELL_ARCANE_ANNIHILATOR, 70.0f);
                            events.ScheduleEvent(EVENT_ANNIHILATOR, 6000);
                            break;
                        default:
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_arcanotronAI(creature);
        }
};

class npc_electron : public CreatureScript
{
    public:
        npc_electron() : CreatureScript("npc_electron") { }

        struct npc_electronAI : public generic_botAI
        {
            npc_electronAI(Creature * creature) : generic_botAI(creature, DATA_OMNOTRON_DEFENSE_SYSTEM) { }

            void Reset()
            {
                generic_botAI::Reset();
            }

            void JustReachedHome()
            {
                generic_botAI::JustReachedHome();
            }

            void EnterCombat(Unit* who)
            {
                generic_botAI::EnterCombat(who);
                events.Reset();
                events.ScheduleEvent(EVENT_CONDUCTOR, urand(11000, 15000));
                events.ScheduleEvent(EVENT_DISCHARGE, 3000);
            }

            void EnterEvadeMode()
            {
                generic_botAI::EnterEvadeMode();
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_RESET_EVENTS)
                    events.Reset();
                else if (action == ACTION_ACTIVATE)
                {
                    events.Reset();
                    events.ScheduleEvent(EVENT_CONDUCTOR, urand(11000, 15000));
                    events.ScheduleEvent(EVENT_DISCHARGE, 3000);
                }
                else if (action == ACTION_BERSERK)
                    DoCast(me, SPELL_BERSERK, true);
            }

            void DamageTaken(Unit* attacker, uint32 &damage)
            {
                generic_botAI::DamageTaken(attacker, damage);
            }

            void SpellHitTarget(Unit* victim, SpellInfo const* spell)
            {
                if (spell->Id == RAID_MODE(79912, 91456, 91457, 91458))
                    achievement[1] = false;
            }

            void UpdateAI(uint32 const diff)
            {
                generic_botAI::UpdateAI(diff);

                if (!UpdateVictim() || me->HasAura(SPELL_INACTIVE))
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CONDUCTOR:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                            {
                                DoCast(target, SPELL_LIGHTNING_CONDUCTOR);

                                if (IsHeroic())
                                    if (Creature* nefarius = me->GetCreature(*me, instance->GetData64(DATA_NEFARIUS_OMNOTRON_GUID)))
                                        nefarius->AI()->DoAction(ACTION_SHADOW_INFUSION);
                                Talk(EMOTE_CONDUCTOR, target->GetGUID());
                            }
                            events.ScheduleEvent(EVENT_CONDUCTOR, IsHeroic() ? 20000 : 25000);
                            break;
                        case EVENT_DISCHARGE:
                            DoCastRandom(SPELL_ELECTRICAL_DISCHARGE, 70.0f);
                            events.ScheduleEvent(EVENT_DISCHARGE, 3000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_electronAI(creature);
        }
};

class npc_magmatron : public CreatureScript
{
    public:
        npc_magmatron() : CreatureScript("npc_magmatron") { }

        struct npc_magmatronAI : public generic_botAI
        {
            npc_magmatronAI(Creature* creature) : generic_botAI(creature, DATA_OMNOTRON_DEFENSE_SYSTEM) { }

            void Reset()
            {
                castCount = 0;
                BossAI::Reset();
                generic_botAI::Reset();
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
            }

            void JustReachedHome()
            {
                generic_botAI::JustReachedHome();
            }

            void EnterCombat(Unit* who)
            {
                generic_botAI::EnterCombat(who);
                events.Reset();
                events.ScheduleEvent(EVENT_INCINERATION, 10000);
                events.ScheduleEvent(EVENT_ACQUIRING_TARGET, 20000);
            }

            void EnterEvadeMode()
            {
                generic_botAI::EnterEvadeMode();
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_RESET_EVENTS)
                    events.Reset();
                else if (action == ACTION_ACTIVATE)
                {
                    castCount = 0;
                    events.Reset();
                    events.ScheduleEvent(EVENT_INCINERATION, 10000);
                    events.ScheduleEvent(EVENT_ACQUIRING_TARGET, 20000);
                }
                else if (action == ACTION_FAIL_ACHIEVEMENT)
                    achievement[2] = false;
                else if (action == ACTION_BERSERK)
                    DoCast(me, SPELL_BERSERK, true);
            }

            void DamageTaken(Unit* attacker, uint32 &damage)
            {
                generic_botAI::DamageTaken(attacker, damage);
            }

            void UpdateAI(uint32 const diff)
            {
                generic_botAI::UpdateAI(diff);

                if (!UpdateVictim() || me->HasAura(SPELL_INACTIVE))
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_INCINERATION:
                            DoCast(SPELL_INCINERATION);
                            if (castCount >= 1)
                                events.ScheduleEvent(EVENT_INCINERATION, 27000);
                            else
                                events.ScheduleEvent(EVENT_INCINERATION, 32000);
                            castCount++;
                            break;
                        case EVENT_ACQUIRING_TARGET:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            {
                                DoCast(target, SPELL_ACQUIRING_TARGET);
                                if (IsHeroic())
                                    if (Creature* nefarius = me->GetCreature(*me, instance->GetData64(DATA_NEFARIUS_OMNOTRON_GUID)))
                                        nefarius->AI()->DoAction(ACTION_ENCASTING_SHADOW);
                                Talk(EMOTE_FLAMETHROWER, target->GetGUID());
                                OmnotronTalk(SAY_MAGMATRON_FLAMETHROWER);
                            }
                            events.DelayEvents(8000);
                            events.ScheduleEvent(EVENT_ACQUIRING_TARGET, IsHeroic() ? 27000 : 40000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        private:
            uint8 castCount;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_magmatronAI(creature);
        }
};

class npc_toxitron : public CreatureScript
{
    public:
        npc_toxitron() : CreatureScript("npc_toxitron") { }

        struct npc_toxitronAI : public generic_botAI
        {
            npc_toxitronAI(Creature* creature) : generic_botAI(creature, DATA_OMNOTRON_DEFENSE_SYSTEM) { }

            void Reset()
            {
                generic_botAI::Reset();
            }

            void JustReachedHome()
            {
                generic_botAI::JustReachedHome();
            }

            void EnterCombat(Unit* who)
            {
                generic_botAI::EnterCombat(who);
                events.ScheduleEvent(EVENT_CHEMICAL_BOMB, IsHeroic() ? 25000 : 11000);
                events.ScheduleEvent(EVENT_POISON_PROTOCOL, IsHeroic() ? 15000 : 21000);
            }

            void EnterEvadeMode()
            {
                generic_botAI::EnterEvadeMode();
            }

            void JustSummoned(Creature* summon)
            {
                generic_botAI::JustSummoned(summon);
                if (summon->GetEntry() == NPC_POISON_BOMB)
                {
                    summon->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SNARE, false);
                    summon->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, false);
                    summon->CastSpell(summon, SPELL_POISON_BOMB, true);
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true, -SPELL_FIXATE))
                    {
                        summon->CastSpell(target, SPELL_FIXATE, true);
                        summon->ClearUnitState(UNIT_STATE_CASTING);
                        summon->AddThreat(target, 5000000.0f);
                        summon->AI()->AttackStart(target);
                    }
                }
                else if (summon->GetEntry() == NPC_CHEMICAL_BOMB)
                {
                    summon->CastSpell(summon, SPELL_CHEMICAL_CLOUD, true);
                    summon->CastSpell(summon, SPELL_CHEMICAL_CLOUD_NPC, true);
                    summon->SetReactState(REACT_PASSIVE);
                    summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE);
                    if (IsHeroic())
                        if (Creature* nefarius = me->FindNearestCreature(NPC_NEFARIUS_OMNOTRON, 500.0f))
                            nefarius->AI()->DoAction(ACTION_GRIP);
                }
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_RESET_EVENTS)
                    events.Reset();
                else if (action == ACTION_ACTIVATE)
                {
                    events.ScheduleEvent(EVENT_CHEMICAL_BOMB, IsHeroic() ? 25000 : 11000);
                    events.ScheduleEvent(EVENT_POISON_PROTOCOL, IsHeroic() ? 15000 : 21000);
                }
                else if (action == ACTION_BERSERK)
                    DoCast(me, SPELL_BERSERK, true);
            }

            void DamageTaken(Unit* attacker, uint32 &damage)
            {
                generic_botAI::DamageTaken(attacker, damage);
            }

            void UpdateAI(uint32 const diff)
            {
                generic_botAI::UpdateAI(diff);

                if (!UpdateVictim() || me->HasAura(SPELL_INACTIVE))
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CHEMICAL_BOMB:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, -INTERACTION_DISTANCE, true))
                                DoCastRandom(SPELL_CHEMICAL_BOMB, 70.0f, false);
                            events.ScheduleEvent(EVENT_CHEMICAL_BOMB, 30000);
                            break;
                        case EVENT_POISON_PROTOCOL:
                            me->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), false); // HACK (players see nothing from this, but this avoid the channel interrupt)
                            DoCast(SPELL_POISON_PROTOCOL);
                            events.ScheduleEvent(EVENT_POISON_PROTOCOL, IsHeroic() ? 25000 : 45000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_toxitronAI(creature);
        }
};

class npc_victor_nefarius_omnotron : public CreatureScript
{
    public:
        npc_victor_nefarius_omnotron() : CreatureScript("npc_victor_nefarius_omnotron") { }

        struct npc_victor_nefarius_omnotronAI : public ScriptedAI
        {
            npc_victor_nefarius_omnotronAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_PASSIVE);
                me->CastSpell(me, SPELL_NEFARIUS_AURA, true);
            }

            void Reset()
            {
                events.Reset();
                events.ScheduleEvent(EVENT_INTRO, 2000);
                _canCast = true;
            }

            void DoAction(int32 const action)
            {
                if (!_canCast)
                    return;
                if (action == ACTION_DEATH)
                    events.ScheduleEvent(EVENT_DEATH, 5000);
                else if (action == ACTION_GRIP)
                    events.ScheduleEvent(EVENT_GRIP, 1500);
                else if (action == ACTION_OVERCHARGE_GENERATOR)
                    events.ScheduleEvent(EVENT_OVERCHARGE_GENERATOR, urand(9000, 12000));
                else if (action == ACTION_ENCASTING_SHADOW)
                    events.ScheduleEvent(EVENT_ENCASTING_SHADOW, 1000);
                else if (action == ACTION_SHADOW_INFUSION)
                    events.ScheduleEvent(EVENT_SHADOW_INFUSION, 1000);
                if (_canCast)
                    events.ScheduleEvent(EVENT_HEROIC_COOLDOWN, 30000);
                _canCast = false;
            }

            void UpdateAI(uint32 const diff)
            {
                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_INTRO:
                            Talk(SAY_NEF_AGGRO);
                            break;
                        case EVENT_DEATH:
                            Talk(SAY_NEF_DEATH);
                            me->DespawnOrUnsummon(5000);
                            break;
                        case EVENT_OVERCHARGE_GENERATOR:
                            Talk(SAY_NEF_ARCANOTRON);
                            DoCast(SPELL_NEFARIUS_SURCHARGE);
                            break;
                        case EVENT_GRIP:
                            Talk(SAY_NEF_TOXITRON);
                            DoCast(SPELL_SHADOW_TELEPORT);
                            DoCast(SPELL_GRIP_OF_DEATH);
                            events.ScheduleEvent(EVENT_GRIP_END, 5000);
                            break;
                        case EVENT_GRIP_END:
                            DoCast(SPELL_SHADOW_TELEPORT_BACK);
                            break;
                        case EVENT_ENCASTING_SHADOW:
                            Talk(SAY_NEF_MAGMATRON);
                            DoCast(SPELL_ENCASTING_SHADOWS);
                            break;
                        case EVENT_SHADOW_INFUSION:
                            Talk(SAY_NEF_ELECTRON);
                            DoCast(SPELL_LIGHTING_INFUSION);
                            break;
                        case EVENT_HEROIC_COOLDOWN:
                            _canCast = true;
                            break;
                    }
                }
            }

        private:
            EventMap events;
            bool _canCast;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_victor_nefarius_omnotronAI(creature);
        }
};

class npc_power_generator : public CreatureScript
{
    public:
        npc_power_generator() : CreatureScript("npc_power_generator") { }

        struct npc_power_generatorAI : public PassiveAI
        {
            npc_power_generatorAI(Creature* creature) : PassiveAI(creature)
            {
            }

            void Reset()
            {
                growing = false;
                actionTimer = 0;
            }

            void DamageTaken(Unit* , uint32 &damage)
            {
                damage = 0;
            }

            void UpdateAI(uint32 const diff)
            {
                if (!me->HasAura(SPELL_OVERCHARGED_GENERATOR))
                    return;

                if (actionTimer <= diff)
                {
                    if (!growing)
                    {
                        growing = true;
                        me->AddAura(SPELL_GROW_STACKER, me);
                        actionTimer = 5000;
                    }
                    else
                    {
                        DoCast(SPELL_ARCANIC_RETURN);
                        me->DespawnOrUnsummon(1000);
                        actionTimer = 2000;
                    }
                } else actionTimer -= diff;

            }

        private:
            bool growing;
            uint32 actionTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_power_generatorAI(creature);
        }
};

class spell_power_generator_periodic : public SpellScriptLoader
{
    public:
        spell_power_generator_periodic() : SpellScriptLoader("spell_power_generator_periodic") { }

        class spell_power_generator_periodic_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_power_generator_periodic_SpellScript);

            void FilterTargets(std::list<WorldObject*>& SummonList)
            {
                Unit* caster = GetCaster();
                if (!caster || SummonList.empty())
                    return;

                uint32 stackAmount = 0;
                if (Aura* aur = caster->GetAura(SPELL_GROW))
                    stackAmount = aur->GetStackAmount();

                for (std::list<WorldObject*>::iterator itr = SummonList.begin(); itr != SummonList.end();)
                {
                    if (!(*itr)->IsWithinDist(caster, INTERACTION_DISTANCE + 3.0f * stackAmount))
                        itr = SummonList.erase(itr);
                    else
                        ++itr;
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_power_generator_periodic_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENTRY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_power_generator_periodic_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENTRY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_power_generator_periodic_SpellScript();
        }
};

class spell_shadow_conductor : public SpellScriptLoader
{
    public:
        spell_shadow_conductor() : SpellScriptLoader("spell_shadow_conductor") { }

        enum
        {
            SPELL_ID_10M        = 92051
        };

        class spell_shadow_conductor_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_shadow_conductor_SpellScript);

            void HandleEffect(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || !target)
                    return;

                float damage = 6000.0f;
                float dist = caster->GetDistance(target);
                SetHitDamage(int32(damage * std::max(dist / 2.0f, 1.0f)));
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_shadow_conductor_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_shadow_conductor_SpellScript();
        }
};

class npc_test : public CreatureScript
{
    public:
        npc_test() : CreatureScript("npc_test") { }

        struct npc_testAI : public ScriptedAI
        {
            npc_testAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void Reset()
            {
            }

            void SpellHit(Unit* , const SpellInfo* )
            {
            }

            void DoAction(int32 const )
            {
            }

            void DamageTaken(Unit* , uint32 &damage)
            {
                damage = 0;
            }

            void UpdateAI(uint32 const )
            {
            }

        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_testAI(creature);
        }
};

class spell_electron_lighting_charge : public SpellScriptLoader
{
public:
    spell_electron_lighting_charge() : SpellScriptLoader("spell_electron_lighting_charge") { }

    class spell_electron_lighting_charge_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_electron_lighting_charge_SpellScript);

        bool Load()
        {
            targetCount = 1;
            return true;
        }

        void CalculateDamage(SpellEffIndex /*effIndex*/)
        {
            if (!GetHitUnit())
                return;
            int32 damages = GetHitDamage();
            SetHitDamage(int32((damages / 3) * (targetCount / 3) + damages * 2 / 3));
            targetCount++;
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_electron_lighting_charge_SpellScript::CalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }

    private :
        uint32 targetCount;
        uint32 maxTargets;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_electron_lighting_charge_SpellScript();
    }
};

class spell_electron_shield : public SpellScriptLoader
{
public:
    spell_electron_shield() : SpellScriptLoader("spell_electron_shield") { }

    class spell_electron_shield_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_electron_shield_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            return true;
        }

        bool Load()
        {
            return true;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            GetCaster()->CastSpell(eventInfo.GetProcTarget(), SPELL_STATIC_HORION, true);
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(spell_electron_shield_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_electron_shield_AuraScript();
    }
};

class spell_magmatron_shield : public SpellScriptLoader
{
public:
    spell_magmatron_shield() : SpellScriptLoader("spell_magmatron_shield") { }

    class spell_magmatron_shield_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_magmatron_shield_AuraScript);

        void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
            if (removeMode != AURA_REMOVE_BY_DAMAGE && removeMode != AURA_REMOVE_BY_ENEMY_SPELL)
                return;
            if (!GetCaster())
                return;
            GetCaster()->InterruptNonMeleeSpells(true);
            GetCaster()->CastSpell(GetCaster(), SPELL_BACKDRAFT, true);
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_magmatron_shield_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_magmatron_shield_AuraScript();
    }
};

class spell_electron_poison_bomb : public SpellScriptLoader
{
public:
    spell_electron_poison_bomb() : SpellScriptLoader("spell_electron_poison_bomb") { }

    class spell_electron_poison_bomb_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_electron_poison_bomb_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            return true;
        }

        bool Load()
        {
            return true;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            if (!GetCaster())
                return;
            if (!GetCaster()->ToCreature())
                return;
            GetCaster()->CastSpell(GetCaster(), SPELL_POISON_EXPLOSION, true);
            GetCaster()->CastSpell(GetCaster(), SPELL_POISON_PUDDLE, true);
            GetCaster()->SetDisplayId(11686);
            GetCaster()->GetMotionMaster()->Clear();
            GetCaster()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
            GetCaster()->ToCreature()->DespawnOrUnsummon(20000);
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(spell_electron_poison_bomb_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_electron_poison_bomb_AuraScript();
    }
};

class spell_tron_activated_dummy : public SpellScriptLoader
{
public:
    spell_tron_activated_dummy() : SpellScriptLoader("spell_tron_activated_dummy") { }

    class spell_tron_activated_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_tron_activated_dummy_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit *target = GetCaster())
            {
                Powers powerType = Powers(3);
                target->ModifyPower(powerType, -1);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_tron_activated_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_tron_activated_dummy_SpellScript();
    }
};

class spell_magmatron_flame_launch : public SpellScriptLoader
{
    public:
        spell_magmatron_flame_launch() : SpellScriptLoader("spell_magmatron_flame_launch") { }

        class spell_magmatron_flame_launch_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_magmatron_flame_launch_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targets.clear();
                if (Unit *caster = GetCaster())
                    if (InstanceScript *instance = caster->GetInstanceScript())
                        if (Creature *magmatron = Creature::GetCreature(*caster, instance->GetData64(DATA_MAGMATRON_GUID)))
                        {
                            ThreatContainer::StorageType const &threatList = magmatron->getThreatManager().getThreatList();
                            for (ThreatContainer::StorageType::const_iterator itr = threatList.begin(); itr != threatList.end(); ++itr)
                                if (Unit* target = (*itr)->getTarget())
                                    if (magmatron->isInFrontInMap(target, 200.0f, static_cast<float>(M_PI / 6)))
                                        targets.push_back(target);

                            if (targets.size() > 1)
                                magmatron->AI()->DoAction(ACTION_FAIL_ACHIEVEMENT);
                        }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_magmatron_flame_launch_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_104);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_magmatron_flame_launch_SpellScript();
        }
};

class spell_chemical_cloud_bwd : public SpellScriptLoader
{
public:
    spell_chemical_cloud_bwd() : SpellScriptLoader("spell_chemical_cloud_bwd") { }

    class poisonBombCheck
    {
    public:
        bool operator()(WorldObject* target) const
        {
            if (target->GetTypeId() != TYPEID_UNIT)
                return true;
            if (Creature* creature = target->ToCreature())
                if (creature->GetEntry() == NPC_ARCANOTRON || creature->GetEntry() == NPC_ELECTRON ||
                    creature->GetEntry() == NPC_MAGMATRON || creature->GetEntry() == NPC_TOXITRON)
                    return false;
            return true;
        }
    };


    class spell_chemical_cloud_bwd_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_chemical_cloud_bwd_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(poisonBombCheck());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_chemical_cloud_bwd_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_chemical_cloud_bwd_SpellScript();
    }
};

class spell_nefarius_overcharge : public SpellScriptLoader
{
public:
    spell_nefarius_overcharge() : SpellScriptLoader("spell_nefarius_overcharge") { }

    class spell_nefarius_overcharge_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefarius_overcharge_SpellScript);

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            if (Unit *target = GetHitUnit())
            {
                uint32 spellId = sSpellMgr->GetSpellIdForDifficulty(SPELL_POWER_GENERATOR_AURA, target->ToUnit());
                target->RemoveAurasDueToSpell(spellId);
                target->AddAura(SPELL_OVERCHARGED_GENERATOR, target);
                //                target->AddAura(SPELL_GROW_STACKER, target);
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_nefarius_overcharge_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_nefarius_overcharge_SpellScript();
    }
};

class spell_nefraius_encasting_shadow : public SpellScriptLoader
{
public:
    spell_nefraius_encasting_shadow() : SpellScriptLoader("spell_nefraius_encasting_shadow") { }

    class AcquiertTargetCheck
    {
    public:
        bool operator()(WorldObject* target) const
        {
            uint32 spellId = sSpellMgr->GetSpellIdForDifficulty(SPELL_ACQUIRING_TARGET, target->ToUnit());
            return (!target->ToUnit()->HasAura(spellId));
        }
    };


    class spell_nefraius_encasting_shadow_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefraius_encasting_shadow_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(AcquiertTargetCheck());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_nefraius_encasting_shadow_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_nefraius_encasting_shadow_SpellScript();
    }
};

// 92048
class spell_nefraius_lighting_infusion : public SpellScriptLoader
{
public:
    spell_nefraius_lighting_infusion() : SpellScriptLoader("spell_nefraius_lighting_infusion") { }

    class LightingCheck
    {
    public:
        bool operator()(WorldObject* target) const
        {
            uint32 spellId = sSpellMgr->GetSpellIdForDifficulty(SPELL_LIGHTNING_CONDUCTOR, target->ToUnit());
            return (!target->ToUnit()->HasAura(spellId));
        }
    };


    class spell_nefraius_lighting_infusion_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefraius_lighting_infusion_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(LightingCheck());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_nefraius_lighting_infusion_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_nefraius_lighting_infusion_SpellScript();
    }
};

// 92050
class spell_nefarius_lighting_infusion_trigger : public SpellScriptLoader
{
public:
    spell_nefarius_lighting_infusion_trigger() : SpellScriptLoader("spell_nefarius_lighting_infusion_trigger") { }

    class spell_nefarius_lighting_infusion_trigger_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefarius_lighting_infusion_trigger_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            if (Unit *target = GetHitUnit())
            {
                uint32 spellId = sSpellMgr->GetSpellIdForDifficulty(SPELL_LIGHTNING_CONDUCTOR, target);
                if (Aura *aura = target->GetAura(spellId))
                {
                    int32 duration = aura->GetDuration();
                    target->RemoveAurasDueToSpell(spellId);
                    target->CastSpell(target, SPELL_SHADOW_CONDUCTOR, true);
                    if (Aura *aura2 = target->GetAura(SPELL_SHADOW_CONDUCTOR))
                        aura2->SetDuration(duration);
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_nefarius_lighting_infusion_trigger_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_nefarius_lighting_infusion_trigger_SpellScript();
    }
};

void AddSC_boss_omnotron_defense_system()
{
    new boss_omnotron(); // Verified
    new npc_arcanotron(); // Recently Verified
    new npc_electron(); // Recently  Verified
    new npc_magmatron(); // Recently Verified
    new npc_toxitron(); // Recently Verified
    new npc_victor_nefarius_omnotron(); // Recently Verified
    new npc_power_generator(); // Recently Verified
    new spell_power_generator_periodic(); // Verified
    new spell_shadow_conductor(); // Verified
    new npc_test();
    new spell_electron_lighting_charge(); // Not Yet Verified
    new spell_electron_shield();
    new spell_magmatron_shield();
    new spell_electron_poison_bomb();
    new spell_tron_activated_dummy();
    new spell_magmatron_flame_launch();
    new spell_chemical_cloud_bwd();

    // heroic
    new spell_nefarius_overcharge();
    new spell_nefraius_encasting_shadow();
    new spell_nefraius_lighting_infusion(); // Recently Verified
    new spell_nefarius_lighting_infusion_trigger(); // Recently Verified
}
