
#include "AccountMgr.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "throne_of_the_four_winds.h"

enum Spells
{
    SPELL_TOO_CLOSE_KNOCK_BACK_EFFECT = 90663,
    SPELL_FIXATE_ON_HIT_PASSIVE = 92039,

    // phase 1
    SPELL_ICE_STORM_SUMMON         = 88239,
    SPELL_LIGHTNING_STRIKE         = 91327,
    SPELL_LIGHTNING_STRIKE_TRIGGER = 91326,
    SPELL_ELECTROCUTE              = 88427,
    SPELL_SUMMON_SQUALL_LINE_SE    = 91104,
    SPELL_SUMMON_SQUALL_LINE_SW    = 88781,
    SPELL_SQUALL_LINE              = 91129,
    SPELL_SQUALL_LINE_2            = 91110,
    SPELL_SQUALL_LINE_PERIODIC     = 87652,
    SPELL_WIND_BURST               = 87770,

    SPELL_SUMMON_CHEST             = 95386,

    // phase 2
    SPELL_ACID_RAIN                = 88290,
    SPELL_ACID_RAIN_2              = 91216,
    SPELL_SUMMON_STORMLING         = 88272,
    SPELL_STATIC_SHOCK             = 87873,

    // phase 3
    SPELL_BIG_RAIN                 = 82651,
    SPELL_RELENTLESS_STORM_INITIAL_VEHICLE_RIDE_TRIGGER = 89528,
    SPELL_RELENTLESS_STORM_INITIAL_VEHICLE_RIDE = 89527,
    SPELL_RELENTLESS_STORM_CHANNEL = 88875,
    SPELL_EYE_OF_THE_STORM         = 89107,
    SPELL_WIND_BURST_2             = 89638,
    SPELL_LIGHTNING                = 89644,
    SPELL_LIGHTNING_ROD            = 89690,
    SPELL_LIGHTNING_2              = 89641,
    SPELL_LIGHTNING_CLOUD_SUMMON   = 89565,
    SPELL_LIGHTNING_CLOUD_PERIODIC = 89639,
    SPELL_LIGHTING_CLOUD_DUMMY     = 89628,

    // ice storm SUMMONER
    SPELL_ICE_STORM_PERIODIC       = 87053,
    SPELL_ICE_STORM                = 87408,
    SPELL_ICE_STORM_MISSLE         = 87103,
    SPELL_ICE_STORM_2              = 87406,
    // ice storm
    SPELL_ICESTORM_PERSISTENT_AREA_AURA = 87472,
    SPELL_ICESTORM_PERIODIC        = 87469,

    // squall line
    SPELL_SQUALL_LINE_TRIGGER      = 88779,
    SPELL_SQUALL_LINE_BOARDED      = 87856,
    SPELL_SQUALL_LINE_AURA         = 87621,

    // stormling
    SPELL_STORMLING                = 87913,
    SPELL_STORMLING_SUMMON         = 87914,
    // stormlings summon o0?
    SPELL_STORMLING_PERIODIC       = 87906,
    SPELL_STORMLING_PERIODIC_2     = 87905,
    SPELL_FEEDBACK                 = 87904,
    // lightning cloud
    SPELL_LIGHTNING_CLOUDS_SUMMON  = 89577,
    SPELL_LIGHTNING_CLOUDS         = 89575,
    // lightning cloud's summon
    SPELL_LIGHTNING_CLOUDS_PERIODIC = 89564,
    SPELL_LIGHTNING_CLOUDS_DUMMY    = 89569,
    SPELL_LIGHTING_CLOUD_DAMAGE     = 89587,
    SPELL_LIGHTING_STRIKE_DUMMY     = 88230,
    SPELL_LIGHTING_STRIKE_CONE_DAMAGE = 88214,
    SPELL_LIGHTING_STRIKE_2         = 93247,
    SPELL_LIGHTING_STRIKE_SC_EFF    = 88238,
    SPELL_LIGHTING_STRIKE_PLAYER    = 95764,
    // npc fall catcher
    SPELL_FALL_CATCHER              = 85282,
    SPELL_EJECT_PASSENGER           = 68576,
    SPELL_CATCH_FALL                = 85274,
    SPELL_CATCH_FALL_TRIGGER_1      = 85275,
    SPELL_CATCH_FALL_TRIGGER_2      = 85269,
    SPELL_CATCHER_SUMMON            = 85268,

    SPELL_PLAYERS_CAN_FLIGHT        = 82724,
};

enum Events
{
    EVENT_ICE_STORM = 1,
    EVENT_ELECTROCUTE,
    EVENT_LIGHTING_STRIKE,
    EVENT_SQUALL_LINE_W,
    EVENT_SQUALL_LINE_E,
    EVENT_WIND_BURST,

    EVENT_ACID_RAIN,
    EVENT_SUMMON_STORMLING,
    EVENT_STATIC_SHOCK,

    EVENT_REMOVE_ACID_RAID,
    EVENT_WIND_BURST_P3,
    EVENT_LIGHTING_CLOUD,
    EVENT_LIGHTING_CLOUD_SUMMON,
    EVENT_LIGHTING_ROD,
    EVENT_LIGHTING,

    EVENT_DESTROY_PLATFORM,
};

enum Npcs
{
    NPC_ICESTORM_SUMMONER = 46734,
    NPC_ICESTORM = 46973,
    NPC_ICESTORM_STARTER = 46766,
    NPC_INFINITE_AOE_TRIGGER = 36171,
    NPC_LIGHTING_STRIKE_TRIGGER = 48977,
    NPC_SQUALL_LINE_VEHICLE_W = 47034,
    NPC_SQUALL_LINE_W = 48854,
    NPC_SQUALL_LINE_VEHICLE_E = 48852,
    NPC_SQUALL_LINE_E = 48855,
    NPC_FALL_CATCHER = 45710,
    NPC_WORLD_TRIGGER = 21252,
    NPC_STORMLING_PRE_EFFECT = 47177,
    NPC_STORMLING = 47175,
    NPC_RELENTLESS_STORM_INITIAL_VEHICLE = 47806,
    NPC_LIGHTING_CLOUD = 48190,
    NPC_LIGHTING_CLOUD_EXTRA_VISUAL = 48196,
};

enum alak_phases
{
    PHASE_INTRO,
    PHASE_1,
    PHASE_2,
    PHASE_3,
    PHASE_OUTRO,
};

class MoveAroundPlatformEvent : public BasicEvent
{
public:
    MoveAroundPlatformEvent(Unit* alakir, Unit *trash, uint32 speed, float radius, float orient, bool sens, float precision) : _alakir(alakir), _trash(trash), _speed(speed), _radius(radius), _orient(orient), _sens(sens), _precision(precision)
    {
    }

    bool Execute(uint64 execTime, uint32 /*diff*/)
    {
        float x, y;
        _alakir->GetNearPoint2D(x, y, _radius, _orient);
        _trash->GetMotionMaster()->Clear();
        _trash->GetMotionMaster()->MovePoint(0, x, y,  _alakir->GetPositionZ());
        if (!_trash->HasAura(SPELL_ICE_STORM_PERIODIC) && _trash->ToCreature()->GetEntry() == NPC_ICESTORM_SUMMONER)
            _trash->ToCreature()->DespawnOrUnsummon(1000);
        else
            _trash->m_Events.AddEvent(new MoveAroundPlatformEvent(_alakir, _trash, _speed, _radius, _orient + (_sens ? _precision : -_precision), _sens, _precision), execTime + _speed);
        return false;
    }

private:
    Unit* _alakir;
    Unit *_trash;
    uint32 _speed;
    float _radius;
    float _orient;
    bool _sens;
    float _precision;
};

class SquallBoardEvent : public BasicEvent
{
public:
    SquallBoardEvent(Unit *trash) : _trash(trash)
    {
    }


    bool Execute(uint64 execTime, uint32 /*diff*/)
    {
        std::list<Player*> targets = _trash->GetPlayersInRange(1, true);
        for (std::list<Player*>::iterator itr = targets.begin(); itr != targets.end(); itr++)
            if (Player *player = *itr)
                if (!player->HasAura(SPELL_SQUALL_LINE_BOARDED) && !player->HasAura(95751) && !player->GetVehicle() && !player->HasAura(85269))
                    player->CastSpell(_trash, SPELL_SQUALL_LINE_BOARDED, true);
        if (!_trash->HasAura(SPELL_SQUALL_LINE_PERIODIC))
            _trash->AddAura(SPELL_SQUALL_LINE_PERIODIC, _trash);
        _trash->m_Events.AddEvent(this, execTime + 500);
        return false;
    }

private:
    Unit *_trash;
};

class IntroP3VisualEvent : public BasicEvent
{
public:
    IntroP3VisualEvent(Unit *trash, uint64 playerGUID) : _trash(trash), _playerGUID(playerGUID)
    {
    }

    bool Execute(uint64 execTime, uint32 /*diff*/)
    {
        if (_trash)
        {
            if (Unit* player = ObjectAccessor::GetUnit(*_trash, _playerGUID))
                player->UpdatePosition(player->GetPositionX(), player->GetPositionY(), _trash->GetPositionZ(), true);
            _trash->SetFacingTo(_trash->GetOrientation() + 0.01f);
            _trash->m_Events.AddEvent(this, execTime + 100);
        }
        return false;
    }
private:
    Unit *_trash;
    uint64 _playerGUID;
};

class EndIntroP3VisualEvent : public BasicEvent
{
public:
    EndIntroP3VisualEvent(Unit *trash, uint64 player) : _trash(trash), _playerGUID(player)
    {
    }

    bool Execute(uint64 execTime, uint32 /*diff*/)
    {
        if (!_trash)
            return true;
        if (Unit *_player = Unit::GetUnit(*_trash, _playerGUID))
            if (_player->GetTypeId() == TYPEID_PLAYER)
            {
                _player->ExitVehicle();
                _player->RemoveAurasDueToSpell(SPELL_RELENTLESS_STORM_INITIAL_VEHICLE_RIDE);
                _player->UpdatePosition(_player->GetPositionX(), _player->GetPositionY(), 341.0f, true);
                _player->ToPlayer()->SetViewpoint(_trash, false);
                _trash->ToCreature()->DespawnOrUnsummon(1000);
                _player->CastSpell(_player, SPELL_BIG_RAIN, true);
            }
        return true;
    }
private:
    Unit *_trash;
    uint64 _playerGUID;
};


class boss_alakir : public CreatureScript
{
public:
    boss_alakir() : CreatureScript("boss_alakir") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new boss_alakirAI(creature);
    }

    struct boss_alakirAI : public BossAI
    {
        boss_alakirAI(Creature * creature) : BossAI(creature, DATA_ALAKIR)
        {
        }

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PLAYERS_CAN_FLIGHT);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BIG_RAIN);
            instance->DoRemoveAurasDueToSpellOnPlayers(95751);
            instance->DoRemoveAurasDueToSpellOnPlayers(85269);
            if (GameObject *go = instance->instance->GetGameObject(instance->GetData64(GOB_CENTER_PLATFORM)))
                go->SetDestructibleState(GO_DESTRUCTIBLE_INTACT);
            _phase = PHASE_INTRO;
            if (instance->GetBossState(DATA_CONCLAVE_OF_WIND_EVENT) != DONE && instance->GetBossState(DATA_CONCLAVE_OF_WIND_EVENT) != DONE_HM)
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            HideTrigger(true);
            instance->DisableFallDamage(false);
            _Reset();
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_DISABLE_SLIPSTREAM_TRIGGER:
                    HideTrigger(true);
                    break;
                default:
                    break;
            }
        }

        void HideTrigger(bool hide)
        {
            std::list<Creature*> creatures;
            GetCreatureListWithEntryInGrid(creatures, me, NPC_WORLD_TRIGGER, 90);

            if (creatures.empty())
                return;

            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
            {
                if (hide)
                    (*iter)->DespawnOrUnsummon();
                else
                    (*iter)->Respawn(true);
            }
        }

        void JustSummoned(Creature * summon)
        {
            switch (summon->GetEntry())
            {
                case NPC_ICESTORM:
                    summon->CastWithDelay(2000, summon, SPELL_ICESTORM_PERSISTENT_AREA_AURA, false);
                    summon->CastWithDelay(5000, summon, SPELL_ICESTORM_PERIODIC, false);
                    summon->AddUnitState(UNIT_STATE_CANNOT_TURN);
                    summon->DespawnOrUnsummon(15000);
                    break;
                case NPC_ICESTORM_SUMMONER:
                {
                    summon->CastSpell(summon, SPELL_ICE_STORM_PERIODIC, false);
                    summon->CastWithDelay(2000, summon, SPELL_ICE_STORM, true);
                    summon->CastWithDelay(2000, summon, SPELL_ICE_STORM_MISSLE, true);
                    summon->CastWithDelay(2000, summon, SPELL_ICE_STORM_2, true);
                    if (Creature *trigger = me->FindNearestCreature(NPC_INFINITE_AOE_TRIGGER, 100, true))
                    {
                        float ray = trigger->GetDistance(summon);
                        if (ray < 45.0f || ray > 60.0f)
                        {
                            ray = 45.0f + frand(0, 15.0f);
                            float x, y;
                            trigger->GetNearPoint2D(x, y, ray, trigger->GetAngle(summon));
                            summon->NearTeleportTo(x, y,  summon->GetPositionZ(), 0.0f);
                        }
                        summon->m_Events.AddEvent(new MoveAroundPlatformEvent(trigger, summon, 1000, trigger->GetDistance(summon), trigger->GetAngle(summon), urand(0, 1), 0.1f), summon->m_Events.CalculateTime(200));
                    }
                    break;
                }
                case NPC_SQUALL_LINE_VEHICLE_W:
                {
                    bool specialPass = false;
                    Creature *trigger = me->FindNearestCreature(NPC_INFINITE_AOE_TRIGGER, 100, true);
                    if (!trigger)
                        break;
                    summon->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
                    float x, y;
                    trigger->GetNearPoint2D(x, y, 45, trigger->GetAngle(summon));
                    summon->NearTeleportTo(x, y,  summon->GetPositionZ(), 0.0f);
                    for (int nwSquallCnt = 0; nwSquallCnt < 8; nwSquallCnt++)
                    {
                        if ((rand() % 3 == 1 && !specialPass) || (nwSquallCnt == 4 && !specialPass))
                        {
                            nwSquallCnt++;
                            specialPass = true;
                        }
                        if (Creature *c = me->SummonCreature(NPC_SQUALL_LINE_W, x, y, summon->GetPositionZ()))
                        {
                            c->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
                            c->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, nwSquallCnt + 1, summon, TRIGGERED_IGNORE_CASTER_MOUNTED_OR_ON_VEHICLE);
                            c->m_Events.AddEvent(new SquallBoardEvent(c), c->m_Events.CalculateTime(5000));
                            c->DespawnOrUnsummon(48000);
                        }
                    }
                    summon->CastSpell(summon, SPELL_SQUALL_LINE_TRIGGER, true);
                    summon->m_Events.AddEvent(new MoveAroundPlatformEvent(trigger, summon, 250, 42, trigger->GetAngle(summon), true, 0.05f), summon->m_Events.CalculateTime(200));
                    summon->DespawnOrUnsummon(50000);
                    break;
                }
                case NPC_SQUALL_LINE_VEHICLE_E:
                {
                    bool specialPass = false;
                    Creature *trigger = me->FindNearestCreature(NPC_INFINITE_AOE_TRIGGER, 100, true);
                    if (!trigger)
                        break;
                    float x, y;
                    summon->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
                    trigger->GetNearPoint2D(x, y, 45, trigger->GetAngle(summon));
                    summon->NearTeleportTo(x, y,  summon->GetPositionZ(), 0.0f);
                    for (int nwSquallCnt = 0; nwSquallCnt < 8; nwSquallCnt++)
                    {
                        if ((rand() % 3 == 1 && !specialPass) || (nwSquallCnt == 4 && !specialPass))
                        {
                            nwSquallCnt++;
                            specialPass = true;
                        }
                        if (Creature *c = me->SummonCreature(NPC_SQUALL_LINE_W, x, y, summon->GetPositionZ()))
                        {
                            c->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
                            c->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, nwSquallCnt + 1, summon, TRIGGERED_IGNORE_CASTER_MOUNTED_OR_ON_VEHICLE);
                            c->m_Events.AddEvent(new SquallBoardEvent(c), c->m_Events.CalculateTime(5000));
                            c->DespawnOrUnsummon(48000);
                        }
                    }
                    summon->CastSpell(summon, SPELL_SQUALL_LINE_TRIGGER, true);
                    summon->m_Events.AddEvent(new MoveAroundPlatformEvent(trigger, summon, 250, 42, trigger->GetAngle(summon), false, 0.05f), summon->m_Events.CalculateTime(200));
                    summon->DespawnOrUnsummon(50000);
                    break;
                }
                case NPC_STORMLING_PRE_EFFECT:
                    summon->CastSpell(summon, SPELL_STORMLING, true);
                    summon->CastWithDelay(4000, summon, SPELL_STORMLING_SUMMON, true);
                    break;
                case NPC_STORMLING:
                    summon->CastSpell(summon, SPELL_STORMLING_PERIODIC, true);
                    summon->CastSpell(summon, SPELL_STORMLING_PERIODIC_2, true);
                    break;
                case NPC_LIGHTING_CLOUD:
                {
                    summon->DespawnOrUnsummon(RAID_MODE(20000, 20000, 30000, 30000));
                    float orient = 0.0f;
                    for (int i = 0; i < 12; i++)
                    {
                        float x, y;
                        //                        summon->GetNearPoint2D(x, y, frand(40, 80), orient - M_PI / 24);
                        //  summon->CastSpell(x, y, summon->GetPositionZ(), SPELL_LIGHTNING_CLOUDS_SUMMON, true);
                        summon->GetNearPoint2D(x, y, frand(60, 120), orient);
                        summon->CastSpell(x, y, summon->GetPositionZ(), SPELL_LIGHTNING_CLOUDS_SUMMON, true);
                        orient += 2.0f * M_PI / 12.0f;
                    }
                    summon->CastWithDelay(5000, summon, SPELL_LIGHTING_CLOUD_DAMAGE, true);
                    break;
                }
                case NPC_LIGHTING_CLOUD_EXTRA_VISUAL:
                    summon->DespawnOrUnsummon(RAID_MODE(20000, 20000, 30000, 30000));
                    summon->CastSpell(summon, SPELL_LIGHTNING_CLOUDS_PERIODIC, true);
                    summon->CastSpell(summon, SPELL_LIGHTNING_CLOUDS_DUMMY, true);
                    break;
                default:
                    break;
            }
            BossAI::JustSummoned(summon);
        }

        void EnterCombat(Unit * /*who*/)
        {
            HideTrigger(false);
            instance->DisableFallDamage(true);
            _phase = PHASE_1;
            Talk(0);
            events.SetPhase(_phase);
            DoCast(SPELL_TOO_CLOSE_KNOCK_BACK_EFFECT);
            instance->DoCastSpellOnPlayers(SPELL_BIG_RAIN);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PLAYERS_CAN_FLIGHT);
            events.ScheduleEvent(EVENT_ICE_STORM, 21000, 0, PHASE_1);
            events.ScheduleEvent(EVENT_ELECTROCUTE, 1000, 0, PHASE_1);
            events.ScheduleEvent(EVENT_LIGHTING_STRIKE, 9000, 0, PHASE_1);
            events.ScheduleEvent(EVENT_SQUALL_LINE_W, 5000, 0, PHASE_1);
            events.ScheduleEvent(EVENT_WIND_BURST, 25000, 0, PHASE_1);
            events.ScheduleEvent(EVENT_STATIC_SHOCK, 5000, 0, PHASE_1);
            _EnterCombat();
        }

        void DamageTaken(Unit* caster, uint32& damage)
        {
            if (me->HasReactState(REACT_PASSIVE))
                me->SetReactState(REACT_AGGRESSIVE);
            if (_phase == PHASE_1 && HealthBelowPct(81))
            {
                uint32 nextElectrocute = events.GetNextEventTime(EVENT_ELECTROCUTE);
                uint32 nextStaticShock = events.GetNextEventTime(EVENT_STATIC_SHOCK);
                _phase = PHASE_2;
                Talk(3);
                events.Reset();
                events.SetPhase(_phase);
                events.ScheduleEvent(EVENT_ACID_RAIN, 0, 0, PHASE_2);
                events.ScheduleEvent(EVENT_STATIC_SHOCK, nextStaticShock, 0, PHASE_2);
                events.ScheduleEvent(EVENT_SUMMON_STORMLING, 10000, 0, PHASE_2);
                events.ScheduleEvent(EVENT_ELECTROCUTE, nextElectrocute, 0, PHASE_2);
                events.ScheduleEvent(EVENT_SQUALL_LINE_W, 0, 0, PHASE_2);
            }
            else if (_phase == PHASE_2 && HealthBelowPct(26))
            {
                _phase = PHASE_3;
                Talk(5);
                summons.DespawnAll();
                events.Reset();
                events.SetPhase(_phase);
                events.ScheduleEvent(EVENT_REMOVE_ACID_RAID, 0, 0, PHASE_3);
                DoCast(SPELL_RELENTLESS_STORM_INITIAL_VEHICLE_RIDE_TRIGGER);
                DoCast(SPELL_RELENTLESS_STORM_CHANNEL);
                StartP3();
                events.ScheduleEvent(EVENT_WIND_BURST_P3, 5000, 0, PHASE_3);
                events.ScheduleEvent(EVENT_LIGHTING_CLOUD, 5000, 0, PHASE_3);
                events.ScheduleEvent(EVENT_LIGHTING_CLOUD_SUMMON, 20000, 0, PHASE_3);
                events.ScheduleEvent(EVENT_LIGHTING_ROD, 5000, 0, PHASE_3);
                events.ScheduleEvent(EVENT_LIGHTING, 8000, 0, PHASE_3);
                me->CastSpell(me->GetPositionX(), me->GetPositionY(), 192.0f, SPELL_LIGHTNING_CLOUD_SUMMON, true); // like on video's

            }
            else if (_phase == PHASE_3 && damage > me->GetHealth())
            {
                _phase = PHASE_OUTRO;
                Talk(6);
                DoCast(SPELL_SUMMON_CHEST);
            }
        }

        void StartP3()
        {
            me->GetMotionMaster()->Clear();
            DoStopAttack();
            SetCombatMovement(false);
            Map::PlayerList const& players = me->GetMap()->GetPlayers();
            if (!players.isEmpty())
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                    if (Player* player = itr->getSource())
                        if (player->isAlive() && !AccountMgr::IsGMAccount(player->GetSession()->GetSecurity()))
                            if (Creature *transportPl = me->SummonCreature(NPC_RELENTLESS_STORM_INITIAL_VEHICLE, player->GetPositionX(), player->GetPositionY(), 341.0f, 0.0f))
                            {
                                player->CastSpell(player, SPELL_PLAYERS_CAN_FLIGHT, true);
                                player->CastSpell(transportPl, SPELL_RELENTLESS_STORM_INITIAL_VEHICLE_RIDE, true);
                                player->SetViewpoint(transportPl, true);
                                transportPl->m_Events.AddEvent(new IntroP3VisualEvent(transportPl, player->GetGUID()), transportPl->m_Events.CalculateTime(100));
                                transportPl->m_Events.AddEvent(new EndIntroP3VisualEvent(transportPl, player->GetGUID()), transportPl->m_Events.CalculateTime(8000));
                            }
            HideTrigger(true);
            events.ScheduleEvent(EVENT_DESTROY_PLATFORM, 1000, 0, PHASE_3);
        }

        void JustDied(Unit * /*killer*/)
        {
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BIG_RAIN);
            instance->DoRemoveAurasDueToSpellOnPlayers(95751);
            instance->DoRemoveAurasDueToSpellOnPlayers(85269);
            instance->CompleteGuildCriteriaForGuildGroup(CRITERIA_GUILD_RUN_ALAKIR);
            instance->CompleteGuildCriteriaForGuildGroup(CRITERIA_GUILD_RUN_ALAKIR_2);
            _JustDied();
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
                switch (eventId)
                {
                    case EVENT_ICE_STORM:
                        DoCastRandom(SPELL_ICE_STORM_SUMMON, 0.0f);
                        events.ScheduleEvent(EVENT_ICE_STORM, urand(15000, 20000), 0, PHASE_1);
                        break;
                    case EVENT_ELECTROCUTE:
                        if (Unit *victim = me->getVictim())
                            if (victim->GetDistance2d(me->GetPositionX(), me->GetPositionY()) > 35.0f)
                                DoCastVictim(SPELL_ELECTROCUTE);
                        events.ScheduleEvent(EVENT_ELECTROCUTE, 1000, 0, _phase);
                        break;
                    case EVENT_LIGHTING_STRIKE:
                        me->CastCustomSpell(SPELL_LIGHTNING_STRIKE, SPELLVALUE_MAX_TARGETS, 1, me, false);
                        events.ScheduleEvent(EVENT_LIGHTING_STRIKE, urand(9000, 14000), 0, PHASE_1);
                        break;
                    case EVENT_SQUALL_LINE_W:
                        Talk(2);
                        me->CastSpell(me, SPELL_SUMMON_SQUALL_LINE_SW, true);
                        DoCast(SPELL_SQUALL_LINE);
                        events.ScheduleEvent(EVENT_SQUALL_LINE_E, urand(25000, 45000), 0, _phase);
                        break;
                    case EVENT_SQUALL_LINE_E:
                        Talk(2);
                        me->CastSpell(me, SPELL_SUMMON_SQUALL_LINE_SE, true);
                        DoCast(SPELL_SQUALL_LINE_2);
                        events.ScheduleEvent(EVENT_SQUALL_LINE_W, urand(25000, 45000), 0, _phase);
                        break;
                    case EVENT_WIND_BURST:
                        Talk(1);
                        DoCast(SPELL_WIND_BURST);
                        events.ScheduleEvent(EVENT_WIND_BURST, urand(25000, 30000), 0, PHASE_1);
                        break;
                    case EVENT_SUMMON_STORMLING:
                        Talk(5);
                        DoCastRandom(SPELL_SUMMON_STORMLING, 0.0f);
                        events.ScheduleEvent(EVENT_SUMMON_STORMLING, 20000, 0, PHASE_2);
                        break;
                    case EVENT_ACID_RAIN:
                        DoCast(SPELL_ACID_RAIN);
                        break;
                    case EVENT_REMOVE_ACID_RAID:
                        me->RemoveAurasDueToSpell(RAID_MODE(88290, 101451, 101452, 101453));
                        DoCast(SPELL_ACID_RAIN_2);
                        break;
                    case EVENT_WIND_BURST_P3:
                        DoCast(SPELL_WIND_BURST_2);
                        break;
                    case EVENT_LIGHTING_CLOUD:
                        DoCast(SPELL_LIGHTNING_CLOUD_PERIODIC);
                        break;
                    case EVENT_LIGHTING_ROD:
                        DoCast(SPELL_LIGHTNING_ROD);
                        break;
                    case EVENT_LIGHTING:
                        DoCastRandom(SPELL_LIGHTNING_2, 0.0f);
                        events.ScheduleEvent(EVENT_LIGHTING, 2000, 0, PHASE_3);
                        break;
                    case EVENT_LIGHTING_CLOUD_SUMMON:
                    {
                        float z = 350.0f;
                        if (Unit *target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            z = target->GetPositionZ();
                        me->CastSpell(me->GetPositionX(), me->GetPositionY(), z, SPELL_LIGHTNING_CLOUD_SUMMON, true);
                        events.ScheduleEvent(EVENT_LIGHTING_CLOUD_SUMMON, RAID_MODE(15000, 15000, 10000, 10000), 0, PHASE_3);
                        break;
                    }
                    // heroic part
                    case EVENT_STATIC_SHOCK:
                    {
                        DoCast(SPELL_STATIC_SHOCK);
                        events.ScheduleEvent(EVENT_STATIC_SHOCK, 5000, 0, _phase);
                        break;
                    }
                    case EVENT_DESTROY_PLATFORM:
                        if (GameObject *go = instance->instance->GetGameObject(instance->GetData64(GOB_CENTER_PLATFORM)))
                            go->SetDestructibleState(GO_DESTRUCTIBLE_DESTROYED);
                        break;
                    default:
                        break;
                }

            DoMeleeAttackIfReady();
        }

    private:
        uint8 _phase;
    };
};

class npc_alakir_icestorm : public CreatureScript
{
public:
    npc_alakir_icestorm() : CreatureScript("npc_alakir_icestorm") { }

    struct npc_alakir_icestormAI : public ScriptedAI
    {
        npc_alakir_icestormAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
        }

        void IsSummonedBy(Unit* summoner)
        {
            if (instance)
                if (Creature* alakir = Creature::GetCreature(*me, instance->GetData64(DATA_ALAKIR)))
                    alakir->AI()->JustSummoned(me);
        }
    private:
        InstanceScript* instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_alakir_icestormAI(creature);
    }
};

class spell_alakir_electrocute : public SpellScriptLoader
{
public:
    spell_alakir_electrocute() : SpellScriptLoader("spell_alakir_electrocute") { }

    class spell_alakir_electrocute_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_alakir_electrocute_AuraScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            return true;
        }

        void OnPeriodic(AuraEffect const* /*aurEff*/)
        {
            Unit* target = GetTarget();
            Unit *caster = GetCaster();
            if (!caster || !target)
                return;
            if (target->GetDistance2d(caster->GetPositionX(), caster->GetPositionY()) <= 35.0f)
                target->RemoveAurasDueToSpell(SPELL_ELECTROCUTE);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_alakir_electrocute_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_alakir_electrocute_AuraScript();
    }
};

class npc_azril_lighting_strike : public CreatureScript
{
public:
    npc_azril_lighting_strike() : CreatureScript("npc_azril_lighting_strike") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_azril_lighting_strikeAI(creature);
    }

    struct npc_azril_lighting_strikeAI : public ScriptedAI
    {
        npc_azril_lighting_strikeAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            triggerDamage = 0;
            playerGUID[0] = 0;
            playerGUID[1] = 0;
            strikeCount[0] = 0;
            strikeCount[1] = 0;
        }

        void PlayerInflictDamages()
        {
            int currentStrike = 0;
            if (strikeCount[0] != 0)
                currentStrike = 1;
            Unit *caster = Unit::GetUnit(*me, playerGUID[currentStrike]);
            if (!caster)
                return;
            Map::PlayerList const& players = me->GetMap()->GetPlayers();
            if (!players.isEmpty())
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                    if (Player* player = itr->getSource())
                        if (player->GetGUID() != caster->GetGUID())
                            if (caster->GetDistance2d(player) < 30.0f)
                            {
                                Position pos;
                                player->GetPosition(&pos);
                                if (caster->HasInArc(static_cast<float>(M_PI / 3), &pos))
                                    caster->CastSpell(player, SPELL_LIGHTING_STRIKE_PLAYER, true);
                            }
        }

        void visualEffect()
        {
            int currentStrike = 0;
            if (strikeCount[0] != 0)
                currentStrike = 1;
            float temp_orient = orient[currentStrike] - M_PI / 4;
            for (int i = 0; i < 12; i++)
            {
                float x, y;
                me->GetNearPoint2D(x, y, 65, temp_orient);
                me->CastSpell(x, y, me->GetPositionZ(), SPELL_LIGHTING_STRIKE_DUMMY, true);
                temp_orient += M_PI / 24;
            }
        }

        void damagePlayers()
        {
            int currentStrike = 0;
            if (strikeCount[0] != 0)
                currentStrike = 1;
            if (Unit *caster = Unit::GetUnit(*me, playerGUID[currentStrike]))
            {
                me->SetOrientation(orient[currentStrike]);
                visualEffect();
                //                me->CastSpell(caster, SPELL_LIGHTING_STRIKE_DUMMY, false);
                me->CastSpell(caster, SPELL_LIGHTING_STRIKE_CONE_DAMAGE, false);
                me->CastSpell(me, SPELL_LIGHTING_STRIKE_SC_EFF, false);
            }
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_LIGHTNING_STRIKE_TRIGGER)
            {
                me->SetFacingToObject(caster);
                int currentStrike = 0;
                if (strikeCount[0] != 0)
                    currentStrike = 1;
                playerGUID[currentStrike] = caster->GetGUID();
                orient[currentStrike] = me->GetOrientation();
                PlayerInflictDamages();
                if (!IsHeroic())
                    damagePlayers();
                else
                    strikeCount[currentStrike] = 16;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (triggerDamage <= diff)
            {
                int currentStrike = 0;
                if (strikeCount[0] != 0)
                    currentStrike = 1;
                if (strikeCount[currentStrike] > 0)
                {
                    strikeCount[currentStrike]--;
                    damagePlayers();
                }
                triggerDamage = 1000;
            }
            else
                triggerDamage -= diff;
        }

    private :
        uint64 playerGUID[2];
        float orient[2];
        uint32 triggerDamage;
        uint8 strikeCount[2];
    };
};

class npc_alakir_stormling : public CreatureScript
{
public:
    npc_alakir_stormling() : CreatureScript("npc_alakir_stormling") { }

    struct npc_alakir_stormlingAI : public ScriptedAI
    {
        npc_alakir_stormlingAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
            end = false;
        }

        void DamageTaken(Unit* caster, uint32& damage)
        {
            if (damage > me->GetHealth())
                if (Creature* alakir = Creature::GetCreature(*me, instance->GetData64(DATA_ALAKIR)))
                    me->AddAura(SPELL_FEEDBACK, alakir);
        }

        void IsSummonedBy(Unit* summoner)
        {
            if (instance)
                if (Creature* alakir = Creature::GetCreature(*me, instance->GetData64(DATA_ALAKIR)))
                    alakir->AI()->JustSummoned(me);
        }
    private:
        InstanceScript* instance;
        bool end;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_alakir_stormlingAI(creature);
    }
};

class spell_alakir_wind_burst : public SpellScriptLoader
{
public:
    spell_alakir_wind_burst() : SpellScriptLoader("spell_alakir_wind_burst") { }

    class spell_alakir_wind_burst_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_alakir_wind_burst_AuraScript);

        void PeriodicTick(AuraEffect const* aurEff)
        {
            if (Unit *caster = GetCaster())
                if (Unit *target = GetTarget())
                {
                    target->RemoveAurasDueToSpell(SPELL_PLAYERS_CAN_FLIGHT);
                    float x, y;
                    target->GetNearPoint2D(x, y, 4, target->GetAngle(caster) + M_PI);
                    target->NearTeleportTo(x, y, target->GetPositionZ(), target->GetOrientation());
                    target->CastSpell(target, SPELL_PLAYERS_CAN_FLIGHT, true);
                }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_alakir_wind_burst_AuraScript::PeriodicTick, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_alakir_wind_burst_AuraScript();
    }
};

class spell_lighting_cloud_dummy : public SpellScriptLoader
{
public:
    spell_lighting_cloud_dummy() : SpellScriptLoader("spell_lighting_cloud_dummy") { }

    class spell_lighting_cloud_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_lighting_cloud_dummy_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Player *target = GetHitPlayer())
            {
                target->CastSpell(target, SPELL_LIGHTNING_CLOUD_SUMMON, true);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_lighting_cloud_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_lighting_cloud_dummy_SpellScript();
    }
};

class npc_alakir_lighting_cloud : public CreatureScript
{
public:
    npc_alakir_lighting_cloud() : CreatureScript("npc_alakir_lighting_cloud") { }

    struct npc_alakir_lighting_cloudAI : public ScriptedAI
    {
        npc_alakir_lighting_cloudAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
        }

        void IsSummonedBy(Unit* summoner)
        {
            if (instance)
                if (Creature* alakir = Creature::GetCreature(*me, instance->GetData64(DATA_ALAKIR)))
                    alakir->AI()->JustSummoned(me);
        }
    private:
        InstanceScript* instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_alakir_lighting_cloudAI(creature);
    }
};

class spell_lighting_cloud_damage : public SpellScriptLoader
{
public:
    spell_lighting_cloud_damage() : SpellScriptLoader("spell_lighting_cloud_damage") { }

    class LightingCloudTargetSelector
    {
    public:
        LightingCloudTargetSelector(Unit *caster): _caster(caster) { }

        bool operator() (WorldObject* target)
        {
            float dist = target->GetPositionZ() - _caster->GetPositionZ();
            if (dist < 0)
                dist *= -1;
            if (dist > 5)
                return true;
            return false;
        }
    private:
        Unit *_caster;
    };

    class spell_lighting_cloud_damage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_lighting_cloud_damage_SpellScript);

        bool Load()
        {
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            if (Unit *caster = GetCaster())
                unitList.remove_if(LightingCloudTargetSelector(caster));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_lighting_cloud_damage_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_lighting_cloud_damage_SpellScript();
    }
};

// 95751
class spell_alakir_eject_platform : public SpellScriptLoader
{
public:
    spell_alakir_eject_platform() : SpellScriptLoader("spell_alakir_eject_platform") { }

    class spell_alakir_eject_platform_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_alakir_eject_platform_AuraScript);

        void SetFlags(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
        }

        void RemoveFlags(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit *caster = GetUnitOwner())
                if (InstanceScript *instance = caster->GetInstanceScript())
                    if (Creature* alakir = Creature::GetCreature(*caster, instance->GetData64(DATA_ALAKIR)))
                    {
                        float x, y;
                        alakir->GetNearPoint2D(x, y, 50.0f, alakir->GetAngle(caster));
                        caster->AddAura(85269, caster);
                        caster->GetMotionMaster()->MoveJump(x, y, alakir->GetPositionZ(), 15.0f, 17.0f);
                    }
        }


        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_alakir_eject_platform_AuraScript::SetFlags, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_alakir_eject_platform_AuraScript::RemoveFlags, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_alakir_eject_platform_AuraScript();
    }
};

void AddSC_boss_alakir()
{
    new boss_alakir();
    new npc_alakir_icestorm();
    new spell_alakir_electrocute();
    new npc_azril_lighting_strike();
    new npc_alakir_stormling();
    new spell_alakir_wind_burst();
    new npc_alakir_lighting_cloud();
    new spell_lighting_cloud_damage();
    new spell_alakir_eject_platform();
}
