/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2007 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Boss_Hex_Lord_Malacrass
SD%Complete:
SDComment:
SDCategory: Zul'Aman
EndScriptData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SharedDefines.h"
#include "zulaman.h"


enum says
{
    SAY_AGGRO        = 0,
    SAY_SPIRIT_BOLTS = 1,
    SAY_DRAIN_POWER  = 2,
    SAY_KILL         = 3,
    SAY_DEATH        = 4,
};
//Defines for various powers he uses after using soul drain

enum Spells
{
    // Malacrass
    SPELL_SPIRIT_BOLTS              = 43383,
    SPELL_DRAIN_POWER               = 44131,
    SPELL_SIPHON_SOUL               = 43501,

    // Alyson Antille
    SPELL_FLASH_HEAL                = 43575,
    SPELL_EMPOWERED_SMITE           = 97510,
    SPELL_ARCANE_TORRENT            = 33390,

    // Slither
    SPELL_VENOM_SPIT                = 43579,

    // Gazakroth
    SPELL_FIREBOLT                  = 43584,

    // Dearkheart
    SPELL_PSYCHIC_WAIL              = 43590,
    SPELL_WAIL_OF_SOULS             = 97511,

    // Death Knight
    SPELL_DK_PLAGUE_STRIKE          = 61600,
    SPELL_DK_DEATH_AND_DECAY        = 61603,
    SPELL_DK_BLOOD_WORMS            = 97628,
    SPELL_DK_BLOOD_WORMS_HEAL       = 97630,

    // Druid
    SPELL_DR_TYPHOON                = 97636,
    SPELL_DR_LIFEBLOOM              = 43421,
    SPELL_DR_MOONFIRE               = 43545,

    // Hunter
    SPELL_HU_EXPLOSIVE_TRAP         = 43444,
    SPELL_HU_FREEZING_TRAP          = 43447,
    SPELL_HU_SNAKE_TRAP             = 43449,
    SPELL_CRIPPLING_POISON          = 25809,
    SPELL_MIND_NUMBING_POISON       = 25810,

    // Mage
    SPELL_MG_FROST_NOVA             = 43426,
    SPELL_MG_ICE_LANCE              = 43427,
    SPELL_MG_FROSTBOLT              = 43428,

    // Paladin
    SPELL_PA_CONSECRATION           = 43429,
    SPELL_PA_AVENGING_WRATH         = 43430,
    SPELL_PA_HOLY_LIGHT             = 43451,

    // Priest
    SPELL_PR_FLASH_HEAL             = 43431,
    SPELL_PR_SW_PAIN                = 97642,
    SPELL_PR_PSYCHIC_SCREAM         = 43432,
    SPELL_PR_LEAP_OF_FAITH          = 97640,

    // Rogue
    SPELL_RO_SMOKE_BOMB             = 97643,
    SPELL_RO_SLICE_AND_DICE         = 43547,
    SPELL_RO_WOUND_POISON           = 43461,

    // Shaman
    SPELL_SH_CHAIN_LIGHTNING        = 43435,
    SPELL_SH_FIRE_NOVA_TOTEM        = 43436,
    SPELL_SH_HEALING_WAVE           = 43548,
    SPELL_FIRE_NOVA              = 43464,

    // Warlock
    SPELL_WL_BANE_OF_DOOM           = 43439,
    SPELL_WL_RAIN_OF_FIRE           = 43440,
    SPELL_WL_UNSTABLE_AFFLICTION    = 43522,
    SPELL_WL_UNSTABLE_AFFL_DISPEL   = 43523,

    // Warrior
    SPELL_WR_MORTAL_STRIKE          = 43441,
    SPELL_WR_HEROIC_LEAP            = 97521,
    SPELL_WR_SPELL_REFLECTION       = 43443
};

#define ORIENT                  1.5696f
#define POS_Y                   921.2795f
#define POS_Z                   33.8883f

static float Pos_X[2] = {112.8827f, 122.8827f};

static uint32 addEntryList[4]=
{
    24240, //Alyson Antille
    24242, //Slither
    24244, //Gazakroth
    24246 //Darkheart
};

enum Events
{
    EVENT_SPIRIT_BOLTS = 1,
    EVENT_DRAIN_POWER,
    EVENT_SIPHON_SOUL,
    EVENT_SIPHON_SOUL_POWER,
    EVENT_SIPHON_SOUL_CANCEL,

    // Alyson Antille
    EVENT_FLASH_HEAL,
    EVENT_EMPOWERED_SMITE,
    EVENT_ARCANE_TORRENT,

    // Slither
    EVENT_VENOM_SPIT,

    // Gazakroth
    EVENT_FIREBOLT,

    // Darkheart
    EVENT_PSYCHIC_WAIL,
    EVENT_WAIL_OF_SOULS,

    // Death Knight
    EVENT_DK_PLAGUE_STRIKE,
    EVENT_DK_DEATH_AND_DECAY,
    EVENT_DK_BLOOD_WORMS,

    // Druid
    EVENT_DR_TYPHOON,
    EVENT_DR_MOONFIRE,
    EVENT_DR_LIFEBLOOM,

    // Hunter
    EVENT_HU_EXPLOSIVE_TRAP,
    EVENT_HU_FREEZING_TRAP,
    EVENT_HU_SNAKE_TRAP,
    EVENT_CRIPPLING_POISON,
    EVENT_MIND_NUMBING_POISON,

    // Mage
    EVENT_MG_FROST_NOVA,
    EVENT_MG_FROSTBOLT,
    EVENT_MG_ICE_LANCE,

    // Paladin
    EVENT_PA_AVENGING_WRATH,
    EVENT_PA_CONSECRATION,
    EVENT_PA_HOLY_LIGHT,

    // Priest
    EVENT_PR_FLASH_HEAL,
    EVENT_PR_PSYCHIC_SCREAM,
    EVENT_PR_LEAP_OF_FAITH,
    EVENT_PR_SW_PAIN,

    // Rogue
    EVENT_RO_SMOKE_BOMB,
    EVENT_RO_WOUND_POISON,
    EVENT_RO_SLICE_AND_DICE,

    // Shaman
    EVENT_SH_CHAIN_LIGHTNING,
    EVENT_SH_FIRE_NOVA_TOTEM,
    EVENT_SH_HEALING_WAVE,
    EVENT_FIRE_NOVA,

    // Warlock
    EVENT_WL_BANE_OF_DOOM,
    EVENT_WL_UNSTABLE_AFFLICTION,
    EVENT_WL_RAIN_OF_FIRE,

    // Warrior
    EVENT_WR_MORTAL_STRIKE,
    EVENT_WR_SPELL_REFLECTION,
    EVENT_WR_HEROIC_LEAP

};

class boss_hex_lord_malacrass : public CreatureScript
{
    public:
        boss_hex_lord_malacrass() : CreatureScript("boss_hex_lord_malacrass") { }

        struct boss_hex_lord_malacrassAI : public BossAI
        {
            boss_hex_lord_malacrassAI(Creature* creature) : BossAI(creature, DATA_HEXLORDEVENT), summons(me)
            {
                instance = creature->GetInstanceScript();
                SelectAddEntry();
                for (uint8 i = 0; i < 2; ++i)
                    addGUID[i] = 0;
            }

            InstanceScript* instance;
            EventMap events;
            SummonList summons;

            uint8 playerClass;
            uint64 addGUID[2];
            uint32 addEntry[2];
            bool isSpecial;

            void Reset()
            {
                _Reset();
                events.Reset();
                isSpecial = false;
                SpawnAdds();
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                DoZoneInCombat();
                Talk(SAY_AGGRO);
                events.ScheduleEvent(EVENT_SPIRIT_BOLTS, 30000);
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_KILL);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
            }

            void SelectAddEntry()
            {
                std::vector<uint32> addList;

                for (uint8 i = 0; i < 4; ++i)
                    addList.push_back(addEntryList[i]);

                while (addList.size() > 2)
                    addList.erase(addList.begin()+rand()%addList.size());

                uint8 i = 0;
                for (std::vector<uint32>::const_iterator itr = addList.begin(); itr != addList.end(); ++itr, ++i)
                    addEntry[i] = *itr;
            }

            void SpawnAdds()
            {
                for (uint8 i = 0; i < 2; ++i)
                {
                    Creature* creature = (Unit::GetCreature((*me), addGUID[i]));
                    if (!creature || !creature->isAlive())
                    {
                        if (creature)
                            creature->setDeathState(DEAD);
                        creature = me->SummonCreature(addEntry[i], Pos_X[i], POS_Y, POS_Z, ORIENT, TEMPSUMMON_DEAD_DESPAWN, 0);
                        if (creature)
                            addGUID[i] = creature->GetGUID();
                    }
                    else
                    {
                        creature->AI()->EnterEvadeMode();
                        creature->SetPosition(Pos_X[i], POS_Y, POS_Z, ORIENT);
                        creature->StopMoving();
                    }
                }
            }

            void SpellHitTarget(Unit* target, SpellInfo const* spell)
            {
                if (spell->Id == SPELL_SIPHON_SOUL)
                {
                    playerClass = target->getClass();
                    events.CancelEvent(EVENT_SIPHON_SOUL_POWER);
                    events.ScheduleEvent(EVENT_SIPHON_SOUL_POWER, 1);
                    isSpecial = true;
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                {
                    if (isSpecial)
                    {
                        me->ClearUnitState(UNIT_STATE_CASTING);
                        isSpecial = false;
                    }
                    return;
                }

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SPIRIT_BOLTS:
                            Talk(SAY_SPIRIT_BOLTS);
                            me->CastSpell(me, SPELL_SPIRIT_BOLTS, true);
                            events.ScheduleEvent(EVENT_DRAIN_POWER, 5000);
                            break;
                        case EVENT_DRAIN_POWER:
                            me->CastSpell(me, SPELL_DRAIN_POWER, false);
                            events.ScheduleEvent(EVENT_SIPHON_SOUL, 1);
                            break;
                        case EVENT_SIPHON_SOUL:
                        {
                            Talk(SAY_DRAIN_POWER);
                            DoCastRandom(SPELL_SIPHON_SOUL, 0.0f);
                            isSpecial = true;
                            events.ScheduleEvent(EVENT_SIPHON_SOUL_POWER, 5000);
                            break;
                        }
                        case EVENT_SIPHON_SOUL_POWER:
                        {
                            switch (playerClass)
                            {
                                case CLASS_DEATH_KNIGHT:
                                    events.ScheduleEvent(EVENT_DK_PLAGUE_STRIKE, 2000);
                                    events.ScheduleEvent(EVENT_DK_DEATH_AND_DECAY, 16000);
                                    events.ScheduleEvent(EVENT_DK_BLOOD_WORMS, 15000);
                                    break;
                                case CLASS_DRUID:
                                    events.ScheduleEvent(EVENT_DR_TYPHOON, 13000);
                                    events.ScheduleEvent(EVENT_DR_MOONFIRE, urand(5000, 7000));
                                    events.ScheduleEvent(EVENT_DR_LIFEBLOOM, urand(10000, 12000));
                                    break;
                                case CLASS_HUNTER:
                                    events.ScheduleEvent(EVENT_HU_EXPLOSIVE_TRAP, 5000);
                                    events.ScheduleEvent(EVENT_HU_FREEZING_TRAP, 10000);
                                    events.ScheduleEvent(EVENT_HU_SNAKE_TRAP, 14000);
                                    break;
                                case CLASS_MAGE:
                                    events.ScheduleEvent(EVENT_MG_FROST_NOVA, 10000);
                                    events.ScheduleEvent(EVENT_MG_FROSTBOLT, 3000);
                                    events.ScheduleEvent(EVENT_MG_ICE_LANCE, 6000);
                                    break;
                                case CLASS_PALADIN:
                                    events.ScheduleEvent(EVENT_PA_AVENGING_WRATH, 9000);
                                    events.ScheduleEvent(EVENT_PA_HOLY_LIGHT, 10000);
                                    events.ScheduleEvent(EVENT_PA_CONSECRATION, 12000);
                                    break;
                                case CLASS_PRIEST:
                                    events.ScheduleEvent(EVENT_PR_FLASH_HEAL, 15000);
                                    events.ScheduleEvent(EVENT_PR_PSYCHIC_SCREAM, 18000);
                                    events.ScheduleEvent(EVENT_PR_LEAP_OF_FAITH, 10000);
                                    events.ScheduleEvent(EVENT_PR_SW_PAIN, 5000);
                                    break;
                                case CLASS_ROGUE:
                                    events.ScheduleEvent(EVENT_RO_SMOKE_BOMB, 10000);
                                    events.ScheduleEvent(EVENT_RO_WOUND_POISON, 5000);
                                    events.ScheduleEvent(EVENT_RO_SLICE_AND_DICE, 1000);
                                    break;
                                case CLASS_SHAMAN:
                                    events.ScheduleEvent(EVENT_SH_CHAIN_LIGHTNING, 1000);
                                    events.ScheduleEvent(EVENT_SH_FIRE_NOVA_TOTEM, 15000);
                                    events.ScheduleEvent(EVENT_SH_HEALING_WAVE, 13000);
                                    break;
                                case CLASS_WARLOCK:
                                    events.ScheduleEvent(EVENT_WL_BANE_OF_DOOM, 11000);
                                    events.ScheduleEvent(EVENT_WL_UNSTABLE_AFFLICTION, 10000);
                                    events.ScheduleEvent(EVENT_WL_RAIN_OF_FIRE, 5000);
                                    break;
                                case CLASS_WARRIOR:
                                    events.ScheduleEvent(EVENT_WR_MORTAL_STRIKE, 2000);
                                    events.ScheduleEvent(EVENT_WR_SPELL_REFLECTION, 13000);
                                    events.ScheduleEvent(EVENT_WR_HEROIC_LEAP, 7000);
                                    break;
                            }

                            events.ScheduleEvent(EVENT_SIPHON_SOUL_CANCEL, 30000);
                            break;
                        }
                        case EVENT_SIPHON_SOUL_CANCEL:
                        {
                            switch (playerClass)
                            {
                                case CLASS_DEATH_KNIGHT:
                                    events.CancelEvent(EVENT_DK_PLAGUE_STRIKE);
                                    events.CancelEvent(EVENT_DK_DEATH_AND_DECAY);
                                    events.CancelEvent(EVENT_DK_BLOOD_WORMS);
                                    break;
                                case CLASS_DRUID:
                                    events.CancelEvent(EVENT_DR_TYPHOON);
                                    events.CancelEvent(EVENT_DR_MOONFIRE);
                                    events.CancelEvent(EVENT_DR_LIFEBLOOM);
                                    break;
                                case CLASS_HUNTER:
                                    events.CancelEvent(EVENT_HU_EXPLOSIVE_TRAP);
                                    events.CancelEvent(EVENT_HU_FREEZING_TRAP);
                                    events.CancelEvent(EVENT_HU_SNAKE_TRAP);
                                    break;
                                case CLASS_MAGE:
                                    events.CancelEvent(EVENT_MG_FROST_NOVA);
                                    events.CancelEvent(EVENT_MG_FROSTBOLT);
                                    events.CancelEvent(EVENT_MG_ICE_LANCE);
                                    break;
                                case CLASS_PALADIN:
                                    events.CancelEvent(EVENT_PA_AVENGING_WRATH);
                                    events.CancelEvent(EVENT_PA_HOLY_LIGHT);
                                    events.CancelEvent(EVENT_PA_CONSECRATION);
                                    break;
                                case CLASS_PRIEST:
                                    events.CancelEvent(EVENT_PR_FLASH_HEAL);
                                    events.CancelEvent(EVENT_PR_PSYCHIC_SCREAM);
                                    events.CancelEvent(EVENT_PR_LEAP_OF_FAITH);
                                    events.CancelEvent(EVENT_PR_SW_PAIN);
                                    break;
                                case CLASS_ROGUE:
                                    events.CancelEvent(EVENT_RO_SMOKE_BOMB);
                                    events.CancelEvent(EVENT_RO_WOUND_POISON);
                                    events.CancelEvent(EVENT_RO_SLICE_AND_DICE);
                                    break;
                                case CLASS_SHAMAN:
                                    events.CancelEvent(EVENT_SH_CHAIN_LIGHTNING);
                                    events.CancelEvent(EVENT_SH_FIRE_NOVA_TOTEM);
                                    events.CancelEvent(EVENT_SH_HEALING_WAVE);
                                    break;
                                case CLASS_WARLOCK:
                                    events.CancelEvent(EVENT_WL_BANE_OF_DOOM);
                                    events.CancelEvent(EVENT_WL_UNSTABLE_AFFLICTION);
                                    events.CancelEvent(EVENT_WL_RAIN_OF_FIRE);
                                    break;
                                case CLASS_WARRIOR:
                                    events.CancelEvent(EVENT_WR_MORTAL_STRIKE);
                                    events.CancelEvent(EVENT_WR_SPELL_REFLECTION);
                                    events.CancelEvent(EVENT_WR_HEROIC_LEAP);
                                    break;
                            }
                            events.ScheduleEvent(EVENT_SPIRIT_BOLTS, 1);
                            break;
                        }
                        case EVENT_DK_PLAGUE_STRIKE:
                            me->CastSpell(me->getVictim(), SPELL_DK_PLAGUE_STRIKE, false);
                            events.ScheduleEvent(EVENT_DK_PLAGUE_STRIKE, 10000);
                            break;
                        case EVENT_DK_DEATH_AND_DECAY:
                            DoCastRandom(SPELL_DK_DEATH_AND_DECAY, 0.0f);
                            events.ScheduleEvent(EVENT_DK_DEATH_AND_DECAY, urand(10000, 12000));
                            break;
                        case EVENT_DK_BLOOD_WORMS:
                            DoCastRandom(SPELL_DK_BLOOD_WORMS, 0.0f);
                            break;
                        case EVENT_DR_TYPHOON:
                            DoCastVictim(SPELL_DR_TYPHOON);
                            events.ScheduleEvent(EVENT_DR_TYPHOON, 10000);
                            break;
                        case EVENT_DR_MOONFIRE:
                            DoCastRandom(SPELL_DR_MOONFIRE, 0.0f);
                            events.ScheduleEvent(EVENT_DR_MOONFIRE, urand(3000, 5000));
                            break;
                        case EVENT_DR_LIFEBLOOM:
                            me->CastSpell(me, SPELL_DR_LIFEBLOOM, false);
                            events.ScheduleEvent(EVENT_DR_LIFEBLOOM, 10000);
                            break;
                        case EVENT_HU_EXPLOSIVE_TRAP:
                            me->CastSpell(me, SPELL_HU_EXPLOSIVE_TRAP, false);
                            events.ScheduleEvent(EVENT_HU_EXPLOSIVE_TRAP, 10000);
                            break;
                        case EVENT_HU_FREEZING_TRAP:
                            me->CastSpell(me, SPELL_HU_FREEZING_TRAP, false);
                            events.ScheduleEvent(EVENT_HU_FREEZING_TRAP, 10000);
                            break;
                        case EVENT_HU_SNAKE_TRAP:
                            me->CastSpell(me, SPELL_HU_SNAKE_TRAP, false);
                            events.ScheduleEvent(EVENT_HU_SNAKE_TRAP, 10000);
                            break;
                        case EVENT_MG_FROST_NOVA:
                            me->CastSpell(me, SPELL_MG_FROST_NOVA, false);
                            events.ScheduleEvent(EVENT_MG_FROST_NOVA, 10000);
                            break;
                        case EVENT_MG_FROSTBOLT:
                            DoCastRandom(SPELL_MG_FROSTBOLT, 0.0f);
                            events.ScheduleEvent(EVENT_MG_FROSTBOLT, 2000);
                            break;
                        case EVENT_MG_ICE_LANCE:
                            DoCastRandom(SPELL_MG_ICE_LANCE, 0.0f);
                            events.ScheduleEvent(EVENT_MG_ICE_LANCE, 5000);
                            break;
                        case EVENT_PA_AVENGING_WRATH:
                            me->CastSpell(me, SPELL_PA_AVENGING_WRATH, false);
                            events.ScheduleEvent(EVENT_PA_AVENGING_WRATH, 12000);
                            break;
                        case EVENT_PA_HOLY_LIGHT:
                        {
                            if (Unit* target = DoSelectLowestHpFriendly(40))
                                me->CastSpell(target, SPELL_PA_HOLY_LIGHT, false);
                            else
                                me->CastSpell(me, SPELL_PA_HOLY_LIGHT, false);
                            events.ScheduleEvent(EVENT_PA_HOLY_LIGHT, 15000);
                            break;
                        }
                        case EVENT_PA_CONSECRATION:
                            me->CastSpell(me, SPELL_PA_CONSECRATION, false);
                            events.ScheduleEvent(EVENT_PA_CONSECRATION, urand(12000, 15000));
                            break;
                        case EVENT_PR_FLASH_HEAL:
                        {
                            if (Unit* target = DoSelectLowestHpFriendly(40))
                                me->CastSpell(target, SPELL_PR_FLASH_HEAL, false);
                            else
                                me->CastSpell(me, SPELL_PR_FLASH_HEAL, false);
                            events.ScheduleEvent(EVENT_PR_FLASH_HEAL, 15000);
                            break;
                        }
                        case EVENT_PR_PSYCHIC_SCREAM:
                            me->CastSpell(me, SPELL_PR_PSYCHIC_SCREAM, false);
                            break;
                        case EVENT_PR_LEAP_OF_FAITH:
                            if (Unit* target = DoSelectLowestHpFriendly(40))
                                me->CastSpell(target, SPELL_PR_LEAP_OF_FAITH, false);
                            events.ScheduleEvent(EVENT_PR_LEAP_OF_FAITH, 10000);
                            break;
                        case EVENT_PR_SW_PAIN:
                            DoCastRandom(SPELL_PR_SW_PAIN, 0.0f);
                            break;
                        case EVENT_RO_SMOKE_BOMB:
                            DoCastRandom(SPELL_RO_SMOKE_BOMB, 0.0f);
                            events.ScheduleEvent(EVENT_RO_SMOKE_BOMB, 15000);
                            break;
                        case EVENT_RO_WOUND_POISON:
                            DoCastVictim(SPELL_RO_WOUND_POISON);
                            events.ScheduleEvent(EVENT_RO_WOUND_POISON, urand(3000, 5000));
                            break;
                        case EVENT_RO_SLICE_AND_DICE:
                            me->CastSpell(me, SPELL_RO_SLICE_AND_DICE, false);
                            break;
                        case EVENT_SH_CHAIN_LIGHTNING:
                            DoCastRandom(SPELL_SH_CHAIN_LIGHTNING, 0.0f);
                            events.ScheduleEvent(EVENT_SH_CHAIN_LIGHTNING, 10000);
                            break;
                        case EVENT_SH_FIRE_NOVA_TOTEM:
                            me->SummonCreature(24320, me->GetPositionX()+rand()%5, me->GetPositionY()+rand()%5, me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 6000);
                            events.ScheduleEvent(EVENT_SH_FIRE_NOVA_TOTEM, 10000);
                            break;
                        case EVENT_SH_HEALING_WAVE:
                        {
                            if (Unit* target = DoSelectLowestHpFriendly(40))
                                me->CastSpell(target, SPELL_SH_HEALING_WAVE, false);
                            else
                                me->CastSpell(me, SPELL_SH_HEALING_WAVE, false);
                            events.ScheduleEvent(EVENT_SH_HEALING_WAVE, urand(13000, 15000));
                            break;
                        }
                        case EVENT_WL_BANE_OF_DOOM:
                            DoCastRandom(SPELL_WL_BANE_OF_DOOM, 0.0f);
                            break;
                        case EVENT_WL_UNSTABLE_AFFLICTION:
                            DoCastRandom(SPELL_WL_UNSTABLE_AFFLICTION, 0.0f);
                            events.ScheduleEvent(EVENT_WL_UNSTABLE_AFFLICTION, 10000);
                            break;
                        case EVENT_WL_RAIN_OF_FIRE:
                            DoCastRandom(SPELL_WL_RAIN_OF_FIRE, 0.0f);
                            events.ScheduleEvent(EVENT_WL_RAIN_OF_FIRE, urand (7000, 10000));
                            break;
                        case EVENT_WR_MORTAL_STRIKE:
                            DoCastVictim(SPELL_WR_MORTAL_STRIKE);
                            events.ScheduleEvent(EVENT_WR_MORTAL_STRIKE, 10000);
                            break;
                        case EVENT_WR_HEROIC_LEAP:
                            DoCastRandom(SPELL_WR_HEROIC_LEAP, 0.0f);
                            events.ScheduleEvent(EVENT_WR_HEROIC_LEAP, 10000);
                            break;
                        case EVENT_WR_SPELL_REFLECTION:
                            me->CastSpell(me, SPELL_WR_SPELL_REFLECTION, false);
                            events.ScheduleEvent(EVENT_WR_SPELL_REFLECTION, 10000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_hex_lord_malacrassAI(creature);
        }
};

class boss_alyson_antille : public CreatureScript
{
    public:
        boss_alyson_antille() : CreatureScript("boss_alyson_antille") { }

        struct boss_alyson_antilleAI : public ScriptedAI
        {

            boss_alyson_antilleAI(Creature* creature) : ScriptedAI(creature) {}

            EventMap events;

            void Reset()
            {}

            void EnterCombat(Unit* /*who*/)
            {
                DoZoneInCombat();
                events.ScheduleEvent(EVENT_FLASH_HEAL, 1);
                events.ScheduleEvent(EVENT_EMPOWERED_SMITE, 2500);
                events.ScheduleEvent(EVENT_ARCANE_TORRENT, 10000);
            }

            void AttackStart(Unit* who)
            {
                if (!who)
                    return;

                if (who->isTargetableForAttack())
                {
                    if (me->Attack(who, false))
                    {
                        me->GetMotionMaster()->MoveChase(who, 20);
                        me->AddThreat(who, 0.0f);
                    }
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FLASH_HEAL:
                            if (Unit* target = DoSelectLowestHpFriendly(40))
                                me->CastSpell(target, SPELL_FLASH_HEAL, false);
                            events.ScheduleEvent(EVENT_FLASH_HEAL, 5000);
                            break;
                        case EVENT_EMPOWERED_SMITE:
                            DoCastRandom(SPELL_EMPOWERED_SMITE, 0.0f);
                            events.ScheduleEvent(EVENT_EMPOWERED_SMITE, 5000);
                            break;
                        case EVENT_ARCANE_TORRENT:
                            me->CastSpell(me, SPELL_ARCANE_TORRENT, false);
                            events.ScheduleEvent(EVENT_ARCANE_TORRENT, 10000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_alyson_antilleAI(creature);
        }
};

class boss_gazakroth : public CreatureScript
{
    public:
        boss_gazakroth() : CreatureScript("boss_gazakroth") { }

        struct boss_gazakrothAI : public ScriptedAI
        {

            boss_gazakrothAI(Creature* creature) : ScriptedAI(creature) {}

            EventMap events;

            void Reset()
            {}

            void EnterCombat(Unit* /*who*/)
            {
                DoZoneInCombat();
                events.ScheduleEvent(EVENT_FIREBOLT, 2000);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FIREBOLT:
                            DoCastRandom(SPELL_FIREBOLT, 0.0f);
                            events.ScheduleEvent(EVENT_FIREBOLT, 1000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_gazakrothAI(creature);
        }
};

class boss_darkheart : public CreatureScript
{
    public:
        boss_darkheart() : CreatureScript("boss_darkheart") { }

        struct boss_darkheartAI : public ScriptedAI
        {
            boss_darkheartAI(Creature* creature) : ScriptedAI(creature)  {}

            EventMap events;

            void Reset()
            {}

            void EnterCombat(Unit* /*who*/)
            {
                DoZoneInCombat();
                events.ScheduleEvent(EVENT_WAIL_OF_SOULS, urand(3000, 5000));
                events.ScheduleEvent(EVENT_PSYCHIC_WAIL, 10000);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_WAIL_OF_SOULS:
                            me->CastSpell(me, SPELL_WAIL_OF_SOULS, false);
                            events.ScheduleEvent(EVENT_WAIL_OF_SOULS, 1000);
                            break;
                        case EVENT_PSYCHIC_WAIL:
                            me->CastSpell(me, SPELL_PSYCHIC_WAIL, false);
                            events.ScheduleEvent(EVENT_PSYCHIC_WAIL, 10000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_darkheartAI(creature);
        }
};

class boss_slither : public CreatureScript
{
    public:
        boss_slither() : CreatureScript("boss_slither") { }

        struct boss_slitherAI : public ScriptedAI
        {
            boss_slitherAI(Creature* creature) : ScriptedAI(creature) {}

            EventMap events;

            void Reset()
            {}

            void EnterCombat(Unit* /*who*/)
            {
                DoZoneInCombat();
                events.ScheduleEvent(EVENT_VENOM_SPIT, 5000);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_VENOM_SPIT:
                            DoCastRandom(SPELL_VENOM_SPIT, 0.0f);
                            events.ScheduleEvent(EVENT_VENOM_SPIT, 2500);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_slitherAI(creature);
        }
};

class npc_fire_nova_totem : public CreatureScript
{
    public:
        npc_fire_nova_totem() : CreatureScript("npc_fire_nova_totem") { }

        struct npc_fire_nova_totemAI : public ScriptedAI
        {
            npc_fire_nova_totemAI(Creature* creature) : ScriptedAI(creature) {}

            EventMap events;

            void Reset()
            {}

            void IsSummonedBy(Unit* /*summoner*/)
            {
                events.ScheduleEvent(EVENT_FIRE_NOVA, 5000);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FIRE_NOVA:
                            me->CastSpell(me, SPELL_FIRE_NOVA, true);
                            break;
                    }
                }

            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_fire_nova_totemAI(creature);
        }
};

class npc_blood_worms : public CreatureScript
{
    public:
        npc_blood_worms() : CreatureScript("npc_blood_worms") { }

        struct npc_blood_wormsAI : public ScriptedAI
        {
            npc_blood_wormsAI(Creature* creature) : ScriptedAI(creature) {}

            void Reset()
            {}

            void IsSummonedBy(Unit* summoner)
            {
                me->SetReactState(REACT_PASSIVE);
                me->GetMotionMaster()->MoveChase(summoner, 0.0f, 0.0f);
                me->GetMotionMaster()->MoveFollow(summoner, 0.0f, 0.0f);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                if (TempSummon* summ = me->ToTempSummon())
                {
                    if(Unit* summoner = summ->GetSummoner())
                    {
                        if (me->IsWithinDistInMap(summoner, 1.0f))
                        {
                            me->CastSpell(summoner, SPELL_DK_BLOOD_WORMS_HEAL, true);
                            me->DisappearAndDie();
                        }
                    }
                }

            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_blood_wormsAI(creature);
        }
};

class npc_amani_snake : public CreatureScript
{
    public:
        npc_amani_snake() : CreatureScript("npc_amani_snake") { }

        struct npc_amani_snakeAI : public ScriptedAI
        {
            npc_amani_snakeAI(Creature* creature) : ScriptedAI(creature) {}

            EventMap events;

            void Reset()
            {}

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_CRIPPLING_POISON, urand(2000, 10000));
                events.ScheduleEvent(EVENT_MIND_NUMBING_POISON, urand(5000, 13000));
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CRIPPLING_POISON:
                            DoCastVictim(SPELL_CRIPPLING_POISON);
                            break;
                        case EVENT_MIND_NUMBING_POISON:
                            DoCastVictim(SPELL_MIND_NUMBING_POISON);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_amani_snakeAI(creature);
        }
};

class spell_hexlord_unstable_affliction : public SpellScriptLoader
{
    public:
        spell_hexlord_unstable_affliction() : SpellScriptLoader("spell_hexlord_unstable_affliction") { }

        class spell_hexlord_unstable_affliction_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hexlord_unstable_affliction_AuraScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WL_UNSTABLE_AFFL_DISPEL))
                    return false;
                return true;
            }

            void HandleDispel(DispelInfo* dispelInfo)
            {
                if (Unit* caster = GetCaster())
                    caster->CastSpell(dispelInfo->GetDispeller(), SPELL_WL_UNSTABLE_AFFL_DISPEL, true, NULL, GetEffect(EFFECT_0));
            }

            void Register()
            {
                AfterDispel += AuraDispelFn(spell_hexlord_unstable_affliction_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hexlord_unstable_affliction_AuraScript();
        }
};

void AddSC_boss_hex_lord_malacrass()
{
    new boss_hex_lord_malacrass();
    new boss_gazakroth();
    new boss_darkheart();
    new boss_slither();
    new boss_alyson_antille();
    new npc_fire_nova_totem();
    new npc_blood_worms();
    new npc_amani_snake();
    new spell_hexlord_unstable_affliction();
}
