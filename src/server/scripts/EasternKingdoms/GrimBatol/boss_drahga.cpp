
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "grimbatol.h"

enum SayDrahga
{
    SAY_ENGAGE          = 0,
    SAY_SUM_SPIRIT      = 1,
    SAY_SUM_VALIONA     = 2,
    SAY_SLAY            = 3,
    SAY_DEATH           = 4,
    EMOTE_SUMMON        = 5
};

enum SayValiona
{
    SAY_KILL_YOU        = 0,
    SAY_NOT_DIE         = 1,
    EMOTE_FLAMES        = 2
};

enum Spells
{
    // Drahgas Spells
    SPELL_BURNING_SHADOWBOLT                = 75245,
    SPELL_BURNING_SHADOWBOLT_H              = 90915,

    SPELL_INVOCATION_OF_FLAME               = 75218, // Summons Trigger at Random Location
    SPELL_INVOCATION_TRIGGER                = 75222, // Summons & Visual is casted by the Trigger

    SPELL_TWILIGHT_PROTECTION               = 76303,

    // Valionas Spells
    SPELL_VALIONAS_FLAME                    = 75321,
    SPELL_SHREDDING_SWIPE                   = 75271,
    SPELL_SEEPING_TWILIGHT                  = 75274,
    SPELL_DEVOURING_FLAME_TRACKER_SUMMON    = 90944,
    SPELL_DEVOURING_FLAMES_H                = 90950,
    SPELL_TWILIGHT_SHIFT                    = 75328,

    // Invoked Flame Spirits Spells
    SPELL_AURA_NOVA                         = 75235,
    SPELL_CONCENTRATE_FIRE_AURA             = 82850,

    SPELL_SUPERNOVA                         = 75238,
    SPELL_SUPERNOVA_H                       = 90972,
};

enum Phase
{
    PHASE_CASTER_PHASE      = 1,
    PHASE_DRAGON_PHASE      = 2,
    PHASE_FINAL_PHASE       = 3,
    PHASE_NON               = 4,
};

enum Events
{
    EVENT_BURNING_SHADOWBOLT            = 1,
    EVENT_SUMMON_INVOKED_FLAME_SPIRIT   = 2,
    EVENT_VALIONAS_FLAME                = 3,
    EVENT_SHREDDING_SWIPE               = 4,
    EVENT_SEEPING_TWILIGHT              = 5,
    EVENT_DEVOURING_FLAMES              = 6,
    EVENT_DRAHGA_ENTER_VEHICLE          = 7,
};

enum Actions
{
    ACTION_DRAHGA_CALLS_VALIONA_FOR_HELP    = 1,
    ACTION_VALIONA_SHOULD_FLY_AWAY          = 2,
    ACTION_DRAHGA_IS_ON_THE_GROUND          = 3,
};

enum Points
{
    POINT_VALIONA_FLY_IN_THE_AIR    = 1,
    POINT_VALIONA_LAND              = 2,
    POINT_VALIONA_FLY_AWAY          = 3,
    POINT_VALIONA_IS_AWAY           = 4,
    POINT_DRAHGA_GO_TO_THE_LAVA     = 5,
};

Position const position[5] =
{
    {-400.613f, -671.578f, 265.896f, 0.102f},   // Drahga Point from who he jump down
    {-388.189f, -668.078f, 280.316f, 3.470f},   // Valionas Way to the Platform
    {-435.54f, -695.072f, 280.316f, 3.4010f},
    {-435.54f, -695.072f, 268.687f, 3.4010f},   // Valiona first land Position
    {-375.742f, -519.749f, 300.663f, 0.0f}      // Valionas End Position
};

class boss_drahga_shadowburner : public CreatureScript
{
public:
    boss_drahga_shadowburner() : CreatureScript("boss_drahga_shadowburner") {}

    struct boss_drahga_shadowburnerAI : public ScriptedAI
    {
        boss_drahga_shadowburnerAI(Creature* creature) : ScriptedAI(creature), summons(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            timerFlameSpirit = 10000;
            valionaGUID = 0;
            isCasting = false;
            SaveDespawnCreatures();
            events.Reset();
            events.SetPhase(PHASE_NON);
            if (instance)
                instance->SetData(DATA_DRAHGA_SHADOWBURNER_EVENT, NOT_STARTED);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NOT_SELECTABLE);
        }

        void EnterCombat(Unit* /*pWho*/)
        {
            if (instance)
                instance->SetData(DATA_DRAHGA_SHADOWBURNER_EVENT, IN_PROGRESS);
            events.SetPhase(PHASE_CASTER_PHASE);
            me->SetReactState(REACT_AGGRESSIVE);
            Talk(SAY_ENGAGE);
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveChase(me->getVictim());
            events.ScheduleEvent(EVENT_BURNING_SHADOWBOLT, 2000);
            events.ScheduleEvent(EVENT_SUMMON_INVOKED_FLAME_SPIRIT, timerFlameSpirit);
        }

        void JustSummoned(Creature* summon)
        {
            if(summon->GetEntry() == NPC_INVOCATION_OF_THE_FLAME_STALKER)
            {
                summon->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NOT_SELECTABLE);
                summon->AI()->DoCast(SPELL_INVOCATION_TRIGGER);
            }
            summon->SetInCombatWithZone();
        }

        void JustDied(Unit * /*victim*/)
        {
            events.Reset();
            Talk(SAY_DEATH);
            SaveDespawnCreatures();
            if (instance)
                instance->SetData(DATA_DRAHGA_SHADOWBURNER_EVENT, DONE);
        }

        void KilledUnit(Unit* victim)
        {
            Talk(SAY_SLAY);
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE)
            {
                switch (id)
                {
                    case POINT_DRAHGA_GO_TO_THE_LAVA:
                        Talk(SAY_SUM_VALIONA);
                        if (Creature *valiona = Unit::GetCreature(*me,  valionaGUID))
                        {
                            valiona->GetAI()->DoAction(ACTION_DRAHGA_CALLS_VALIONA_FOR_HELP);
                            me->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, valiona, false);
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        }
                        events.ScheduleEvent(EVENT_DRAHGA_ENTER_VEHICLE,2000);
                        break;
                     default:
                         break;
                }
            }
        }

        void DoAction(const int32 action)
        {
            switch(action)
            {
               case ACTION_DRAHGA_IS_ON_THE_GROUND:
                   me->SetReactState(REACT_AGGRESSIVE);
                   events.ScheduleEvent(EVENT_BURNING_SHADOWBOLT, 2000);
                   events.ScheduleEvent(EVENT_SUMMON_INVOKED_FLAME_SPIRIT, timerFlameSpirit);
                   break;
                case ACTION_VALIONA_SHOULD_FLY_AWAY:
                    events.SetPhase(PHASE_FINAL_PHASE);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->RemoveAllAuras();
                    valionaGUID = 0;
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (events.IsInPhase(PHASE_CASTER_PHASE) && !HealthAbovePct(30))
            {
                events.SetPhase(PHASE_DRAGON_PHASE);
                me->SetSpeed(MOVE_RUN, 1.5f);
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->CastStop();
                me->RemoveAllAuras(); // He should not die when he is jumping down...
                DoCast(me, SPELL_TWILIGHT_PROTECTION, true);
                timerFlameSpirit = events.GetNextEventTime(EVENT_SUMMON_INVOKED_FLAME_SPIRIT) - events.GetTimer();
                events.Reset(); // He Should not cast while he is flying
                me->GetMotionMaster()->MovePoint(POINT_DRAHGA_GO_TO_THE_LAVA, position[0]);
                if (Creature *valiona = me->SummonCreature(NPC_VALIONA, -375.33f, -667.291f, 250.0f, 3.29545f, TEMPSUMMON_CORPSE_DESPAWN))
                    valionaGUID = valiona->GetGUID();
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            isCasting = false;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_BURNING_SHADOWBOLT:
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(pTarget, SPELL_BURNING_SHADOWBOLT);
                        events.ScheduleEvent(EVENT_BURNING_SHADOWBOLT, 2000);
                        isCasting = true;
                    break;
                    case EVENT_SUMMON_INVOKED_FLAME_SPIRIT:
                        Talk(SAY_SUM_SPIRIT);
                        Talk(EMOTE_SUMMON);
                        DoCast(SPELL_INVOCATION_OF_FLAME);
                        events.ScheduleEvent(EVENT_SUMMON_INVOKED_FLAME_SPIRIT, 20000);
                        isCasting = true;
                        break;
                    default:
                        break;
                }

                if (isCasting)
                    break;
            }
            DoMeleeAttackIfReady();
        }

    private:

        void DespawnCreatures(uint32 entry, float distance)
        {
            std::list<Creature*> creatures;
            GetCreatureListWithEntryInGrid(creatures, me, entry, distance);

            if (creatures.empty())
                return;

            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                (*iter)->DespawnOrUnsummon();
        }

        void SaveDespawnCreatures()
        {
            DespawnCreatures(NPC_INVOCATION_OF_THE_FLAME_STALKER,500.0f); // Maybe the Core crashes on unload the Trigger
            DespawnCreatures(NPC_INVOKED_FLAMING_SPIRIT,500.0f);
            DespawnCreatures(NPC_SEEPING_TWILIGHT_TRIGGER,500.0f);
            DespawnCreatures(NPC_VALIONA,500.0f);
            DespawnCreatures(NPC_DEVOURING_FLAMES, 500.0f);
        }

        InstanceScript* instance;
//        Phase phase;
        EventMap events;
        SummonList summons;
        
        uint32 timerFlameSpirit;

        uint64 valionaGUID;

        bool isCasting;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_drahga_shadowburnerAI(creature);
    }
};

class mob_valiona_gb : public CreatureScript
{
public:
    mob_valiona_gb() : CreatureScript("mob_valiona_gb") { }

    struct mob_valiona_gbAI : public ScriptedAI
    {
        mob_valiona_gbAI(Creature* creature) : ScriptedAI(creature), summons(creature)
        {
            pInstance = creature->GetInstanceScript();
        }

        void Reset()
        {
            SetCombatMovement(false);
            me->SetSpeed(MOVE_WALK, 3.0f);
            me->SetCanFly(true);
            events.Reset();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            drahgaGUID = 0;
            mui_checkWipe = 2000;
        }

        void EnterCombat(Unit* /*pWho*/) {}

        void DamageTaken(Unit* done_by, uint32 &damage)
        {
            if (!HealthAbovePct(20))
            {
                me->GetVehicleKit()->RemoveAllPassengers();
                if (Creature *drahga = Unit::GetCreature(*me, drahgaGUID))
                    drahga->AI()->DoAction(ACTION_VALIONA_SHOULD_FLY_AWAY);
                DoAction(ACTION_VALIONA_SHOULD_FLY_AWAY);
            }
        }

        void JustSummoned(Creature* summon)
        {
            summon->setActive(true);
            if(summon->GetEntry() == NPC_SEEPING_TWILIGHT_TRIGGER)
            {
                summon->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NOT_SELECTABLE);
                summon->GetAI()->DoCast(SPELL_SEEPING_TWILIGHT);
            }
            if(me->isInCombat())
                summon->AI()->DoZoneInCombat();
        }

        void IsSummonedBy(Unit* summoner)
        {
            drahgaGUID = summoner->GetGUID();
            me->SetReactState(REACT_PASSIVE);
        }

        bool checkWipe()
        {
            Map::PlayerList const& players = pInstance->instance->GetPlayers();

            if (!players.isEmpty())
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                    if (Player* player = itr->getSource())
                        if (player->isAlive() && !player->isGameMaster())
                            return false;
            return true;
        }

        void JustReachedHome()
        {
            if (Creature *drahga = Unit::GetCreature(*me, drahgaGUID))
                drahga->AI()->EnterEvadeMode();
            me->DespawnOrUnsummon(500);
        }

        void UpdateAI(const uint32 diff)
        {
            if (mui_checkWipe <= diff)
            {
                if (checkWipe())
                {
                    if (Creature *drahga = Unit::GetCreature(*me, drahgaGUID))
                        drahga->AI()->EnterEvadeMode();
                    me->DespawnOrUnsummon(500);
                }
                mui_checkWipe = 2000;
            }
            else mui_checkWipe -= diff;

            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            isCasting = false;

            while (uint32 eventId = events.GetEvent())
            {
                switch (eventId)
                {
                    case EVENT_VALIONAS_FLAME:
                        if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(pTarget, SPELL_VALIONAS_FLAME);
                        isCasting = true;
                        events.RepeatEvent(urand(15000,25000));
                        break;
                    case EVENT_SHREDDING_SWIPE:
                        if(me->getVictim())
                            DoCastVictim(SPELL_SHREDDING_SWIPE);
                        isCasting = true;
                        events.RepeatEvent(urand(21000,30000));
                        break;
                    case EVENT_DEVOURING_FLAMES:
                        Talk(EMOTE_FLAMES);
                //        DoCast(SelectTarget(SELECT_TARGET_RANDOM, 0), SPELL_DEVOURING_FLAME_TRACKER_SUMMON);
                        if (const Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        {
                            if (Creature* trigger = me->SummonCreature(NPC_DEVOURING_FLAMES, *target, TEMPSUMMON_TIMED_DESPAWN, 20000))
                            {
                                me->StopMoving();
                                me->SetFacingToObject(trigger);
                                me->SendMeleeAttackStop();
                            }
                        }
                        DoCast(SPELL_DEVOURING_FLAMES_H);
                        isCasting = true;
                        events.RepeatEvent(urand(30000,35000));
                        break;
                    default:
                        events.PopEvent();
                        break;
                }
                if (isCasting)
                    break;
            }
            DoMeleeAttackIfReady();
        }

        void DoAction(const int32 action)
        {
            switch(action)
            {
                case ACTION_DRAHGA_CALLS_VALIONA_FOR_HELP:
                    Talk(SAY_KILL_YOU);
                    DoZoneInCombat();
                    currentWaypoint = 1;
                    me->GetMotionMaster()->MovePoint(POINT_VALIONA_FLY_IN_THE_AIR, position[1]);
                    break;
                case ACTION_VALIONA_SHOULD_FLY_AWAY:
                    Talk(SAY_NOT_DIE);
                    SetCombatMovement(false);
                    events.Reset();
                    me->RemoveAllAuras();
                    DoCast(me, SPELL_TWILIGHT_SHIFT, true);
                    me->SetReactState(REACT_PASSIVE);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->SetCanFly(true);
                    me->GetMotionMaster()->MovePoint(POINT_VALIONA_FLY_AWAY, position[1]);
                    break;
                default:
                    break;
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            switch (id)
            {
                case POINT_VALIONA_FLY_IN_THE_AIR:
                    currentWaypoint++;
                    if (currentWaypoint < 3)
                        me->GetMotionMaster()->MovePoint(POINT_VALIONA_FLY_IN_THE_AIR, position[currentWaypoint]);
                    else
                        me->GetMotionMaster()->MovePoint(POINT_VALIONA_LAND, position[3]);
                    break;
                case POINT_VALIONA_LAND:
                    SetCombatMovement(true);
                    me->SetReactState(REACT_AGGRESSIVE);
                    if (Creature *drahga = Unit::GetCreature(*me, drahgaGUID))
                        drahga->GetAI()->DoAction(ACTION_DRAHGA_IS_ON_THE_GROUND);
                    me->SetSpeed(MOVE_WALK, 1.0f);
                    me->SetCanFly(false);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveChase(me->getVictim());
                    events.ScheduleEvent(EVENT_VALIONAS_FLAME, urand(4000,7000));
                    events.ScheduleEvent(EVENT_SHREDDING_SWIPE, urand(10000,13000));
                    if (IsHeroic())
                        events.ScheduleEvent(EVENT_DEVOURING_FLAMES, urand(15000,17000));
                    break;
                case POINT_VALIONA_FLY_AWAY:
                    DespawnCreatures(NPC_SEEPING_TWILIGHT_TRIGGER,500.0f);
                    me->GetMotionMaster()->MovePoint(POINT_VALIONA_IS_AWAY, position[4]);
                    break;
                case POINT_VALIONA_IS_AWAY:
                    me->DespawnOrUnsummon();
                    break;
                default:
                    break;
            }
        }

    private:

        void DespawnCreatures(uint32 entry, float distance)
        {
            std::list<Creature*> creatures;
            GetCreatureListWithEntryInGrid(creatures, me, entry, distance);

            if (creatures.empty())
                return;

            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                (*iter)->DespawnOrUnsummon();
        }

        InstanceScript* pInstance;
        EventMap events;
        uint8 currentWaypoint;
        SummonList summons;
        uint64 drahgaGUID;
        uint32 mui_checkWipe;
        bool isCasting;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_valiona_gbAI(creature);
    }
};

class mob_invoked_flame_spirit : public CreatureScript
{
public:
    mob_invoked_flame_spirit() : CreatureScript("mob_invoked_flame_spirit") { }

    struct mob_invoked_flame_spiritAI : public ScriptedAI
    {
        mob_invoked_flame_spiritAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            targetGUID = 0;
            checkTargetTimer = 1000;
        }

        void IsSummonedBy(Unit* summoner)
        {
            DoZoneInCombat();
            if (Player *player = me->FindNearestPlayer(100.0f))
            {
                me->SetReactState(REACT_PASSIVE);
                me->CastSpell(player, SPELL_CONCENTRATE_FIRE_AURA, true);
                me->ClearUnitState(UNIT_STATE_CASTING);
                me->GetMotionMaster()->MoveFollow(player, 0.0f, 0.0f);
                targetGUID = player->GetGUID();
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return ;
            if (checkTargetTimer <= diff)
            {
                if (Player *player = Unit::GetPlayer(*me, targetGUID))
                    if (me->GetDistance(player) < 1.0f)
                    {
                        me->RemoveAura(SPELL_CONCENTRATE_FIRE_AURA);
                        DoCastVictim(RAID_MODE(SPELL_SUPERNOVA,    SPELL_SUPERNOVA_H));
                        me->DespawnOrUnsummon();
                    }
                checkTargetTimer = 1000;
            }
            else checkTargetTimer -= diff;
        }

    private :
        uint64 targetGUID;
        uint32 checkTargetTimer;
        bool first;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_invoked_flame_spiritAI(creature);
    }
};

class spell_devouring_flames_grim : public SpellScriptLoader // 92051
{
public:
    spell_devouring_flames_grim() : SpellScriptLoader("spell_devouring_flames_grim") { }

    class spell_devouring_flames_grim_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_devouring_flames_grim_SpellScript);

        void CalculateDamage(SpellEffIndex /*effIndex*/)
        {
            if (!GetHitUnit())
                return;

            if (Unit* caster = GetCaster())
            {
                float distance = caster->GetDistance2d(GetHitUnit());

                if (distance > 1.0f)
                    SetHitDamage(int32(GetHitDamage() - (500 * distance)));

                if (GetHitDamage() < 0)
                    SetHitDamage(0);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_devouring_flames_grim_SpellScript::CalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_devouring_flames_grim_SpellScript();
    }
};

void AddSC_boss_drahga_shadowburner()
{
    new boss_drahga_shadowburner();
    new mob_valiona_gb();
    new mob_invoked_flame_spirit();
    new spell_devouring_flames_grim();
}
