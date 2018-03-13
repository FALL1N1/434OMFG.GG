#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Cell.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "the_vortex_pinnacle.h"

enum spells
{
    SPELL_CYCLONE_SHIELD            = 86292,
    SPELL_STORMS_EDGE               = 86295,
    SPELL_STORMS_EDGE_KNOCKBACK     = 86310,
    SPELL_STORMS_EDGE_DAMAGE        = 86309,
    SPELL_LIGHTNING_BOLT            = 86331,
    SPELL_SUMMON_TEMPEST            = 86340,
    SPELL_VORTEX_VISUAL             = 86267,
    SPELL_OREE_TEMPEST              = 86299
};

enum creatures
{
    NPC_CYCLONE                     = 46007,
    NPC_VORTEX_ROTATION_FOCUS       = 438780
};

enum eventIds
{
    EVENT_LIGHTNING_BOLT            = 1,
    EVENT_SUMMON_TEMPEST            = 2,
    EVENT_CYCLONE_SHIELD            = 3
};

enum talks
{
    SAY_AGGRO                       = 1,
    SAY_DEATH                       = 2,
    SAY_KILL                        = 3
};

enum actions
{
    ACTION_CYCLONE_SHIELD           = 1,
    ACTION_CYCLONE_SHIELD_START,
    ACTION_CYCLONE_SHIELD_END,
    ACTION_INVOC_VORTEX,
    ACTION_DESPAWN_VORTEX,
    ACTION_MARAUDE_START_ATTACK,
    ACTION_MARAUDE_STOP_ATTACK,
    ACTION_MARAUDE_EVADE_COMBAT,
};

enum divers
{
    NB_CYCLONES                     = 8
};

class boss_grand_vizier_ertan : public CreatureScript
{
    public:
        boss_grand_vizier_ertan() : CreatureScript("boss_grand_vizier_ertan")
        {
        }

        struct boss_grand_vizier_ertanAI : public BossAI
        {
            boss_grand_vizier_ertanAI(Creature* creature) : BossAI(creature, BOSS_GRAND_VIZIER_ERTAN)
            {
            }

            void Reset()
            {
                _Reset();
                events.ScheduleEvent(EVENT_LIGHTNING_BOLT, 2000);
                events.ScheduleEvent(EVENT_CYCLONE_SHIELD, 12000);
                if (IsHeroic())
                    events.ScheduleEvent(EVENT_SUMMON_TEMPEST, 10000);
                cycloneShield = false;
                me->RemoveAurasDueToSpell(SPELL_STORMS_EDGE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                vortexFocusGUID = 0;
                summons.DespawnAll();
            }

            void JustReachedHome()
            {
                if (Creature *c = Unit::GetCreature(*me, vortexFocusGUID))
                    c->AI()->DoAction(ACTION_DESPAWN_VORTEX);
                if (instance)
                    instance->SetBossState(BOSS_GRAND_VIZIER_ERTAN, NOT_STARTED);
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                me->CastSpell(me, SPELL_STORMS_EDGE, true);
                Talk(SAY_AGGRO);
                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_SET_COMBAT_RES_LIMIT, me);
                Position pos;
                me->GetPosition(&pos);
                if (Creature *c = me->SummonCreature(NPC_VORTEX_ROTATION_FOCUS, pos, TEMPSUMMON_MANUAL_DESPAWN))
                    vortexFocusGUID = c->GetGUID();
            }

            void KilledUnit(Unit* who)
            {
                if (who && who->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_KILL);
            }

            void JustDied(Unit* /*who*/)
            {
                Talk(SAY_DEATH);
                if (instance)
                    instance->SetData(DATA_GRAND_VIZIER_ERTAN, DONE);
                if (Creature *c = Unit::GetCreature(*me, vortexFocusGUID))
                    c->AI()->DoAction(ACTION_DESPAWN_VORTEX);
                summons.DespawnAll();
                _JustDied();
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case ACTION_CYCLONE_SHIELD:
                        cycloneShield = false;
                        if (Creature *c = Unit::GetCreature(*me, vortexFocusGUID))
                            c->AI()->DoAction(ACTION_CYCLONE_SHIELD_END);
                        break;
                    default:
                        break;
                }
            }

            void CycloneShield()
            {
                me->InterruptNonMeleeSpells(false);
                me->CastSpell(me, SPELL_STORMS_EDGE_KNOCKBACK, true);
            }

            void JustSummoned(Creature* summoned)
            {
                switch (summoned->GetEntry())
                {
                    case NPC_VORTEX_ROTATION_FOCUS:
                        summoned->AI()->DoAction(ACTION_INVOC_VORTEX);
                        break;
                    default:
                        break;
                }
                summons.Summon(summoned);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (cycloneShield)
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_LIGHTNING_BOLT:
                            DoCastVictim(SPELL_LIGHTNING_BOLT);
                            events.ScheduleEvent(EVENT_LIGHTNING_BOLT, 2000);
                            break;
                        case EVENT_SUMMON_TEMPEST:
                            me->CastSpell(me, SPELL_SUMMON_TEMPEST, true);
                            events.ScheduleEvent(EVENT_SUMMON_TEMPEST, urand(30000, 40000));
                            break;
                        case EVENT_CYCLONE_SHIELD:
                            cycloneShield = true;
                            CycloneShield();
                            if (Creature *c = Unit::GetCreature(*me, vortexFocusGUID))
                                c->AI()->DoAction(ACTION_CYCLONE_SHIELD_START);
                            events.ScheduleEvent(EVENT_CYCLONE_SHIELD, 30000);
                            events.ScheduleEvent(EVENT_LIGHTNING_BOLT, 2000);
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private :
            bool cycloneShield;
            uint64 vortexFocusGUID;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetVortexPinnacleAI<boss_grand_vizier_ertanAI>(creature);
        }
};

class mob_vortex_rotation_focus : public CreatureScript
{
public:
    mob_vortex_rotation_focus() : CreatureScript("mob_vortex_rotation_focus") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_vortex_rotation_focusAI(pCreature);
    }

    struct mob_vortex_rotation_focusAI : public ScriptedAI
    {
        mob_vortex_rotation_focusAI(Creature* pCreature) : ScriptedAI(pCreature),  Summons(me)
        {
            instance = pCreature->GetInstanceScript();
            Reset();
        }

        void Reset()
        {
            m_timer = 0;
            radius = 24;
            m_rotate_timer = 0;
            own_vortex.clear();
            rotate = true;
            _baseSpeedRun = 2.0f;
            _baseSpeedWalk = 1.0f;
        }

        void JustDied(Unit* /*who*/)
        {
            Summons.DespawnAll();
        }

        void DamageTaken(Unit* pDone_by, uint32& uiDamage)
        {
            uiDamage = 0;
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_INVOC_VORTEX:
                {
                    float orient = me->GetOrientation();
                    for (int i = 0; i < NB_CYCLONES; i++)
                    {
                        orient += 2 * M_PI / 8;
                        float x, y;
                        me->GetNearPoint2D(x, y, radius, orient);
                        if (Creature *c = me->SummonCreature(NPC_CYCLONE, x, y, me->GetPositionZ(), 0.0f, TEMPSUMMON_MANUAL_DESPAWN))
                        {
                            c->SetReactState(REACT_PASSIVE);
                            _baseSpeedRun = c->GetSpeedRate(MOVE_RUN);
                            _baseSpeedWalk = c->GetSpeedRate(MOVE_RUN);
                            c->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        }
                    }
                    break;
                }
                case ACTION_CYCLONE_SHIELD_START:
                {
                    radius = me->GetObjectSize() + 2.0f;
                    rotate = false;
                    float orient = me->GetOrientation();
                    me->GetMotionMaster()->Clear();
                    for (std::list<uint64 >::iterator itr = own_vortex.begin(); itr != own_vortex.end(); itr++)
                    {
                        if (Creature *vortex = Unit::GetCreature(*me, (*itr)))
                        {
                            vortex->SetSpeed(MOVE_WALK, 1.0f);
                            vortex->SetSpeed(MOVE_RUN, 1.0f);
                            float x,y;
                            orient +=  2 * M_PI / 8;
                            me->GetNearPoint2D(x, y, radius, orient);
                            vortex->GetMotionMaster()->Clear();
                            vortex->GetMotionMaster()->MovePoint(0, x, y,  me->GetPositionZ());
                        }
                    }
                    break;
                }
                case ACTION_CYCLONE_SHIELD_END:
                {
                    radius = 24;
                    float orient = me->GetOrientation();
                    for (std::list<uint64 >::iterator itr = own_vortex.begin(); itr != own_vortex.end(); itr++)
                    {
                        if (Creature *vortex = Unit::GetCreature(*me, (*itr)))
                        {
                            vortex->SetSpeed(MOVE_WALK, 1.0f);
                            vortex->SetSpeed(MOVE_RUN, 1.0f);
                            float x,y;
                            orient +=  2 * M_PI / 8;
                            me->GetNearPoint2D(x, y, radius, orient);
                            vortex->GetMotionMaster()->Clear();
                            vortex->GetMotionMaster()->MovePoint(0, x, y,  me->GetPositionZ());
                        }
                    }
                    m_rotate_timer = 3000;
                    m_timer = 3000;
                    rotate = true;
                    break;
                }
                case ACTION_DESPAWN_VORTEX:
                {
                    Summons.DespawnAll();
                    break;
                }
                default:
                    break;
            }
        }

        void AttackStart(Unit *who)
        {
            return;
        }

        void JustSummoned(Creature* summoned)
        {
            own_vortex.push_back(summoned->GetGUID());
            Summons.Summon(summoned);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (rotate)
            {
                if (m_rotate_timer <= uiDiff)
                {
                    m_rotate_timer = 60000;
                    me->GetMotionMaster()->MoveRotate(m_rotate_timer, ROTATE_DIRECTION_RIGHT);
                }
                else
                    m_rotate_timer -= uiDiff;
            }

            if (m_timer <= uiDiff)
            {
                float orient = me->GetOrientation();
                for (std::list<uint64 >::iterator itr = own_vortex.begin(); itr != own_vortex.end(); itr++)
                {
                    if (Creature *vortex = Unit::GetCreature(*me, (*itr)))
                    {
                        float x,y;
                        orient +=  2 * M_PI / 8;
                        me->GetNearPoint2D(x, y, radius, orient);
                        if (rotate)
                        {
                            vortex->SetSpeed(MOVE_WALK, _baseSpeedWalk);
                            vortex->SetSpeed(MOVE_RUN, _baseSpeedRun);
                        }
                        vortex->GetMotionMaster()->Clear();
                        if (vortex->GetDistance2d(x, y) < 3.0f || !rotate)
                            vortex->GetMotionMaster()->MovePoint(0, x, y,  me->GetPositionZ());
                        else
                            vortex->NearTeleportTo(x, y,  me->GetPositionZ(), 0.0f);
                    }
                }
                m_timer = 200;
            }
            else m_timer -= uiDiff;

        }

    private :
        InstanceScript* instance;
        uint32 m_timer;
        uint32 m_rotate_timer;
        float m_direction, m_nextdirection;
        std::list<uint64 > own_vortex;
        uint32 radius;
        SummonList Summons;
        bool rotate;
        float _baseSpeedRun;
        float _baseSpeedWalk;
    };
};


class mob_tempest_maraude : public CreatureScript
{
public:
    mob_tempest_maraude() : CreatureScript("mob_tempest_maraude") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_tempest_maraudeAI(pCreature);
    }

    struct mob_tempest_maraudeAI : public ScriptedAI
    {
        mob_tempest_maraudeAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
        }

        void Reset()
        {
            isActive = true;
            m_timer = 0;
            events.Reset();
            events.ScheduleEvent(EVENT_LIGHTNING_BOLT, 2000);
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_MARAUDE_START_ATTACK:
                    isActive = true;
                    me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->RemoveAura(85267);
                    me->CastSpell(me, 85467);
                    me->CastSpell(me, 85294);
                    if (me->FindNearestCreature(NPC_GRAND_VIZIER_ERTAN, 30.0f) != NULL) //only during ertan fight
                        me->SetInCombatWithZone();
                    break;
                case ACTION_MARAUDE_STOP_ATTACK:
                    me->DeleteThreatList();
                    me->RemoveAura(85467);
                    me->CastSpell(me, 85267);
                    me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                    me->SetReactState(REACT_PASSIVE);
                    me->GetMotionMaster()->Clear();
                    me->StopMoving();
                    isActive = false;
                    break;
                case ACTION_MARAUDE_EVADE_COMBAT:
                    me->DeleteThreatList();
                    isActive = true;
                    // me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                    me->SetReactState(REACT_AGGRESSIVE);
                    if (me->HasAura(85267))
                    {
                        me->RemoveAura(85267);
                        me->CastSpell(me, 85467);
                        me->CastSpell(me, 85294);
                    }
                    if (me->FindNearestCreature(NPC_GRAND_VIZIER_ERTAN, 30.0f) != NULL) //only during ertan fight
                        me->SetInCombatWithZone();
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim() || !isActive)
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_LIGHTNING_BOLT:
                    DoCastVictim(SPELL_LIGHTNING_BOLT);
                    events.ScheduleEvent(EVENT_LIGHTNING_BOLT, 2000);
                    break;
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }

        bool isActive;
    private :
        InstanceScript* instance;
        uint32 m_timer;
        EventMap events;
    };
};

class spell_ertan_storms_edge : public SpellScriptLoader
{
public:
    spell_ertan_storms_edge() : SpellScriptLoader("spell_ertan_storms_edge")
    {
    }

    class StormsEdgeCheck
    {
        Unit const* _caster;

    public:
        StormsEdgeCheck(Unit* caster) : _caster(caster)
        {
        }

        bool operator() (WorldObject* target)
        {
            if (target->IsInRange(_caster, 0.0f, 4.4f, true))
                return true;

            return false;
        }
    };

    class spell_ertan_storms_edge_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ertan_storms_edge_SpellScript);

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            if (Unit* caster = GetCaster())
                unitList.remove_if(StormsEdgeCheck(caster));
        }

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (Unit *target = GetHitUnit())
            {
                if (target->GetTypeId() == TYPEID_PLAYER || target->isPet() || target->isTotem())
                    if (Creature *c =  GetHitUnit()->FindNearestCreature(NPC_CYCLONE, 100, true))
                        c->CastSpell(GetHitUnit(), SPELL_STORMS_EDGE_DAMAGE, true);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ertan_storms_edge_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(spell_ertan_storms_edge_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_ertan_storms_edge_SpellScript();
    }
};

class spell_ertan_storms_edge_triggered : public SpellScriptLoader
{
public:
    spell_ertan_storms_edge_triggered() : SpellScriptLoader("spell_ertan_storms_edge_triggered")
    {
    }

    class spell_ertan_storms_edge_triggered_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ertan_storms_edge_triggered_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            GetCaster()->CastSpell(GetHitUnit(), uint32(GetEffectValue()), true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_ertan_storms_edge_triggered_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_ertan_storms_edge_triggered_SpellScript();
    }
};

class spell_ertan_storms_edge_knockback : public SpellScriptLoader
{
public:
    spell_ertan_storms_edge_knockback() : SpellScriptLoader("spell_ertan_storms_edge_knockback")
    {
    }

    class spell_ertan_storms_edge_knockback_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_ertan_storms_edge_knockback_AuraScript);

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Creature* Ertan = GetCaster()->ToCreature())
                Ertan->AI()->DoAction(ACTION_CYCLONE_SHIELD);
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_ertan_storms_edge_knockback_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_ertan_storms_edge_knockback_AuraScript();
    }
};

class DontLooksDirectlyInTempestCheck : public std::unary_function<WorldObject*, bool>
{
public:
    DontLooksDirectlyInTempestCheck(Unit* caster) : __caster(caster) {}

    bool operator() (WorldObject* target)
    {
        Position pos;
        __caster->GetPosition(&pos);
        return !target->HasInArc(static_cast<float>(M_PI), &pos);
    }

private:
    Unit* __caster;
};

//85294
class spell_lurk_search : public SpellScriptLoader
{
public:
    spell_lurk_search() : SpellScriptLoader("spell_lurk_search") { }

    class spell_lurk_search_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_lurk_search_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) { return true; }

        void HandleEffectPeriodic(AuraEffect const* aurEff)
        {
            if (GetCaster())
                if (GetCaster()->IsAIEnabled)
                {
                    bool canAttack = true;
                    bool checkDist = false;
                    bool checkInArc = false;
                    Position pos;

                    GetCaster()->GetPosition(&pos);
                    Map::PlayerList const& players = GetCaster()->GetMap()->GetPlayers();
                    if (!players.isEmpty())
                        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                            if (Player* player = itr->getSource())
                                if (player->GetDistance(GetCaster()) < 40.0f)
                                {
                                    checkDist = true;
                                    Position pos2;
                                    player->GetPosition(&pos2);
                                    if (player->HasInArc(static_cast<float>(M_PI), &pos))
                                        canAttack = false;
                                    if (GetCaster()->HasInArc(static_cast<float>(M_PI), &pos2))
                                        checkInArc = true;
                                    if (checkInArc && !canAttack && checkDist)
                                        break;
                                }
                    if ((!checkDist || !checkInArc) && GetCaster()->isInCombat())
                        GetCaster()->GetAI()->DoAction(ACTION_MARAUDE_EVADE_COMBAT);
                    else if (!CAST_AI(mob_tempest_maraude::mob_tempest_maraudeAI, (GetCaster()->GetAI()))->isActive && canAttack)
                        GetCaster()->GetAI()->DoAction(ACTION_MARAUDE_START_ATTACK);
                    else if (CAST_AI(mob_tempest_maraude::mob_tempest_maraudeAI, (GetCaster()->GetAI()))->isActive && !canAttack)
                        GetCaster()->GetAI()->DoAction(ACTION_MARAUDE_STOP_ATTACK);
                }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_lurk_search_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_lurk_search_AuraScript();
    }
};

void AddSC_boss_grand_vizier_ertan()
{
    new boss_grand_vizier_ertan();
    new mob_vortex_rotation_focus();
    new mob_tempest_maraude();
    new spell_ertan_storms_edge();
    new spell_ertan_storms_edge_triggered();
    new spell_ertan_storms_edge_knockback();
    new spell_lurk_search();
};
