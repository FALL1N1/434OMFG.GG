/*
 * Copyright (C) 2013-2015 OMFG.GG Network <http://www.omfg.gg/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"
#include "well_of_eternity.h"

enum Talks
{
    TALK_ILLIDAN_CLOSE_PORTAL               = -54103,
    TALK_MALFURION_IT_IS_MAINTAINED         = -54106,
    TALK_MALFURION_CANNOT_BREAK             = -54107,
    TALK_ILLIDAN_VERY_WELL                  = -54104,
    TALK_TYRANDE_HE_KNOWS_WHAT              = -54108,
    TALK_ILLIDAN_LET_THEM_COME              = -54105,
    TALK_TYRANDE_MOTHER_MOON                = -54110,
    TALK_ILLIDAN_WEAK_PITIFUL               = -54111,
    TALK_ILLIDAN_THIS_WILL_BE_FUN           = -54112,
    TALK_ILLIDAN_WAIT_I_HAVE_AN_IDEA        = -54113,
    TALK_TYRANDE_ILLIDAN_WHAT_IS_IN         = -54114,
    TALK_ILLIDAN_WHAT_OUR_PEOPLE            = -54116,
    TALK_ILLIDAN_EMOTE_DRINK                = -54155,
    TALK_ILLIDAN_YES_YES_I_CAN_FEEL         = -54117,
    TALK_TYRANDE_EMOTE_BLESSING             = -54153,
    TALK_TYRANDE_EMOTE_BLESSING_BOSS        = -54233,
    TALK_MANNOROTH_INTRO                    = -54118,
    TALK_CAPTAIN_INTRO                      = -54119,
    TALK_CAPTAIN_AGGRO                      = -54225,
    TALK_MONNOROTH_AGGRO                    = -54223,
    TALK_TYRANDE_LIGHT_OF_ELUNE_SAVE        = -54229,
    TALK_MANNOROTH_EMOTE_FIRESTORM          = -48107,
    TALK_TYRANDE_I_WILL_HANDLE_DEMON        = -54227,
    TALK_TYRANDE_EMOTE_CAN_HOLD_HER         = -54236,
    TALK_TYRANDE_HOLD_THEM_BACK_AGAIN       = -54231,
    TALK_CAPTAIN_DEATH                      = -54253,
    TALK_EMOTE_SWORD_FALLS_TO_THE_GROUND    = -54255,
    TALK_MONNOROTH_RRRAAAGHHH               = -54555,
    TALK_ILLIDAN_THE_SWORD_HAS_PIERCED      = -54254,
    TALK_MONNOROTH_NETHER_TEAR              = -54256,
    TALK_EMOTE_PORTAL_OPEN                  = -54260,
    TALK_ILLIDAN_HE_IS_STILL_CONNECTED      = -54257,
    TALK_TYRANDE_I_AM_OUT_OF_ARROWS         = -54258,
    TALK_TYRANDE_EMOTE_HAND_OF_ELUNE        = -54268,
    TALK_EMOTE_FELHOUND_SUMMON              = -54605,
    TALK_EMOTE_FELGUARD_SUMMON              = -54606,
    TALK_EMOTE_DOOMGUARD_SUMMON             = -54607,
    TALK_MANNOROTH_INFERNO                  = -54259,
    TALK_MANNOROTH_EMOTE_INFERNAL           = -54608,
    TALK_TYRANDE_THERE_ARE_TOO_MANY         = -54609,
    TALK_TYRANDE_EMOTE_COLLAPSE             = -54263,
    TALK_ILLIDAN_YOU_ARE_NOT_THE_SOLE       = -54261,
    TALK_TYRANDE_ILLIDAN_YOU_MUSNT          = -54262,
    TALK_ILLIDAN_I_WILL_BE_THE_SAVIOR       = -54264,
    TALK_TYRANDE_NO_ILLIDAN                 = -54265,
    TALK_ILLIDAN_DEMONIC                    = -54266,
    TALK_TYRANDE_COMBAT_DONE                = -54270,
};

enum Spells
{
    // Mannoroth
    SPELL_PORTAL_TO_THE_NETHER      = 102920,
    SPELL_FEL_FIRESTORM             = 103888,
    SPELL_FEL_FIRESTORM_PERIODIC    = 103894,
    SPELL_FELBLADE                  = 103966,
    SPELL_FEL_DRAIN                 = 104961,
    SPELL_NETHER_PORTAL             = 104625,
    SPELL_NETHER_PORTAL_TRIGGERED   = 104648,
    SPELL_NETHER_TEAR               = 105041,
    SPELL_SUMMON_INFERNO            = 105141,
    SPELL_EMBEDDED_BLADE            = 104820,
    SPELL_ACHIEVEMENT_SPELL         = 105576,
    SPELL_FINALE_ANIMKIT            = 105422,
    SPELL_PORTAL_PULL               = 105334,
    SPELL_FEL_FIRE_NOVA             = 105093,
    SPELL_MAGISTRIKE_ARC_DAMAGE     = 105523,
    SPELL_MAGISTRIKE_ARC_VISUAL     = 104822,

    // Illidan
    SPELL_WATERS_OF_ETERNITY        = 103952,
    SPELL_DEMON_RUSH                = 104205,
    SPELL_DEMONIC_SIGHT             = 104746,
    SPELL_DARKLANCE                 = 104394,
    SPELL_AURA_OF_IMMOLATION        = 104379,
    SPELL_GIFT_OF_SARGERAS          = 104998,
    SPELL_GIFT_OF_SARGERAS_PLAYER   = 105009,
    SPELL_DEMONIC_TAUNT             = 104461,

    // Tyrande Whisperwind
    SPELL_LUNAR_SHOT_DOOMBRINGER    = 104214,
    SPELL_LUNAR_SHOT_INSTANT        = 104313,
    SPELL_BLESSING_OF_ELUNE         = 103917,
    SPELL_ELUNES_WRATH              = 103919,
    SPELL_HAND_OF_ELUNE             = 105072,
    SPELL_COLLAPSE                  = 96733,  // correct spell missing

    // Abyssal Doombringer
    SPELL_ABYSSAL_FLAMETHROWER      = 105218, // target 55091
    SPELL_ABYSSAL_FLAMES            = 103992,

    // Shadowbat
    SPELL_RIDE_VEHICLE_HARDCODED    = 46598,
    SPELL_DISPLACEMENT              = 103763,

    // Captain
    SPELL_MAGISTRIKE                = 103669,
    SPEL_VAROTHENS_MAGICAL_BLADE    = 104815,

    // Dreadlord Debilitator
    SPELL_DEBILITATING_FLAY         = 104678,
    SPELL_COSMETIC_PURPLE_STATE     = 83237,
    SPELL_COSMETIC_SUMMON_EFFECT    = 104672,

    // Fel Flames
    SPELL_FEL_FLAMES_PERIODIC       = 103892,

    // Blade
    SPELL_BLADE_VISUAL              = 104823,

    // Spellclick Blade
    SPELL_MAGICAL_BLADE_TRIGGER     = 104817,
    SPELL_MAGICAL_BLADE_VISUAL      = 104819,

    // Tear Portal
    SPELL_SUMMON_FELHOUND_PERIODIC  = 105053,
    SPELL_SUMMON_FELGUARD_PERIODIC  = 105057,
    SPELL_SUMMON_DOOMGUARD_PERIODIC = 105061,
};

enum Events
{
    // Mannoroth
    EVENT_FEL_FIRESTORM         = 1,
    EVENT_FELBLADE              = 2,
    EVENT_NETHER_PORTAL         = 3,
    EVENT_NETHER_TEAR           = 4,
    EVENT_NETHER_POTAL          = 5,
    EVENT_FEL_FIRE_NOVA         = 6,
    EVENT_SUMMON_DOOMGUARD      = 7,

    // Illidan
    EVENT_CHECK_DEMONS          = 1,
    EVENT_DRINK                 = 2,
    EVENT_ATTACK_DOOMBRINGER    = 3,
    EVENT_PLAYER_ALIVE_CHECK    = 4,
    EVENT_AURA_OF_IMMOLATION    = 5,
    EVENT_TAUNT                 = 6,

    // Tyramde Whisperwind
    EVENT_START_ESCORT          = 1,
    EVENT_BLESSING_OF_ELUNE     = 2,
    EVENT_SEND_EMOTE            = 4,
    EVENT_HAND_OF_ELUNE         = 5,
    EVENT_COLLAPSE              = 6,
    EVENT_ATTACK_DOOMGUARD      = 7,

    // Abyssal Doombringer
    EVENT_ABYSSAL_FLAMES        = 1,
    EVENT_ABYSSAL_FLAMETHROWER  = 2,

    // Captain Verothen
    EVENT_INTRO                 = 1,
    EVENT_MAGISTRIKE            = 2,
};

enum Actions
{
    ACTION_DONT_CALL_FOR_HELP       = 1,
    ACTION_START_ESCORT_EVENT       = 2,
    ACTION_RESTART_ESCORT           = 3,
    ACTION_ATTACK_DOOMBRINGER       = 4,
    ACTION_CAST_BLESSING_OF_ELUNE   = 5,
    ACTION_HIGHGUARD_DIED           = 6,
    ACTION_START_INTRO              = 7,
    ACTION_LIGHT_OF_ELUNE           = 8,
    ACTION_RESTART_COMBAT           = 9,
    ACTION_RESET_BOSS_FIGHT         = 10,
    ACTION_HAND_OF_ELUNE            = 11,
    ACTION_COLLAPSE                 = 12,
    ACTION_ILLIDAN_START_PHASE_3    = 13,
    ACTION_DISABLE_DAMAGE_COUNTER   = 14,
    ACTION_FIGHT_FINISHED           = 15,
};

const Position highguardElitePos[2] = 
{
    { 3298.469f, -5721.270f, 18.899f, 2.3561f },
    { 3310.100f, -5694.490f, 18.070f, 2.9321f }
};

const Position doomguardDevastatorPos = { 3426.192f, -5462.206f, 26.174f, 5.5580f };
const Position doomguardDevastatorLandingPos[5] =
{
    { 3288.266f, -5669.797f, 11.758f, 4.5082f },
    { 3294.253f, -5674.673f, 12.054f, 4.5082f },
    { 3306.797f, -5677.270f, 12.785f, 4.5082f },
    { 3299.251f, -5687.083f, 14.363f, 4.5082f },
    { 3280.885f, -5686.088f, 14.380f, 4.5082f }
};

#define POINT_LADING    1
#define QUEST_THE_PATH_TO_THE_DRAGON_SOUL 30101
#define DATA_THATS_NOT_CANON 1
#define CRITERIA_GUILD_RUN_WELL_OF_ETERNITY 18476

class doomguardSummonEvent : public BasicEvent
{
public:
    explicit doomguardSummonEvent(Creature* owner) : _owner(owner) {}

    bool Execute(uint64 /*time*/, uint32 /*diff*/)
    {
        for (int8 i = 0; i < 2; i++)
        {
            if (Creature* doomguard = _owner->SummonCreature(NPC_DOOMGUARD_DEVASTATOR, doomguardDevastatorPos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000))
            {
                int8 pos = urand(0, 4);
                doomguard->GetMotionMaster()->MovePoint(0, doomguardDevastatorLandingPos[pos], false);
            }
        }

        _owner->m_Events.AddEvent(this, _owner->m_Events.CalculateTime(urand(2500, 5000)));
        return false;
    }

private:
    Creature* _owner;
};

class boss_woe_mannoroth : public CreatureScript
{
public:
    boss_woe_mannoroth() : CreatureScript("boss_woe_mannoroth") {}

    struct boss_woe_mannorothAI : public BossAI
    {
        boss_woe_mannorothAI(Creature* creature) : BossAI(creature, DATA_MANNOROTH)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        }

        void Reset() override
        {
            DoCast(me, SPELL_PORTAL_TO_THE_NETHER, false);
            RemoveEncounterFrame();
            aliveDemons = 0;
            felstormCounter = 0;
            damageReceived = 0;
            canDamageCount = true;
            phase85PctTriggered = false;
            phase70PctTriggered = false;
            phase60PctTriggered = false;
            phase50PctTriggered = false;
            _Reset();
        }

        void EnterEvadeMode()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
            _EnterEvadeMode();
            BossAI::EnterEvadeMode();
        }

        void DamageTaken(Unit* attacker, uint32& damage)
        {
            if (canDamageCount)
            {
                damageReceived += damage;
                if (damageReceived >= 10000000)
                {
                    canDamageCount = false;
                    if (Creature* captain = instance->GetCreature(DATA_CAPTAIN_VAROTHEN))
                        if (captain->isAlive())
                            DoCast(captain, SPELL_FEL_DRAIN, true);
                }
            }

            if (!phase85PctTriggered && me->HealthBelowPctDamaged(86, damage))
            {
                phase85PctTriggered = true;
                Talk(TALK_MONNOROTH_NETHER_TEAR);
                DoCast(me, SPELL_NETHER_TEAR, false);
                if (Creature* tyrande = instance->GetCreature(DATA_TYRANDE_WHISPERWIND))
                    tyrande->AI()->DoAction(ACTION_HAND_OF_ELUNE);

                if (Creature* illidan = instance->GetCreature(DATA_ILLIDAN_PART_TWO))
                    illidan->AI()->TalkWithDelay(16000, TALK_ILLIDAN_HE_IS_STILL_CONNECTED);
            }
            else if (!phase70PctTriggered && me->HealthBelowPctDamaged(71, damage))
            {
                phase70PctTriggered = true;
                Talk(TALK_EMOTE_FELGUARD_SUMMON, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                Talk(TALK_MANNOROTH_EMOTE_FIRESTORM, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                if (Creature* portal = instance->GetCreature(DATA_PORTAL))
                    portal->AddAura(SPELL_SUMMON_FELGUARD_PERIODIC, portal);
            }
            else if (!phase60PctTriggered && me->HealthBelowPctDamaged(61, damage))
            {
                phase60PctTriggered = true;
                Talk(TALK_EMOTE_DOOMGUARD_SUMMON, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                if (Creature* portal = instance->GetCreature(DATA_PORTAL))
                    portal->AddAura(SPELL_SUMMON_DOOMGUARD_PERIODIC, portal);
            }
            else if (!phase50PctTriggered && me->HealthBelowPctDamaged(51, damage))
            {
                phase50PctTriggered = true;
                Talk(TALK_MANNOROTH_INFERNO);
                TalkWithDelay(14000, TALK_MANNOROTH_EMOTE_INFERNAL, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                DoCast(me, SPELL_SUMMON_INFERNO, false);

                if (Creature* tyrande = instance->GetCreature(DATA_TYRANDE_WHISPERWIND))
                {
                    tyrande->AI()->DoAction(ACTION_COLLAPSE);
                    tyrande->AI()->TalkWithDelay(20000, TALK_TYRANDE_THERE_ARE_TOO_MANY);
                    tyrande->AI()->TalkWithDelay(22000, TALK_TYRANDE_EMOTE_COLLAPSE, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                    tyrande->AI()->TalkWithDelay(32000, TALK_TYRANDE_ILLIDAN_YOU_MUSNT);
                    tyrande->AI()->TalkWithDelay(44000, TALK_TYRANDE_NO_ILLIDAN);
                }

                if (Creature* illidan = instance->GetCreature(DATA_ILLIDAN_PART_TWO))
                {
                    illidan->CastWithDelay(27000, illidan, SPELL_GIFT_OF_SARGERAS, false);
                    illidan->CastWithDelay(57000, illidan, SPELL_GIFT_OF_SARGERAS_PLAYER, true);
                    illidan->AI()->TalkWithDelay(27000, TALK_ILLIDAN_YOU_ARE_NOT_THE_SOLE);
                    illidan->AI()->TalkWithDelay(35000, TALK_ILLIDAN_I_WILL_BE_THE_SAVIOR);
                    illidan->AI()->TalkWithDelay(46000, TALK_ILLIDAN_DEMONIC);
                }
            }
            else if (me->HealthBelowPctDamaged(25, damage))
            {
                DoCastAOE(SPELL_ACHIEVEMENT_SPELL, true);
                instance->CompleteGuildCriteriaForGuildGroup(CRITERIA_GUILD_RUN_WELL_OF_ETERNITY);
                events.Reset();
                me->RemoveAllAuras();
                if (Creature* portal = instance->GetCreature(DATA_PORTAL))
                    portal->RemoveAllAuras();

                if (Creature* illidan = instance->GetCreature(DATA_ILLIDAN_PART_TWO))
                    illidan->AI()->DoAction(ACTION_FIGHT_FINISHED);

                if (Creature* tyrande = instance->GetCreature(DATA_TYRANDE_WHISPERWIND))
                    tyrande->AI()->DoAction(ACTION_FIGHT_FINISHED);

                if (Creature* chromie = instance->GetCreature(DATA_CHROMIE))
                    chromie->SetVisible(true);

                if (GameObject* cache = instance->GetGameObject(DATA_MANNOROTH_CACHE))
                    cache->SetRespawnTime(cache->GetRespawnDelay());

                Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();
                for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                    if (Player* player = itr->getSource())
                        if (player->GetQuestStatus(QUEST_THE_PATH_TO_THE_DRAGON_SOUL) == QUEST_STATUS_INCOMPLETE)
                            player->KilledMonsterCredit(me->GetEntry());

                me->Kill(me);
                me->DespawnOrUnsummon(5000);
                instance->FinishLfgDungeon(me);
                RemoveEncounterFrame();
                _JustDied();
            }
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_DISABLE_DAMAGE_COUNTER)
                canDamageCount = false;
        }

        void EnterCombat(Unit* victim) override
        {
            AddEncounterFrame();
            me->InterruptSpell(CURRENT_CHANNELED_SPELL);
            Talk(TALK_MONNOROTH_AGGRO);
            if (Creature* tyrande = instance->GetCreature(DATA_TYRANDE_WHISPERWIND))
                tyrande->AI()->TalkWithDelay(12000, TALK_TYRANDE_I_WILL_HANDLE_DEMON);
            events.ScheduleEvent(EVENT_FEL_FIRESTORM, 15000);
            events.ScheduleEvent(EVENT_FELBLADE, 30000);
            events.ScheduleEvent(EVENT_NETHER_POTAL, 71000);
            events.ScheduleEvent(EVENT_SUMMON_DOOMGUARD, 1000);
            me->CastWithDelay(2000, me, SPELL_FELBLADE);
            if (Creature* portal = instance->GetCreature(DATA_DOOMGUARD_PORTAL))
                portal->m_Events.AddEvent(new doomguardSummonEvent(portal), portal->m_Events.CalculateTime(1000));
            _EnterCombat();
        }

        uint32 GetData(uint32 data) const override
        {
            if (data == DATA_THATS_NOT_CANON)
                return damageReceived;
            return 0;
        }

        void JustSummoned(Creature* summon) override
        {
            switch (summon->GetEntry())
            {
                case NPC_FEL_FLAME:
                    summon->CastWithDelay(750, summon, SPELL_FEL_FLAMES_PERIODIC, true);
                    break;
                case NPC_DREADLORD_DEBILITATOR:
                    aliveDemons++;
                    summon->SetReactState(REACT_PASSIVE);
                    summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                    summon->CastSpell(summon, SPELL_COSMETIC_PURPLE_STATE, true);
                    summon->CastSpell(summon, SPELL_COSMETIC_SUMMON_EFFECT, true);
                    if (Creature* tyrande = instance->GetCreature(DATA_TYRANDE_WHISPERWIND))
                    {
                        summon->SetFacingToObject(tyrande);
                        tyrande->AI()->DoAction(ACTION_LIGHT_OF_ELUNE);
                        summon->CastSpell(tyrande, SPELL_DEBILITATING_FLAY, false);
                    }
                    break;
                case NPC_EMBEDDED_BLADE:
                    summon->CastSpell(me, SPELL_RIDE_VEHICLE_HARDCODED, true);
                    break;
                default:
                    break;
            }
            BossAI::JustSummoned(summon);
        }

        void SpellHit(Unit* /*attacker*/, SpellInfo const* spellInfo)
        {
            if (spellInfo->Id == SPELL_MAGICAL_BLADE_TRIGGER)
            {
                Talk(TALK_MONNOROTH_RRRAAAGHHH);
                DoCast(me, SPELL_EMBEDDED_BLADE, true);
                events.CancelEvent(EVENT_FEL_FIRESTORM);
                events.CancelEvent(EVENT_FELBLADE);
                events.ScheduleEvent(EVENT_FEL_FIRE_NOVA, 5000);
                Position pos(*me);
                me->SummonCreature(NPC_EMBEDDED_BLADE, pos);
                if (Creature* illidan = instance->GetCreature(DATA_ILLIDAN_PART_TWO))
                    illidan->AI()->TalkWithDelay(5000, TALK_ILLIDAN_THE_SWORD_HAS_PIERCED);
                if (Creature* portal = instance->GetCreature(DATA_DOOMGUARD_PORTAL))
                    portal->m_Events.KillAllEvents(false);
            }
        }

        void SpellHitTarget(Unit* target, SpellInfo const* spellInfo)
        {
            if (spellInfo->Id == SPELL_NETHER_TEAR)
            {
                Talk(TALK_EMOTE_PORTAL_OPEN, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                Talk(TALK_EMOTE_FELHOUND_SUMMON, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                target->AddAura(SPELL_SUMMON_FELHOUND_PERIODIC, target);
            }
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            switch (summon->GetEntry())
            {
                case NPC_DREADLORD_DEBILITATOR:
                    if (--aliveDemons <= 0)
                        if (Creature* tyrande = instance->GetCreature(DATA_TYRANDE_WHISPERWIND))
                            tyrande->AI()->DoAction(ACTION_RESTART_COMBAT);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 const diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_FEL_FIRESTORM:
                        Talk(TALK_MANNOROTH_EMOTE_FIRESTORM, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                        DoCast(me, SPELL_FEL_FIRESTORM, false);
                        me->CastWithDelay(3000, me, SPELL_FEL_FIRESTORM_PERIODIC, true);
                        if (++felstormCounter <= 2)
                            events.ScheduleEvent(EVENT_FEL_FIRESTORM, 39000);
                        break;
                    case EVENT_FELBLADE:
                        DoCast(me, SPELL_FELBLADE, false);
                        events.ScheduleEvent(EVENT_FELBLADE, 30000);
                        break;
                    case EVENT_NETHER_POTAL:
                        DoCast(me, SPELL_NETHER_PORTAL, false);
                        break;
                    case EVENT_FEL_FIRE_NOVA:
                        DoCastAOE(SPELL_FEL_FIRE_NOVA);
                        events.ScheduleEvent(EVENT_FEL_FIRE_NOVA, urand(5000, 6000));
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        int8 aliveDemons;
        uint8 felstormCounter;
        uint32 damageReceived;
        bool phase85PctTriggered;
        bool phase70PctTriggered;
        bool phase60PctTriggered;
        bool phase50PctTriggered;
        bool canDamageCount;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_woe_mannorothAI(creature);
    }
};

class npc_woe_illidan_part_two : public CreatureScript
{
public:
    npc_woe_illidan_part_two() : CreatureScript("npc_woe_illidan_part_two") { }

    struct npc_woe_illidan_part_twoAI : public npc_escortAI
    {
        npc_woe_illidan_part_twoAI(Creature* creature) : npc_escortAI(creature)
        {
            isEventActive = false;
            restartEscortAtHome = true;
            instance = me->GetInstanceScript();
        }

        bool CanAIAttack(Unit const* target) const override
        {
            return target->GetEntry() != NPC_CAPTAIN_VAROTHEN;
        }

        void Reset()
        {
            DoCast(me, SPELL_DEMONIC_SIGHT, true);
            events.Reset();
        }

        void ResetCombat()
        {
            events.Reset();
            me->SetReactState(REACT_PASSIVE);
            me->RemoveAllAuras();
            me->CombatStop(true);
            me->m_Events.KillAllEvents(false);
            restartEscortAtHome = false;
            SetNextWaypoint(14, false, false);
            me->SetHomePosition(resetPosition);
            me->GetMotionMaster()->MoveTargetedHome();
            if (Creature* mannoroth = instance->GetCreature(DATA_MANNOROTH))
            {
                mannoroth->SetReactState(REACT_PASSIVE);
                mannoroth->CombatStop(true);
                mannoroth->AI()->EnterEvadeMode();
            }
            if (Creature* captain = instance->GetCreature(DATA_CAPTAIN_VAROTHEN))
            {
                captain->SetReactState(REACT_PASSIVE);
                if (!captain->isAlive())
                    captain->Respawn(true);
                else
                    captain->CombatStop(true);
            }
            if (Creature* tyrande = instance->GetCreature(DATA_TYRANDE_WHISPERWIND))
                tyrande->AI()->DoAction(ACTION_RESET_BOSS_FIGHT);

            if (Creature* portal = instance->GetCreature(DATA_PORTAL))
                portal->RemoveAllAuras();

            if (Creature* portal = instance->GetCreature(DATA_DOOMGUARD_PORTAL))
                portal->m_Events.KillAllEvents(false);

            me->DespawnCreaturesInArea(NPC_DOOMGUARD_DEVASTATOR, 500.00f);
            me->DespawnCreaturesInArea(NPC_EMBEDDED_BLADE, 500.00f);
            me->DespawnCreaturesInArea(NPC_FELHOUND, 500.00f);
            me->DespawnCreaturesInArea(NPC_FELGUARD, 500.00f);
            me->DespawnCreaturesInArea(NPC_DOOMGUARD, 500.00f);
            me->DespawnCreaturesInArea(NPC_EMBEDDED_BLADE_CAPTAIN, 500.00f);
        }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
                case ACTION_RESTART_ESCORT:
                    me->CombatStop(true);
                    me->GetMotionMaster()->MoveTargetedHome();
                    break;
                case ACTION_FIGHT_FINISHED:
                    me->CombatStop(true);
                    me->RemoveAllAuras();
                    events.Reset();
                    SetEscortPaused(false);
                    break;
                default:
                    break;
            }
        }

        void DamageTaken(Unit* attacker, uint32& damage)
        {
            if (me->HealthBelowPctDamaged(10, damage))
                me->SetHealth(me->GetMaxHealth());
        }

        void EnterCombat(Unit* attacker)
        {
            events.ScheduleEvent(EVENT_AURA_OF_IMMOLATION, 5000);
            events.ScheduleEvent(EVENT_TAUNT, 2000);
        }

        void EnterEvadeMode() override {}

        void JustReachedHome() override
        {
            me->SetWalk(false);
            if (restartEscortAtHome)
                SetEscortPaused(false);
            restartEscortAtHome = true;
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (instance->IsDone(DATA_MANNOROTH))
                return;

            if (!isEventActive && who->GetTypeId() == TYPEID_PLAYER)
            {
                if (me->GetDistance2d(who) <= 50.00f)
                {
                    isEventActive = true;
                    SetMaxPlayerDistance(500.0f);
                    SetDespawnAtEnd(false);
                    SetDespawnAtFar(false);
                    events.ScheduleEvent(EVENT_CHECK_DEMONS, 1000);
                    Talk(TALK_ILLIDAN_CLOSE_PORTAL);
                    TalkWithDelay(15000, TALK_ILLIDAN_VERY_WELL);
                    TalkWithDelay(26000, TALK_ILLIDAN_LET_THEM_COME);
                    TalkWithDelay(36000, TALK_ILLIDAN_WEAK_PITIFUL);

                    if (Creature* malfurion = instance->GetCreature(DATA_MALFURION_STORMRAGE))
                    {
                        malfurion->AI()->TalkWithDelay(3000, TALK_MALFURION_IT_IS_MAINTAINED);
                        malfurion->AI()->TalkWithDelay(10000, TALK_MALFURION_CANNOT_BREAK);
                    }

                    if (Creature* tyrande = instance->GetCreature(DATA_TYRANDE_WHISPERWIND))
                    {
                        tyrande->AI()->TalkWithDelay(20000, TALK_TYRANDE_HE_KNOWS_WHAT);
                        tyrande->AI()->TalkWithDelay(30000, TALK_TYRANDE_MOTHER_MOON);
                        tyrande->AI()->DoAction(ACTION_CAST_BLESSING_OF_ELUNE);
                    }
                }
            }
            npc_escortAI::MoveInLineOfSight(who);
        }

        void WaypointReached(uint32 waypointId) override
        {
            switch (waypointId)
            {
                case 10:
                    me->GetPosition(&resetPosition);
                    me->SetHomePosition(resetPosition);
                    SetEscortPaused(true);
                    TalkWithDelay(4000, TALK_ILLIDAN_THIS_WILL_BE_FUN);
                    TalkWithDelay(7000, TALK_ILLIDAN_WAIT_I_HAVE_AN_IDEA);
                    TalkWithDelay(17000, TALK_ILLIDAN_WHAT_OUR_PEOPLE);
                    events.ScheduleEvent(EVENT_DRINK, 21000);
                    TalkWithDelay(23000, TALK_ILLIDAN_YES_YES_I_CAN_FEEL);

                    if (Creature* tyrande = instance->GetCreature(DATA_TYRANDE_WHISPERWIND))
                        tyrande->AI()->TalkWithDelay(10000, TALK_TYRANDE_ILLIDAN_WHAT_IS_IN);
                    break;
                case 12:
                    me->SetReactState(REACT_PASSIVE);
                    me->GetPosition(&resetPosition);
                    me->SetHomePosition(resetPosition);
                    me->RemoveAurasDueToSpell(SPELL_WATERS_OF_ETERNITY);
                    SetEscortPaused(true);
                    if (Creature* mannoroth = instance->GetCreature(DATA_MANNOROTH))
                        mannoroth->AI()->Talk(TALK_MANNOROTH_INTRO);

                    if (Creature* captain = instance->GetCreature(DATA_CAPTAIN_VAROTHEN))
                        captain->AI()->DoAction(ACTION_START_INTRO);
                    break;
                case 13:
                    me->GetPosition(&resetPosition);
                    me->SetHomePosition(resetPosition);
                    if (Creature* captain = instance->GetCreature(DATA_CAPTAIN_VAROTHEN))
                        if (!captain->isInCombat())
                            SetEscortPaused(true);
                    break;
                case 14:
                    SetEscortPaused(true);
                    events.ScheduleEvent(EVENT_PLAYER_ALIVE_CHECK, 5000);
                    if (Creature* mannoroth = instance->GetCreature(DATA_MANNOROTH))
                    {
                        me->SetReactState(REACT_AGGRESSIVE);
                        mannoroth->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC);
                        AttackStart(mannoroth);
                        mannoroth->AI()->AttackStart(me);
                        mannoroth->getThreatManager().addThreat(me, 100000.00f);
                        DoCast(mannoroth, SPELL_DEMON_RUSH, true);
                    }
                    break;
                case 17:
                    SetEscortPaused(true);
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 const diff) override
        {
            npc_escortAI::UpdateAI(diff);
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CHECK_DEMONS:
                    {
                        bool aliveGuardFound = false;
                        std::list<Creature*> doomguards;
                        me->GetCreatureListWithEntryInGrid(doomguards, NPC_DOOMGUARD_ANNIHILATOR, 300.0f);
                        for (std::list<Creature*>::iterator itr = doomguards.begin(); itr != doomguards.end(); ++itr)
                        {
                            if ((*itr)->isAlive())
                            {
                                aliveGuardFound = true;
                                break;
                            }
                        }

                        if (!aliveGuardFound || doomguards.empty())
                        {
                            Start(false, true);
                            if (Creature* tyrande = instance->GetCreature(DATA_TYRANDE_WHISPERWIND))
                                tyrande->AI()->DoAction(ACTION_START_ESCORT_EVENT);

                        }
                        else
                            events.ScheduleEvent(EVENT_CHECK_DEMONS, 1000);
                        break;
                    }
                    case EVENT_DRINK:
                        Talk(TALK_ILLIDAN_EMOTE_DRINK, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                        DoCast(me, SPELL_WATERS_OF_ETERNITY, false);
                        events.ScheduleEvent(EVENT_ATTACK_DOOMBRINGER, 2500);
                        break;
                    case EVENT_ATTACK_DOOMBRINGER:
                        if (Creature* doombringer = instance->GetCreature(DATA_DOOMBRINGER))
                        {
                            doombringer->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                            doombringer->AI()->AttackStart(me);
                            AttackStart(doombringer);
                        }
                        if (Creature* tyrande = instance->GetCreature(DATA_TYRANDE_WHISPERWIND))
                            tyrande->AI()->DoAction(ACTION_ATTACK_DOOMBRINGER);
                        break;
                    case EVENT_PLAYER_ALIVE_CHECK:
                    {
                        bool alivePlayerFound = false;
                        Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();
                        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                            if (Player* player = itr->getSource())
                                if (!player->isGameMaster() && player->isAlive())
                                {
                                    alivePlayerFound = true;
                                    break;
                                }

                        if (!alivePlayerFound)
                            ResetCombat();
                        else
                            events.ScheduleEvent(EVENT_PLAYER_ALIVE_CHECK, 1000);
                        break;
                    }
                    case EVENT_AURA_OF_IMMOLATION:
                        DoCast(me, SPELL_AURA_OF_IMMOLATION, false);
                        events.ScheduleEvent(EVENT_AURA_OF_IMMOLATION, 15000);
                        break;
                    case EVENT_TAUNT:
                        DoCastVictim(SPELL_DEMONIC_TAUNT);
                        events.ScheduleEvent(EVENT_TAUNT, urand(5000, 10000));
                        break;
                }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
        InstanceScript* instance;
        Position resetPosition;
        bool isEventActive;
        bool restartEscortAtHome;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_woe_illidan_part_twoAI(creature);
    }
};

class npc_woe_tyrande_whisperwind : public CreatureScript
{
public:
    npc_woe_tyrande_whisperwind() : CreatureScript("npc_woe_tyrande_whisperwind") { }

    struct npc_woe_tyrande_whisperwindAI : public npc_escortAI
    {
        npc_woe_tyrande_whisperwindAI(Creature* creature) : npc_escortAI(creature)
        {
            instance = me->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            restartEscortAtHome = true;
        }

        bool CanAIAttack(Unit const* target) const override
        {
            return target->GetEntry() != NPC_CAPTAIN_VAROTHEN;
        }

        void EnterEvadeMode() override {}

        void JustReachedHome() override
        {
            me->SetWalk(false);
            if (restartEscortAtHome)
                SetEscortPaused(false);
            restartEscortAtHome = true;
        }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
                case ACTION_START_ESCORT_EVENT:
                    events.CancelEvent(EVENT_BLESSING_OF_ELUNE);
                    me->CombatStop(true);
                    me->RemoveAurasDueToSpell(SPELL_BLESSING_OF_ELUNE);
                    SetMaxPlayerDistance(500.0f);
                    SetDespawnAtEnd(false);
                    SetDespawnAtFar(false);
                    Start(false, true);
                    break;
                case ACTION_RESTART_ESCORT:
                    me->GetMotionMaster()->MoveTargetedHome();
                    break;
                case ACTION_ATTACK_DOOMBRINGER:
                    events.ScheduleEvent(EVENT_ATTACK_DOOMBRINGER, 1000);
                    break;
                case ACTION_CAST_BLESSING_OF_ELUNE:
                    events.ScheduleEvent(EVENT_BLESSING_OF_ELUNE, 30000);
                    break;
                case ACTION_LIGHT_OF_ELUNE:
                    if (!actionStarted)
                    {
                        actionStarted = true;
                        Talk(TALK_TYRANDE_LIGHT_OF_ELUNE_SAVE);
                        DoCast(me, SPELL_BLESSING_OF_ELUNE, false);
                        events.ScheduleEvent(EVENT_SEND_EMOTE, 3000);
                        events.CancelEvent(EVENT_ATTACK_DOOMGUARD);
                    }
                    break;
                case ACTION_RESTART_COMBAT:
                    events.ScheduleEvent(EVENT_ATTACK_DOOMGUARD, 500);
                    me->RemoveAurasDueToSpell(SPELL_BLESSING_OF_ELUNE);
                    Talk(TALK_TYRANDE_HOLD_THEM_BACK_AGAIN);
                    Talk(TALK_TYRANDE_EMOTE_CAN_HOLD_HER, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                    break;
                case ACTION_RESET_BOSS_FIGHT:
                    events.Reset();
                    restartEscortAtHome = false;
                    me->CombatStop(true);
                    me->RemoveAllAuras();
                    me->m_Events.KillAllEvents(false);
                    SetNextWaypoint(15, false, false);
                    me->SetHomePosition(resetPosition);
                    me->GetMotionMaster()->MoveTargetedHome();
                    break;
                case ACTION_HAND_OF_ELUNE:
                    events.ScheduleEvent(EVENT_HAND_OF_ELUNE, 26000);
                    break;
                case ACTION_COLLAPSE:
                    events.ScheduleEvent(EVENT_COLLAPSE, 22000);
                    break;
                case ACTION_FIGHT_FINISHED:
                    Talk(TALK_TYRANDE_COMBAT_DONE);
                    me->CombatStop(true);
                    me->RemoveAllAuras();
                    events.Reset();
                    break;
                default:
                    break;
            }
        }

        void WaypointReached(uint32 waypointId) override
        {
            switch (waypointId)
            {
                case 11:
                case 13:
                    me->GetPosition(&resetPosition);
                    me->SetHomePosition(resetPosition);
                    SetEscortPaused(true);
                    break;
                case 17:
                    events.ScheduleEvent(EVENT_ATTACK_DOOMGUARD, 1000);
                    actionStarted = false;
                    SetEscortPaused(true);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 const diff) override
        {
            npc_escortAI::UpdateAI(diff);
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ATTACK_DOOMBRINGER:
                        if (Creature* doombringer = instance->GetCreature(DATA_DOOMBRINGER))
                        {
                            if (doombringer->isAlive())
                            {
                                DoCast(doombringer, SPELL_LUNAR_SHOT_DOOMBRINGER, false);
                                events.ScheduleEvent(EVENT_ATTACK_DOOMBRINGER, 2000);
                            }
                        }
                        break;
                    case EVENT_BLESSING_OF_ELUNE:
                        Talk(TALK_TYRANDE_EMOTE_BLESSING, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                        DoCast(me, SPELL_BLESSING_OF_ELUNE, false);
                        break;
                    case EVENT_SEND_EMOTE:
                        Talk(TALK_TYRANDE_EMOTE_BLESSING_BOSS, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                        break;
                    case EVENT_HAND_OF_ELUNE:
                        Talk(TALK_TYRANDE_I_AM_OUT_OF_ARROWS);
                        Talk(TALK_TYRANDE_EMOTE_HAND_OF_ELUNE, 0, CHAT_MSG_RAID_BOSS_EMOTE);
                        if (Creature* mannoroth = instance->GetCreature(DATA_MANNOROTH))
                            me->SetFacingToObject(mannoroth);
                        DoCast(me, SPELL_HAND_OF_ELUNE, false);
                        break;
                    case EVENT_COLLAPSE:
                        me->RemoveAurasDueToSpell(SPELL_HAND_OF_ELUNE);
                        DoCast(me, SPELL_COLLAPSE, true);
                        break;
                    case EVENT_ATTACK_DOOMGUARD:
                        if (Creature* doomguard = me->FindNearestCreature(NPC_DOOMGUARD_DEVASTATOR, 45.00f))
                        {
                            me->SetFacingToObject(doomguard);
                            DoCast(doomguard, SPELL_LUNAR_SHOT_INSTANT, true);
                        }
                        events.ScheduleEvent(EVENT_ATTACK_DOOMGUARD, 1000);
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        EventMap events;
        InstanceScript* instance;
        Position resetPosition;
        bool actionStarted;
        bool restartEscortAtHome;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_woe_tyrande_whisperwindAI(creature);
    }
};

class npc_woe_doomguard_annihilator : public CreatureScript
{
public:
    npc_woe_doomguard_annihilator() : CreatureScript("npc_woe_doomguard_annihilator") {}

    struct npc_woe_doomguard_annihilatorAI : public ScriptedAI
    {
        npc_woe_doomguard_annihilatorAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
            canCallForHelp = true;
        }

        void Reset() override
        {
            canCallForHelp = true;
        }

        void DoAction(int32 const action) override
        {
            if (action == ACTION_DONT_CALL_FOR_HELP)
                canCallForHelp = false;
        }

        void EnterCombat(Unit* attacker) override
        {
            if (canCallForHelp)
            {
                std::list<Creature*> doomguards;
                me->GetCreatureListWithEntryInGrid(doomguards, NPC_DOOMGUARD_ANNIHILATOR, 300.0f);
                for (std::list<Creature*>::iterator itr = doomguards.begin(); itr != doomguards.end(); ++itr)
                {
                    if ((*itr)->isAlive())
                    {
                        (*itr)->AI()->DoAction(ACTION_DONT_CALL_FOR_HELP);
                        (*itr)->AI()->AttackStart(attacker);
                    }
                }
            }
        }

        private:
            InstanceScript* instance;
            bool canCallForHelp;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_woe_doomguard_annihilatorAI(creature);
    }
};

class npc_woe_abyssal_doombringer : public CreatureScript
{
public:
    npc_woe_abyssal_doombringer() : CreatureScript("npc_woe_abyssal_doombringer") {}

    struct npc_woe_abyssal_doombringerAI : public ScriptedAI
    {
        npc_woe_abyssal_doombringerAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
            events.Reset();
            events.ScheduleEvent(EVENT_ABYSSAL_FLAMETHROWER, 1000);
        }

        void EnterCombat(Unit* attacker) override
        {
            events.CancelEvent(EVENT_ABYSSAL_FLAMETHROWER);
            me->InterruptSpell(CURRENT_CHANNELED_SPELL);
            events.ScheduleEvent(EVENT_ABYSSAL_FLAMES, 2000);
        }

        void JustDied(Unit* killer) override
        {
            if (me->GetEntry() == NPC_ABYSSAL_DOOMBRINGER)
            {
                if (Creature* illidan = instance->GetCreature(DATA_ILLIDAN_PART_TWO))
                    illidan->AI()->DoAction(ACTION_RESTART_ESCORT);

                if (Creature* tyrande = instance->GetCreature(DATA_TYRANDE_WHISPERWIND))
                    tyrande->AI()->DoAction(ACTION_RESTART_ESCORT);

                me->DespawnOrUnsummon(1000);
            }
        }

        void UpdateAI(uint32 const diff) override
        {
            events.Update(diff);

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_ABYSSAL_FLAMES:
                        DoCast(me, SPELL_ABYSSAL_FLAMES, true);
                        events.ScheduleEvent(EVENT_ABYSSAL_FLAMES, 2000);
                        break;
                    case EVENT_ABYSSAL_FLAMETHROWER:
                        if (Creature* flameTarget = me->FindNearestCreature(NPC_FLAME_TARGET, 50.00f))
                            DoCast(flameTarget, SPELL_ABYSSAL_FLAMETHROWER, false);
                        events.ScheduleEvent(EVENT_ABYSSAL_FLAMETHROWER, 10000);
                        break;
                    default:
                        break;
                }
            }
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

        private:
            EventMap events;
            InstanceScript* instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_woe_abyssal_doombringerAI(creature);
    }
};

class npc_woe_captain_varothen : public CreatureScript
{
public:
    npc_woe_captain_varothen() : CreatureScript("npc_woe_captain_varothen") {}

    struct npc_woe_captain_varothenAI : public ScriptedAI
    {
        npc_woe_captain_varothenAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            instance = me->GetInstanceScript();
            aliveGuards = 0;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        }

        void InitializeAI() override
        {
            for (int8 i = 0; i < 2; i++)
                me->SummonCreature(NPC_HIGHGUARD_ELITE, highguardElitePos[i], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
            ScriptedAI::InitializeAI();
        }

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            events.Reset();
        }

        bool CanAIAttack(Unit const* target) const override
        {
            return target->GetEntry() != NPC_TYRANDE_WHISPERWIND && target->GetEntry() != NPC_ILLIDAN_PART_TWO;
        }

        void DoAction(int32 const action) override
        {
            if (action == ACTION_START_INTRO)
                events.ScheduleEvent(EVENT_INTRO, 7000);
        }

        void EnterCombat(Unit* attacker) override
        {
            if (instance->GetBossState(DATA_MANNOROTH) != IN_PROGRESS)
            {
                if (Creature* illidan = instance->GetCreature(DATA_ILLIDAN_PART_TWO))
                    illidan->AI()->DoAction(ACTION_RESTART_ESCORT);
                if (Creature* tyrande = instance->GetCreature(DATA_TYRANDE_WHISPERWIND))
                    tyrande->AI()->DoAction(ACTION_RESTART_ESCORT);
            }

            Talk(TALK_CAPTAIN_AGGRO);
            me->SetReactState(REACT_AGGRESSIVE);
            events.ScheduleEvent(EVENT_MAGISTRIKE, 10000);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
        }

        void EnterEvadeMode()
        {
            _EnterEvadeMode();
            ScriptedAI::EnterEvadeMode();
        }

        void JustDied(Unit* killer) override
        {
            Talk(TALK_CAPTAIN_DEATH);
            if (Creature* mannoroth = instance->GetCreature(DATA_MANNOROTH))
                mannoroth->AI()->DoAction(ACTION_DISABLE_DAMAGE_COUNTER);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

            // WORKAROUND - we are out of time... - maybe some know the real issue... 
            // if a unfriedly npc summons the blade the spellclick doesnt work.. we use temp. a friendly trigger npc until the bug is found...
            Position pos(*me);
            if (Creature* trigger = me->SummonCreature(55091, pos, TEMPSUMMON_TIMED_DESPAWN, 5000))
                trigger->CastSpell(trigger, SPEL_VAROTHENS_MAGICAL_BLADE, true);
        }

        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            switch (summon->GetEntry())
            {
                case NPC_HIGHGUARD_ELITE:
                {
                    aliveGuards++;
                    Position pos(*summon);
                    summon->SetReactState(REACT_PASSIVE);
                    summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
                    if (Creature* shadowbat = me->SummonCreature(NPC_SHADOWBAT, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000))
                    {
                        shadowbat->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC);
                        summon->CastSpell(shadowbat, SPELL_RIDE_VEHICLE_HARDCODED, true);
                        //shadowbat->CastSpell(shadowbat, SPELL_DISPLACEMENT, true);
                    }
                    break;
                }
                default:
                    break;
            }
        }

        void SummonedCreatureDies(Creature* summon, Unit* killer) override
        {
            switch (summon->GetEntry())
            {
                case NPC_HIGHGUARD_ELITE:
                    if (--aliveGuards <= 0)
                    {
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                        if (Creature* illidan = instance->GetCreature(DATA_ILLIDAN_PART_TWO))
                            illidan->AI()->DoAction(ACTION_RESTART_ESCORT);
                    }
                    break;
                case NPC_SHADOWBAT:
                    for (uint64 summonGUID : summons)
                    {
                        if (Creature* summon = ObjectAccessor::GetCreature(*me, summonGUID))
                        {
                            if (summon->GetEntry() == NPC_HIGHGUARD_ELITE)
                            {
                                if (!summon->GetVehicleBase() || (summon->GetVehicleBase() && !summon->GetVehicleBase()->isAlive()))
                                {
                                    summon->SetReactState(REACT_AGGRESSIVE);
                                    summon->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
                                }
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
            ScriptedAI::SummonedCreatureDies(summon, killer);
        }

        void UpdateAI(uint32 const diff) override
        {
            events.Update(diff);

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch (eventid)
                {
                    case EVENT_INTRO:
                        Talk(TALK_CAPTAIN_INTRO);
                        for (uint64 summonGUID : summons)
                            if (Creature* summon = ObjectAccessor::GetCreature(*me, summonGUID))
                                if (summon->GetEntry() == NPC_SHADOWBAT)
                                {
                                    summon->SetReactState(REACT_AGGRESSIVE);
                                    summon->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC);
                                }
                        break;
                    case EVENT_MAGISTRIKE:
                        DoCastVictim(SPELL_MAGISTRIKE);
                        events.ScheduleEvent(EVENT_MAGISTRIKE, 10000);
                        break;
                }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
        InstanceScript* instance;
        SummonList summons;
        int8 aliveGuards;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_woe_captain_varothenAI(creature);
    }
};

class npc_woe_fel_flames : public CreatureScript
{
public:
    npc_woe_fel_flames() : CreatureScript("npc_woe_fel_flames") { }

    struct npc_woe_fel_flamesAI : public ScriptedAI
    {
        npc_woe_fel_flamesAI(Creature* creature) : ScriptedAI(creature) {}

        void IsSummonedBy(Unit* owner)
        {
            if (InstanceScript* instance = me->GetInstanceScript())
                if (Creature* mannoroth = instance->GetCreature(DATA_MANNOROTH))
                    if (owner != mannoroth)
                        mannoroth->AI()->JustSummoned(me);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_woe_fel_flamesAI(creature);
    }
};

class npc_woe_varothens_magical_blade : public CreatureScript
{
public:
    npc_woe_varothens_magical_blade() : CreatureScript("npc_woe_varothens_magical_blade") { }

    struct npc_woe_varothens_magical_bladeAI : public ScriptedAI
    {
        npc_woe_varothens_magical_bladeAI(Creature* creature) : ScriptedAI(creature) 
        {
        }

        void IsSummonedBy(Unit* owner)
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            Talk(TALK_EMOTE_SWORD_FALLS_TO_THE_GROUND, 0, CHAT_MSG_RAID_BOSS_EMOTE);
            DoCast(me, SPELL_MAGICAL_BLADE_VISUAL, true);
        }

        void OnSpellClick(Unit* /*clicker*/, bool& /*result*/)
        {
            me->DespawnOrUnsummon();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_woe_varothens_magical_bladeAI(creature);
    }
};

class npc_woe_mixed_creatures_ai : public CreatureScript
{
public:
    npc_woe_mixed_creatures_ai() : CreatureScript("npc_woe_mixed_creatures_ai") { }

    struct npc_woe_mixed_creatures_aiAI : public ScriptedAI
    {
        npc_woe_mixed_creatures_aiAI(Creature* creature) : ScriptedAI(creature) {}

        void IsSummonedBy(Unit* owner) override
        {
            if (me->GetEntry() == NPC_FELHOUND || me->GetEntry() == NPC_DOOMGUARD || me->GetEntry() == NPC_FELGUARD || me->GetEntry() == NPC_INFERNO)
                DoZoneInCombat();
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (me->GetEntry() == NPC_DOOMGUARD_DEVASTATOR)
            {
                if (type == POINT_MOTION_TYPE && id == POINT_LADING)
                    DoZoneInCombat();
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_woe_mixed_creatures_aiAI(creature);
    }
};

class spell_woe_nether_portal : public SpellScriptLoader
{
public:
    spell_woe_nether_portal() : SpellScriptLoader("spell_woe_nether_portal") { }

    class spell_woe_nether_portal_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_woe_nether_portal_AuraScript);

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                Position pos(*GetTarget());
                caster->SummonCreature(NPC_DREADLORD_DEBILITATOR, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000);
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_woe_nether_portal_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_woe_nether_portal_AuraScript();
    }
};

class spell_woe_wrath_of_elune : public SpellScriptLoader
{
public:
    spell_woe_wrath_of_elune() : SpellScriptLoader("spell_woe_wrath_of_elune") {}

    class spell_woe_wrath_of_elune_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_woe_wrath_of_elune_SpellScript)

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if([](WorldObject* target){
                return (target->GetEntry() != NPC_FELGUARD && target->GetEntry() != NPC_FELHOUND && target->GetEntry() != NPC_DOOMGUARD);
            });
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_woe_wrath_of_elune_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_woe_wrath_of_elune_SpellScript();
    }
};

class spell_woe_embedded_blade : public SpellScriptLoader
{
public:
    spell_woe_embedded_blade() : SpellScriptLoader("spell_woe_embedded_blade")
    {}

    class spell_woe_embedded_blade_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_woe_embedded_blade_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            if (GetTarget()->HasSpellCooldown(GetId()) || (eventInfo.GetActor() && eventInfo.GetActor()->GetTypeId() != TYPEID_PLAYER))
                return false;

            GetTarget()->AddSpellCooldown(GetId(), 0, time(NULL) + 4);

            return true;
        }

        void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
        {
            if (eventInfo.GetActor())
                eventInfo.GetActor()->CastSpell(GetTarget(), SPELL_MAGISTRIKE_ARC_DAMAGE, true);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_woe_embedded_blade_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_woe_embedded_blade_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_woe_embedded_blade_AuraScript();
    }
};

class spell_woe_magistrike_arc : public SpellScriptLoader
{
public:
    spell_woe_magistrike_arc() : SpellScriptLoader("spell_woe_magistrike_arc") { }

    class spell_woe_magistrike_arc_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_woe_magistrike_arc_SpellScript);

        void DisableDamageEffect(SpellEffIndex effIndex)
        {
            // implemented at spell_woe_embedded_blade
            PreventHitDefaultEffect(effIndex);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_woe_magistrike_arc_SpellScript::DisableDamageEffect, EFFECT_0, SPELL_EFFECT_FORCE_CAST);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_woe_magistrike_arc_SpellScript();
    }
};

class spell_woe_aura_of_immolation : public SpellScriptLoader
{
public:
    spell_woe_aura_of_immolation() : SpellScriptLoader("spell_woe_aura_of_immolation") {}

    class spell_woe_aura_of_immolation_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_woe_aura_of_immolation_SpellScript)

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if([](WorldObject* target)
            {
                if (target->GetEntry() == NPC_FELGUARD || target->GetEntry() == NPC_FELHOUND || target->GetEntry() == NPC_DOOMGUARD
                    || target->GetEntry() == NPC_MANNOROTH || target->GetEntry() == NPC_ABYSSAL_DOOMBRINGER)
                    return false;
                return true;
            });
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_woe_aura_of_immolation_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_woe_aura_of_immolation_SpellScript();
    }
};

class spell_woe_blessing_of_elune : public SpellScriptLoader
{
public:
    spell_woe_blessing_of_elune() : SpellScriptLoader("spell_woe_blessing_of_elune") { }

    class spell_woe_blessing_of_elune_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_woe_blessing_of_elune_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            return !eventInfo.GetActor()->HasSpellCooldown(GetId());
        }

        void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            if (eventInfo.GetActionTarget())
            {
                eventInfo.GetActor()->AddSpellCooldown(GetId(), 0, time(NULL) + 2);
                eventInfo.GetActor()->CastSpell(eventInfo.GetActionTarget(), SPELL_ELUNES_WRATH, true);
            }
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_woe_blessing_of_elune_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_woe_blessing_of_elune_AuraScript::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_woe_blessing_of_elune_AuraScript();
    }
};

class spell_woe_inferno : public SpellScriptLoader
{
public:
    spell_woe_inferno() : SpellScriptLoader("spell_woe_inferno") { }

    class spell_woe_inferno_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_woe_inferno_SpellScript);

        void CheckTarget(SpellEffIndex effIndex)
        {
            if (!GetHitDest())
                return;

            float x, y, z;
            GetHitDest()->GetPosition(x, y, z);
            GetCaster()->SummonCreature(NPC_INFERNO, x, y, z);
        }

        void Register()
        {
            OnEffectHit += SpellEffectFn(spell_woe_inferno_SpellScript::CheckTarget, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_woe_inferno_SpellScript();
    }
};

class achievement_woe_thats_not_canon : public AchievementCriteriaScript
{
public:
    achievement_woe_thats_not_canon() : AchievementCriteriaScript("achievement_woe_thats_not_canon") { }

    bool OnCheck(Player* /*player*/, Unit* target)
    {
        return target && (target->GetAI()->GetData(DATA_THATS_NOT_CANON) >= 10000000);
    }
};

void AddSC_boss_mannoroth_and_varothen()
{
    new boss_woe_mannoroth();
    new npc_woe_illidan_part_two();
    new npc_woe_tyrande_whisperwind();
    new npc_woe_doomguard_annihilator();
    new npc_woe_abyssal_doombringer();
    new npc_woe_captain_varothen();
    new npc_woe_fel_flames();
    new npc_woe_varothens_magical_blade();
    new npc_woe_mixed_creatures_ai();
    new spell_woe_nether_portal();
    new spell_woe_wrath_of_elune();
    new spell_woe_embedded_blade();
    new spell_woe_magistrike_arc();
    new spell_woe_aura_of_immolation();
    new spell_woe_blessing_of_elune();
    new spell_woe_inferno();
    new achievement_woe_thats_not_canon();
}
