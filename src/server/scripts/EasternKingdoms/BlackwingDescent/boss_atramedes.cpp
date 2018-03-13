
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"
#include "blackwing_descent.h"

enum Spells
{
    SPELL_BREATH_ACTIVATE_GAMEOBJECT         = 81573, // hit 204276 gob
    SPELL_DEVASTATION                        = 78898, // select target
    SPELL_SOUND_BAR_PLAYER                   = 88824,
    SPELL_SOUND_BAR                          = 89683,
    SPELL_SUMMON_SONAR_PULSE                 = 77672, // sum
    SPELL_SONAR_AURA                         = 77674, // trigger 77675

    SPELL_SONIC_BREATH_1                     = 78075, // trigger SPELL_SONAR_BREATH_SUMMON 22:28:55.376
    SPELL_SONIC_BREATH_SUMMON                = 78091, // summon 41879
    SPELL_SONIC_BREATH                       = 78098, // cast on 41879
    SPELL_TRACKING                           = 78092,

    SPELL_MODULATION                         = 77612,

    SPELL_FLAME_BREATH_1                     = 78207, // trigger 78213
    SPELL_FLAME_BREATH_SUMMON                = 78213, // summon 41962
    SPELL_ROARING_FLAME_BREATH_REVERSE       = 78230, // target 41442 reverse cast
    SPELL_ROARING_FLAME_BREATH               = 78221, // target 41962
    SPELL_FLAME_BREATH_SPEED_AURA            = 78217,
    SPELL_FLAME_BREATH_SUMMON_2              = 78272, // summon 42001

    SPELL_ROARING_FLAME_BREATH_TRIGGER       = 78354,
    SPELL_FLAME_BREATH_DAMAGE_2              = 78353,
    SPELL_AGGRO_CREATE                       = 63709,

    SPELL_ROAMING_FLAME_VISUAL               = 78024, // on 42001
    SPELL_ROAMING_FLAMME_TRIGGER             = 78018,

    SPELL_SEARING_FLAME                      = 77840,
    SPELL_SEARING_FLAME_DMG                  = 77974,
    SPELL_CLEAR_POWER                        = 88773,
    SPELL_SONAR_BOMB                         = 92557, // hit 49623
    //SPELL_ROARING_FLAME_BREATH_SUM  = 78503, // Cast on the player that touches the shield
    SPELL_LIFT_OFF                           = 92569,
    SPELL_RESONATING_CLASH                   = 77611,
    SPELL_RESONATING_CLASH_PLR               = 78168,
    SPELL_RESONATING_CLASH_AUR               = 77709,

    SPELL_VERTIGO                            = 77717,
    SPELL_SONIC_SHIELD                       = 78864, // break shield
    SPELL_SONIC_FLAMES                       = 77782, // kill NPC_ANCIENT_DWARVEN_SHIELD


    SPELL_NOISY                              = 78897,
    SPELL_FLAME_BREATH                       = 92815,
    SPELL_SUMMON_IMP                         = 92708,
    SPELL_BERSERK                            = 26662,

    SPELL_TAKE_OFF_ANIM_KIT                  = 86915,
    SPELL_LAUNCH_SONAR_PULSE                 = 92519,
    SPELL_SONAR_PULSE_X                      = 92526,

    SPELL_LUNAR_COLUMN_VISUAL                = 95660,
};

enum Events
{
    EVENT_MODULATION                = 1,
    EVENT_SONIC_BREATH,
    EVENT_SONAR_PULSE,
    EVENT_SEARING_FLAME,
    EVENT_AIR_PHASE,
    EVENT_SONAR_BOMB,
    EVENT_ROARING_FLAME_BREATH,
    EVENT_GROUND_PHASE,
    EVENT_SET_FLYING,
    EVENT_DESTROY_SHIELD,
    EVENT_BREATH_END,
    EVENT_SUMMON_IMP,
    EVENT_AGGRESSIVE,
};

enum Entities
{
    NPC_SONAR_PULSE                 = 49623,
    NPC_TRACKING_FLAMES             = 41879,
    NPC_REVERBERATING_FLAME         = 41962,
    NPC_ROARING_FLAME               = 41807,
    NPC_ANCIENT_DWARVEN_SHIELD      = 42949,
    NPC_LUNAR_COLUMN                = 51506,
};

enum Misc
{
    PATH_INTRO                      = 4144200,
    ACTION_GONG                     = 1,
    ACTION_DEATH,
    ACTION_OBNOXIOUS
};

enum Quotes
{
    SAY_AGGRO,
    SAY_SEARING_FLAME,
    SAY_AIR_PHASE,
    SAY_SLAY,
    SAY_DEATH
};

#define NPC_PULSAR_2 41546

static const Position centerPos = {128.681808f, -225.258270f, 75.454468f, M_PI};

static const Position homePos = {196.520615f, -225.095123f, 75.453438f, M_PI};

typedef std::list<Unit*> UnitList;

class boss_atramedes : public CreatureScript
{
    class SoundOrderPred
    {
    public:
        SoundOrderPred() {}

        bool operator() (const Unit* a, const Unit* b) const
        {
            return a->GetPower(POWER_ALTERNATE_POWER) != 100 && a->GetPower(POWER_ALTERNATE_POWER) > b->GetPower(POWER_ALTERNATE_POWER);
        }
    };

    struct boss_atramedesAI : public BossAI
    {
        boss_atramedesAI(Creature * creature) : BossAI(creature, DATA_ATRAMEDES)
        {
            introDone = false;
            nefarianGUID = 0;
            me->DisableMovementFlagUpdate(true);
        }

        void Reset()
        {
            me->setActive(true);
            airPhase = false;
            enrageTimer = 600000;
            flameGUID = 0;
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SOUND_BAR_PLAYER);
            _dataSilenceIsGolden = 1;
            if (introDone)
            {
                me->SetReactState(REACT_AGGRESSIVE);
                me->SetCanFly(false);
                me->SetDisableGravity(false);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            }
            else
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->SetReactState(REACT_PASSIVE);
                me->GetMotionMaster()->MovePath(PATH_INTRO, false);
                me->SetCanFly(true);
                me->SetDisableGravity(true);
                introTimer = 0;
            }
            if(Creature * nefarian = me->GetCreature(*me, nefarianGUID))
            {
                nefarian->DisappearAndDie();
                nefarian->DespawnOrUnsummon(3000);
            }
            _Reset();
            ResetShields();
        }

        void SetData(uint32 type, uint32 value)
        {
            switch (type)
            {
                case DATA_SILENCE_IS_GOLDEN:
                    _dataSilenceIsGolden = value;
                    break;
                default:
                    break;
            }
        }

        uint32 GetData(uint32 type) const
        {
            switch (type)
            {
                case DATA_SILENCE_IS_GOLDEN:
                    return _dataSilenceIsGolden;
                default:
                    return 0;
            }
        }

        void ResetShields()
        {
            std::list<Creature*> gongList;
            me->GetCreatureListWithEntryInGrid(gongList, NPC_ANCIENT_DWARVEN_SHIELD, 1000.0f);

            if (gongList.empty())
                return;

            for (std::list<Creature*>::const_iterator itr = gongList.begin(); itr != gongList.end(); ++itr)
            {
                (*itr)->Respawn(true);
                (*itr)->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_HIDE_NAMEPLATE | UNIT_FLAG2_UNK2);
                (*itr)->RemoveAllAuras();
                (*itr)->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                (*itr)->SetCorpseDelay(3600);
            }
        }

        void JustSummoned(Creature * summon)
        {
            switch (summon->GetEntry())
            {
                case NPC_TRACKING_FLAMES:
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    me->CastSpell(summon, SPELL_SONIC_BREATH, false);
                    me->AddUnitMovementFlag(MOVEMENTFLAG_TRACKING_TARGET);
                    me->SetUInt64Value(UNIT_FIELD_TARGET, summon->GetGUID());
                    me->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, summon->GetGUID());
                    me->SetFacingTo(me->GetAngle(summon));
                    events.ScheduleEvent(EVENT_AGGRESSIVE, 8500);
                    break;
                case NPC_ROARING_FLAME:
                {
                    summon->CastSpell(summon, SPELL_ROAMING_FLAME_VISUAL, true);
                    summon->CastSpell(summon, SPELL_ROAMING_FLAMME_TRIGGER, true);
                    BossAI::JustSummoned(summon);
                    break;
                }
                case NPC_NEFARIUS_ATRAMEDES:
                    nefarianGUID = summon->GetGUID();
                    break;
                default:
                    BossAI::JustSummoned(summon);
                    break;
            }
        }

        void SpellHit(Unit * caster, const SpellInfo * spell)
        {
            if (spell->Id == SPELL_RESONATING_CLASH_PLR)
            {
                if (!airPhase)
                {
                    me->RemoveAurasDueToSpell(SPELL_SEARING_FLAME);
                    me->RemoveAurasDueToSpell(SPELL_SEARING_FLAME_DMG);
                    DoCast(me, SPELL_VERTIGO, true);
                }
                else
                {
                    if (Creature * flame = me->GetCreature(*me, flameGUID))
                        flame->AI()->SetGUID(caster->GetGUID());
                }
                if (IsHeroic())
                    if(Creature * nefarian = me->GetCreature(*me, nefarianGUID))
                        nefarian->AI()->DoAction(ACTION_GONG);
            }
        }

        void EnterCombat(Unit * /*who*/)
        {
            Talk(SAY_AGGRO);
            events.ScheduleEvent(EVENT_MODULATION, urand(10000, 12000));
            events.ScheduleEvent(EVENT_SONIC_BREATH, urand(20000, 25000));
            events.ScheduleEvent(EVENT_SONAR_PULSE, urand(10000, 14000));
            events.ScheduleEvent(EVENT_SEARING_FLAME, 45000);
            events.ScheduleEvent(EVENT_AIR_PHASE, 80000);
            if (IsHeroic())
            {
                me->SummonCreature(NPC_NEFARIUS_ATRAMEDES, 98.551704f, -225.954895f, 98.597633f, 0.0f);
                events.ScheduleEvent(EVENT_SUMMON_IMP, 10000);
            }
            DoCast(SPELL_SOUND_BAR);
            instance->DoCastSpellOnPlayers(SPELL_CLEAR_POWER);
            DoCast(SPELL_DEVASTATION);
            _EnterCombat();
        }


        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE)
            {
                me->GetMotionMaster()->MoveIdle();
                DoCast(SPELL_LIFT_OFF);
                events.ScheduleEvent(EVENT_SET_FLYING, 4500);
                events.ScheduleEvent(EVENT_GROUND_PHASE, 40000);
                events.ScheduleEvent(EVENT_SONAR_BOMB, 8000);
                events.ScheduleEvent(EVENT_ROARING_FLAME_BREATH, 5000);
            }
            else if (type == WAYPOINT_MOTION_TYPE && id == 1)
            {
                DoCast(SPELL_FLAME_BREATH);
                if(GameObject * go = me->FindNearestGameObject(GO_ANCIENT_BELL, 100.0f))
                    go->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);
                me->SetHomePosition(homePos);
                introTimer = 5000;
            }
        }

        void KilledUnit(Unit * victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_SLAY);
        }

        void JustDied(Unit * /*killer*/)
        {
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SOUND_BAR_PLAYER);

            if (Creature * nefarian = me->GetCreature(*me, nefarianGUID))
                nefarian->AI()->DoAction(ACTION_DEATH);
            Talk(SAY_DEATH);
            _JustDied();
        }

        void UpdateAI(const uint32 diff)
        {
            if (!introDone && introTimer)
            {
                if (introTimer <= diff)
                {
                    introDone = true;
                    me->SetSpeed(MOVE_FLIGHT, 1.5f, true);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->SetHomePosition(homePos);
                    EnterEvadeMode();
                }
                else introTimer -= diff;
            }

            if (!UpdateVictim())
                return;

            if (IsHeroic())
            {
                if(enrageTimer <= diff)
                {
                    DoCast(SPELL_BERSERK);
                    enrageTimer = 600000;
                }
                else enrageTimer -= diff;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING) || me->HasAura(SPELL_VERTIGO))
                if(!airPhase)
                    return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_AGGRESSIVE:
                        me->SetReactState(REACT_AGGRESSIVE);
                        break;
                    case EVENT_MODULATION:
                        DoCast(SPELL_MODULATION);
                        events.ScheduleEvent(EVENT_MODULATION, urand(20000, 25000));
                        break;
                    case EVENT_SONIC_BREATH:
                        events.DelayEvents(8500);
                        me->CastCustomSpell(SPELL_SONIC_BREATH_1, SPELLVALUE_MAX_TARGETS, 1, me, false);
                        events.ScheduleEvent(EVENT_SONIC_BREATH, urand(25000, 27000));
                        break;
                    case EVENT_SONAR_PULSE:
                    {
                        me->CastCustomSpell(SPELL_SUMMON_SONAR_PULSE, SPELLVALUE_MAX_TARGETS, 4, me, false);
                        events.ScheduleEvent(EVENT_SONAR_PULSE, urand(8000, 10000));
                        break;
                    }
                    case EVENT_SEARING_FLAME:
                        Talk(SAY_SEARING_FLAME);
                        DoCast(me, SPELL_SEARING_FLAME, true);
                        DoCast(me, SPELL_SEARING_FLAME_DMG, true);
                        events.ScheduleEvent(EVENT_SEARING_FLAME, 45000);
                        break;
                    case EVENT_AIR_PHASE:
                        airPhase = true;
                        events.Reset();
                        me->SetReactState(REACT_PASSIVE);
                        me->SetUInt64Value(UNIT_FIELD_TARGET, 0);
                        me->GetMotionMaster()->MoveIdle();
                        me->GetMotionMaster()->MovePoint(0, centerPos);
                        break;
                    case EVENT_SET_FLYING:
                        me->SetCanFly(true);
                        me->SetDisableGravity(true);
                        me->NearTeleportTo(centerPos.GetPositionX(), centerPos.GetPositionY(), centerPos.GetPositionZ() + 20.0f, M_PI);
                        break;
                    case EVENT_SONAR_BOMB:
                        me->CastCustomSpell(SPELL_SONAR_PULSE_X, SPELLVALUE_MAX_TARGETS, RAID_MODE<int32>(5, 12, 5, 12), NULL);
                        events.ScheduleEvent(EVENT_SONAR_BOMB, 3000);
                        break;
                    case EVENT_ROARING_FLAME_BREATH:
                    {
                        Talk(SAY_AIR_PHASE);

                        UnitList targetList;

                        ThreatContainer::StorageType const &threatList = me->getThreatManager().getThreatList();
                        for (ThreatContainer::StorageType::const_iterator itr = threatList.begin(); itr != threatList.end(); ++itr)
                            if ((*itr)->getTarget()->GetTypeId() == TYPEID_PLAYER)
                                targetList.push_back((*itr)->getTarget());

                        targetList.sort(SoundOrderPred());

                        if (Unit * target = targetList.front())
                        {
                            if (Creature * flame = me->SummonCreature(NPC_REVERBERATING_FLAME, me->GetPositionX(), me->GetPositionY(), 75.46f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 25000))
                            {
                                flame->AI()->SetGUID(target->GetGUID());
                                flame->SetInCombatWithZone();
                                flameGUID = flame->GetGUID();
                                DoCast(flame, SPELL_ROARING_FLAME_BREATH, false);
                                flame->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                                flame->SetDisplayId(11686);
                            }
                        }
                    }
                    break;
                case EVENT_GROUND_PHASE:
                    airPhase = false;
                    if(Creature * flame = me->GetCreature(*me, flameGUID))
                        flame->DespawnOrUnsummon();
                    flameGUID = 0;
                    me->SetCanFly(false);
                    me->SetDisableGravity(false);
                    me->SetReactState(REACT_AGGRESSIVE);
                    if(Unit * victim = me->getVictim())
                    {
                        me->SetUInt64Value(UNIT_FIELD_TARGET, victim->GetGUID());
                        DoStartMovement(victim);
                    }
                    me->MonsterMoveWithSpeed(centerPos.GetPositionX(), centerPos.GetPositionY(), centerPos.GetPositionZ(), 100);
                    events.Reset();
                    events.ScheduleEvent(EVENT_MODULATION, urand(10000, 12000));
                    events.ScheduleEvent(EVENT_SONIC_BREATH, urand(15000, 20000));
                    events.ScheduleEvent(EVENT_SONAR_PULSE, urand(10000, 14000));
                    events.ScheduleEvent(EVENT_SEARING_FLAME, 45000);
                    events.ScheduleEvent(EVENT_AIR_PHASE, 80000);
                    if (IsHeroic())
                        events.ScheduleEvent(EVENT_SUMMON_IMP, 5000);
                    break;
                case EVENT_SUMMON_IMP:
                    if(Creature * nefarian = me->GetCreature(*me, nefarianGUID))
                        nefarian->AI()->DoAction(ACTION_OBNOXIOUS);
                    events.ScheduleEvent(EVENT_SUMMON_IMP, 60000);
                    break;
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
            EnterEvadeIfOutOfCombatArea(diff);
        }
        private:
            bool airPhase;
            bool introDone;
            uint32 introTimer;
            uint32 enrageTimer;
            uint64 flameGUID;
            uint64 nefarianGUID;
            uint32 _dataSilenceIsGolden;
    };
public:
    boss_atramedes() : CreatureScript("boss_atramedes") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new boss_atramedesAI(creature);
    }
};

class spell_sound_bar : public SpellScriptLoader
{
public:
    spell_sound_bar() : SpellScriptLoader("spell_sound_bar") { }

    class spell_sound_bar_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sound_bar_AuraScript);

        void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            GetTarget()->RemoveVehicleKit();
        }

        void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            // 843
            // 445
            // 443
            uint32 vehicleId = 443;

            if (!GetTarget()->CreateVehicleKit(vehicleId, 0))
                return;

            if (Player* player = GetTarget()->ToPlayer())
            {
                WorldPacket data(SMSG_PLAYER_VEHICLE_DATA, player->GetPackGUID().size() + 4);
                data.appendPackGUID(player->GetGUID());
                data << uint32(vehicleId);
                player->SendMessageToSet(&data, true);

                player->SendOnCancelExpectedVehicleRideAura();
            }
        }

        void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
        {
            Unit* target = GetTarget();
            if (InstanceScript* instance = target->GetInstanceScript())
                if (Creature* atramedes = ObjectAccessor::GetCreature(*target, instance->GetData64(NPC_ATRAMEDES)))
                {
                    if (target->GetPower(POWER_ALTERNATE_POWER) > 50)
                        atramedes->AI()->SetData(DATA_SILENCE_IS_GOLDEN, 0);
                    if (target->GetPower(POWER_ALTERNATE_POWER) >= 100 && !target->HasAura(SPELL_NOISY))
                        target->CastSpell(target, SPELL_NOISY, true);
                }
        }

        void Register() override
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_sound_bar_AuraScript::HandleEffectRemove, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            AfterEffectApply += AuraEffectApplyFn(spell_sound_bar_AuraScript::HandleEffectApply, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_sound_bar_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_sound_bar_AuraScript();
    }
};

class npc_tracking_flames : public CreatureScript
{
    enum
    {
        SPELL_BUILDING_SPEED            = 78217,
        SPELL_BUILDING_SPEED_EFF        = 78218,
        SPELL_BUILDING_SPEED_BOOST      = 78969,
        SPELL_FLAME_BREATH_PER_EFF      = 78431,
        SPELL_SUMMON_TRACKER            = 78354,
    };

    struct npc_tracking_flamesAI : public ScriptedAI
    {
        npc_tracking_flamesAI(Creature * creature) : ScriptedAI(creature) {}

        void Reset()
        {
            targetGUID = 0;
            moveTimer = 0;
            change = false;
            me->CastSpell(me, SPELL_SUMMON_TRACKER, true);
            summFlammes = 3000;
            me->SetSpeed(MOVE_WALK, 0.6f);
            me->SetSpeed(MOVE_RUN, 0.6f);
            DoCast(SPELL_BUILDING_SPEED_BOOST);
            start = false;
        }

        void DamageTaken(Unit * /*who*/, uint32 &damage)
        {
            damage = 0;
        }

        void SetGUID(uint64 guid, int32)
        {
            me->ClearUnitState(UNIT_STATE_CASTING);
            me->RemoveAurasDueToSpell(SPELL_BUILDING_SPEED);
            me->RemoveAurasDueToSpell(SPELL_BUILDING_SPEED_EFF);
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveIdle();

            if (Unit* unit = me->GetUnit(*me, guid))
            {
                if (Unit* old = me->GetUnit(*me, targetGUID))
                {
                    me->getThreatManager().modifyThreatPercent(old, -100);
                    old->RemoveAurasDueToSpell(SPELL_TRACKING);
                }
                unit->CastSpell(unit, SPELL_TRACKING, true);
                me->AddThreat(unit, 500000.0f);
                me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                float speed = me->GetSpeed(MOVE_RUN);
                moveTimer = uint32(me->GetExactDist(unit) / speed * 1000);
                change = true;
                start = true;
            }
            targetGUID = guid;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!start)
                return;

            if (summFlammes <= diff)
            {
                if (Creature* creature = me->SummonCreature(NPC_ROARING_FLAME, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 80000))
                {
                    if (Unit* player = me->GetUnit(*me, targetGUID))
                    {
                        Position destPos;
                        player->GetPosition(&destPos);
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MoveIdle();
                        me->GetMotionMaster()->MovePoint(0, destPos);
                        creature->CastSpell(creature, SPELL_ROAMING_FLAME_VISUAL, true);
                        creature->CastSpell(creature, SPELL_ROAMING_FLAMME_TRIGGER, true);
                    }
                }
                summFlammes = 500;
            }
            else
                summFlammes -= diff;

            if (!change)
                return;

            if (moveTimer <= diff)
            {
                change = false;
                me->RemoveAurasDueToSpell(SPELL_BUILDING_SPEED_BOOST);
                me->RemoveAurasDueToSpell(SPELL_BUILDING_SPEED_EFF);
                me->RemoveAurasDueToSpell(SPELL_BUILDING_SPEED);
                DoCast(me, SPELL_BUILDING_SPEED, true);
            }
            else
                moveTimer -= diff;
        }

    private:
        bool change, start;
        uint32 moveTimer;
        uint64 targetGUID;
        uint32 summFlammes;
    };

public:
    npc_tracking_flames() : CreatureScript("npc_tracking_flames") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_tracking_flamesAI(creature);
    }
};

class spell_modulation : public SpellScriptLoader
{
    class spell_modulation_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_modulation_SpellScript);

        void HandleEffect(SpellEffIndex /*effIndex*/)
        {
            if(Unit * target = GetHitUnit())
            {
                int32 damage = GetHitDamage();
                int32 sound = target->GetPower(POWER_ALTERNATE_POWER);
                SetHitDamage(damage + (damage * sound * 2) / 100);
                if (Map *map = target->GetMap())
                    if (!map->IsHeroic())
                        target->ModifyPower(POWER_ALTERNATE_POWER, 7);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_modulation_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };
public:
    spell_modulation() : SpellScriptLoader("spell_modulation") { }

    SpellScript* GetSpellScript() const
    {
        return new spell_modulation_SpellScript();
    }
};

class npc_ancient_dwarven_shield : public CreatureScript
{
public:
    npc_ancient_dwarven_shield() : CreatureScript("npc_ancient_dwarven_shield") {}

    struct npc_ancient_dwarven_shieldAI : public PassiveAI
    {
        npc_ancient_dwarven_shieldAI(Creature * creature) : PassiveAI(creature) {}

        void OnSpellClick(Unit* player, bool& result)
        {
            if (!result)
                return;

            DoCast(SPELL_RESONATING_CLASH);
            DoCast(me, SPELL_RESONATING_CLASH_AUR, true);
            player->CastSpell(player, SPELL_RESONATING_CLASH_PLR, true);
            DoCast(SPELL_SONIC_SHIELD);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->Kill(me);
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_ancient_dwarven_shieldAI(creature);
    }
};


class go_ancient_bell : public GameObjectScript
{
    bool OnGossipHello(Player* player, GameObject* pGO)
    {
        if (InstanceScript * instance = player->GetInstanceScript())
        {
            instance->SetData(DATA_ATRAMEDES_SUMMON, 1);
            pGO->SendCustomAnim(0);
            pGO->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
            if (Creature *lunarCol = player->FindNearestCreature(NPC_LUNAR_COLUMN, 100))
                lunarCol->DespawnOrUnsummon();
        }
        return true;
    }
public:
    go_ancient_bell() : GameObjectScript("go_ancient_bell") { }
};

// npc 49740
enum
{
    SPELL_PESTERED      = 92685,
    SPELL_OBNOXIOUS     = 92677,
    SPELL_PHASE_SHIFT   = 92681
};

class npc_obnoxious_fiend : public CreatureScript
{
    struct npc_obnoxious_fiendAI : public ScriptedAI
    {
        npc_obnoxious_fiendAI(Creature * creature) : ScriptedAI(creature) {}

        void Reset()
        {
            me->CastSpell(me, SPELL_PHASE_SHIFT, true);
            me->SetInCombatWithZone();
            if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true, -SPELL_PESTERED))
                me->AddThreat(target, 5000000.0f);
            if (Creature * atramedes = me->FindNearestCreature(NPC_ATRAMEDES, 500.0f))
                atramedes->AI()->JustSummoned(me);
        }

        void DamageTaken(Unit * /*who*/, uint32 &damage)
        {
            if (!me->GetVehicle())
                damage = 0;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (!me->HasUnitState(UNIT_STATE_CASTING))
            {
                if (Vehicle *veh = me->GetVehicle())
                    if (Unit *u = veh->GetBase())
                        me->CastSpell(u, SPELL_OBNOXIOUS, false);
            }

            DoMeleeAttackIfReady();
        }
    };

public:
    npc_obnoxious_fiend() : CreatureScript("npc_obnoxious_fiend") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_obnoxious_fiendAI(creature);
    }
};

// npc 49580
class npc_victor_nefarius_atramedes : public CreatureScript
{
    enum
    {
        SAY_GONG,
        SAY_DEATH,
        SAY_OBNOXIOUS,
        SPELL_HOVER_ANIM           = 90766,
        SPELL_FEAR_AURA            = 78494,
        SPELL_BREAK_SHIELD         = 92607, // target shield 42947
        SPELL_IMP_PROJECTILE       = 92708, // 49807 imp portal
        SPELL_IMP_PORTAL_SUM       = 92709, // summon 49807 // 11 28 38 - 11:38:34.234
        SPELL_IMP_PORTAL_AURA      = 92707,
        SPELL_IMP_SUM              = 92625,
    };

    struct npc_victor_nefarius_atramedesAI : public ScriptedAI
    {
        npc_victor_nefarius_atramedesAI(Creature * creature) : ScriptedAI(creature)
        {
            me->setActive(true);
        }

        void Reset()
        {
            canTalk = true;
            canBreakGong = false;
            textCooldown = 0;
            mui_break_shield = 3000;
            me->CastSpell(me, SPELL_FEAR_AURA, true);
        }

        void IsSummonedBy(Unit *)
        {
        }

        void JustSummoned(Creature* summon)
        {
            summon->CastSpell(summon, SPELL_IMP_PORTAL_AURA, true);
        }

        void DoAction(const int32 action)
        {
            switch(action)
            {
            case ACTION_GONG:
                if(canTalk)
                {
                    canTalk = false;
                    textCooldown = 10000;
                    Talk(SAY_GONG);
                }
                canBreakGong = true;
                break;
            case ACTION_DEATH:
                Talk(SAY_DEATH);
                me->DespawnOrUnsummon(5000);
                break;
            case ACTION_OBNOXIOUS:
                if (Creature *atramedes = me->FindNearestCreature(41442, 500, true))
                    me->CastSpell(atramedes, SPELL_IMP_PROJECTILE);
                Talk(SAY_OBNOXIOUS);
                break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!canTalk)
            {
                if (textCooldown <= diff)
                    canTalk = true;
                else
                    textCooldown -= diff;
            }
            if (canBreakGong)
            {
                if (mui_break_shield <= diff)
                {
                    me->CastCustomSpell(SPELL_BREAK_SHIELD, SPELLVALUE_MAX_TARGETS, 1, me, false);
                    canBreakGong = false;
                    mui_break_shield = 3000;
                }
                else
                    mui_break_shield -= diff;
            }
        }

    private:
        bool canTalk;
        uint32 textCooldown;
        bool canBreakGong;
        uint32 mui_break_shield;
    };

public:
    npc_victor_nefarius_atramedes() : CreatureScript("npc_victor_nefarius_atramedes") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_victor_nefarius_atramedesAI(creature);
    }
};

class npc_sonnar_pulse_summon : public CreatureScript
{
public:
    npc_sonnar_pulse_summon() : CreatureScript("npc_sonnar_pulse_summon") {}

    struct npc_sonnar_pulse_summonAI : public ScriptedAI
    {
        npc_sonnar_pulse_summonAI(Creature * creature) : ScriptedAI(creature)
        {
        }

        void Reset()
        {
        }

        void IsSummonedBy(Unit *player)
        {
            me->CastSpell(me, SPELL_SONAR_AURA, true);
            float orient = player->GetOrientation() + M_PI;
            Position::NormalizeOrientation(orient);
            float x, y;
            me->GetNearPoint2D(x, y, 80.0f, orient);
            me->GetMotionMaster()->MovePoint(0, x, y,  me->GetPositionZ() + 1.0f);
        }

        void UpdateAI(const uint32 diff)
        {
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_sonnar_pulse_summonAI(creature);
    }
};

class npc_sonic_breath : public CreatureScript
{
public:
    npc_sonic_breath() : CreatureScript("npc_sonic_breath") {}

    class TargetChaseEvent : public BasicEvent
    {
    public:
        TargetChaseEvent(Creature& owner, uint64 targetGUID) : BasicEvent(), m_owner(owner), m_targetGUID(targetGUID) { }

        bool Execute(uint64 /*eventTime*/, uint32 /*diff*/)
        {
            if (Player * target = m_owner.GetPlayer(m_owner, m_targetGUID))
            {
                m_owner.ClearUnitState(UNIT_STATE_CASTING);
                m_owner.GetMotionMaster()->Clear();
                m_owner.GetMotionMaster()->MoveFollow(target, 0.0f, 0.0f);
            }
            return true;
        }

    private:
        Creature& m_owner;
        uint64 m_targetGUID;
    };

    class TargetStopChaseEvent : public BasicEvent
    {
    public:
        TargetStopChaseEvent(Creature& owner, uint64 targetGUID) : BasicEvent(), m_owner(owner), m_targetGUID(targetGUID) { }

        bool Execute(uint64 /*eventTime*/, uint32 /*diff*/)
        {
             if (Player * target = m_owner.GetPlayer(m_owner, m_targetGUID))
             {
                 target->RemoveAurasDueToSpell(SPELL_TRACKING);
                 m_owner.DespawnOrUnsummon(100);
             }
             return true;
        }

    private:
        Creature& m_owner;
        uint64 m_targetGUID;
    };

    struct npc_sonic_breathAI : public ScriptedAI
    {
        npc_sonic_breathAI(Creature * creature) : ScriptedAI(creature)
        {
        }

        void Reset()
        {
        }

        void IsSummonedBy(Unit *summoner)
        {
            if (InstanceScript *instance = me->GetInstanceScript())
                if (Creature *atramedes = Unit::GetCreature(*me, instance->GetData64(NPC_ATRAMEDES)))
                {
                    summoner->CastSpell(summoner, SPELL_TRACKING, true);
                    atramedes->AI()->JustSummoned(me);
                }
            me->m_Events.AddEvent(new TargetChaseEvent(*me, summoner->GetGUID()), me->m_Events.CalculateTime(2250));
            me->m_Events.AddEvent(new TargetStopChaseEvent(*me, summoner->GetGUID()), me->m_Events.CalculateTime(2250 + 8000));
        }

        void UpdateAI(const uint32 diff)
        {
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_sonic_breathAI(creature);
    }
};

class spell_altramedes_devastation : public SpellScriptLoader
{
    class spell_altramedes_devastation_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_altramedes_devastation_SpellScript);

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            if (unitList.empty())
                return;
            for (std::list<WorldObject*>::iterator itr = unitList.begin(); itr != unitList.end();)
            {
                Unit *u = (*itr)->ToUnit();
                if (!u || u->GetTypeId() != TYPEID_PLAYER || u->GetPower(POWER_ALTERNATE_POWER) != 100)
                    itr = unitList.erase(itr);
                else
                    ++itr;
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_altramedes_devastation_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

public:
    spell_altramedes_devastation() : SpellScriptLoader("spell_altramedes_devastation") { }

    SpellScript * GetSpellScript() const
    {
        return new spell_altramedes_devastation_SpellScript();
    }
};
#define SPELL_DEVASTATION_TRIG 78868

class spell_atramedes_devastation_periodic : public SpellScriptLoader
{
public:
    spell_atramedes_devastation_periodic() : SpellScriptLoader("spell_atramedes_devastation_periodic") { }

    class spell_atramedes_devastation_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_atramedes_devastation_periodic_AuraScript);

        void HandleTriggerSpell(AuraEffect const* aurEff)
        {
            PreventDefaultAction();
            if (Unit *caster = GetCaster())
            {
                Map::PlayerList const& players = caster->GetMap()->GetPlayers();
                if (!players.isEmpty())
                    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                        if (Player* player = itr->getSource())
                            if (player->GetPower(POWER_ALTERNATE_POWER) == 100)
                            {
                                caster->CastSpell(caster, SPELL_DEVASTATION_TRIG, true);
                                break;
                            }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_atramedes_devastation_periodic_AuraScript::HandleTriggerSpell, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_atramedes_devastation_periodic_AuraScript();
    }
};

class npc_sonnar_pulse_summon_air_phase : public CreatureScript
{
public:
    npc_sonnar_pulse_summon_air_phase() : CreatureScript("npc_sonnar_pulse_summon_air_phase") {}

    struct npc_sonnar_pulse_summon_air_phaseAI : public ScriptedAI
    {
        npc_sonnar_pulse_summon_air_phaseAI(Creature * creature) : ScriptedAI(creature)
        {
        }

        void Reset()
        {
        }

        void IsSummonedBy(Unit *)
        {
            if (Creature *atramedes = me->FindNearestCreature(41442, 100))
                atramedes->CastSpell(me, SPELL_SONAR_BOMB, true);
        }

        void UpdateAI(const uint32 diff)
        {
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_sonnar_pulse_summon_air_phaseAI(creature);
    }
};

class npc_maloriak_atramedes_event : public CreatureScript
{
    enum
    {
        NPC_NEFARIUS_EVENT = 43396, // X: 150.7813 Y: -231.1962 Z: 75.53674
        NPC_MALORIAK_EVENT = 43404, //  X: 149.7569 Y: -207.6285 Z: 75.53674
        NPC_LITTLE_ATRAMEDES = 43407, // X: 136.0764 Y: -207.6441 Z: 75.53674
        NPC_FOG_DREAM = 43400,

        GOB_THRONE = 202832,

        SPELL_COSMETIC_ALPHA_STATE_50_PCT = 69676,
        SPELL_POTION_BELT = 81271,
        SPELL_POTION_HAND = 81217,
        SPELL_USE_POTION = 81221, // hit 43407
        SPELL_DRAGON_CANT_SEE = 81214,
        SPELL_FOG_AURA = 81178,
    };

    struct npc_maloriak_atramedes_eventAI : public ScriptedAI
    {
        npc_maloriak_atramedes_eventAI(Creature * creature) : ScriptedAI(creature), summons(creature)
        {
        }

        void Reset()
        {
            me->SetVisible(false);
            canTalk = false;
            start = false;
            textCooldown = 5000;
            eventId = 0;
            me->CastSpell(me, SPELL_COSMETIC_ALPHA_STATE_50_PCT, true);
            me->CastSpell(me, SPELL_POTION_BELT, true);
            mui_start = 5000;
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!start)
            {
                if (mui_start <= diff)
                {
                    if (!me->FindNearestCreature(43125, 100, true) && !me->FindNearestCreature(43128, 100, true) && !me->FindNearestCreature(43129, 100, true)
                        && !me->FindNearestCreature(43119, 100, true) && !me->FindNearestCreature(43127, 100, true) && !me->FindNearestCreature(43122, 100, true)
                        && !me->FindNearestCreature(43130, 100, true) && !me->FindNearestCreature(43126, 100, true))
                    {
                        start = true;
                        canTalk = true;
                        me->SetVisible(true);
                        me->NearTeleportTo(149.7569f, -207.6285f, 75.53674f, 0.0f);
                        if (Creature *nefarius = me->SummonCreature(NPC_NEFARIUS_EVENT, 150.7813f, -231.1962f, 75.53674f, 0.0f))
                        {
                            nefarius->CastSpell(nefarius, SPELL_COSMETIC_ALPHA_STATE_50_PCT, true);
                            nefarius->SetFacingToObject(me);
                            me->SetFacingToObject(nefarius);
                            if (Creature *atramedes = me->SummonCreature(NPC_LITTLE_ATRAMEDES, 136.0764f, -207.6441f, 75.53674f, 0.0f))
                            {
                                atramedes->CastSpell(atramedes, SPELL_COSMETIC_ALPHA_STATE_50_PCT, true);
                                atramedes->SetFacingToObject(nefarius);
                            }
                            if (Creature *fog = me->SummonCreature(NPC_FOG_DREAM, 166.7326f, -220.3958f, 75.53674f, 0.0f))
                                fog->CastSpell(fog, SPELL_FOG_AURA, true);
                            if (Creature *fog = me->SummonCreature(NPC_FOG_DREAM, 155.2049f, -212.3611f, 75.53674f,  0.0f))
                                fog->CastSpell(fog, SPELL_FOG_AURA, true);
                            if (Creature *fog = me->SummonCreature(NPC_FOG_DREAM, 145.0451f, -225.7205f, 75.53674f, 0.0f))
                                fog->CastSpell(fog, SPELL_FOG_AURA, true);
                            me->SummonGameObject(GOB_THRONE, 151.47f, -235.87f, 74.94f, 1.67f, 0, 0, 0, 0, 3600);
                        }
                    }
                    mui_start = 5000;
                }
                else
                    mui_start -= diff;
            }

            if (canTalk)
            {
                if (textCooldown <= diff)
                {
                    switch (eventId)
                    {
                    case 0: // 43396
                        if (Creature *nefarius = me->FindNearestCreature(NPC_NEFARIUS_EVENT, 100))
                            nefarius->AI()->Talk(0);
                        textCooldown = 9000;
                        break; // 22:25:24.197
                    case 1:
                        Talk(0); // 43404
                        textCooldown = 10000;
                        break; // 22:25:33.932
                    case 2:
                        if (Creature *nefarius = me->FindNearestCreature(NPC_NEFARIUS_EVENT, 100))
                            nefarius->AI()->Talk(1);
                        textCooldown = 5000;
                        break; // 22:25:43.526
                    case 3:
                        Talk(1);
                        textCooldown = 2000;
                        break; // 22:25:48.346
                    case 4:
                        Talk(2);
                        textCooldown = 10000;
                        break; // 22:25:50.780
                    case 5:
                        Talk(3);
                        textCooldown = 0;
                        break; // 22:26:00.437
                    case 6:
                        if (Creature *atramedes = me->FindNearestCreature(NPC_LITTLE_ATRAMEDES, 100))
                            atramedes->GetMotionMaster()->MovePoint(0, 150.49f, -225.26f, 75.45f);
                        textCooldown = 5000;
                        break;
                    case 7:
                        Talk(4);
                        textCooldown = 6000;
                        break; // 22:26:05.273
                    case 8:
                        Talk(5);
                        me->CastSpell(me, SPELL_POTION_BELT, true);
                        textCooldown = 9000;
                        break; //  22:26:11.294
                    case 9:
                        me->CastSpell(me, SPELL_USE_POTION, true);
                        Talk(6);
                        textCooldown = 9000;
                        break; // 22:26:42.728 DONT exist
                    case 10:
                        if (Creature *atramedes = me->FindNearestCreature(NPC_LITTLE_ATRAMEDES, 100))
                            atramedes->CastSpell(atramedes, SPELL_DRAGON_CANT_SEE, true);
                        if (Creature *nefarius = me->FindNearestCreature(NPC_NEFARIUS_EVENT, 100))
                            nefarius->AI()->Talk(2);
                        textCooldown = 8000;
                        break; // 22:26:29.453
                    case 11:
                        if (Creature *nefarius = me->FindNearestCreature(NPC_NEFARIUS_EVENT, 100))
                            nefarius->AI()->Talk(3);
                        textCooldown = 5000;
                        break; // 22:26:37.892
                    case 12:
                        Talk(7);
                        textCooldown = 5000;
                        break; //  22:26:42.728
                    case 13:
                        if (Creature *nefarius = me->FindNearestCreature(NPC_NEFARIUS_EVENT, 100))
                            nefarius->AI()->Talk(4);
                        textCooldown = 4000;
                        break; // 22:26:47.564
                    case 14:
                        if (Creature *nefarius = me->FindNearestCreature(NPC_NEFARIUS_EVENT, 100))
                            nefarius->AI()->Talk(5);
                        textCooldown = 7000;
                        break; // 22:26:51.199
                    case 15:
                        if (Creature *nefarius = me->FindNearestCreature(NPC_NEFARIUS_EVENT, 100))
                            nefarius->AI()->Talk(6);
                        textCooldown = 7000;
                        break; // 22:26:58.422
                    case 16:
                        Talk(8);
                        textCooldown = 3000;
                        break; // 22:27:05.723
                    case 17:
                        if (Creature *lunarCol = me->FindNearestCreature(NPC_LUNAR_COLUMN, 100))
                            lunarCol->CastSpell(lunarCol, SPELL_LUNAR_COLUMN_VISUAL, true);
                        if (GameObject *gob = me->FindNearestGameObject(GOB_THRONE, 100))
                            gob->Delete();
                        if (GameObject *gob = me->FindNearestGameObject(GO_ANCIENT_BELL, 100))
                            gob->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
                        summons.DespawnAll();
                        me->SetVisible(false);
                        canTalk = false;
                        break;
                    default:
                        textCooldown = 5000;
                        break;
                    }
                    eventId++;
                }
                else
                    textCooldown -= diff;
            }
        }

    private:
        bool canTalk;
        uint32 textCooldown;
        uint8 eventId;
        bool start;
        uint32 mui_start;
        SummonList summons;
    };

public:
    npc_maloriak_atramedes_event() : CreatureScript("npc_maloriak_atramedes_event") {}

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_maloriak_atramedes_eventAI(creature);
    }
};

class spell_atramedes_periodic_flame_damage : public SpellScriptLoader
{
    class spell_atramedes_periodic_flame_damage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_atramedes_periodic_flame_damage_SpellScript);

        void HandleEffect(SpellEffIndex /*effIndex*/)
        {
            if(Unit * target = GetHitUnit())
            {
                int32 damage = GetHitDamage();
                int32 sound = target->GetPower(POWER_ALTERNATE_POWER);
                SetHitDamage(damage + (damage * sound * 2) / 100);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_atramedes_periodic_flame_damage_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };
public:
    spell_atramedes_periodic_flame_damage() : SpellScriptLoader("spell_atramedes_periodic_flame_damage") { }

    SpellScript* GetSpellScript() const
    {
        return new spell_atramedes_periodic_flame_damage_SpellScript();
    }
};

class achievement_silence_is_golden : public AchievementCriteriaScript
{
public:
    achievement_silence_is_golden() : AchievementCriteriaScript("achievement_silence_is_golden") { }

    bool OnCheck(Player* /*source*/, Unit* target)
    {
        if (!target)
            return false;

        return (target->GetAI() && !target->GetAI()->GetData(DATA_SILENCE_IS_GOLDEN));
    }
};

class spell_obnoxious_phase_shift : public SpellScriptLoader
{
public:
    spell_obnoxious_phase_shift() : SpellScriptLoader("spell_obnoxious_phase_shift") { }

    class spell_obnoxious_phase_shift_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_obnoxious_phase_shift_AuraScript);

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
            if (Unit *victim = GetCaster()->getVictim())
            {
                GetCaster()->CastSpell(victim, SPELL_PESTERED, true);
                GetCaster()->RemoveAura(SPELL_PHASE_SHIFT);
            }
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(spell_obnoxious_phase_shift_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_obnoxious_phase_shift_AuraScript();
    }
};


class spell_sonic_breath_periodic : public SpellScriptLoader
{
        class spell_sonic_breath_periodic_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sonic_breath_periodic_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targets.clear();
                if (Unit *atramedes = GetOriginalCaster())
                {
                    if (Unit *trigg = atramedes->FindNearestCreature(41879, 100, true))
                        atramedes->SetOrientation(atramedes->GetAngle(trigg));
                    Map::PlayerList const& players = atramedes->GetMap()->GetPlayers();
                    if (!players.isEmpty())
                        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                            if (Player* target = itr->getSource())
                                if (atramedes->isInFrontInMap(target, 500.0f, static_cast<float>(M_PI / 12)))
                                    targets.push_back(target);
                }
            }


            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sonic_breath_periodic_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_24);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sonic_breath_periodic_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_CONE_ENEMY_24);
            }
        };



public:
    spell_sonic_breath_periodic() : SpellScriptLoader("spell_sonic_breath_periodic") { }

    SpellScript * GetSpellScript() const
    {
        return new spell_sonic_breath_periodic_SpellScript();
    }
};

enum npcs_dwaren
{
    // right side
    NPC_SPIRIT_ANGEFORGE  = 43119,
    NPC_SPIRIT_THAURISSAN = 43126,
    NPC_SPRIT_IRONSTAR    = 43127,
    NPC_SPRIT_BURNINGEYE  = 43130,

    // left side
    NPC_SPRIT_ANVILRAGE = 43128,
    NPC_SPRIT_MOLTENFIST = 43125,
    NPC_SPRIT_SHADOWFORGE = 43129,
    NPC_SPRIT_COREHAMMER = 43122,
};

enum swareb_spells
{
    SPELL_STONEBLOOD = 80655,
    SPELL_AVATAR = 80645,
    SPELL_EXECUTION = 80727,
    SPELL_SHIELD_OF_LIGHTING = 80747,
    SPELL_WHIRLWIND = 80652,

    SPELL_STORMBOLT = 80648,
    SPELL_THUNDERCLAP = 80649,
    SPELL_CHAIN_OF_LIGHTING = 80646,
    SPELL_BURDEN_OF_THE_CROWN = 80718,

    SPELL_AURA_ANGEFORME   = 80762,
    SPELL_AURA_MOLTENFIST  = 80763,
    SPELL_AURA_COREHAMMER  = 80764,
    SPELL_AURA_THAURISSAN  = 80766,
    SPELL_AURA_BURNINGEYE  = 80770,
    SPELL_AURA_IRONSTAR    = 80767,
    SPELL_AURA_ANVILRAGE   = 80768,
    SPELL_AURA_SHADOWFORGE = 80769,
};

struct DwarenSpirits
{
    uint32 entry;
    uint32 eventId;
    uint32 eventStartTime;
    uint32 auraId;
};

enum dwarven_events
{
    EVENT_ANGEFORME_STONEBLOOD = 1,
    EVENT_THAURISSAN_AVATAR = 2,
    EVENT_IRONSTAR_EXECUTION_SENTENCE = 3,
    EVENT_IRONSTAR_SHIELD_OF_LIGHT = 4,
    EVENT_BURNINGEYE_WHIRLWIND = 5,

    EVENT_ANVILRAGE_STORMBOLT = 6,
    EVENT_MOLTENFIST_THUNDERCLAP = 7,
    EVENT_SHADOWFORGE_CHAIN_LIGHTING = 8,
    EVENT_COREHAMMER_BURDEN_OF_THE_CROWN = 9,
};

static const DwarenSpirits rightSpirits[4] =
{
    {NPC_SPIRIT_ANGEFORGE,  EVENT_ANGEFORME_STONEBLOOD,           2000, SPELL_AURA_ANGEFORME},
    {NPC_SPIRIT_THAURISSAN, EVENT_THAURISSAN_AVATAR,              2000, SPELL_AURA_THAURISSAN},
    {NPC_SPRIT_IRONSTAR,    EVENT_IRONSTAR_EXECUTION_SENTENCE,    2000, SPELL_AURA_IRONSTAR},
    {NPC_SPRIT_BURNINGEYE,  EVENT_BURNINGEYE_WHIRLWIND,           2000, SPELL_AURA_BURNINGEYE},
};

static const DwarenSpirits leftSpirits[4] =
{
    {NPC_SPRIT_ANVILRAGE,   EVENT_ANVILRAGE_STORMBOLT,            2000, SPELL_AURA_ANVILRAGE},
    {NPC_SPRIT_MOLTENFIST,  EVENT_MOLTENFIST_THUNDERCLAP,         2000, SPELL_AURA_MOLTENFIST},
    {NPC_SPRIT_SHADOWFORGE, EVENT_SHADOWFORGE_CHAIN_LIGHTING,     2000, SPELL_AURA_SHADOWFORGE},
    {NPC_SPRIT_COREHAMMER,  EVENT_COREHAMMER_BURDEN_OF_THE_CROWN, 2000, SPELL_AURA_COREHAMMER},
};


class npc_spirit_right_side : public CreatureScript
{
public:
    npc_spirit_right_side() : CreatureScript("npc_spirit_right_side") { }

    struct npc_spirit_right_sideAI : public ScriptedAI
    {
        npc_spirit_right_sideAI(Creature* creature) : ScriptedAI(creature)
        {
            for (int i= 0; i < 3; i++)
                _spirits[i] = 0;
        }

        void Reset()
        {
            events.Reset();
            for (int i= 0; i < 3; i++)
                if (Creature *c = Unit::GetCreature(*me, _spirits[i]))
                    if (!c->isAlive())
                    {
                        c->Respawn(true);
                        c->AI()->EnterEvadeMode();
                    }
        }



        void JustDied(Unit *killer)
        {
            for (int i= 0; i < 3; i++)
                if (Creature *c = Unit::GetCreature(*me, _spirits[i]))
                    if (c->isAlive())
                        c->AI()->DoAction(me->GetEntry());
        }

        void DoAction(int32 const action)
        {
            if (action == me->GetEntry())
                return;
            for (int i= 0; i < 4; i++)
                if (rightSpirits[i].entry == action)
                {
                    me->AddAura(rightSpirits[i].auraId, me);
                    me->SetHealth(me->GetMaxHealth());
                    events.ScheduleEvent(rightSpirits[i].eventId, rightSpirits[i].eventStartTime);
                    break;
                }
        }

        void EnterCombat(Unit* who)
        {
            int cnt = 0;
            for (int i= 0; i < 4; i++)
            {
                if (rightSpirits[i].entry == me->GetEntry())
                {
                    me->AddAura(rightSpirits[i].auraId, me);
                    events.ScheduleEvent(rightSpirits[i].eventId, rightSpirits[i].eventStartTime);
                }
                else if (Creature *spirit = me->FindNearestCreature(rightSpirits[i].entry, 100, true))
                {
                    _spirits[cnt++] = spirit->GetGUID();
                    spirit->AI()->AttackStart(who);
                }
            }
            me->SetInCombatWithZone();
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ANGEFORME_STONEBLOOD:
                    {
                        DoCast(SPELL_STONEBLOOD);
                        events.ScheduleEvent(EVENT_ANGEFORME_STONEBLOOD, 10000);
                        break;
                    }
                    case EVENT_THAURISSAN_AVATAR:
                    {
                        DoCast(SPELL_AVATAR);
                        events.ScheduleEvent(EVENT_THAURISSAN_AVATAR, 10000);
                        break;
                    }
                    case EVENT_IRONSTAR_EXECUTION_SENTENCE:
                    {
                        DoCast(SPELL_SHIELD_OF_LIGHTING);
                        events.ScheduleEvent(EVENT_IRONSTAR_EXECUTION_SENTENCE, 10000);
                        events.ScheduleEvent(EVENT_IRONSTAR_SHIELD_OF_LIGHT, 1000);
                        break;
                    }
                    case EVENT_IRONSTAR_SHIELD_OF_LIGHT:
                    {
                        DoCastRandom(SPELL_EXECUTION, 0.0f);
                        break;
                    }
                    case EVENT_BURNINGEYE_WHIRLWIND:
                    {
                        DoCast(SPELL_WHIRLWIND);
                        events.ScheduleEvent(EVENT_BURNINGEYE_WHIRLWIND, 10000);
                        break;
                    }
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private:
        uint64 _spirits[3];
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_spirit_right_sideAI(creature);
    }
};

class npc_spirit_left_side : public CreatureScript
{
public:
    npc_spirit_left_side() : CreatureScript("npc_spirit_left_side") { }

    struct npc_spirit_left_sideAI : public ScriptedAI
    {
        npc_spirit_left_sideAI(Creature* creature) : ScriptedAI(creature)
        {
            for (int i= 0; i < 3; i++)
                _spirits[i] = 0;
        }

        void Reset()
        {
            events.Reset();
            for (int i= 0; i < 3; i++)
                if (Creature *c = Unit::GetCreature(*me, _spirits[i]))
                    if (!c->isAlive())
                    {
                        c->Respawn(true);
                        c->AI()->EnterEvadeMode();
                    }
        }



        void JustDied(Unit *killer)
        {
            for (int i= 0; i < 3; i++)
                if (Creature *c = Unit::GetCreature(*me, _spirits[i]))
                    if (c->isAlive())
                        c->AI()->DoAction(me->GetEntry());
        }

        void DoAction(int32 const action)
        {
            if (action == me->GetEntry())
                return;
            for (int i= 0; i < 4; i++)
                if (leftSpirits[i].entry == action)
                {
                    me->AddAura(leftSpirits[i].auraId, me);
                    me->SetHealth(me->GetMaxHealth());
                    events.ScheduleEvent(leftSpirits[i].eventId, leftSpirits[i].eventStartTime);
                    break;
                }
        }

        void EnterCombat(Unit* who)
        {
            int cnt = 0;
            for (int i= 0; i < 4; i++)
            {
                if (leftSpirits[i].entry == me->GetEntry())
                {
                    me->AddAura(leftSpirits[i].auraId, me);
                    events.ScheduleEvent(leftSpirits[i].eventId, leftSpirits[i].eventStartTime);
                }
                else if (Creature *spirit = me->FindNearestCreature(leftSpirits[i].entry, 100, true))
                {
                    _spirits[cnt++] = spirit->GetGUID();
                    spirit->AI()->AttackStart(who);
                }
            }
            me->SetInCombatWithZone();
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ANVILRAGE_STORMBOLT:
                    {
                        DoCast(SPELL_STORMBOLT);
                        events.ScheduleEvent(EVENT_ANVILRAGE_STORMBOLT, 10000);
                        break;
                    }
                    case EVENT_MOLTENFIST_THUNDERCLAP:
                    {
                        DoCast(SPELL_THUNDERCLAP);
                        events.ScheduleEvent(EVENT_MOLTENFIST_THUNDERCLAP, 10000);
                        break;
                    }
                    case EVENT_SHADOWFORGE_CHAIN_LIGHTING:
                    {
                        DoCastRandom(SPELL_CHAIN_OF_LIGHTING, 0.0f);
                        events.ScheduleEvent(EVENT_SHADOWFORGE_CHAIN_LIGHTING, 10000);
                        break;
                    }
                    case EVENT_COREHAMMER_BURDEN_OF_THE_CROWN:
                    {
                        DoCast(SPELL_BURDEN_OF_THE_CROWN);
                        events.ScheduleEvent(EVENT_COREHAMMER_BURDEN_OF_THE_CROWN, 10000);
                        break;
                    }
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private:
        uint64 _spirits[3];
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_spirit_left_sideAI(creature);
    }
};

void AddSC_boss_atramedes()
{
    new boss_atramedes();
    new spell_sound_bar();
    new npc_tracking_flames();
    new spell_modulation();
    new npc_ancient_dwarven_shield();
    new go_ancient_bell();
    new npc_obnoxious_fiend();
    new npc_victor_nefarius_atramedes();
    new npc_sonnar_pulse_summon();
    new npc_sonic_breath();
    new spell_altramedes_devastation();
    new spell_atramedes_devastation_periodic();
    new npc_sonnar_pulse_summon_air_phase();
    new npc_maloriak_atramedes_event();
    new spell_atramedes_periodic_flame_damage();
    new achievement_silence_is_golden();
    new spell_obnoxious_phase_shift();
    new spell_sonic_breath_periodic();

    new npc_spirit_right_side();
    new npc_spirit_left_side();
};
