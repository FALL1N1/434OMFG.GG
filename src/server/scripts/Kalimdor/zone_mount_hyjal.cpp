#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "Player.h"

enum eNpcs
{
    NPC_CHARRED_INVADER = 52853,
    NPC_CHAINED_INVADER = 52557,
    NPC_LEARA = 52863,
    NPC_MALFURION = 52845,
    NPC_THISALEE = 52907,
    NPC_OLD_BRANCH = 52906,
    NPC_THOLO = 52903,
    NPC_ANREN = 52904,
    NPC_MATOCLAW = 52669,
    // NPC_MALFURION_2 = 52854,
    NPC_RHYOLITH = 53258,
    NPC_RAGING_INVADER = 52557,
    NPC_MALORNE = 53493,

    NPC_LEYARA_EVENT = 53014,
    NPC_HAMUUL_LEYARA_EVENT = 53015,
};

enum archActions
{
    ACTION_START_RITUAL = 1,
    ACTION_START_SANCTUARY_EVENT,
};

enum archEvents
{
    EVENT_START_INFERNAL_WAVES = 1,
    EVENT_SAFE_RESET,
    EVENT_START_LEARA,
    EVENT_START_SACTUARY_EVENT,
    EVENT_STOP_SANCTUARY_EVENT,
    EVENT_SUMMON_ADDS,
    EVENT_SUMMON_MALORNE,
};

enum eQuests
{
    QUEST_RITUAL_OF_FLAME = 29195,
    QUEST_TAKEN_BY_SURPRISE = 29197,
    QUEST_SANCTUARY_MUST_STAY = 29198,
    QUEST_LEYARA = 29200,
};

class npc_archidrud_hamuul : public CreatureScript
{
public:
    npc_archidrud_hamuul() : CreatureScript("npc_archidrud_hamuul") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_archidrud_hamuulAI(creature);
    }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const*_Quest)
    {
        if (_Quest->GetQuestId() == QUEST_RITUAL_OF_FLAME)
            creature->AI()->DoAction(ACTION_START_RITUAL);
        else if (_Quest->GetQuestId() == QUEST_SANCTUARY_MUST_STAY)
        {
            creature->AI()->DoAction(ACTION_START_SANCTUARY_EVENT);
            creature->AI()->SetGUID(player->GetGUID());
            player->KilledMonsterCredit(52845, 0);
        }
        return true;
    }

    struct npc_archidrud_hamuulAI : public ScriptedAI
    {
        npc_archidrud_hamuulAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            isEventInProgress = false;
        }

        void Reset()
        {
        }

        void SetData(uint32 id, uint32 value)
        {

        }

        void DoAction(int32 const param)
        {
            if (isEventInProgress)
                return;

            switch (param)
            {
                case ACTION_START_RITUAL:
                    waveCount = 0;
                    isEventInProgress = true;
                    events.Reset();
                    events.ScheduleEvent(EVENT_START_INFERNAL_WAVES, 0);
                    events.ScheduleEvent(EVENT_SAFE_RESET, 900000);
                    break;
                case ACTION_START_SANCTUARY_EVENT:
                    isEventInProgress = true;
                    events.Reset();
                    events.ScheduleEvent(EVENT_START_SACTUARY_EVENT, 0);
                    break;
            }
        }

        void SetGUID(uint64 guid, int32 /*id*/)
        {
            _plGUID = guid;
        }

        void MovementInform(uint32 /*type*/, uint32 id)
        {
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
        }

        void DamageTaken(Unit* /*done_by*/, uint32 &damage)
        {
            damage = 0;
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/)
        {
            if (summon->GetEntry() == NPC_LEARA)
            {
                summon->AI()->Talk(0);
                isEventInProgress = false;
                TalkWithDelay(5000, 0);
                if (Creature *malfurion = me->FindNearestCreature(NPC_MALFURION, 20))
                    malfurion->AI()->TalkWithDelay(8000, 0);
                summons.DespawnAll();
                events.CancelEvent(EVENT_SAFE_RESET);
            }
        }

        void JustSummoned(Creature* summon)
        {
            switch (summon->GetEntry())
            {
                case NPC_RHYOLITH:
                {
                    summon->SetSpeed(MOVE_WALK, 1.0f);
                    summon->SetSpeed(MOVE_RUN, 1.0f);
                    summon->GetMotionMaster()->MovePoint(0, 4445.4f, -2089.42f, 1206.0f);
                    summon->AI()->TalkWithDelay(5000, 0);
                    summon->AI()->TalkWithDelay(9000, 1);
                    summon->AI()->TalkWithDelay(12000, 2);
                    for (int i = 0; i < 8; i++)
                    {
                        Position spawn1;
                        summon->GetRandomNearPosition(spawn1, 10);
                        me->SummonCreature(NPC_RAGING_INVADER, spawn1, TEMPSUMMON_TIMED_DESPAWN, 20000);
                    }
                    events.ScheduleEvent(EVENT_SUMMON_MALORNE, 15000);
                    break;
                }
                case NPC_MALORNE:
                {
                    if (Creature *rhy = me->FindNearestCreature(NPC_RHYOLITH, 100.0f))
                        summon->AI()->AttackStart(rhy);
                }
            }
            summons.Summon(summon);
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_START_INFERNAL_WAVES:
                        me->SummonCreature(NPC_CHARRED_INVADER, 4605.53f, -2096.02f, 1238.90f, 0.22f, TEMPSUMMON_TIMED_DESPAWN, 8000);
                        me->SummonCreature(NPC_CHARRED_INVADER, 4603.53f, -2094.02f, 1238.90f, 0.22f, TEMPSUMMON_TIMED_DESPAWN, 8000);
                        waveCount++;
                        if (waveCount < 5)
                            events.ScheduleEvent(EVENT_START_INFERNAL_WAVES, 8000);
                        else
                            events.ScheduleEvent(EVENT_START_LEARA, 8000);
                        break;
                    case EVENT_START_LEARA:
                        waveCount = 0;
                        me->SummonCreature(NPC_LEARA, 4605.53f, -2096.02f, 1238.90f, 0.22f);
                        break;
                    case EVENT_SAFE_RESET:
                        summons.DespawnAll();
                        isEventInProgress = false;
                        break;
                    case EVENT_START_SACTUARY_EVENT:
                    {
                        if (Creature *mato = me->FindNearestCreature(NPC_MATOCLAW, 20.0f))
                            if (Creature *malfurion = me->FindNearestCreature(NPC_MALFURION, 20.0f))
                            {
                                malfurion->SetReactState(REACT_PASSIVE);
                                mato->AI()->TalkWithDelay(5000, 0);
                                mato->AI()->TalkWithDelay(10000, 1);
                                malfurion->AI()->TalkWithDelay(15000, 1);
                                malfurion->AI()->TalkWithDelay(20000, 2);
                                TalkWithDelay(25000, 1);
                                mato->AI()->TalkWithDelay(30000, 2);
                                TalkWithDelay(35000, 2);
                                malfurion->AI()->TalkWithDelay(40000, 3);
                                TalkWithDelay(45000, 3);
                            }
                        events.ScheduleEvent(EVENT_STOP_SANCTUARY_EVENT, 55000);
                        events.ScheduleEvent(EVENT_SUMMON_ADDS, 30000);
                        break;
                    }
                    case EVENT_SUMMON_ADDS:
                        me->SummonCreature(NPC_RHYOLITH, 4485.75f, -2064.86f, 1206.1f, 3.58f, TEMPSUMMON_TIMED_DESPAWN, 27000);
                        break;
                    case EVENT_STOP_SANCTUARY_EVENT:
                    {
                        std::list<Player *> _players = me->GetPlayersInRange(20.0f, true);
                        for (std::list<Player *>::iterator itr = _players.begin(); itr != _players.end(); itr++)
                            if ((*itr)->hasQuest(QUEST_SANCTUARY_MUST_STAY))
                                (*itr)->KilledMonsterCredit(52845, 0);
                        if (Creature *malorne = me->FindNearestCreature(NPC_MALORNE, 100.0f))
                            malorne->GetMotionMaster()->MovePoint(0, 4417.0f, -2079.69f, 1210.4f);
                        isEventInProgress = false;
                        break;
                    }
                    case EVENT_SUMMON_MALORNE:
                    {
                        Position pos;
                        me->GetPosition(&pos);
                        me->SummonCreature(NPC_MALORNE, 4440.23f, -2089.86f, 1205.8f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 33000);
                        break;
                    }
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
        uint8 waveCount;
        bool isEventInProgress;
        SummonList summons;
        uint64 _plGUID;
    };
};

// 52907
class npcs_taken_by_surprise : public CreatureScript
{
public:
    npcs_taken_by_surprise() : CreatureScript("npcs_taken_by_surprise") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npcs_taken_by_surpriseAI(creature);
    }

    struct npcs_taken_by_surpriseAI : public ScriptedAI
    {
        npcs_taken_by_surpriseAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            canHello = true;
        }

        void Reset()
        {
            events.Reset();
            if (me->GetPhaseMask() == 512)
                events.ScheduleEvent(EVENT_START_INFERNAL_WAVES, 0);
        }

        void DamageTaken(Unit* /*done_by*/, uint32 &damage)
        {
            damage = 0;
        }

        void SummonedCreatureDies(Creature* summon, Unit* killer)
        {
            if (me->GetPhaseMask() != 512)
                return;

            if (summon->GetEntry() == NPC_CHAINED_INVADER)
                if (Player *player = killer->ToPlayer())
                {
                    player->KilledMonsterCredit(me->GetEntry(), 0);
                    killedCount[player->GetGUID()] += 1;
                    if (killedCount[player->GetGUID()] == 6)
                    {
                        if (me->GetEntry() != NPC_THOLO)
                            Talk(1, killer->GetGUID());
                        else
                        {
                            if (Creature *anren = me->FindNearestCreature(NPC_ANREN, 5.0f))
                                anren->AI()->Talk(1);
                            TalkWithDelay(2000, 2);
                        }
                    }
                }
        }

        void JustSummoned(Creature* summon)
        {
            AttackStart(summon);
            summons.Summon(summon);
        }

        void MoveInLineOfSight(Unit* unit)
        {
            if (me->GetPhaseMask() != 512)
                return;

            if (!canHello)
                return;

            if (unit->GetTypeId() != TYPEID_PLAYER)
                return;

            if (killedCount[unit->GetGUID()] == 6)
                return;

            if (me->GetDistance2d(unit->GetPositionX(), unit->GetPositionY()) <= 10.0f)
            {
                if (me->GetEntry() != NPC_THOLO)
                    Talk(0, unit->GetGUID());
                else
                {
                    Talk(0, unit->GetGUID());
                    if (Creature *anren = me->FindNearestCreature(NPC_ANREN, 5.0f))
                        anren->AI()->TalkWithDelay(3000, 0);
                    TalkWithDelay(6000, 1);
                }
                canHello = false;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_START_INFERNAL_WAVES:
                    {
                        Position spawn1, spawn2;
                        me->GetRandomNearPosition(spawn1, 10);
                        me->GetRandomNearPosition(spawn2, 15);
                        me->SummonCreature(NPC_CHAINED_INVADER, spawn1, TEMPSUMMON_TIMED_DESPAWN, 20000);
                        me->SummonCreature(NPC_CHAINED_INVADER, spawn2, TEMPSUMMON_TIMED_DESPAWN, 20000);
                        events.ScheduleEvent(EVENT_START_INFERNAL_WAVES, 20000);
                        break;
                    }
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
        SummonList summons;
        float canHello;
        std::map<uint64, uint32> killedCount;
    };
};

// 93593
class spell_tortolla_save_turtle : public SpellScriptLoader
{
public:
    spell_tortolla_save_turtle() : SpellScriptLoader("spell_tortolla_save_turtle") { }

    class spell_tortolla_save_turtle_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_tortolla_save_turtle_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            if (Unit *target = GetCaster())
            {
                if (Player *player = target->ToPlayer())
                    player->KilledMonsterCredit(52177, 0);
                else if (target->ToTempSummon())
                    if (Unit *sumoner = target->ToTempSummon()->GetSummoner())
                        if (Player *player = sumoner->ToPlayer())
                            player->KilledMonsterCredit(52177, 0);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_tortolla_save_turtle_SpellScript::HandleScriptEffect, EFFECT_2, SPELL_EFFECT_TRIGGER_SPELL);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_tortolla_save_turtle_SpellScript();
    }
};

enum leyaraEventSpells
{
    // LEYARA
    SPELL_SMOLDERING_ROOTS = 98064,
    SPELL_SMOLDERING_ROOTS2 = 98078,
    SPELL_BACKDRAFT = 98972,
    SPELL_SHIFT_FLY_FORM = 97723,
    SPELL_BURN = 98049,

    // HAMUUL
    SPELL_HAMUUL_WRATH = 98980,
};

class npc_hyjal_leyara : public CreatureScript
{
public:
    npc_hyjal_leyara() : CreatureScript("npc_hyjal_leyara") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_hyjal_leyaraAI(creature);
    }

    struct npc_hyjal_leyaraAI : public ScriptedAI
    {
        npc_hyjal_leyaraAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            isEventInProgress = false;
            eventStep = 0;
        }

        void Reset()
        {
            me->SetVisible(true);
            events.Reset();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }

        void DamageTaken(Unit* /*done_by*/, uint32 &damage)
        {
            damage = 0;
        }

        void MoveInLineOfSight(Unit* unit)
        {
            if (isEventInProgress || unit->GetTypeId() != TYPEID_PLAYER)
                return;

            if (Player *player = unit->ToPlayer())
                if (player->GetQuestStatus(QUEST_LEYARA) != QUEST_STATUS_INCOMPLETE)
                    return;

            if (me->GetDistance2d(unit->GetPositionX(), unit->GetPositionY()) <= 10.0f)
            {
                isEventInProgress = true;
                eventStep = 0;
                _playerGUID = unit->GetGUID();
                unit->NearTeleportTo(5237.92f, -1212.21f, 1377.0f, 4.04f);
                me->CastSpell(unit, SPELL_SMOLDERING_ROOTS, false);
                events.ScheduleEvent(EVENT_START_LEARA, 1000);
            }
        }

        void JustSummoned(Creature* summon)
        {
            switch (summon->GetEntry())
            {
                case NPC_HAMUUL_LEYARA_EVENT:
                    summon->SetReactState(REACT_PASSIVE);
                    summon->NearTeleportTo(5228.96f, -1226.64f, 1375.70f, 1.11f);
                    _billiGUID = summon->GetGUID();
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
                    case EVENT_START_LEARA:
                    {
                        uint32 nextEventTimer = 8000;
                        switch (eventStep)
                        {
                            case 0:
                                if (Creature *billi = me->FindNearestCreature(NPC_HAMUUL_LEYARA_EVENT, 50.0f))
                                    JustSummoned(billi);
                                Talk(0);
                                break;
                            case 1:
                                if (Creature *billi = Unit::GetCreature(*me, _billiGUID))
                                    billi->AI()->Talk(0);
                                break;
                            case 2:
                                if (Creature *billi = Unit::GetCreature(*me, _billiGUID))
                                {
                                    billi->AI()->Talk(1);
                                    billi->CastSpell(me, SPELL_HAMUUL_WRATH, false);
                                }
                                break;
                            case 3:
                                Talk(1);
                                nextEventTimer = 2000;
                                break;
                            case 4:
                                if (Creature *billi = Unit::GetCreature(*me, _billiGUID))
                                {
                                    me->CastSpell(billi, SPELL_SMOLDERING_ROOTS2, false);
                                    billi->AI()->Talk(2);
                                }
                                break;
                            case 5:
                                if (Creature *billi = Unit::GetCreature(*me, _billiGUID))
                                {
                                    me->CastSpell(billi, SPELL_BURN, false);
                                    billi->CastSpell(billi, 29266, true);
                                }
                                Talk(2);
                                nextEventTimer = 3000;
                                break;
                            case 6:
                                if (Creature *billi = Unit::GetCreature(*me, _billiGUID))
                                    me->CastSpell(billi, SPELL_BURN, false);
                                Talk(3);
                                break;
                            case 7:
                                Talk(4);
                                nextEventTimer = 23000;
                                break;
                            case 8:
                                if (Player *player = Unit::GetPlayer(*me, _playerGUID))
                                    player->KilledMonsterCredit(me->GetEntry(), 0);
                                me->CastSpell(me, SPELL_SHIFT_FLY_FORM, false);
                                Talk(5);
                                break;
                            case 9:
                                me->SetVisible(false);
                                break;
                            case 10:
                                break;
                            case 11:
                                break;
                            default:
                                eventStep = 0;
                                isEventInProgress = false;
                                me->RemoveAura(SPELL_SHIFT_FLY_FORM);
                                me->SetVisible(true);
                                return;
                        }
                        eventStep++;
                        events.ScheduleEvent(EVENT_START_LEARA, 8000);
                        break;
                    }
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
        SummonList summons;
        bool isEventInProgress;
        uint8 eventStep;
        uint64 _playerGUID, _billiGUID;
    };
};

enum BounceEvent
{
    EVENT_BOUNCE_PLAYERS                = 1,
    SPELL_TRAMPOLINE_BOUNCE             = 79024,
    SPELL_TRAMPOLINE_BOUNCE_2           = 79040,
    SPELL_TRAMPOLINE_BOUNCE_3           = 79043,
    SPELL_TRAMPOLINE_BOUNCE_4           = 79044,
    SPELL_TRAMPOLINE_BOUNCE_5           = 79046,
    SPELL_BOUNCE_ACHIEVEMENT_TRIGGER    = 95531,
    SPELL_BOUNCE_ACHIEVEMENT_AURA       = 95529,
};

class npc_bounce_controller : public CreatureScript
{
public:
    npc_bounce_controller() : CreatureScript("npc_bounce_controller") { }

    struct npc_bounce_controllerAI : public ScriptedAI
    {
        npc_bounce_controllerAI(Creature* creature) : ScriptedAI(creature) {}

        void InitializeAI() override
        {
            events.ScheduleEvent(EVENT_BOUNCE_PLAYERS, 1000);
            ScriptedAI::InitializeAI();
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_BOUNCE_PLAYERS:
                        std::list<Player *> players = me->GetPlayersInRange(3.0f, true);
                        for (Player* player : players)
                        {
                            if (!player->IsFalling())
                            {
                                DoCast(player, SPELL_BOUNCE_ACHIEVEMENT_TRIGGER, true);
                                DoCast(player, RAND(SPELL_TRAMPOLINE_BOUNCE, SPELL_TRAMPOLINE_BOUNCE_2, SPELL_TRAMPOLINE_BOUNCE_3, SPELL_TRAMPOLINE_BOUNCE_4, SPELL_TRAMPOLINE_BOUNCE_5), true);
                            }
                        }
                        events.ScheduleEvent(EVENT_BOUNCE_PLAYERS, 1500);
                        break;
                }
            }
        }

    private:
        EventMap events;
    };
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bounce_controllerAI(creature);
    }
};

class spell_bounce_achievement_aura : public SpellScriptLoader
{
public:
    spell_bounce_achievement_aura() : SpellScriptLoader("spell_bounce_achievement_aura") { }

    class spell_bounce_achievement_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_bounce_achievement_aura_AuraScript);

        void Remove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
                return;
            GetTarget()->ToPlayer()->ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, ACHIEVEMENT_CRITERIA_CONDITION_NONE, 95529, true);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_bounce_achievement_aura_AuraScript::Remove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_bounce_achievement_aura_AuraScript();
    }
};


class npc_lycanthoth : public CreatureScript
{
public:
	npc_lycanthoth() : CreatureScript("npc_lycanthoth") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_lycanthothAI(creature);
	}

	struct npc_lycanthothAI : public ScriptedAI
	{
		npc_lycanthothAI(Creature* creature) : ScriptedAI(creature)
		{
		}


		void JustDied(Unit* killer)
		{
			Unit* originalKiller = killer->GetCharmerOrOwnerOrSelf();
			if (originalKiller && (originalKiller->GetTypeId() == TYPEID_PLAYER))
				killer->CastSpell(killer, (originalKiller->ToPlayer()->GetTeam() == TEAM_HORDE ? 74077 : 74078), true);
		}
	};
};

class npc_marion_wormswing : public CreatureScript
{
public:
	npc_marion_wormswing() : CreatureScript("npc_marion_wormswing") { }

	CreatureAI* GetAI(Creature* pCreature) const
	{
		return new npc_marion_wormswingAI(pCreature);
	}

	struct npc_marion_wormswingAI : public ScriptedAI
	{
		npc_marion_wormswingAI(Creature* pCreature) : ScriptedAI(pCreature)
		{
		}

		void DamageTaken(Unit* attacker, uint32 &damage)
		{
			if (damage > me->GetHealth() || me->HealthBelowPct(50) || me->HealthBelowPctDamaged(50, damage))
			{
				if (Player* owner = attacker->GetCharmerOrOwnerPlayerOrPlayerItself())
					if (owner->GetQuestStatus(25731) == QUEST_STATUS_INCOMPLETE)
					{
						owner->KilledMonsterCredit(41169, 0);
						owner->KilledMonsterCredit(41170, 0);
					}
				me->setFaction(35);
				me->DespawnOrUnsummon(5000);
			}
		}
	};
};

class go_harpy_signal_fire : public GameObjectScript
{
public:
	go_harpy_signal_fire() : GameObjectScript("go_harpy_signal_fire") {}

	bool OnGossipHello(Player* pPlayer, GameObject* pGo)
	{
		if (!pGo->FindNearestCreature(41112, 100.0f) && pPlayer->GetQuestStatus(25731) == QUEST_STATUS_INCOMPLETE)
			pGo->SummonCreature(41112, pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), pPlayer->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 60000);
		return false;
	}
};

void AddSC_mount_hyjal()
{
    new npc_archidrud_hamuul();
    new npcs_taken_by_surprise();
    new spell_tortolla_save_turtle();
    new npc_hyjal_leyara();
    new npc_bounce_controller();
    new spell_bounce_achievement_aura();
	new npc_lycanthoth();
	new npc_marion_wormswing();
	new go_harpy_signal_fire();
}

