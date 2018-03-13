/*
 * OMFG.GG
 */

/* TODOO :
 * implement gauntlet event
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



enum Texts
{
    // gauntlet event
    TALK_GAUNTLET_1        = 0,
    TALK_GAUNTLET_2        = 1,
    TALK_GAUNTLET_3        = 2,
    TALK_GAUNTLET_4        = 3,
    TALK_GAUNTLET_5        = 4,
    TALK_GAUNTLET_6        = 5,
    // fight
    TALK_ENTER_COMBAT      = 6,
    TALK_EYES_OF_GODDESS   = 7,
    TALK_MOONLANCE         = 8,
    TALK_GUIDANCE          = 9,
    EMOTE_STARFALL         = 10,
    TALK_TEARS             = 11,
    TALK_DEATH             = 12,
    // moonlight
    TALK_MOONLIGHT_DESPAWN = 5,
};

enum Spells
{
    SPELL_TYRANDE_ACHIEVEMENT_FAIL       = 102539,
    SPELL_TYRANDE_ACHIEVEMENT            = 102542,
    SPELL_TYRANDE_ACHIEVEMENT_TRACKER    = 102491,
    SPELL_IN_SHADOW                      = 101841,

    SPELL_LUNAR_BOLT                     = 102193,
    SPELL_STAR_DUST                      = 102173,
    SPELL_MOONLANCE                      = 102151,
    SPELL_LUNAR_GUIDANCE                 = 102472,
    SPELL_EYES_OF_THE_GODDESS_SC         = 102608,
    //    SPELL_SUMMON_EYES_OF_THE_GODDESS  = 102605, // unsued in sniff
    // SPELL_EYES_OF_THE_GODDESS_2       = 102248, // unsued in sniff
    SPELL_SUMMON_EYES_OF_THE_GODDESS     = 102181,
    SPELL_TEARS_OF_ELUNE                 = 102241,
    SPELL_TEARS_OF_ELUNE_SC              = 102242,
    SPELL_TEARS_OF_ELUNE_MISSILE         = 102243,
    SPELL_TEARS_OF_ELUNE_DAMAGE          = 102244,
    SPELL_TEARS_OF_ELUNE_STUN            = 102249,
    // NPC_GODDESS_EYES
    SPELL_PIERCING_GAZE_OF_ELUNE         = 102182,
    SPELL_PIERCING_GAZE_OF_ELUNE_DAMAGE  = 102182,
    // NPC_ELUNE_MOONLANCE_X
    SPELL_ELUNE_MOONLANCE_AURA           = 102150,
    SPELL_ELUNE_MOONLANCE_DAMAGE         = 102149,
    SPELL_LUNAR_MOONLANCE_DIVIDE         = 102152,
    // NPC_GEN_BUNNY
    SPELL_DARK_LUNAR_LIGHT               = 102414,
    SPELL_MOONLIGHT_AURA                 = 101946,
};

enum Events
{
    EVENT_LUNAR_BOLT = 1,
    EVENT_STAR_DUST,
    EVENT_MOONLANCE,
    EVENT_EYES_GODDESS,
    EVENT_MOVE_TURN,
    EVENT_TEAR_OF_ELUNE,
    EVENT_LUNAR_GUIDANCE,

    EVENT_ACTIVATE_NPCS,
    EVENT_MOONLIGHT_NPCS_CHECK,
    EVENT_ACTIVATE_POOL
};

enum Npcs
{
    NPC_GEN_BUNNY         = 45979, // not impl => SPELL_DARK_LUNAR_LIGHT (gauntlet part)
    NPC_GODDESS_EYES_A    = 54594,
    NPC_GODDESS_EYES_B    = 54597,
    NPC_ELUNE_MOONLANCE   = 54574,
    NPC_ELUNE_MOONLANCE_A = 54580,
    NPC_ELUNE_MOONLANCE_B = 54582,
    NPC_ELUNE_MOONLANCE_C = 54581,
    NPC_POOL_OF_MOONLIGHT = 54508,
    // talk Ein Strahl aus Mondlicht erscheint! Type: RaidBossEmote (41) 01:21:52.736
    // talk Das Mondlicht verschwindet in der Dunkelheit! Type: RaidBossEmote (41) 01:22:23.531
    NPC_TIMED_TWISTED_NIGHT_SABER = 54688,
};

Position const PoolPositions[5] =
{
    {2906.522f, 66.774f, 3.324f, 5.326f},
    {2864.864f, 139.631f, 2.898f, 5.326f},
    {2775.498f, 125.622f, 1.148f, 5.326f},
    {2697.498f, 2.112f, 3.708f, 5.326f},
    {2785.555f, -1.730f, 2.550f, 5.326f},
};

enum Miscs
{
    POINT_TURN = 1,
    ACTION_START_COMBAT = 1,
    ACTION_START_INTRO_EVENT,
    ACTION_ACHIEVMENT_FAIL,
    // areatrigger enter = 6955 01:21:06.669
    // start event 01:21:46.559
    // 54544
};

class boss_echo_of_tyrande : public CreatureScript
{
public:
    boss_echo_of_tyrande() : CreatureScript("boss_echo_of_tyrande") { }
    struct boss_echo_of_tyrandeAI : public BossAI
    {
        boss_echo_of_tyrandeAI(Creature* creature) : BossAI(creature, BOSS_ECHO_OF_TYRANDE) 
        {
            creature->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            creature->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
        }

        void Reset()
        {
            _moonLanceTargetGUID = 0;
            _angle = 0.0f;
            _goddessAngle = 0.0f;
            firstTear = false;
            secondTear = false;
            tearofElune = false;
            _poolCount = 0;
            me->CastSpell(me, SPELL_TYRANDE_ACHIEVEMENT_TRACKER, true);
            _gountletNpcs.clear();
            _npcsSize = 0;
            _currentTrashCount = 0;
            _currenMoonLightGUID = 0;
            _Reset();
            if (instance->GetData(DATA_TYRANDE_GAUNTLET) != DONE)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE);
                isInIntroPhase = true;
            }
            else
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE);
                isInIntroPhase = false;
                DoAction(ACTION_START_COMBAT);
            }
        }

        void JustDied(Unit* killer)
        {
            Talk(TALK_DEATH);
            _JustDied();
        }

        void IntializeGauntletTrashes()
        {
            me->GetCreatureListWithEntryInGrid(_gountletNpcs, 54688, 500.0f);
            me->GetCreatureListWithEntryInGrid(_gountletNpcs, 54699, 500.0f);
            me->GetCreatureListWithEntryInGrid(_gountletNpcs, 54700, 500.0f);
            me->GetCreatureListWithEntryInGrid(_gountletNpcs, 54701, 500.0f);
            me->GetCreatureListWithEntryInGrid(_gountletNpcs, 54512, 500.0f);
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_START_INTRO_EVENT:
                    if (_gountletNpcs.empty())
                    {
                        Talk(TALK_GAUNTLET_1);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE);
                        isInIntroPhase = true;
                        IntializeGauntletTrashes();
                        _npcsSize = _gountletNpcs.size();
                        achievementFailed = false;
                        DoCastAOE(SPELL_TYRANDE_ACHIEVEMENT_TRACKER);
                        events.ScheduleEvent(EVENT_ACTIVATE_POOL, 10000);
                        events.ScheduleEvent(EVENT_MOONLIGHT_NPCS_CHECK, 11000);
                        events.ScheduleEvent(EVENT_ACTIVATE_NPCS, 3000);
                    }
                    break;
                case ACTION_START_COMBAT:
                {
                    if (Creature *moonlight = Unit::GetCreature(*me, _currenMoonLightGUID))
                    {
                        moonlight->AI()->Talk(TALK_MOONLIGHT_DESPAWN);
                        moonlight->DespawnOrUnsummon();
                    }
                    if (Creature *c = me->FindNearestCreature(NPC_GEN_BUNNY, 5.0f))
                        c->CastSpell(c, SPELL_DARK_LUNAR_LIGHT, true);

                    if (!achievementFailed)
                        DoCastAOE(SPELL_TYRANDE_ACHIEVEMENT);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TYRANDE_ACHIEVEMENT_TRACKER);
                    isInIntroPhase = false;
                    events.CancelEvent(EVENT_ACTIVATE_NPCS);
                    events.CancelEvent(EVENT_MOONLIGHT_NPCS_CHECK);
                    events.CancelEvent(EVENT_ACTIVATE_POOL);
                    me->RemoveAurasDueToSpell(SPELL_IN_SHADOW);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    break;
                }
                case ACTION_ACHIEVMENT_FAIL:
                    achievementFailed = true;
                    break;
            }
        }

        void EnterCombat(Unit* who)
        {
            Talk(TALK_ENTER_COMBAT);
            events.ScheduleEvent(EVENT_LUNAR_BOLT, 3000);
            events.ScheduleEvent(EVENT_STAR_DUST, 8000);
            events.ScheduleEvent(EVENT_MOONLANCE, 14000);
            events.ScheduleEvent(EVENT_EYES_GODDESS, 8000);
            _EnterCombat();
        }

        void DamageTaken(Unit* /*caster*/, uint32& damage)
        {
            if (HealthBelowPct(31) && !tearofElune)
            {
                events.ScheduleEvent(EVENT_TEAR_OF_ELUNE, 0);
                tearofElune = true;
            }
            else if (HealthBelowPct(55) && !secondTear)
            {
                events.ScheduleEvent(EVENT_LUNAR_GUIDANCE, 0);
                secondTear = true;
            }
            else if (HealthBelowPct(80) && !firstTear)
            {
                events.ScheduleEvent(EVENT_LUNAR_GUIDANCE, 0);
                firstTear = true;
            }
        }

        void JustSummoned(Creature *summon)
        {
            summon->SetReactState(REACT_PASSIVE);
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            switch (summon->GetEntry())
            {
                case NPC_POOL_OF_MOONLIGHT:
                {
                    _currenMoonLightGUID = summon->GetGUID();
                    summon->CastSpell(summon, SPELL_MOONLIGHT_AURA, true);
                    break;
                }
                case NPC_ELUNE_MOONLANCE:
                {
                    summon->SetReactState(REACT_PASSIVE);
                    summon->CastSpell(summon, SPELL_ELUNE_MOONLANCE_AURA, true);
                    if (Unit *target = Unit::GetUnit(*me, _moonLanceTargetGUID))
                    {
                        _angle = summon->GetAngle(target) - summon->GetOrientation();
                        Position pos;
                        summon->GetPosition(&pos);
                        summon->GetNearPosition(pos, 20.0f, _angle);
                        summon->GetMotionMaster()->MovePoint(0, pos.m_positionX, pos.m_positionY, pos.m_positionZ);
                    }
                    summon->CastWithDelay(1500, summon, SPELL_LUNAR_MOONLANCE_DIVIDE, false);
                    summon->DespawnOrUnsummon(2000);
                    break;
                }
                case NPC_ELUNE_MOONLANCE_A:
                {
                    summon->CastSpell(summon, SPELL_ELUNE_MOONLANCE_AURA, true);
                    Position pos;
                    summon->GetPosition(&pos);
                    float newOri = summon->NormalizeOrientation(_angle + M_PI / 6.0f);
                    summon->GetNearPosition(pos, 40.0f, newOri);
                    summon->GetMotionMaster()->MovePoint(0, pos.m_positionX, pos.m_positionY, pos.m_positionZ);
                    summon->DespawnOrUnsummon(10000);
                    break;
                }
                case NPC_ELUNE_MOONLANCE_B:
                {
                    summon->CastSpell(summon, SPELL_ELUNE_MOONLANCE_AURA, true);
                    Position pos;
                    summon->GetPosition(&pos);
                    float newOri = summon->NormalizeOrientation(_angle - M_PI / 6.0f);
                    summon->GetNearPosition(pos, 40.0f, newOri);
                    summon->GetMotionMaster()->MovePoint(0, pos.m_positionX, pos.m_positionY, pos.m_positionZ);
                    summon->DespawnOrUnsummon(10000);
                    break;
                }
                case NPC_ELUNE_MOONLANCE_C:
                {
                    summon->CastSpell(summon, SPELL_ELUNE_MOONLANCE_AURA, true);
                    Position pos;
                    summon->GetPosition(&pos);
                    summon->GetNearPosition(pos, 40.0f, _angle);
                    summon->GetMotionMaster()->MovePoint(0, pos.m_positionX, pos.m_positionY, pos.m_positionZ);
                    summon->DespawnOrUnsummon(10000);
                    break;
                }
                case NPC_GODDESS_EYES_A:
                {
                    summon->CastSpell(summon, SPELL_PIERCING_GAZE_OF_ELUNE, true);
                    _goddessAngle = (float)rand_norm()*static_cast<float>(2*M_PI);
                    Position pos;
                    me->GetPosition(&pos);
                    me->GetNearPosition(pos, 20.0f, _goddessAngle);
                    summon->GetMotionMaster()->MovePoint(POINT_TURN, pos.m_positionX, pos.m_positionY, pos.m_positionZ);
                    break;
                }
                case NPC_GODDESS_EYES_B:
                {
                    summon->CastSpell(summon, SPELL_PIERCING_GAZE_OF_ELUNE, true);
                    _goddessAngle += M_PI;
                    _goddessAngle = summon->NormalizeOrientation(_goddessAngle);
                    Position pos;
                    me->GetPosition(&pos);
                    me->GetNearPosition(pos, 20.0f, _goddessAngle);
                    summon->GetMotionMaster()->MovePoint(POINT_TURN, pos.m_positionX, pos.m_positionY, pos.m_positionZ);
                    break;
                }
                default:
                    break;
            }
        }

        void ManageMoonLightScale()
        {
            for (std::list<Creature *>::iterator itr = _gountletNpcs.begin(); itr != _gountletNpcs.end(); )
            {
                Creature *trash = *itr;
                itr++;
                if (!trash || !trash->isAlive() || !trash->HasAura(SPELL_IN_SHADOW))
                {
                    _gountletNpcs.remove(trash);
                    _currentTrashCount++;
                    if (Creature *moonlight = Unit::GetCreature(*me, _currenMoonLightGUID))
                        moonlight->SetObjectScale(moonlight->GetFloatValue(OBJECT_FIELD_SCALE_X) - 0.03);
                }
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim() && !isInIntroPhase)
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ACTIVATE_POOL:
                    {
                        if (Creature *moonlight = me->SummonCreature(NPC_POOL_OF_MOONLIGHT, PoolPositions[_poolCount]))
                        {
                            _gountletNpcs.sort(Trinity::ObjectDistanceOrderPred(moonlight));
                            moonlight->AI()->Talk(_poolCount);
                        }
                        _poolCount++;
                        break;
                    }
                    case EVENT_MOONLIGHT_NPCS_CHECK:
                    {
                        ManageMoonLightScale();
                        if (_gountletNpcs.empty())
                        {
                            instance->SetData(DATA_TYRANDE_GAUNTLET, DONE);
                            DoAction(ACTION_START_COMBAT);
                            return;
                        }
                        if (_currentTrashCount >= _npcsSize / 5.0f)
                        {
                            _currentTrashCount -= _npcsSize / 5.0f;
                            if (Creature *moonlight = Unit::GetCreature(*me, _currenMoonLightGUID))
                            {
                                moonlight->AI()->Talk(TALK_MOONLIGHT_DESPAWN);
                                moonlight->DespawnOrUnsummon();
                            }
                            if (_poolCount < 5)
                            {
                                events.ScheduleEvent(EVENT_ACTIVATE_POOL, 5000);
                                events.ScheduleEvent(EVENT_MOONLIGHT_NPCS_CHECK, 6000);
                            }
                        }
                        else
                            events.ScheduleEvent(EVENT_MOONLIGHT_NPCS_CHECK, 500);
                        break;
                    }
                    case EVENT_ACTIVATE_NPCS:
                    {
                        uint32 activateTrash = 2;
                        for (std::list<Creature *>::iterator itr = _gountletNpcs.begin(); itr != _gountletNpcs.end(); itr++)
                            if (Creature *trash = *itr)
                                if (!trash->isInCombat())
                                {
                                    trash->SetInCombatWithZone();
                                    activateTrash--;
                                    if (!activateTrash)
                                        break;
                                }
                        events.ScheduleEvent(EVENT_ACTIVATE_NPCS, 10000);
                        break;
                    }
                    case EVENT_LUNAR_BOLT:
                    {
                        uint32 timer = 3000;
                        if (secondTear)
                            timer *= 0.5f;
                        else if (firstTear)
                            timer *= 0.75f;
                        DoCastVictim(SPELL_LUNAR_BOLT);
                        events.ScheduleEvent(EVENT_LUNAR_BOLT, timer);
                        break;
                    }
                    case EVENT_STAR_DUST:
                    {
                        DoCast(SPELL_STAR_DUST);
                        events.ScheduleEvent(EVENT_STAR_DUST, urand(15000, 20000));
                        break;
                    }
                    case EVENT_MOONLANCE:
                    {
                        Talk(TALK_MOONLANCE);
                        if (Unit *target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        {
                            _moonLanceTargetGUID = target->GetGUID();
                            DoCast(target, SPELL_MOONLANCE);
                        }
                        events.ScheduleEvent(EVENT_MOONLANCE, urand(7000, 15000));
                        break;
                    }
                    case EVENT_EYES_GODDESS:
                    {
                        Talk(TALK_EYES_OF_GODDESS);
                        DoCast(SPELL_EYES_OF_THE_GODDESS_SC);
                        events.ScheduleEvent(EVENT_EYES_GODDESS, 24000);
                        break;
                    }
                    case EVENT_TEAR_OF_ELUNE:
                    {
                        Talk(TALK_TEARS);
                        Talk(EMOTE_STARFALL);
                        DoCast(SPELL_TEARS_OF_ELUNE);
                        break;
                    }
                    case EVENT_LUNAR_GUIDANCE:
                    {
                        Talk(TALK_GUIDANCE);
                        DoCast(SPELL_LUNAR_GUIDANCE);
                        break;
                    }
                    default:
                        break;
                }
            }
        }

    private:
        uint64 _moonLanceTargetGUID, _currenMoonLightGUID;
        float _angle, _goddessAngle;
        bool tearofElune, firstTear, secondTear, isInIntroPhase, achievementFailed;
        std::list<Creature *> _gountletNpcs;
        uint32 _poolCount, _npcsSize, _currentTrashCount;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_echo_of_tyrandeAI(creature);
    }
};

class npc_tyrande_goddess_eyes : public CreatureScript
{
public:
    npc_tyrande_goddess_eyes() : CreatureScript("npc_tyrande_goddess_eyes") {}

    struct npc_tyrande_goddess_eyesAI : public ScriptedAI
    {
        npc_tyrande_goddess_eyesAI(Creature * creature) : ScriptedAI(creature), instance(creature->GetInstanceScript())
        {
        }

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE && id == POINT_TURN)
                events.ScheduleEvent(EVENT_MOVE_TURN, 0);
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_MOVE_TURN:
                    {
                        if (Creature *tyrande = Unit::GetCreature(*me, instance->GetData64(DATA_ECHO_OF_TYRANDE_GUID)))
                        {
                            Position pos;
                            tyrande->GetPosition(&pos);
                            float dist = me->GetExactDist2d(tyrande->GetPositionX(), tyrande->GetPositionY());
                            me->GetMotionMaster()->MoveAroundPoint(pos, dist, true, 254);
                        }
                        break;
                    }
                }
            }
        }

    private:
        InstanceScript *instance;
        EventMap events;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_tyrande_goddess_eyesAI(creature);
    }
};

class npc_tyrande_trashes : public CreatureScript
{
public:
    npc_tyrande_trashes() : CreatureScript("npc_tyrande_trashes") {}

    struct npc_tyrande_trashesAI : public ScriptedAI
    {
        npc_tyrande_trashesAI(Creature * creature) : ScriptedAI(creature), instance(creature->GetInstanceScript())
        {
        }

        void Reset()
        {
            if (instance->GetData(DATA_TYRANDE_GAUNTLET) == DONE)
                me->DespawnOrUnsummon(1000);
        }


        void EnterCombat(Unit* who)
        {
            if (Creature *tyrande = Unit::GetCreature(*me, instance->GetData64(DATA_ECHO_OF_TYRANDE_GUID)))
                tyrande->AI()->DoAction(ACTION_START_INTRO_EVENT);
        }

    private:
        InstanceScript *instance;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_tyrande_trashesAI(creature);
    }
};

class npc_tyrande_moonlance : public CreatureScript
{
public:
    npc_tyrande_moonlance() : CreatureScript("npc_tyrande_moonlance") {}

    struct npc_tyrande_moonlanceAI : public ScriptedAI
    {
        npc_tyrande_moonlanceAI(Creature * creature) : ScriptedAI(creature), instance(creature->GetInstanceScript())
        {
        }

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void IsSummonedBy(Unit *)
        {
            if (Creature *tyrande = Unit::GetCreature(*me, instance->GetData64(DATA_ECHO_OF_TYRANDE_GUID)))
                tyrande->AI()->JustSummoned(me);
        }

        void UpdateAI(const uint32 diff)
        {
        }

    private:
        InstanceScript *instance;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_tyrande_moonlanceAI(creature);
    }
};

// 102608
class spell_tyrande_goddess_eyes : public SpellScriptLoader
{
public:
    spell_tyrande_goddess_eyes() : SpellScriptLoader("spell_tyrande_goddess_eyes") { }

    class spell_tyrande_goddess_eyes_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_tyrande_goddess_eyes_SpellScript);

        void HandleScriptEffect(SpellEffIndex effIndex)
        {
            if (Unit *caster = GetCaster())
            {
                uint32 spellId = GetSpellInfo()->Effects[effIndex].CalcValue();
                caster->CastSpell(caster, spellId, false);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_tyrande_goddess_eyes_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_tyrande_goddess_eyes_SpellScript();
    }
};

// 102242
class spell_tyrande_tears_of_elune : public SpellScriptLoader
{
public:
    spell_tyrande_tears_of_elune() : SpellScriptLoader("spell_tyrande_tears_of_elune") { }

    class spell_tyrande_tears_of_elune_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_tyrande_tears_of_elune_SpellScript);

        void HandleScriptEffect(SpellEffIndex effIndex)
        {
            if (Unit *caster = GetCaster())
                if (caster->GetTypeId() == TYPEID_UNIT)
                {
                    uint32 spellId = GetSpellInfo()->Effects[effIndex].CalcValue();
                    caster->ToCreature()->AI()->DoCastRandom(spellId, 0.0f, true);
                }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_tyrande_tears_of_elune_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_tyrande_tears_of_elune_SpellScript();
    }
};

class spell_tyrande_achievement_tracker : public SpellScriptLoader
{
public:
    spell_tyrande_achievement_tracker() : SpellScriptLoader("spell_tyrande_achievement_tracker") { }

    class spell_tyrande_achievement_tracker_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_tyrande_achievement_tracker_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            return GetTarget()->GetTypeId() == TYPEID_PLAYER && GetTarget()->ToPlayer()->HasHealSpec();
        }

        void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            if (InstanceScript* instance = GetTarget()->GetInstanceScript())
                if (Creature* tyrande = ObjectAccessor::GetCreature(*GetTarget(), instance->GetData64(DATA_ECHO_OF_TYRANDE_GUID)))
                    tyrande->AI()->DoAction(ACTION_ACHIEVMENT_FAIL);
        }


        void Register()
        {
            DoCheckProc += AuraCheckProcFn(spell_tyrande_achievement_tracker_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_tyrande_achievement_tracker_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_tyrande_achievement_tracker_AuraScript();
    }
};

void AddSC_boss_echo_of_tyrande()
{
    new boss_echo_of_tyrande();
    new npc_tyrande_goddess_eyes();
    new npc_tyrande_moonlance();
    new npc_tyrande_trashes();
    new spell_tyrande_goddess_eyes();
    new spell_tyrande_tears_of_elune();
    new spell_tyrande_achievement_tracker();
}
