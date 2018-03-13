/*Copyright (C) 2012 SkyMist Project.
*
* - ToDo: Drake respawn at boss reset after wipe.
*
* This file is NOT free software. Third-party users can NOT redistribute it or modify it :).
* If you find it, you are either hacking something, or very lucky (presuming someone else managed to hack it).
*/

#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "bastion_of_twilight.h"

enum Yells
{
   SAY_AGGRO      = 0,
   SAY_KILL       = 1,
   SAY_KILL_2     = 2,
   SAY_DRAGON     = 0,
};

enum Spells
{
    // Proto-Behemoth
    SPELL_FIREBALL                           = 86058, // Basic spell. 1.5s cast.
    SPELL_FIREBALL_TD                        = 83862, // Time Dilation. 3s cast.
    SPELL_SCORCHING_BREATH                   = 83707,  // Triggers 83855 which needs radius.
    SPELL_FIREBALL_BARRAGE                   = 83706,  // This is channel aura, trigger 83719.
    SPELL_FIREBALL_BARRAGE_DAMAGE            = 83720,  // This with 83719 which has dummy script.
    SPELL_FIREBALL_BARRAGE_DAMAGE_TD         = 83733,  // This with 83719 which has dummy script.
    SPELL_DANCING_FLAMES                     = 84106,  // Activates Fireball Barrage, aura on activating Time Warden.
    SPELL_SUPERHEATED_BREATH                 = 83956,  // Activates Scorching Breath, aura on activating Orphaned Emerald Whelps.

    // Halfus Wyrmbreaker
    SPELL_FURIOUS_ROAR                       = 83710,  // Three times in a row. Needs radius 100 y.
    SPELL_MALEVOLENT_STRIKES                 = 39171,  // Slate Dragon awakes.
    SPELL_FRENZIED_ASSAULT                   = 83693,  // Nether Scion awakes.
    SPELL_SHADOW_WRAPPED                     = 83952,  // Gains aura when Storm Rider awakes.
    SPELL_SHADOW_NOVA                        = 83703,  // Every other 30 - 40 seconds.
    SPELL_BERSERK                            = 26662,  // 6 min enrage timer.
    SPELL_BIND_WILL                          = 83432,  // On drakes at aggro.

    // Dragon debuffs on Halfus
    SPELL_NETHER_BLINDNESS                   = 83611, // From Nether Scion.
    SPELL_CYCLONE_WINDS                      = 84092, // From Storm Rider.
    SPELL_ATROPHIC_POISON                    = 83609, // Eight stacks, from whelps.
    SPELL_TIME_DILATION                      = 83601, // From Time Warden.
    SPELL_STONE_TOUCH_NORMAL                 = 83603,
    SPELL_STONE_TOUCH_HEROIC                 = 84593, // Stone Grip, from Slate Dragon.
    SPELL_PARALYSIS                          = 84030,
    SPELL_PETRIFICATION                      = 84591,
    SPELL_DRAGONS_VENGEANCE                  = 87683, // Debuff on Halfus by dragons when they die.

    //  Dragons
    SPELL_UNRESPONSIVE_DRAGON                = 86003, // Dragons.
    SPELL_UNRESPONSIVE_WHELP                 = 86022,  // Whelps.
    SPELL_FREE_DRAGON                        = 83589
};

enum Events
{
    // Halfus
    EVENT_SHADOW_NOVA = 1,
    EVENT_FURIOUS_ROAR,
    EVENT_FURIOUS_ROAR_CAST,
    EVENT_BERSERK, // 6 min.
    // Behemoth
    EVENT_FIREBALL,
    EVENT_SCORCHING_BREATH,
    EVENT_FIREBALL_BARRAGE
};

enum Creatures
{
    NPC_PROTO_BEHEMOTH                       = 44687,
    NPC_SLATE_DRAGON                         = 44652,
    NPC_NETHER_SCION                         = 44645,
    NPC_STORM_RIDER                          = 44650,
    NPC_TIME_RIDER                           = 44797,
    NPC_ORPHANED_WHELP                       = 44641
};

struct Position const dragonPos [5] =
{
    {-272.912f, -730.280f, 904.511f, 2.377750f},
    {-285.921f, -660.572f, 888.096f, 3.128040f},
    {-270.511f, -679.909f, 888.086f, 3.074224f},
    {-319.047f, -721.080f, 888.086f, 2.860756f},
    {-350.260f, -698.793f, 888.100f, 5.531962f}
};

struct Position const welpPos [8] =
{
    {-340.75f, -722.476f, 891.764f, 1.16937f},
    {-350.564f, -721.663f, 891.763f, 1.01229f},
    {-342.002f, -726.528f, 891.764f, 1.16937f},
    {-344.839f, -720.163f, 891.764f, 1.0821f},
    {-350.571f, -725.417f, 891.763f, 1.06465f},
    {-347.578f, -717.302f, 891.763f, 1.0472f},
    {-347.08f, -728.128f, 891.764f, 1.11701f},
    {-346.434f, -724.049f, 891.764f, 1.09956f}
};

#define HALFUS_DRAGON_WORLDSTATE    20070

class blindWillEvent : public BasicEvent
{
public:
    blindWillEvent(Creature* halfus, Creature* dragon) : _halfus(halfus), _dragon(dragon) { }

    bool Execute(uint64 /*execTime*/, uint32 /*diff*/) override
    {
        _halfus->CastSpell(_dragon, SPELL_BIND_WILL, true);
        _dragon->SetReactState(REACT_AGGRESSIVE);
        _dragon->SetInCombatWithZone();
        return true;
    }

private:
    Creature* _halfus;
    Creature* _dragon;
};

class boss_halfus : public CreatureScript
{
    public:
        boss_halfus() : CreatureScript("boss_halfus_wyrmbreaker") {}

        struct boss_halfusAI : public BossAI
        {
            boss_halfusAI(Creature* creature) : BossAI(creature, DATA_HALFUS) {}

            void Reset()
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                me->RemoveAllAuras();
                welpDieCount = 0;
                RoarCasts = 3;
                roarPhase = false;
                canNova   = false;
                _Reset();
                initDragons();

                std::list<GameObject*> CageList;
                me->GetGameObjectListWithEntryInGrid(CageList, 205087, 100.0f);
                if (!CageList.empty())
                    for (std::list<GameObject*>::const_iterator itr = CageList.begin(); itr != CageList.end(); ++itr)
                    {
                        (*itr)->ResetDoorOrButton();
                        (*itr)->SetGoState(GO_STATE_READY);
                        (*itr)->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    }
            }

            void JustDied(Unit* /*killer*/)
            {
                if (Creature* behemoth = me->FindNearestCreature(NPC_PROTO_BEHEMOTH, 200.0f, true))
                    behemoth->DespawnOrUnsummon();
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                _JustDied();
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(RAND(SAY_KILL, SAY_KILL_2));
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
            }

            void EnterCombat(Unit* who)
            {
                Talk(SAY_AGGRO);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                events.ScheduleEvent(EVENT_BERSERK, 6 * MINUTE * IN_MILLISECONDS);
                if (Creature* behemoth = me->FindNearestCreature(NPC_PROTO_BEHEMOTH, 500.0f, true))
                    behemoth->SetInCombatWith(who);

                std::list<GameObject*> CageList;
                me->GetGameObjectListWithEntryInGrid(CageList, 205087, 100.0f);
                if (!CageList.empty())
                    for (std::list<GameObject*>::const_iterator itr = CageList.begin(); itr != CageList.end(); ++itr)
                        (*itr)->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);

                _EnterCombat();
            }

            void initDragons()
            {
                me->SummonCreature(NPC_PROTO_BEHEMOTH, dragonPos[0], TEMPSUMMON_MANUAL_DESPAWN);
                me->SummonCreature(NPC_SLATE_DRAGON, dragonPos[1], TEMPSUMMON_MANUAL_DESPAWN);
                me->SummonCreature(NPC_NETHER_SCION, dragonPos[2], TEMPSUMMON_MANUAL_DESPAWN);
                me->SummonCreature(NPC_STORM_RIDER, dragonPos[3], TEMPSUMMON_MANUAL_DESPAWN);
                me->SummonCreature(NPC_TIME_RIDER, dragonPos[4], TEMPSUMMON_MANUAL_DESPAWN);
                for (uint8 i = 0; i < 8; ++i)
                    me->SummonCreature(NPC_ORPHANED_WHELP, welpPos[i], TEMPSUMMON_MANUAL_DESPAWN);
                selectActiveDragons();
            }

            void SummonedCreatureDies(Creature* summon, Unit* /*killer*/)
            {
                switch (summon->GetEntry())
                {
                    case NPC_ORPHANED_WHELP:
                        welpDieCount++;
                        if (welpDieCount > 7)
                            me->AddAura(SPELL_DRAGONS_VENGEANCE, me);
                        break;
                    case NPC_SLATE_DRAGON:
                    case NPC_NETHER_SCION:
                    case NPC_STORM_RIDER:
                    case NPC_TIME_RIDER:
                        me->AddAura(SPELL_DRAGONS_VENGEANCE, me);
                        break;
                    default:
                        break;
                }
            }

            void setWhelpsUnresponsive()
            {
                std::list<Creature*> creatures;
                GetCreatureListWithEntryInGrid(creatures, me, NPC_ORPHANED_WHELP, 100.0f);

                if (creatures.empty())
                   return;

                for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                {
                    me->AddAura(SPELL_UNRESPONSIVE_WHELP, (*iter));
                    (*iter)->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                }
            }

            void selectActiveDragons()
            {
                Creature* slateDragon  = me->FindNearestCreature(NPC_SLATE_DRAGON, 100.0f, true);
                Creature* netherScion  = me->FindNearestCreature(NPC_NETHER_SCION, 100.0f, true);
                Creature* stormRider   = me->FindNearestCreature(NPC_STORM_RIDER, 100.0f, true);
                Creature* timeRider    = me->FindNearestCreature(NPC_TIME_RIDER, 100.0f, true);

                if (!IsHeroic())
                {
                    uint8 drakeCombination = sWorld->getWorldState(HALFUS_DRAGON_WORLDSTATE);

                    switch(drakeCombination)
                    {
                        case 1:  // Slate, Storm, Whelps.
                            me->AddAura(SPELL_UNRESPONSIVE_DRAGON, netherScion);
                            me->AddAura(SPELL_UNRESPONSIVE_DRAGON, timeRider);
                            break;
                        case 2:  // Slate, Nether, Time.
                            setWhelpsUnresponsive();
                            me->AddAura(SPELL_UNRESPONSIVE_DRAGON, stormRider);
                            break;
                        case 3:  // Slate, Storm, Time.
                            setWhelpsUnresponsive();
                            me->AddAura(SPELL_UNRESPONSIVE_DRAGON, netherScion);
                            break;
                        case 4:  // Storm, Nether, Time.
                            setWhelpsUnresponsive();
                            me->AddAura(SPELL_UNRESPONSIVE_DRAGON, slateDragon);
                            break;
                        case 5:  // Slate, Storm, Nether.
                            setWhelpsUnresponsive();
                            me->AddAura(SPELL_UNRESPONSIVE_DRAGON, timeRider);
                            break;
                        case 6:  // Slate, Whelps, Time.
                            me->AddAura(SPELL_UNRESPONSIVE_DRAGON, netherScion);
                            me->AddAura(SPELL_UNRESPONSIVE_DRAGON, stormRider);
                            break;
                        case 7:  // Whelps, Nether, Time.
                            me->AddAura(SPELL_UNRESPONSIVE_DRAGON, slateDragon);
                            me->AddAura(SPELL_UNRESPONSIVE_DRAGON, stormRider);
                            break;
                        case 8:  // Storm, Whelps, Time.
                            me->AddAura(SPELL_UNRESPONSIVE_DRAGON, netherScion);
                            me->AddAura(SPELL_UNRESPONSIVE_DRAGON, slateDragon);
                            break;
                        case 9:  // Storm, Whelps, Nether.
                            me->AddAura(SPELL_UNRESPONSIVE_DRAGON, timeRider);
                            me->AddAura(SPELL_UNRESPONSIVE_DRAGON, slateDragon);
                            break;
                        case 10: // Slate, Whelps, Nether.
                            me->AddAura(SPELL_UNRESPONSIVE_DRAGON, timeRider);
                            me->AddAura(SPELL_UNRESPONSIVE_DRAGON, stormRider);
                            break;
                        default:
                            me->AddAura(SPELL_UNRESPONSIVE_DRAGON, netherScion);
                            me->AddAura(SPELL_UNRESPONSIVE_DRAGON, timeRider);
                            break;
                    }
                }

                if (!slateDragon->HasAura(SPELL_UNRESPONSIVE_DRAGON))
                    me->AddAura(SPELL_MALEVOLENT_STRIKES, me);
                else slateDragon->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

                if (!netherScion->HasAura(SPELL_UNRESPONSIVE_DRAGON))
                    me->AddAura(RAID_MODE(83693, 86160, 86161, 86162), me);
                else netherScion->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

                if (!stormRider->HasAura(SPELL_UNRESPONSIVE_DRAGON))
                    me->AddAura(SPELL_SHADOW_WRAPPED, me);
                else stormRider->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

                if (Creature* behemoth = me->FindNearestCreature(NPC_PROTO_BEHEMOTH, 100.0f, true))
                {
                    if (Creature* welp = me->FindNearestCreature(NPC_ORPHANED_WHELP, 100.0f, true))
                        if (!welp->HasAura(SPELL_UNRESPONSIVE_WHELP))
                            behemoth->AddAura(SPELL_SUPERHEATED_BREATH, behemoth);

                    if (!timeRider->HasAura(SPELL_UNRESPONSIVE_DRAGON))
                        behemoth->AddAura(SPELL_DANCING_FLAMES, behemoth);
                    else timeRider->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                }
            }

            void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/) override
            {
                if (HealthBelowPct(51) && !roarPhase)
                {
                    events.ScheduleEvent(EVENT_FURIOUS_ROAR, 2000);
                    roarPhase = true;
                }
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (me->HasAura(SPELL_SHADOW_WRAPPED) && !canNova)
                {
                    events.ScheduleEvent(EVENT_SHADOW_NOVA, urand(7000, 10000));
                    canNova = true;
                }

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_SHADOW_NOVA:
                            DoCast(me, SPELL_SHADOW_NOVA);
                            events.ScheduleEvent(EVENT_SHADOW_NOVA, 7200);
                            break;
                        case EVENT_FURIOUS_ROAR:
                            RoarCasts = 3;
                            if(!me->HasAura(SPELL_CYCLONE_WINDS))
                                events.ScheduleEvent(EVENT_FURIOUS_ROAR_CAST, 2100);
                            else
                                events.ScheduleEvent(EVENT_FURIOUS_ROAR_CAST, 10100);
                            break;
                        case EVENT_FURIOUS_ROAR_CAST:
                            if (RoarCasts > 0)
                            {
                                DoCast(me, SPELL_FURIOUS_ROAR);
                                --RoarCasts;

                                if(!me->HasAura(SPELL_CYCLONE_WINDS))
                                    events.ScheduleEvent(EVENT_FURIOUS_ROAR_CAST, 2100);
                                else
                                    events.ScheduleEvent(EVENT_FURIOUS_ROAR_CAST, 10100);

                                if (RoarCasts == 0)
                                    events.ScheduleEvent(EVENT_FURIOUS_ROAR, 30000);
                            }
                            break;
                        case EVENT_BERSERK:
                            DoCast(me, SPELL_BERSERK);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
            private:
                bool roarPhase, canNova, dragonsPicked;
                uint8 welpDieCount;
                uint8 RoarCasts;
        };
        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_halfusAI(creature);
        }

};

class npc_proto_behemoth : public CreatureScript
{
    public:
        npc_proto_behemoth() : CreatureScript("npc_proto_behemoth") {}

        struct npc_proto_behemothAI : public ScriptedAI
        {
            npc_proto_behemothAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->SetCanFly(true);
                me->SetDisableGravity(true);
            }

            void Reset()
            {
                events.Reset();
                me->RemoveAllAuras();
                me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, 0x02);
            }

            void AttackStart(Unit* target)
            {
                DoStartNoMovement(target);
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_FIREBALL, 3000);

                if (!me->HasAura(SPELL_DANCING_FLAMES) && me->HasAura(SPELL_SUPERHEATED_BREATH))
                    events.ScheduleEvent(EVENT_SCORCHING_BREATH, urand(12000, 17000));

                if (me->HasAura(SPELL_DANCING_FLAMES))
                    events.ScheduleEvent(EVENT_FIREBALL_BARRAGE, urand(12000, 17000));
            }

            void UpdateAI(const uint32 diff)
            {
                if (!me->isInCombat())
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_FIREBALL:
                            if (Creature* halfus = Creature::GetCreature(*me, instance->GetData64(NPC_HALFUS_WORMBREAKER))) // innerhit halfu's threat list
                            {
                                if (Unit* target = halfus->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                                {
                                    if (!me->HasAura(SPELL_TIME_DILATION))
                                        DoCast(target, SPELL_FIREBALL);
                                    else
                                        DoCast(target, SPELL_FIREBALL_TD);
                                }
                            }
                            if (!me->HasAura(SPELL_TIME_DILATION))
                                events.ScheduleEvent(EVENT_FIREBALL, 1500);
                            else
                                events.ScheduleEvent(EVENT_FIREBALL, 3000);
                            break;
                        case EVENT_SCORCHING_BREATH:
                            events.DelayEvents(10000); // for some reason "if (me->HasUnitState(UNIT_STATE_CASTING))" check isnt working for this spell...
                            DoCast(me, SPELL_SCORCHING_BREATH, true);
                            if (!me->HasAura(SPELL_DANCING_FLAMES))
                                events.ScheduleEvent(EVENT_SCORCHING_BREATH, 25000);
                            break;
                        case EVENT_FIREBALL_BARRAGE:
                            events.DelayEvents(10000); // for some reason "if (me->HasUnitState(UNIT_STATE_CASTING))" check isnt working for this spell...
                            DoCast(me, SPELL_FIREBALL_BARRAGE, true);
                            if (me->HasAura(SPELL_SUPERHEATED_BREATH))
                                events.ScheduleEvent(EVENT_SCORCHING_BREATH, 10000);
                            events.ScheduleEvent(EVENT_FIREBALL_BARRAGE, 25000);
                            break;
                    }
                }
            }
            private:
                InstanceScript* instance;
                EventMap events;
        };
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_proto_behemothAI(creature);
        }
};

class npc_halfus_dragon: public CreatureScript
{
public:
    npc_halfus_dragon () : CreatureScript("npc_halfus_dragon") { }

    struct npc_halfus_dragonAI: public ScriptedAI
    {
        npc_halfus_dragonAI (Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            me->SetReactState(REACT_PASSIVE);
        }

        void SpellHit(Unit* /*target*/, SpellInfo const* spellInfo) override
        {
            if (spellInfo->Id == SPELL_FREE_DRAGON)
            {
                if (Creature* Halfus = me->FindNearestCreature(NPC_HALFUS_WORMBREAKER, 500.0f, true))
                {
                    Talk(SAY_DRAGON);
                    switch (me->GetEntry())
                    {
                        case NPC_SLATE_DRAGON:
                            if (IsHeroic())
                                DoCast(Halfus, SPELL_STONE_TOUCH_HEROIC, true);
                            else
                                DoCast(Halfus, SPELL_STONE_TOUCH_NORMAL, true);
                            break;
                        case NPC_NETHER_SCION:
                            DoCast(Halfus, SPELL_NETHER_BLINDNESS, true);
                            break;
                        case NPC_STORM_RIDER:
                            DoCast(Halfus, SPELL_CYCLONE_WINDS, true);
                            break;
                        case NPC_TIME_RIDER:
                            if (Creature* behemoth = me->FindNearestCreature(NPC_PROTO_BEHEMOTH, 100.0f, true))
                                DoCast(behemoth, SPELL_TIME_DILATION, true);
                            break;
                    }

                    Halfus->m_Events.AddEvent(new blindWillEvent(Halfus, me), Halfus->m_Events.CalculateTime(1500));
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                    me->SetDisableGravity(true);
                    me->SetHover(true);
                }
            }
        }

        void UpdateAI (const uint32 /*diff*/) override
        {
            if (!me->HasAura(SPELL_UNRESPONSIVE_DRAGON) && instance->GetBossState(DATA_HALFUS) == IN_PROGRESS)
            {
                me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            } else me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    private:
        InstanceScript* instance;
    };

    CreatureAI* GetAI (Creature* creature) const
    {
        return new npc_halfus_dragonAI(creature);
    }
};

class npc_orphaned_whelp : public CreatureScript
{
public:
    npc_orphaned_whelp() : CreatureScript("npc_orphaned_whelp") { }

    struct npc_orphaned_whelpAI : public ScriptedAI
    {
        npc_orphaned_whelpAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset ()
        {
            me->DeleteThreatList();
            me->CombatStop(true);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }

        void EnterCombat(Unit* /*pWho*/)
        {
            if (Creature* Halfus = me->FindNearestCreature(NPC_HALFUS_WORMBREAKER, 150.0f, true))
            {
                if (!me->HasAura(SPELL_UNRESPONSIVE_WHELP))
                {
                    if (Creature* behemoth = me->FindNearestCreature(NPC_PROTO_BEHEMOTH, 100.0f, true))
                        DoCast(behemoth, SPELL_ATROPHIC_POISON, true);

                    if (!Halfus->isInCombat())
                        Halfus->AI()->DoZoneInCombat(Halfus, 150.0f);
                    Halfus->CastSpell(me, SPELL_BIND_WILL, true);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                }
            }
        }

        void UpdateAI(uint32 const /*diff*/) override
        {
            DoMeleeAttackIfReady();
        }

        private:
            InstanceScript* instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_orphaned_whelpAI(creature);
    }
};

class spell_proto_fireball : public SpellScriptLoader // 86058, 83862
{
public:
    spell_proto_fireball() : SpellScriptLoader("spell_proto_fireball") { }

    class spell_proto_fireballSpellScript : public SpellScript
    {
        PrepareSpellScript(spell_proto_fireballSpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();

            if (caster->HasAura(SPELL_TIME_DILATION))
                caster->CastSpell(GetHitUnit(), SPELL_FIREBALL_BARRAGE_DAMAGE_TD, false);
            else
                caster->CastSpell(GetHitUnit(), SPELL_FIREBALL_BARRAGE_DAMAGE, false);
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_proto_fireballSpellScript::HandleDummy,EFFECT_0,SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_proto_fireballSpellScript();
    }
};

class spell_proto_fireball_barrage : public SpellScriptLoader // 83719.
{
public:
    spell_proto_fireball_barrage() : SpellScriptLoader("spell_proto_fireball_barrage") { }

    class spell_proto_fireball_barrageSpellScript : public SpellScript
    {
        PrepareSpellScript(spell_proto_fireball_barrageSpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            if (InstanceScript* instance = caster->GetInstanceScript())
                if (Creature* halfus = Creature::GetCreature(*caster, instance->GetData64(NPC_HALFUS_WORMBREAKER)))
                    if (Unit* target = halfus->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                        caster->CastSpell(target, caster->HasAura(SPELL_TIME_DILATION) ? SPELL_FIREBALL_BARRAGE_DAMAGE_TD : SPELL_FIREBALL_BARRAGE_DAMAGE, true);
        }

        void Register() override
        {
            OnEffectHit += SpellEffectFn(spell_proto_fireball_barrageSpellScript::HandleDummy,EFFECT_0,SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript *GetSpellScript() const override
    {
        return new spell_proto_fireball_barrageSpellScript();
    }
};

// 84593
class spell_halfus_stone_touch: public SpellScriptLoader
{
public:
    spell_halfus_stone_touch() : SpellScriptLoader("spell_halfus_stone_touch") { }

    class spell_halfus_stone_touch_AuraScript: public AuraScript
    {
        PrepareAuraScript(spell_halfus_stone_touch_AuraScript)

        void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
        {
            Unit* halfus = GetTarget();
            if (GetId() == SPELL_STONE_TOUCH_NORMAL)
                halfus->CastSpell(halfus, SPELL_PARALYSIS, true);
            else if (GetId() == SPELL_STONE_TOUCH_HEROIC)
                halfus->CastSpell(halfus, SPELL_PETRIFICATION, true);
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_halfus_stone_touch_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_halfus_stone_touch_AuraScript();
    }
};

class go_halfus_whelp_cage : public GameObjectScript
{
    public:
        go_halfus_whelp_cage() : GameObjectScript("go_halfus_whelp_cage") { }

        bool OnGossipHello(Player* player, GameObject* go)
        {
            std::list<Creature*> creatures;
            GetCreatureListWithEntryInGrid(creatures, go, NPC_ORPHANED_WHELP, 50.0f);

            if (creatures.empty())
               return false;

            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                (*iter)->AI()->AttackStart(player);

            go->UseDoorOrButton();
            player->PlayerTalkClass->ClearMenus();

            return true;
        }
};

class spell_halfus_malevolent_strikes : public SpellScriptLoader
{
public:
    spell_halfus_malevolent_strikes() : SpellScriptLoader("spell_halfus_malevolent_strikes") { }

    class spell_halfus_malevolent_strikes_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_halfus_malevolent_strikes_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            if (eventInfo.GetHitMask() & PROC_EX_NORMAL_HIT || eventInfo.GetHitMask() & PROC_EX_CRITICAL_HIT)
                return true;
            return false;
        }

        void Register()
        {
            DoCheckProc += AuraCheckProcFn(spell_halfus_malevolent_strikes_AuraScript::CheckProc);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_halfus_malevolent_strikes_AuraScript();
    }
};

// All are verified below to DB, creature_template & spell_script_names
void AddSC_boss_halfus()
{
    new boss_halfus();
    new npc_proto_behemoth();
    new npc_halfus_dragon();
    new npc_orphaned_whelp();
    new spell_proto_fireball();
    new spell_proto_fireball_barrage();
    new spell_halfus_stone_touch();
    new go_halfus_whelp_cage();
    new spell_halfus_malevolent_strikes();
}
