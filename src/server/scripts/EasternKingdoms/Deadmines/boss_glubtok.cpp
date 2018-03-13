
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "deadmines.h"
#include "MoveSplineInit.h"
#include "VMapFactory.h"


enum Spells
{
    SPELL_FIRE_BLOSSOM          = 88129,
    SPELL_FIRE_BLOSSOM_VISUAL   = 88164,
    SPELL_FIRE_BLOSSOM_SUMMON   = 91301,
    SPELL_FROST_BLOSSOM         = 88169,
    SPELL_FROST_BLOSSOM_VISUAL  = 88165,
    SPELL_FROST_BLOSSOM_SUMMON  = 91302,
    SPELL_ARCANE_POWER          = 88009,
    SPELL_FIST_OF_FLAME         = 87859,
    SPELL_FIST_OF_FROST         = 87861,
    SPELL_BLINK                 = 87925,
    SPELL_MASS_TELEPORT         = 87999,
	SPELL_TELEPORT_VISUAL       = 88002,
    SPELL_DEEP_FREEZE           = 88000,

    SPELL_ARCANE_FIRE_RAY       = 88072, // 47242
    SPELL_ARCANE_FROST_RAY      = 88093, // 472421

    SPELL_FIREWALL_DAMAGE       = 91398,


    SPELL_TRANSITION_INVIS = 90424,
    SPELL_ARCANE_OVERCHARGE = 88183,
    SPELL_ARCANE_OVERCHARGE_2 = 90520,
    SPELL_ARCANE_DEAD = 88185,
};

enum Texts
{
    SAY_AGGRO           = 0,
    SAY_FISTS_OF_FLAME,
    SAY_FISTS_OF_FROST,
    SAY_READY,
    SAY_LETS_DO_IT,
    SAY_ARCANE_POWER,
    EMOTE_FLAME_WALL,
    SAY_TOO_MUCH_POWER,
    SAY_KILL,
};

enum npcs
{
    NPC_FIRE_BUNNY = 47242,
    NPC_FROST_BUNNY = 472421,
    NPC_FIRE_ELEMENTAL = 48957,
    NPC_FROST_ELEMENTAL = 48958,
};

//
//#define SAY_AGGRO               "Glubtok show you da power of de Arcane."
//#define SOUND_AGGRO             21151
//
//#define SAY_KILL                "Ha..Ha..Ha..Ha..Ah!"
//#define SOUND_KILL              21152
//
//#define SAY_FISTS_OF_FLAME       "Fists of Flame!"
//#define SOUND_FISTS_OF_FLAME     21153
//
//#define SAY_FISTS_OF_FROST      "Fists of Frost!"
//#define SOUND_FISTS_OF_FROST    21156
//
//#define SAY_READY               "Glubtok ready?"
//#define SOUND_READY             21154
////
//#define SAY_LETS_DO_IT          "Let's do it!"
//#define SOUND_LETS_DO_IT        21157
//
//#define SAY_ARCANE_POWER        "ARCANE POWER"
//#define SOUND_ARCANE_POWER      21146
//
//#define SAY_TOO_MUCH_POWER      "TOO... MUCH... POWER"
//#define SOUND_TOO_MUCH_POWER    21145
//#define SAY_FLAME               "Elemental Fists!"

enum Events
{
    EVENT_FISTS_OF_FLAME   = 1, // phase 1
    EVENT_FISTS_OF_FROST   = 2, // phase 1
    EVENT_BLINK            = 3, // phase 1
    EVENT_FROST_BLOSSOM    = 4, // phase 2
    EVENT_FIRE_BLOSSOM     = 5, // phase 2 hc only?
    EVENT_ARCANE_POWER     = 6,
    EVENT_SUMMON_FIRE_WALL = 7,

    EVENT_OUTRO,
    EVENT_OUTRO1,
};

enum misc
{
    PHASE_ONE           = 1,
    PHASE_50_PERCENT    = 2,
    PHASE_OUTRO         = 3,
    POINT_CENTER        = 4
};

enum actions
{
    ACTION_BLINK = 42,
};

class RotateEvent : public BasicEvent
{
public:
    RotateEvent(Creature* owner) : _owner(owner) { }

    bool Execute(uint64 execTime, uint32 /*diff*/) override
    {
        _owner->GetMotionMaster()->MoveRotate(20000, ROTATE_DIRECTION_RIGHT);
        _owner->m_Events.AddEvent(this, execTime + 20000);
        return false;
    }
private :
    Creature* _owner;
};

class boss_glubtok : public CreatureScript
{
public:
    boss_glubtok() : CreatureScript("boss_glubtok") { }

    struct boss_glubtokAI : public BossAI
    {
        boss_glubtokAI(Creature* creature) : BossAI(creature, BOSS_GLUBTOK_DATA) { }

        void Reset()
        {
            _Reset();
            instance->SetData(DATA_FIREWALL, 1);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            me->SetHover(false);
            die = false;
            cnt = 0;
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            Talk(SAY_AGGRO);
            events.SetPhase(PHASE_ONE);
            events.ScheduleEvent(urand(EVENT_FISTS_OF_FLAME, EVENT_FISTS_OF_FROST), 3000, 0, PHASE_ONE);
            events.ScheduleEvent(EVENT_BLINK, urand(5000, 10000), 0, PHASE_ONE);
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            // open door
            if (GameObject* door = me->FindNearestGameObject(17153, 20))
            {
                door->SetLootState(GO_READY);
                door->UseDoorOrButton(10000, false);
            }
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_KILL);
        }

        void DoAction(int32 const act)
        {
            if (act == ACTION_BLINK)
            {
                if (events.IsInPhase(PHASE_50_PERCENT))
                    return;
                DoCast(SPELL_BLINK);
                if (IsHeroic())
                    DoResetThreat();
            }
        }

        void EnterEvadeMode()
        {
            if (events.IsInPhase(PHASE_50_PERCENT))
                me->SetControlled(false, UNIT_STATE_ROOT);
            me->RemoveAllAuras();
            me->CombatStop();
            BossAI::EnterEvadeMode();
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage)
        {
            if (die)
            {
                damage = 0;
                return;
            }
            if (HealthBelowPct(50) && events.IsInPhase(PHASE_ONE))
            {
                events.Reset();
                events.SetPhase(PHASE_50_PERCENT);
                me->RemoveAllAuras();
                Position pos = me->GetHomePosition();
                me->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), true);
				DoCast(SPELL_TELEPORT_VISUAL);
                events.ScheduleEvent(EVENT_ARCANE_POWER, 500, 0, PHASE_50_PERCENT);
            }

            if (damage > me->GetHealth())
            {
                die = true;
                damage = 0;
                events.Reset();
                events.SetPhase(PHASE_OUTRO);
                me->SetHover(false);
                me->CastSpell(me, SPELL_ARCANE_OVERCHARGE, true);
                instance->SetBossState(BOSS_GLUBTOK_DATA, DONE);
                events.ScheduleEvent(EVENT_OUTRO, 1000);
            }
        }

        void JustSummoned(Creature* summon)
        {
            switch (summon->GetEntry())
            {
                case NPC_FIRE_BLOSSOM_BUNNY:
                    summon->CastSpell(summon, SPELL_FIRE_BLOSSOM_VISUAL, true);
                    me->CastSpell(summon, SPELL_FIRE_BLOSSOM, false);
                    break;
                case NPC_FROST_BLOSSOM_BUNNY:
                    summon->CastSpell(summon, SPELL_FROST_BLOSSOM_VISUAL, true);
                    me->CastSpell(summon, SPELL_FROST_BLOSSOM, false);
                    break;
                case 49042:
                case 49040:
                case 48975:
                case 48976:
                case 49039:
                case 49041:
                {
                    if (Creature* vehicle = me->FindNearestCreature(48974, 100.0f, true))
                    {
                        cnt++;
                        summon->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, cnt, vehicle, false);
                    }
                    summon->AddAura(SPELL_FIREWALL_DAMAGE, summon);
                    break;
                }
                case 48974:
                {
                    summon->m_Events.AddEvent(new RotateEvent(summon), summon->m_Events.CalculateTime(5000));
                    summon->SetDisableGravity(true);
                    Position pos = me->GetHomePosition();
                    summon->NearTeleportTo(summon->GetPositionX(), summon->GetPositionY(), summon->GetPositionZ() + 3.5f, summon->GetOrientation(), false);
                    me->SummonCreature(49042, pos, TEMPSUMMON_MANUAL_DESPAWN);
                    me->SummonCreature(49040, pos, TEMPSUMMON_MANUAL_DESPAWN);
                    me->SummonCreature(48975, pos, TEMPSUMMON_MANUAL_DESPAWN);
                    me->SummonCreature(48976, pos, TEMPSUMMON_MANUAL_DESPAWN);
                    me->SummonCreature(49039, pos, TEMPSUMMON_MANUAL_DESPAWN);
                    me->SummonCreature(49041, pos, TEMPSUMMON_MANUAL_DESPAWN);
                    me->SummonCreature(49042, pos, TEMPSUMMON_MANUAL_DESPAWN);
                    me->SummonCreature(49040, pos, TEMPSUMMON_MANUAL_DESPAWN);
                    break;
                }
                case NPC_FROST_ELEMENTAL:
                case NPC_FIRE_ELEMENTAL:
                    summon->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_DISABLE_MOVE);
                    summon->SetReactState(REACT_AGGRESSIVE);
                    summon->SetInCombatWithZone();
                    break;
                default:
                    break;
            }
            BossAI::JustSummoned(summon);
        }

        void UpdateAI(uint32 const diff)
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
                    case EVENT_OUTRO:
                        me->CastSpell(me, SPELL_TRANSITION_INVIS, true);
                        me->CastSpell(me, SPELL_ARCANE_OVERCHARGE_2, true);
                        events.ScheduleEvent(EVENT_OUTRO1, 2300);
                        break;
                    case EVENT_OUTRO1:
                        die = false;
                        me->CastSpell(me, SPELL_ARCANE_DEAD, true);
                        break;
                    case EVENT_FISTS_OF_FLAME:
                        Talk(SAY_FISTS_OF_FLAME);
                        DoCast(me, SPELL_FIST_OF_FLAME, false);
                        events.ScheduleEvent(EVENT_FISTS_OF_FROST, 12100, 0, PHASE_ONE);
                        break;
                    case EVENT_FISTS_OF_FROST:
                        Talk(SAY_FISTS_OF_FROST);
                        DoCast(me, SPELL_FIST_OF_FROST, false);
                        events.ScheduleEvent(EVENT_FISTS_OF_FLAME, 12100, 0, PHASE_ONE);
                        break;
                    case EVENT_FROST_BLOSSOM:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                            me->SummonCreature(NPC_FROST_BLOSSOM_BUNNY, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 5000);
                        events.ScheduleEvent(EVENT_FIRE_BLOSSOM, 3000, 0, PHASE_50_PERCENT);
                        break;
                    case EVENT_FIRE_BLOSSOM:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                            me->SummonCreature(NPC_FIRE_BLOSSOM_BUNNY, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 5000);
                        events.ScheduleEvent(EVENT_FROST_BLOSSOM, 3000, 0, PHASE_50_PERCENT);
                        break;
                    case EVENT_ARCANE_POWER:
                    {
                        Talk(SAY_ARCANE_POWER);
                        DoCast(me, SPELL_ARCANE_POWER, true);
                        me->SetHover(true);
                        me->SetControlled(true, UNIT_STATE_ROOT);
						Movement::MoveSplineInit init(me);
						init.MoveTo(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 2.0f, false, true);
						init.SetVelocity(1.5f);
						init.Launch();
						me->SetDisableGravity(true);
                        std::list<Creature*> stalker;
                        me->GetCreatureListWithEntryInGrid(stalker, NPC_FIRE_BUNNY, 200.0f);
                        for (std::list<Creature *>::iterator itr = stalker.begin(); itr != stalker.end(); itr++)
                            if (Creature *c = *itr)
                            {
                                c->GetMotionMaster()->MoveRandom(5);
                                c->CastSpell(me, SPELL_ARCANE_FIRE_RAY, true);
                            }
                        stalker.clear();
                        me->GetCreatureListWithEntryInGrid(stalker, NPC_FROST_BUNNY, 200.0f);
                        for (std::list<Creature *>::iterator itr = stalker.begin(); itr != stalker.end(); itr++)
                            if (Creature *c = *itr)
                            {
                                c->GetMotionMaster()->MoveRandom(5);
                                c->CastSpell(me, SPELL_ARCANE_FROST_RAY, true);
                            }
                        events.ScheduleEvent(urand(EVENT_FROST_BLOSSOM, EVENT_FIRE_BLOSSOM), 10000, 0, PHASE_50_PERCENT);
                        if (IsHeroic())
                            events.ScheduleEvent(EVENT_SUMMON_FIRE_WALL, 6000, 0, PHASE_50_PERCENT);
                        break;
                    }
                    case EVENT_SUMMON_FIRE_WALL:
                        Talk(EMOTE_FLAME_WALL);
                        me->SummonCreature(48974, -193.287f, -441.953f, 53.763f, M_PI / 2.0f, TEMPSUMMON_MANUAL_DESPAWN);
                        break;
                }
            }

            if (!events.IsInPhase(PHASE_50_PERCENT))
                DoMeleeAttackIfReady();
        }

    private :
        int cnt;
        bool die;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_glubtokAI(creature);
    }
};

// 87925
class spell_glubtok_blink : public SpellScriptLoader
{
public:
    spell_glubtok_blink() : SpellScriptLoader("spell_glubtok_blink") { }

    class NonPlayer
    {
    public:
        explicit NonPlayer() {};

        bool operator()(WorldObject* target) const
        {
            return target->GetTypeId() != TYPEID_PLAYER;
        }
    };


    class spell_glubtok_blink_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_glubtok_blink_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(NonPlayer());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_glubtok_blink_SpellScript::FilterTargets, EFFECT_0, TARGET_DEST_TARGET_RANDOM);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_glubtok_blink_SpellScript();
    }
};

// 87859 87861
class spell_glubtok_fist : public SpellScriptLoader
{
public:
    spell_glubtok_fist() : SpellScriptLoader("spell_glubtok_fist") { }

    class spell_glubtok_fist_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_glubtok_fist_AuraScript);

        void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
                if (Creature *glubtok = caster->ToCreature())
                    glubtok->AI()->DoAction(ACTION_BLINK);
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_glubtok_fist_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_glubtok_fist_AuraScript();
    }
};

class spell_blottom_fire : public SpellScriptLoader
{
public:
    spell_blottom_fire() : SpellScriptLoader("spell_blottom_fire") { }

    class spell_blottom_fire_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_blottom_fire_SpellScript);

        void HitTargetBunny(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            if (Unit *caster = GetCaster())
                if (Unit *trigger = caster->FindNearestCreature(NPC_FIRE_BLOSSOM_BUNNY, 200.0f))
                    caster->CastSpell(trigger, 88173, true);
        }

        void HeroicSummon(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            if (Unit *caster = GetCaster())
                if (Unit *trigger = caster->FindNearestCreature(NPC_FIRE_BLOSSOM_BUNNY, 200.0f))
                    caster->CastSpell(trigger, 91301, true);
        }

        void Register()
        {
            OnEffectHit += SpellEffectFn(spell_blottom_fire_SpellScript::HitTargetBunny, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
            if (m_scriptSpellId == 91286)
                OnEffectHit += SpellEffectFn(spell_blottom_fire_SpellScript::HeroicSummon, EFFECT_1, SPELL_EFFECT_TRIGGER_MISSILE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_blottom_fire_SpellScript();
    }
};

class spell_blottom_frost : public SpellScriptLoader
{
public:
    spell_blottom_frost() : SpellScriptLoader("spell_blottom_frost") { }

    class spell_blottom_frost_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_blottom_frost_SpellScript);

        void HitTargetBunny(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            if (Unit *caster = GetCaster())
                if (Unit *trigger = caster->FindNearestCreature(NPC_FROST_BLOSSOM_BUNNY, 200.0f))
                    caster->CastSpell(trigger, 88177, true);
        }

        void HeroicSummon(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            if (Unit *caster = GetCaster())
                if (Unit *trigger = caster->FindNearestCreature(NPC_FROST_BLOSSOM_BUNNY, 200.0f))
                    caster->CastSpell(trigger, 91302, true);
        }

        void Register()
        {
            OnEffectHit += SpellEffectFn(spell_blottom_frost_SpellScript::HitTargetBunny, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
            if (m_scriptSpellId == 91287)
                OnEffectHit += SpellEffectFn(spell_blottom_frost_SpellScript::HeroicSummon, EFFECT_1, SPELL_EFFECT_TRIGGER_MISSILE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_blottom_frost_SpellScript();
    }
};

// 16184
class achievement_ready_for_raid : public AchievementCriteriaScript
{
public:
    achievement_ready_for_raid() : AchievementCriteriaScript("achievement_ready_for_raid") { }

    bool OnCheck(Player* /*source*/, Unit* target)
    {
        if (!target)
            return false;
        if (InstanceScript *instance = target->GetInstanceScript())
            return instance->GetData(DATA_FIREWALL);
        return false;
    }
};

// 91397
class spell_gl_firewall : public SpellScriptLoader
{
public:
    spell_gl_firewall() : SpellScriptLoader("spell_gl_firewall") { }

    class spell_gl_firewall_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_gl_firewall_SpellScript);

        void CalculateDamage(SpellEffIndex /*effIndex*/)
        {
            if (!GetHitUnit())
                return;
            if (Unit *caster = GetCaster())
                if (InstanceScript *instance = caster->GetInstanceScript())
                    instance->SetData(DATA_FIREWALL, 0);
        }

        /*void FilterTargets(std::list<WorldObject*>& unitList)
        {
            Unit *caster = GetCaster();
            if (!caster)
                return;
            Map *map = caster->GetMap();
            if (!map)
                return;
            unitList.clear();
            Map::PlayerList const &PlayerList = map->GetPlayers();
            if (PlayerList.isEmpty())
                return;
            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                if (Player *player = i->getSource())
                    if (player->isAlive())
                        if (caster->GetDistance2d(player->GetPositionX(), player->GetPositionY()) <= 3.0f)
                            unitList.push_back(player);
        }*/

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_gl_firewall_SpellScript::CalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_gl_firewall_SpellScript();
    }
};


class spell_gl_firewall_b : public SpellScriptLoader
{
public:
    spell_gl_firewall_b() : SpellScriptLoader("spell_gl_firewall_b") { }

    class spell_gl_firewall_b_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_gl_firewall_b_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            return true;
        }

        bool Load()
        {
            return true;
        }

        void HandleProc(AuraEffect const* aurEff)
        {
            PreventDefaultAction();
            if (Unit *target = GetCaster())
            {
                Position pos;
                target->GetPosition(&pos);
                float z = target->GetMap()->GetHeight(pos.m_positionX, pos.m_positionY, pos.m_positionZ);
                target->CastSpell(pos.m_positionX, pos.m_positionY, z, 91397, true);
            }
        }


        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_gl_firewall_b_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_gl_firewall_b_AuraScript();
    }
};

void AddSC_boss_glubtok()
{
    new boss_glubtok();
    new spell_glubtok_blink();
    new spell_glubtok_fist();
    new spell_blottom_fire();
    new spell_blottom_frost();

    new achievement_ready_for_raid();
    new spell_gl_firewall();
    new spell_gl_firewall_b();
}
