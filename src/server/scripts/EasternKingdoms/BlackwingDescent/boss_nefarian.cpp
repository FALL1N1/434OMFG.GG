
#include "blackwing_descent.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "AccountMgr.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"

enum spells
{

    SPELL_HITTING_YA = 94132,
    SPELL_HITTING_YA_PLAYER = 94131,

    SPELL_ONYXIA_FEIGN_DEATH = 29266,
    SPELL_INTRO_2_STALKER_TRANSFORM = 78750,
    SPELL_INTRO_SHINK_AURA = 78753,
    SPELL_INVISIBLE_STALKER_CATACLYSM_BOSS_AURA = 80857,
    SPELL_CHAIN_A_STEEL = 81159,
    SPELL_CHAIN_B_STEEL = 81174,
    SPELL_CHAIN_C_STEEL = 81176,
    SPELL_CHAIN_A_LIGHTING = 81158,
    SPELL_CHAIN_B_LIGHTING = 81175,
    SPELL_CHAIN_C_LIGHTING = 81177,
    SPELL_ONYXIA_START_FIGHT_PERIODIC = 81516,
    SPELL_ONYXIA_START_FIGHT_2 = 81517,
    SPELL_NEFARIAN_INTRO_4_LIFT_OFF_ANIM_KIT = 78692,
    SPELL_NEFARIAN_INTRO_5_START_FIGHT_PROC = 78730,
    SPELL_ONYXIA_ELETRIC_CHARGE = 78949,
    SPELL_ONYXIA_SHADOW_OF_COWARDICE = 79353,
    SPELL_ONYXIA_SHADOWFLAME_BREATH = 77826,
    SPELL_ONYXIA_TAIL_LASH = 77827,
    SPELL_ONYXIA_LIGHTING_DISCHARGE = 78090,
    SPELL_ONYXIA_LIGHTING_DISCHARGE_TRIGGER = 77832,
    SPELL_ONYXIA_LIGHTING_DISCHARGE_TRIGGER_2_DUMMY = 77836,
    SPELL_ONYXIA_LIGHTING_DISCHARGE_DUMMY = 81435,
    SPELL_ONYXIA_LIGHTING_DISCHARGE_DUMMY_2 = 81436,
    SPELL_ONYXIA_LIGHTING_DISCHARGE_FRONT_DUMMY = 81437,
    SPELL_ONYXIA_LIGHTING_DISCHARGE_BACK_DUMMY = 81438,
    SPELL_ONYXIA_LIGHTING_DISCHARGE_CONE_ENEMY_54_DUMMY_1 = 77833,
    SPELL_ONYXIA_LIGHTING_DISCHARGE_CONE_ENEMY_54_DUMMY_2 = 77919,
    SPELL_ONYXIA_LIGHTING_DISCHARGE_DAMAGE_1 = 77939,
    SPELL_ONYXIA_LIGHTING_DISCHARGE_DAMAGE_2 = 77942,
    SPELL_ONYXIA_LIGHTING_DISCHARGE_DAMAGE_3 = 77943,
    SPELL_ONYXIA_LIGHTING_DISCHARGE_DAMAGE_4 = 77944,
    SPELL_ONYXIA_DEATHWING_CHILDREN_PERIODIC_DUMMY = 80785,
    SPELL_ONYXIA_DEATHWING_CHILDREN = 78619,
    SPELL_ONYXIA_ELECTRICAL_SURCHARGE = 78999,
    SPELL_ONYXIA_CLEAVE = 103001,
    SPELL_NEFARIAN_HAIL_OF_BONES = 78679,
    SPELL_NEFARIAN_HAIL_OF_BONES_TRIGGER = 78682,
    SPELL_NEFARIAN_HAIL_OF_BONES_DAMAGE = 78684,
    SPELL_NEFARIAN_ELETRIC_CHARGE = 95793,
    SPELL_NEFARIAN_SHADOW_OF_COWARDICE = 79355,
    SPELL_NEFARIAN_SHADOW_OF_COWARDICE_TRIGGER = 80963,
    SPELL_NEFARIAN_DEATHWING_CHILDREN_PERIODIC_DUMMY = 80787,
    SPELL_NEFARIAN_DEATHWING_CHILDREN = 78620,
    SPELL_NEFARIAN_TAIL_LASH = 77827,
    SPELL_NEFARIAN_SHADOWFLAME_BREATH = 77826,
    SPELL_NEFARIAN_ELECTROCUTE = 81272,
    SPELL_NEFARIAN_ELECTROCUTE_DUMMY = 81198,
    SPELL_NEFARIAN_CLEAVE = 103001,
    SPELL_NEFARIAN_BRUSHFIRE_SUMMON = 79492,
    SPELL_NEFARIAN_OMBREFLAME = 78621,
    SPELL_NEFARIAN_OMBRASE = 81031,
    SPELL_NEFARIAN_OMBRASE_START = 79813,
    SPELL_ELECTROCUTE_1 = 81198,
    SPELL_ELECTROCUTE_2 = 94089,
    SPELL_BERSERK = 64238,

    // NPC_ANIMATED_BONE_WARRIOR
    SPELL_FULL_POWER_NO_REGEN = 78120,
    SPELL_ANIMATED_BONES = 78122,
    SPELL_ANIMATED_BONES_DUMMY = 78123,
    SPELL_EMPOWERING_STRIKES = 79329,
    SPELL_EMPOWER = 79330,
    SPELL_BONES_LAUNCH = 81586, // hm only ?

    // NPC_CHROMATIC_PROTOTYPE
    SPELL_JUMP_DOWN_TO_PLATFORM = 79205,
    SPELL_READY_UNARMED = 94610,
    SPELL_NUKE_DUMMY = 80776, // not used
    SPELL_BLASTER_NOVA = 101431,

    // NPC_SHADOWBLAZE_FLASHPOINT
    SPELL_BRUSHFIRE_FLASHPOINT_CONTROL = 79392,
    SPELL_BRUSHFIRE_GROWTH = 79393,
    SPELL_BRUSHFIRE_AURA = 79396,
    SPELL_BRUSHFIRE_CHECK_VALID_LOCATION = 79401,
    SPELL_BRUSHFIRE_BURN_AURA = 79396,
    SPELL_BRUSHFIRE_SUMMON = 79405,

    // NPC_SHADOWBLAZE
    SPELL_SHADOWBLAZE = 94086,

    // heroic
    SPELL_DOMINION = 79318, // 11:53:23.515 casted by BOSS_NEFARIAN = 41376
    SPELL_DOMINION_DUMMY = 94211,
    SPELL_DOMINION_IMMUNE_MASK = 95900, // by player on himself 11:53:25.250
    SPELL_DOMINATION_TRACKER_SUMMON_SUD = 81746, // by player 11:53:25.547 summon 43656
    SPELL_DOMINATION_TRACKER_SUMMON_WEST = 81747, // 11:53:25.547
    SPELL_DOMINATION_TRACKER_SUMMON_NORTH = 81665, // 11:53:25.547
    SPELL_DOMINATION_TRACKER_SUMMON_EAST = 81745, // 11:53:25.547
    SPELL_PLAYER_MIND_FREE = 79323,
    SPELL_PLAYER_ABSORB_POWER = 79319,
    SPELL_PLAYER_STEALTH_POWER = 80627,
    SPELL_PLAYER_MIND_LIBERATE = 98932,
    SPELL_NEFARIAN_EXPLOSIVE_CINDER = 79339,
    SPELL_PLAYER_EXPLOSIVE_CINDER = 79347,

    // NPC_DOMINION_TRACKER
    SPELL_DOMINION_DETERMINE_FARTHEST_PORTAL_STALKER = 81664, // 11:53:26.062
    SPELL_DOMINATION_PORTAL_TRIGGER = 81752, // casted by 43656 on control target 11:53:26.062
    SPELL_DOMINION_PORTAL_BEAM_DUMMY = 81709, // casted by 43656 on control player  11:53:26.062
};

enum Entities
{
    BOSS_ONYXIA = 41270,
    BOSS_NEFARIAN = 41376,
    NPC_INVISIBLE_STALKER_CATACLYSM_BOSS_2 = 42844,
    NPC_ANIMATED_BONE_WARRIOR = 41918,
    NPC_CHROMATIC_PROTOTYPE = 41948,
    NPC_SHADOWBLAZE_FLASHPOINT = 42595,
    NPC_SHADOWBLAZE = 42596,
    NPC_CONTROLLER_STALKER = 42856,
    NPC_NEFARIAN_LIGHTING_MACHINE = 51089,
    NPC_NEFARIUS_INTRO = 41379,
    NPC_INVISIBLE_STALKER = 35592,
    NPC_PILLAR_MARKER = 320000,
    NPC_DOMINION_TRACKER = 43656,
};

enum Events
{
    EVENT_ONYXIA_ELETRIC_CHARGE = 1,
    EVENT_ONYXIA_SHADOW_OF_COWARDICE,
    EVENT_ONYXIA_SHADOWFLAME_BREATH,
    EVENT_ONYXIA_TAIL_LASH,
    EVENT_ONYXIA_PRE_LIGHTING_DISCHARGE,
    EVENT_ONYXIA_LIGHTING_DISCHARGE,
    EVENT_ONYXIA_LIGHTING_DISCHARGE_1,
    EVENT_ONYXIA_LIGHTING_DISCHARGE_2,
    EVENT_ONYXIA_LIGHTING_DISCHARGE_3,
    EVENT_ONYXIA_DEATHWING_CHILDREN_PERIODIC_DUMMY,
    EVENT_ONYXIA_OVERCHARGE,
    EVENT_ONYXIA_CLEAVE,
    EVENT_ONYXIA_REVIVE,

    EVENT_INTRO,

    EVENT_NEFARIAN_INTRO_1,
    EVENT_NEFARIAN_INTRO_2,
    EVENT_NEFARIAN_HAIL_OF_BONES,
    EVENT_NEFARIAN_ELETRIC_CHARGE,
    EVENT_NEFARIAN_SHADOW_OF_COWARDICE,
    EVENT_NEFARIAN_DEATHWING_CHILDREN_PERIODIC_DUMMY,
    EVENT_NEFARIAN_TAIL_LASH,
    EVENT_NEFARIAN_DOMINION,
    EVENT_NEFARIAN_SHADOWFLAME_BREATH,
    EVENT_NEFARIAN_HITING_YA,
    EVENT_NEFARIAN_ELECTROCUTE,
    EVENT_NEFARIAN_CLEAVE,
    EVENT_NEFARIAN_BRUSHFIRE_SUMMON,
    EVENT_CHROMATIC_SPAWN,
    EVENT_NEFARIAN_PHASE_3,
    EVENT_NEFARIAN_OMBRASE,
    EVENT_ELEVATOR_UP_P3,
    EVENT_START_PHASE_1,

    EVENT_BONES_LAUNCH,
    EVENT_LOST_ENERGY,

    EVENT_NEFARIAN_EXPLOSIVE_CINDER,

    EVENT_BERSERK,
};

enum EncounterPhases
{
    PHASE_INTRO = 1,
    PHASE_1,
    PHASE_2,
    PHASE_3,
};

enum actions
{
    ACTION_DISCHARGE,
    ACTION_OVERCHARGE,
    ACTION_ONYXIA_DEATH,
    ACTION_STOP_INTRO,
    ACTION_INTRO,
    START_PHASE_PRE_P1,
    ACTION_DESACTIVATE_BONES_WARRIOR,
};

enum datas
{
    DATA_PHASE,
};

enum points
{
    NEFARIAN_INTRO_POINT,
    NEFARIAN_P2_POINT,
};

enum paths
{
    NEFARIAN_INTRO_PATH = 4137600,
};

class boss_bwd_onyxia : public CreatureScript
{
    struct boss_bwd_onyxiaAI : public BossAI
    {
        boss_bwd_onyxiaAI(Creature * creature) : BossAI(creature, DATA_ONYXIA_GUID)
        {
            _phase = PHASE_INTRO;
        }

        void Reset()
        {
            start = false;
            me->SetReactState(REACT_PASSIVE);
            me->SetVisible(false);
            events.Reset();
            events.SetPhase(PHASE_INTRO);
            _phase = PHASE_INTRO;
            summons.DespawnAll();
            me->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
            for (int cnt = 0; cnt < 3; cnt++)
                chain[cnt] = NULL;
            me->CastSpell(me, SPELL_ONYXIA_FEIGN_DEATH, true);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            _Reset();
        }

        void enableTransportMovements(Creature *c, bool enable)
        {
            if (enable)
            {
                c->m_movementInfo.t_guid = instance->GetData64(DATA_NEFARIAN_ELEVATOR_GUID);
                c->m_movementInfo.t_pos.Relocate(0, 0, 0, 0);
            }
            else
                c->m_movementInfo.t_guid = 0;
        }

        void JustSummoned(Creature * summon)
        {
            BossAI::JustSummoned(summon);
        }

        void JustReachedHome()
        {
            if (Creature *nefarian = me->FindNearestCreature(BOSS_NEFARIAN, 500))
                if (!nefarian->IsInEvadeMode())
                {
                    nefarian->GetMotionMaster()->MoveIdle();
                    nefarian->AI()->EnterEvadeMode();
                }
            _JustReachedHome();
        }

        void EnterCombat(Unit * /*who*/)
        {
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            _phase = PHASE_1;
            me->CastSpell(me, SPELL_HITTING_YA, true);
            events.ScheduleEvent(EVENT_ONYXIA_ELETRIC_CHARGE, 0);
            events.ScheduleEvent(EVENT_ONYXIA_SHADOWFLAME_BREATH, 13000);
            events.ScheduleEvent(EVENT_ONYXIA_TAIL_LASH, 18000);
            events.ScheduleEvent(EVENT_ONYXIA_DEATHWING_CHILDREN_PERIODIC_DUMMY, 32000);
            events.ScheduleEvent(EVENT_ONYXIA_PRE_LIGHTING_DISCHARGE, 24000);
            events.ScheduleEvent(EVENT_ONYXIA_CLEAVE, 5000);
            _EnterCombat();
        }

        void DoAction(const int32 act)
        {
            switch (act)
            {
                case ACTION_OVERCHARGE:
                    events.ScheduleEvent(EVENT_ONYXIA_OVERCHARGE, 0, 0, PHASE_1);
                    break;
                case ACTION_INTRO:
                {
                    me->SetVisible(true);
                    if (chain[0] = me->SummonCreature(NPC_INVISIBLE_STALKER_CATACLYSM_BOSS, -141.3331f, -224.6247f, 8.380039f, 0.0f))
                    {
                        chain[0]->RemoveAllAuras();
                        enableTransportMovements(chain[0], true);
                        chain[0]->CastSpell(me, SPELL_CHAIN_A_STEEL, true);
                    }
                    if (chain[1] = me->SummonCreature(NPC_INVISIBLE_STALKER_CATACLYSM_BOSS, -90.35625f, -253.4714f, 8.380039f, 0.0f))
                    {
                        chain[1]->RemoveAllAuras();
                        enableTransportMovements(chain[1], true);
                        chain[1]->CastSpell(me, SPELL_CHAIN_B_STEEL, true);
                    }
                    if (chain[2] = me->SummonCreature(NPC_INVISIBLE_STALKER_CATACLYSM_BOSS, -90.22045f, -195.6071f, 8.380039f, 0.0f))
                    {
                        chain[2]->RemoveAllAuras();
                        enableTransportMovements(chain[2], true);
                        chain[2]->CastSpell(me, SPELL_CHAIN_C_STEEL, true);
                    }
                    enableTransportMovements(me, true);
                    break;
                }
                case ACTION_STOP_INTRO:
                {
                    if (chain[0])
                        chain[0]->CastSpell(me, SPELL_CHAIN_A_LIGHTING, true);
                    if (chain[1])
                        chain[1]->CastSpell(me, SPELL_CHAIN_B_LIGHTING, true);
                    if (chain[2])
                        chain[2]->CastSpell(me, SPELL_CHAIN_C_LIGHTING, true);
                    events.ScheduleEvent(EVENT_ONYXIA_REVIVE, 1500);
                    break;
                }
            }
        }

        void DamageTaken(Unit* caster, uint32& damage)
        {
            if (!start)
            {
                me->SetReactState(REACT_AGGRESSIVE);
                me->SetInCombatWithZone();
                start = true;
                if (Creature *nefarian = me->FindNearestCreature(BOSS_NEFARIAN, 500))
                    nefarian->AI()->DoAction(START_PHASE_PRE_P1);
            }
        }

        void JustDied(Unit * /*killer*/)
        {
            if (Creature *nefarian = me->FindNearestCreature(BOSS_NEFARIAN, 500))
                nefarian->AI()->DoAction(ACTION_ONYXIA_DEATH);
            summons.DespawnAll();
            me->SetVisible(false);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            _JustDied();
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim() && _phase != PHASE_INTRO)
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ONYXIA_REVIVE:
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                        enableTransportMovements(me, false);
                        me->RemoveAura(SPELL_ONYXIA_FEIGN_DEATH);
                        me->RemoveAura(SPELL_CHAIN_A_STEEL);
                        me->RemoveAura(SPELL_CHAIN_B_STEEL);
                        me->RemoveAura(SPELL_CHAIN_C_STEEL);
                        me->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), 7.0f, me->GetOrientation());
                        me->SetReactState(REACT_DEFENSIVE);
                        break;
                    case EVENT_ONYXIA_ELETRIC_CHARGE:
                        DoCast(SPELL_ONYXIA_ELETRIC_CHARGE);
                        break;
                    case EVENT_ONYXIA_SHADOW_OF_COWARDICE:
                        DoCastVictim(SPELL_ONYXIA_SHADOW_OF_COWARDICE);
                        events.ScheduleEvent(EVENT_ONYXIA_SHADOW_OF_COWARDICE, urand(1000, 2000));
                        break;
                    case EVENT_ONYXIA_SHADOWFLAME_BREATH:
                        DoCastVictim(SPELL_ONYXIA_SHADOWFLAME_BREATH);
                        events.ScheduleEvent(EVENT_ONYXIA_SHADOWFLAME_BREATH, urand(11000, 25000));
                        break;
                    case EVENT_ONYXIA_TAIL_LASH:
                        DoCast(SPELL_ONYXIA_TAIL_LASH);
                        events.ScheduleEvent(EVENT_ONYXIA_TAIL_LASH, urand(11000, 15000));
                        break;
                    case EVENT_ONYXIA_PRE_LIGHTING_DISCHARGE:
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE);
                        events.ScheduleEvent(EVENT_ONYXIA_LIGHTING_DISCHARGE, 6000);
                        break;
                    case EVENT_ONYXIA_LIGHTING_DISCHARGE:
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_DUMMY);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_DUMMY_2);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_FRONT_DUMMY);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_BACK_DUMMY);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_CONE_ENEMY_54_DUMMY_1);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_CONE_ENEMY_54_DUMMY_2);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_DAMAGE_1);
                        events.DelayEvents(3100);
                        events.ScheduleEvent(EVENT_ONYXIA_LIGHTING_DISCHARGE_1, 1000);
                        break;
                    case EVENT_ONYXIA_LIGHTING_DISCHARGE_1:
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_DAMAGE_2);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_DUMMY);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_DUMMY_2);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_FRONT_DUMMY);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_BACK_DUMMY);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_CONE_ENEMY_54_DUMMY_1);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_CONE_ENEMY_54_DUMMY_2);
                        events.ScheduleEvent(EVENT_ONYXIA_LIGHTING_DISCHARGE_2, 1000);
                        break;
                    case EVENT_ONYXIA_LIGHTING_DISCHARGE_2:
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_DAMAGE_3);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_DUMMY);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_DUMMY_2);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_FRONT_DUMMY);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_BACK_DUMMY);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_CONE_ENEMY_54_DUMMY_1);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_CONE_ENEMY_54_DUMMY_2);
                        events.ScheduleEvent(EVENT_ONYXIA_LIGHTING_DISCHARGE_3, 1000);
                        break;
                    case EVENT_ONYXIA_LIGHTING_DISCHARGE_3:
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_DAMAGE_4);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_DUMMY);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_DUMMY_2);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_FRONT_DUMMY);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_BACK_DUMMY);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_CONE_ENEMY_54_DUMMY_1);
                        DoCast(SPELL_ONYXIA_LIGHTING_DISCHARGE_CONE_ENEMY_54_DUMMY_2);
                        events.ScheduleEvent(EVENT_ONYXIA_PRE_LIGHTING_DISCHARGE, 20000);
                        break;
                    case EVENT_ONYXIA_DEATHWING_CHILDREN_PERIODIC_DUMMY:
                        DoCast(SPELL_ONYXIA_DEATHWING_CHILDREN_PERIODIC_DUMMY);
                        break;
                    case EVENT_ONYXIA_OVERCHARGE:
                        DoCast(SPELL_ONYXIA_ELECTRICAL_SURCHARGE);
                        break;
                    case EVENT_ONYXIA_CLEAVE:
                        DoCastVictim(SPELL_ONYXIA_CLEAVE);
                        events.ScheduleEvent(EVENT_ONYXIA_CLEAVE, urand(10000, 12000));
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private :
        bool start;
        uint8 _phase;
        Creature *chain[3];
    };
public:
    boss_bwd_onyxia() : CreatureScript("boss_bwd_onyxia") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new boss_bwd_onyxiaAI(creature);
    }
};

class boss_bwd_nefarian : public CreatureScript
{

    class BrushFireEvent : public BasicEvent
    {
    public:
        BrushFireEvent(Unit* owner, EncounterPhases cu) : _owner(owner), _currentPhase(cu)
        {
        }

        bool Execute(uint64 execTime, uint32 /*diff*/)
        {
            if (Creature *c = _owner->ToCreature())
            {
                if (_currentPhase != PHASE_3)
                    c->DespawnOrUnsummon(1000);
                else
                {
                    Position destPos, pos;
                    c->GetPosition(&pos);
                    if (Creature *t = c->SummonCreature(NPC_SHADOWBLAZE_FLASHPOINT, pos.m_positionX, pos.m_positionY, pos.m_positionZ, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000))
                        t->CastSpell(t, SPELL_BRUSHFIRE_AURA, true);
                    Player *player = NULL;
                    float dist = 0.0f;
                    Map::PlayerList const& players = c->GetMap()->GetPlayers();
                    if (!players.isEmpty())
                        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                            if (Player* plr = itr->getSource())
                                if (plr->isAlive())
                                    if (!AccountMgr::IsGMAccount(plr->GetSession()->GetSecurity()))
                                    {
                                        float tempDist = c->GetDistance2d(plr->GetPositionX(), plr->GetPositionY());
                                        if (player == NULL || dist > tempDist)
                                        {
                                            dist = tempDist;
                                            player = plr;
                                        }
                                    }
                    // if (Player *player = c->FindNearestPlayer(500.0f)) grid are not correctly update when players are on transport...
                    if (player)
                    {
                        player->GetPosition(&destPos);
                        c->GetMotionMaster()->MovePoint(0, destPos);
                        c->SetSpeed(MOVE_RUN, 0.15f);
                        c->SetSpeed(MOVE_WALK, 0.15f);
                        c->SetSpeed(MOVE_SWIM, 0.15f);
                        c->SetSpeed(MOVE_FLIGHT, 0.15f);
                        c->m_Events.AddEvent(new BrushFireEvent(c, _currentPhase), c->m_Events.CalculateTime(6000));
                    }
                }
            }
            return false;
        }

    private:
        Unit* _owner;
        EncounterPhases _currentPhase;
    };

    class CheckReactState : public BasicEvent
    {
    public:
        CheckReactState(Unit* owner) : _owner(owner)
        {
        }

        bool Execute(uint64 execTime, uint32 /*diff*/)
        {
            if (_owner->HasUnitState(UNIT_STATE_CASTING))
                _owner->m_Events.AddEvent(new CheckReactState(_owner), _owner->m_Events.CalculateTime(1000));
            else if (_owner->ToCreature())
            {
                _owner->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                _owner->CastSpell(_owner, SPELL_BLASTER_NOVA, false);
                _owner->m_Events.AddEvent(new CheckReactState(_owner), _owner->m_Events.CalculateTime(_owner->GetMap()->Is25ManRaid() ? 6000 : 12000));
            }
            return false;
        }

    private:
        Unit* _owner;
        EncounterPhases _currentPhase;
    };

    struct boss_bwd_nefarianAI : public BossAI
    {
        boss_bwd_nefarianAI(Creature * creature) : BossAI(creature, DATA_NEFARIAN)
        {
            alreadyStart = false;
        }

        void Reset()
        {
            _Reset();
            changePhase(PHASE_INTRO);
            summons.DespawnAll();
            electrocuteCount = 0;
            me->SetReactState(REACT_PASSIVE);
            me->SetHover(true);
            events.ScheduleEvent(EVENT_INTRO, 0);
            me->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            _chromaticCount = 0;
            _ombraseTimer = 25000;
            achievement = false;
            p3Started = false;
        }

        void JustReachedHome()
        {
            if (Creature *onyxia = me->FindNearestCreature(BOSS_ONYXIA, 500.0f, false))
                onyxia->Respawn(true);
            instance->SetData(DATA_NEFARIAN_ELEVATOR, GO_STATE_TRANSPORT_READY);
            _JustReachedHome();
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            switch (summon->GetEntry())
            {
                case NPC_CHROMATIC_PROTOTYPE:
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, summon);
                    break;
                default:
                    break;
            }
        }

        void JustSummoned(Creature * summon)
        {
            summon->SetInCombatWithZone();
            switch (summon->GetEntry())
            {
                case NPC_SHADOWBLAZE_FLASHPOINT:
                    summon->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                    summon->SetReactState(REACT_PASSIVE);
                    summon->m_Events.AddEvent(new BrushFireEvent(summon, _currentPhase), summon->m_Events.CalculateTime(1000));
                    break;
                case NPC_SHADOWBLAZE:
                    summon->CastWithDelay(1000, summon, SPELL_BRUSHFIRE_AURA, true);
                    summon->CastSpell(summon, SPELL_BRUSHFIRE_CHECK_VALID_LOCATION, true);
                    break;
                case NPC_CHROMATIC_PROTOTYPE:
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, summon);
                    summon->SetReactState(REACT_PASSIVE);
                    summon->CastSpell(summon, SPELL_JUMP_DOWN_TO_PLATFORM, true);
                    summon->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                    summon->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                    summon->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                    summon->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                    summon->m_Events.AddEvent(new CheckReactState(summon), summon->m_Events.CalculateTime(5000));
                    _chromaticCount++;
                    break;
                case NPC_DOMINION_TRACKER:
                    summon->CastSpell(summon, SPELL_DOMINION_DETERMINE_FARTHEST_PORTAL_STALKER, true);
                    summon->CastSpell(summon, SPELL_DOMINATION_PORTAL_TRIGGER, true);
                    summon->CastSpell(summon, SPELL_DOMINION_PORTAL_BEAM_DUMMY, true);
                    break;
            }
            BossAI::JustSummoned(summon);
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/)
        {
            switch (summon->GetEntry())
            {
            case NPC_CHROMATIC_PROTOTYPE:
                if (_chromaticCount > 0)
                    _chromaticCount--;
                if (_chromaticCount == 0 || (IsHeroic() && !p3Started))
                {
                    events.ScheduleEvent(EVENT_ELEVATOR_UP_P3, 0, 0, PHASE_2);
                    p3Started = true;
                }
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, summon);
                break;
            default:
                break;
            }
        }

        void EnterCombat(Unit * /*who*/)
        {
            me->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 15.0f * DEFAULT_WORLD_OBJECT_SIZE);
            me->SetFloatValue(UNIT_FIELD_COMBATREACH, 10.0f * DEFAULT_COMBAT_REACH);
            me->CastSpell(me, SPELL_HITTING_YA, true);
            events.ScheduleEvent(EVENT_BERSERK, 630000);
            _EnterCombat();
        }

        void DoAction(const int32 act)
        {
            switch(act)
            {
                case ACTION_ONYXIA_DEATH:
                {
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    changePhase(PHASE_2);
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    me->GetMotionMaster()->Clear(false);
                    me->GetMotionMaster()->MoveIdle();
                    me->SetCanFly(true);
                    me->SetHover(true);
                    me->GetMotionMaster()->MovePoint(0, -107.14f, -224.18f, 19.5f);
                    std::list<Creature *> creatures;
                    GetCreatureListWithEntryInGrid(creatures, me, NPC_ANIMATED_BONE_WARRIOR, 500.0f);
                    if (!creatures.empty())
                        for (std::list<Creature *>::iterator iter = creatures.begin(); iter != creatures.end(); iter++)
                            if (Creature *c = *iter)
                                c->AI()->DoAction(ACTION_DESACTIVATE_BONES_WARRIOR);
                    break;
                }
                case START_PHASE_PRE_P1:
                    events.ScheduleEvent(EVENT_NEFARIAN_HAIL_OF_BONES, 0);
                    events.ScheduleEvent(EVENT_START_PHASE_1, 30000);
                    break;
            }
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(5);
        }

        void changePhase(EncounterPhases phase)
        {
            events.SetPhase(phase);
            _currentPhase = phase;
            switch (phase)
            {
                case PHASE_1:
                    Talk(1);
                    events.ScheduleEvent(EVENT_NEFARIAN_ELETRIC_CHARGE, 0, 0, PHASE_1);
                    events.ScheduleEvent(EVENT_NEFARIAN_DEATHWING_CHILDREN_PERIODIC_DUMMY, 0, 0, PHASE_1);
                    events.ScheduleEvent(EVENT_NEFARIAN_TAIL_LASH, 18000, 0, PHASE_1);
                    events.ScheduleEvent(EVENT_NEFARIAN_SHADOWFLAME_BREATH, 13000, 0, PHASE_1);
                    events.ScheduleEvent(EVENT_NEFARIAN_HITING_YA, 6000, 0, PHASE_1);
                    events.ScheduleEvent(EVENT_NEFARIAN_CLEAVE, 5000, 0, PHASE_1);
                    events.ScheduleEvent(EVENT_NEFARIAN_SHADOW_OF_COWARDICE, 0, 0, PHASE_1);
                    if (IsHeroic())
                        events.ScheduleEvent(EVENT_NEFARIAN_DOMINION, 5000, 0, PHASE_1);
                    break;
                case PHASE_2:
                    me->RemoveAurasDueToSpell(SPELL_NEFARIAN_SHADOW_OF_COWARDICE);
                    Talk(3);
                    instance->SetData(DATA_NEFARIAN_ELEVATOR, GO_STATE_TRANSPORT_ACTIVE);
                    events.ScheduleEvent(EVENT_CHROMATIC_SPAWN, 9000, 0, PHASE_2);
                    events.ScheduleEvent(EVENT_NEFARIAN_BRUSHFIRE_SUMMON, 9000, 0, PHASE_2);
                    events.ScheduleEvent(EVENT_ELEVATOR_UP_P3, 189000, 0, PHASE_2);
                    events.ScheduleEvent(EVENT_NEFARIAN_SHADOW_OF_COWARDICE, 12000, 0, PHASE_2);
                    if (IsHeroic())
                        events.ScheduleEvent(EVENT_NEFARIAN_EXPLOSIVE_CINDER, 12000, 0, PHASE_2);
                    break;
                case PHASE_3:
                {
                    Talk(7);
                    me->RemoveAurasDueToSpell(SPELL_NEFARIAN_SHADOW_OF_COWARDICE);
                    std::list<Creature *> creatures;
                    GetCreatureListWithEntryInGrid(creatures, me, NPC_CHROMATIC_PROTOTYPE, 500.0f);
                    if (!creatures.empty())
                        for (std::list<Creature *>::iterator iter = creatures.begin(); iter != creatures.end(); iter++)
                            if (Creature *c = *iter)
                            {
                                c->NearTeleportTo(c->GetPositionX(), c->GetPositionY(), 17.0f, c->GetOrientation());
                                c->m_Events.AddEvent(new CheckReactState(c), c->m_Events.CalculateTime(1000));
                            }
                    events.ScheduleEvent(EVENT_NEFARIAN_ELETRIC_CHARGE, 0, 0, PHASE_3);
                    events.ScheduleEvent(EVENT_NEFARIAN_TAIL_LASH, 9000, 0, PHASE_3);
                    events.ScheduleEvent(EVENT_NEFARIAN_SHADOWFLAME_BREATH, 4000, 0, PHASE_3);
                    events.ScheduleEvent(EVENT_NEFARIAN_HITING_YA, 0, 0, PHASE_3);
                    events.ScheduleEvent(EVENT_NEFARIAN_CLEAVE, 0, 0, PHASE_3);
                    events.ScheduleEvent(EVENT_NEFARIAN_OMBRASE, 21000, 0, PHASE_3);
                    events.ScheduleEvent(EVENT_NEFARIAN_SHADOW_OF_COWARDICE, 12000, 0, PHASE_3);
                    if (IsHeroic())
                        events.ScheduleEvent(EVENT_NEFARIAN_DOMINION, 15000, 0, PHASE_3);
                    if (HealthBelowPct(50))
                        achievement = true;
                    break;
                }
                default:
                    break;
            }
        }

        void JustDied(Unit * /*killer*/)
        {
            Talk(9);
            summons.DespawnAll();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            me->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), 8.6f, me->GetOrientation());
            _JustDied();
            if (achievement)
                instance->DoCompleteAchievement(4849); // Keeping it in the family
        }

        void DamageTaken(Unit* caster, uint32& damage)
        {
            if ((!HealthAbovePct(90) && electrocuteCount == 0) ||
                (!HealthAbovePct(80) && electrocuteCount == 1) ||
                (!HealthAbovePct(70) && electrocuteCount == 2) ||
                (!HealthAbovePct(60) && electrocuteCount == 3) ||
                (!HealthAbovePct(50) && electrocuteCount == 4) ||
                (!HealthAbovePct(40) && electrocuteCount == 5) ||
                (!HealthAbovePct(30) && electrocuteCount == 6) ||
                (!HealthAbovePct(20) && electrocuteCount == 7) ||
                (!HealthAbovePct(10) && electrocuteCount == 8))
            {
                Talk(0);
                events.ScheduleEvent(EVENT_NEFARIAN_ELECTROCUTE, 3000, 0, _currentPhase);
                if (electrocuteCount == 4 && _currentPhase == PHASE_2)
                    events.ScheduleEvent(EVENT_ELEVATOR_UP_P3, 0, 0, PHASE_3);
                electrocuteCount++;
            }
        }

        uint32 GetData(uint32 data) const
        {
            if (data == DATA_PHASE)
                return _currentPhase;
            return 0;
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE)
                me->GetMotionMaster()->MoveIdle();
        }

        void CheckNearPortalPosition(int &index, Unit *target)
        {
            float angle = 0.0f;
            float objSize = target->GetObjectSize();
            float dist = 25.0f;
            Position pos[4];
            float maxDist = 0.0f;
            for (int i = 0; i < 4; i++)
            {
                switch (i)
                {
                case 0:
                    angle = 0.0f;
                    break;
                case 1:
                    angle = static_cast<float>(M_PI);
                    break;
                case 2:
                    angle = static_cast<float>(-M_PI/2);
                    break;
                case 3:
                    angle = static_cast<float>(M_PI/2);
                    break;
                default:
                    break;
                }
                target->GetFirstCollisionPosition(pos[i], dist, angle);
                if (maxDist < target->GetDistance2d(pos[i].m_positionX, pos[i].m_positionY))
                {
                    index = i;
                    maxDist = target->GetDistance2d(pos[i].m_positionX, pos[i].m_positionY);
                }
            }
        }

        uint32 GetPortalSpellByIndex(int index)
        {
            switch (index)
            {
                case 0:
                    return SPELL_DOMINATION_TRACKER_SUMMON_NORTH;
                    break;
                case 1:
                    return SPELL_DOMINATION_TRACKER_SUMMON_SUD;
                    break;
                case 2:
                    return SPELL_DOMINATION_TRACKER_SUMMON_EAST;
                    break;
                case 3:
                    return SPELL_DOMINATION_TRACKER_SUMMON_WEST;
                    break;
                default:
                    return SPELL_DOMINATION_TRACKER_SUMMON_NORTH;
                    break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim() && _currentPhase != PHASE_INTRO && _currentPhase != PHASE_2)
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_INTRO:
                        if (alreadyStart)
                        {
                            me->RemoveAllAuras();
                            events.ScheduleEvent(EVENT_NEFARIAN_INTRO_2, 9000);
                            break;
                        }
                        else
                            alreadyStart = true;
                        me->CastSpell(me, SPELL_INTRO_2_STALKER_TRANSFORM, true);
                        me->CastSpell(me, SPELL_INTRO_SHINK_AURA, true);
                        events.ScheduleEvent(EVENT_NEFARIAN_INTRO_1, 29000);
                        break;
                   case EVENT_NEFARIAN_INTRO_1:
                        me->CastSpell(me, SPELL_NEFARIAN_INTRO_4_LIFT_OFF_ANIM_KIT, true);
                        events.ScheduleEvent(EVENT_NEFARIAN_INTRO_2, 10000);
                        break;
                    case EVENT_NEFARIAN_INTRO_2:
                        me->RemoveAllAuras();
                        me->CastSpell(me, SPELL_NEFARIAN_INTRO_4_LIFT_OFF_ANIM_KIT, true);
                        me->GetMotionMaster()->MovePath(NEFARIAN_INTRO_PATH, true);
                        if (Creature *onyxia = me->FindNearestCreature(BOSS_ONYXIA, 200.0f))
                        {
                            onyxia->SetVisible(true);
                            onyxia->AI()->DoAction(ACTION_STOP_INTRO);
                        }
                        break;
                    case EVENT_NEFARIAN_HAIL_OF_BONES:
                        DoCast(SPELL_NEFARIAN_HAIL_OF_BONES);
                        break;
                    case EVENT_START_PHASE_1:
                        me->SetHover(false);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                        me->GetMotionMaster()->MoveIdle();
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->SetInCombatWithZone();
                        if (Unit* victim = me->SelectVictim())
                            AttackStart(victim);
                        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                        changePhase(PHASE_1);
                        break;
                    case EVENT_NEFARIAN_ELETRIC_CHARGE:
                        DoCast(SPELL_NEFARIAN_ELETRIC_CHARGE);
                        break;
                    case EVENT_NEFARIAN_SHADOW_OF_COWARDICE:
                        if (_currentPhase == PHASE_3)
                        {
                            std::list<Creature *> creatures;
                            GetCreatureListWithEntryInGrid(creatures, me, NPC_ANIMATED_BONE_WARRIOR, 500.0f);
                            if (!creatures.empty())
                                for (std::list<Creature *>::iterator iter = creatures.begin(); iter != creatures.end(); iter++)
                                    if (Creature *c = *iter)
                                        c->NearTeleportTo(c->GetPositionX(), c->GetPositionY(), 9.0f, c->GetOrientation());
                        }
                        DoCast(SPELL_NEFARIAN_SHADOW_OF_COWARDICE);
                        break;
                    case EVENT_NEFARIAN_DEATHWING_CHILDREN_PERIODIC_DUMMY:
                        DoCast(SPELL_NEFARIAN_DEATHWING_CHILDREN_PERIODIC_DUMMY);
                        break;
                    case EVENT_NEFARIAN_TAIL_LASH:
                        DoCast(SPELL_NEFARIAN_TAIL_LASH);
                        events.ScheduleEvent(EVENT_NEFARIAN_TAIL_LASH, urand(11000, 15000), 0, _currentPhase);
                        break;
                    case EVENT_NEFARIAN_DOMINION:
                    {
                        std::list<Unit * > targets;
                        Map::PlayerList const& players = me->GetMap()->GetPlayers();
                        if (!players.isEmpty())
                            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                                if (Player* player = itr->getSource())
                                    if (player->isAlive() && !player->HasAura(SPELL_HITTING_YA_PLAYER) && !player->HasAura(SPELL_DOMINION) && !player->HasAura(SPELL_PLAYER_MIND_LIBERATE))
                                        targets.push_back(player);
                        if (targets.empty())
                        {
                            events.ScheduleEvent(EVENT_NEFARIAN_DOMINION, 1000, 0, _currentPhase);
                            break;
                        }
                        for (int i = 0; i < RAID_MODE(2, 5, 2, 5); i++) // set to 1 for test only
                        {
                            if (targets.empty())
                                break;
                            if (Unit* target = Trinity::Containers::SelectRandomContainerElement(targets))
                            {
                                if (target->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_FALLING_FAR))
                                    target->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_FALLING_FAR);
                                if (target->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_FALLING))
                                    target->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_FALLING);
                                int index = 0;
                                CheckNearPortalPosition(index, target);
                                uint32 spell_id = GetPortalSpellByIndex(index);
                                target->CastSpell(target, spell_id, true);
                                me->AddAura(SPELL_DOMINION, target);
                                me->AddAura(SPELL_DOMINION_IMMUNE_MASK, target);
                                targets.remove(target);
                            }
                        }
                        events.ScheduleEvent(EVENT_NEFARIAN_DOMINION, urand(15000, 16000), 0, _currentPhase);
                        break;
                    }
                    case EVENT_NEFARIAN_SHADOWFLAME_BREATH:
                        DoCast(SPELL_NEFARIAN_SHADOWFLAME_BREATH);
                        events.ScheduleEvent(EVENT_NEFARIAN_SHADOWFLAME_BREATH, urand(11000, 25000), 0, _currentPhase);
                        break;
                    case EVENT_NEFARIAN_ELECTROCUTE:
                        me->CastSpell(me, SPELL_NEFARIAN_ELECTROCUTE, true);
                        if (Creature *nefarianLightingMachine = me->FindNearestCreature(NPC_NEFARIAN_LIGHTING_MACHINE, 1000.0f))
                            nefarianLightingMachine->CastSpell(nefarianLightingMachine, SPELL_NEFARIAN_ELECTROCUTE_DUMMY, true);
                        break;
                    case EVENT_NEFARIAN_CLEAVE:
                        DoCastVictim(SPELL_NEFARIAN_CLEAVE);
                        events.ScheduleEvent(EVENT_NEFARIAN_CLEAVE, urand(10000, 12000), 0, _currentPhase);
                        break;
                    case EVENT_NEFARIAN_BRUSHFIRE_SUMMON:
                        //                        me->CastSpell(me, SPELL_NEFARIAN_OMBREFLAME, true);
                        //                        DoCast(SPELL_NEFARIAN_OMBREFLAME);
                        me->CastCustomSpell(SPELL_NEFARIAN_OMBREFLAME, SPELLVALUE_MAX_TARGETS, RAID_MODE(4, 10, 4, 10), me, true);
                        events.ScheduleEvent(EVENT_NEFARIAN_BRUSHFIRE_SUMMON, 2000, 0, PHASE_2);
                        break;
                    case EVENT_CHROMATIC_SPAWN:
                        me->SummonCreature(NPC_CHROMATIC_PROTOTYPE, -183.972f, -225.163f, 43.17013f, 0.05f);
                        me->SummonCreature(NPC_CHROMATIC_PROTOTYPE, -63.20486f, -135.6719f, 65.17735f, 4.29f);
                        me->SummonCreature(NPC_CHROMATIC_PROTOTYPE, -62.87326f, -312.467f, 65.01746f, 2.007f);
                        break;
                    case EVENT_ELEVATOR_UP_P3:
                        me->RemoveAurasDueToSpell(SPELL_NEFARIAN_SHADOW_OF_COWARDICE);
                        instance->SetData(DATA_NEFARIAN_ELEVATOR, GO_STATE_TRANSPORT_READY);
                        events.ScheduleEvent(EVENT_NEFARIAN_PHASE_3, 9000, 0, _currentPhase);
                        break;
                    case EVENT_NEFARIAN_PHASE_3:
                        me->SetHover(false);
                        me->SetCanFly(false);
                        me->GetMotionMaster()->MoveIdle();
                        me->SetReactState(REACT_AGGRESSIVE);
                        if (Unit* victim = me->SelectVictim())
                            AttackStart(victim);
                        changePhase(PHASE_3);
                        break;
                    case EVENT_NEFARIAN_OMBRASE:
                    {
                        Talk(8);
                        std::list<Creature *> creatures;
                        GetCreatureListWithEntryInGrid(creatures, me, NPC_ANIMATED_BONE_WARRIOR, 500.0f);
                        if (!creatures.empty())
                        {
                            Creature *target = NULL;
                            for (std::list<Creature *>::iterator iter = creatures.begin(); iter != creatures.end(); iter++)
                                if (Creature *c = *iter)
                                    if (c->HasAura(SPELL_ONYXIA_FEIGN_DEATH))
                                    {
                                        target = *iter;
                                        break;
                                    }
                            if (target == NULL)
                            {
                                std::list<Creature *>::const_iterator itr = creatures.begin();
                                std::advance(itr, rand() % creatures.size());
                                target = *itr;
                            }
                            if (target)
                                me->CastSpell(target, SPELL_NEFARIAN_OMBRASE, false);
                        }
                        if (_ombraseTimer > 10000)
                            _ombraseTimer -= 5000;
                        events.ScheduleEvent(EVENT_NEFARIAN_OMBRASE, _ombraseTimer, 0, _currentPhase);
                        break;
                    }
                    case EVENT_NEFARIAN_EXPLOSIVE_CINDER:
                    {
                        me->CastCustomSpell(SPELL_NEFARIAN_EXPLOSIVE_CINDER, SPELLVALUE_MAX_TARGETS, 1, me, true);
                        events.ScheduleEvent(EVENT_NEFARIAN_EXPLOSIVE_CINDER, urand(22000, 26000), 0, _currentPhase);
                        break;
                    }
                    case EVENT_BERSERK:
                        DoCast(me, SPELL_BERSERK);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private :
        uint32 electrocuteCount;
        EncounterPhases _currentPhase;
        uint32 _chromaticCount;
        uint32 _ombraseTimer;
        bool alreadyStart;
        bool achievement;
        bool p3Started;
    };
public:
    boss_bwd_nefarian() : CreatureScript("boss_bwd_nefarian") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new boss_bwd_nefarianAI(creature);
    }
};

// NPC_ANIMATED_BONE_WARRIOR = 41918
class npc_animated_bone_warrior : public CreatureScript
{
public:
    npc_animated_bone_warrior() : CreatureScript("npc_animated_bone_warrior") { }

    struct npc_animated_bone_warriorAI : public ScriptedAI
    {
        npc_animated_bone_warriorAI(Creature* creature) : ScriptedAI(creature),
                                                          _instance(creature->GetInstanceScript())
        {
        }

        void Reset()
        {
            me->SetMaxPower(POWER_ENERGY, 100);
            me->SetPower(POWER_ENERGY, 100);
            DoCast(SPELL_FULL_POWER_NO_REGEN);
            me->AddAura(SPELL_EMPOWERING_STRIKES, me);
            me->CastSpell(me, SPELL_HITTING_YA, true);
            _events.Reset();
            _events.ScheduleEvent(EVENT_LOST_ENERGY, 1000);
        }

        void SpellHit(Unit* /*caster*/, SpellInfo const* spell)
        {
        }

        void DoMeleeAttackIfReady()
        {
            if (me->getVictim())
            {
                if (me->isAttackReady() && !me->IsNonMeleeSpellCasted(false))
                {
                    if (me->IsWithinMeleeRange(me->getVictim()))
                    {
                        me->AttackerStateUpdate(me->getVictim());
                        me->resetAttackTimer();
                    }
                    else if (!me->HasUnitState(UNIT_STATE_CASTING))
                    {
                        if (me->HasUnitState(UNIT_STATE_ROOT))
                        {
                            if (Creature *nefarian = me->FindNearestCreature(BOSS_NEFARIAN, 500))
                                if (nefarian->AI()->GetData(DATA_PHASE) == PHASE_3)
                                    DoCastVictim(SPELL_BONES_LAUNCH);
                            me->resetAttackTimer();
                        }
                    }
                }
            }
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_DESACTIVATE_BONES_WARRIOR)
            {
                Position HomePosition;
                me->GetPosition(&HomePosition);
                if (HomePosition.m_positionZ < 9.0f)
                    HomePosition.m_positionZ = 9.0f;
                me->SetHomePosition(HomePosition);
                me->RemoveAurasDueToSpell(SPELL_EMPOWERING_STRIKES);
                me->RemoveAura(RAID_MODE(79330, 94091, 94092, 94093));
                me->CastSpell(me, SPELL_ONYXIA_FEIGN_DEATH, true);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            _events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (!me->HasAura(SPELL_ONYXIA_FEIGN_DEATH) && !me->HasAura(SPELL_EMPOWERING_STRIKES))
                me->AddAura(SPELL_EMPOWERING_STRIKES, me);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_LOST_ENERGY:
                        if (me->GetPower(POWER_ENERGY) <= 2)
                            DoAction(ACTION_DESACTIVATE_BONES_WARRIOR);
                        else if (me->HasAura(SPELL_ONYXIA_FEIGN_DEATH))
                        {
                            me->RemoveAura(SPELL_ONYXIA_FEIGN_DEATH);
                            me->DeleteThreatList();
                            me->SetInCombatWithZone();
                            me->AddAura(SPELL_EMPOWERING_STRIKES, me);
                        }
                        else
                            me->ModifyPower(POWER_ENERGY, -2);
                        _events.ScheduleEvent(EVENT_LOST_ENERGY, 1000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private:
        EventMap _events;
        InstanceScript* _instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_animated_bone_warriorAI(creature);
    }
};

// 78949
class spell_onyxia_eletric_charge_periodic_dummy : public SpellScriptLoader
{
public:
    spell_onyxia_eletric_charge_periodic_dummy() : SpellScriptLoader("spell_onyxia_eletric_charge_periodic_dummy") { }

    class spell_onyxia_eletric_charge_periodic_dummy_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_onyxia_eletric_charge_periodic_dummy_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            tick = 0;
            return true;
        }

        void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
        {
            tick++;
            if (Unit *target = GetCaster())
            {
                if (target->GetPower(POWER_ALTERNATE_POWER) == 100)
                {
                    if (Creature *onyxia = target->ToCreature())
                        onyxia->AI()->DoAction(ACTION_OVERCHARGE);
                    target->ModifyPower(POWER_ALTERNATE_POWER, -100);
                }
                else if (tick % 3 == 0)
                    target->ModifyPower(POWER_ALTERNATE_POWER, 1);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_onyxia_eletric_charge_periodic_dummy_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }

    private :
        uint32 tick;

    };

    AuraScript* GetAuraScript() const
    {
        return new spell_onyxia_eletric_charge_periodic_dummy_AuraScript();
    }
};

// SPELL_ONYXIA_DEATHWING_CHILDREN_PERIODIC_DUMMY 80785
class spell_onyxia_deathwing_children_periodic_dummy : public SpellScriptLoader
{
public:
    spell_onyxia_deathwing_children_periodic_dummy() : SpellScriptLoader("spell_onyxia_deathwing_children_periodic_dummy") { }

    class spell_onyxia_deathwing_children_periodic_dummy_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_onyxia_deathwing_children_periodic_dummy_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            return true;
        }

        void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
        {
            if (Unit* onyxia = GetCaster())
                if (Creature* nefarian = onyxia->FindNearestCreature(BOSS_NEFARIAN, 35.0f))
                    if (!onyxia->HasAura(SPELL_ONYXIA_DEATHWING_CHILDREN))
                        onyxia->AddAura(SPELL_ONYXIA_DEATHWING_CHILDREN, onyxia);

        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_onyxia_deathwing_children_periodic_dummy_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_onyxia_deathwing_children_periodic_dummy_AuraScript();
    }
};


// SPELL_NEFARIAN_DEATHWING_CHILDREN_PERIODIC_DUMMY 80787
class spell_nefarian_deathwing_children_periodic_dummy : public SpellScriptLoader
{
public:
    spell_nefarian_deathwing_children_periodic_dummy() : SpellScriptLoader("spell_nefarian_deathwing_children_periodic_dummy") { }

    class spell_nefarian_deathwing_children_periodic_dummy_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_nefarian_deathwing_children_periodic_dummy_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            return true;
        }

        void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
        {
            if (Unit *nefarian = GetCaster())
                if (Creature *onyxia = nefarian->FindNearestCreature(BOSS_ONYXIA, 35.0f))
                    if (!nefarian->HasAura(SPELL_NEFARIAN_DEATHWING_CHILDREN))
                        nefarian->AddAura(SPELL_NEFARIAN_DEATHWING_CHILDREN, nefarian);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_nefarian_deathwing_children_periodic_dummy_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_nefarian_deathwing_children_periodic_dummy_AuraScript();
    }
};

// SPELL_NEFARIAN_ELETRIC_CHARGE 95793
class spell_nefarian_eletric_charge_periodic_dummy : public SpellScriptLoader
{
public:
    spell_nefarian_eletric_charge_periodic_dummy() : SpellScriptLoader("spell_nefarian_eletric_charge_periodic_dummy") { }

    class spell_nefarian_eletric_charge_periodic_dummy_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_nefarian_eletric_charge_periodic_dummy_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            return true;
        }

        void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
        {
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_nefarian_eletric_charge_periodic_dummy_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_nefarian_eletric_charge_periodic_dummy_AuraScript();
    }
};

// SPELL_NEFARIAN_SHADOW_OF_COWARDICE_TRIGGER 80963
class spell_nefarian_shadow_of_cowardice_dummy : public SpellScriptLoader
{
public:
    spell_nefarian_shadow_of_cowardice_dummy() : SpellScriptLoader("spell_nefarian_shadow_of_cowardice_dummy") { }

    class spell_nefarian_shadow_of_cowardice_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefarian_shadow_of_cowardice_dummy_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit *caster = GetCaster())
            {
                Map::PlayerList const& players = caster->GetMap()->GetPlayers();
                if (!players.isEmpty())
                    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                        if (Player* player = itr->getSource())
                            if (player->isAlive())
                                if (!AccountMgr::IsGMAccount(player->GetSession()->GetSecurity()))
                                    if (player->GetPositionZ() >= 12.59f)
                                        caster->CastSpell(player, SPELL_ONYXIA_SHADOW_OF_COWARDICE, true);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_nefarian_shadow_of_cowardice_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_nefarian_shadow_of_cowardice_dummy_SpellScript();
    }
};

// SPELL_DOMINION_DUMMY 94211
class spell_nefarian_dominion_dummy : public SpellScriptLoader
{
public:
    spell_nefarian_dominion_dummy() : SpellScriptLoader("spell_nefarian_dominion_dummy") { }

    class spell_nefarian_dominion_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefarian_dominion_dummy_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {

        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_nefarian_dominion_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_nefarian_dominion_dummy_SpellScript();
    }
};

// SPELL_NEFARIAN_SHADOWFLAME_BREATH 77826
class spell_onyxia_nefarian_shadowflame_breath_dummy : public SpellScriptLoader
{
public:
    spell_onyxia_nefarian_shadowflame_breath_dummy() : SpellScriptLoader("spell_onyxia_nefarian_shadowflame_breath_dummy") { }

    class spell_onyxia_nefarian_shadowflame_breath_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_onyxia_nefarian_shadowflame_breath_dummy_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            // Also resurrects Animated Bone Warriors in the area of effect. from http://www.wowwiki.com/Nefarian_%28Blackwing_Descent_tactics%29
            if (Unit *target = GetHitUnit())
                target->CastSpell(target, SPELL_ANIMATED_BONES, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_onyxia_nefarian_shadowflame_breath_dummy_SpellScript::HandleScriptEffect, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_onyxia_nefarian_shadowflame_breath_dummy_SpellScript();
    }
};

class spell_onyxia_nefarian_tail_slash : public SpellScriptLoader
{
public :
    spell_onyxia_nefarian_tail_slash() : SpellScriptLoader("spell_onyxia_nefarian_tail_slash") {}

    class spell_onyxia_nefarian_tail_slash_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_onyxia_nefarian_tail_slash_SpellScript);

        bool Load()
        {
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            unitList.clear();
            Unit *caster = GetCaster();
            if (!caster)
                return;
            Map *map = caster->GetMap();
            if (!map)
                return;
            Map::PlayerList const &PlayerList = map->GetPlayers();
            if (PlayerList.isEmpty())
                return;
            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                if (Player *player = i->getSource())
                    if (player->isAlive())
                        if (!AccountMgr::IsGMAccount(player->GetSession()->GetSecurity()))
                            if (caster->isInBackInMap(player, 40.0f, static_cast<float>(M_PI / 6)))
                                unitList.push_back(player);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_onyxia_nefarian_tail_slash_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_104);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_onyxia_nefarian_tail_slash_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_CONE_ENEMY_104);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_onyxia_nefarian_tail_slash_SpellScript();
    }
};

class spell_nefarian_electrocute_dummy : public SpellScriptLoader
{
public:
    spell_nefarian_electrocute_dummy() : SpellScriptLoader("spell_nefarian_electrocute_dummy") { }

    class spell_nefarian_electrocute_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefarian_electrocute_dummy_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit *target = GetHitUnit())
                if (target->GetTypeId() != TYPEID_PLAYER)
                    if (target->GetEntry() == BOSS_ONYXIA)
                        target->ModifyPower(POWER_ALTERNATE_POWER, 25);
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            if (Unit *me = GetCaster())
            {
                std::list<Creature *> creatures;
                GetCreatureListWithEntryInGrid(creatures, me, NPC_INVISIBLE_STALKER_CATACLYSM_BOSS_2, 1000.0f);
                if (!creatures.empty())
                    for (std::list<Creature *>::iterator iter = creatures.begin(); iter != creatures.end(); iter++)
                        unitList.push_back(*iter);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_nefarian_electrocute_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_nefarian_electrocute_dummy_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);

        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_nefarian_electrocute_dummy_SpellScript();
    }
};

class LightingDischargeTargetSelector
{

public:
    explicit LightingDischargeTargetSelector(Unit* unit) : _me(unit) {};

    bool operator()(WorldObject* target) const
    {
        return _me->isInFrontInMap(target->ToUnit(), 60.0f, static_cast<float>(M_PI / 5)) || _me->isInBackInMap(target->ToUnit(), 60.0f, static_cast<float>(M_PI / 5));
    }

private:
    Unit const* _me;
};

class spell_lighting_discharge_damage : public SpellScriptLoader
{
    class spell_lighting_discharge_damage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_lighting_discharge_damage_SpellScript);

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            if (Unit *caster = GetCaster())
                unitList.remove_if(LightingDischargeTargetSelector(caster));
        }

        void HandleEffect(SpellEffIndex /*effIndex*/)
        {
            if (Unit * target = GetHitUnit())
            {
                int32 damage = GetHitDamage();
                int32 eletric = target->GetPower(POWER_ALTERNATE_POWER);
                SetHitDamage(damage + (damage * eletric * 2) / 100);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_lighting_discharge_damage_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(spell_lighting_discharge_damage_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };
public:
    spell_lighting_discharge_damage() : SpellScriptLoader("spell_lighting_discharge_damage") { }

    SpellScript* GetSpellScript() const
    {
        return new spell_lighting_discharge_damage_SpellScript();
    }
};

enum malorianIntro_spells
{
    SPELL_AURA_OF_DREAD_1 = 78495,
    SPELL_AURA_OF_DREAD_2 = 78494,
    SPELL_DRAGON_ORB = 78173,
    SPELL_DRAGON_ORB_TRIGGER_1 = 78220,
    SPELL_DRAGON_ORB_TRIGGER_2 = 78219,
    SPELL_ORANGE_COMSMETIC_VISUAL = 80857,
};

enum nafriusIntroActions
{
    START_INTRO = 1,
};

enum nefariusIntroEvents
{
    EVENT_START_INTRO = 1,
    EVENT_INTRO_STEP_1,
    EVENT_INTRO_STEP_2,
    EVENT_INTRO_STEP_3,
    EVENT_DISAPEAR,
};

class npc_nefarius_nefarian_intro : public CreatureScript
{
public:
    npc_nefarius_nefarian_intro() : CreatureScript("npc_nefarius_nefarian_intro") { }

    struct npc_nefarius_nefarian_introAI : public ScriptedAI
    {
        npc_nefarius_nefarian_introAI(Creature* creature) : ScriptedAI(creature),
                                                          _instance(creature->GetInstanceScript())
        {
        }

        void Reset()
        {
            _events.Reset();
            me->CastSpell(me, 83970, true);
            me->CastSpell(me, SPELL_AURA_OF_DREAD_2, true);
            if (Creature *c = me->SummonCreature(NPC_INVISIBLE_STALKER, -27.80208f, -224.4497f, 63.34686f, 0.0f))
                c->CastSpell(c, SPELL_DRAGON_ORB, true);
            // me->SetVisible(false);
            start = false;
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case START_INTRO:
                    me->RemoveAurasDueToSpell(83970);
                    if (!start)
                        _events.ScheduleEvent(EVENT_START_INTRO, 1000);
                    start = true;
                    break;
                default:
                    break;
            }
        }

        void SpellHit(Unit* /*caster*/, SpellInfo const* spell)
        {
        }


        void UpdateAI(uint32 const diff)
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_START_INTRO:
                        me->SummonCreature(BOSS_NEFARIAN, me->GetPositionX(),  me->GetPositionY(),  me->GetPositionZ(), me->GetOrientation());
                        _events.ScheduleEvent(EVENT_INTRO_STEP_1, 1000);
                        break;
                    case EVENT_INTRO_STEP_1:
                        Talk(0);
                        _events.ScheduleEvent(EVENT_INTRO_STEP_2, 21000);
                        break;
                    case EVENT_INTRO_STEP_2:
                        Talk(1);
                        if (Creature *onyxia = me->FindNearestCreature(BOSS_ONYXIA, 200.0f))
                            onyxia->AI()->DoAction(ACTION_INTRO);
                        _instance->SetData(DATA_NEFARIAN_ELEVATOR, GO_STATE_TRANSPORT_READY);
                        _events.ScheduleEvent(EVENT_INTRO_STEP_3, 11000);
                        break;
                    case EVENT_INTRO_STEP_3:
                        Talk(2);
                        _events.ScheduleEvent(EVENT_DISAPEAR, 6000);
                        break;
                    case EVENT_DISAPEAR:
                        me->SetVisible(false);
                        me->DespawnOrUnsummon(1000);
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        EventMap _events;
        InstanceScript* _instance;
        bool start;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_nefarius_nefarian_introAI(creature);
    }
};

#define GOSSIP_DRAGON_ORB "lancer le combat"

class go_dragon_orb : public GameObjectScript
{
public:
    go_dragon_orb() : GameObjectScript("go_dragon_orb") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        // add here the previous boss done check
        if (InstanceScript *instance = go->GetInstanceScript())
        {
            if (instance->IsDone(DATA_NEFARIAN))
                return false;
            if (!(instance->IsDone(DATA_MAGMAW) && instance->IsDone(DATA_OMNOTRON_DEFENSE_SYSTEM)  &&
                  instance->IsDone(DATA_MALORIAK) && instance->IsDone(DATA_ATRAMEDES)  &&
                  instance->IsDone(DATA_CHIMAERON)))
                return false;

            if (instance->instance->IsHeroic() && (!instance->IsDoneInHeroic(DATA_MAGMAW) ||
                                                   !instance->IsDoneInHeroic(DATA_OMNOTRON_DEFENSE_SYSTEM) ||
                                                   !instance->IsDoneInHeroic(DATA_MALORIAK) ||
                                                   !instance->IsDoneInHeroic(DATA_ATRAMEDES) ||
                                                   !instance->IsDoneInHeroic(DATA_CHIMAERON)))
                return false;

            if (Creature *nefarius = go->FindNearestCreature(NPC_NEFARIUS_INTRO, 200, true))
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_DRAGON_ORB, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
                nefarius->SetVisible(true);
            }
            else if (Creature *nefarius = go->FindNearestCreature(NPC_NEFARIUS_INTRO, 200, false))
            {
                nefarius->Respawn(true);
                if (Creature *onyxia = go->FindNearestCreature(BOSS_ONYXIA, 500.0f, false))
                    onyxia->Respawn(true);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_DRAGON_ORB, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
                nefarius->SetVisible(true);

            }
        }
        player->SEND_GOSSIP_MENU(11492, go->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, GameObject* go, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        player->CLOSE_GOSSIP_MENU();
        if (Creature *c = go->FindNearestCreature(NPC_INVISIBLE_STALKER, 10, true))
            c->DespawnOrUnsummon();
        if (Creature *c = go->FindNearestCreature(NPC_NEFARIUS_INTRO, 200, true))
            c->AI()->DoAction(START_INTRO);
        return true;
    }
};

class spell_nefarian_magma_damage : public SpellScriptLoader
{
public:
    spell_nefarian_magma_damage() : SpellScriptLoader("spell_nefarian_magma_damage") { }

    class spell_nefarian_magma_damage_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_nefarian_magma_damage_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            return true;
        }

        void onPeriodicTick(AuraEffect const* /*aurEff*/)
        {
            if (Unit* caster = GetCaster())
                if (AuraEffect* effect = GetAura()->GetEffect(EFFECT_0))
                    effect->RecalculateAmount(caster);
        }

        void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
        {
            canBeRecalculated = true;
            if (Unit* caster = GetCaster())
            {
                uint32 spellId = sSpellMgr->GetSpellIdForDifficulty(81118, GetCaster());
                if (Aura* magmaStack = caster->GetAura(spellId, caster->GetGUID()))
                    amount = 5000 + magmaStack->GetStackAmount() * magmaStack->GetSpellInfo()->Effects[EFFECT_0].BasePoints;
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_nefarian_magma_damage_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_nefarian_magma_damage_AuraScript::onPeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_nefarian_magma_damage_AuraScript();
    }
};

class spell_nefarian_ombrase : public SpellScriptLoader
{
public:
    spell_nefarian_ombrase() : SpellScriptLoader("spell_nefarian_ombrase") { }

    class spell_nefarian_ombrase_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefarian_ombrase_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            if (Unit *target = GetHitUnit())
                if (Creature *nefarian = target->FindNearestCreature(BOSS_NEFARIAN, 500))
                    if (nefarian->AI()->GetData(DATA_PHASE) == PHASE_3)
                    {
                        target->CastSpell(target, SPELL_ANIMATED_BONES, true);
                        target->RemoveAurasByType(SPELL_AURA_MOD_STUN);
                        target->RemoveAurasByType(SPELL_AURA_MOD_CONFUSE);
                        target->RemoveAurasByType(SPELL_AURA_MOD_CHARM);
                        target->RemoveAurasByType(SPELL_AURA_MOD_FEAR);
                        target->RemoveAurasByType(SPELL_AURA_MOD_ROOT);
                        target->RemoveAurasByType(SPELL_AURA_MOD_STUN);
                    }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_nefarian_ombrase_SpellScript::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_nefarian_ombrase_SpellScript();
    }
};

class spell_nefarian_ombrase_P2 : public SpellScriptLoader
{
public :
    spell_nefarian_ombrase_P2() : SpellScriptLoader("spell_nefarian_ombrase_P2") {}

    class spell_nefarian_ombrase_P2_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefarian_ombrase_P2_SpellScript);

        bool Load()
        {
            for (int cnt = 0; cnt < 3; cnt++)
            {
                pillar[cnt] = NULL;
                pl[cnt] = NULL;
            }
            return true;
        }

        bool BadTarget (const WorldObject *player)
        {
            if (player->GetTypeId() == TYPEID_PLAYER)
                for (int cnt = 0; cnt < 3; cnt++)
                    if (pl[cnt])
                        if (player->GetDistance(pl[cnt]) <= 1.0f)
                            return false;
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            if (!GetCaster())
                return;
            std::list<Creature *> creatures;
            GetCreatureListWithEntryInGrid(creatures, GetCaster(), NPC_PILLAR_MARKER, 500.0f);
            int cnt = 0;
            for (std::list<Creature *>::iterator itr = creatures.begin(); itr != creatures.end(); itr++)
            {
                if (cnt < 3)
                    pillar[cnt] = *itr;
                cnt++;
            }

            for (std::list<WorldObject*>::iterator itr = unitList.begin(); itr != unitList.end(); itr++)
            {
                if (Player *player = (*itr)->ToPlayer())
                {
                    for (int cnt = 0; cnt < 3; cnt++)
                        if (Creature *marker = pillar[cnt])
                            if (pl[cnt] == NULL)
                                if (player->GetDistance(marker) <= 7.0f)
                                    pl[cnt] = player;
                }
            }

            for (std::list<WorldObject*>::iterator itr = unitList.begin(); itr != unitList.end(); )
            {
                if (BadTarget(*itr))
                    itr = unitList.erase(itr);
                else
                    ++itr;
            }
        }

    private:
        Creature *pillar[3];
        Player *pl[3];

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_nefarian_ombrase_P2_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_nefarian_ombrase_P2_SpellScript();
    }
};

class npc_dominion_tracker_summon : public CreatureScript
{
public:
    npc_dominion_tracker_summon() : CreatureScript("npc_dominion_tracker_summon") {}

    struct npc_dominion_tracker_summonAI : public ScriptedAI
    {
        npc_dominion_tracker_summonAI(Creature * creature) : ScriptedAI(creature)
        {
        }

        void Reset()
        {
            plGUID = 0;
            checkPlDist = 1000;
        }

        void IsSummonedBy(Unit *player)
        {
            if (Creature *nefarian = me->FindNearestCreature(BOSS_NEFARIAN, 500))
                nefarian->AI()->JustSummoned(me);
            Position pos;
            me->GetPosition(&pos);
            player->GetMotionMaster()->Clear();
            player->GetMotionMaster()->ForceMovePoint(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 1.2f, 0, false);
            plGUID = player->GetGUID();
        }

        void UpdateAI(const uint32 diff)
        {
            if (checkPlDist <= diff)
            {
                if (Unit *player = Unit::GetUnit(*me, plGUID))
                    if (me->GetDistance2d(player->GetPositionX(), player->GetPositionY()) <= 0.5f)
                    {
                        player->Kill(player);
                        me->DespawnOrUnsummon(1000);
                    }
                checkPlDist = 1000;
            }
            else checkPlDist -= diff;
        }

    private :
        uint64 plGUID;
        uint32 checkPlDist;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_dominion_tracker_summonAI(creature);
    }
};

class spell_nefarian_mind_free : public SpellScriptLoader
{
public:
    spell_nefarian_mind_free() : SpellScriptLoader("spell_nefarian_mind_free") { }

    class spell_nefarian_mind_free_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_nefarian_mind_free_AuraScript);

        void Remove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* target = GetTarget())
            {
                std::list<Creature *> dominions_trackers;
                target->GetCreatureListWithEntryInGrid(dominions_trackers, NPC_DOMINION_TRACKER, 200.0f);
                for (std::list<Creature *>::iterator itr = dominions_trackers.begin(); itr != dominions_trackers.end(); itr++)
                    if ((*itr)->ToTempSummon() && (*itr)->ToTempSummon()->GetSummonerGUID() == target->GetGUID())
                    {
                        (*itr)->DespawnOrUnsummon(1000);
                        break;
                    }
                target->RemoveAurasDueToSpell(SPELL_DOMINION);
                target->RemoveAurasDueToSpell(SPELL_DOMINION_IMMUNE_MASK);
                target->GetMotionMaster()->Clear();
                target->GetMotionMaster()->MoveIdle();
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_nefarian_mind_free_AuraScript::Remove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_nefarian_mind_free_AuraScript();
    }
};

class spell_nefarian_absorb_power_dummy : public SpellScriptLoader
{
public:
    spell_nefarian_absorb_power_dummy() : SpellScriptLoader("spell_nefarian_absorb_power_dummy") { }

    class spell_nefarian_absorb_power_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_nefarian_absorb_power_dummy_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit *caster = GetCaster())
            {
                if (Aura* aura = caster->GetAura(SPELL_PLAYER_STEALTH_POWER))
                {
                    uint32 stacks = 0;
                    switch (aura->GetStackAmount())
                    {
                        case 1: stacks = 2; break;
                        case 2: stacks = 4; break;
                        case 4: stacks = 7; break;
                        case 7: stacks = 11; break;
                        case 11: stacks = 16; break;
                        case 16: stacks = 22; break;
                        case 22: stacks = 29; break;
                        case 29: stacks = 37; break;
                        case 37: stacks = 46; break;
                        case 46: stacks = 56; break;
                        case 56: stacks = 67; break;
                        case 67: stacks = 79; break;
                        case 79: stacks = 92; break;
                        case 92: stacks = 106; break;
                        case 106: stacks = 121; break;
                        case 121: stacks = 137; break;
                        case 137: stacks = 150; break;
                        default: break;
                    }
                    if (stacks > 0)
                        aura->SetStackAmount(stacks);
                    aura->RefreshDuration();
                }
                else
                    caster->CastSpell(caster, SPELL_PLAYER_STEALTH_POWER, true);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_nefarian_absorb_power_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_nefarian_absorb_power_dummy_SpellScript();
    }
};

class spell_nefarian_explosive_cinder : public SpellScriptLoader
{
public:
    spell_nefarian_explosive_cinder() : SpellScriptLoader("spell_nefarian_explosive_cinder") { }

    class spell_nefarian_explosive_cinder_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_nefarian_explosive_cinder_AuraScript);

        void Remove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* target = GetTarget())
            {
                // Rogues (Cloak of Shadows), Mages (Ice Block) and Paladins (Divine Shield)
                // can get rid of the Explosive Cinders without any negative effects (ie. it will not knock back players on your platform).
                if (target->HasAura(31224) || target->HasAura(45438) || target->HasAura(642))
                    return;

                target->CastSpell(target, SPELL_PLAYER_EXPLOSIVE_CINDER, true);
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_nefarian_explosive_cinder_AuraScript::Remove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_nefarian_explosive_cinder_AuraScript();
    }
};

void AddSC_boss_bwd_nefarian()
{
    new boss_bwd_onyxia();
    new boss_bwd_nefarian();
    new spell_onyxia_eletric_charge_periodic_dummy();
    new spell_onyxia_deathwing_children_periodic_dummy();
    new spell_nefarian_deathwing_children_periodic_dummy();
    new spell_nefarian_eletric_charge_periodic_dummy();
    new spell_nefarian_shadow_of_cowardice_dummy();
    new spell_nefarian_dominion_dummy();
    new spell_onyxia_nefarian_shadowflame_breath_dummy();
    new spell_onyxia_nefarian_tail_slash();
    new spell_nefarian_electrocute_dummy();
    new npc_animated_bone_warrior();
    new spell_lighting_discharge_damage();
    new npc_nefarius_nefarian_intro();
    new go_dragon_orb();
    new spell_nefarian_magma_damage();
    new spell_nefarian_ombrase();
    new npc_dominion_tracker_summon();
    new spell_nefarian_mind_free();
    new spell_nefarian_absorb_power_dummy();
    new spell_nefarian_explosive_cinder();
}
