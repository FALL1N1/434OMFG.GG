
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "grimbatol.h"

enum Texts
{
    SAY_ENGAGE                  = 0,
    SAY_SHAD_GALE               = 1,
    SAY_GUARDIAN                = 2,
    SAY_SLAY                    = 3,
    SAY_DEATH                   = 4,
    EMOTE_SHADOW_GALE           = 5,
    EMOTE_FACELESS              = 6,
    EMOTE_SHIELD_OF_NIGHTMARES  = 7
};

enum Spells
{
    SPELL_ENFEEBLING_BLOW                       = 75789,
    SPELL_SHADOW_GALE                           = 75664,
    SPELL_SHADOW_GALE_SPEED_TRIGGER             = 75675,
    SPELL_SPAWN_FACELESS                        = 75704,
    SPELL_PORTAL_VISUAL                         = 82847,
    SPELL_SHIELD_OF_NIGHTMARES                  = 75809,
    SPELL_BINDING_SHADOWS                       = 79466,
    SPELL_UMBRAL_MENDING                        = 79467,
    SPELL_TWILIGHT_CORRUPTION                   = 75520,
    SPELL_SIPHON_ESSENCE                        = 75755,
    SPELL_SUMMON_TWILIGHT_HATCHLINGS            = 91058,
    SPELL_SHADOW_GALE_SPEED                     = 75694,
    SPELL_SUMMON_SHADOWGALE_TRIGGER             = 75655,
    SPELL_SHADOW_GALE_TRIGGER                   = 75656,
    SPELL_SUMMON_TWILIGHT_EGG                   = 75723,
    SPELL_TWILIGHT_BLAST_PERIODIC               = 76192
};

enum Events
{
    // Erudax
    EVENT_NONE,
    EVENT_ENFEEBLING_BLOW,
    EVENT_SHADOW_GALE,
    EVENT_SUMMON_FACELESS,
    EVENT_REMOVE_TWILIGHT_PORTAL,
    EVENT_CAST_SHIELD_OF_NIGHTMARE_DELAY,
    EVENT_BINDING_SHADOWS,
    EVENT_SHIELD_OF_NIGHTMARES,
    // Faceless Corruptor
    EVENT_MOVE_WP_2,
    EVENT_MOVE_EGG,
    EVENT_TARGET_EGG,
    EVENT_SIPHON,
    EVENT_UMBRAL
};

enum achievements
{
    ACHIEVEMENT_GRIMBATOL               = 4840,
    ACHIEVEMENT_GRIMBATOL_HEROIC        = 5062,
    ACHIEVEMENT_WITHOUT_CORRUPTION      = 5298,
    ACHIEVEMENT_GRIMBATOL_GUILD_GROUP_CRITERIA = 14486
};

Position const wp1Left = { -647.6017f, -831.5976f, 239.2672f };
Position const wp2Left = { -701.7274f, -833.2674f, 232.4126f };
Position const wp1Right = { -651.6711f, -825.3476f, 238.7546f };
Position const wp2Right = { -699.342f, -818.5434f, 232.4729f };

enum Waypoints
{
    WP_1_LEFT,
    WP_2_LEFT,
    WP_1_RIGHT,
    WP_2_RIGHT,
    WP_EGG
};

Position const portalStalkerPosition = { -624.135f, -826.972f, 239.5073f, 3.176499f };
#define HATCHLING_CENTER_X -731.478f
#define HATCHLING_CENTER_Y -822.027f
#define HATCHLING_RADIUS 30.0f
#define HATCHLING_HEIGHT 245.0f + urand(0, 10)

class boss_erudax: public CreatureScript
{
public:
    boss_erudax() : CreatureScript("boss_erudax") { }

    struct boss_erudaxAI : public ScriptedAI
    {
        boss_erudaxAI(Creature* pCreature) : ScriptedAI(pCreature), ShouldSummonAdds(false)
        {
            instance = pCreature->GetInstanceScript();
        }

        void Reset()
        {
            events.Reset();
            ResetMinions();
            RemoveShadowGaleDebuffFromPlayers();
            FacelessPortalStalkerGUID = 0;
            isCorruption = false;
            isCasting = false;
            if (instance)
                instance->SetData(DATA_ERUDAX, NOT_STARTED);
        }

        void EnterCombat(Unit* /*who*/)
        {
            ShouldSummonAdds = false;
            me->SetReactState(REACT_AGGRESSIVE);
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveChase(me->getVictim());
            events.ScheduleEvent(EVENT_ENFEEBLING_BLOW, urand(17000, 23000));
            events.ScheduleEvent(EVENT_BINDING_SHADOWS, urand(8000, 12000));
            events.ScheduleEvent(EVENT_SHADOW_GALE, urand(30000, 50000));
            Talk(SAY_ENGAGE);
            if (instance)
                instance->SetData(DATA_ERUDAX, IN_PROGRESS);
            if (Creature* FacelessPortalStalker = me->SummonCreature(NPC_FACELESS_PORTAL_STALKER, portalStalkerPosition, TEMPSUMMON_MANUAL_DESPAWN))
                FacelessPortalStalkerGUID = FacelessPortalStalker->GetGUID();
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (ShouldSummonAdds)
            {
                ShouldSummonAdds = false;
                RemoveShadowGaleDebuffFromPlayers();
                me->SetReactState(REACT_AGGRESSIVE);
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveChase(me->getVictim());
                if (rand() % 2)
                    Talk(SAY_GUARDIAN);
                if (Creature* FacelessPortalStalker = Unit::GetCreature(*me, FacelessPortalStalkerGUID))
                {
                    if (FacelessPortalStalker->AI())
                    {
                        FacelessPortalStalker->AI()->DoCast(FacelessPortalStalker, SPELL_PORTAL_VISUAL, true);
                        FacelessPortalStalker->AI()->DoCast(FacelessPortalStalker, SPELL_SPAWN_FACELESS, true);
                    }
                }
                Talk(EMOTE_FACELESS);
                events.ScheduleEvent(EVENT_REMOVE_TWILIGHT_PORTAL, 7000);
                events.ScheduleEvent(EVENT_SHADOW_GALE, urand(40000, 50000));

                if (IsHeroic())
                    events.ScheduleEvent(EVENT_SHIELD_OF_NIGHTMARES, urand(15000, 30000));
            }

            isCasting = false;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ENFEEBLING_BLOW:
                        if (uint32 cooldown = me->GetSpellCooldownDelay(sSpellMgr->GetSpellIdForDifficulty(SPELL_ENFEEBLING_BLOW, me)))
                        {
                            events.ScheduleEvent(EVENT_ENFEEBLING_BLOW, cooldown);
                            break;
                        }
                        DoCastVictim(SPELL_ENFEEBLING_BLOW);
                        isCasting = true;
                        events.ScheduleEvent(EVENT_ENFEEBLING_BLOW, urand(19000,24000));
                        break;
                    case EVENT_SHADOW_GALE:
                        if (uint32 cooldown = me->GetSpellCooldownDelay(sSpellMgr->GetSpellIdForDifficulty(SPELL_SHADOW_GALE_TRIGGER, me)))
                        {
                            events.ScheduleEvent(EVENT_SHADOW_GALE, cooldown);
                            break;
                        }
                        me->SetReactState(REACT_PASSIVE);
                        Talk(SAY_SHAD_GALE);
                        DoCast(me, SPELL_SHADOW_GALE_TRIGGER, true);
                        DoCast(SPELL_SHADOW_GALE);
                        Talk(EMOTE_SHADOW_GALE);
                        break;
                    case EVENT_REMOVE_TWILIGHT_PORTAL:
                        if (Creature* FacelessPortalStalker = Unit::GetCreature(*me, FacelessPortalStalkerGUID))
                            FacelessPortalStalker->RemoveAllAuras();
                        break;
                    case EVENT_BINDING_SHADOWS:
                        if (uint32 cooldown = me->GetSpellCooldownDelay(sSpellMgr->GetSpellIdForDifficulty(SPELL_BINDING_SHADOWS, me)))
                        {
                            events.ScheduleEvent(EVENT_BINDING_SHADOWS, cooldown);
                            break;
                        }
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, SIZE_OF_GRIDS, true))
                            DoCast(target, SPELL_BINDING_SHADOWS);
                        isCasting = true;
                        events.ScheduleEvent(EVENT_BINDING_SHADOWS, urand(12000, 17000));
                        break;
                    case EVENT_SHIELD_OF_NIGHTMARES:
                        DoCast(SPELL_SHIELD_OF_NIGHTMARES);
                        isCasting = true;
                        Talk(EMOTE_SHIELD_OF_NIGHTMARES);
                        break;
                    default:
                        break;
                }

                if (isCasting)
                    break;
            }

            DoMeleeAttackIfReady();
            EnterEvadeIfOutOfCombatArea(diff);
        }

        void KilledUnit(Unit* victim)
        {
            Talk(SAY_SLAY);
        }

        virtual void JustReachedHome()
        {
            ResetMinions();
        }


        void JustDied(Unit* /*killer*/)
        {
            ResetMinions();
            RemoveShadowGaleDebuffFromPlayers();
            Talk(SAY_DEATH);
            if (instance)
            {
                instance->SetData(DATA_ERUDAX, DONE);
                if (IsHeroic())
                {
                    if (!isCorruption)
                        instance->DoCompleteAchievement(ACHIEVEMENT_WITHOUT_CORRUPTION);

                    if (IsHeroic())
                        instance->CompleteGuildCriteriaForGuildGroup(ACHIEVEMENT_GRIMBATOL_GUILD_GROUP_CRITERIA);
                }
            }
        }

        void JustSummoned(Creature* summon)
        {
            summon->setActive(true);
            switch (summon->GetEntry())
            {
                case NPC_SHADOW_GALE_STALKER:
                    summon->CastSpell(summon, SPELL_SHADOW_GALE_SPEED_TRIGGER, false);
                    ShouldSummonAdds = true;
                    break;
                default:
                    break;
            }
        }

        bool isCorruption;
    private:
        void ResetMinions()
        {
            DespawnCreatures(NPC_FACELESS_LEFT);
            DespawnCreatures(NPC_FACELESS_RIGHT);
            DespawnCreatures(NPC_FACELESS_PORTAL_STALKER);
            DespawnCreatures(NPC_TWILIGHT_HATCHLING);
            DespawnCreatures(NPC_SHADOW_GALE_STALKER);
            DespawnCreatures(NPC_TWILIGHT_EGG);
            RespawnEggs();
        }

        void DespawnCreatures(uint32 entry)
        {
            std::list<Creature*> creatures;
            GetCreatureListWithEntryInGrid(creatures, me, entry, SIZE_OF_GRIDS);
            if (creatures.empty())
                return;
            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                (*iter)->DespawnOrUnsummon();
        }

        void RespawnEggs()
        {
            std::list<Creature*> creatures;
            GetCreatureListWithEntryInGrid(creatures, me, NPC_ALEXSTRASZAS_EGG, SIZE_OF_GRIDS);
            if (creatures.empty())
                return;
            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
            {
                if ((*iter)->isDead())
                    (*iter)->Respawn();
                (*iter)->SetHealth(77500);
                (*iter)->SetMaxHealth(77500);
                (*iter)->SetVisible(true);
            }
        }

        void RemoveShadowGaleDebuffFromPlayers()
        {
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SHADOW_GALE_SPEED);
        }

        std::map<Waypoints, Position> waypoints;
        uint64 FacelessPortalStalkerGUID;
        InstanceScript* instance;
        EventMap events;
        bool ShouldSummonAdds;
        bool isCasting;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_erudaxAI(creature);
    }
};

class mob_faceless : public CreatureScript
{
public:
    mob_faceless() : CreatureScript("mob_faceless") { }

    struct mob_facelessAI : public ScriptedAI
    {
        mob_facelessAI(Creature* creature) : ScriptedAI(creature), isAtAnEgg(false), targetGUID(0), instance(creature->GetInstanceScript())
        {
            waypoints[WP_1_LEFT] = wp1Left;
            waypoints[WP_2_LEFT] = wp2Left;
            waypoints[WP_1_RIGHT] = wp1Right;
            waypoints[WP_2_RIGHT] = wp2Right;
        }

        void IsSummonedBy(Unit* summoner)
        {
            isAtAnEgg = false;
            me->SetReactState(REACT_PASSIVE);
            me->SetWalk(true);
            me->SetSpeed(MOVE_WALK, 3.0f);

            switch (me->GetEntry())
            {
                case NPC_FACELESS_LEFT:
                    me->GetMotionMaster()->MovePoint(WP_1_LEFT, waypoints[WP_1_LEFT]);
                    break;
                case NPC_FACELESS_RIGHT:
                    me->GetMotionMaster()->MovePoint(WP_1_RIGHT, waypoints[WP_1_RIGHT]);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);
            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_MOVE_WP_2:
                        switch (me->GetEntry())
                        {
                        case NPC_FACELESS_LEFT:
                            me->GetMotionMaster()->MovePoint(WP_2_LEFT, waypoints[WP_2_LEFT]);
                            break;
                        case NPC_FACELESS_RIGHT:
                            me->GetMotionMaster()->MovePoint(WP_2_RIGHT, waypoints[WP_2_RIGHT]);
                            break;
                        default:
                            break;
                        }
                        break;
                    case EVENT_MOVE_EGG:
                    {
                        Creature* egg = NULL;
                        egg = GetRandomEgg();
                        if (egg)
                        {
                            Position pos;
                            egg->GetContactPoint(me, pos.m_positionX, pos.m_positionY, pos.m_positionZ);
                            egg->GetFirstCollisionPosition(pos, egg->GetObjectSize(), egg->GetRelativeAngle(me));
                            me->GetMotionMaster()->MovePoint(WP_EGG, pos);
                            targetGUID = egg->GetGUID();
                        }
                        events.ScheduleEvent(EVENT_TARGET_EGG, 0);
                        break;
                    }
                    case EVENT_SIPHON:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(SPELL_SIPHON_ESSENCE);
                        events.ScheduleEvent(EVENT_SIPHON, urand(9500, 12000));
                        break;
                    }
                    case EVENT_UMBRAL:
                    {
                        if (Unit* erudax = me->FindNearestCreature(BOSS_ERUDAX, SIZE_OF_GRIDS, true))
                            DoCast(erudax, SPELL_UMBRAL_MENDING, false);
                        break;
                    }
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE)
            {
                switch (id)
                {
                    case WP_1_LEFT:
                    case WP_1_RIGHT:
                        events.ScheduleEvent(EVENT_MOVE_WP_2, 0);
                        break;
                    case WP_2_LEFT:
                    case WP_2_RIGHT:
                        events.ScheduleEvent(EVENT_MOVE_EGG, 0);
                        break;
                    case WP_EGG:
                    {
                        if (instance)
                            if (Creature *c = Unit::GetCreature(*me, instance->GetData64(DATA_ERUDAX)))
                                CAST_AI(boss_erudax::boss_erudaxAI, c->AI())->isCorruption = true;
                        DoCast(SPELL_TWILIGHT_CORRUPTION);
                        isAtAnEgg = true;
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() != TYPEID_PLAYER)
                if (victim->GetEntry() == NPC_ALEXSTRASZAS_EGG)
                {
                    events.Reset();
                    events.ScheduleEvent(EVENT_SIPHON, 5000);
                    events.ScheduleEvent(EVENT_UMBRAL, 1000);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->SetInCombatWithZone();
                }
        }

    private:
        Creature* GetRandomEgg()
        {
            std::list<Creature*> creatures;
            GetCreatureListWithEntryInGrid(creatures, me, NPC_ALEXSTRASZAS_EGG, 30.0f);
            if (creatures.empty())
                return GetNextEgg();
            return Trinity::Containers::SelectRandomContainerElement(creatures);
      }

        inline Creature* GetNextEgg()
        {
            return me->FindNearestCreature(NPC_ALEXSTRASZAS_EGG, SIZE_OF_GRIDS, true);
        }

        uint64 targetGUID;
        bool isAtAnEgg;
        EventMap events;
        InstanceScript *instance;
        std::map<Waypoints, Position> waypoints;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_facelessAI(creature);
    }
};

class mob_alexstraszas_eggs : public CreatureScript
{
public:
    mob_alexstraszas_eggs() : CreatureScript("mob_alexstraszas_eggs") { }

    struct mob_alexstraszas_eggsAI : public ScriptedAI
    {
        mob_alexstraszas_eggsAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_PASSIVE);
        }

        void DamageTaken(Unit* /*who*/, uint32& damage)
        {
            if (damage < me->GetHealth())
                return;

            DoCast(SPELL_SUMMON_TWILIGHT_HATCHLINGS);
            DoCast(SPELL_SUMMON_TWILIGHT_EGG);
            me->SetVisible(false);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_PASSIVE);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_alexstraszas_eggsAI(creature);
    }
};

class npc_twilight_hatchling : public CreatureScript
{
public:
    npc_twilight_hatchling() : CreatureScript("npc_twilight_hatchling") { }

    struct npc_twilight_hatchlingAI : public ScriptedAI
    {
        npc_twilight_hatchlingAI(Creature* creature) : ScriptedAI(creature){}

        bool bClockwise;
        bool next;
        uint32 uiTimer;
        uint32 uiCheckTimer;
        float x, y, c;

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetDisableGravity(true);
            me->AddAura(SPELL_TWILIGHT_BLAST_PERIODIC, me);
            bClockwise = urand(0, 1);
            next = false;

            float x, y, amount, newx, newy;
            x = me->GetPositionX() - HATCHLING_CENTER_X;
            y = me->GetPositionY() - HATCHLING_CENTER_Y;

            amount = sqrt((x * x) + (y * y));

            newx = HATCHLING_CENTER_X + HATCHLING_RADIUS * x / amount;
            newy = HATCHLING_CENTER_Y + HATCHLING_RADIUS * y / amount;

            me->GetMotionMaster()->MovePoint(1, newx, newy, HATCHLING_HEIGHT);

            c = acos((newx - HATCHLING_CENTER_X) / HATCHLING_RADIUS);

            if (newy - HATCHLING_CENTER_Y < 0)
                c = 2 * M_PI - c;
        }

        void MovementInform(uint32 type, uint32 /*id*/)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            next = true;
        }

        void UpdateAI(const uint32 /*diff*/)
        {
            if (!next)
                return;
            next = false;
            bClockwise = urand(0, 1);
            if (bClockwise)
            {
                c += M_PI / 32;

                if (c >= 2 * M_PI)
                    c = 0;
            }
            else
            {
                c -= M_PI / 32;

                if (c <= -2 * M_PI)
                    c = 0;
            }
            y = HATCHLING_CENTER_Y + HATCHLING_RADIUS * sin(c);
            x = HATCHLING_CENTER_X + HATCHLING_RADIUS * cos(c);

            me->GetMotionMaster()->MovePoint(1, x, y, HATCHLING_HEIGHT);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_twilight_hatchlingAI(creature);
    }

};

class DistanceCheck
{
public:
    explicit DistanceCheck(Unit* _caster) : caster(_caster) { }

    bool operator() (WorldObject* unit) const
    {
        if (caster->GetExactDist2d(unit) <= 4.0f || unit->GetGUID() == caster->GetGUID() || caster->GetEntry() != NPC_SHADOW_GALE_STALKER)
            return true;
        return false;
    }

    Unit* caster;
};


class spell_shadow_gale_damage : public SpellScriptLoader
{
public:
    spell_shadow_gale_damage() : SpellScriptLoader("spell_shadow_gale_damage") { }

    class spell_shadow_gale_damage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_shadow_gale_damage_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targetList)
        {
            targetList.remove_if(DistanceCheck(GetCaster()));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_shadow_gale_damage_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_shadow_gale_damage_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_shadow_gale_damage_SpellScript();
    }
};

void AddSC_boss_erudax()
{
    new boss_erudax();
    new mob_faceless();
    new mob_alexstraszas_eggs();
    new npc_twilight_hatchling();
    new spell_shadow_gale_damage();
}
