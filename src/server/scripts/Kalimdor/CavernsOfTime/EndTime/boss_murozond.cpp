/*
 * OMFG.GG
 */

/*
 * TODOO
 * Clone should restore caster auras at rewind time
 *
 */

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "PassiveAI.h"
#include "SpellScript.h"
#include "MoveSplineInit.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "endTime.h"

enum Spells
{
    // 01:32:59.157
    // 01:33:50.294 start fight
    // move X: 4181.117 Y: -420.2193 Z: 138.3806
    // update unit_flags 32768 01:34:02.353
    SPELL_INSTANT_TEMPORALITY           = 101592, // 01:34:30.917 forecast SPELL_INSTANT_TEMPORALITY_FC
    SPELL_INSTANT_TEMPORALITY_FC        = 101593, // summon NPC_MIRROR_IMAGE
    SPELL_DISTORTION_BOMB               = 101983, // missile SPELL_DISTORTION_BOMB_MISSILE 01:34:36.954 01:34:44.193 01:34:50.183 01:34:56.236 01:35:02.242 01:35:13.084
    SPELL_DISTORTION_BOMB_2             = 102516, // missile SPELL_DISTORTION_BOMB_MISSILE 01:35:28.918 01:35:34.924
    SPELL_DISTORTION_BOMB_MISSILE       = 101984,
    SPELL_DISTORTION_BOMB_DUMMY         = 102652, // 01:35:15.174 01:35:16.001 01:35:16.469 01:35:16.703
    SPELL_TEMPORALITY_DEFLAG            = 102381, // 01:34:41.759 01:34:53.818 01:35:05.877 01:35:22.756 01:35:36.157
    SPELL_INFINITE_BREATH               = 102569, // 01:34:45.363 01:35:08.279 01:35:29.963
    SPELL_TAIL_SWEEP                    = 108589, // 01:34:51.384 01:35:03.443 01:35:25.158  01:35:38.481 01:35:50.509

    SPELL_SAND_HOURGLASS                = 102668,

    // GameObject Entry: 209249
    SPELL_REWIND_TIME                   = 101590, // 01:35:14.316 Die Mächte des Stundenglases können mir nichts anhaben! (really ? german is a so beautifull language XDDDD
    SPELL_PLAYERS_REWIND_TIME           = 101591, // effect 0 should trigger SPELL_PLAYERS_BLESSING or 108026 spell_dbc (target area ally)
    SPELL_PLAYERS_BLESSING              = 102364,


    SPELL_TELEPORT_TO_CLONE             = 102818,
    SPELL_TRACK_MASTER_HELPFULL_AURAS   = 102541,
    SPELL_CLONE_MASTER_HEALTH           = 102571,
    SPELL_MARKED_MASTER_AS_DESUMMONED   = 80927,
    SPELL_ALPHA_STATE                   = 69676,
    SPELL_CLONE_ME                      = 45204, // player on clone
    SPELL_CLONE_WEAPONS                 = 41055
};

enum Events
{
    EVENT_INSTANT_TEMPORALITY           = 1,
    EVENT_DISTORTION_BOMB               = 2,
    EVENT_DISTORTION_BOMB_SOFT_ENRAGE   = 3,
    EVENT_TEMPORALITY_DEFLAG            = 4,
    EVENT_INFINITE_BREATH               = 5,
    EVENT_TAIL_SWEEP                    = 6
};

enum Npcs
{
    NPC_MIRROR_IMAGE        = 54435,
    NPC_INFINITE_WARDEN     = 54923,
    NPC_INFINITE_SUPPRESSOR = 54920
};

enum Miscs
{
    ACTION_ENTER_COMBAT      = 1,
    ACTION_REWIND_TIME       = 2,
    DATA_REWIND_COUNT        = 4,
    POINT_ENTER_COMBAT       = 5,
    GO_DYN_DEFLAG            = 101984,
    SPELL_ACHIEVEMENT_CREDIT = 110158, // server side spell
    CRITERIA_GUILD_RUN_END_TIME = 18475
};

Position const infiniteSuppressorPos[4] =
{
    { 4047.39f, -438.898f, 119.089f, 2.22507f },
    { 4061.68f, -427.949f, 118.416f, 2.22513f },
    { 4104.93f, -405.556f, 120.733f, 4.83456f },
    { 4085.56f, -430.510f, 121.071f, 6.17846f }
};

Position const infiniteWardenPos[4] =
{
    { 4124.25f, -407.835f, 122.155f, 4.13643f },
    { 4056.91f, -431.599f, 118.557f, 2.22508f },
    { 4052.15f, -435.248f, 118.609f, 2.22464f },
    { 4088.73f, -414.411f, 120.696f, 5.70723f }
};

// 54432
class boss_murozond : public CreatureScript
{
public:
    boss_murozond() : CreatureScript("boss_murozond") { }

    struct boss_murozondAI : public BossAI
    {
        boss_murozondAI(Creature* creature) : BossAI(creature, BOSS_MUROZOND) 
        {
            trashCount = 0;
            trashSummoned = false;
            creature->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            creature->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
        }

        void Reset()
        {
            if (!trashSummoned)
            {
                trashSummoned = true;
                for (uint8 i = 0; i < 4; i++)
                    me->SummonCreature(NPC_INFINITE_SUPPRESSOR, infiniteSuppressorPos[i], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60000);

                for (uint8 i = 0; i < 4; i++)
                    me->SummonCreature(NPC_INFINITE_WARDEN, infiniteWardenPos[i], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60000);
            }

            me->RemoveDynObject(GO_DYN_DEFLAG);
            RemoveEncounterFrame();
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SAND_HOURGLASS);
            if (GameObject *go = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_HOURGLASS_OF_TIME_GUID)))
                go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
            _rewindCount = 5;
            events.Reset();
            summons.DespawnEntry(NPC_MIRROR_IMAGE);
            if (me->isAlive())
                instance->SetBossState(BOSS_MUROZOND, NOT_STARTED);
        }

        void JustDied(Unit* killer)
        {
            instance->CompleteGuildCriteriaForGuildGroup(CRITERIA_GUILD_RUN_END_TIME);
            instance->FinishLfgDungeon(me);
            me->RemoveDynObject(GO_DYN_DEFLAG);
            RemoveEncounterFrame();
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SAND_HOURGLASS);
            instance->DoUpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_ACHIEVEMENT_CREDIT);
            if (GameObject *go = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_HOURGLASS_OF_TIME_GUID)))
                go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
            _JustDied();
        }

        void EnterCombat(Unit* who)
        {
            events.ScheduleEvent(EVENT_INSTANT_TEMPORALITY, 3000);
            events.ScheduleEvent(EVENT_DISTORTION_BOMB, 6000);
            events.ScheduleEvent(EVENT_DISTORTION_BOMB_SOFT_ENRAGE, 58000);
            events.ScheduleEvent(EVENT_TEMPORALITY_DEFLAG, 11000);
            events.ScheduleEvent(EVENT_INFINITE_BREATH, 15000);
            events.ScheduleEvent(EVENT_TAIL_SWEEP, 21000);
            me->SetCanFly(false);
            me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, 0x02);
            me->RemoveUnitMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_FLYING);
            AddEncounterFrame();
            _EnterCombat();
        }

        void UseHourglassSand()
        {
            _rewindCount--;
            EntryCheckPredicate pred(NPC_MIRROR_IMAGE);
            summons.DoAction(ACTION_REWIND_TIME, pred);
            me->RemoveDynObject(GO_DYN_DEFLAG);
            if (!_rewindCount)
                if (GameObject *go = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_HOURGLASS_OF_TIME_GUID)))
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TEMPORALITY_DEFLAG);
            events.DelayEvents(5000);
        }

        uint32 GetData(uint32 type) const
        {
            if (type == DATA_REWIND_COUNT)
                return _rewindCount;
            return 0;
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_ENTER_COMBAT:
                    me->GetMotionMaster()->MovePoint(POINT_ENTER_COMBAT, 4181.117f, -420.2193f, 145.3806f);
                    me->SetHomePosition(4181.117f, -420.2193f, 145.3806f, 2.9845f);
                    break;
                case ACTION_REWIND_TIME:
                    if (_rewindCount)
                        UseHourglassSand();
                    break;
            }
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE && id == POINT_ENTER_COMBAT)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE);
                me->SetReactState(REACT_AGGRESSIVE);
            }
        }

        void JustSummoned(Creature *summon) override
        {
            switch (summon->GetEntry())
            {
                case NPC_INFINITE_WARDEN:
                case NPC_INFINITE_SUPPRESSOR:
                    trashCount++;
                    break;
                default:
                    break;
            }
            BossAI::JustSummoned(summon);
        }

        void SummonedCreatureDies(Creature* summon, Unit* killer)
        {
            switch (summon->GetEntry())
            {
                case NPC_INFINITE_WARDEN:
                case NPC_INFINITE_SUPPRESSOR:
                    if (--trashCount == 0)
                        DoAction(ACTION_ENTER_COMBAT);
                    break;
                default:
                    break;
            }
            BossAI::SummonedCreatureDies(summon, killer);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_INSTANT_TEMPORALITY:
                        DoCast(SPELL_INSTANT_TEMPORALITY);
                        instance->DoCastSpellOnPlayers(SPELL_SAND_HOURGLASS);
                        if (GameObject *go = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_HOURGLASS_OF_TIME_GUID)))
                            go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                        break;
                    case EVENT_DISTORTION_BOMB:
                        DoCastRandom(SPELL_DISTORTION_BOMB, 0.0f);
                        events.ScheduleEvent(EVENT_DISTORTION_BOMB, 6000);
                        break;
                    case EVENT_DISTORTION_BOMB_SOFT_ENRAGE:
                        events.CancelEvent(EVENT_DISTORTION_BOMB);
                        DoCastRandom(SPELL_DISTORTION_BOMB_2, 0.0f);
                        events.ScheduleEvent(EVENT_DISTORTION_BOMB_SOFT_ENRAGE, 6000);
                        break;
                    case EVENT_TEMPORALITY_DEFLAG:
                        DoCast(SPELL_TEMPORALITY_DEFLAG);
                        events.ScheduleEvent(EVENT_TEMPORALITY_DEFLAG, urand(12000, 15000));
                        break;
                    case EVENT_INFINITE_BREATH:
                        DoCast(SPELL_INFINITE_BREATH);
                        events.ScheduleEvent(EVENT_INFINITE_BREATH, 21000);
                        break;
                    case EVENT_TAIL_SWEEP:
                        DoCast(SPELL_TAIL_SWEEP);
                        events.ScheduleEvent(EVENT_TAIL_SWEEP, urand(12000, 20000));
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        uint32 _rewindCount;
        uint8 trashCount;
        bool trashSummoned;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_murozondAI(creature);
    }
};

// 54435
class npc_murozond_player_clone : public CreatureScript
{
public:
    npc_murozond_player_clone() : CreatureScript("npc_murozond_player_clone") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_murozond_player_cloneAI(creature);
    }

    struct npc_murozond_player_cloneAI : public ScriptedAI
    {
        npc_murozond_player_cloneAI(Creature* creature) : ScriptedAI(creature), instance(creature->GetInstanceScript()) { }

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE);
        }

        void IsSummonedBy(Unit* summoner) override
        {
            me->CastSpell(me, SPELL_TRACK_MASTER_HELPFULL_AURAS, true);
            me->CastSpell(me, SPELL_CLONE_MASTER_HEALTH, true);
            me->CastSpell(me, SPELL_MARKED_MASTER_AS_DESUMMONED, true);
            me->CastSpell(me, SPELL_ALPHA_STATE, true);
            summoner->CastSpell(me, SPELL_CLONE_ME, true);
            me->CastSpell(summoner, SPELL_CLONE_WEAPONS, true);
            if (Creature *murozond = Unit::GetCreature(*me, instance->GetData64(DATA_MUROZOND_GUID)))
                murozond->AI()->JustSummoned(me);
            _health = summoner->GetHealth();
            _mana = summoner->GetPower(POWER_MANA);
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_REWIND_TIME:
                {
                    if (Creature *murozond = Unit::GetCreature(*me, instance->GetData64(DATA_MUROZOND_GUID)))
                        if (Unit *summoner = me->ToTempSummon()->GetSummoner())
                            if (Player *player = summoner->ToPlayer())
                            {
                                if (!summoner->isAlive())
                                    player->ResurrectPlayer(100.0f, false);
                                summoner->SetHealth(_health);
                                if (_mana && player->getPowerType() == POWER_MANA)
                                    summoner->SetPower(POWER_MANA, _mana);
                                summoner->CastSpell(me, SPELL_TELEPORT_TO_CLONE, true);
                                summoner->SetPower(POWER_ALTERNATE_POWER, murozond->AI()->GetData(DATA_REWIND_COUNT));
                                player->RemoveAll30MinCdSpellCooldowns();
                                player->CastSpell(player, SPELL_PLAYERS_BLESSING, true);
                                player->RemoveAura(57724);
                                player->RemoveAura(57723);
                                player->RemoveAura(80354);
                                player->RemoveAura(95809);
                            }
                    break;
                }
            }
        }

        void DamageTaken(Unit* /*done_by*/, uint32 & damage) { damage = 0; }

    private:
        uint32 _health;
        uint32 _mana;
        InstanceScript *instance;
    };
};

// 101590
class spell_murozond_rewind_time : public SpellScriptLoader
{
public:
    spell_murozond_rewind_time() : SpellScriptLoader("spell_murozond_rewind_time") { }

    class spell_murozond_rewind_time_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_murozond_rewind_time_SpellScript);

        void HandleOnEffectHitTarget(SpellEffIndex /*effIndex*/)
        {
            if (Unit* target = GetHitUnit())
                if (InstanceScript *instance = target->GetInstanceScript())
                    if (Creature *murozond = Unit::GetCreature(*target, instance->GetData64(DATA_MUROZOND_GUID)))
                        murozond->AI()->DoAction(ACTION_REWIND_TIME);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_murozond_rewind_time_SpellScript::HandleOnEffectHitTarget, EFFECT_1, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_murozond_rewind_time_SpellScript();
    }
};


void AddSC_boss_murozond()
{
    new boss_murozond();
    new npc_murozond_player_clone();
    new spell_murozond_rewind_time();
}
